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

#ifndef STORAGE_ENGINE_H
#define STORAGE_ENGINE_H

#include <cstdint>
#include <map>
#include <vector>

namespace OHOS::ObjectStore {
using Key = std::vector<uint8_t>;
using Value = std::vector<uint8_t>;
using Field = std::vector<uint8_t>;

class StorageEngine {
public:
    StorageEngine(const StorageEngine &) = delete;
    StorageEngine &operator=(const StorageEngine &) = delete;
    StorageEngine(StorageEngine &&) = delete;
    StorageEngine &operator=(StorageEngine &&) = delete;
    StorageEngine() = default;
    virtual ~StorageEngine() = default;
    virtual uint32_t Open() = 0;
    virtual uint32_t Clear() = 0;
    virtual uint32_t Close() = 0;
    virtual uint32_t DeleteKey(const Key &key) = 0;
    virtual uint32_t PutHash(const Key &key, const std::map<Field, Value> &fieldVals) = 0;
    virtual uint32_t GetHash(const Key &key, std::map<Field, Value> &result) = 0;
};
}  // namespace OHOS::ObjectStore
#endif