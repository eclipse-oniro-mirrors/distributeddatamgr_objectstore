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

#include "local_object_task.h"
#include "delete_message.h"
#include "get_message.h"
#include "object_coordinator.h"
#include "objectstore_errors.h"
#include "operation.h"
#include "put_message.h"

namespace OHOS::ObjectStore {
std::atomic<message_t> LocalObjectTask::idGenerator_ { 1 };
uint32_t LocalObjectTask::Init()
{
    communicator_.RegisterMessageHandler(MessageType::PUT_REQUEST, this);
    communicator_.RegisterMessageHandler(MessageType::GET_REQUEST, this);
    communicator_.RegisterMessageHandler(MessageType::DELETE_REQUEST, this);
    return SUCCESS;
}

void LocalObjectTask::Destory()
{
    communicator_.UnregisterMessageHandler(MessageType::PUT_REQUEST, this);
    communicator_.UnregisterMessageHandler(MessageType::GET_REQUEST, this);
    communicator_.UnregisterMessageHandler(MessageType::DELETE_REQUEST, this);
}

void LocalObjectTask::HandlePutRequest(const std::shared_ptr<Message> &message)
{
    PutRequest *putRequest = reinterpret_cast<PutRequest *>(message.get());
    std::vector<uint8_t> operationData(putRequest->operation);
    uint32_t dataSize = putRequest->operationLen;
    std::shared_ptr<Operation> operation = std::make_shared<PutOperation>(operationData.data(), dataSize);
    if (operation == nullptr) {
        LOG_ERROR("LocalObjectTask-%s: fail to create operation", __func__);
        return;
    }
    auto ret = operation->Execute();

    Bytes key;
    operation->GetOperationKey(key);
    std::any value;
    operation->GetOperationValue(value);
    const std::map<Bytes, Bytes> &mapValue = std::any_cast<const std::map<Bytes, Bytes> &>(value);
    ObjectCoordinator::GetInstance().Publish(key, mapValue);
    LOG_DEBUG("LocalObjectTask-%s: notify to observers", __func__);
    std::shared_ptr<Message> response = std::make_shared<PutResponse>(idGenerator_++, putRequest->GetId(), ret);
    std::string deviceId;
    message->GetSource(deviceId);
    ret = SendMessage(deviceId, response);
    if (ret != SUCCESS) {
        LOG_ERROR("LocalObjectTask-%s: fail to send response", __func__);
    }
}

void LocalObjectTask::HandleGetRequest(const std::shared_ptr<Message> &message)
{
    GetRequest *getRequest = reinterpret_cast<GetRequest *>(message.get());
    std::vector<uint8_t> operationData(getRequest->operationData);
    uint32_t dataSize = getRequest->operationLen;
    std::shared_ptr<Operation> getOperation = std::make_shared<GetOperation>(operationData.data(), dataSize);
    if (getOperation == nullptr) {
        LOG_ERROR("LocalObjectTask-%s: fail to create operation", __func__);
        return;
    }
    getOperation->Execute();
    std::any value;
    getOperation->GetOperationValue(value);
    Bytes key;
    getOperation->GetOperationKey(key);
    char* tmp = new char[key.size() + 1];
    memcpy_s(tmp, key.size(), key.data(), key.size());
    tmp[key.size()] = 0;
    LOG_ERROR("hanlu get: %s", tmp);
    std::shared_ptr<Operation> putOperation = std::make_shared<PutOperation>(getOperation->GetDataType(), key, value);
    if (putOperation == nullptr) {
        LOG_ERROR("LocalObjectTask-%s: fail to create operation", __func__);
        return;
    }
    auto size = putOperation->GetDataSize();
    std::vector<uint8_t> operation;
    operation.reserve(size);
    uint8_t *ptr = putOperation->Encode();
    if (ptr == nullptr) {
        LOG_ERROR("LocalObjectTask-%s: fail to encode", __func__);
        return;
    }
    operation.assign(ptr, ptr + size);
    std::shared_ptr<Message> response =
        std::make_shared<GetResponse>(idGenerator_++, getRequest->GetId(), std::move(operation));
    std::string deviceId;
    message->GetSource(deviceId);
    auto ret = SendMessage(deviceId, response);
    if (ret != SUCCESS) {
        LOG_ERROR("LocalObjectTask-%s: fail to send response", __func__);
    }
}

void LocalObjectTask::HandleDeleteRequest(const std::shared_ptr<Message> &message)
{
    DeleteRequest *deleteRequest = reinterpret_cast<DeleteRequest *>(message.get());
    std::shared_ptr<Operation> deleteOperation =
        std::make_shared<DeleteOperation>(deleteRequest->key.data(), deleteRequest->keyLen);
    if (deleteOperation == nullptr) {
        LOG_ERROR("LocalObjectTask-%s: fail to create operation", __func__);
        return;
    }
    uint32_t ret = deleteOperation->Execute();
    std::shared_ptr<Message> response = std::make_shared<DeleteResponse>(idGenerator_++, deleteRequest->GetId(), ret);
    std::string deviceId;
    message->GetSource(deviceId);
    ret = SendMessage(deviceId, response);
    if (ret != SUCCESS) {
        LOG_ERROR("LocalObjectTask-%s: fail to send response", __func__);
    }
    LOG_INFO("LocalObjectTask-%s: delete success", __func__);
    Bytes key;
    deleteOperation->GetOperationKey(key);
    ObjectCoordinator::GetInstance().PublishDelete(key);
}

void LocalObjectTask::OnNewMessage(const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        return;
    }
    switch (message->GetType()) {
        case MessageType::PUT_REQUEST:
            HandlePutRequest(message);
            break;
        case MessageType::GET_REQUEST:
            HandleGetRequest(message);
            break;
        case MessageType::DELETE_REQUEST:
            HandleDeleteRequest(message);
            break;
        default:
            break;
    }
}

uint32_t LocalObjectTask::SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        LOG_ERROR("LocalObjectTask-%s: invalid message", __func__);
        return ERR_INVALMSG;
    }
    return communicator_.SendMessage(deviceId, message);
}
}  // namespace OHOS::ObjectStore