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

#ifndef RPC_NETWORK_H
#define RPC_NETWORK_H

#include "network.h"
#include "communicator_softbus_adapter.h"

namespace OHOS::ObjectStore {
enum DeviceStatus {
    DISCONNECT,
    CONNCET
};

struct SessionInfo {
    uint32_t waitCount { 0 };
    std::mutex sessionMutex {};
    std::condition_variable CV {};
    enum DeviceStatus connectStatus_ {DISCONNECT};
    int sessionId = -1;
    SessionInfo() = default;
};

class RPCNetwork final : public Network {
public:
    // there is sessionid same as deviceId
    RPCNetwork(const std::string &name, const device_t &local, NetworkObserver *observer);
    ~RPCNetwork() override;
    int32_t OnSessionOpened(const std::string& deviceId);
    void OnSessionClosed(const std::string& deviceId);
    void OnMessageReceived(const std::string& deviceId, const char *data, uint32_t len);
    void OnRemoteDied();
    uint32_t Init() override;
    void Destory() override;
    int32_t OpenSession(const std::string &networkId) override;
    void CloseSession(const std::string &networkId) override;
    uint32_t SendMessage(const std::string &networkId, std::shared_ptr<Message> message) override;

    std::string GetDeviceIdBySession(int session);
private:
    uint32_t WriteMessageToSession(const std::string &networkId, std::shared_ptr<Message> &message);
    SessionInfo* GetSessionFromCache(const std::string& networkId);
    std::map<const std::string, SessionInfo> sessionCache_; // session in info
    std::shared_mutex sessionCacheMutex_ {};
    std::shared_mutex openSessionMutex_ {};
    std::shared_ptr<SoftBusListener> networkListener_ { nullptr };
};

class SoftBusListener {
public:

    INodeStateCb *GetNodeStateCallback() const;
    ISessionListener *GetSessionListener() const;
    uint32_t Register();
    void UnRegister();
    explicit SoftBusListener(RPCNetwork *rpcNetwork);
    virtual ~SoftBusListener();
    static void OnNodeOnline(NodeBasicInfo *info);
    static void OnNodeOffline(NodeBasicInfo *info);
    static int OnSessionOpened(int sessionId, int result);
    static void OnSessionClosed(int sessionId);
    static void OnMessageReceived(int sessionId, const void *data, uint32_t len);
private:
    static RPCNetwork *network_;
    INodeStateCb *nodeStateCallback_ = nullptr;
    ISessionListener *sessionListener_ = nullptr;
};
}  // namespace OHOS::ObjectStore

#endif