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
    storeManager_ = nullptr;
    LOG_INFO("FlatObjectStorageEngine::~FlatObjectStorageEngine Crash! end");
}

uint32_t FlatObjectStorageEngine::Open()
{
    if (opened_) {
        LOG_INFO("FlatObjectDatabase: No need to reopen it");
        return SUCCESS;
    }
    storeManager_ = std::make_shared<KvStoreDelegateManager>("objectstore",
        "user0"); //todo 建议按照设备id配置，保证各设备的不一致(APP_ID,USER_ID)
    if (storeManager_ == nullptr) {
        LOG_ERROR("FlatObjectStorageEngine::make shared fail");
        return ERR_MOMEM;
    }
    bool autoSync = true;
    PragmaData data = static_cast<ProgmaData>(&autoSync);
    DBStatus status = storeManager_->Pragma(AUTO_SYNC, data);
    if (status != DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::Open Pragma fail[%d]", status);
    }
    opened_ = true;
    LOG_INFO("FlatObjectDatabase::Open Succeed");
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::Close()
{
    if (!opened_) {
        LOG_INFO("FlatObjectStorageEngine::Close has been closed!");
        return SUCCESS;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    storeManager_ = nullptr;
    opened_ = false;
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::CreateTable(const std::string &key)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates->count(key) != 0) {
        LOG_INFO("FlatObjectStorageEngine::CreateTable %s already created", key.c_str());
        return SUCCESS;
    }
    KvStoreNbDelegate *kvStore = nullptr;
    DBStatus status;
    KvStoreNbDelegate::Option option = { true, true, false }; // createIfNecessary, isMemoryDb, isEncryptedDb
    storeManager_->GetKvStore(key, option, [&status, &kvStore](DBStatus dbStatus, KvStoreNbDelegate *kvStoreNbDelegate) {
        status = dbStatus;
        kvStore = kvStoreNbDelegate;
    });

    if (status != DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::CreateTable %s getkvstore fail[%d]", key.c_str(), status);
        return ERR_DE_GETKV_FAIL;
    }
    delegates.insert_or_assign(key, kvStore);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::GetTable(const std::string &key, std::map<Field, Value> &result)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates->count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    result.clear();
    KvStoreResultSet *resultSet = nullptr;
    Key emptyKey;
    DBStatus status = delegates.at(key)->GetEntries(emptyKey, resultSet);
    if (status != DBStatus::OK) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %s GetEntries fail", key.c_str());
        return ERR_DB_GET_FAIL;
    }
    while (resultSet->IsAfterLast()) {
        Entry entry;
        status = resultSet->GetEntry(entry);
        if (status != DBStatus::OK) {
            LOG_INFO("FlatObjectStorageEngine::GetTable  GetEntry fail");
            return ERR_DB_ENTRY_FAIL;
        }
        result.insert_or_assign(entry.key, entry.value);
        resultSet->MoveToNext();
    }
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::UpdateItems(const std::string &key, std::map<Field, Value> &data)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates->count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    auto iter = data.begin();
    std::vector<Entry> items;
    auto delegate = delegates.at(key);
    while (iter != data.end()) {
        Entry entry;
        entry.key = iter->first;
        entry.value = iter->second;
        items.insert(items.end(), entry);
        iter++;
    }
    delegate->PutBatch(items);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::DeleteTable(const std::string &key)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates->count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::GetTable %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    auto status = storeManager_->CloseKvStore(delegates.at(key));
    if (status != DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::CloseKvStore %s CloseKvStore fail[%d]", key.c_str(), status);
        return ERR_CLOSE_STORAGE;
    }
    delegates.erase(key);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::GetItem(const std::string &key, const Field &itemKey, const Field &value)
{
    if (!opened_) {
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates->count(key) == 0) {
        LOG_ERROR("FlatObjectStorageEngine::GetItem %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    DBStatus status = delegates.at(key)->Get(itemKey, value);
    if (status != DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::GetItem %s item not exist", itemKey.c_str());
        return status;
    }
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::RegisterObserver(const std::string &key, std::shared_ptr<TableWatcher> watcher)
{
    if (!opened_) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates->count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    if (observerMap_.count(key) != 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver observer already exist.");
        return SUCCESS;
    }
    auto delegate = delegates.at(key);
    DBStatus status = delegate.RegisterObserver(key, ObserverMode::OBSERVER_CHANGES_NATIVE, watcher);
    if (status != DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver watch err %d", status);
        return ERR_REGISTER;
    }
    observerMap_.insert_or_assign(key, watcher);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::UnRegisterObserver(const std::string &key)
{
    if (!opened_) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates->count(key) == 0) {
        LOG_INFO("FlatObjectStorageEngine::RegisterObserver %s not exist", key.c_str());
        return ERR_DE_NOT_EXIST;
    }
    auto watcher = observerMap_.find(key);
    if (watcher == observerMap_.end()) {
        LOG_ERROR("FlatObjectStorageEngine::UnRegisterObserver observer not exist.");
        return ERR_NO_OBSERVER;
    }
    auto delegate = delegates.at(key);
    DBStatus status = delegate.UnRegisterObserver(watcher);
    if (status != DBStatus::OK) {
        LOG_ERROR("FlatObjectStorageEngine::UnRegisterObserver unRegister err %d", status);
        return ERR_UNRIGSTER;
    }
    observerMap_.erase(key);
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::ChangeKey(const std::string &oldKey, const std::string &newKey)
{
    if (!opened_) {
        LOG_ERROR("FlatObjectStorageEngine::RegisterObserver kvStore has not init");
        return ERR_DB_NOT_INIT;
    }
    std::unique_lock<std::shared_mutex> lock(operationMutex_);
    if (delegates->count(oldKey) == 0) {
        LOG_INFO("FlatObjectStorageEngine::ChangeKey oldKey %s not exist", oldKey.c_str());
        return ERR_DE_NOT_EXIST;
    }
    auto delegate = delegates.find(oldKey);
    delegates.erase(oldKey);
    delegates.insert_or_assign(newKey, delegate);
    auto watcher = observerMap_.find(oldKey);
    if (watcher != observerMap_.end()) {
        UnRegisterObserver(oldKey);
        DBStatus status = delegate.RegisterObserver(newKey, ObserverMode::OBSERVER_CHANGES_NATIVE, watcher);
        if (status != DBStatus::OK) {
            LOG_ERROR("FlatObjectStorageEngine::ChangeKey watch err %d", status);
            return ERR_REGISTER;
        }
    }
    return SUCCESS;
}

uint32_t FlatObjectStorageEngine::ChangeSession(const std::string &objectId, const std::string &sessionId)
{
    //todo
}

} // namespace OHOS::ObjectStore
