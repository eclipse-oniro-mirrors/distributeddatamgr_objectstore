//
// Created by h00284894 on 2021/12/8.
//

#ifndef JS_DISTRIBUTEDOBJECT_H
#define JS_DISTRIBUTEDOBJECT_H


#include <js_native_api.h>
namespace OHOS::ObjectStore {
class JSDistributedObject {
public:
    static napi_value JSConstructor(napi_env env, napi_callback_info info);
    static napi_value JSGet(napi_env env, napi_callback_info info);
    static napi_value JSPut(napi_env env, napi_callback_info info);
};
}



#endif //HANLU_JS_DISTRIBUTEDOBJECT_H
