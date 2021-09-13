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

#include "coordinate_engine.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
CoordinateEngine &CoordinateEngine::GetInstance()
{
    static CoordinateEngine coordinateEngine;
    return coordinateEngine;
}

CoordinateEngine::CoordinateEngine()
    : communicator_(), observerManager_(communicator_), objectTaskManager_(communicator_, observerManager_)
{}

CoordinateEngine::~CoordinateEngine()
{
    Destory();
}

uint32_t CoordinateEngine::Init(const CommunicatorConfig &config)
{
    if (initFlag_) {
        return SUCCESS;
    }
    auto ret = communicator_.Init(config);
    if (ret != SUCCESS) {
        LOG_ERROR("CoordinateEngine-%s: fail to init communicator while successful for local, %u", __func__, ret);
    }
    ret = observerManager_.Init();
    if (ret != SUCCESS) {
        LOG_ERROR("CoordinateEngine-%s: fail to init observer manager, %u", __func__, ret);
        return ret;
    }
    ret = objectTaskManager_.Init();
    if (ret != SUCCESS) {
        LOG_ERROR("CoordinateEngine-%s: fail to init object task manager, %u", __func__, ret);
        return ret;
    }
    initFlag_ = true;
    return ret;
}

void CoordinateEngine::Destory()
{
    observerManager_.Destory();
    objectTaskManager_.Destory();
    communicator_.Destory();
    initFlag_ = false;
}

uint32_t CoordinateEngine::GetLocalDeviceId(std::string &local)
{
    device_t deviceId;
    uint32_t ret = communicator_.GetLocalDeviceId(deviceId);
    if (ret == SUCCESS) {
        local = deviceId;
    }
    return ret;
}

uint32_t CoordinateEngine::AddObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer, ObserverMode mode)
{
    return observerManager_.AddObserver(key, observer, mode);
}

uint32_t CoordinateEngine::RemoveObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer)
{
    return observerManager_.RemoveObserver(key, observer);
}

uint32_t CoordinateEngine::Delete(const Bytes &key)
{
    return objectTaskManager_.Delete(key);
}

uint32_t CoordinateEngine::DeleteAsync(const Bytes &key)
{
    return objectTaskManager_.DeleteAsync(key);
}

uint32_t CoordinateEngine::Publish(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    return observerManager_.Publish(key, changes);
}

uint32_t CoordinateEngine::PublishDelete(const Bytes &key)
{
    return observerManager_.PublishDelete(key);
}

uint32_t CoordinateEngine::Put(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    return objectTaskManager_.Put(key, changes);
}

uint32_t CoordinateEngine::PutAsync(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    return objectTaskManager_.PutAsync(key, changes);
}

uint32_t CoordinateEngine::Get(const Bytes &key, std::shared_ptr<Operation> &values)
{
    return objectTaskManager_.Get(key, values);
}

uint32_t CoordinateEngine::GetRemoteStoreIds(std::vector<std::string> &remoteId)
{
    return communicator_.GetRemoteStoreIds(remoteId);
}
}  // namespace OHOS::ObjectStore