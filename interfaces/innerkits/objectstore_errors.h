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
#ifndef OBJECTSTORE_ERRORS_H
#define OBJECTSTORE_ERRORS_H

#include <stdint.h>

namespace OHOS::ObjectStore {
constexpr uint32_t MODULE_OBJECT_STORE = 2;
constexpr uint32_t BASE_ERR_OFFSET = 1650;

/* module defined errors */
constexpr uint32_t SUCCESS = 0;                              // Operation succeed
constexpr uint32_t ERR_STORE_MANAGER_NULL = BASE_ERR_OFFSET + 1;          // Invalid argument
constexpr uint32_t ERR_CREATE_FAIL = BASE_ERR_OFFSET + 2;          // Out of memory
constexpr uint32_t ERR_DB_NOT_INIT = BASE_ERR_OFFSET + 3;    // Database not init
constexpr uint32_t ERR_NETWORK = BASE_ERR_OFFSET + 4;       // Network error
constexpr uint32_t ERR_OPENSESSION = BASE_ERR_OFFSET + 5;   // Open session fail
constexpr uint32_t ERR_INVALMSG = BASE_ERR_OFFSET + 6;      // Invalid message
constexpr uint32_t ERR_INIT = BASE_ERR_OFFSET + 7;          // Init fail
constexpr uint32_t ERR_HASHDB_GET = BASE_ERR_OFFSET + 8;    // Get hash failed
constexpr uint32_t ERR_NOENT = BASE_ERR_OFFSET + 9;         // No such file
constexpr uint32_t ERR_EXIST = BASE_ERR_OFFSET + 10;         // Already exist
constexpr uint32_t ERR_DATA_LEN = BASE_ERR_OFFSET + 11;     // get data len failed
constexpr uint32_t ERR_KEYVAL = BASE_ERR_OFFSET + 12;     // get key value failed
constexpr uint32_t ERR_GET_OBJECT = BASE_ERR_OFFSET + 13;     // get double size failed
constexpr uint32_t ERR_OPENSESSION_REPEAT = BASE_ERR_OFFSET + 14;   // Open session fail
constexpr uint32_t ERR_MOMEM = BASE_ERR_OFFSET + 15;   // make shared fail
constexpr uint32_t ERR_DE_GETKV_FAIL = BASE_ERR_OFFSET + 16;   // KvStoreNbDelegate get KV failed
constexpr uint32_t ERR_DE_NOT_EXIST = BASE_ERR_OFFSET + 17;   //get KvStoreNbDelegate failed
constexpr uint32_t ERR_DB_GET_FAIL = BASE_ERR_OFFSET + 18;   //DB get entries failed
constexpr uint32_t ERR_DB_ENTRY_FAIL = BASE_ERR_OFFSET + 19;   //DB get entry failed
constexpr uint32_t ERR_CLOSE_STORAGE = BASE_ERR_OFFSET + 20;   //close kvStore failed
constexpr uint32_t ERR_REGISTER = BASE_ERR_OFFSET + 21;   //register observer failed
constexpr uint32_t ERR_NO_OBSERVER = BASE_ERR_OFFSET + 22;   //unregister observer not exist
constexpr uint32_t ERR_UNRIGSTER = BASE_ERR_OFFSET + 23;   //unregister observer failed
constexpr uint32_t ERR_NULL_OBJECT = BASE_ERR_OFFSET + 24;   //get object failed
constexpr uint32_t ERR_NULL_OBJECTSTORE = BASE_ERR_OFFSET + 25;   //get objectStore failed
}  // namespace OHOS::ObjectStore

#endif
