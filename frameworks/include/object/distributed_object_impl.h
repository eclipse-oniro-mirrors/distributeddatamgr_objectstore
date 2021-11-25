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

#ifndef DISTRIBUTED_OBJECT_IMPL_H
#define DISTRIBUTED_OBJECT_IMPL_H
#include <string>
#include "flat_object.h"
#include "flat_object_store.h"
#include "distributed_object.h"

namespace OHOS::ObjectStore {
class DistributedObjectImpl : public DistributedObject {
public:
    DistributedObjectImpl(FlatObject *flatObject, FlatObjectStore *flatObjectStore);
    ~DistributedObjectImpl();
    uint32_t PutChar(const std::string &key, char value) override;
    uint32_t PutInt(const std::string &key, int32_t value) override;
    uint32_t PutShort(const std::string &key, int16_t value) override;
    uint32_t PutLong(const std::string &key, int64_t value) override;
    uint32_t PutFloat(const std::string &key, float value) override;
    uint32_t PutDouble(const std::string &key, double value) override;
    uint32_t PutBoolean(const std::string &key, bool value) override;
    uint32_t PutString(const std::string &key, const std::string &value) override;
    uint32_t PutByte(const std::string &key, int8_t value) override;
    uint32_t GetChar(const std::string &key, char &value) override;
    uint32_t GetInt(const std::string &key, int32_t &value) override;
    uint32_t GetShort(const std::string &key, int16_t &value) override;
    uint32_t GetLong(const std::string &key, int64_t &value) override;
    uint32_t GetFloat(const std::string &key, float &value) override;
    uint32_t GetDouble(const std::string &key, double &value) override;
    uint32_t GetBoolean(const std::string &key, bool &value) override;
    uint32_t GetString(const std::string &key, std::string &value) override;
    uint32_t GetByte(const std::string &key, int8_t &value) override;
    uint32_t GetObjectId(std::string &objectId) override;
    FlatObject *GetObject();

private:
    uint32_t UpdateObject();
    Bytes StrToFieldBytes(const std::string &src);
    FlatObject *flatObject_;
    FlatObjectStore *flatObjectStore_ = nullptr;
};
}  // namespace OHOS

#endif // DISTRIBUTED_OBJECT_IMPL_H
