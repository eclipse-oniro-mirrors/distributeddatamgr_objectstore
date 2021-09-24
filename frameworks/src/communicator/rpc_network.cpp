/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "rpc_network.h"
#include <chrono>
#include <thread>
#include <functional>
#include "objectstore_errors.h"
#include "rpc_session.h"

namespace OHOS::ObjectStore {
namespace {
    constexpr uint32_t OPEN_SESSION_TIMEOUT = 20000;
} 

RPCNetwork::RPCNetwork(const std::string &name, const device_t &local, NetworkObserver *observer)
    : Network(name, local, observer)
{}

RPCNetwork::~RPCNetwork()
{
    LOG_INFO("RPCNetwork-%s", __func__);
}

uint32_t RPCNetwork::Init()
{
    networkListener_ = std::make_shared<SoftBusListener>(this);
    if (networkListener_ == nullptr) {
        return ERR_NOMEM;
    }

    networkListener_->Register();
    dataManager_.Init(networkListener_);
    return SUCCESS;
}

void RPCNetwork::Destory()
{
    dataManager_.Destroy();
    if (networkListener_ != nullptr) {
        networkListener_->UnRegister();
        networkListener_ = nullptr;
    }
    LOG_INFO("RPCNetwork-%s: Succeed to unregister and destory", __func__);
}

int32_t RPCNetwork::OpenSession(const std::string& networkId)
{
    LOG_INFO("RPCNetwork-%s, start Open session,%s", __func__, networkId.c_str());
    std::unique_lock<std::shared_mutex> cacheLock(openSessionMutex_);
    SessionInfo* session;
    {
        std::unique_lock<std::shared_mutex> cacheLock(sessionCacheMutex_);
        session = GetSessionFromCache(networkId);
        if (session != nullptr && session->connectStatus_ == CONNCET) {
            return 0;
        }
        session = &sessionCache_[networkId];
    }
    std::unique_lock<std::mutex> sessionLock(session->sessionMutex);
    session->connectStatus_ = DISCONNECT;
    session->waitCount++;
    dataManager_.OpenSoftbusLink(networkName_, networkId);
    if (session->connectStatus_ == CONNCET) {
        session->waitCount--;
        return 0;
    }
    if (session->CV.wait_for(sessionLock, std::chrono::milliseconds(OPEN_SESSION_TIMEOUT)) ==
        std::cv_status::timeout) {
        LOG_INFO("RPCNetwork-%s,Open session timeout", __func__);
        session->waitCount--;
        HandleSessionDisConnected(networkId);
        return -1;
    }
    session->waitCount--;
    LOG_INFO("RPCNetwork-%s, Open proxy success", __func__);
    return session->connectStatus_ == CONNCET ? 0 : -1;
}

void RPCNetwork::OnRemoteDied()
{
    std::unique_lock<std::shared_mutex> lock(sessionCacheMutex_);
    sessionCache_.clear();
    LOG_ERROR("RPCNetwork-%s: kv service died, clear all session", __func__);
}

SessionInfo* RPCNetwork::GetSessionFromCache(const std::string& networkId)
{
    auto ite = sessionCache_.find(networkId);
    if (ite != sessionCache_.end()) {
        return &ite->second;
    }
    return nullptr;
}

void RPCNetwork::CloseSession(const std::string &networkId)
{
    dataManager_.Destroy();
    {
        std::unique_lock<std::shared_mutex> lock(sessionCacheMutex_);
        sessionCache_.erase(networkId);
    }
}

int32_t RPCNetwork::OnSessionOpened(const std::string& deviceId)
{
    SessionInfo* sessionInfo = nullptr;
    {
        std::unique_lock<std::shared_mutex> lock(sessionCacheMutex_);
        sessionInfo = &sessionCache_[deviceId];
    }
    {
        std::unique_lock<std::mutex> sessionLock(sessionInfo->sessionMutex);
        sessionInfo->connectStatus_ = CONNCET;
        sessionInfo->CV.notify_one();
    }
    HandleSessionConnected(deviceId);
    LOG_INFO("RPCNetwork-%s:OnSessionOpened end %s", __func__, deviceId.c_str());
    return 0;
}

void RPCNetwork::OnSessionClosed(const std::string& deviceId)
{
    SessionInfo* sessionInfo;
    {
        std::unique_lock<std::shared_mutex> lock(sessionCacheMutex_);
        sessionInfo = &sessionCache_[deviceId];
    }
    std::unique_lock<std::mutex> sessionLock(sessionInfo->sessionMutex);
    sessionInfo->connectStatus_ = DISCONNECT;
    sessionInfo->CV.notify_one();
    HandleSessionDisConnected(deviceId);
    LOG_INFO("RPCNetwork-%s:OnSessionClosed", __func__);
    return;
}

void RPCNetwork::OnMessageReceived(const std::string& deviceId, const char *data, uint32_t len)
{
    std::shared_ptr<CommunicatorSession> session = std::make_shared<RPCSession>(const_cast<char *>(data), len);
    if (session == nullptr) {
        LOG_ERROR("RPCNetworkService-%s: fail to create session", __func__);
        return;
    }
    ReceiveData(session, 0);
    return;
}

uint32_t RPCNetwork::SendMessage(const std::string &networkId, std::shared_ptr<Message> message)
{
    uint32_t ret = OpenSession(networkId);
    if (ret != 0) {
        LOG_ERROR("RPCNetwork-%s: fail to OpenSession", __func__);
        return ret;
    }
    if (message != nullptr) {
        ret = WriteMessageToSession(networkId, message);
        if (ret == SUCCESS) {
            LOG_DEBUG("RPCNetwork-%s: Send message (%d: %d)", __func__, message->GetId(),
                      static_cast<uint8_t>(message->GetType()));
            return ret;
        }

        LOG_ERROR("RPCNetwork-%s: Failed to send message (%d: %d), write failure", __func__, message->GetId(),
                  static_cast<uint8_t>(message->GetType()));
    }
    return SUCCESS;
}

std::string RPCNetwork::GetDeviceIdBySession(int session)
{
    std::string devId;
    if (dataManager_.GetDeviceIdBySession(session, devId) != SUCCESS) {
        return std::string();
    }
    return devId;
}

uint32_t RPCNetwork::WriteMessageToSession(const std::string &networkId, std::shared_ptr<Message> &message)
{
    message->SetSource(local_.deviceId);
    size_t dataSize = 0;
    auto data = RPCSession::EncodeMessage(*message, dataSize);
    if (data == nullptr || dataSize < sizeof(MessageHeader)) {
        LOG_ERROR("Network-%s: encode message fail", __func__);
        return ERR_INVALMSG;
    }
    if (dataSize > MAX_SIZE_ONCE) {
        LOG_ERROR("Network-%s: message is too big", __func__);
        return ERR_INVALMSG;
    }

    uint32_t ret = (uint32_t)dataManager_.SendMsg(networkId, data, dataSize);
    if (ret  != SUCCESS) {
        LOG_ERROR("Network-%s: Failed to SendMsg, error: len = %zd, reason = %d", __func__, dataSize, ret);
        return ERR_NETWORK;
    }
    return SUCCESS;
}

int32_t SoftBusListener::OnSessionOpened(int32_t sessionId, int32_t result)
{
    if (network_ == nullptr) {
        LOG_INFO("Network-%s: null", __func__);
        return SUCCESS;
    }
    LOG_INFO("Network-%s: opened %d %d", __func__, sessionId, result);
    if (result != SUCCESS) {
        OnSessionClosed(sessionId);
        return result;
    }
    network_->OnSessionOpened(network_->GetDeviceIdBySession(sessionId));
    if (result != 0) {
        return ERR_NETWORK;
    }
    return SUCCESS;
}


void SoftBusListener::OnSessionClosed(int sessionId)
{
    if (network_ == nullptr) {
        LOG_INFO("Network-%s: null", __func__);
        return;
    }
    LOG_INFO("Network-%s: %d", __func__, sessionId);
    network_->OnSessionClosed(network_->GetDeviceIdBySession(sessionId));
}

void SoftBusListener::OnMessageReceived(int sessionId, const void *data, uint32_t len)
{
    if (network_ == nullptr) {
        LOG_INFO("Network-%s: null", __func__);
        return;
    }
    LOG_ERROR("Network-%s: OnMessageReceived %d", __func__, len);
    network_->OnMessageReceived(network_->GetDeviceIdBySession(sessionId), (char *)data, len);
}

RPCNetwork *SoftBusListener::network_ = nullptr;
SoftBusListener::SoftBusListener(RPCNetwork *rpcNetwork)
{
    network_ = rpcNetwork;
}

SoftBusListener::~SoftBusListener()
{
    UnRegister();
    network_ = nullptr;
}

INodeStateCb *SoftBusListener::GetNodeStateCallback() const
{
    return nodeStateCallback_;
}

ISessionListener *SoftBusListener::GetSessionListener() const
{
    return sessionListener_;
}

uint32_t SoftBusListener::Register()
{
    UnRegister();
    nodeStateCallback_ = new (std::nothrow) INodeStateCb;
    if (nodeStateCallback_ == nullptr) {
        return ERR_NOMEM;
    }
    
    sessionListener_ = new (std::nothrow) ISessionListener;
    if (sessionListener_ == nullptr) {
        return ERR_NOMEM;
    }

    sessionListener_->OnSessionOpened = OnSessionOpened;
    sessionListener_->OnBytesReceived = OnMessageReceived;
    sessionListener_->OnSessionClosed = OnSessionClosed;

    nodeStateCallback_->events = EVENT_NODE_STATE_ONLINE | EVENT_NODE_STATE_OFFLINE;
    nodeStateCallback_->onNodeOnline = OnNodeOnline;
    nodeStateCallback_->onNodeOffline = OnNodeOffline;
    return SUCCESS;
}

void SoftBusListener::OnNodeOnline(NodeBasicInfo *info)
{
    // char array can construct str directly
    if (network_ == nullptr) {
        return;
    }
    LOG_ERROR("Network-%s: node online %s", __func__, info->networkId);
    network_->AddRemoteStoreIds(info->networkId);
}

void SoftBusListener::OnNodeOffline(NodeBasicInfo *info)
{
    if (network_ == nullptr) {
        return;
    }
    LOG_ERROR("Network-%s: node offline %s", __func__, info->networkId);
    network_->RemoveRemoteStoreIds(info->networkId);
}

void SoftBusListener::UnRegister()
{
    if (nodeStateCallback_ != nullptr) {
        delete nodeStateCallback_;
        nodeStateCallback_ = nullptr;
    }
    if (sessionListener_ != nullptr) {
        delete sessionListener_;
        sessionListener_ = nullptr;
    }
}
}  // namespace OHOS::ObjectStore
