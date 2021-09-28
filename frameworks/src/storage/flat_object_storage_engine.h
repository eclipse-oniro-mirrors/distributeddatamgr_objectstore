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

#ifndef FLAT_OBJECT_STORAGE_ENGINE_H
#define FLAT_OBJECT_STORAGE_ENGINE_H

#include <cstdint>
#include <map>
#include <vector>
#include <shared_mutex>

#include "storage_engine.h"

namespace OHOS::ObjectStore {
class FlatObjectStorageEngine : public StorageEngine {
public:
    FlatObjectStorageEngine() = default;
    ~FlatObjectStorageEngine() override;

    uint32_t Open() override;
    uint32_t Clear() override;
    uint32_t Close() override;
    uint32_t DeleteKey(const Key &key) override;
    uint32_t PutHash(const Key &key, const std::map<Field, Value> &fieldVals) override;
    uint32_t GetHash(const Key &key, std::map<Field, Value> &result) override;

private:
    // Only using std::map for memory storage data
    std::map<Key, std::map<Field, Value>> *hashDic_ = nullptr;
    bool opened_ = false;
    std::shared_mutex operationMutex_ {};
};
}  // namespace OHOS::ObjectStore
#endif
