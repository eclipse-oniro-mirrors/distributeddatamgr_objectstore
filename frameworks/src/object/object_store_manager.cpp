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

#include "object_store_manager.h"
#include "object_utils.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
ObjectStoreManager &ObjectStoreManager::GetInstance()
{
    static ObjectStoreManager objectStoreManager;
    return objectStoreManager;
}

ObjectStoreManager::ObjectStoreManager()
{
    objectStoreExecutor_.Init();
}

ObjectStoreManager::~ObjectStoreManager()
{
    objectStoreExecutor_.Destory();
}

int32_t ObjectStoreManager::SetObjectStore(const std::string &storeName, const std::shared_ptr<StorageEngine> &store)
{
    std::unique_lock<std::shared_mutex> lock(storeMutex_);
    storeMap_[storeName] = store;
    return SUCCESS;
}

std::shared_ptr<StorageEngine> ObjectStoreManager::GetObjectStore(const Bytes &key)
{
    std::string storeName = ObjectUtils::GetObjectStoreName(std::string(key.begin(), key.end()));
    std::shared_lock<std::shared_mutex> lock(storeMutex_);
    if (storeMap_.count(storeName) == 0) {
        return nullptr;
    }
    LOG_INFO("ObjectStoreManager-%s: Find store for %s", __func__, storeName.data());
    return storeMap_[storeName];
}
}  // namespace OHOS::ObjectStore