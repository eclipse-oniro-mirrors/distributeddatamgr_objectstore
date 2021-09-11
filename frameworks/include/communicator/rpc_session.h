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

#ifndef RPC_SESSION_H
#define RPC_SESSION_H

#include "communicator_session.h"
namespace OHOS::ObjectStore {
class RPCSession final : public CommunicatorSession {
public:
    RPCSession(const std::string &networkId)
        : CommunicatorSession(), networkId_(networkId)
    {}
    RPCSession(void *data, size_t len) : data_(data), len_(len)
    {}
    virtual ~RPCSession() = default;
    static const uint8_t *EncodeMessage(Message &message, size_t &dataSize);
    std::shared_ptr<Message> DecodeMessage(const MessageHeader &msgHeader, size_t msgSize, size_t &decodeSize) override;

private:
    ssize_t Recv(void *buf, ssize_t len, int timeout) override;
    const std::string &GetSessionId() const override {
        return networkId_;
    }
    std::string networkId_ {};
    void *data_ { nullptr };
    size_t len_ { 0 };
    size_t cursor_ { 0 };
};
}  // namespace OHOS::ObjectStore
#endif