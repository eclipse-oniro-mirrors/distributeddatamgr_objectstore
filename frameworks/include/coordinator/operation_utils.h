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
 
#ifndef OPERATION_UTILS_H
#define OPERATION_UTILS_H

#include <vector>
#include "logger.h"
#include "objectstore_errors.h"
#include "securec.h"

namespace OHOS::ObjectStore {
class OperationUtils final {
public:
    template<typename T>
    static uint32_t WriteValue(uint8_t *data, uint32_t dataSize, uint32_t &cursor, const T &value)
    {
        if ((dataSize <= cursor) || (data == nullptr)) {
            LOG_ERROR("OperationUtils-%s: Failed to write value, error: invalid cursor", __func__);
            return ERR_INVAL;
        }
        uint32_t remainSize = dataSize - cursor;
        if (remainSize >= sizeof(T)) {
            *reinterpret_cast<T *>(data + cursor) = value;
            cursor += sizeof(T);
        } else {
            LOG_ERROR("OperationUtils-%s: Failed to write value, error: memory is not enough", __func__);
            return ERR_INVAL;
        }
        return SUCCESS;
    }

    static uint32_t WriteValue(uint8_t *data, uint32_t dataSize, uint32_t &cursor, const std::vector<uint8_t> &value)
    {
        if ((dataSize <= cursor) || (data == nullptr) || (value.empty())) {
            LOG_ERROR("OperationUtils-%s: Failed to write value, error: invalid cursor", __func__);
            return ERR_INVAL;
        }
        uint32_t len = value.size();
        uint32_t remainSize = dataSize - cursor;
        uint32_t writeSize = sizeof(len) + sizeof(value[0]) * len;
        if (writeSize > remainSize) {
            LOG_ERROR("OperationUtils-%s: Failed to write value, error: invalid value", __func__);
            return ERR_INVAL;
        }
        WriteValue(data, dataSize, cursor, len);
        remainSize = dataSize - cursor;
        if (memcpy_s(data + cursor, remainSize, value.data(), len) != EOK) {
            LOG_ERROR("OperationUtils-%s: Fail to write value, error: memcpy_s fail", __func__);
        }
        cursor += value.size();  // still update cursor even if memcpy_s fail
        return SUCCESS;
    }

    template<typename T>
    static uint32_t ReadValue(const uint8_t *data, uint32_t dataSize, uint32_t &cursor, T &value)
    {
        if ((dataSize <= cursor) || (data == nullptr)) {
            LOG_ERROR("OperationUtils-%s: Failed to read value, error: invalid cursor, %u", __func__, cursor);
            return ERR_INVAL;
        }
        uint32_t remainSize = dataSize - cursor;
        if (remainSize >= sizeof(T)) {
            value = *reinterpret_cast<const T *>(data + cursor);
            cursor += sizeof(T);
        } else {
            LOG_ERROR("OperationUtils-%s: Failed to read value, error: memory is not enough", __func__);
            return ERR_INVAL;
        }
        return SUCCESS;
    }

    static uint32_t ReadValue(const uint8_t *data, uint32_t dataSize, uint32_t &cursor, std::vector<uint8_t> &value)
    {
        if ((dataSize <= cursor) || (data == nullptr)) {
            LOG_ERROR("OperationUtils-%s: Failed to read value, error: invalid cursor", __func__);
            return ERR_INVAL;
        }
        uint32_t remainSize = dataSize - cursor;
        if (remainSize >= sizeof(uint32_t)) {
            uint32_t size = *reinterpret_cast<const uint32_t *>(data + cursor);
            cursor += sizeof(uint32_t);
            if ((size > 0) && (remainSize >= size + sizeof(uint32_t))) {
                auto start = reinterpret_cast<const uint8_t *>(data + cursor);
                value = std::vector<uint8_t>(start, start + size);
                cursor += size;
            }
        } else {
            LOG_ERROR("OperationUtils-%s: Failed to read value, error: memory is not enough", __func__);
            return ERR_INVAL;
        }
        return SUCCESS;
    }

private:
    OperationUtils() = default;
    ~OperationUtils() = default;
};
}  // namespace OHOS::ObjectStore
#endif