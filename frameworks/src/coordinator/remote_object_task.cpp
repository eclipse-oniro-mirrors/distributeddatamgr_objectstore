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

#include "remote_object_task.h"
#include "delete_message.h"
#include "get_message.h"
#include "object_utils.h"
#include "objectstore_errors.h"
#include "put_message.h"
#include "string_utils.h"

namespace OHOS::ObjectStore {
namespace {
constexpr uint32_t TASK_MERGE_LIMIT = 2;  // 2 : merge at least two task
constexpr uint32_t TASK_LOOP_PERIOD = 10;  // 10 milliseconds
}  // namespace
std::atomic<message_t> RemoteObjectTask::idGenerator_ = 1;
uint32_t RemoteObjectTask::Init()
{
    communicator_.RegisterMessageHandler(MessageType::PUT_RESPONSE, this);
    communicator_.RegisterMessageHandler(MessageType::GET_RESPONSE, this);
    communicator_.RegisterMessageHandler(MessageType::DELETE_RESPONSE, this);
    stoptask_ = false;
    taskThreadPtr_ = std::make_unique<std::thread>(std::bind(&RemoteObjectTask::TaskLoop, this));
    if (taskThreadPtr_ == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to create task thread", __func__);
        return ERR_NOMEM;
    }
    return SUCCESS;
}

void RemoteObjectTask::Destory()
{
    communicator_.UnregisterMessageHandler(MessageType::PUT_RESPONSE, this);
    communicator_.UnregisterMessageHandler(MessageType::GET_RESPONSE, this);
    communicator_.UnregisterMessageHandler(MessageType::DELETE_RESPONSE, this);
    stoptask_ = true;
    if (taskThreadPtr_ != nullptr && taskThreadPtr_->joinable()) {
        taskCV_.notify_one();
        taskThreadPtr_->join();
    }
}

void RemoteObjectTask::MergeTask(TaskQueue &taskQueue)
{
    if (taskQueue.size() < TASK_MERGE_LIMIT) {
        return;
    }
    auto dst = taskQueue.front();
    taskQueue.pop_front();
    while (!taskQueue.empty()) {
        auto src = taskQueue.front();
        if (dst->Merge(src) == SUCCESS) {
            taskQueue.pop_front();
        } else {
            break;
        }
    }
    taskQueue.push_front(dst);
}

void RemoteObjectTask::TaskLoop()
{
    while (!stoptask_) {
            std::unique_lock<std::mutex> lock(taskMutex_);
            for (auto ite = tasks_.begin(); ite != tasks_.end();) {
                auto &deviceChange = ite->second;
                for (auto changeIte = deviceChange.begin(); changeIte != deviceChange.end();) {
                    auto &taskQueue = changeIte->second;
                    MergeTask(taskQueue);
                    while (!taskQueue.empty()) {
                        auto task = taskQueue.front();
                        lock.unlock();
                        auto ret = DoRemoteTask(changeIte->first, task);
                        lock.lock();
                        if (ret == SUCCESS) {
                            taskQueue.pop_front();
                        } else {
                            break;
                        }
                    }
                    if (taskQueue.empty()) {
                        changeIte = deviceChange.erase(changeIte);
                    } else {
                        changeIte++;
                    }
                }
                if (deviceChange.empty()) {
                    ite = tasks_.erase(ite);
                } else {
                    ite++;
                }
            }
            if (tasks_.empty()) {
                taskCV_.wait(lock);
            } else {
                taskCV_.wait_for(lock, std::chrono::milliseconds(TASK_LOOP_PERIOD));
        }
    }
}

uint32_t RemoteObjectTask::DoRemoteTask(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    auto type = changes->GetOperationType();
    auto ret = SUCCESS;
    switch (type) {
        case OperationType::PUT_OPERATION:
            ret = PutRemote(key, changes);
            break;
        case OperationType::DELETE_OPERATION:
            ret = DeleteRemote(key);
            break;
        default:
            LOG_ERROR("RemoteObjectTask-%s: unsupport operation type", __func__);
            ret = ERR_INVAL;
            break;
    }
    return ret;
}

uint32_t RemoteObjectTask::PutRemote(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    uint32_t size = changes->GetDataSize();
    std::vector<uint8_t> operation;
    operation.reserve(size);
    uint8_t *ptr = changes->Encode();
    if (ptr == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to encode", __func__);
        return ERR_INVAL;
    }
    operation.assign(ptr, ptr + size);
    std::shared_ptr<Message> request = std::make_shared<PutRequest>(idGenerator_++, std::move(operation));
    std::shared_ptr<Message> response = nullptr;
    auto ret = SendMessage(GetObjectHostId(key), request, response);
    if (ret != SUCCESS || response == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to send request (%d)", __func__, ret);
        return ret;
    }
    PutResponse *putResponse = reinterpret_cast<PutResponse *>(response.get());
    ret = putResponse->status;
    if (ret != SUCCESS) {
        LOG_ERROR("RemoteObjectTask-%s: put remote object fail (%d)", __func__, ret);
    }
    return ret;
}

uint32_t RemoteObjectTask::PutRemoteAsync(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    AddRemoteTask(key, changes);
    taskCV_.notify_one();
    LOG_INFO("RemoteObjectTask-%s: success add put remote async task", __func__);
    return SUCCESS;
}

uint32_t RemoteObjectTask::AddRemoteTask(const Bytes &key, std::shared_ptr<Operation> &changes)
{
    std::unique_lock<std::mutex> lock(taskMutex_);
    auto targetDevice = GetObjectHostId(key);
    auto deviceIte = tasks_.find(targetDevice);
    if (deviceIte == tasks_.end()) {
        TaskQueue changeQueue;
        changeQueue.push_back(changes);
        std::map<Bytes, TaskQueue> changeMap;
        changeMap.emplace(key, changeQueue);
        tasks_.emplace(targetDevice, changeMap);
        return SUCCESS;
    }
    auto changeIte = deviceIte->second.find(key);
    if (changeIte == deviceIte->second.end()) {
        TaskQueue changeQueue;
        changeQueue.push_back(changes);
        deviceIte->second.emplace(key, changeQueue);
        return SUCCESS;
    }
    changeIte->second.push_back(changes);
    return SUCCESS;
}

uint32_t RemoteObjectTask::GetFromLocal(const Bytes &key, std::shared_ptr<Operation> &values)
{
    auto ret = values->Execute();
    if (ret != SUCCESS) {
        LOG_INFO("RemoteObjectTask-%s: Do not exist in local", __func__);
        return ret;
    }
    std::any valueParam;
    values->GetOperationValue(valueParam);
    if (!valueParam.has_value()) {
        LOG_INFO("RemoteObjectTask-%s: Do not have value", __func__);
        return ERR_NOENT;
    }
    auto &mapValue = std::any_cast<const std::map<Bytes, Bytes> &>(valueParam);
    if (mapValue.empty()) {
        LOG_WARN("RemoteObjectTask-%s: Local value is empty", __func__);
        return ERR_NOENT;
    }
    return SUCCESS;
}

uint32_t RemoteObjectTask::DoGetRemote(const Bytes &key, std::shared_ptr<Operation> &values)
{
    uint32_t size = values->GetDataSize();
    std::vector<uint8_t> operation;
    operation.reserve(size);
    uint8_t *ptr = values->Encode();
    if (ptr == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to encode", __func__);
        return ERR_INVAL;
    }
    operation.assign(ptr, ptr + size);
    std::shared_ptr<Message> request = std::make_shared<GetRequest>(idGenerator_++, std::move(operation));
    std::shared_ptr<Message> response = nullptr;
    char* tmp = new char[key.size() + 1];
    memcpy_s(tmp, key.size(), key.data(), key.size());
    tmp[key.size()] = 0;
    LOG_ERROR("hanlu send get: %s", tmp);
    auto ret = SendMessage(GetObjectHostId(key), request, response);
    if (ret != SUCCESS || response == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to send request (%d)", __func__, ret);
        return ret;
    }
    GetResponse *getResponse = reinterpret_cast<GetResponse *>(response.get());
    std::shared_ptr<PutOperation> putOperation =
        std::make_shared<PutOperation>(getResponse->operationData.data(), getResponse->operationLen);
    if (putOperation == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to create operation", __func__);
        return ERR_NOMEM;
    }
    ret = putOperation->Execute();
    if (ret != SUCCESS) {
        LOG_ERROR("RemoteObjectTask-%s: fail to execute response operation", __func__);
        return ERR_INVALMSG;
    }
    return values->Execute();
}

uint32_t RemoteObjectTask::GetRemote(const Bytes &key, std::shared_ptr<Operation> &values)
{
    auto ret = GetFromLocal(key, values);
    if (ret == SUCCESS) {
        LOG_INFO("RemoteObjectTask-%s: Get from local", __func__);
        return ret;
    }
    LOG_INFO("RemoteObjectTask-%s: Get remote object directly", __func__);
    return DoGetRemote(key, values);
}

uint32_t RemoteObjectTask::DeleteRemote(const Bytes &key)
{
    std::shared_ptr<Operation> deleteOperation =
        std::make_shared<DeleteOperation>(DataType::MAP_TYPE, const_cast<Bytes &>(key));
    if (deleteOperation == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to create operation", __func__);
        return ERR_NOMEM;
    }
    uint32_t size = deleteOperation->GetDataSize();
    std::vector<uint8_t> operationData;
    operationData.reserve(size);
    uint8_t *ptr = deleteOperation->Encode();
    if (ptr == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to encode operation", __func__);
        return ERR_INVAL;
    }
    operationData.assign(ptr, ptr + size);
    std::shared_ptr<Message> request = std::make_shared<DeleteRequest>(idGenerator_++, operationData);
    std::shared_ptr<Message> response = nullptr;
    auto ret = SendMessage(GetObjectHostId(key), request, response);
    if (ret != SUCCESS || response == nullptr) {
        LOG_ERROR("RemoteObjectTask=%s: fail to send request (%d)", __func__, ret);
        return ret;
    }
    DeleteResponse *deleteResponse = reinterpret_cast<DeleteResponse *>(response.get());
    return deleteResponse->response;
}

uint32_t RemoteObjectTask::DeleteRemoteAsync(const Bytes &key)
{
    std::shared_ptr<Operation> deleteOperation =
        std::make_shared<DeleteOperation>(DataType::MAP_TYPE, const_cast<Bytes &>(key));
    if (deleteOperation == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail to create operation", __func__);
        return ERR_NOMEM;
    }
    AddRemoteTask(key, deleteOperation);
    taskCV_.notify_one();
    LOG_INFO("RemoteObjectTask-%s: success add delete remote async task", __func__);
    return SUCCESS;
}

uint32_t RemoteObjectTask::SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: invalid message", __func__);
        return ERR_INVALMSG;
    }
    return communicator_.SendMessage(deviceId, message);
}

