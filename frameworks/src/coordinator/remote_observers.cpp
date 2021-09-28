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

#include "remote_observers.h"
#include "objectstore_errors.h"
#include "observe_message.h"
#include "publish_message.h"
#include "publish_delete_message.h"

namespace OHOS::ObjectStore {
namespace {
constexpr uint32_t CHANGE_MERGE_LIMIT = 2;  // 2: more than 2 changes to do merge
}  // namespace
std::atomic<message_t> RemoteObservers::idGenerator_ { 1 };

RemoteObserverProxy::RemoteObserverProxy(device_t deviceId, Communicator &communicator)
    : deviceId_(deviceId), communicator_(communicator)
{
    stopNotify_ = false;
    notifyThread_ = std::thread(std::bind(&RemoteObserverProxy::NotifyLoop, this));
}

RemoteObserverProxy::~RemoteObserverProxy()
{
    stopNotify_ = true;
    notifyCv_.notify_one();
    if (notifyThread_.joinable()) {
        notifyThread_.join();
    }
}

void RemoteObserverProxy::NotifyLoop()
{
    auto changeMerge = [](std::deque<std::shared_ptr<Operation>> &notifyQueue) {
        if (notifyQueue.size() < CHANGE_MERGE_LIMIT) {
            return;
        }
        std::shared_ptr<Operation> dst = notifyQueue.front();
        notifyQueue.pop_front();
        while (!notifyQueue.empty()) {
            auto src = notifyQueue.front();
            if (dst->Merge(src) == SUCCESS) {
                notifyQueue.pop_front();
            } else {
                break;
            }
            notifyQueue.push_front(dst);
        }
    };
    while (!stopNotify_) {
        std::unique_lock<std::mutex> lock(observerInfoMutex_);
        bool hasChange = false;
        for (auto observerIte = observerInfo_.begin(); observerIte != observerInfo_.end(); observerIte++) {
            auto &notifyQueue = observerIte->second.notifyQueue;
            if (!notifyQueue.empty()) {
                hasChange = true;
                changeMerge(notifyQueue);
                if (PublishToRemote(notifyQueue.front()) == SUCCESS) {
                    notifyQueue.pop_front();
                }
            }
            auto &deleteQueue = observerIte->second.deleteQueue;
            if (!deleteQueue.empty()) {
                hasChange = true;
                if (PublishDeleteToRemote(deleteQueue.front()) == SUCCESS) {
                    deleteQueue.pop_front();
                }
            }
        }
        if (!hasChange) {
            LOG_INFO("RemoteObserverProxy-%s: start wait", __func__);
            notifyCv_.wait(lock);
            LOG_INFO("RemoteObserverProxy-%s: start process", __func__);
        }
    }
}

uint32_t RemoteObserverProxy::PublishDeleteToRemote(Bytes &key)
{
    std::shared_ptr<PublishDeleteOperation> publishOperation =
        std::make_shared<PublishDeleteOperation>(DataType::MAP_TYPE, key);
    uint32_t size = publishOperation->GetDataSize();
    std::vector<uint8_t> operationData;
    operationData.reserve(size);
    uint8_t *ptr = publishOperation->Encode();
    if (ptr == nullptr) {
        LOG_ERROR("RemoteObserverProxy-%s: fail to encode operation", __func__);
        return ERR_INVAL;
    }
    operationData.assign(ptr, ptr + size);
    std::shared_ptr<Message> publishDeleteRequest = std::make_shared<PublishDeleteRequest>(0, std::move(operationData));
    return SendMessage(deviceId_, publishDeleteRequest);
}

uint32_t RemoteObserverProxy::PublishToRemote(std::shared_ptr<Operation> &changes)
{
    PublishOperation *publishOperation = reinterpret_cast<PublishOperation *>(changes.get());
    uint32_t size = publishOperation->GetDataSize();
    std::vector<uint8_t> operationData;
    operationData.reserve(size);
    uint8_t *ptr = publishOperation->Encode();
    if (ptr == nullptr) {
        LOG_ERROR("RemoteObserverProxy-%s: fail to encode operation", __func__);
        return ERR_INVAL;
    }
    operationData.assign(ptr, ptr + size);
    std::shared_ptr<Message> publishRequest = std::make_shared<PublishRequest>(0, std::move(operationData));
    return SendMessage(deviceId_, publishRequest);
}

uint32_t RemoteObserverProxy::SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        LOG_ERROR("RemoteObserverProxy-%s: invalid message", __func__);
        return ERR_INVALMSG;
    }
    return communicator_.SendMessage(deviceId, message);
}

