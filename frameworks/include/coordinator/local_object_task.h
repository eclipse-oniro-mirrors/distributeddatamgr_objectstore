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

#ifndef LOCAL_OBJECT_TASK_H
#define LOCAL_OBJECT_TASK_H

#include "communicator.h"

namespace OHOS::ObjectStore {
class LocalObjectTask final : public MessageHandler {
public:
    LocalObjectTask(Communicator &communicator) : communicator_(communicator)
    {}
    ~LocalObjectTask() = default;
    uint32_t Init();
    void Destory();

private:
    void OnNewMessage(const std::shared_ptr<Message> &message) override;
    uint32_t SendMessage(const device_t &deviceId, const std::shared_ptr<Message> &message);
    void HandlePutRequest(const std::shared_ptr<Message> &message);
    void HandleGetRequest(const std::shared_ptr<Message> &message);
    void HandleDeleteRequest(const std::shared_ptr<Message> &message);
    Communicator &communicator_;
    static std::atomic<message_t> idGenerator_;
};
}  // namespace OHOS::ObjectStore

#endif
