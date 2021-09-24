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

#include "network_manager.h"
#include "rpc_network.h"
#include "objectstore_errors.h"
#include "softbus_bus_center.h"

namespace OHOS::ObjectStore {
namespace {
    static constexpr int32_t ID_BUF_LEN = 65;
}  // namespace

uint32_t NetworkManager::Init()
{
    return InitLocalDeviceId();
}

void NetworkManager::Destory()
{
    for (auto &network : networks_) {
        network.second->Stop();
    }
    networks_.clear();
    LOG_INFO("NetworkManager-%s: succeed to stop", __func__);
}

uint32_t NetworkManager::CreateNetwork(const std::string &name, NetworkType type, NetworkObserver *observer)
{
    if (name.empty() || observer == nullptr) {
        LOG_ERROR("NetworkManager-%s: invalid param", __func__);
        return ERR_INVAL;
    }
    auto it = networks_.find({ name, type });
    if (it != networks_.end()) {
        LOG_WARN("NetworkManager-%s: do not need create repeated network", __func__);
        return SUCCESS;
    }
    auto ret = InitLocalDeviceId();
    if (ret != SUCCESS) {
        return ret;
    }
    std::shared_ptr<Network> network = nullptr;
    switch (type) {
        case NetworkType::RPC:
            network = std::make_shared<RPCNetwork>(name, local_, observer);
            break;
        default:
            break;
    }
    if (network == nullptr) {
        LOG_ERROR("NetworkManager-%s: invalid network type", __func__);
        return ERR_INVAL;
    }
    ret = network->Start();
    if (ret != SUCCESS) {
        return ret;
    }
    networks_.insert({ { name, type }, network });
    return ret;
}

uint32_t NetworkManager::GetLocalDeviceId(device_t &local)
{
    auto ret = InitLocalDeviceId();
    if (ret == SUCCESS) {
        local = local_;
    }
    return ret;
}

uint32_t NetworkManager::SendMessage(const device_t &target, std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        LOG_ERROR("NetworkManager-%s: invalid message", __func__);
        return ERR_INVAL;
    }
    auto ret = ERR_NETWORK;
    for (auto &[networkName, network] : networks_) {
        ret = network->SendMessage(target, message);
        if (ret == SUCCESS) {
            return ret;
        }
    }
    LOG_ERROR("NetworkManager-%s: fail to send message (%d: %d), error: %u, %s", __func__, message->GetId(),
              static_cast<uint8_t>(message->GetType()), ret, target.c_str());
    return ret;
}

uint32_t NetworkManager::InitLocalDeviceId()
{
    if (local_ != "") {
        return SUCCESS;
    }
    NodeBasicInfo info;
    int32_t ret = GetLocalNodeDeviceInfo(PKG_NAME, &info);
    if (ret != SUCCESS) {
        LOG_ERROR("GetLocalNodeDeviceInfo error");
        return ERR_NETWORK;
    }
    local_ = info.networkId;
    return SUCCESS;
}

uint32_t NetworkManager::GetRemoteStoreIds(std::vector<std::string> &remoteId)
{
    for (auto ite = networks_.begin(); ite != networks_.end(); ite++) {
        return ite->second->GetRemoteStoreIds(remoteId);
    }
    return ERR_INVAL;
}
}  // namespace OHOS::ObjectStore