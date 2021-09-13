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

#ifndef COORDINATE_ENGINE_H
#define COORDINATE_ENGINE_H

#include "bytes.h"
#include "communicator.h"
#include "object_observer.h"
#include "object_task_manager.h"
#include "observer_manager.h"
#include "operation.h"
#include "macro.h"

namespace OHOS::ObjectStore {
class CoordinateEngine final {
public:
    static CoordinateEngine &GetInstance();
    uint32_t Init(const CommunicatorConfig &config);
    void Destory();
    uint32_t GetLocalDeviceId(std::string &local);
    uint32_t AddObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer,
                         ObserverMode mode = ObserverMode::OBSERVER_DEFAULT_MODE);
    uint32_t RemoveObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer);
    uint32_t Delete(const Bytes &key);
    uint32_t DeleteAsync(const Bytes &key);
    uint32_t Publish(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t PublishDelete(const Bytes &key);
    uint32_t Put(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t PutAsync(const Bytes &key, std::shared_ptr<Operation> &changes);
    uint32_t Get(const Bytes &key, std::shared_ptr<Operation> &values);
    uint32_t GetRemoteStoreIds(std::vector<std::string> &remoteId);

private:
    CoordinateEngine();
    ~CoordinateEngine();
    DISABLE_COPY_AND_MOVE(CoordinateEngine);
    Communicator communicator_;
    ObserverManager observerManager_;
    ObjectTaskManager objectTaskManager_;
    bool initFlag_ { false };
};
}  // namespace OHOS::ObjectStore
#endif