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


#ifndef OBJECT_STORE_H
#define OBJECT_STORE_H

#include <cstdint>
#include <memory>
#include "data_object.h"
#include "watcher.h"

namespace OHOS::ObjectStore {
class ObjectWatcher : public Watcher<ObjectId> {
};

class ObjectStore {
public:
    virtual ~ObjectStore() = default;
    virtual uint32_t Open() = 0;
    virtual uint32_t Close() = 0;
    virtual uint32_t Put(const DataObject &object) = 0;
    virtual uint32_t Get(const ObjectId &objectId, DataObject &object) = 0;
    virtual uint32_t Delete(const ObjectId &objectId) = 0;
    virtual uint32_t Watch(const ObjectId &objectId, std::shared_ptr<ObjectWatcher> watcher) = 0;
    virtual uint32_t Unwatch(const ObjectId &objectId, std::shared_ptr<ObjectWatcher> watcher) = 0;
};
}  // namespace OHOS::ObjectStore

#endif  // OBJECT_STORE_H
