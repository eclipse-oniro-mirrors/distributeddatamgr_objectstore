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

#include "communicator.h"
#include "logger.h"
#include "network_types.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
uint32_t Communicator::Init(const CommunicatorConfig &config)
{
    if (config.appName.empty()) {
        LOG_ERROR("Communicator-%s: invalid config", __func__);
        return ERR_INVAL;
    }
    auto ret = networkManager_.Init();
    if (ret != SUCCESS) {
        LOG_ERROR("Communicator-%s: init network manager fail", __func__);
        return ret;
    }
    LOG_INFO("Communicator-%s: init", __func__);
    return networkManager_.CreateNetwork(config.appName, NetworkType::RPC, this);
}

Communicator::~Communicator()
{
    LOG_INFO("Communicator-%s: communicator start descontrouctor", __func__);
}

void Communicator::Destory()
{
    std::unique_lock<std::shared_mutex> lock(handlersMutex_);
    messageHandlers_.clear();
    networkManager_.Destory();
    LOG_INFO("Communicator-%s: communicator destory", __func__);
}

uint32_t Communicator::SendMessage(const device_t &target, std::shared_ptr<Message> message)
{
    if (message == nullptr) {
        return ERR_INVAL;
    }
    return networkManager_.SendMessage(target, message);
}

uint32_t Communicator::GetLocalDeviceId(device_t &local)
{
    return networkManager_.GetLocalDeviceId(local);
}

void Communicator::OnNodeOnline(const device_t &device)
{
    std::shared_lock<std::shared_mutex> lock(observerMutex_);
    for (auto &observer : deviceObservers_) {
        observer->OnDeviceOnline(device);
    }
}

void Communicator::OnNodeOffline(const device_t &device)
{
    std::shared_lock<std::shared_mutex> lock(observerMutex_);
    for (auto &observer : deviceObservers_) {
        observer->OnDeviceOffline(device);
    }
}

void Communicator::OnNewMessage(const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        return;
    }
    std::set<MessageHandler *> handlers;
    {
        std::shared_lock<std::shared_mutex> lock(handlersMutex_);
        auto it = messageHandlers_.find(message->GetType());
        if (it != messageHandlers_.end()) {
            handlers = it->second;
        }
    }
    for (auto handler : handlers) {
        handler->OnNewMessage(message);
    }
}

uint32_t Communicator::RegisterMessageHandler(MessageType type, MessageHandler *handler)
{
    if (handler == nullptr) {
        return ERR_INVAL;
    }
    std::unique_lock<std::shared_mutex> lock(handlersMutex_);
    auto it = messageHandlers_.find(type);
    if (it == messageHandlers_.end()) {
        auto result = messageHandlers_.emplace(type, std::set<MessageHandler *>());
        it = result.first;
    }
    it->second.insert(handler);
    return SUCCESS;
}

uint32_t Communicator::UnregisterMessageHandler(MessageType type, MessageHandler *handler)
{
    if (handler == nullptr) {
        return ERR_INVAL;
    }
    std::unique_lock<std::shared_mutex> lock(handlersMutex_);
    auto it = messageHandlers_.find(type);
    if (it != messageHandlers_.end()) {
        it->second.erase(handler);
    }
    return SUCCESS;
}

uint32_t Communicator::RegisterDeviceObserver(DeviceObserver *observer)
{
    if (observer == nullptr) {
        LOG_ERROR("Communicator-%s: invalid param", __func__);
        return ERR_INVAL;
    }
    std::unique_lock<std::shared_mutex> lock(observerMutex_);
    deviceObservers_.insert(observer);
    return SUCCESS;
}

uint32_t Communicator::UnregisterDeviceObserver(DeviceObserver *observer)
{
    if (observer == nullptr) {
        LOG_ERROR("Communicator-%s: invalid param", __func__);
        return ERR_INVAL;
    }
    std::unique_lock<std::shared_mutex> lock(observerMutex_);
    deviceObservers_.erase(observer);
    return SUCCESS;
}

uint32_t Communicator::GetRemoteStoreIds(std::vector<std::string> &remoteId)
{
    return networkManager_.GetRemoteStoreIds(remoteId);
}
};  // namespace OHOS::ObjectStore