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

#ifndef NETWORK_H
#define NETWORK_H

#include <condition_variable>
#include <map>
#include <set>
#include <mutex>
#include <shared_mutex>

#include "message.h"
#include "communicator_session.h"
#include "thread_pool.h"
#include "communicator_softbus_adapter.h"

namespace OHOS::ObjectStore {
enum class NetworkStatus {
    OFF_LINE,
    ON_LINE
};
struct NetworkNode {
    device_t deviceId { "" };
    std::string networkId {};
    std::string type {};
    NetworkStatus status { NetworkStatus::OFF_LINE };
    NetworkNode()
    {}
};

class NetworkObserver {
public:
    virtual ~NetworkObserver() = default;
    virtual void OnNodeOnline(const device_t &device) = 0;
    virtual void OnNodeOffline(const device_t &device) = 0;
    virtual void OnNewMessage(const std::shared_ptr<Message> &message) = 0;
};

class Network {
public:
    Network() = default;
    Network(const std::string &name, const device_t &local, NetworkObserver *observer);
    virtual ~Network();
    uint32_t Start();
    void Stop();
    virtual uint32_t Init() = 0;
    virtual void Destory() = 0;
    virtual int32_t OpenSession(const std::string &networkId) = 0;
    virtual void CloseSession(const std::string &networkId) = 0;
    virtual uint32_t SendMessage(const std::string &networkId, std::shared_ptr<Message> message) = 0;
    uint32_t GetRemoteStoreIds(std::vector<std::string> &remoteId);
    uint32_t AddRemoteStoreIds(const std::string &remoteId);
    uint32_t RemoveRemoteStoreIds(const std::string &remoteId);
protected:
    enum class UpdateStatus { INITIAL = 0, RUNNING, FINISHED };
    struct UpdateTaskInfo {
        std::condition_variable updateCV {};
        UpdateStatus status { UpdateStatus::INITIAL };
    };
    uint32_t InitLocalNetworkID();
    uint32_t GetDeviceNetworkId(const device_t &deviceId, std::string &networkId);
    uint32_t FindNetworkIdFromLocal(const device_t &deviceId, std::string &networkId);
    void UpdateNetworkId(const device_t &deviceId);
    bool ReceiveData(std::shared_ptr<CommunicatorSession> &session, uint32_t status);
    bool ReadMessageFromSession(std::shared_ptr<CommunicatorSession> &session,
        std::vector<std::shared_ptr<Message>> &messages);
    DataBuffer *GetSessionDataBuffer();
    void HandleSessionConnected(const std::string &deviceId);
    void HandleSessionDisConnected(const std::string &deviceId);
    void HandleHandShakeResponse(std::shared_ptr<CommunicatorSession> &session, std::shared_ptr<Message> message);
    void UpdateNeighborStatus(const NetworkNode &neighbor);
    void NotifyObservers(const NetworkNode &neighbor) const;

    std::map<device_t, NetworkNode> neighbors_ {};
    std::shared_mutex neighborsMutex_ {};
    std::string networkName_ {};
    NetworkNode local_ {};
    NetworkObserver *observer_ { nullptr };
    ThreadPool threadPool_ { "objectnetwork" };
    CommunicatorSoftbusAdapter dataManager_;
    std::set<std::string> remoteIds_;
};
}  // namespace OHOS::ObjectStore
#endif