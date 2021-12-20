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

#include <js_object_wrapper.h>
#include <js_distributedobjectstore.h>
#include <js_common.h>
#include "js_distributedobject.h"
#include <logger.h>

namespace OHOS::ObjectStore {
constexpr size_t KEY_SIZE = 64;
constexpr size_t STRING_MAX_SIZE = 10240;
napi_value JSDistributedObject::JSConstructor(napi_env env, napi_callback_info info) {
    napi_value thisVar = nullptr;
    void* data = nullptr;
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    NAPI_ASSERT(env, argc == 1, "requires 1 parameter");

    ConstructContext *context = nullptr;
    status = napi_unwrap(env, argv[0], (void**)&context);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    NAPI_ASSERT(env, context != nullptr, "ConstructContext is null");
    JSObjectWrapper *objectWrapper = new JSObjectWrapper(context->objectStore, context->object);
    status = napi_wrap( env, thisVar, objectWrapper,
               [](napi_env env, void* data, void* hint) {
                   auto objectWrapper = (JSObjectWrapper*)data;
                   if (objectWrapper != nullptr) {
                       delete objectWrapper;
                   }
               },
               nullptr, nullptr);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    return thisVar;
}

// get(key: string): ValueType;
napi_value JSDistributedObject::JSGet(napi_env env, napi_callback_info info) {
    size_t requireArgc = 1;
    size_t argc = 0;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    char key[KEY_SIZE] = { 0 };
    size_t keyLen = 0;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 1 parameter");
    status = napi_get_value_string_utf8(env, argv[0], key, KEY_SIZE, &keyLen);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    JSObjectWrapper *wrapper = nullptr;
    status = napi_unwrap(env, thisVar, (void**)&wrapper);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    NAPI_ASSERT(env, wrapper != nullptr, "object wrapper is null");
    // todo wrapper->GetObject()->
    napi_value result = nullptr;
    return result;
}

// put(key: string, value: ValueType): void;
napi_value JSDistributedObject::JSPut(napi_env env, napi_callback_info info) {
    size_t requireArgc = 2;
    size_t argc = 0;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    char key[KEY_SIZE] = { 0 };
    size_t keyLen = 0;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");
    status = napi_get_value_string_utf8(env, argv[0], key, KEY_SIZE, &keyLen);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    napi_valuetype valueType;
    status = napi_typeof(env, argv[1], &valueType);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    JSObjectWrapper *wrapper = nullptr;
    status = napi_unwrap(env, thisVar, (void**)&wrapper);
    CHECK_EQUAL_WITH_RETURN_NULL(status, napi_ok);
    NAPI_ASSERT(env, wrapper != nullptr, "object wrapper is null");
    DoPut(nullptr, wrapper, key, keyLen, valueType, argv[1]);
    LOG_INFO("put %s success", key);
    return nullptr;
}

void JSDistributedObject::DoPut(napi_env env, JSObjectWrapper *wrapper, char *key, size_t len, napi_valuetype type,
                                napi_value value) {
    std::string keyString = key;
    switch (type) {
        case napi_boolean: {
            bool putValue = false;
            napi_status status = napi_get_value_bool(env, value, &putValue);
            CHECK_EQUAL_WITH_RETURN_VOID(status, napi_ok);
            wrapper->GetObject()->PutBoolean(keyString, putValue);
            break;
        }
        case napi_number: {
            double putValue = false;
            napi_status status = napi_get_value_double(env, value, &putValue);
            CHECK_EQUAL_WITH_RETURN_VOID(status, napi_ok);
            wrapper->GetObject()->PutDouble(keyString, putValue);
            break;
        }
        case napi_string: {
            size_t size = 0;
            napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &size);
            CHECK_EQUAL_WITH_RETURN_VOID(status, napi_ok);
            ASSERT_MATCH_ELSE_RETURN_VOID(size > 0 && size < STRING_MAX_SIZE);
            char *stringBuf = new char[size];
            status = napi_get_value_string_utf8(env, value, stringBuf, size, &size);
            CHECK_EQUAL_WITH_RETURN_VOID(status, napi_ok);
            wrapper->GetObject()->PutString(keyString, stringBuf);
            delete[] stringBuf;
            break;
        }
        default: {
            LOG_ERROR("error type! %d", type);
            break;
        }

    }
}
}