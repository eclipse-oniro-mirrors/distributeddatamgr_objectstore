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
constexpr uint32_t ERR_INVAL = BASE_ERR_OFFSET + 1;          // Invalid argument
constexpr uint32_t ERR_NOMEM = BASE_ERR_OFFSET + 2;          // Out of memory
constexpr uint32_t ERR_CREATE_ARGS = BASE_ERR_OFFSET + 3;    // Create args failed
constexpr uint32_t ERR_DELDB = BASE_ERR_OFFSET + 4;          // Delete database failed
constexpr uint32_t ERR_STRDB_INIT = BASE_ERR_OFFSET + 5;     // Init String database failed
constexpr uint32_t ERR_STRDB_CLEAR = BASE_ERR_OFFSET + 6;    // Clear String database failed
constexpr uint32_t ERR_CLOSE_STORAGE = BASE_ERR_OFFSET + 7;  // Close storage engine failed
constexpr uint32_t ERR_DEL_KEY = BASE_ERR_OFFSET + 8;        // Delete key failed
constexpr uint32_t ERR_DB_NOT_INIT = BASE_ERR_OFFSET + 9;    // Database not init
constexpr uint32_t ERR_NETWORK = BASE_ERR_OFFSET + 10;       // Network error
constexpr uint32_t ERR_TIMEOUT = BASE_ERR_OFFSET + 11;       // Time out
constexpr uint32_t ERR_OPENSESSION = BASE_ERR_OFFSET + 12;   // Open session fail
constexpr uint32_t ERR_INVALMSG = BASE_ERR_OFFSET + 13;      // Invalid message
constexpr uint32_t ERR_INIT = BASE_ERR_OFFSET + 14;          // Init fail
constexpr uint32_t ERR_HASHDB_INIT = BASE_ERR_OFFSET + 15;   // Init Hash database failed
constexpr uint32_t ERR_HASHDB_CLEAR = BASE_ERR_OFFSET + 16;  // Clear Hash database failed
constexpr uint32_t ERR_HASHDB_DEL = BASE_ERR_OFFSET + 17;    // Delete Hash database failed
constexpr uint32_t ERR_HASHDB_PUT = BASE_ERR_OFFSET + 18;    // Put to Hash database failed
constexpr uint32_t ERR_HASHDB_GET = BASE_ERR_OFFSET + 19;    // Get hash failed
constexpr uint32_t ERR_HASH_LEN = BASE_ERR_OFFSET + 20;      // Get hash length failed
constexpr uint32_t ERR_NOENT = BASE_ERR_OFFSET + 21;         // No such file
constexpr uint32_t ERR_SETDB_PUT = BASE_ERR_OFFSET + 22;     // Put set database failed
constexpr uint32_t ERR_SETDB_ADD = BASE_ERR_OFFSET + 23;     // Put set database failed
constexpr uint32_t ERR_SETDB_GET = BASE_ERR_OFFSET + 24;     // Get set database failed
constexpr uint32_t ERR_SETDB_REM = BASE_ERR_OFFSET + 25;     // Remove value int set database failed
constexpr uint32_t ERR_COUNT_SET = BASE_ERR_OFFSET + 26;     // Get count of set failed
constexpr uint32_t ERR_STORAGE_INIT = BASE_ERR_OFFSET + 27;  // Storage engine init failed
constexpr uint32_t ERR_PUSH_LIST = BASE_ERR_OFFSET + 28;     // Push list data failed
constexpr uint32_t ERR_POP_LIST = BASE_ERR_OFFSET + 29;      // Pop list data failed
constexpr uint32_t ERR_REMOVE_LIST = BASE_ERR_OFFSET + 30;   // Remove list data failed
constexpr uint32_t ERR_COUNT_LIST = BASE_ERR_OFFSET + 31;    // Count list failed
constexpr uint32_t ERR_RANGE_LIST = BASE_ERR_OFFSET + 32;    // Get list range data failed
constexpr uint32_t ERR_PUT_LIST = BASE_ERR_OFFSET + 33;      // Put list data failed
constexpr uint32_t ERR_GET_LIST = BASE_ERR_OFFSET + 34;      // Get list data failed
constexpr uint32_t ERR_EXIST = BASE_ERR_OFFSET + 35;         // Already exist
constexpr uint32_t ERR_NOTOPENED = BASE_ERR_OFFSET + 36;     // Invalid argument
}  // namespace OHOS::ObjectStore

#endif
