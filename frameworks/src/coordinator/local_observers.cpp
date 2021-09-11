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

#include "local_observers.h"
#include "message.h"
#include "object_utils.h"
#include "objectstore_errors.h"
#include "observe_message.h"
#include "publish_message.h"
#include "string_utils.h"

namespace OHOS::ObjectStore {
std::atomic<message_t> LocalObservers::idGenerator_ { 1 };
namespace {
constexpr uint32_t RETRY_LIMIT = 3;
constexpr uint32_t RETRY_WAIT_TIME = 10;
}  // namespace

uint32_t LocalObservers::Init()
{
    device_t deviceId;
    auto ret = communicator_.GetLocalDeviceId(deviceId);
    if (ret != SUCCESS) {
        return ret;
    }
    localDeviceId_ = deviceId;
    communicator_.RegisterMessageHandler(MessageType::OBSERVE_RESPONSE, this);
    communicator_.RegisterMessageHandler(MessageType::PUBLISH_REQUEST, this);
    communicator_.RegisterMessageHandler(MessageType::PUBLISH_DELETE_REQUEST, this);
    terminate_ = false;
    notifyThreadPtr_ = std::make_unique<std::thread>(std::bind(&LocalObservers::NotifyLoop, this));
    if (notifyThreadPtr_ == nullptr) {
        LOG_INFO("LocalObservers-%s: fail to create notify thread", __func__);
        return ERR_NOMEM;
    }
    observeThreadPtr_ = std::make_unique<std::thread>(std::bind(&LocalObservers::ObserveLoop, this));
    if (observeThreadPtr_ == nullptr) {
        LOG_INFO("LocalObservers-%s: fail to create observer thread", __func__);
        return ERR_NOMEM;
    }
    communicator_.RegisterDeviceObserver(this);
    LOG_INFO("LocalObservers-%s: local observers init success", __func__);
    return SUCCESS;
}

void LocalObservers::Destory()
{
    communicator_.UnregisterDeviceObserver(this);
    terminate_ = true;
    notifyCv_.notify_one();
    if (notifyThreadPtr_ != nullptr && notifyThreadPtr_->joinable()) {
        notifyThreadPtr_->join();
        notifyThreadPtr_.reset();
    }
    remoteObserveCV_.notify_one();
    if (observeThreadPtr_ != nullptr && observeThreadPtr_->joinable()) {
        observeThreadPtr_->join();
        observeThreadPtr_.reset();
    }
    communicator_.UnregisterMessageHandler(MessageType::OBSERVE_RESPONSE, this);
    communicator_.UnregisterMessageHandler(MessageType::PUBLISH_REQUEST, this);
    communicator_.UnregisterMessageHandler(MessageType::PUBLISH_DELETE_REQUEST, this);
}

void LocalObservers::ObserveLoop()
{
    while (!terminate_) {
        std::unique_lock<std::mutex> lock(remoteObserveMutex_);
        LOG_INFO("LocalObservers-%s: start loop", __func__);
        while (!remoteObserveQueue_.empty()) {
            auto &request = remoteObserveQueue_.front();
            uint32_t retry = 0;
            uint32_t ret = SUCCESS;
            while ((ret = SendRemoteObserveRequest(request)) == ERR_NETWORK) {
                if (++retry >= RETRY_LIMIT || terminate_) {
                    break;
                }
                LOG_WARN("LocalObservers-%s: Retry remote request %u", __func__, retry);
                std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_TIME));
                if (terminate_) {
                    return;
                }
            }
            if (terminate_) {
                return;
            }
            if (ret == ERR_NETWORK) {
                break;
            }
            remoteObserveQueue_.pop();
        }
        remoteObserveCV_.wait(lock);
    }
}


void LocalObservers::OnDeviceOnline(const device_t &device)
{
    LOG_INFO("LocalObservers-%s: device online", __func__);
    ReSubRemoteObjects();
}

void LocalObservers::OnDeviceOffline(const device_t &device)
{
    LOG_INFO("LocalObservers-%s: device offline", __func__);
}

