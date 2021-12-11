//
// Created by h00284894 on 2021/12/10.
//

#include <js_watcher.h>
#include <cstring>
#include <objectstore_errors.h>
#include <logger.h>
#include "native_api.h"
#include "native_node_api.h"


namespace OHOS::ObjectStore {
JSWatcher::JSWatcher(const napi_env env, DistributedObjectStore *objectStore, DistributedObject *object) : env_(env),
    objectStore_(objectStore), object_(object)
{
    std::string sessionId;
    object->GetObjectId(sessionId);
    listeners_[EVENT_CHANGE].type_ = "change";
    listeners_[EVENT_STATUS].type_ = "status";
    std::shared_ptr<WatcherImpl> watcher = std::make_shared<WatcherImpl>(this);
    uint32_t ret = objectStore->Watch(object, watcher);
    if (ret != SUCCESS) {
        LOG_ERROR("watch %s error", sessionId.c_str());
    } else {
        LOG_INFO("watch %s error", sessionId.c_str());
    }

}

JSWatcher::~JSWatcher() {
    listeners_[EVENT_CHANGE].Clear(env_);
    listeners_[EVENT_STATUS].Clear(env_);
    if (objectStore_ != nullptr) {
        objectStore_->UnWatch(object_);
    }
}

void JSWatcher::On(const char *type, napi_value handler)
{
    Event event = Find(type);
    if (event == EVENT_UNKNOWN) {
        return;
    }
    listeners_[event].Add(env_, handler);
}

void JSWatcher::Off(const char *type, napi_value handler)
{
    Event event = Find(type);
    if (event == EVENT_UNKNOWN) {
        return;
    }
    if (handler == nullptr) {
        listeners_[event].Clear(env_);
    } else {
        listeners_[event].Del(env_, handler);
    }
}

void JSWatcher::Emit(napi_value thisArg, const char *type)
{
    Event event = Find(type);
    if (event == EVENT_UNKNOWN) {
        return;
    }
    for (EventHandler* handler = listeners_[event].handlers_; handler != nullptr; handler = handler->next) {
        if (thisArg == nullptr) {
            napi_get_undefined(env_, &thisArg);
        }
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env_, handler->callbackRef, &callback);
        napi_call_function(env_, thisArg, callback, 0, nullptr, &result);
    }
}

Event JSWatcher::Find(const char *type) const
{
    Event result = EVENT_UNKNOWN;
    if (!strcmp(listeners_[EVENT_CHANGE].type_, type)) {
        result = EVENT_CHANGE;
    } else if (!strcmp(listeners_[EVENT_STATUS].type_, type)) {
        result = EVENT_STATUS;
    }
    return result;
}

EventHandler *EventListener::Find(napi_env env, napi_value handler)
{
    EventHandler* result = nullptr;
    for (EventHandler* i = handlers_; i != nullptr; i = i->next) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, i->callbackRef, &callback);
        bool isEquals = false;
        napi_strict_equals(env, handler, callback, &isEquals);
        if (isEquals) {
            result = i;
        }
    }
    return result;
}

void EventListener::Clear(napi_env env)
{
    for (EventHandler* i = handlers_; i != nullptr; i = handlers_) {
        handlers_ = i->next;
        napi_delete_reference(env, i->callbackRef);
        delete i;
    }
}

void EventListener::Del(napi_env env, napi_value handler)
{
    EventHandler* temp = nullptr;
    for (EventHandler* i = handlers_; i != nullptr; i = handlers_) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, i->callbackRef, &callback);
        bool isEquals = false;
        napi_strict_equals(env, handler, callback, &isEquals);
        if (isEquals) {
            if (temp == nullptr) {
                handlers_ = i->next;
            } else {
                temp->next = i->next;
            }
            napi_delete_reference(env, i->callbackRef);
            delete i;
        } else {
            temp = i;
        }
    }
}

void EventListener::Add(napi_env env, napi_value handler)
{
    if (Find(env, handler) != nullptr)
        return;

    if (handlers_ == nullptr) {
        handlers_ = new EventHandler();
        handlers_->next = nullptr;
    } else {
        auto temp = new EventHandler();
        temp->next = handlers_;
        handlers_ = temp;
    }
    napi_create_reference(env, handler, 1, &handlers_->callbackRef);
}

void WatcherImpl::OnChanged(const std::string &sessionid, const std::vector<const std::string> &changedData) {
    // todo watcher_->Emit();
}

void WatcherImpl::OnDeleted(const std::string &sessionid) {
    // todo watcher_->Emit();
}

WatcherImpl::WatcherImpl(JSWatcher *watcher) : watcher_(watcher) {}
}
