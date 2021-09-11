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

#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <map>
#include <set>
#include <shared_mutex>
#include <string>
#include "network_manager.h"

namespace OHOS::ObjectStore {
class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    virtual void OnNewMessage(const std::shared_ptr<Message> &message) = 0;
};

class DeviceObserver {
public:
    virtual ~DeviceObserver() = default;
    virtual void OnDeviceOnline(const device_t &device) = 0;
    virtual void OnDeviceOffline(const device_t &device) = 0;
};

class Communicator final : public NetworkObserver {
public:
    Communicator() = default;
    ~Communicator();
    uint32_t Init(const CommunicatorConfig &config);
    void Destory();
    uint32_t SendMessage(const device_t &target, std::shared_ptr<Message> message);
    uint32_t GetLocalDeviceId(device_t &local);
    uint32_t RegisterMessageHandler(MessageType type, MessageHandler *handler);
    uint32_t UnregisterMessageHandler(MessageType type, MessageHandler *handler);
    uint32_t RegisterDeviceObserver(DeviceObserver *observer);
    uint32_t UnregisterDeviceObserver(DeviceObserver *observer);
    uint32_t GetRemoteStoreIds(std::vector<std::string> &remoteId);
    void OnNodeOnline(const device_t &device) override;
    void OnNodeOffline(const device_t &device) override;
    void OnNewMessage(const std::shared_ptr<Message> &message) override;
private:
    NetworkManager networkManager_;
    std::map<MessageType, std::set<MessageHandler *>> messageHandlers_ {};
    std::shared_mutex handlersMutex_ {};
    std::set<DeviceObserver *> deviceObservers_ {};
    std::shared_mutex observerMutex_ {};
    CommunicatorConfig config_;
};
}  // namespace OHOS::ObjectStore

#endif