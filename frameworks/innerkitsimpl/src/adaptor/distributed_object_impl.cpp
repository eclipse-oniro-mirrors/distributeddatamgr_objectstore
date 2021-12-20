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

#include "objectstore_errors.h"
#include "string_utils.h"
#include "distributed_object_impl.h"

namespace OHOS::ObjectStore {
DistributedObjectImpl::~DistributedObjectImpl()
{
   // delete flatObject_;
}


uint32_t DistributedObjectImpl::PutChar(const std::string &key, char value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutInt(const std::string &key, int32_t value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutShort(const std::string &key, int16_t value)
{
    return SUCCESS;
}


uint32_t DistributedObjectImpl::PutLong(const std::string &key, int64_t value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutFloat(const std::string &key, float value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutDouble(const std::string &key, double value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutBoolean(const std::string &key, bool value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutString(const std::string &key, const std::string &value)
{
  //  flatObject_->SetField(StrToFieldBytes(key), StrToFieldBytes(value));
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutByte(const std::string &key, int8_t value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetChar(const std::string &key, char &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetInt(const std::string &key, int32_t &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetShort(const std::string &key, int16_t &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetLong(const std::string &key, int64_t &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetFloat(const std::string &key, float &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetDouble(const std::string &key, double &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetBoolean(const std::string &key, bool &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetByte(const std::string &key, int8_t &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetString(const std::string &key, std::string &value)
{
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetObjectId(std::string &objectId)
{
    return SUCCESS;
}

DistributedObjectImpl::DistributedObjectImpl() {}

uint32_t DistributedObjectImpl::GetType(const std::string &key, Type &type) {
    return 0;
}
}