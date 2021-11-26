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

#ifndef OBJECT_STORE_EXECUTOR_H
#define OBJECT_STORE_EXECUTOR_H
#include "objectstore_errors.h"
#include "operation_dispatcher.h"

namespace OHOS::ObjectStore {
class MapStoreExecutor : public OperationExecutor {
public:
    MapStoreExecutor() = default;
    ~MapStoreExecutor() = default;
    static constexpr DataType type = DataType::MAP_TYPE;

private:
    uint32_t Publish(const Bytes &key, const std::any &value) override;
    uint32_t PublishDelete(const Bytes &key) override;
    uint32_t Put(const Bytes &key, const std::any &value) override;
    uint32_t Get(const Bytes &key, std::any &value) override;
    uint32_t Delete(const Bytes &key) override;
};


class ObjectStoreExecutor {
public:
    ObjectStoreExecutor() = default;
    ~ObjectStoreExecutor() = default;

    uint32_t Init()
    {
        AddExecutor(MapStoreExecutor::type, std::make_shared<MapStoreExecutor>());
        return SUCCESS;
    }
    uint32_t Destory()
    {
        for (auto &[type, executor] : executors_) {
            OperationDispatcher::GetInstance().UnregisterExecutor(type, executor);
        }
        executors_.clear();
        return SUCCESS;
    }

private:
    template<typename T>
    void AddExecutor(DataType type, T &&executor)
    {
        if (executor == nullptr) {
            return;
        }
        OperationDispatcher::GetInstance().RegisterExecutor(type, executor);
        executors_.emplace(type, executor);
    }
    std::map<DataType, std::shared_ptr<OperationExecutor>> executors_ {};
};
}  // namespace OHOS::ObjectStore

#endif