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

#ifndef NETWORK_TYPES_H
#define NETWORK_TYPES_H

#include <string>

namespace OHOS::ObjectStore {
using device_t = std::string;
constexpr ssize_t MAX_SIZE_ONCE = 1024 * 512 + 1024;  // 512K + 1K
constexpr uint32_t SEND_TIMEOUT = 3000;               // 3: send time out
enum class NetworkType {
    INVALID = 0,
    RPC = 1,
};

struct CommunicatorConfig {
    std::string appName {};
    CommunicatorConfig()
    {}

    CommunicatorConfig(const std::string &appName) : appName(appName)
    {}
};

struct DataBuffer {
public:
    bool Init(size_t size)
    {
        if (data != nullptr) {
            return false;
        }
        capacity = std::min(size, static_cast<size_t>(MAX_SIZE_ONCE));
        data = std::make_unique<char[]>(capacity);
        usedSize = 0;
        return true;
    }

    std::unique_ptr<char[]> data { nullptr };
    size_t usedSize { 0 };
    size_t capacity { 0 };
};
}  // namespace OHOS::ObjectStore
#endif