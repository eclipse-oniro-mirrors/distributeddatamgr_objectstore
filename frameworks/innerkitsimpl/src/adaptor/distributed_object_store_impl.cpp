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
// #include "distributed_object_impl.h"
#include "objectstore_errors.h"
#include "string_utils.h"
#include "object_utils.h"
#include "securec.h"

namespace OHOS::ObjectStore {
/* DistributedObjectStoreImpl::DistributedObjectStoreImpl(FlatObjectStore *flatObjectStore)
    : flatObjectStore_(flatObjectStore)
{} */

DistributedObjectStoreImpl::~DistributedObjectStoreImpl()
{
   // delete flatObjectStore_;
}


DistributedObject *DistributedObjectStoreImpl::CreateObject(const std::string &sessionId)
{
    LOG_ERROR("DistributedObjectStoreImpl::CreateObject %s", sessionId.c_str());
    return nullptr;
}

uint32_t DistributedObjectStoreImpl::Sync(DistributedObject *object)
{
    // todo
    return SUCCESS;
}

uint32_t DistributedObjectStoreImpl::DeleteObject(const std::string &sessionId)
{
    // todo
    return SUCCESS;
}

uint32_t DistributedObjectStoreImpl::Get(const std::string &objectId, DistributedObject *object)
{
    // todo
    return SUCCESS;
}

uint32_t DistributedObjectStoreImpl::Watch(DistributedObject *object, std::shared_ptr<ObjectWatcher> watcher)
{
    // todo
    return SUCCESS;
}

uint32_t DistributedObjectStoreImpl::UnWatch(DistributedObject *object)
{
    // todo
    return SUCCESS;
}

WatcherProxy::WatcherProxy(const std::shared_ptr<ObjectWatcher> objectWatcher)
    : objectWatcher_(objectWatcher)
{}

void WatcherProxy::OnChanged(const Bytes &id)
{
    std::string str;
    uint32_t ret = StringUtils::BytesToString(id, str);
    if (ret != SUCCESS) {
        LOG_ERROR("WatcherProxy:OnChanged bytesToString err,ret %d", ret);
    }
    // todo objectWatcher_->OnChanged(str);
}

void WatcherProxy::OnDeleted(const Bytes &id)
{
    std::string str;
    uint32_t ret = StringUtils::BytesToString(id, str);
    if (ret != SUCCESS) {
        LOG_ERROR("WatcherProxy:OnChanged bytesToString err,ret %d", ret);
    }
    objectWatcher_->OnDeleted(str);
}

DistributedObjectStore *DistributedObjectStore::GetInstance()
{
    static char instMemory[sizeof(DistributedObjectStoreImpl)];
    static std::mutex instLock_;
    static std::atomic<DistributedObjectStore *> instPtr = nullptr;
    if (instPtr == nullptr) {
        std::lock_guard<std::mutex> lock(instLock_);
        if (instPtr == nullptr) {
            // Use instMemory to make sure this singleton not free before other object.
            // This operation needn't to malloc memory, we needn't to check nullptr.
            instPtr = new (instMemory) DistributedObjectStoreImpl;
        }
    }
    return instPtr;
}
}  // namespace OHOS::ObjectStore
