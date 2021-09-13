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

#ifndef PUT_MESSAGE_H
#define PUT_MESSAGE_H

#include "message.h"
namespace OHOS::ObjectStore {
constexpr uint32_t PUT_OPERATION_LEN_LIMIT = 256 * 1024;  // 256k

struct PutRequest final : public Message {
    PutRequest(message_t id, std::vector<uint8_t> &&operationParam)
        : Message(id, MessageType::PUT_REQUEST, MSG_RESPONSE_TIMEOUT), operation(std::move(operationParam))
    {
        operationLen = operation.size();
        header.dataSize = GetDataSize();
    }
    PutRequest(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, operationLen);
            if (operationLen > PUT_OPERATION_LEN_LIMIT) {
                LOG_ERROR("PutRequest-%s: object operation too long", __func__);
                operationLen = PUT_OPERATION_LEN_LIMIT;
            }
            operation.assign(operationLen, 0);
            for (uint32_t i = 0; i < operationLen; ++i) {
                ReadValue(data, dataSize, cursor, operation[i]);
            }
        }
    }
    virtual ~PutRequest() = default;

    const uint8_t *GetData() override
    {
        if (msgBuffer_ != nullptr) {
            return msgBuffer_.get();
        }
        uint32_t dataSize = GetDataSize();
        msgBuffer_ = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer_ == nullptr) {
            LOG_ERROR("PutRequest-%s: Failed to create message buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        WriteValue(msgBuffer_.get(), dataSize, cursor, header);
        WriteValue(msgBuffer_.get(), dataSize, cursor, operationLen);
        for (uint32_t i = 0; i < operationLen; ++i) {
            WriteValue(msgBuffer_.get(), dataSize, cursor, operation[i]);
        }
        return msgBuffer_.get();
    }

    uint32_t GetDataSize() const override
    {
        return sizeof(header) + sizeof(operationLen) + sizeof(uint8_t) * operationLen;
    }
    std::vector<uint8_t> operation {};
    uint32_t operationLen { 0 };
};

struct PutResponse final : public Message {
    PutResponse(message_t id, message_t requestId, uint32_t status)
        : Message(id, requestId, MessageType::PUT_RESPONSE), status(status)
    {
        header.dataSize = GetDataSize();
    }
    PutResponse(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, status);
        }
    }
    virtual ~PutResponse() = default;

    const uint8_t *GetData() override
    {
        if (msgBuffer_ != nullptr) {
            return msgBuffer_.get();
        }
        uint32_t dataSize = GetDataSize();
        msgBuffer_ = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer_ == nullptr) {
            LOG_ERROR("PutResponse-%s: Failed to create message buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        WriteValue(msgBuffer_.get(), dataSize, cursor, header);
        WriteValue(msgBuffer_.get(), dataSize, cursor, status);
        return msgBuffer_.get();
    }

    uint32_t GetDataSize() const override
    {
        return sizeof(header) + sizeof(status);
    }
    uint32_t status;
};
}  // namespace OHOS::ObjectStore
#endif