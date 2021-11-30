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

#include "network.h"

#include <functional>

#include "softbus_bus_center.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
namespace {
constexpr static uint32_t THREAD_NUM = 3;               // 3: thread pool size
};                                                      // namespace
Network::Network(const std::string &name, const device_t &local, NetworkObserver *observer)
    : networkName_(name), observer_(observer)
{
    local_.deviceId = local;
}

Network::~Network()
{
    LOG_INFO("destroy network");
}

uint32_t Network::Start()
{
    auto ret = InitLocalNetworkID();
    if (ret != SUCCESS) {
        return ret;
    }

    threadPool_.Start(THREAD_NUM);
    return Init();
}

void Network::Stop()
{
    Destory();
    {
        std::unique_lock<std::shared_mutex> lock(neighborsMutex_);
        neighbors_.clear();
    }
}

uint32_t Network::InitLocalNetworkID()
{
    NodeBasicInfo info;
    int32_t ret = GetLocalNodeDeviceInfo(PKG_NAME, &info);
    if (ret != SUCCESS) {
        LOG_ERROR("GetLocalNodeDeviceInfo error");
        return ERR_NETWORK;
    }
    local_.networkId = info.networkId;
    return SUCCESS;
}

uint32_t Network::GetDeviceNetworkId(const device_t &deviceId, std::string &networkId)
{
    auto ret = FindNetworkIdFromLocal(deviceId, networkId);
    if (ret == SUCCESS) {
        return ret;
    }
    ret = FindNetworkIdFromLocal(deviceId, networkId);
    if (ret == SUCCESS) {
        return ret;
    }
    LOG_ERROR("Network-%s: fail to find the target device ", __func__);
    return ret;
}

uint32_t Network::FindNetworkIdFromLocal(const device_t &deviceId, std::string &networkId)
{
    std::shared_lock<std::shared_mutex> lock(neighborsMutex_);
    auto neighbor = neighbors_.find(deviceId);
    if ((neighbor != neighbors_.end()) && (neighbor->second.status == NetworkStatus::ON_LINE)) {
        networkId = neighbor->second.networkId;
        return SUCCESS;
    }
    return ERR_NETWORK;
}

bool Network::ReceiveData(const std::string &deviceId, std::shared_ptr<CommunicatorSession> &session, uint32_t status)
{
    if (session == nullptr) {
        LOG_ERROR("Network-%s: Data is available but session is null", __func__);
        return false;
    }
    std::vector<std::shared_ptr<Message>> messages;
    if (!ReadMessageFromSession(session, messages)) {
        LOG_ERROR("Network-%s: Data is available but failed to read message", __func__);
        return false;
    }
    for (auto &message : messages) {
        message->SetSource(deviceId);
        LOG_DEBUG("Network-%s:Received message (%d: %d)", __func__, message->GetId(),
                  static_cast<uint8_t>(message->GetType()));
        if (observer_ != nullptr) {
            observer_->OnNewMessage(message);
        }
    }
    return true;
}

void Network::UpdateNeighborStatus(const NetworkNode &neighbor)
{
    if (neighbor.deviceId == "") {
        LOG_ERROR("Network-%s: deviceId is null", __func__);
        return;
    }
    device_t expiredNodeId = "";
    {
        std::unique_lock<std::shared_mutex> lock(neighborsMutex_);
        bool exist = false;
        for (auto &it : neighbors_) {
            if (it.second.networkId != neighbor.networkId) {
                continue;
            }
            if (it.second.status != neighbor.status) {
                it.second.status = neighbor.status;
                NotifyObservers(it.second);
            }
            exist = true;
            break;
        }
        if (!exist) {
            neighbors_.emplace(neighbor.deviceId, neighbor);
            NotifyObservers(neighbor);
        }
    }
    return;
}

