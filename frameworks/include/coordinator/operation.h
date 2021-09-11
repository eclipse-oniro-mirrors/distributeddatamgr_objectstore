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

#ifndef OPERATION_H
#define OPERATION_H

#include <any>
#include <map>
#include <memory>
#include <vector>
#include "bytes.h"
#include "logger.h"
#include "objectstore_errors.h"
#include "operation_utils.h"

namespace OHOS::ObjectStore {
enum class DataType : uint8_t {
    INVALID_TYPE = 0,
    MAP_TYPE = 1,
};

enum class OperationType : uint8_t {
    INVALID_OPERATION = 0,
    PUBLISH_OPERATION = 1,
    PUT_OPERATION = 2,
    GET_OPERATION = 3,
    DELETE_OPERATION = 4,
    PUBLISH_DELETE_OPERATION = 5,
};

class Operator {
public:
    Operator(OperationType operationType) : operationType_(operationType)
    {}
    virtual ~Operator() = default;
    virtual uint32_t Merge(std::any &value1, std::any &value2) = 0;
    virtual uint32_t Encode(uint8_t *data, uint32_t dataSize, uint32_t &cursor, const std::any &value) = 0;
    virtual uint32_t Execute(const Bytes &key, std::any &value) = 0;
    virtual uint32_t GetDataSize(const std::any &value) = 0;
    virtual uint32_t Decode(const uint8_t *data, uint32_t dataSize, uint32_t &cursor, std::any &value) = 0;

protected:
    OperationType operationType_ { OperationType::INVALID_OPERATION };
};

class MapOperator : public Operator {
public:
    MapOperator(OperationType operationType) : Operator(operationType)
    {}
    ~MapOperator() = default;

private:
    uint32_t Merge(std::any &value1, std::any &value2) override;
    uint32_t Encode(uint8_t *data, uint32_t dataSize, uint32_t &cursor, const std::any &value) override;
    uint32_t Execute(const Bytes &key, std::any &value) override;
    uint32_t GetDataSize(const std::any &value) override;
    uint32_t Decode(const uint8_t *data, uint32_t dataSize, uint32_t &cursor, std::any &value) override;
};

class Operation {
public:
    Operation(DataType dataType, OperationType operationType, const Bytes &key, std::any &value)
        : key_(key), dataType_(dataType), operationType_(operationType), value_(value)
    {
        InitOperator();
    }
    Operation(DataType dataType, OperationType operationType, const Bytes &key)
        : key_(key), dataType_(dataType), operationType_(operationType)
    {
        InitOperator();
    }
    Operation(const uint8_t *data, uint32_t dataSize);
    virtual ~Operation() = default;

    virtual uint32_t Merge(std::shared_ptr<Operation> &target)
    {
        LOG_ERROR("Operation-%s: do not support merge", __func__);
        return ERR_INVAL;
    }
    virtual uint8_t *Encode()
    {
        if (operator_ == nullptr) {
            return nullptr;
        }
        auto dataSize = GetDataSize();
        dataBuffer_ = std::make_unique<uint8_t[]>(dataSize);
        if (dataBuffer_ == nullptr) {
            LOG_ERROR("Operation-%s: fail cerate data buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        OperationUtils::WriteValue(dataBuffer_.get(), dataSize, cursor, static_cast<uint8_t>(dataType_));
        OperationUtils::WriteValue(dataBuffer_.get(), dataSize, cursor, static_cast<uint8_t>(operationType_));
        OperationUtils::WriteValue(dataBuffer_.get(), dataSize, cursor, key_);
        operator_->Encode(dataBuffer_.get(), dataSize, cursor, value_);
        return dataBuffer_.get();
    }

    virtual uint32_t GetDataSize() const
    {
        if (operator_ == nullptr) {
            return 0;
        }
        return sizeof(dataType_) + sizeof(operationType_) + sizeof(uint32_t) + key_.size() * sizeof(uint8_t) +
               operator_->GetDataSize(value_);
    }

    uint32_t Execute();

    DataType GetDataType()
    {
        return dataType_;
    }

    OperationType GetOperationType()
    {
        return operationType_;
    }

    void GetOperationKey(Bytes &key)
    {
        return key.assign(key_.begin(), key_.end());
    }

    void GetOperationValue(std::any &value)
    {
        value = value_;
    }

protected:
    Bytes key_;
    DataType dataType_ { DataType::INVALID_TYPE };
    OperationType operationType_ { OperationType::INVALID_OPERATION };
    std::unique_ptr<Operator> operator_ { nullptr };
    std::any value_;
    std::unique_ptr<uint8_t[]> dataBuffer_ { nullptr };

private:
    void InitOperator()
    {
        switch (dataType_) {
            case DataType::MAP_TYPE:
                operator_ = std::make_unique<MapOperator>(GetOperationType());
                break;
            default:
                LOG_ERROR("Operation-%s: unsupported operation type %d", __func__, static_cast<uint8_t>(dataType_));
                break;
        }
    }
};

class PublishOperation : public Operation {
public:
    PublishOperation(DataType dataType, const Bytes &key, std::any &value)
        : Operation(dataType, OperationType::PUBLISH_OPERATION, key, value)
    {}
    PublishOperation(const uint8_t *data, uint32_t dataSize) : Operation(data, dataSize)
    {}
    ~PublishOperation() = default;
    uint32_t Merge(std::shared_ptr<Operation> &target) override;
};

class PublishDeleteOperation : public Operation {
public:
    PublishDeleteOperation(DataType dataType, const Bytes &key)
        : Operation(dataType, OperationType::PUBLISH_DELETE_OPERATION, key)
    {}
    PublishDeleteOperation(const uint8_t *data, uint32_t dataSize) : Operation(data, dataSize)
    {}
    ~PublishDeleteOperation() = default;
};

class PutOperation : public Operation {
public:
    PutOperation(DataType dataType, const Bytes &key, std::any &value)
        : Operation(dataType, OperationType::PUT_OPERATION, key, value)
    {}
    PutOperation(const uint8_t *data, uint32_t dataSize) : Operation(data, dataSize)
    {}
    ~PutOperation() = default;
};

class GetOperation : public Operation {
public:
    GetOperation(DataType dataType, const Bytes &key) : Operation(dataType, OperationType::GET_OPERATION, key)
    {}
    GetOperation(const uint8_t *data, uint32_t dataSize) : Operation(data, dataSize)
    {}
    ~GetOperation() = default;
};

class DeleteOperation : public Operation {
public:
    DeleteOperation(DataType dataType, const Bytes &key) : Operation(dataType, OperationType::DELETE_OPERATION, key)
    {}
    DeleteOperation(const uint8_t *data, uint32_t dataSize) : Operation(data, dataSize)
    {}
    ~DeleteOperation() = default;
};
}  // namespace OHOS::ObjectStore
#endif