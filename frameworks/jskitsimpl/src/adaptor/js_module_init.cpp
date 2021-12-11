//
// Created by h00284894 on 2021/12/11.
//

#include <js_distributedobject.h>
#include <js_distributedobjectstore.h>
#include <cstring>
#include "native_api.h"
#include "native_node_api.h"

using namespace OHOS::ObjectStore;
static napi_value  DistributeddataobjectExport(napi_env env, napi_value exports)
{
    const char* objectClassName = "DistributedDataObject";
    napi_value objectClass = nullptr;
    static napi_property_descriptor objectDesc[] = {
            DECLARE_NAPI_FUNCTION("createObject", JSDistributedObjectStore::JSCreateObject),
            DECLARE_NAPI_FUNCTION("createObjectSync", JSDistributedObjectStore::JSCreateObjectSync),
            DECLARE_NAPI_FUNCTION("destroyObject", JSDistributedObjectStore::JSDestroyObject),
            DECLARE_NAPI_FUNCTION("destroyObjectSync",JSDistributedObjectStore::JSDestroyObjectSync),
            DECLARE_NAPI_FUNCTION("sync", JSDistributedObjectStore::JSSync),
            DECLARE_NAPI_FUNCTION("on", JSDistributedObjectStore::JSOn),
            DECLARE_NAPI_FUNCTION("off", JSDistributedObjectStore::JSOff),
    };
    napi_define_class(env, objectClassName, strlen(objectClassName), JSDistributedObjectStore::JSConstructor, nullptr,
                      sizeof(objectDesc) / sizeof(objectDesc[0]), objectDesc, &objectClass);

    const char* distributedObjectName = "DistributedObject";
    napi_value distributedObjectClass = nullptr;
    static napi_property_descriptor distributedObjectDesc[] = {
            DECLARE_NAPI_FUNCTION("put", OHOS::ObjectStore::JSDistributedObject::JSPut),
            DECLARE_NAPI_FUNCTION("get", OHOS::ObjectStore::JSDistributedObject::JSGet),
    };
    napi_define_class(env, distributedObjectName, strlen(distributedObjectName), OHOS::ObjectStore::JSDistributedObject::JSConstructor, nullptr,
                      sizeof(distributedObjectDesc) / sizeof(distributedObjectDesc[0]), distributedObjectDesc, &distributedObjectClass);

    static napi_property_descriptor desc[] = {
            DECLARE_NAPI_PROPERTY("DistributedDataObject", objectClass),
            DECLARE_NAPI_PROPERTY("DistributedObject", distributedObjectClass),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

// storage module define
static napi_module storageModule = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = DistributeddataobjectExport,
        .nm_modname = "distributeddataobject",
        .nm_priv = ((void*)0),
        .reserved = { 0 },
};

// distributeddataobject module register
extern "C" __attribute__((constructor)) void DistributeddataobjectRegister()
{
    napi_module_register(&storageModule);
}