void Network::NotifyObservers(const NetworkNode &neighbor) const
{
    if (observer_ == nullptr) {
        LOG_WARN("Network-%s: no observer", __func__);
        return;
    }
    switch (neighbor.status) {
        case NetworkStatus::ON_LINE:
            LOG_INFO("Network-%s: new neighbor online", __func__);
            observer_->OnNodeOnline(neighbor.deviceId);
            break;
        case NetworkStatus::OFF_LINE:
            LOG_INFO("Network-%s: neighbor offline", __func__);
            observer_->OnNodeOffline(neighbor.deviceId);
            break;
        default:
            break;
    }
}

void Network::HandleSessionConnected(const std::string &deviceId)
{
    NetworkNode neighbor;
    neighbor.deviceId = deviceId;
    neighbor.networkId = deviceId;
    neighbor.status = NetworkStatus::ON_LINE;
    UpdateNeighborStatus(neighbor);
}

void Network::HandleSessionDisConnected(const std::string &deviceId)
{
    NetworkNode neighbor;
    neighbor.deviceId = deviceId;
    neighbor.networkId = deviceId;
    neighbor.status = NetworkStatus::OFF_LINE;
    UpdateNeighborStatus(neighbor);
}

DataBuffer *Network::GetSessionDataBuffer()
{
    DataBuffer *dataBuffer = new (std::nothrow) DataBuffer();
    if (dataBuffer == nullptr) {
        LOG_ERROR("Network-%s: Failed to create buffer for session", __func__);
        return nullptr;
    }
    if (!dataBuffer->Init(MAX_SIZE_ONCE)) {
        delete dataBuffer;
        return nullptr;
    }
    return dataBuffer;
}

bool Network::ReadMessageFromSession(std::shared_ptr<CommunicatorSession> &session,
    std::vector<std::shared_ptr<Message>> &messages)
{
    DataBuffer *dataBuffer = GetSessionDataBuffer();
    if (dataBuffer == nullptr) {
        LOG_ERROR("Network-%s: buffer of session is null", __func__);
        return false;
    }
    auto &buffer = dataBuffer->data;
    size_t maxSize = dataBuffer->capacity;
    size_t usedSize = dataBuffer->usedSize;
    ssize_t rc = session->Recv(buffer.get(), maxSize, 0);
    if (rc <= 0) {
        LOG_ERROR("Network-%s: CommunicatorSession is broken down, rc = %zd", __func__, rc);
        delete dataBuffer;
        return false;
    }

    usedSize += rc;
    size_t cursor = 0;
    while (cursor + sizeof(MessageHeader) < usedSize) {
        MessageHeader *msgHeader = reinterpret_cast<MessageHeader *>(buffer.get() + cursor);
        if (msgHeader->dataSize < sizeof(MessageHeader) || msgHeader->dataSize > MAX_SIZE_ONCE) {
            cursor = usedSize;
            LOG_ERROR("Network-%s: received illegal message, dataSize = %d, version is %u", __func__,
                      msgHeader->dataSize, msgHeader->version);
            break;
        }
        size_t decodeSize = 0;
        auto message = session->DecodeMessage(*msgHeader, usedSize - cursor, decodeSize);
        if (decodeSize == 0) {
            break;
        }

        if (message != nullptr) {
            messages.push_back(message);
        }
        cursor += decodeSize;
    }
    delete dataBuffer;
    return true;
}

uint32_t Network::AddRemoteStoreIds(const std::string &remoteId)
{
    return SUCCESS;
}

uint32_t Network::RemoveRemoteStoreIds(const std::string &remoteId)
{
    return SUCCESS;
}

uint32_t Network::GetRemoteStoreIds(std::vector<std::string> &remoteId)
{
    NodeBasicInfo *info = nullptr;
    int32_t size;
    remoteId.clear();
    GetAllNodeDeviceInfo(PKG_NAME, &info, &size);

    if (size == 0 || info == nullptr) {
        LOG_INFO("Network-%s: single device", __func__);
        return SUCCESS;
    }

    uint32_t i;
    for (i = 0; i < size; i++) {
        LOG_INFO("Network-%s: device %s", __func__, info[i].networkId);
        remoteId.insert(remoteId.end(), info[i].networkId);
    }
    return SUCCESS;
}
}  // namespace OHOS::ObjectStore
