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

#ifndef HANDSHAKE_MESSAGE_H
#define HANDSHAKE_MESSAGE_H

#include "message.h"

namespace OHOS::ObjectStore {
struct HandShakeRequest final : public Message {
    HandShakeRequest(message_t id, const device_t &deviceId, const std::string &networkId)
        : Message(id, MessageType::HANDSHAKE_REQUEST), deviceId(deviceId), networkId(networkId)
    {
        header.dataSize = GetDataSize();
    }
    HandShakeRequest(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, deviceId);
            ReadValue(data, dataSize, cursor, networkId);
        }
    }
    virtual ~HandShakeRequest() = default;
    const uint8_t *GetData() override
    {
        uint32_t dataSize = GetDataSize();
        msgBuffer = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer == nullptr) {
            LOG_ERROR("HandShakeRequest-%s: Failed to create message buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        WriteValue(msgBuffer.get(), dataSize, cursor, header);
        WriteValue(msgBuffer.get(), dataSize, cursor, deviceId);
        WriteValue(msgBuffer.get(), dataSize, cursor, networkId);
        return msgBuffer.get();
    }
    uint32_t GetDataSize() const override
    {
        return sizeof(header) + sizeof(deviceId) + sizeof(uint32_t) + networkId.size();
    }
    device_t deviceId;
    std::string networkId {};
};

struct HandShakeResponse final : public Message {
    HandShakeResponse(message_t id, message_t requestId, const device_t &deviceId, const std::string &networkId)
        : Message(id, requestId, MessageType::HANDSHAKE_RESPONSE), deviceId(deviceId), networkId(networkId)
    {
        header.dataSize = GetDataSize();
    }
    HandShakeResponse(const uint8_t *data, uint32_t dataSize)
    {
        if (data != nullptr) {
            uint32_t cursor = 0;
            ReadValue(data, dataSize, cursor, header);
            ReadValue(data, dataSize, cursor, deviceId);
            ReadValue(data, dataSize, cursor, networkId);
        }
    }
    virtual ~HandShakeResponse() = default;

    const uint8_t *GetData() override
    {
        uint32_t dataSize = GetDataSize();
        msgBuffer = std::make_unique<uint8_t[]>(GetDataSize());
        if (msgBuffer == nullptr) {
            LOG_ERROR("HandShakeResponse-%s: Failed to malloc message buffer", __func__);
            return nullptr;
        }
        uint32_t cursor = 0;
        WriteValue(msgBuffer.get(), dataSize, cursor, header);
        WriteValue(msgBuffer.get(), dataSize, cursor, deviceId);
        WriteValue(msgBuffer.get(), dataSize, cursor, networkId);
        return msgBuffer.get();
    }

    uint32_t GetDataSize() const override
    {
        return sizeof(header) + sizeof(deviceId) + sizeof(uint32_t) + networkId.size();
    }
    device_t deviceId;
    std::string networkId {};
};
}  // namespace OHOS::ObjectStore
#endif