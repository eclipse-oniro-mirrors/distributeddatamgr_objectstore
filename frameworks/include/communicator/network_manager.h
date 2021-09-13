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

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <map>
#include "message.h"
#include "network.h"
#include "network_types.h"

namespace OHOS::ObjectStore {
class NetworkManager final {
public:
    NetworkManager() = default;
    ~NetworkManager() = default;
    uint32_t Init();
    uint32_t CreateNetwork(const std::string &name, NetworkType type, NetworkObserver *observer);
    void Destory();
    uint32_t SendMessage(const device_t &target, std::shared_ptr<Message> &message);
    uint32_t GetLocalDeviceId(device_t &local);
    uint32_t GetRemoteStoreIds(std::vector<std::string> &remoteId);

private:
    struct NetworkID {
        std::string name {};
        NetworkType type { NetworkType::INVALID };
        NetworkID(const std::string &name, NetworkType type) : name(name), type(type)
        {}
        bool operator<(const NetworkID &networkID) const
        {
            if (type != networkID.type) {
                return type < networkID.type;
            }
            return name < networkID.name;
        }
    };
    uint32_t InitLocalDeviceId();
    std::map<NetworkID, std::shared_ptr<Network>> networks_ {};
    device_t local_ { "" };
    std::string configPath_;
};
}  // namespace OHOS::ObjectStore
#endif