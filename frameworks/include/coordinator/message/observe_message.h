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

#ifndef OBSERVE_MESSAGE_H
#define OBSERVE_MESSAGE_H

#include "message.h"

namespace OHOS::ObjectStore {
constexpr uint32_t OBSERVE_KEY_LEN_LIMIT = 256;
constexpr uint32_t OBSERVE_OPERATION_LEN_LIMIT = 256 * 1024;  // 256k

struct ObserveRequest final : public Message {
    ObserveRequest(message_t id, const std::vector<uint8_t> &key, uint32_t mode)
        : Message(id, MessageType::OBSERVE_REQUEST, MSG_RESPONSE_TIMEOUT), key(key), mode(mode)
    {
        keyLen = key.size();
        header.dataSize = GetDataSize();
    }
    ObserveRequest(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, keyLen);
            if (keyLen > OBSERVE_KEY_LEN_LIMIT) {
                LOG_ERROR("ObserveRequest-%s: object key too long", __func__);
                keyLen = OBSERVE_KEY_LEN_LIMIT;
            }
            key.assign(keyLen, 0);
            for (uint32_t i = 0; i < keyLen; ++i) {
                ReadValue(data, dataSize, cursor, key[i]);
            }
            ReadValue(data, dataSize, cursor, mode);
        }
    }
    virtual ~ObserveRequest() = default;

    const uint8_t *GetData() override
    {
        if (msgBuffer_ != nullptr) {
            return msgBuffer_.get();
        }
        uint32_t dataSize = GetDataSize();
        msgBuffer_ = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer_ == nullptr) {
            LOG_ERROR("ObserveRequest-%s: Failed to create message buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        WriteValue(msgBuffer_.get(), dataSize, cursor, header);
        WriteValue(msgBuffer_.get(), dataSize, cursor, keyLen);
        for (uint32_t i = 0; i < keyLen; ++i) {
            WriteValue(msgBuffer_.get(), dataSize, cursor, key[i]);
        }
        WriteValue(msgBuffer_.get(), dataSize, cursor, mode);
        return msgBuffer_.get();
    }

    uint32_t GetDataSize() const override
    {
        return sizeof(header) + sizeof(keyLen) + sizeof(uint8_t) * keyLen + sizeof(mode);
    }
    std::vector<uint8_t> key {};
    uint32_t keyLen { 0 };
    uint32_t mode { 0 };
};

struct ObserveResponse final : public Message {
    ObserveResponse(message_t id, message_t requestId, uint32_t response, std::vector<uint8_t> &&operation)
        : Message(id, requestId, MessageType::OBSERVE_RESPONSE), response(response), operation(std::move(operation))
    {
        operationLen = operation.size();
        header.dataSize = GetDataSize();
    }

    ObserveResponse(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, response);
            ReadValue(data, dataSize, cursor, operationLen);
            if (operationLen > OBSERVE_OPERATION_LEN_LIMIT) {
                LOG_ERROR("ObserveResponse-%s: object operation too long", __func__);
                operationLen = OBSERVE_OPERATION_LEN_LIMIT;
            }
            operation.assign(operationLen, 0);
            for (uint32_t i = 0; i < operationLen; ++i) {
                ReadValue(data, dataSize, cursor, operation[i]);
            }
        }
    }
    virtual ~ObserveResponse() = default;

    const uint8_t *GetData() override
    {
        if (msgBuffer_ != nullptr) {
            return msgBuffer_.get();
        }
        uint32_t dataSize = GetDataSize();
        msgBuffer_ = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer_ == nullptr) {
            LOG_ERROR("ObserveResponse-%s: Failed to create message buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        WriteValue(msgBuffer_.get(), dataSize, cursor, header);
        WriteValue(msgBuffer_.get(), dataSize, cursor, response);
        WriteValue(msgBuffer_.get(), dataSize, cursor, operationLen);
        for (uint32_t i = 0; i < operationLen; ++i) {
            WriteValue(msgBuffer_.get(), dataSize, cursor, operation[i]);
        }
        return msgBuffer_.get();
    }

    uint32_t GetDataSize() const override
    {
        return sizeof(header) + sizeof(response) + sizeof(operationLen) + sizeof(uint8_t) * operationLen;
    }
    uint32_t response { 0 };
    std::vector<uint8_t> operation {};
    uint32_t operationLen { 0 };
};

struct RemoveObserverRequest final : public Message {
    RemoveObserverRequest(message_t id, const std::vector<uint8_t> &key)
        : Message(id, MessageType::REMOVE_OBSERVER_REQUEST), key(key)
    {
        keyLen = key.size();
        header.dataSize = GetDataSize();
    }
    RemoveObserverRequest(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, keyLen);
            if (keyLen > OBSERVE_KEY_LEN_LIMIT) {
                LOG_ERROR("RemoveObserverRequest-%s: object key too long", __func__);
                keyLen = OBSERVE_KEY_LEN_LIMIT;
            }
            key.reserve(keyLen);
            for (uint32_t i = 0; i < keyLen; ++i) {
                ReadValue(data, dataSize, cursor, key[i]);
            }
        }
    }
    virtual ~RemoveObserverRequest() = default;

    const uint8_t *GetData() override
    {
        if (msgBuffer_ != nullptr) {
            return msgBuffer_.get();
        }
        uint32_t dataSize = GetDataSize();
        msgBuffer_ = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer_ == nullptr) {
            LOG_ERROR("RemoveObserverRequest-%s: Failed to create message buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        WriteValue(msgBuffer_.get(), dataSize, cursor, header);
        WriteValue(msgBuffer_.get(), dataSize, cursor, keyLen);
        for (uint32_t i = 0; i < keyLen; ++i) {
            WriteValue(msgBuffer_.get(), dataSize, cursor, key[i]);
        }
        return msgBuffer_.get();
    }

    uint32_t GetDataSize() const override
    {
        return sizeof(header) + sizeof(keyLen) + sizeof(uint8_t) * keyLen;
    }
    std::vector<uint8_t> key {};
    uint32_t keyLen { 0 };
};
}  // namespace OHOS::ObjectStore
#endif