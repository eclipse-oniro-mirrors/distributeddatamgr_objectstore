//
// Created by h00284894 on 2021/12/8.
//

#ifndef JS_OBJECT_WRAPPER_H
#define JS_OBJECT_WRAPPER_H

#include <shared_mutex>
#include <distributed_objectstore.h>
#include "distributed_object.h"
#include "js_watcher.h"

namespace OHOS::ObjectStore {
class JSObjectWrapper {
public:
    JSObjectWrapper(DistributedObjectStore *objectStore, DistributedObject *object);
    virtual ~JSObjectWrapper();
    DistributedObject *GetObject();
    void AddWatch(napi_env env, const char *type, napi_value handler);
    void DeleteWatch(napi_env env, const char *type, napi_value handler = nullptr);
private:
    DistributedObjectStore *objectStore_;
    DistributedObject *object_;
    std::shared_ptr<JSWatcher> watcher_ = nullptr;
    std::shared_mutex watchMutex_ {};
};
}

#endif //HANLU_JS_OBJECT_WRAPPER_H