uint32_t LocalObservers::SendRemoteObserveRequest(ObserveInfo &request)
{
    switch (request.cmd) {
        case ObserveCmd::ADD_OBSERVER:
            return SendAddObserverRequest(request.key, request.mode);
        case ObserveCmd::REMOVE_OBSERVER:
            return SendRemoveObserverRequest(request.key);
        default:
            LOG_ERROR("LocalObservers-%s: invalid observe cmd", __func__);
            return ERR_INVAL;
    }
}

void LocalObservers::NotifyLoop()
{
    while (!terminate_) {
        std::unique_lock<std::mutex> lock(observerMutex_);
        for (auto objectIte = observerMap_.begin(); objectIte != observerMap_.end(); objectIte++) {
            if (!objectIte->second.changeQueue.empty()) {
                NotifyObservers(objectIte->first);
                objectIte->second.changeQueue.clear();
            }
            if (!objectIte->second.deleteQueue.empty()) {
                NotifyDeleteObservers(objectIte->first);
                objectIte->second.deleteQueue.clear();
            }
        }
        notifyCv_.wait(lock);
    }
}

void LocalObservers::NotifyObservers(const Bytes &key)
{
    auto objectIte = observerMap_.find(key);
    std::for_each(objectIte->second.observers.begin(), objectIte->second.observers.end(),
                  [&key](auto &observer) { observer.first->OnChanged(key); });
}

void LocalObservers::NotifyDeleteObservers(const Bytes &key)
{
    auto objectIte = observerMap_.find(key);
    std::for_each(objectIte->second.observers.begin(), objectIte->second.observers.end(),
                  [&key](auto &observer) { observer.first->OnDeleted(key); });
}


uint32_t LocalObservers::Publish(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    std::unique_lock<std::mutex> lock(observerMutex_);
    auto objectIte = observerMap_.find(key);
    if (objectIte == observerMap_.end()) {
        return SUCCESS;
    }
    objectIte->second.changeQueue.push_back(changes);
    notifyCv_.notify_one();
    LOG_INFO("LocalObservers-%s: publish changes", __func__);
    return SUCCESS;
}

uint32_t LocalObservers::PublishDelete(const Bytes &key)
{
    std::unique_lock<std::mutex> lock(observerMutex_);
    auto objectIte = observerMap_.find(key);
    if (objectIte == observerMap_.end()) {
        return SUCCESS;
    }
    Bytes keytmp = key;
    objectIte->second.deleteQueue.push_back(keytmp);
    notifyCv_.notify_one();
    LOG_INFO("LocalObservers-%s: publish delete", __func__);
    return SUCCESS;
}

uint32_t LocalObservers::AddObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer, ObserverMode mode)
{
    if (observer == nullptr) {
        LOG_ERROR("LocalObservers-%s: invalid observer", __func__);
        return ERR_INVAL;
    }

    if (IsLocalObject(key)) {
        return AddLocalObjectObserver(key, observer, mode);
    } else {
        return AddRemoteObjectObserver(key, observer, mode);
    }
}

uint32_t LocalObservers::RemoveObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer)
{
    auto ret = RemoveLocalObserver(key, observer);
    if (ret != SUCCESS) {
        return ret;
    }
    {
        std::unique_lock<std::mutex> lock(observerMutex_);
        if (observerMap_.count(key) != 0) {
            return ret;
        }
        lock.unlock();
        if (!IsLocalObject(key)) {
            AddObserveCmd(key, ObserveCmd::REMOVE_OBSERVER);
        }
    }
    return ret;
}

uint32_t LocalObservers::RemoveLocalObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer)
{
    std::unique_lock<std::mutex> lock(observerMutex_);
    auto objectIte = observerMap_.find(key);
    if (objectIte == observerMap_.end()) {
        LOG_ERROR("LocalObservers-%s: no such key observer", __func__);
        return ERR_NOENT;
    }
    if (objectIte->second.observers.erase(observer) == 0) {
        LOG_ERROR("LocalObservers-%s: no such observer", __func__);
        return ERR_NOENT;
    }
    if (objectIte->second.observers.empty()) {
        observerMap_.erase(objectIte);
    }
    LOG_INFO("LocalObservers-%s: remove observer success", __func__);
    return SUCCESS;
}

