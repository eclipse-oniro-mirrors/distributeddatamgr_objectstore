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
 
#ifndef OBJECT_OBSERVER_H
#define OBJECT_OBSERVER_H

#include <string>
#include <vector>
#include "bytes.h"

namespace OHOS::ObjectStore {
enum class ObserverMode : uint32_t {
    OBSERVER_INVALID_MODE = 0x00,
    OBSERVER_AUTO_GET = 0x01,
    OBSERVER_AUTO_REOBSERVER = 0x01 << 1,
    OBSERVER_DEFAULT_MODE = OBSERVER_AUTO_GET | OBSERVER_AUTO_REOBSERVER,
};

class ObjectObserver {
public:
    virtual ~ObjectObserver() = default;
    virtual void OnChanged(const Bytes &key) = 0;
    virtual void OnDeleted(const Bytes &key) = 0;
};
}  // namespace OHOS::ObjectStore
#endif