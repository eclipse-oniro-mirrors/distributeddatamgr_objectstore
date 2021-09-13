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

#ifndef FLAT_OBJECT_STORE_H
#define FLAT_OBJECT_STORE_H

#include <memory>
#include <string>
#include "flat_object.h"
#include "watcher.h"

namespace OHOS::ObjectStore {
class StorageEngine;
class WatcherHolder;

class FlatObjectWatcher : public Watcher<Bytes> {
};

class FlatObjectStore {
public:
    explicit FlatObjectStore(const std::string &bundleName, const std::string &storeName);
    ~FlatObjectStore();
    uint32_t Open();
    uint32_t Close();
    const std::string &GetName() const;
    const std::string &GetPrefix() const;
    const std::string &GetBundleName() const;
    const std::vector<std::string> &GetRemoteStoreIds();
    uint32_t Put(const FlatObject &flatObject);
    uint32_t Get(const Bytes &objectId, FlatObject &flatObject) const;
    uint32_t Delete(const Bytes &objectId);
    uint32_t Watch(const Bytes &objectId, std::shared_ptr<FlatObjectWatcher> watcher);
    uint32_t Unwatch(const Bytes &objectId, std::shared_ptr<FlatObjectWatcher> watcher);

private:
    bool IsLocalObject(const Bytes &objectId) const;

    std::string bundleName_;
    std::string storeName_;
    std::string prefix_;
    std::vector<std::string> remoteIds_ {};
    std::string deviceId_;
    std::unique_ptr<WatcherHolder> watcherHolder_;
    std::shared_ptr<StorageEngine> storageEngine_;
};
}  // namespace OHOS::ObjectStore

#endif  // FLAT_OBJECT_STORE_H
