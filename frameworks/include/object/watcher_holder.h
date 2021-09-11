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

#ifndef WATCHER_HOLDER_H
#define WATCHER_HOLDER_H

#include <map>
#include <memory>
#include <mutex>
#include "flat_object_store.h"
#include "object_observer.h"

namespace OHOS::ObjectStore {
class WatcherHolder {
public:
    std::shared_ptr<ObjectObserver> hold(const Bytes &id, std::shared_ptr<FlatObjectWatcher> watcher)
    {
        if (watcher == nullptr) {
            return nullptr;
        }
        auto key = std::make_pair(id, watcher);
        std::lock_guard<std::mutex> lock(adaptersMutex_);
        auto it = adapters_.find(key);
        if (it != adapters_.end()) {
            return it->second;
        }
        auto observer = std::make_shared<ObserverAdapter>(watcher);
        adapters_.emplace(key, observer);
        return observer;
    }

    std::shared_ptr<ObjectObserver> unhold(const Bytes &id, std::shared_ptr<FlatObjectWatcher> watcher)
    {
        if (watcher == nullptr) {
            return nullptr;
        }
        auto key = std::make_pair(id, watcher);
        std::lock_guard<std::mutex> lock(adaptersMutex_);
        auto it = adapters_.find(key);
        if (it == adapters_.end()) {
            return nullptr;
        }
        adapters_.erase(it);
        return it->second;
    }

private:
    class ObserverAdapter : public ObjectObserver {
    public:
        ObserverAdapter(std::shared_ptr<FlatObjectWatcher> watcher) : watcher_(watcher)
        {}

        ~ObserverAdapter() override = default;

        void OnChanged(const Bytes &objectId) override
        {
            if (watcher_ != nullptr) {
                watcher_->OnChanged(objectId);
            }
        }

        void OnDeleted(const Bytes &objectId) override
        {
            if (watcher_ != nullptr) {
                watcher_->OnDeleted(objectId);
            }
        }

    private:
        std::shared_ptr<FlatObjectWatcher> watcher_;
    };

    std::mutex adaptersMutex_;
    std::map<std::pair<Bytes, std::shared_ptr<FlatObjectWatcher>>, std::shared_ptr<ObserverAdapter>> adapters_;
};
}  // namespace OHOS::ObjectStore

#endif  // WATCHER_HOLDER_H
