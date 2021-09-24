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

#ifndef MESSAGE_H
#define MESSAGE_H

#include <algorithm>
#include <cstdint>
#include <vector>
#include "logger.h"
#include "network_types.h"
#include "securec.h"

namespace OHOS::ObjectStore {
using message_t = uint16_t;

enum class MessageType : uint8_t {
    INVALID = 0,
    OBSERVE_REQUEST = 1,
    OBSERVE_RESPONSE = 2,
    REMOVE_OBSERVER_REQUEST = 3,  // no need reponse
    PUT_REQUEST = 4,
    PUT_RESPONSE = 5,
    GET_REQUEST = 6,
    GET_RESPONSE = 7,
    PUBLISH_REQUEST = 8,  // no need response

    DELETE_REQUEST = 9,
    DELETE_RESPONSE = 10,
    PUBLISH_DELETE_REQUEST = 11,  // no need response
};

constexpr uint8_t MESSAGE_MAGIC = 0xF6;  // magic number
constexpr uint8_t MAX_DEVICE_LEN = 65;  // magic number

enum class MessageFlag : uint8_t {
    FLAG_UNENCRYPT = 0x0,
    FLAG_ENCRYPT = 0x1,
};
#pragma pack (1)
struct MessageHeader {
    MessageHeader(message_t id, MessageType type) : type(type), id(id)
    {}

    MessageHeader(message_t id, message_t requestId, MessageType type) : type(type), id(id), requestId(requestId)
    {}
    MessageHeader() = default;
    ~MessageHeader() = default;
    bool IsValid() const
    {
        return (magic == MESSAGE_MAGIC);
    }

    bool HasFlag(MessageFlag flag) const
    {
        return flags & static_cast<uint8_t>(flag);
    }
    uint8_t magic { MESSAGE_MAGIC };
    uint8_t version { 1 };  // version
    MessageType type { MessageType::INVALID };
    uint8_t flags { static_cast<uint8_t>(MessageFlag::FLAG_UNENCRYPT) };
    message_t id { 0 };
    message_t requestId { 0 };
    uint32_t dataSize { 0 };
    char deviceId[MAX_DEVICE_LEN] {0};
};
#pragma pack ()
struct Message {
    static constexpr uint32_t MSG_RESPONSE_TIMEOUT = 5000;  // 5000: 5s time out
    Message(message_t id, MessageType type, uint32_t timeout = 0) : header(id, type), timeout_(timeout)
    {}

    Message(message_t id, message_t requestId, MessageType type) : header(id, requestId, type)
    {}

    Message() = default;
    virtual ~Message() = default;

    message_t GetId() const
    {
        return header.id;
    }

    MessageType GetType() const
    {
        return header.type;
    }

    void GetSource(std::string &devId) const
    {
        devId = header.deviceId;
    }

    void SetSource(const device_t &deviceId)
    {
        errno_t ret = strcpy_s(header.deviceId, MAX_DEVICE_LEN, deviceId.c_str());
        if (ret != 0) {
            LOG_ERROR("Message-%s: strcpy_s, error: memory is not enough", __func__);
        }
    }

    uint32_t GetTimeout() const
    {
        return timeout_;
    }

    message_t GetRequestId() const
    {
        return header.requestId;
    }

    void SetFlag(MessageFlag flag)
    {
        header.flags |= static_cast<uint8_t>(flag);
    }

    void ResetFlag(MessageFlag flag)
    {
        header.flags &= ~static_cast<uint8_t>(flag);
    }

    bool HasFlag(MessageFlag flag) const
    {
        return header.HasFlag(flag);
    }

    void SwapBuffer(std::unique_ptr<uint8_t[]> &buffer)
    {
        std::swap(msgBuffer_, buffer);
    }

    virtual const uint8_t *GetData() = 0;
    virtual uint32_t GetDataSize() const = 0;

protected:

    template<typename T>
    void ReadValue(const uint8_t *data, uint32_t dataSize, uint32_t &cursor, T &value) const
    {
        if (dataSize <= cursor) {
            LOG_ERROR("Message-%s: Failed to read value, error: invalid cursor", __func__);
            return;
        }
        uint32_t remainSize = dataSize - cursor;
        if (remainSize >= sizeof(T)) {
            value = *reinterpret_cast<const T *>(data + cursor);
            cursor += sizeof(T);
        } else {
            LOG_ERROR("Message-%s: Failed to read value, error: memory is not enough", __func__);
        }
    }

