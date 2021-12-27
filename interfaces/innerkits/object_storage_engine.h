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

#ifndef OBJECT_STORAGE_ENGINE_H
#define OBJECT_STORAGE_ENGINE_H

#include <kv_store_observer.h>

#include <cstdint>
#include <map>
#include <vector>

namespace OHOS::ObjectStore {
using Key = std::vector<uint8_t>;
using Value = std::vector<uint8_t>;
using Field = std::vector<uint8_t>;

class TableWatcher : public DistributedDB::KvStoreObserver {
    void OnChange(const KvStoreChangedData &data) override;
};

class ObjectStorageEngine {
public:
    ObjectStorageEngine(const ObjectStorageEngine &) = delete;
    ObjectStorageEngine &operator=(const ObjectStorageEngine &) = delete;
    ObjectStorageEngine(ObjectStorageEngine &&) = delete;
    ObjectStorageEngine &operator=(ObjectStorageEngine &&) = delete;
    ObjectStorageEngine() = default;
    virtual ~ObjectStorageEngine() = default;
    virtual uint32_t Open() = 0;
    virtual uint32_t Clear() = 0;
    virtual uint32_t Close() = 0;
    virtual uint32_t DeleteTable(const std::string &key) = 0;
    virtual uint32_t CreateTable(const std::string &key) = 0;
    virtual uint32_t GetTable(const std::string &key, std::map<Field, Value> &result) = 0;
    virtual uint32_t UpdateItems(const std::string &key, std::map<Field, Value> &data) = 0;
    virtual uint32_t GetItem(const std::string &key, const Field &itemKey, const Field &value) = 0;
    virtual uint32_t RegisterObserver(const std::string &key, std::shared_ptr<TableWatcher> watcher) = 0;
    virtual uint32_t UnRegisterObserver(const std::string &key) = 0;
    virtual uint32_t ChangeKey(const std::string &oldKey, const std::string &newKey) = 0;
    virtual uint32_t ChangeSession(const std::string &objectId, const std::string &sessionId) = 0;
};

} // namespace OHOS::ObjectStore
#endif