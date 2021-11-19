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

#include "distributed_object_impl.h"
#include "objectstore_errors.h"
#include "string_utils.h"
#include "bytes.h"
#include "logger.h"
#include "string.h"
#include "securec.h"

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
void PutNum(int32_t offset, void* val, int32_t valLen, Bytes &data)
{
    uint32_t len = valLen + offset;
    if (len > sizeof(data.front()) * data.size()) {
        data.resize(len);
    }

    for (int i = 0; i < valLen; i++) {
        // 8 bit = 1 byte
        data[offset + i] = *(static_cast<uint64_t *>(val)) >> ((valLen - i - 1) * 8);
    }
    return;
}

uint32_t GetNum(Bytes &data, int32_t offset, void* val, int32_t valLen)
{
    uint8_t *value = (uint8_t *)val;
    if (data.size() != offset + valLen) {
        LOG_ERROR("DistributedObjectImpl:GetNum data.size() %d, offset %d, valLen %d",data.size(), offset, valLen);
        return ERR_RANGE_LIST;
    }
    for (int i = 0; i < valLen; i++) {
        value[i] = data[offset + valLen - 1 - i];
    }
    return SUCCESS;
}

/*uint32_t BytesToString(Bytes input, std::string& str)
{
    if (input.end() - input.begin() <= sizeof(int32_t)) {
        return ERR_INVAL;
    }
    std::vector<uint8_t>::const_iterator first = input.begin() + sizeof(int32_t);
    std::vector<uint8_t>::const_iterator end = input.end();
    Bytes rstStr(first,end);
    str.assign(reinterpret_cast<char*>(rstStr.data()), rstStr.size());
    return SUCCESS;
}*/

void TransBytes(void* input, int32_t inputLen, int32_t flag, Bytes &data)
{
    data.resize(sizeof(int32_t) + inputLen, 0);
    PutNum(0, &flag, sizeof(flag), data);
    PutNum(sizeof(int32_t), input, inputLen, data);
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


Bytes DistributedObjectImpl::StrToFieldBytes (const std::string &src)
{
    Bytes data;
    PutNum(0, &stringVal, sizeof(stringVal), data);
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
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetChar update object failed.ret %d", ret);
        return ERR_GET_LIST;
    }
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetChar field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    int16_t tmpVal;
    GetNum(data, sizeof(characterVal), &tmpVal, sizeof(tmpVal));
    value = tmpVal;
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetInt(const std::string &key, int32_t &value)
{
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetInt update object failed.ret %d", ret);
        return ERR_GET_LIST;
    }
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetInt field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    GetNum(data, sizeof(integerVal), &value, sizeof(value));
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetShort(const std::string &key, int16_t &value)
{
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetShort update object failed.ret %d", ret);
        return ERR_GET_LIST;
    }
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetShort field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    GetNum(data, sizeof(shortVal), &value, sizeof(value));
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetLong(const std::string &key, int64_t &value)
{
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetLong update object failed.ret %d", ret);
        return ERR_GET_LIST;
    }
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetLong field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    GetNum(data, sizeof(longVal), &value, sizeof(value));
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetFloat(const std::string &key, float &value)
{
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetFloat update object failed.ret %d", ret);
        return ERR_GET_LIST;
    }
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetFloat field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    GetNum(data, sizeof(floatVal), &value, sizeof(value));
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetDouble(const std::string &key, double &value)
{
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetDouble update object failed.ret %d", ret);
        return ERR_GET_LIST;
    }
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetDouble field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    GetNum(data, sizeof(doubleVal), &value, sizeof(value));
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetBoolean(const std::string &key, bool &value)
{
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetBoolean update object failed.ret %d", ret);
        return ERR_GET_LIST;
    }
    int32_t flag = 0;
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetBoolean field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    GetNum(data, sizeof(booleanVal), &flag, sizeof(flag));
    value = flag == 0 ? false : true;
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetByte(const std::string &key, int8_t &value)
{
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetByte Failed to update object.ret %d", ret);
        return ERR_GET_LIST;
    }
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetByte field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    if (data.size() != sizeof(value)) {
        LOG_ERROR("DistributedObjectImpl:GetByte size err.");
        return ERR_NOT_EXIST;
    }
    value =data[0];
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetString(const std::string &key, std::string &value)
{
    uint32_t ret = updateObject();
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString Failed to update object.ret %d", ret);
        return ERR_GET_LIST;
    }
    Bytes data;
    Bytes keyBytes= StrToFieldBytes(key);
    ret = flatObject_->GetField(keyBytes, data);
    if (ret != SUCCESS) {
        LOG_ERROR("DistributedObjectImpl:GetString field not exist. %d %s", ret, key.c_str());
        return ERR_NOT_EXIST;
    }
    StringUtils::BytesToString(data, value);
    return SUCCESS;
}

uint32_t DistributedObjectImpl::GetObjectId(std::string &objectId)
{
    int ret = updateObject();
    if(ret != SUCCESS){
        LOG_ERROR("DistributedObjectImpl:GetObjectId updateObject err. ret %d",ret);
        return ERR_ID_EXIST;
    }
    StringUtils::BytesToString(flatObject_->GetId(), objectId);
    return SUCCESS;
}



uint32_t DistributedObjectImpl::updateObject()
{
    std::string id;
    StringUtils::BytesToString(flatObject_->GetId(), id);
    LOG_ERROR("update id %s", id.c_str());
    int32_t ret = flatObjectStore_->Get(flatObject_->GetId(), *flatObject_);
    std::map<Bytes, Bytes> fields = flatObject_->GetFields();
    auto iter = fields.begin();
    while (iter != fields.end()) {
        std::string key;
        std::string value;
        StringUtils::BytesToString(iter->first, key);
        StringUtils::BytesToString(iter->second, value);
        LOG_ERROR("update key %s, value %s",key.c_str(), value.c_str());
        iter++;
    }
    LOG_INFO("DistributedObjectImpl:updateObject %d",ret);
    return ret;
}

FlatObject *DistributedObjectImpl::GetObject()
{
    return flatObject_;
}

DistributedObjectImpl::DistributedObjectImpl(FlatObject *flatObject, FlatObjectStore *flatObjectStore)
        : flatObject_(flatObject), flatObjectStore_(flatObjectStore) {}
}