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

#include "distributed_object_impl.h"

#include "objectstore_errors.h"
#include "string_utils.h"

namespace OHOS::ObjectStore {
DistributedObjectImpl::~DistributedObjectImpl()
{
    delete flatObject_;
}
namespace {

void PutNum(void *val, int32_t len, Bytes &data)
{
    if (len > sizeof(data.front()) * data.size()) {
        data.resize(len);
    }
    for (int i = 0; i < len; i++) {
        // 8 bit = 1 byte
        data[i] = *(static_cast<uint64_t *>(val)) >> ((len - i - 1) * 8);
    }
}

uint32_t GetNum(Bytes &data, void *val, int32_t valLen)
{
    uint8_t *value = (uint8_t *)val;
    if (data.size() != valLen) {
        LOG_ERROR("DistributedObjectImpl:GetNum data.size() %d, valLen %d", data.size(), valLen);
        return ERR_DATA_LEN;
    }
    for (int i = 0; i < valLen; i++) {
        value[i] = data[valLen - 1 - i];
    }
    return SUCCESS;
}
} // namespace

uint32_t DistributedObjectImpl::PutDouble(const std::string &key, double value)
{
    Bytes data;
    PutNum(&value, sizeof(value), data);
    keyType_.insert_or_assign(key, TYPE_DOUBLE);
    uint32_t status = flatObject_->SetItem(StringUtils::StrToBytes("p_" + key), data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::PutDouble setField err %d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::PutBoolean(const std::string &key, bool value)
{
    Bytes data;
    int32_t val = value ? 1 : 0;
    PutNum(&val, sizeof(value), data);
    keyType_.insert_or_assign(key, TYPE_BOOLEAN);
    uint32_t status = flatObject_->SetItem(StringUtils::StrToBytes("p_" + key), data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::PutBoolean setField err %d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::PutString(const std::string &key, const std::string &value)
{
    keyType_.insert_or_assign(key, TYPE_STRING);
    uint32_t status = flatObject_->SetItem(StringUtils::StrToBytes("p_" + key), StringUtils::StrToBytes(value));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::PutBoolean setField err %d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::GetDouble(const std::string &key, double &value)
{
    uint32_t status = UpdateObject();
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetDouble update object failed. %d", status);
        return status;
    }
    Bytes data;
    Bytes keyBytes = StringUtils::StrToBytes("p_" + key);
    status = flatObject_->GetItem(keyBytes, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetDouble field not exist. %d %s", status, key.c_str());
        return status;
    }
    status = GetNum(data, &value, sizeof(value));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetDouble getNum err. %d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::GetBoolean(const std::string &key, bool &value)
{
    uint32_t status = UpdateObject();
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetBoolean update object failed. %d", status);
        return status;
    }
    int32_t flag = 0;
    Bytes data;
    Bytes keyBytes = StringUtils::StrToBytes("p_" + key);
    status = flatObject_->GetItem(keyBytes, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetBoolean field not exist. %d %s", status, key.c_str());
        return status;
    }
    status = GetNum(data, &flag, sizeof(flag));
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetBoolean getNum err. %d", status);
        return status;
    }
    value = flag == 0 ? false : true;
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetString(const std::string &key, std::string &value)
{
    uint32_t status = UpdateObject();
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString Failed to update object. %d", status);
        return status;
    }
    Bytes data;
    Bytes keyBytes = StringUtils::StrToBytes("p_" + key);
    status = flatObject_->GetItem(keyBytes, data);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString field not exist. %d %s", status, key.c_str());
        return status;
    }
    status = StringUtils::BytesToString(data, value);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetString dataToVal err. %d", status);
    }
    return status;
}

uint32_t DistributedObjectImpl::GetObjectId(std::string &objectId)
{
    uint32_t status = StringUtils::BytesToString(flatObject_->GetId(), objectId);
    if (status != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetObjectId bytesToString err. %d", status);
    }
    return status;
}

DistributedObjectImpl::DistributedObjectImpl()
{
}

uint32_t DistributedObjectImpl::GetType(const std::string &key, Type &type)
{
    if (keyType_.count(key) != 0) {
        type = keyType_[key];
        return SUCCESS;
    }
    LOG_ERROR("DistributedObjectImpl::GetType key %s has no type",key.c_str());
    return ERR_KEY_TYPE;
}

uint32_t DistributedObjectImpl::UpdateObject()
{
    int32_t ret = flatObjectStore_->Get(flatObject_->GetId(), *flatObject_);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:UpdateObject err, ret %d", ret);
        return ret;
    }
    LOG_INFO("DistributedObjectImpl:update object success.");
    return SUCCESS;
}

FlatObject *DistributedObjectImpl::GetObject()
{
    return flatObject_;
}

DistributedObjectImpl::DistributedObjectImpl(FlatObject *flatObject, FlatObjectStore *flatObjectStore)
    : flatObject_(flatObject), flatObjectStore_(flatObjectStore)
{
}
} // namespace OHOS::ObjectStore