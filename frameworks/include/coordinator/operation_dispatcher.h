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

#ifndef OPERATION_DISPATCHER_H
#define OPERATION_DISPATCHER_H

#include <map>
#include <set>
#include <shared_mutex>
#include "bytes.h"
#include "macro.h"
#include "operation.h"

namespace OHOS::ObjectStore {
class OperationExecutor {
public:
    OperationExecutor() = default;
    virtual ~OperationExecutor() = default;
    virtual uint32_t Publish(const Bytes &key, const std::any &value) = 0;
    virtual uint32_t PublishDelete(const Bytes &key) = 0;
    virtual uint32_t Put(const Bytes &key, const std::any &value) = 0;
    virtual uint32_t Get(const Bytes &key, std::any &value) = 0;
    virtual uint32_t Delete(const Bytes &key) = 0;
};

class OperationDispatcher final {
public:
    static OperationDispatcher &GetInstance();
    uint32_t RegisterExecutor(DataType type, std::shared_ptr<OperationExecutor> executor);
    uint32_t UnregisterExecutor(DataType type, std::shared_ptr<OperationExecutor> executor);
    uint32_t Publish(DataType type, const Bytes &key, const std::any &value);
    uint32_t PublishDelete(DataType type, const Bytes &key);
    uint32_t Put(DataType type, const Bytes &key, const std::any &value);
    uint32_t Get(DataType type, const Bytes &key, std::any &value);
    uint32_t Delete(DataType type, const Bytes &key);

private:
    DISABLE_COPY_AND_MOVE(OperationDispatcher);
    OperationDispatcher() = default;
    ~OperationDispatcher() = default;
    std::map<DataType, std::set<std::shared_ptr<OperationExecutor>>> executorMap_ {};
    std::shared_mutex executorMutex_ {};
};
}  // namespace OHOS::ObjectStore
#endif