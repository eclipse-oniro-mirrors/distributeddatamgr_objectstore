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

#include "observer_manager.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
uint32_t ObserverManager::Init()
{
    auto ret = localObservers_.Init();
    if (ret != SUCCESS) {
        LOG_WARN("ObserverManager-%s: local observer init fail", __func__);
    }
    ret = remoteObservers_.Init();
    if (ret != SUCCESS) {
        LOG_WARN("ObserverManager-%s: remote observer init fail", __func__);
    }
    return ret;
}

void ObserverManager::Destory()
{
    localObservers_.Destory();
    remoteObservers_.Destory();
}

uint32_t ObserverManager::AddObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer, ObserverMode mode)
{
    return localObservers_.AddObserver(key, observer, mode);
}

uint32_t ObserverManager::RemoveObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer)
{
    return localObservers_.RemoveObserver(key, observer);
}

uint32_t ObserverManager::Publish(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    localObservers_.Publish(key, changes);
    remoteObservers_.Publish(key, changes);
    return SUCCESS;
}

uint32_t ObserverManager::PublishDelete(const Bytes &key)
{
    localObservers_.PublishDelete(key);
    remoteObservers_.PublishDelete(key);
    return SUCCESS;
}
}  // namespace OHOS::ObjectStore