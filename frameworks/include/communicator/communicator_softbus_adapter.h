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
#ifndef COMMUNICATOR_SOFTBUS_ADAPTER_H
#define COMMUNICATOR_SOFTBUS_ADAPTER_H

#include <string>
#include <map>
#include <memory>
#include <mutex>

#include "session.h"
#include "softbus_bus_center.h"

namespace OHOS::ObjectStore {
class ServiceDeathRecipient;
class SoftBusListener;
constexpr int32_t TYPE_SEND_BYTE = 15;
constexpr int32_t TYPE_SEND_MESSAGE = 16;
static const char *PKG_NAME = "com.objectstore.foundation";

class CommunicatorSoftbusAdapter {
public:
    int Init(std::shared_ptr<SoftBusListener> &listener);
    uint32_t OpenSoftbusLink(const std::string &networkId);
    uint32_t Destroy();
    uint32_t SendMsg(const std::string &networkId, const uint8_t *data, uint32_t dataSize,
        int32_t type = TYPE_SEND_BYTE);
    uint32_t GetDeviceIdBySession(int sessionId, std::string &deviceId); // return empty when session not existed
private:
    std::map<std::string, int32_t> sessionDevDic_;  // networikId-sessionId
    std::shared_ptr<SoftBusListener> networkListener_;
    std::mutex operationMutex_ {};
};
}  // namespace OHOS::ObjectStore
#endif // COMMUNICATOR_SOFTBUS_ADAPTER_H
