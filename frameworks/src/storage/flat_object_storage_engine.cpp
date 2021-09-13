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

#include "flat_object_storage_engine.h"
#include "logger.h"

#include "objectstore_errors.h"
#include "securec.h"

namespace OHOS::ObjectStore {
FlatObjectStorageEngine::~FlatObjectStorageEngine()
{
    if (!opened_) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    LOG_INFO("sunpeng: Crash! begine %p", hashDic_); // TODO
    delete hashDic_;
    hashDic_ = nullptr;
    LOG_INFO("sunpeng: Crash! end %p", hashDic_); // TODO
}

uint32_t FlatObjectStorageEngine::Open()
{
    if (opened_) {
        LOG_INFO("FlatObjectDatabase: No need to reopen it");
        return SUCCESS;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    hashDic_ = new (std::nothrow) std::map<Key, std::map<Field, Value>>;
    if (hashDic_== nullptr) {
        LOG_ERROR("FlatObjectDatabase: Failed to open, error: malloc redis db failed");
        return ERR_NOMEM;
    }
    opened_ = true;
    LOG_INFO("FlatObjectDatabase: Succeed to open");
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::Clear()
{
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (!opened_) {
        return SUCCESS;
    }
    if (hashDic_ == nullptr) {
        return SUCCESS;
    }
    hashDic_->clear();
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::Close()
{
    if (!opened_) {
        LOG_INFO("falt object stroage engine has been closed!");
        return SUCCESS;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    delete hashDic_;
    hashDic_ = nullptr;
    opened_ = false;
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::PutHash(const Key &key, const std::map<Field, Value> &fieldValues)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    char* tmp = new char[key.size() + 1];
    memcpy_s(tmp, key.size(), key.data(), key.size());
    tmp[key.size()] = 0;
    LOG_ERROR("hanlu puthash: %s", tmp);
    LOG_INFO("MapStoreExecutor-%s: put %lu", __func__, fieldValues.size());
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    hashDic_->insert_or_assign(key, fieldValues);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::GetHash(const Key &key, std::map<Field, Value> &result)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (hashDic_->count(key) != 0) {
        result = hashDic_->at(key);
    }
    char* tmp = new char[key.size() + 1];
    memcpy_s(tmp, key.size(), key.data(), key.size());
    tmp[key.size()] = 0;
    LOG_ERROR("hanlu gethash: %s", tmp);
    LOG_INFO("MapStoreExecutor-%s: get %lu", __func__, result.size());
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::DeleteKey(const Key &key)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    for(auto it = hashDic_->begin(); it != hashDic_->end(); ) {
        if (it->first == key) {
            it = hashDic_->erase(it);
        } else {
            ++it;
        }
    }
    return SUCCESS;
}
}  // namespace OHOS::ObjectStore

