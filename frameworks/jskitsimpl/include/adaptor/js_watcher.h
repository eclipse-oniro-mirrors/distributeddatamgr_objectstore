//
// Created by h00284894 on 2021/12/10.
//

#ifndef JSWATCHER_H
#define JSWATCHER_H

#include <distributed_objectstore.h>
#include "native_api.h"
#include "native_node_api.h"
namespace OHOS::ObjectStore {
enum Event {
    EVENT_UNKNOWN = -1,
    EVENT_CHANGE,
    EVENT_STATUS
};
struct EventHandler {
    napi_ref callbackRef = nullptr;
    EventHandler* next = nullptr;
};
class EventListener {
public:
    EventListener() : type_(nullptr), handlers_(nullptr) {}
    virtual ~EventListener() {}
    void Add(napi_env env, napi_value handler);
    void Del(napi_env env, napi_value handler);
    void Clear(napi_env env);
    const char* type_;
    EventHandler* handlers_;
private:
    EventHandler* Find(napi_env env, napi_value handler);
};
class JSWatcher {
public:
    JSWatcher(const napi_env env, DistributedObjectStore *objectStore, DistributedObject *object);

    ~JSWatcher();
    void On(const char* type, napi_value handler);
    void Off(const char* type, napi_value handler = nullptr);
    void Emit(napi_value thisArg, const char* type);
    Event Find(const char* type) const;
private:
    napi_env env_;
    EventListener listeners_[3];
    DistributedObjectStore *objectStore_;
    DistributedObject *object_;
};

class WatcherImpl : public ObjectWatcher {
public:
    WatcherImpl(JSWatcher *watcher);

    void OnChanged(const std::string &sessionid, const std::vector<const std::string> &changedData) override;
    void OnDeleted(const std::string &sessionid) override;
private:
    JSWatcher *watcher_ = nullptr;
};
}

#endif //JSWATCHER_H
