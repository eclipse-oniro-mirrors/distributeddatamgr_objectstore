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

#include "distributed_object_impl.h"
#include "distributed_objectstore_impl.h"
#include "objectstore_errors.h"
#include "string_utils.h"

namespace OHOS::ObjectStore {
DistributedObjectStoreImpl::DistributedObjectStoreImpl(FlatObjectStore *flatObjectStore)
    : flatObjectStore_(flatObjectStore)
{
}

DistributedObjectStoreImpl::~DistributedObjectStoreImpl()
{
    delete flatObject_;
    delete flatObjectStore_;
}
DistributedObjectImpl *DistributedObjectStoreImpl::CacheObject(FlatObject *flatObject, FlatObjectStore *flatObjectStore)
{
    DistributedObjectImpl *object = new (std::nothrow) DistributedObjectImpl(flatObject, flatObjectStore);
    if (object == nullptr) {
        return nullptr;
    }
    std::unique_lock<std::shared_mutex> cacheLock(dataMutex_);
    objects_.push_back(object);
    return object;
}
DistributedObject *DistributedObjectStoreImpl::CreateObject(const std::string &sessionId)
{
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::CreateObject store not opened!");
        return nullptr;
    }
    int32_t status = flatObjectStore_->CreateObject(sessionId);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::CreateObject CreateTable err %d", status);
        return nullptr;
    }
    flatObject_ = new (std::nothrow) FlatObject();
    if (flatObject_ == nullptr) {
        LOG_ERROR("no memory for FlatObjectStore malloc!");
        return nullptr;
    }
    status = flatObjectStore_->Get(StringUtils::StrToBytes(sessionId), reinterpret_cast<FlatObject &>(flatObject_));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::CreateObject get object failed %d", status);
    }
    flatObjectStore_->Put(*flatObject_);
    LOG_INFO("create object for owner");
    return CacheObject(flatObject_, flatObjectStore_);
}

uint32_t DistributedObjectStoreImpl::Sync(DistributedObject *object)
{
    if (object == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECT;
    }
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECTSTORE;
    }
    return flatObjectStore_->Put(*static_cast<DistributedObjectImpl *>(object)->GetObject());
}

uint32_t DistributedObjectStoreImpl::DeleteObject(const std::string &sessionId)
{
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECTSTORE;
    }
    uint32_t status = flatObjectStore_->Delete(StringUtils::StrToBytes(sessionId));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::DeleteObject store delete err %d", status);
        return status;
    }
    return SUCCESS;
}

uint32_t DistributedObjectStoreImpl::Get(const std::string &objectId, DistributedObject *object)
{
    std::string oid;
    auto iter = objects_.begin();
    while (iter != objects_.end()) {
        uint32_t ret = (*iter)->GetObjectId(oid);
        if (ret != SUCCESS) {
            LOG_ERROR("DistributedObjectStoreImpl::Get oid err, ret %d", ret);
            return ret;
        }
        if (objectId == oid) {
            object = *iter;
            return SUCCESS;
        }
        iter++;
    }
    LOG_ERROR("DistributedObjectStoreImpl::Get object err, no object");
    return ERR_GET_OBJECT;
}

uint32_t DistributedObjectStoreImpl::Watch(DistributedObject *object, std::shared_ptr<ObjectWatcher> watcher)
{
    if (object == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECT;
    }
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECTSTORE;
    }
    if (watchers_.count(object) != 0) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch already gets object");
        return ERR_EXIST;
    }
    std::string objectId;
    uint32_t status = object->GetObjectId(objectId);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch get objectId failed %d", status);
        return status;
    }
    std::shared_ptr<WatcherProxy> watcherProxy = std::make_shared<WatcherProxy>(watcher);
    status = flatObjectStore_->Watch(StringUtils::StrToBytes(objectId), watcherProxy);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch failed %d", status);
        return status;
    }
    watchers_.insert_or_assign(object, watcherProxy);
    LOG_INFO("DistributedObjectStoreImpl:Watch object success.");
    return SUCCESS;
}

uint32_t DistributedObjectStoreImpl::UnWatch(DistributedObject *object)
{
    if (object == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECT;
    }
    if (flatObjectStore_ == nullptr) {
        LOG_ERROR("DistributedObjectStoreImpl::Sync object err ");
        return ERR_NULL_OBJECTSTORE;
    }
    std::string objectId;
    uint32_t status = object->GetObjectId(objectId);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch get objectId failed %d", status);
        return status;
    }
    status = flatObjectStore_->UnWatch(StringUtils::StrToBytes(objectId));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectStoreImpl::Watch failed %d", status);
        return status;
    }
    watchers_.erase(object);
    LOG_INFO("DistributedObjectStoreImpl:UnWatch object success.");
    return SUCCESS;
}

WatcherProxy::WatcherProxy(const std::shared_ptr<ObjectWatcher> objectWatcher) : objectWatcher_(objectWatcher)
{
}

void WatcherProxy::OnChanged(const std::string &sessionid, const std::vector<std::string> &changedData)
{
    objectWatcher_->OnChanged(sessionid, changedData);
}

void WatcherProxy::OnDeleted(const std::string &sessionid)
{
    objectWatcher_->OnDeleted(sessionid);
}

DistributedObjectStore *DistributedObjectStore::GetInstance()
{
    static char instMemory[sizeof(DistributedObjectStoreImpl)];
    static std::mutex instLock_;
    static std::atomic<DistributedObjectStore *> instPtr = nullptr;
    if (instPtr == nullptr) {
        std::lock_guard<std::mutex> lock(instLock_);
        if (instPtr == nullptr) {
            FlatObjectStore *flatObjectStore = new (std::nothrow) FlatObjectStore();
            if (flatObjectStore == nullptr) {
                LOG_ERROR("no memory for FlatObjectStore malloc!");
                return nullptr;
            }

            uint32_t errCode = flatObjectStore->Open();
            if (errCode != SUCCESS) {
                LOG_ERROR("open failed");
                delete flatObjectStore;
                return nullptr;
            }
            // Use instMemory to make sure this singleton not free before other object.
            // This operation needn't to malloc memory, we needn't to check nullptr.
            instPtr = new (instMemory) DistributedObjectStoreImpl(flatObjectStore);
        }
    }
    return instPtr;
}

} // namespace OHOS::ObjectStore
