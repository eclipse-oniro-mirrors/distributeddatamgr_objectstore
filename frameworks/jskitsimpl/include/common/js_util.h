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
#ifndef OHOS_JS_UTIL_H
#define OHOS_JS_UTIL_H
#include <cstdint>
#include <map>
#include <variant>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::ObjectStore {

class JSUtil final {
public:
    enum {
        /* Blob's first byte is the blob's data ValueType */
        STRING = 0,
        INTEGER = 1,
        FLOAT = 2,
        BYTE_ARRAY = 3,
        BOOLEAN = 4,
        DOUBLE = 5,
        INVALID = 255
    };

    /* for query value related : number|string|boolean */
    using QueryVariant = std::variant<std::string, bool, double>;

    /* napi_value <-> bool */
    static napi_status GetValue(napi_env env, napi_value in, bool& out);
    static napi_status SetValue(napi_env env, const bool& in, napi_value& out);

    /* napi_value <-> int32_t */
    static napi_status GetValue(napi_env env, napi_value in, int32_t& out);
    static napi_status SetValue(napi_env env, const int32_t& in, napi_value& out);

    /* napi_value <-> uint32_t */
    static napi_status GetValue(napi_env env, napi_value in, uint32_t& out);
    static napi_status SetValue(napi_env env, const uint32_t& in, napi_value& out);

    /* napi_value <-> int64_t */
    static napi_status GetValue(napi_env env, napi_value in, int64_t& out);
    static napi_status SetValue(napi_env env, const int64_t& in, napi_value& out);

    /* napi_value <-> double */
    static napi_status GetValue(napi_env env, napi_value in, double& out);
    static napi_status SetValue(napi_env env, const double& in, napi_value& out);

    /* napi_value <-> std::string */
    static napi_status GetValue(napi_env env, napi_value in, std::string& out);
    static napi_status SetValue(napi_env env, const std::string& in, napi_value& out);


    /* napi_value <-> QueryVariant */
    static napi_status GetValue(napi_env env, napi_value in, QueryVariant& out);
    static napi_status SetValue(napi_env env, const QueryVariant& in, napi_value& out);

    /* napi_value <-> std::vector<std::string> */
    static napi_status GetValue(napi_env env, napi_value in, std::vector<std::string>& out);
    static napi_status SetValue(napi_env env, const std::vector<std::string>& in, napi_value& out);

    /* napi_value <-> std::vector<uint8_t> */
    static napi_status GetValue(napi_env env, napi_value in, std::vector<uint8_t>& out);
    static napi_status SetValue(napi_env env, const std::vector<uint8_t>& in, napi_value& out);

    /* napi_value <-> std::vector<int32_t> */
    static napi_status GetValue(napi_env env, napi_value in, std::vector<int32_t>& out);
    static napi_status SetValue(napi_env env, const std::vector<int32_t>& in, napi_value& out);

    /* napi_value <-> std::vector<uint32_t> */
    static napi_status GetValue(napi_env env, napi_value in, std::vector<uint32_t>& out);
    static napi_status SetValue(napi_env env, const std::vector<uint32_t>& in, napi_value& out);

    /* napi_value <-> std::vector<int64_t> */
    static napi_status GetValue(napi_env env, napi_value in, std::vector<int64_t>& out);
    static napi_status SetValue(napi_env env, const std::vector<int64_t>& in, napi_value& out);

    /* napi_value <-> std::vector<double> */
    static napi_status GetValue(napi_env env, napi_value in, std::vector<double>& out);
    static napi_status SetValue(napi_env env, const std::vector<double>& in, napi_value& out);



    /* napi_get_named_property wrapper */
    template <typename T>
    static inline napi_status GetNamedProperty(napi_env env, napi_value in, const std::string& prop, T& value)
    {
        napi_value inner = nullptr;
        napi_status status = napi_get_named_property(env, in, prop.data(), &inner);
        if ((status == napi_ok) && (inner != nullptr)) {
            return GetValue(env, inner, value);
        }
        return status;
    };

    /* napi_define_class  wrapper */
    static napi_value DefineClass(napi_env env, const std::string& name,
        const napi_property_descriptor* properties, size_t count, napi_callback newcb);

    /* napi_new_instance  wrapper */
    static napi_ref NewWithRef(napi_env env, size_t argc, napi_value* argv, void** out, napi_value constructor);

    /* napi_unwrap with napi_instanceof */
    static napi_status Unwrap(napi_env env, napi_value in, void** out, napi_value constructor);

private:
    enum {
        /* std::map<key, value> to js::tuple<key, value> */
        TUPLE_KEY = 0,
        TUPLE_VALUE,
        TUPLE_SIZE
    };
};

/* LOG_ERROR on condition related to argc/argv,  */
#define LOG_ERROR_ON_ARGS(ctxt, condition, message)                        \
    do {                                                               \
        if (!(condition)) {                                            \
            (ctxt)->status = napi_invalid_arg;                         \
            (ctxt)->error = std::string(message);                      \
            LOG_ERROR("test (" #condition ") failed: " message);           \
            return;                                                    \
        }                                                              \
    } while (0)

#define LOG_ERROR_ON_STATUS(ctxt, message)                                 \
    do {                                                               \
        if ((ctxt)->status != napi_ok) {                               \
            (ctxt)->error = std::string(message);                      \
            LOG_ERROR("test (ctxt->status == napi_ok) failed: " message);  \
            return;                                                    \
        }                                                              \
    } while (0)

#define LOG_ERROR_RETURN(condition, message, retVal)             \
    do {                                                     \
        if (!(condition)) {                                  \
            LOG_ERROR("test (" #condition ") failed: " message); \
            return retVal;                                   \
        }                                                    \
    } while (0)

#define LOG_ERROR_RETURN_VOID(condition, message)                \
    do {                                                     \
        if (!(condition)) {                                  \
            LOG_ERROR("test (" #condition ") failed: " message); \
            return;                                          \
        }                                                    \
    } while (0)
}
#endif // OHOS_JS_UTIL_H
