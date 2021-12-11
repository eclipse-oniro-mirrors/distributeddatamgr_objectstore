//
// Created by h00284894 on 2021/12/8.
//

#include <logger.h>
#include "js_object_wrapper.h"

namespace OHOS::ObjectStore {
JSObjectWrapper::JSObjectWrapper(DistributedObjectStore *objectStore, DistributedObject *object) : objectStore_(
        objectStore), object_(object) {}

JSObjectWrapper::~JSObjectWrapper() {
    LOG_INFO("JSObjectWrapper::~JSObjectWrapper");
    std::unique_lock<std::shared_mutex> cacheLock(watchMutex_);
    if (watcher_ != nullptr) {
        watcher_ = nullptr;
    }
    LOG_INFO("JSObjectWrapper::~JSObjectWrapper end");
}

DistributedObject *JSObjectWrapper::GetObject() {
    return object_;
}

void JSObjectWrapper::AddWatch(napi_env env, const char *type, napi_value handler) {
    std::unique_lock<std::shared_mutex> cacheLock(watchMutex_);
    if (watcher_ == nullptr) {
        watcher_ = std::make_shared<JSWatcher>(env, objectStore_, object_);
    }
    watcher_->On(type, handler);
    LOG_INFO("JSObjectWrapper::AddWatch %s", type);
}

void JSObjectWrapper::DeleteWatch(napi_env env, const char *type, napi_value handler) {
    std::unique_lock<std::shared_mutex> cacheLock(watchMutex_);
    if (watcher_ != nullptr) {
        watcher_->Off(type, handler);
        LOG_INFO("JSObjectWrapper::DeleteWatch %s", type);
    }
    LOG_ERROR("JSObjectWrapper::DeleteWatch watcher_ is null");
}
}