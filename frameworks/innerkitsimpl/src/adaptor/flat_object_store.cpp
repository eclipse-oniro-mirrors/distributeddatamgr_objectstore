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

#include "flat_object_store.h"

#include "logger.h"
#include "object_utils.h"
#include "objectstore_errors.h"
#include "string_utils.h"

namespace OHOS::ObjectStore {
namespace {
const std::map<Bytes, Bytes> EMPTY_FIELDS = {};
const std::string DEFAULT_USER_ID = "0";
} // namespace

FlatObjectStore::FlatObjectStore(const std::string &bundleName, const std::string &storeName)
    : bundleName_(bundleName), storeName_(storeName)
{
    storageEngine_ = std::make_shared<FlatObjectStorageEngine>();
}

FlatObjectStore::~FlatObjectStore()
{
}

uint32_t FlatObjectStore::Open()
{
    uint32_t status = storageEngine_->Open();
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to open, error: open storage engine failure %d", status);
        return status;
    }
    return SUCCESS;
}

uint32_t FlatObjectStore::Close()
{
    storageEngine_->Close();
    return SUCCESS;
}

uint32_t FlatObjectStore::CreateObject(const std::string &sessionId)
{
    if (!storageEngine_->opened_) {
        LOG_ERROR("FlatObjectStore::CreateObject DB has not init");
        return ERR_DB_NOT_INIT;
    }
    uint32_t status = storageEngine_->CreateTable(sessionId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::CreateObject createTable err %d", status);
        return status;
    }
    return SUCCESS;
}

uint32_t FlatObjectStore::Put(const FlatObject &flatObject)
{
    if (!storageEngine_->opened_) {
        LOG_ERROR("FlatObjectStore::Put DB has not init");
        return ERR_DB_NOT_INIT;
    }
    std::string oId;
    uint32_t status = StringUtils::BytesToString(flatObject.GetId(), oId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Put get oid err %d", status);
        return status;
    }
    status = storageEngine_->UpdateItems(oId, const_cast<std::map<Bytes, Bytes> &>(flatObject.GetFields()));
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Put updateItems err %d", status);
    }
    return SUCCESS;
}

uint32_t FlatObjectStore::Get(const Bytes &objectId, FlatObject &flatObject) const
{
    if (!storageEngine_->opened_) {
        LOG_ERROR("FlatObjectStore::Get DB has not init");
        return ERR_DB_NOT_INIT;
    }
    std::map<Bytes, Bytes> &fields = const_cast<std::map<Bytes, Bytes> &>(flatObject.GetFields());
    std::string oId;
    uint32_t status = StringUtils::BytesToString(flatObject.GetId(), oId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Get get oid err %d", status);
        return status;
    }
    status = storageEngine_->GetTable(oId, fields);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Get getTable failed %d", status);
        return status;
    }
    flatObject.SetFields(fields);
    flatObject.SetId(objectId);
    return SUCCESS;
}

uint32_t FlatObjectStore::Delete(const Bytes &objectId)
{
    if (!storageEngine_->opened_) {
        LOG_ERROR("FlatObjectStore::Delete DB has not init");
        return ERR_DB_NOT_INIT;
    }
    std::string oId;
    uint32_t status = StringUtils::BytesToString(objectId, oId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Get get oid err %d", status);
        return status;
    }
    status = storageEngine_->DeleteTable(oId);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore: Failed to delete object %d", status);
        return status;
    }
    return SUCCESS;
}

uint32_t FlatObjectStore::Watch(const Bytes &objectId, std::shared_ptr<FlatObjectWatcher> watcher)
{
    std::string oid;
    uint32_t status = StringUtils::BytesToString(objectId, oid);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Watch get oid failed %d", status);
        return status;
    }
    status = storageEngine_->RegisterObserver(oid, watcher);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Watch failed %d", status);
    }
    return status;
}

uint32_t FlatObjectStore::UnWatch(const Bytes &objectId)
{
    std::string oid;
    uint32_t status = StringUtils::BytesToString(objectId, oid);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Watch get oid failed %d", status);
        return status;
    }
    status = storageEngine_->UnRegisterObserver(oid);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Watch failed %d", status);
    }
    return status;
}

} // namespace OHOS::ObjectStore
