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

#ifndef LOCAL_OBSERVERS_H
#define LOCAL_OBSERVERS_H

#include <map>
#include <mutex>
#include <queue>
#include "communicator.h"
#include "message_waiter.h"
#include "object_observer.h"
#include "operation.h"

namespace OHOS::ObjectStore {
class LocalObservers final : public MessageHandler, public DeviceObserver {
public:
    LocalObservers(Communicator &communicator) : communicator_(communicator)
    {}
    ~LocalObservers() = default;
    uint32_t Init();
    void Destory();
    uint32_t AddObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer, ObserverMode mode);
    uint32_t RemoveObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer);
    uint32_t Publish(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t PublishDelete(const Bytes &key);

private:
    enum class ObserveCmd {
        INVALID_CMD = 0,
        ADD_OBSERVER,
        REMOVE_OBSERVER,
    };
    struct LocalObjectInfo {
        std::map<std::shared_ptr<ObjectObserver>, ObserverMode> observers {};
        std::deque<std::shared_ptr<Operation>> changeQueue {};
        std::deque<Bytes> deleteQueue {};
    };
    struct ObserveInfo {
        Bytes key;
        ObserveCmd cmd { ObserveCmd::INVALID_CMD };
        ObserverMode mode { ObserverMode::OBSERVER_DEFAULT_MODE };
        ObserveInfo(const Bytes &key, ObserveCmd cmd, ObserverMode mode = ObserverMode::OBSERVER_DEFAULT_MODE)
            : key(key), cmd(cmd), mode(mode)
        {}
    };
    uint32_t AddLocalObjectObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer, ObserverMode mode);
    uint32_t AddRemoteObjectObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer, ObserverMode mode);
    uint32_t SendAddObserverRequest(const Bytes &key, ObserverMode mode);
    void OnNewMessage(const std::shared_ptr<Message> &message) override;
    uint32_t SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message);
    uint32_t SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &request,
                         std::shared_ptr<Message> &response);
    uint32_t SendRemoveObserverRequest(const Bytes &key);
    uint32_t RemoveLocalObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer);
    uint32_t SendRemoteObserveRequest(ObserveInfo &request);
    void NotifyLoop();
    void NotifyObservers(const Bytes &key);
    void NotifyDeleteObservers(const Bytes &key);
    void ObserveLoop();
    void AddObserveCmd(const Bytes &key, ObserveCmd cmd, ObserverMode mode = ObserverMode::OBSERVER_DEFAULT_MODE);
    device_t GetObjectHostId(const Bytes &key);
    void OnDeviceOnline(const device_t &device) override;
    void OnDeviceOffline(const device_t &device) override;
    void HandlePublishRequest(const std::shared_ptr<Message> &message);
    void HandlePublishDeleteRequest(const std::shared_ptr<Message> &message);
    void ReSubRemoteObjects();
    static std::atomic<message_t> idGenerator_;
    std::map<message_t, std::shared_ptr<MessageWaiter>> waiters_ {};
    std::mutex waiterMutex_ {};
    Communicator &communicator_;
    std::map<Bytes, LocalObjectInfo> observerMap_ {};
    std::mutex observerMutex_ {};
    std::condition_variable notifyCv_ {};
    std::unique_ptr<std::thread> notifyThreadPtr_ { nullptr };
    bool terminate_ { false };
    bool IsLocalObject(const Bytes &key);
    std::string localDeviceId_ {};
    std::queue<ObserveInfo> remoteObserveQueue_ {};
    std::mutex remoteObserveMutex_ {};
    std::condition_variable remoteObserveCV_ {};
    std::unique_ptr<std::thread> observeThreadPtr_ { nullptr };
};
}  // namespace OHOS::ObjectStore
#endif