uint32_t RemoteObjectTask::SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &request,
                                       std::shared_ptr<Message> &response)
{
    LOG_ERROR("RemoteObjectTask-%s: send %s", __func__, deviceId.c_str());
    auto ret = SendMessage(deviceId, request);
    if (ret != SUCCESS) {
        return ret;
    }
    if (request->GetTimeout() == 0) {
        LOG_ERROR("RemoteObjectTask-%s: wait reponse need assign time out", __func__);
        return ERR_INVALMSG;
    }
    auto waiter = std::make_shared<MessageWaiter>();
    if (waiter == nullptr) {
        LOG_ERROR("RemoteObjectTask-%s: fail create message waiter", __func__);
        return ERR_NOMEM;
    }
    {
        std::unique_lock<std::mutex> lock(waiterMutex_);
        waiters_.emplace(request->GetId(), waiter);
    }
    ret = waiter->Wait(request->GetTimeout(), response);
    {
        std::unique_lock<std::mutex> lock(waiterMutex_);
        waiters_.erase(request->GetId());
    }
    if (ret != SUCCESS) {
        LOG_ERROR("RemoteObjectTask-%s: end send request (%d) fail", __func__, request->GetId());
    }
    return ret;
}

void RemoteObjectTask::OnNewMessage(const std::shared_ptr<Message> &message)
{
    if (message == nullptr) {
        return;
    }
    std::unique_lock<std::mutex> lock(waiterMutex_);
    auto it = waiters_.find(message->GetRequestId());
    if (it != waiters_.end()) {
        it->second->Signal(message);
    }
}

device_t RemoteObjectTask::GetObjectHostId(const Bytes &key)
{
    return ObjectUtils::GetObjectHost(std::string(key.begin(), key.end()));
}
}  // namespace OHOS::ObjectStore