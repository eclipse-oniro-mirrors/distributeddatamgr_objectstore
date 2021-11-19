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
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
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
    if (fields_.count(key) == 0) {
        return ERR_INVAL;
    }
    value = fields_.at(key);
    return SUCCESS;
}

void FlatObject::SetField(const Bytes &field, const Bytes &value)
{
    fields_[field] = value;
}

void FlatObject::SetFields(const std::map<Bytes, Bytes> &fields)
{
    for (const auto &field : fields) {
        fields_[field.first] = field.second;
    }
}
}  // namespace OHOS::ObjectStore
