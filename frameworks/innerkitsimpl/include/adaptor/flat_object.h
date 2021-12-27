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

#ifndef FLAT_OBJECT_H
#define FLAT_OBJECT_H

#include <map>

#include "bytes.h"
#include "flat_object_storage_engine.h"

namespace OHOS::ObjectStore {
class FlatObject {
public:
    FlatObject();
    ~FlatObject() = default;
    const Bytes &GetId() const;
    void SetId(const Bytes &id);
    const std::map<Bytes, Bytes> &GetFields() const;
    const uint32_t &GetField(Bytes &key, Bytes &value) const;
    uint32_t SetField(const Bytes &field, const Bytes &value);
    void SetFields(const std::map<Bytes, Bytes> &fields);

private:
    Bytes id_;
    std::shared_ptr<FlatObjectStorageEngine> storageEngine_;
    std::map<Bytes, Bytes> fields_{};
};
} // namespace OHOS::ObjectStore

#endif // FLAT_OBJECT_H
