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
#ifndef DELETE_MESSAGE_H
#define DELETE_MESSAGE_H

#include "message.h"

namespace OHOS::ObjectStore {
constexpr uint32_t DELETE_KEY_LEN_LIMIT = 256;
struct DeleteRequest final : public Message {
    DeleteRequest(message_t id, const std::vector<uint8_t> &key)
        : Message(id, MessageType::DELETE_REQUEST, MSG_RESPONSE_TIMEOUT), key(key)
    {
        keyLen = key.size();
        header.dataSize = GetDataSize();
    }
    DeleteRequest(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, keyLen);
            if (keyLen > DELETE_KEY_LEN_LIMIT) {
                LOG_ERROR("DeleteRequest-%s: object key too long", __func__);
                keyLen = DELETE_KEY_LEN_LIMIT;
            }
            key.assign(keyLen, 0);
            for (uint32_t i = 0; i < keyLen; ++i) {
                ReadValue(data, dataSize, cursor, key[i]);
            }
        }
    }
    virtual ~DeleteRequest() = default;

    const uint8_t *GetData() override
    {
        if (msgBuffer_ != nullptr) {
            return msgBuffer_.get();
        }
        uint32_t dataSize = GetDataSize();
        msgBuffer_ = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer_ == nullptr) {
            LOG_ERROR("DeleteRequest-%s: Failed to create message buffer", __func__);
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

struct DeleteResponse final : public Message {
    DeleteResponse(message_t id, message_t requestId, uint32_t response)
        : Message(id, requestId, MessageType::DELETE_RESPONSE), response(response)
    {
        header.dataSize = GetDataSize();
    }

    DeleteResponse(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, response);
        }
    }
    virtual ~DeleteResponse() = default;

    const uint8_t *GetData() override
    {
        if (msgBuffer_ != nullptr) {
            return msgBuffer_.get();
        }
        uint32_t dataSize = GetDataSize();
        msgBuffer_ = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer_ == nullptr) {
            LOG_ERROR("DeleteResponse-%s: Failed to create message buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        WriteValue(msgBuffer_.get(), dataSize, cursor, header);
        WriteValue(msgBuffer_.get(), dataSize, cursor, response);
        return msgBuffer_.get();
    }

    uint32_t GetDataSize() const override
    {
        return sizeof(header) + sizeof(response);
    }
    uint32_t response { 0 };
};
}  // namespace OHOS::ObjectStore
#endif