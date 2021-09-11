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

#include "message_decoder.h"
#include "delete_message.h"
#include "get_message.h"
#include "observe_message.h"
#include "publish_message.h"
#include "publish_delete_message.h"
#include "put_message.h"

namespace OHOS::ObjectStore {
std::shared_ptr<Message> MessageDecoder::Decode(const uint8_t *data, uint32_t dataSize)
{
    if (data == nullptr) {
        return nullptr;
    }
    std::shared_ptr<Message> message;
    message = DecodeRequest(data, dataSize);
    if (message == nullptr) {
        message = DecodeResponse(data, dataSize);
    }
    if (message == nullptr) {
        return nullptr;
    }
    if (message->GetDataSize() != dataSize) {
        LOG_WARN("MessageDecoder-%s: Failed to decode, error: msg %u size != data %u size", __func__,
                 message->GetDataSize(), dataSize);
    }
    return message;
}

std::shared_ptr<Message> MessageDecoder::DecodeRequest(const uint8_t *data, uint32_t dataSize)
{
    MessageType type = reinterpret_cast<const MessageHeader *>(data)->type;
    switch (type) {
        case MessageType::OBSERVE_REQUEST:
            return std::make_shared<ObserveRequest>(data, dataSize);
        case MessageType::REMOVE_OBSERVER_REQUEST:
            return std::make_shared<RemoveObserverRequest>(data, dataSize);
        case MessageType::PUT_REQUEST:
            return std::make_shared<PutRequest>(data, dataSize);
        case MessageType::GET_REQUEST:
            return std::make_shared<GetRequest>(data, dataSize);
        case MessageType::PUBLISH_REQUEST:
            return std::make_shared<PublishRequest>(data, dataSize);
        case MessageType::PUBLISH_DELETE_REQUEST:
            return std::make_shared<PublishDeleteRequest>(data, dataSize);
        case MessageType::DELETE_REQUEST:
            return std::make_shared<DeleteRequest>(data, dataSize);
        default:
            return nullptr;
    }
}

std::shared_ptr<Message> MessageDecoder::DecodeResponse(const uint8_t *data, uint32_t dataSize)
{
    MessageType type = reinterpret_cast<const MessageHeader *>(data)->type;
    switch (type) {
        case MessageType::OBSERVE_RESPONSE:
            return std::make_shared<ObserveResponse>(data, dataSize);
        case MessageType::PUT_RESPONSE:
            return std::make_shared<PutResponse>(data, dataSize);
        case MessageType::GET_RESPONSE:
            return std::make_shared<GetResponse>(data, dataSize);
        case MessageType::DELETE_RESPONSE:
            return std::make_shared<DeleteResponse>(data, dataSize);
        default:
            return nullptr;
    }
}
}  // namespace OHOS::ObjectStore