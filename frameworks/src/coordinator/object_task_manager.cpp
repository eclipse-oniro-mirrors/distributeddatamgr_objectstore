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

#include "object_task_manager.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
uint32_t ObjectTaskManager::Init()
{
    auto ret = remoteObjectTask_.Init();
    if (ret != SUCCESS) {
        LOG_ERROR("ObjectTaskManager-%s: remote object task init fail", __func__);
        return ERR_INIT;
    }
    ret = localObjectTask_.Init();
    if (ret != SUCCESS) {
        LOG_ERROR("ObjectTaskManager-%s: local object task init fail", __func__);
        return ERR_INIT;
    }
    return ret;
}

void ObjectTaskManager::Destory()
{
    remoteObjectTask_.Destory();
    localObjectTask_.Destory();
}

uint32_t ObjectTaskManager::Put(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    return remoteObjectTask_.PutRemote(key, changes);
}

uint32_t ObjectTaskManager::PutAsync(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    return remoteObjectTask_.PutRemoteAsync(key, changes);
}

uint32_t ObjectTaskManager::Get(const Bytes &key, std::shared_ptr<Operation> &values)
{
    return remoteObjectTask_.GetRemote(key, values);
}

uint32_t ObjectTaskManager::Delete(const Bytes &key)
{
    return remoteObjectTask_.DeleteRemote(key);
}

uint32_t ObjectTaskManager::DeleteAsync(const Bytes &key)
{
    return remoteObjectTask_.DeleteRemoteAsync(key);
}
}  // namespace OHOS::ObjectStore