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

#ifndef COMMUNICATOR_SESSION_H
#define COMMUNICATOR_SESSION_H

#include <sys/types.h>
#include "message_decoder.h"

namespace OHOS::ObjectStore {
class CommunicatorSession {
public:
    CommunicatorSession() = default;
    virtual ~CommunicatorSession() = default;
    virtual ssize_t Recv(void *buf, ssize_t len, int timeout) = 0;
    virtual const std::string &GetSessionId() const = 0;
    virtual std::shared_ptr<Message> DecodeMessage(const MessageHeader &msgHeader, size_t msgSize,
        size_t &decodeSize) = 0;

    void SetDataBuffer(void *dataBuffer)
    {
        dataBuffer_ = dataBuffer;
    }

    const void *GetDataBuffer() const
    {
        return dataBuffer_;
    }

protected:
    void *dataBuffer_ { nullptr };
};
}  // namespace OHOS::ObjectStore
#endif