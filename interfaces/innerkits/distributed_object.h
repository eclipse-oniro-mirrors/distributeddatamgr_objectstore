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

#ifndef DISTRIBUTED_OBJECT_H
#define DISTRIBUTED_OBJECT_H
#include <string>
#include <vector>
#include <memory>

namespace OHOS::ObjectStore {
class DistributedObject {
public:
    virtual ~DistributedObject() {};
    virtual uint32_t PutChar(const std::string &key, char value) = 0;
    virtual uint32_t PutInt(const std::string &key, int32_t value) = 0;
    virtual uint32_t PutShort(const std::string &key, int16_t value) = 0;
    virtual uint32_t PutLong(const std::string &key, int64_t value) = 0;
    virtual uint32_t PutFloat(const std::string &key, float value) = 0;
    virtual uint32_t PutDouble(const std::string &key, double value) = 0;
    virtual uint32_t PutBoolean(const std::string &key, bool value) = 0;
    virtual uint32_t PutString(const std::string &key, const std::string &value) = 0;
    virtual uint32_t PutByte(const std::string &key, int8_t value) = 0;
    virtual uint32_t GetChar(const std::string &key, char &value) = 0;
    virtual uint32_t GetInt(const std::string &key, int32_t &value) = 0;
    virtual uint32_t GetShort(const std::string &key, int16_t &value) = 0;
    virtual uint32_t GetLong(const std::string &key, int64_t &value) = 0;
    virtual uint32_t GetFloat(const std::string &key, float &value) = 0;
    virtual uint32_t GetDouble(const std::string &key, double &value) = 0;
    virtual uint32_t GetBoolean(const std::string &key, bool &value) = 0;
    virtual uint32_t GetString(const std::string &key, std::string &value) = 0;
    virtual uint32_t GetByte(const std::string &key, int8_t &value) = 0;
    virtual uint32_t GetObjectId(std::string &objectId) = 0;
};

class ObjectWatcher {
public:
    virtual void OnChanged(const std::string &id) = 0;
    virtual void OnDeleted(const std::string &id) = 0;
};
}  // namespace OHOS
#endif // DISTRIBUTED_OBJECT_H
