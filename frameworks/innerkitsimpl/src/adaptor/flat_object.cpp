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

#include "flat_object.h"

#include "logger.h"
#include "objectstore_errors.h"
#include "string_utils.h"

namespace OHOS::ObjectStore {
FlatObject::FlatObject()
{
    storageEngine_ = std::make_shared<FlatObjectStorageEngine>();
}
const Bytes &FlatObject::GetId() const
{
    return id_;
}

void FlatObject::SetId(const Bytes &id)
{
    id_ = id;
}

const std::map<Bytes, Bytes> &FlatObject::GetFields() const
{
    return fields_;
}

const uint32_t &FlatObject::GetField(Bytes &key, Bytes &value) const
{
    if (fields_.count(key) != 0) {
        value = fields_.at(key);
        return SUCCESS;
    }
    std::string oid;
    uint32_t status = StringUtils::BytesToString(id_, oid);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObject::GetField get string failed %d", status);
        return status;
    }
    status = storageEngine_->GetItem(oid, key, value);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObject::GetField get item failed %d", status);
    }
    return status;
}

uint32_t FlatObject::SetField(const Bytes &field, const Bytes &value)
{
    std::string oid;
    uint32_t status = StringUtils::BytesToString(id_, oid);
    if (status != SUCCESS) {
        LOG_ERROR("FlatObject::SetFields get oid err %d", status);
        return status;
    }
    std::map<Bytes, Bytes> fields;
    fields.insert_or_assign(field, value);
    status = storageEngine_->UpdateItems(oid, const_cast<std::map<Field, Value> &>(fields));
    if (status != SUCCESS) {
        LOG_ERROR("FlatObjectStore::Get getTable failed %d", status);
    }
    fields_[field] = value;
    return status;
}
void FlatObject::SetFields(const std::map<Bytes, Bytes> &fields)
{
    for (const auto &field : fields) {
        fields_[field.first] = field.second;
    }
}
} // namespace OHOS::ObjectStore
