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

#include "object_utils.h"
#include <sstream>
#include <vector>
#include "string_utils.h"

namespace OHOS::ObjectStore {
namespace {
const std::string OBJECT_ID_SCHEMA = "dataobject://";
const std::string SEPARATOR = "/";
const int OBJECT_INDEX_STORE = 3;
const int OBJECT_INDEX_FIELDS = 4;
}  // namespace

std::string ObjectUtils::GenObjectIdPrefix(const std::string &host, const std::string &user, const std::string &bundle,
    const std::string &store)
{
    std::stringstream stream;
    stream << OBJECT_ID_SCHEMA << host << SEPARATOR << user << SEPARATOR << bundle << SEPARATOR << store;
    return stream.str();
}

std::string ObjectUtils::GetObjectHost(const std::string &objectId)
{
    std::string body = objectId.substr(OBJECT_ID_SCHEMA.length());
    auto index = body.find_first_of(SEPARATOR);
    if (index == std::string::npos) {
        return "";
    }
    return body.substr(0, index);
}

std::string ObjectUtils::GetObjectStoreName(const std::string &objectId)
{
    auto index = objectId.find_first_not_of(OBJECT_ID_SCHEMA);
    if (index == 0 || index == std::string::npos) {
        return "";
    }

    std::vector<std::string> items;
    StringUtils::Split(objectId.substr(index), SEPARATOR, items);
    if (items.size() < OBJECT_INDEX_FIELDS) {
        return "";
    }
    return items[OBJECT_INDEX_STORE];
}
}  // namespace OHOS::ObjectStore