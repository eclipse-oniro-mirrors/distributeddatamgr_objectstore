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

#ifndef OBJECT_COORDINATOR_H
#define OBJECT_COORDINATOR_H

#include <map>
#include "bytes.h"
#include "macro.h"
#include "object_observer.h"

namespace OHOS::ObjectStore {
class ObjectCoordinator final {
public:
    static ObjectCoordinator &GetInstance();
    uint32_t Publish(const Bytes &key, const std::map<Bytes, Bytes> &changes);
    uint32_t PublishDelete(const Bytes &key);
    uint32_t Put(const Bytes &key, const std::map<Bytes, Bytes> &changes);
    uint32_t PutAsync(const Bytes &key, const std::map<Bytes, Bytes> &changes);
    uint32_t Get(const Bytes &key, std::map<Bytes, Bytes> &changes);
    uint32_t Delete(const Bytes &key);
    uint32_t DeleteAsync(const Bytes &key);
    uint32_t AddObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer);
    uint32_t RemoveObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer);

private:
    ObjectCoordinator() = default;
    ~ObjectCoordinator() = default;
    DISABLE_COPY_AND_MOVE(ObjectCoordinator);
};
}  // namespace OHOS::ObjectStore
#endif