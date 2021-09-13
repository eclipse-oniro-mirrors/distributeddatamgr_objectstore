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

#ifndef OBJECT_TASK_MANAGER_H
#define OBJECT_TASK_MANAGER_H

#include "communicator.h"
#include "local_object_task.h"
#include "observer_manager.h"
#include "remote_object_task.h"

namespace OHOS::ObjectStore {
class ObjectTaskManager final {
public:
    ObjectTaskManager(Communicator &communicator, ObserverManager &observerManager)
        : remoteObjectTask_(communicator, observerManager), localObjectTask_(communicator)
    {}
    ~ObjectTaskManager() = default;
    uint32_t Init();
    void Destory();
    uint32_t Put(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t PutAsync(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t Get(const Bytes &key, std::shared_ptr<Operation> &values);
    uint32_t Delete(const Bytes &key);
    uint32_t DeleteAsync(const Bytes &key);

private:
    RemoteObjectTask remoteObjectTask_;
    LocalObjectTask localObjectTask_;
};
}  // namespace OHOS::ObjectStore
#endif