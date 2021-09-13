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

#ifndef REMOTE_OBJECT_TASK_H
#define REMOTE_OBJECT_TASK_H

#include <deque>
#include "bytes.h"
#include "communicator.h"
#include "message_waiter.h"
#include "observer_manager.h"
#include "operation.h"

namespace OHOS::ObjectStore {
class RemoteObjectTask final : public MessageHandler {
public:
    RemoteObjectTask(Communicator &communicator, ObserverManager &observerManager)
        : communicator_(communicator)
    {}
    ~RemoteObjectTask() = default;
    uint32_t Init();
    void Destory();
    uint32_t PutRemote(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t PutRemoteAsync(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t GetRemote(const Bytes &key, std::shared_ptr<Operation> &values);
    uint32_t DeleteRemote(const Bytes &key);
    uint32_t DeleteRemoteAsync(const Bytes &key);

private:
    class SyncObjectObserver : public ObjectObserver {
    public:
        SyncObjectObserver() = default;
        ~SyncObjectObserver() = default;
        void OnChanged(const Bytes &key)
        {
            LOG_INFO("SyncObjectObserver-%s: receive %s change", __func__, key.data());
        }
        void OnDeleted(const Bytes &key)
        {}
    };
    using TaskQueue = std::deque<std::shared_ptr<Operation>>;
    void OnNewMessage(const std::shared_ptr<Message> &message) override;
    uint32_t SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message);
    uint32_t SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &request,
                         std::shared_ptr<Message> &response);
    uint32_t AddRemoteTask(const Bytes &key, std::shared_ptr<Operation> &changes);
    void TaskLoop();
    void MergeTask(TaskQueue &taskQueue);
    uint32_t DoRemoteTask(const Bytes &key, std::shared_ptr<Operation> &changes);
    device_t GetObjectHostId(const Bytes &key);
    uint32_t GetFromLocal(const Bytes &key, std::shared_ptr<Operation> &values);
    uint32_t DoGetRemote(const Bytes &key, std::shared_ptr<Operation> &values);
    Communicator &communicator_;
    static std::atomic<message_t> idGenerator_;
    std::map<message_t, std::shared_ptr<MessageWaiter>> waiters_;
    std::mutex waiterMutex_ {};
    std::map<device_t, std::map<Bytes, TaskQueue>> tasks_ {};
    std::mutex taskMutex_ {};
    std::condition_variable taskCV_ {};
    std::unique_ptr<std::thread> taskThreadPtr_ { nullptr };
    bool stoptask_ { false };
};
}  // namespace OHOS::ObjectStore
#endif