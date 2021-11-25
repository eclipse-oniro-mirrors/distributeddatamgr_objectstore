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

#include <type_traits>

#include "objectstore_errors.h"
#include "string_utils.h"
#include "bytes.h"
#include "logger.h"
#include "securec.h"
#include "distributed_object_impl.h"

namespace OHOS::ObjectStore {
DistributedObjectImpl::~DistributedObjectImpl()
{
    delete flatObject_;
}

namespace {
static int32_t stringVal = 0;     // string
static int32_t integerVal = 1;    // integer
static int32_t booleanVal = 2;    // boolean
static int32_t shortVal = 3;      // short
static int32_t longVal = 4;       // long
static int32_t floatVal = 5;      // float
static int32_t doubleVal = 6;     // double
static int32_t characterVal = 7;  // character
static int32_t byteVal = 8;  // character

inline bool IsBigEndian()
{
    uint32_t data = 0x12345678; // 0x12345678 only used here, for endian test
    uint8_t *firstByte = reinterpret_cast<uint8_t *>(&data);
    if (*firstByte == 0x12) { // 0x12 only used here, for endian test
        return true;
    }
    return false;
}

template<typename T> T HostToNet(const T &from)
{
    if (IsBigEndian()) {
        return from;
    } else {
        T to;
        size_t typeLen = sizeof(T);
        const uint8_t *fromByte = reinterpret_cast<const uint8_t *>(&from);
        uint8_t *toByte = reinterpret_cast<uint8_t *>(&to);
        for (size_t i = 0; i < typeLen; i++) {
            toByte[i] = fromByte[typeLen - i - 1]; // 1 is for index boundary
        }
        return to;
    }
}

template<typename T> T NetToHost(const T &from)
{
    return HostToNet(from);
}

// Is same as > L2, use Little-Endian, 64-bits system
void PutNum(void* val, int32_t offset, int32_t valLen, Bytes &data)
{
    uint32_t len = valLen + offset;
    if (len > sizeof(data.front()) * data.size()) {
        data.resize(len);
    }

    for (int i = 0; i < valLen; i++) {
        // 8 bit = 1 byte
        data[offset + i] = *(static_cast<uint64_t *>(val)) >> ((valLen - i - 1) * 8);
    }
}

uint32_t GetNum(Bytes &data, int32_t offset, void* val, int32_t valLen)
{
    uint8_t *value = (uint8_t *)val;
    int32_t len = offset + valLen;
    if (data.size() != len) {
        LOG_ERROR("DistributedObjectImpl:GetNum data.size() %d, offset %d, valLen %d",
            data.size(), offset, valLen);
        return ERR_DATA_LEN;
    }
    for (int i = 0; i < valLen; i++) {
        value[i] = data[len - 1 - i];
    }
    return SUCCESS;
}

void TransBytes(void* input, int32_t inputLen, int32_t flag, Bytes &data)
{
    data.resize(sizeof(int32_t) + inputLen, 0);
    PutNum(&flag, 0, sizeof(flag), data);
    PutNum(input, sizeof(int32_t), inputLen, data);
    return;
}
}

uint32_t DistributedObjectImpl::PutChar(const std::string &key, char value)
{
    Bytes data;
    uint16_t val = value;
    TransBytes(&val, sizeof(val), characterVal, data);
    flatObject_->SetField(StrToFieldBytes(key), data);
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutInt(const std::string &key, int32_t value)
{
    Bytes data;
    TransBytes(&value, sizeof(value), integerVal, data);
    flatObject_->SetField(StrToFieldBytes(key), data);
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutShort(const std::string &key, int16_t value)
{
    Bytes data;
    TransBytes(&value, sizeof(value), shortVal, data);
    flatObject_->SetField(StrToFieldBytes(key), data);
    return SUCCESS;
}

Bytes DistributedObjectImpl::StrToFieldBytes(const std::string &src)
{
    Bytes data;
    PutNum(&stringVal, 0, sizeof(stringVal), data);
    Bytes dst = StringUtils::StrToBytes(src);
    data.insert(data.end(), dst.begin(), dst.end());
    return data;
}

uint32_t DistributedObjectImpl::PutLong(const std::string &key, int64_t value)
{
    Bytes data;
    TransBytes(&value, sizeof(value), longVal, data);
    flatObject_->SetField(StrToFieldBytes(key), data);
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutFloat(const std::string &key, float value)
{
    Bytes data;
    TransBytes(&value, sizeof(value), floatVal, data);
    flatObject_->SetField(StrToFieldBytes(key), data);
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutDouble(const std::string &key, double value)
{
    Bytes data;
    TransBytes(&value, sizeof(value), doubleVal, data);
    flatObject_->SetField(StrToFieldBytes(key), data);
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutBoolean(const std::string &key, bool value)
{
    Bytes data;
    int32_t val = value ? 1 : 0;
    TransBytes(&val, sizeof(val), booleanVal, data);
    flatObject_->SetField(StrToFieldBytes(key), data);
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutString(const std::string &key, const std::string &value)
{
    flatObject_->SetField(StrToFieldBytes(key), StrToFieldBytes(value));
    return SUCCESS;
}

uint32_t DistributedObjectImpl::PutByte(const std::string &key, int8_t value)
{
    Bytes data;
    TransBytes(&value, sizeof(value), byteVal, data);
    flatObject_->SetField(StrToFieldBytes(key), data);
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetChar(const std::string &key, char &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetChar update object failed.ret %d", ret);
        return ret;
    }
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetChar field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    int16_t tmpVal;
    uint32_t getRet = GetNum(data, sizeof(characterVal), &tmpVal, sizeof(tmpVal));
    if (getRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetChar getNum err.ret %d", getRet);
        return getRet;
    }
    value = tmpVal;
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetInt(const std::string &key, int32_t &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetInt update object failed.ret %d", ret);
        return ret;
    }
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetInt field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    uint32_t getRet = GetNum(data, sizeof(integerVal), &value, sizeof(value));
    if (getRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetInt getNum err.ret %d", getRet);
        return getRet;
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetShort(const std::string &key, int16_t &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetShort update object failed.ret %d", ret);
        return ret;
    }
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetShort field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    uint32_t getRet = GetNum(data, sizeof(shortVal), &value, sizeof(value));
    if (getRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetShort getNum err.ret %d", getRet);
        return getRet;
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetLong(const std::string &key, int64_t &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetLong update object failed.ret %d", ret);
        return ret;
    }
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetLong field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    uint32_t getRet = GetNum(data, sizeof(longVal), &value, sizeof(value));
    if (getRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetLong getNum err.ret %d", getRet);
        return getRet;
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetFloat(const std::string &key, float &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetFloat update object failed.ret %d", ret);
        return ret;
    }
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetFloat field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    uint32_t getRet = GetNum(data, sizeof(floatVal), &value, sizeof(value));
    if (getRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetFloat getNum err.ret %d", getRet);
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetDouble(const std::string &key, double &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetDouble update object failed.ret %d", ret);
        return ret;
    }
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetDouble field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    uint32_t getRet = GetNum(data, sizeof(doubleVal), &value, sizeof(value));
    if (getRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetDouble getNum err.ret %d", getRet);
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetBoolean(const std::string &key, bool &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetBoolean update object failed.ret %d", ret);
        return ret;
    }
    int32_t flag = 0;
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetBoolean field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    uint32_t getRet = GetNum(data, sizeof(booleanVal), &flag, sizeof(flag));
    if (getRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetBoolean getNum err.ret %d", getRet);
        return getRet;
    }
    value = flag == 0 ? false : true;
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetByte(const std::string &key, int8_t &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetByte Failed to update object.ret %d", ret);
        return ret;
    }
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetByte field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    if (data.size() != sizeof(value)) {
        LOG_ERROR("DistributedObjectImpl:GetByte size err.");
        return ERR_BYTE_SIZE;
    }
    value = data[0];
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetString(const std::string &key, std::string &value)
{
    uint32_t ret = UpdateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString Failed to update object.ret %d", ret);
        return ret;
    }
    Bytes data;
    Bytes keyBytes = StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString field not exist. %d %s", ret, key.c_str());
        return ret;
    }
    uint32_t getRet = StringUtils::BytesToString(data, value);
    if (getRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetString dataToVal err.ret %d", getRet);
        return getRet;
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetObjectId(std::string &objectId)
{
    uint32_t ret = StringUtils::BytesToString(flatObject_->GetId(), objectId);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl::GetObjectId bytesToString err.ret %d", ret);
        return ret;
    }
    return SUCCESS;
}

uint32_t DistributedObjectImpl::UpdateObject()
{
    std::string id;
    StringUtils::BytesToString(flatObject_->GetId(), id);
    int32_t ret = flatObjectStore_->Get(flatObject_->GetId(), *flatObject_);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:UpdateObject err, ret %d", ret);
        return ret;
    }
    std::map<Bytes, Bytes> fields = flatObject_->GetFields();
    auto iter = fields.begin();
    std::string key;
    std::string value;
    uint32_t keyRet = StringUtils::BytesToString(iter->first, key);
    if (keyRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:UpdateObject keyToStr err ,ret %d", keyRet);
        return keyRet;
    }
    uint32_t valRet = StringUtils::BytesToString(iter->second, value);
    if (valRet != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:UpdateObject valToStr err ,ret %d", valRet);
        return valRet;
    }
    LOG_INFO("DistributedObjectImpl:update object success.");
    return SUCCESS;
}

FlatObject *DistributedObjectImpl::GetObject()
{
    return flatObject_;
}

DistributedObjectImpl::DistributedObjectImpl(FlatObject *flatObject, FlatObjectStore *flatObjectStore)
    : flatObject_(flatObject), flatObjectStore_(flatObjectStore) {}
}