uint32_t LocalObservers::SendRemoveObserverRequest(const Bytes &key)
{
    std::shared_ptr<Message> request = std::make_shared<RemoveObserverRequest>(idGenerator_++, key);
    if (request == nullptr) {
        LOG_ERROR("LocalObservers-%s: fail to create message object", __func__);
        return ERR_NOMEM;
    }
    auto ret = SendMessage(GetObjectHostId(key), request);
    if (ret != SUCCESS || request == nullptr) {
        return ERR_NETWORK;
    }
    return SUCCESS;
}

uint32_t LocalObservers::AddLocalObjectObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer,
                                                ObserverMode mode)
{
    std::unique_lock<std::mutex> lock(observerMutex_);
    auto objectIte = observerMap_.find(key);
    if (objectIte == observerMap_.end()) {
        objectIte = observerMap_.emplace(key, LocalObjectInfo {}).first;
    }
    auto observerIte = objectIte->second.observers.find(observer);
    if (observerIte == objectIte->second.observers.end()) {
        objectIte->second.observers.emplace(observer, mode);
        LOG_INFO("LocalObservers-%s: add observer success", __func__);
        return SUCCESS;
    }
    if (observerIte->second == mode) {
        LOG_WARN("LocalObservers-%s: observer already exist", __func__);
        return ERR_EXIST;
    }
    observerIte->second = mode;
    LOG_INFO("LocalObservers-%s: update observer mode", __func__);
    return SUCCESS;
}

void LocalObservers::AddObserveCmd(const Bytes &key, ObserveCmd cmd, ObserverMode mode)
{
    std::unique_lock<std::mutex> lock(remoteObserveMutex_);
    remoteObserveQueue_.emplace(key, cmd, mode);
    remoteObserveCV_.notify_one();
}

void LocalObservers::ReSubRemoteObjects()
{
    std::unique_lock<std::mutex> lock(observerMutex_);
    {
        std::unique_lock<std::mutex> lock(remoteObserveMutex_);
        for (auto observer:observerMap_) {
            if (!IsLocalObject(observer.first)) {
                remoteObserveQueue_.emplace(observer.first, ObserveCmd::ADD_OBSERVER,
                    ObserverMode::OBSERVER_DEFAULT_MODE);
            }
        }
    }
    remoteObserveCV_.notify_one();
    LOG_INFO("LocalObservers-%s: ReSubRemoteObjects", __func__);
}

uint32_t LocalObservers::AddRemoteObjectObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer,
                                                 ObserverMode mode)
{
    std::unique_lock<std::mutex> lock(observerMutex_);
    auto objectIte = observerMap_.find(key);
    if (objectIte == observerMap_.end()) {
        AddObserveCmd(key, ObserveCmd::ADD_OBSERVER, mode);
        objectIte = observerMap_.emplace(key, LocalObjectInfo {}).first;
    }
    auto observerIte = objectIte->second.observers.find(observer);
    if (observerIte == objectIte->second.observers.end()) {
        objectIte->second.observers.emplace(observer, mode);
        LOG_INFO("LocalObservers-%s: add observer success", __func__);
        return SUCCESS;
    }
    if (observerIte->second == mode) {
        LOG_WARN("LocalObservers-%s: observer already exist", __func__);
        return ERR_EXIST;
    }
    observerIte->second = mode;
    LOG_INFO("LocalObservers-%s: update observer mode", __func__);
    return SUCCESS;
}

uint32_t LocalObservers::SendAddObserverRequest(const Bytes &key, ObserverMode mode)
{
    std::shared_ptr<Message> request =
        std::make_shared<ObserveRequest>(idGenerator_++, key, static_cast<uint32_t>(mode));
    if (request == nullptr) {
        LOG_ERROR("LocalObservers-%s: fail to create message object", __func__);
        return ERR_NOMEM;
    }
    std::shared_ptr<Message> response = nullptr;
    auto ret = SendMessage(GetObjectHostId(key), request, response);
    if (ret != SUCCESS || response == nullptr) {
        LOG_ERROR("LocalObservers-%s: fail to send request", __func__);
        return ERR_NETWORK;
    }
    ObserveResponse *observeResponse = reinterpret_cast<ObserveResponse *>(response.get());
    ret = observeResponse->response;
    if (ret != SUCCESS) {
        LOG_ERROR("LocalObservers-%s: fail to observe remote object", __func__);
        return ret;
    }
    std::shared_ptr<Operation> putOperation =
        std::make_shared<PutOperation>(observeResponse->operation.data(), observeResponse->operationLen);
    if (putOperation == nullptr) {
        LOG_ERROR("LocalObservers-%s: fail to create operation", __func__);
        return ERR_NOMEM;
    }
    ret = putOperation->Execute();
    return ret;
}

