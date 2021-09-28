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

#include "distributed_objectstore_impl.h"
#include "distributed_object_impl.h"
#include "objectstore_errors.h"
#include "object_store_manager.h"
#include "string_utils.h"
#include "object_utils.h"

namespace OHOS::ObjectStore {
DistributedObjectStoreImpl::DistributedObjectStoreImpl(FlatObjectStore *flatObjectStore)
    : flatObjectStore_(flatObjectStore)
{}

DistributedObjectStoreImpl::~DistributedObjectStoreImpl()
{
    delete flatObjectStore_;
}

namespace {
    const std::string DEFAULT_USER_ID = "0";
    const std::string SEPARATOR = "/";
    const std::string GenerateObjectId(const std::string &prefix, const std::string &classPath, const std::string &key)
    {
        return prefix + SEPARATOR + classPath + SEPARATOR + key;
    }
}  // namespace

DistributedObjectImpl *DistributedObjectStoreImpl::CacheObject(FlatObject *flatObject)
{
    DistributedObjectImpl *object = new (std::nothrow) DistributedObjectImpl(flatObject);
    if (object == nullptr) {
        return nullptr;
    }
    std::unique_lock<std::shared_mutex> cacheLock(dataMutex_);
    objects_.push_back(object);
    return object;
}

DistributedObject *DistributedObjectStoreImpl::CreateObject(const std::string &classPath, const std::string &key)
{
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("store for not opened!");
        return nullptr;
    }

    FlatObject *flatObject = new (std::nothrow) FlatObject();
    if (flatObject == nullptr) {
        LOG_ERROR("no memory for FlatObjectStore malloc!");
        return nullptr;
    }
    DistributedObject *object = nullptr;
    const std::vector<std::string> remoteIds = flatObjectStore_->GetRemoteStoreIds();
    for (const auto &deviceId : flatObjectStore_->GetRemoteStoreIds()) {
        std::string prefix = ObjectUtils::GenObjectIdPrefix(deviceId, DEFAULT_USER_ID,
            flatObjectStore_->GetBundleName(), flatObjectStore_->GetName());
        std::string oid = GenerateObjectId(prefix, classPath, key);
        uint32_t errCode = flatObjectStore_->Get(StringUtils::StrToBytes(oid), *flatObject);
        LOG_INFO("get %s errCode %d", oid.c_str(), errCode);
        if (errCode == SUCCESS) {
            const std::map<Bytes, Bytes> &cFields = flatObject->GetFields();
            if (cFields.size() == 0) {
                LOG_INFO("field empty %s", oid.c_str());
                continue;
            }
            LOG_INFO("find exist object");
            return CacheObject(flatObject);
        }
    }
    std::string oid = GenerateObjectId(flatObjectStore_->GetPrefix(), classPath, key);
    flatObject->SetId(StringUtils::StrToBytes(oid));
    flatObject->SetField(StringUtils::StrToBytes("stubInfo"), StringUtils::StrToBytes(""));
    flatObjectStore_->Put(*flatObject);
    LOG_INFO("create object for owner");
    return CacheObject(flatObject);
}

uint32_t DistributedObjectStoreImpl::Sync(DistributedObject *object)
{
    if (object == nullptr || flatObjectStore_ == nullptr) {
        return ERR_INVAL;
    }
    // object is abstract, it must be DistributedObjectImpl pointer
    return flatObjectStore_->Put(*dynamic_cast<DistributedObjectImpl *>(object)->GetObject());
}

void DistributedObjectStoreImpl::Close()
{
    {
        std::unique_lock<std::shared_mutex> cacheLock(dataMutex_);
        for (auto &item : objects_) {
            flatObjectStore_->Delete(item->GetObject()->GetId());
            delete item;
        }
    }
    flatObjectStore_->Close();
}
}  // namespace OHOS::ObjectStore