uint32_t RemoteObserverProxy::AddSubject(const Bytes &key, ObserverMode mode)
{
    std::unique_lock<std::mutex> lock(observerInfoMutex_);
    auto objectIte = observerInfo_.find(key);
    if (objectIte == observerInfo_.end()) {
        NotifyInfo notify;
        notify.mode = mode;
        observerInfo_.emplace(key, notify);
    } else {
        objectIte->second.mode = mode;
    }
    LOG_INFO("RemoteObserverProxy-%s: Succeed add observer", __func__);
    return SUCCESS;
}
uint32_t RemoteObserverProxy::RemoveSubject(const Bytes &key)
{
    std::unique_lock<std::mutex> lock(observerInfoMutex_);
    if (observerInfo_.erase(key) == 0) {
        LOG_ERROR("RemoteObserverProxy-%s: no such key subject", __func__);
        return ERR_NOENT;
    }
    LOG_INFO("RemoteObserverProxy-%s: remove subject success", __func__);
    return SUCCESS;
}

bool RemoteObserverProxy::IsEmptySubject()
{
    std::unique_lock<std::mutex> lock(observerInfoMutex_);
    return observerInfo_.empty();
}

uint32_t RemoteObserverProxy::Publish(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    std::unique_lock<std::mutex> lock(observerInfoMutex_);
    auto infoIte = observerInfo_.find(key);
    if (infoIte == observerInfo_.end()) {
        LOG_ERROR("RemoteObserverProxy-%s: no observer", __func__);
        return SUCCESS;
    }
    infoIte->second.notifyQueue.push_back(changes);
    LOG_ERROR("RemoteObserverProxy-%s: Publish change", __func__);
    notifyCv_.notify_one();
    return SUCCESS;
}

uint32_t RemoteObserverProxy::PublishDelete(const Bytes &key)
{
    std::unique_lock<std::mutex> lock(observerInfoMutex_);
    auto infoIte = observerInfo_.find(key);
    if (infoIte == observerInfo_.end()) {
        LOG_ERROR("RemoteObserverProxy-%s: no observer", __func__);
        return SUCCESS;
    }
    Bytes bytes = key;
    infoIte->second.deleteQueue.push_back(bytes);
    LOG_ERROR("RemoteObserverProxy-%s: Publish delete", __func__);
    notifyCv_.notify_one();
    return SUCCESS;
}

uint32_t RemoteObservers::Init()
{
    communicator_.RegisterMessageHandler(MessageType::OBSERVE_REQUEST, this);
    communicator_.RegisterMessageHandler(MessageType::REMOVE_OBSERVER_REQUEST, this);
    LOG_INFO("RemoteObservers-%s: remote observer init", __func__);
    return SUCCESS;
}

void RemoteObservers::Destory()
{
    communicator_.UnregisterMessageHandler(MessageType::OBSERVE_REQUEST, this);
    communicator_.UnregisterMessageHandler(MessageType::REMOVE_OBSERVER_REQUEST, this);
    LOG_INFO("RemoteObservers-%s: remote observer destory", __func__);
}

uint32_t RemoteObservers::Publish(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    std::shared_lock<std::shared_mutex> lock(observerMutex_);
    for (auto &[deviceId, observer] : observers_) {
        observer->Publish(key, changes);
    }
    return SUCCESS;
}

uint32_t RemoteObservers::PublishDelete(const Bytes &key)
{
    std::shared_lock<std::shared_mutex> lock(observerMutex_);
    for (auto &[deviceId, observer] : observers_) {
        observer->PublishDelete(key);
    }
    return SUCCESS;
}

