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

#ifndef REMOTE_OBSERVERS_H
#define REMOTE_OBSERVERS_H

#include <deque>
#include "communicator.h"
#include "object_observer.h"
#include "operation.h"

namespace OHOS::ObjectStore {
class RemoteObserverProxy final {
public:
    RemoteObserverProxy(device_t deviceId, Communicator &communicator);
    ~RemoteObserverProxy();
    uint32_t AddSubject(const Bytes &key, ObserverMode mode);
    uint32_t RemoveSubject(const Bytes &key);
    uint32_t Publish(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t PublishDelete(const Bytes &key);
    bool IsEmptySubject();

private:
    struct NotifyInfo {
        ObserverMode mode;
        std::deque<std::shared_ptr<Operation>> notifyQueue {};
        std::deque<Bytes> deleteQueue {};
    };
    void NotifyLoop();
    uint32_t PublishToRemote(std::shared_ptr<Operation> &changes);
    uint32_t PublishDeleteToRemote(Bytes &key);
    uint32_t SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message);
    std::map<Bytes, NotifyInfo> observerInfo_ {};
    std::mutex observerInfoMutex_ {};
    std::thread notifyThread_;
    bool stopNotify_ { false };
    std::condition_variable notifyCv_ {};
    device_t deviceId_ { "" };
    Communicator &communicator_;
};

class RemoteObservers final : public MessageHandler {
public:
    RemoteObservers(Communicator &communicator_) : communicator_(communicator_)
    {}
    ~RemoteObservers() = default;
    uint32_t Init();
    void Destory();
    uint32_t Publish(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t PublishDelete(const Bytes &key);

private:
    void OnNewMessage(const std::shared_ptr<Message> &message) override;
    void HandleAddObserverRequest(const std::shared_ptr<Message> &message);
    void HandleRemoveObserverRequest(const std::shared_ptr<Message> &message);
    uint32_t AddObserver(const Bytes &key, device_t &deviceId, ObserverMode mode);
    uint32_t RemoveObserver(const Bytes &key, device_t &deviceId);
    uint32_t SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message);
    uint32_t GetObserverObject(Bytes &key, std::vector<uint8_t> &operationData);
    std::map<device_t, std::shared_ptr<RemoteObserverProxy>> observers_ {};
    std::shared_mutex observerMutex_ {};
    static std::atomic<message_t> idGenerator_;
    Communicator &communicator_;
};
}  // namespace OHOS::ObjectStore
#endif