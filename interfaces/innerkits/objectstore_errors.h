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
constexpr uint32_t ERR_BYTE_SIZE = BASE_ERR_OFFSET + 13;     // get byte size failed
constexpr uint32_t ERR_FLOAT_SIZE = BASE_ERR_OFFSET + 14;     // get float size failed
constexpr uint32_t ERR_DOUBLE_SIZE = BASE_ERR_OFFSET + 15;     // get double size failed
}  // namespace OHOS::ObjectStore

#endif