bool LocalObservers::IsLocalObject(const Bytes &key)
{
    std::string objectId(key.begin(), key.end());
    auto host = ObjectUtils::GetObjectHost(objectId);
    return localDeviceId_.compare(host) == 0;
}

uint32_t LocalObservers::SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        LOG_ERROR("LocalObservers-%s: invalid message or device id", __func__);
        return ERR_INVALMSG;
    }
    return communicator_.SendMessage(deviceId, message);
}

uint32_t LocalObservers::SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &request,
                                     std::shared_ptr<Message> &response)
{
    auto ret = SendMessage(deviceId, request);
    if (ret != SUCCESS) {
        return ret;
    }
    if (request->GetTimeout() == 0) {
        LOG_ERROR("LocalObservers-%s: wait reponse need assign time out", __func__);
        return ERR_INVALMSG;
    }
    auto waiter = std::make_shared<MessageWaiter>();
    if (waiter == nullptr) {
        LOG_ERROR("LocalObservers-%s: fail create message waiter", __func__);
        return ERR_NOMEM;
    }
    {
        std::unique_lock<std::mutex> lock(waiterMutex_);
        waiters_.emplace(request->GetId(), waiter);
    }
    ret = waiter->Wait(request->GetTimeout(), response);
    {
        std::unique_lock<std::mutex> lock(waiterMutex_);
        waiters_.erase(request->GetId());
    }
    return ret;
}

void LocalObservers::HandlePublishRequest(const std::shared_ptr<Message> &message)
{
    LOG_INFO("LocalObservers-%s: handle publish request", __func__);
    PublishRequest *request = reinterpret_cast<PublishRequest *>(message.get());
    uint32_t size = request->operationLen;
    std::vector<uint8_t> operationData(request->operationData);
    std::shared_ptr<Operation> operation = std::make_shared<PublishOperation>(operationData.data(), size);
    if (operation == nullptr) {
        LOG_ERROR("LocalObservers-%s: Fail to create operation", __func__);
        return;
    }
    operation->Execute();
    Bytes key;
    operation->GetOperationKey(key);
    Publish(key, operation);
}

void LocalObservers::HandlePublishDeleteRequest(const std::shared_ptr<Message> &message)
{
    LOG_INFO("LocalObservers-%s: handle publish delete request", __func__);
    PublishRequest *request = reinterpret_cast<PublishRequest *>(message.get());
    uint32_t size = request->operationLen;
    std::vector<uint8_t> operationData(request->operationData);
    std::shared_ptr<Operation> operation = std::make_shared<PublishDeleteOperation>(operationData.data(), size);
    if (operation == nullptr) {
        LOG_ERROR("LocalObservers-%s: Fail to create operation", __func__);
        return;
    }
    operation->Execute();
    Bytes key;
    operation->GetOperationKey(key);
    PublishDelete(key);
}


void LocalObservers::OnNewMessage(const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        return;
    }
    switch (message->GetType()) {
        case MessageType::PUBLISH_REQUEST:
            return HandlePublishRequest(message);
        case MessageType::PUBLISH_DELETE_REQUEST:
            return HandlePublishDeleteRequest(message);
        default:
            LOG_INFO("LocalObservers-%s: receive response message", __func__);
    }
    std::unique_lock<std::mutex> lock(waiterMutex_);
    auto it = waiters_.find(message->GetRequestId());
    if (it != waiters_.end()) {
        it->second->Signal(message);
    }
}

device_t LocalObservers::GetObjectHostId(const Bytes &key)
{
    return ObjectUtils::GetObjectHost(std::string(key.begin(), key.end()));
}
}  // namespace OHOS::ObjectStore
