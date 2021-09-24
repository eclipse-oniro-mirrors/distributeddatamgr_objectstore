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

#include "object_store_executor.h"
#include "logger.h"
#include "object_store_manager.h"
#include "objectstore_errors.h"
#include "operation_dispatcher.h"

namespace OHOS::ObjectStore {
uint32_t MapStoreExecutor::Publish(const Bytes &key, const std::any &value)
{
    auto store = ObjectStoreManager::GetInstance().GetObjectStore(key);
    if (store == nullptr) {
        LOG_ERROR("MapStoreExecutor-%s: Fail to get store", __func__);
        return ERR_INVAL;
    }
    if (!value.has_value()) {
        LOG_ERROR("MapStoreExecutor-%s: value is empty", __func__);
        return SUCCESS;
    }
    const std::map<Field, Value> &mapValue = std::any_cast<const std::map<Field, Value> &>(value);
    return store->PutHash(key, mapValue);
}

uint32_t MapStoreExecutor::PublishDelete(const Bytes &key)
{
    auto store = ObjectStoreManager::GetInstance().GetObjectStore(key);
    if (store == nullptr) {
        LOG_ERROR("MapStoreExecutor-%s: Fail to get store", __func__);
        return ERR_INVAL;
    }
    return store->DeleteKey(key);
}

uint32_t MapStoreExecutor::Put(const Bytes &key, const std::any &value)
{
    auto store = ObjectStoreManager::GetInstance().GetObjectStore(key);
    if (store == nullptr) {
        LOG_ERROR("MapStoreExecutor-%s: Fail to get store", __func__);
        return ERR_INVAL;
    }
    if (!value.has_value()) {
        LOG_ERROR("MapStoreExecutor-%s: value is empty", __func__);
        return SUCCESS;
    }
    const std::map<Field, Value> &mapValue = std::any_cast<const std::map<Field, Value> &>(value);
    return store->PutHash(key, mapValue);
}

uint32_t MapStoreExecutor::Get(const Bytes &key, std::any &value)
{
    auto store = ObjectStoreManager::GetInstance().GetObjectStore(key);
    if (store == nullptr) {
        LOG_ERROR("MapStoreExecutor-%s: Fail to get store", __func__);
        return ERR_INVAL;
    }
    std::map<Field, Value> mapValue;
    auto result = store->GetHash(key, mapValue);
    if (result != SUCCESS) {
        LOG_INFO("MapStoreExecutor-%s: fail to get value", __func__);
        return ERR_HASHDB_GET;
    }
    value = mapValue;
    return result;
}

uint32_t MapStoreExecutor::Delete(const Bytes &key)
{
    auto store = ObjectStoreManager::GetInstance().GetObjectStore(key);
    if (store == nullptr) {
        LOG_ERROR("MapStoreExecutor-%s: Fail to get store", __func__);
        return ERR_INVAL;
    }
    return store->DeleteKey(key);
}
}  // namespace OHOS::ObjectStore