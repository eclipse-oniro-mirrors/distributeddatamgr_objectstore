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

#include "object_coordinator.h"
#include "coordinate_engine.h"
#include "logger.h"
#include "operation.h"

namespace OHOS::ObjectStore {
ObjectCoordinator &ObjectCoordinator::GetInstance()
{
    static ObjectCoordinator objectCoordinator;
    return objectCoordinator;
}

uint32_t ObjectCoordinator::Publish(const Bytes &key, const std::map<Bytes, Bytes> &changes)
{
    std::any value = changes;
    std::shared_ptr<Operation> operation = std::make_shared<PublishOperation>(DataType::MAP_TYPE, key, value);
    return CoordinateEngine::GetInstance().Publish(key, operation);
}


uint32_t ObjectCoordinator::PublishDelete(const Bytes &key)
{
    return CoordinateEngine::GetInstance().PublishDelete(key);
}

uint32_t ObjectCoordinator::Put(const Bytes &key, const std::map<Bytes, Bytes> &changes)
{
    std::any value = changes;
    std::shared_ptr<Operation> operation = std::make_shared<PutOperation>(DataType::MAP_TYPE, key, value);
    return CoordinateEngine::GetInstance().Put(key, operation);
}

uint32_t ObjectCoordinator::PutAsync(const Bytes &key, const std::map<Bytes, Bytes> &changes)
{
    std::any value = changes;
    std::shared_ptr<Operation> operation = std::make_shared<PutOperation>(DataType::MAP_TYPE, key, value);
    return CoordinateEngine::GetInstance().PutAsync(key, operation);
}

uint32_t ObjectCoordinator::Get(const Bytes &key, std::map<Bytes, Bytes> &changes)
{
    std::shared_ptr<Operation> operation = std::make_shared<GetOperation>(DataType::MAP_TYPE, key);
    auto ret = CoordinateEngine::GetInstance().Get(key, operation);
    if (ret != SUCCESS) {
        LOG_ERROR("ObjectCoordinator-%s: Fail to get object value", __func__);
        return ret;
    }
    std::any value;
    operation->GetOperationValue(value);
    if (!value.has_value()) {
        LOG_ERROR("ObjectCoordinator-%s: empty value", __func__);
        return ERR_INVAL;
    }
    changes = std::any_cast<std::map<Bytes, Bytes>>(value);
    return ret;
}

uint32_t ObjectCoordinator::Delete(const Bytes &key)
{
    return CoordinateEngine::GetInstance().Delete(key);
}

uint32_t ObjectCoordinator::DeleteAsync(const Bytes &key)
{
    return CoordinateEngine::GetInstance().DeleteAsync(key);
}

uint32_t ObjectCoordinator::AddObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer)
{
    return CoordinateEngine::GetInstance().AddObserver(key, observer);
}

uint32_t ObjectCoordinator::RemoveObserver(const Bytes &key, std::shared_ptr<ObjectObserver> observer)
{
    return CoordinateEngine::GetInstance().RemoveObserver(key, observer);
}
};  // namespace OHOS::ObjectStore