    void ReadValue(const uint8_t *data, uint32_t dataSize, uint32_t &cursor, std::string &value) const
    {
        if (dataSize <= cursor) {
            LOG_ERROR("Message-%s: Failed to read value, error: invalid cursor", __func__);
            return;
        }
        uint32_t remainSize = dataSize - cursor;
        if (remainSize >= sizeof(uint32_t)) {
            uint32_t size = *reinterpret_cast<const uint32_t *>(data + cursor);
            cursor += sizeof(uint32_t);
            if ((size > 0) && (remainSize >= size + sizeof(uint32_t))) {
                value = std::string(reinterpret_cast<const char *>(data + cursor), size);
                cursor += size;
            }
        }
    }

    template<typename T>
    void WriteValue(uint8_t *data, uint32_t dataSize, uint32_t &cursor, const T &value) const
    {
        if (dataSize <= cursor) {
            LOG_ERROR("Message-%s: Failed to write value, error: invalid cursor", __func__);
            return;
        }
        uint32_t remainSize = dataSize - cursor;
        if (remainSize >= sizeof(T)) {
            *reinterpret_cast<T *>(data + cursor) = value;
            cursor += sizeof(T);
        } else {
            LOG_ERROR("Message-%s: Failed to write value, error: memory is not enough", __func__);
        }
    }

    void WriteValue(uint8_t *data, uint32_t dataSize, uint32_t &cursor, const std::string &value) const
    {
        if (dataSize <= cursor) {
            LOG_ERROR("Message-%s: Failed to write value, error: invalid cursor", __func__);
            return;
        }
        uint32_t remainSize = dataSize - cursor;
        if (remainSize >= sizeof(uint32_t) + value.size()) {
            *reinterpret_cast<uint32_t *>(data + cursor) = static_cast<uint32_t>(value.size());
            cursor += sizeof(uint32_t);
            if (memcpy_s(data + cursor, remainSize - sizeof(uint32_t), value.data(), value.size()) != EOK) {
                LOG_ERROR("Message-%s: Failed to write string, error: memcpy_s is failed", __func__);
            }
            cursor += value.size();
        }
    }

    template<typename T>
    void ReadBuffer(const uint8_t *data, uint32_t dataSize, uint32_t &cursor, uint32_t size,
                    std::unique_ptr<T[]> &bufferSptr) const
    {
        if (dataSize <= cursor) {
            LOG_ERROR("Message-%s: Failed to read buffer, error: invalid cursor", __func__);
            return;
        }
        uint32_t remainSize = dataSize - cursor;
        if (size > std::min(static_cast<uint32_t>(MAX_SIZE_ONCE), remainSize)) {
            LOG_ERROR("Message-%s: Failed to read buffer, error: buffer size %u error", __func__, size);
            return;
        }

        if (size > 0) {
            if (bufferSptr == nullptr) {
                bufferSptr = std::make_unique<T[]>(size);
                if (bufferSptr == nullptr) {
                    LOG_ERROR("Message-%s: Failed to read buffer, error: no memory %u", __func__, size);
                    return;
                }
            }
            if (memcpy_s(reinterpret_cast<void *>(bufferSptr.get()), size,
                reinterpret_cast<const void *>(data + cursor), size) != EOK) {
                LOG_ERROR("Message-%s: Failed to read buffer, error: memcpy_s is failed", __func__);
            }
            cursor += size;
        }
    }

    template<typename T>
    void WriteBuffer(uint8_t *data, uint32_t dataSize, uint32_t &cursor, uint32_t size, const T *buffer) const
    {
        if (dataSize <= cursor) {
            LOG_ERROR("Message-%s: Failed to write buffer, error: invalid cursor", __func__);
            return;
        }
        uint32_t remainSize = dataSize - cursor;
        if (size > 0 && buffer != nullptr) {
            if (memcpy_s(data + cursor, remainSize, buffer, size) != EOK) {
                LOG_ERROR("Message-%s: Failed to write buffer, error: memcpy_s is failed", __func__);
            }
            cursor += size;
        }
    }

protected:
    MessageHeader header;
    uint32_t timeout_ { 0 };
    std::unique_ptr<uint8_t[]> msgBuffer_ { nullptr };
};
}  // namespace OHOS::ObjectStore
#endif