void RemoteObservers::OnNewMessage(const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        return;
    }
    switch (message->GetType()) {
        case MessageType::OBSERVE_REQUEST:
            LOG_INFO("RemoteObservers-%s: OBSERVE_REQUEST", __func__);
            HandleAddObserverRequest(message);
            break;
        case MessageType::REMOVE_OBSERVER_REQUEST:
            LOG_INFO("RemoteObservers-%s: REMOVE_OBSERVER_REQUEST", __func__);
            HandleRemoveObserverRequest(message);
            break;
        default:
            break;
    }
}

uint32_t RemoteObservers::GetObserverObject(Bytes &key, std::vector<uint8_t> &operationData)
{
    std::shared_ptr<Operation> operation = std::make_shared<GetOperation>(DataType::MAP_TYPE, key);
    if (operation == nullptr) {
        return ERR_NOMEM;
    }
    auto ret = operation->Execute();
    if (ret != SUCCESS) {
        return ret;
    }
    std::any value;
    operation->GetOperationValue(value);
    std::shared_ptr<Operation> putOperation = std::make_shared<PutOperation>(DataType::MAP_TYPE, key, value);
    if (putOperation == nullptr) {
        LOG_ERROR("RemoteObservers-%s: fail to create operation", __func__);
        return ERR_NOMEM;
    }
    auto size = putOperation->GetDataSize();
    auto *ptr = putOperation->Encode();
    if (ptr == nullptr) {
        LOG_ERROR("RemoteObservers-%s: fail to encode operation", __func__);
        return ERR_INVAL;
    }
    operationData.reserve(size);
    operationData.assign(ptr, ptr + size);
    return SUCCESS;
}

void RemoteObservers::HandleAddObserverRequest(const std::shared_ptr<Message> &message)
{
    ObserveRequest *request = reinterpret_cast<ObserveRequest *>(message.get());
    Bytes key = request->key;
    ObserverMode mode = static_cast<ObserverMode>(request->mode);
    device_t deviceId;
    request->GetSource(deviceId);
    std::vector<uint8_t> operationData;
    auto ret = AddObserver(key, deviceId, mode);
    if (ret == SUCCESS) {
        ret = GetObserverObject(key, operationData);
    }
    auto response = std::make_shared<ObserveResponse>(idGenerator_++, request->GetId(), ret, std::move(operationData));
    SendMessage(deviceId, response);
}

void RemoteObservers::HandleRemoveObserverRequest(const std::shared_ptr<Message> &message)
{
    RemoveObserverRequest *request = reinterpret_cast<RemoveObserverRequest *>(message.get());
    Bytes key = request->key;
    device_t deviceId;
    request->GetSource(deviceId);
    RemoveObserver(key, deviceId);
}

uint32_t RemoteObservers::RemoveObserver(const Bytes &key, device_t &deviceId)
{
    std::unique_lock<std::shared_mutex> lock(observerMutex_);
    auto observerIte = observers_.find(deviceId);
    if (observerIte == observers_.end()) {
        LOG_ERROR("RemoteObservers-%s: fail to find the device observer", __func__);
        return ERR_NOENT;
    }
    auto ret = observerIte->second->RemoveSubject(key);
    if (ret != SUCCESS) {
        LOG_ERROR("RemoteObservers-%s: fail to remove the device observer", __func__);
    }
    if (observerIte->second->IsEmptySubject()) {
        observers_.erase(deviceId);
    }
    return ret;
}

uint32_t RemoteObservers::AddObserver(const Bytes &key, device_t &deviceId, ObserverMode mode)
{
    std::unique_lock<std::shared_mutex> lock(observerMutex_);
    auto deviceIte = observers_.find(deviceId);
    if (deviceIte == observers_.end()) {
        auto remoteObserverProxy = std::make_shared<RemoteObserverProxy>(deviceId, communicator_);
        if (remoteObserverProxy == nullptr) {
            LOG_ERROR("RemoteObservers-%s: fail to create device observer", __func__);
            return ERR_NOMEM;
        }
        deviceIte = observers_.emplace(deviceId, remoteObserverProxy).first;
    }
    return deviceIte->second->AddSubject(key, mode);
}

uint32_t RemoteObservers::SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        LOG_ERROR("RemoteObservers-%s: invalid message", __func__);
        return ERR_INVALMSG;
    }
    return communicator_.SendMessage(deviceId, message);
}
}  // namespace OHOS::ObjectStore