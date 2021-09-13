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

#ifndef OBJECT_STORE_MANAGER_H
#define OBJECT_STORE_MANAGER_H

#include <shared_mutex>
#include <string>
#include "bytes.h"
#include "macro.h"
#include "object_store_executor.h"
#include "storage_engine.h"

namespace OHOS::ObjectStore {
class ObjectStoreManager final {
public:
    static ObjectStoreManager &GetInstance();
    int32_t SetObjectStore(const std::string &storeName, const std::shared_ptr<StorageEngine> &store);
    std::shared_ptr<StorageEngine> GetObjectStore(const Bytes &key);

private:
    ObjectStoreManager();
    ~ObjectStoreManager();
    DISABLE_COPY_AND_MOVE(ObjectStoreManager);
    std::map<std::string, std::shared_ptr<StorageEngine>> storeMap_ {};
    std::shared_mutex storeMutex_ {};
    ObjectStoreExecutor objectStoreExecutor_ {};
};
}  // namespace OHOS::ObjectStore

#endif