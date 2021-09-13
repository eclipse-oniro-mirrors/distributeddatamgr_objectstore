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
#ifndef MESSAGE_WAITER_H
#define MESSAGE_WAITER_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include "message.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
class MessageWaiter final {
public:
    uint32_t Wait(uint32_t timeout, std::shared_ptr<Message> &message)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, std::chrono::milliseconds(timeout)) == std::cv_status::timeout) {
            message = nullptr;
            return -1;
        };
        message = message_;
        return SUCCESS;
    }

    void Signal(const std::shared_ptr<Message> &message)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        message_ = message;
        cv_.notify_one();
    }

private:
    std::mutex mutex_ {};
    std::condition_variable cv_ {};
    std::shared_ptr<Message> message_ { nullptr };
};
}  // namespace OHOS::ObjectStore
#endif