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

#include "operation.h"
#include "operation_dispatcher.h"

namespace OHOS::ObjectStore {
uint32_t MapOperator::Merge(std::any &value1, std::any &value2)
{
    auto &mapValue1 = std::any_cast<std::map<Bytes, Bytes> &>(value1);
    auto &mapValue2 = std::any_cast<std::map<Bytes, Bytes> &>(value2);
    for (auto &[keyParam, valueParam] : mapValue2) {
        mapValue1[keyParam] = valueParam;
    }
    return SUCCESS;
}

uint32_t MapOperator::Encode(uint8_t *data, uint32_t dataSize, uint32_t &cursor, const std::any &value)
{
    auto ret = SUCCESS;
    if (!value.has_value()) {
        return SUCCESS;
    }
    auto &mapValue = std::any_cast<const std::map<Bytes, Bytes> &>(value);
    ret = OperationUtils::WriteValue(data, dataSize, cursor, static_cast<uint32_t>(mapValue.size()));
    if (ret != SUCCESS) {
        return ret;
    }
    for (auto &[keyParam, valueParam] : mapValue) {
        ret = OperationUtils::WriteValue(data, dataSize, cursor, keyParam);
        if (ret != SUCCESS) {
            return ret;
        }
        ret = OperationUtils::WriteValue(data, dataSize, cursor, valueParam);
        if (ret != SUCCESS) {
            return ret;
        }
    }
    return SUCCESS;
}

uint32_t MapOperator::Execute(const Bytes &key, std::any &value)
{
    auto ret = SUCCESS;
    switch (operationType_) {
        case OperationType::PUT_OPERATION:
            ret = OperationDispatcher::GetInstance().Put(DataType::MAP_TYPE, key, value);
            break;
        case OperationType::GET_OPERATION:
            ret = OperationDispatcher::GetInstance().Get(DataType::MAP_TYPE, key, value);
            break;
        case OperationType::PUBLISH_OPERATION:
            ret = OperationDispatcher::GetInstance().Publish(DataType::MAP_TYPE, key, value);
            break;
        case OperationType::PUBLISH_DELETE_OPERATION:
            ret = OperationDispatcher::GetInstance().PublishDelete(DataType::MAP_TYPE, key);
            break;
        case OperationType::DELETE_OPERATION:
            ret = OperationDispatcher::GetInstance().Delete(DataType::MAP_TYPE, key);
            break;
        default:
            LOG_ERROR("MapOperator-%s: not support execute", __func__);
            ret = ERR_INVAL;
            break;
    }
    return ret;
}

uint32_t MapOperator::GetDataSize(const std::any &value)
{
    uint32_t totalSize = 0;
    if (!value.has_value()) {
        return totalSize;
    }
    auto &mapValue = std::any_cast<const std::map<Bytes, Bytes> &>(value);
    totalSize += static_cast<uint32_t>(sizeof(mapValue.size()));
    for (auto &[keyParam, valueParam] : mapValue) {
        totalSize += sizeof(uint32_t) + keyParam.size() * sizeof(uint8_t);
        totalSize += sizeof(uint32_t) + valueParam.size() * sizeof(uint8_t);
    }
    return totalSize;
}

uint32_t MapOperator::Decode(const uint8_t *data, uint32_t dataSize, uint32_t &cursor, std::any &value)
{
    std::map<Bytes, Bytes> valueMap;
    uint32_t mapSize = 0;
    OperationUtils::ReadValue(data, dataSize, cursor, mapSize);
    if (mapSize == 0) {
        return ERR_INVAL;
    }
    for (uint32_t i = 0; i < mapSize; ++i) {
        Bytes key, value;
        OperationUtils::ReadValue(data, dataSize, cursor, key);
        OperationUtils::ReadValue(data, dataSize, cursor, value);
        if (key.empty()) {
            return ERR_INVAL;
        }
        valueMap.emplace(std::move(key), std::move(value));
    }
    value = valueMap;
    return SUCCESS;
}

Operation::Operation(const uint8_t *data, uint32_t dataSize)
{
    if (data == nullptr) {
        return;
    }
    uint32_t cursor = 0;
    uint8_t dataType = 0;
    OperationUtils::ReadValue(data, dataSize, cursor, dataType);
    dataType_ = static_cast<DataType>(dataType);
    uint8_t operationType = 0;
    OperationUtils::ReadValue(data, dataSize, cursor, operationType);
    operationType_ = static_cast<OperationType>(operationType);
    InitOperator();
    OperationUtils::ReadValue(data, dataSize, cursor, key_);
    if (operator_ == nullptr) {
        return;
    }
    operator_->Decode(data, dataSize, cursor, value_);
}

uint32_t Operation::Execute()
{
    if (operator_ != nullptr) {
        return operator_->Execute(key_, value_);
    }
    return ERR_INVAL;
}

uint32_t PublishOperation::Merge(std::shared_ptr<Operation> &target)
{
    if (target == nullptr) {
        return ERR_INVAL;
    }
    Bytes targetKey;
    target->GetOperationKey(targetKey);
    if (key_ != targetKey) {
        LOG_ERROR("PublishOperation-%s: different key", __func__);
        return ERR_INVAL;
    }

    auto type = target->GetDataType();
    if (dataType_ != type) {
        LOG_ERROR("PublishOperation-%s: different type", __func__);
        return ERR_INVAL;
    }

    std::any value;
    target->GetOperationValue(value);
    if (operator_ != nullptr) {
        return operator_->Merge(value_, value);
    }
    return ERR_INVAL;
}
}  // namespace OHOS::ObjectStore