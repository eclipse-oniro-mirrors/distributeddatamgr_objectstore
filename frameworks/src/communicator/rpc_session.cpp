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

#include "rpc_session.h"
#include "logger.h"
#include "securec.h"

namespace OHOS::ObjectStore {
ssize_t RPCSession::Recv(void *buf, ssize_t len, [[maybe_unused]] int timeout)
{
    if (cursor_ >= len_) {
        return 0;
    }
    size_t readSize = std::min(static_cast<size_t>(len), len_ - cursor_);
    if (memcpy_s(buf, len, data_, readSize) != EOK) {
        return 0;
    }
    cursor_ += readSize;
    return readSize;
}

const uint8_t *RPCSession::EncodeMessage(Message &message, size_t &dataSize)
{
    dataSize = message.GetDataSize();
    return message.GetData();
}

std::shared_ptr<Message> RPCSession::DecodeMessage(const MessageHeader &msgHeader, size_t msgSize, size_t &decodeSize)
{
    if (msgSize < msgHeader.dataSize) {
        decodeSize = 0;
        return nullptr;
    }
    decodeSize = msgHeader.dataSize;
    return MessageDecoder::Decode(reinterpret_cast<const uint8_t *>(&msgHeader), decodeSize);
}
}  // namespace OHOS::ObjectStore