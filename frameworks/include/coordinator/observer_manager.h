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

#ifndef OBSERVER_MANAGER_H
#define OBSERVER_MANAGER_H

#include "communicator.h"
#include "local_observers.h"
#include "remote_observers.h"

namespace OHOS::ObjectStore {
class ObserverManager {
public:
    ObserverManager(Communicator &communicator) : localObservers_(communicator), remoteObservers_(communicator)
    {}
    ~ObserverManager() = default;
    uint32_t Init();
    void Destory();
    uint32_t AddObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer, ObserverMode mode);
    uint32_t RemoveObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer);
    uint32_t Publish(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t PublishDelete(const Bytes &key);

private:
    LocalObservers localObservers_;
    RemoteObservers remoteObservers_;
};
}  // namespace OHOS::ObjectStore
#endif