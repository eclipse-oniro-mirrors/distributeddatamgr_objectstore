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
 
#include "file_utils.h"
#include <fcntl.h>
#include "logger.h"
#include "objectstore_errors.h"

namespace OHOS::ObjectStore {
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp)                \
    ({                                         \
        decltype(exp) _rc;                     \
        do {                                   \
            _rc = (exp);                       \
        } while (_rc == -1 && errno == EINTR); \
        _rc;                                   \
    })
#endif

uint32_t FileUtils::ForceMakeDirectory(const std::string &strBasePath, mode_t dirMode)
{
    uint32_t ret = ERR_INVAL;
    if (strBasePath.empty()) {
        LOG_ERROR("FileUtils-%s: path is empty", __func__);
        return ret;
    }
    for (std::string::size_type index = 0; index != std::string::npos; ) {
        std::string subPath;
        index = strBasePath.find('/', index + 1);
        if (index == std::string::npos) {
            subPath = strBasePath;
        } else {
            subPath = strBasePath.substr(0, index);
        }
        if ((!FileExists(subPath.data())) && (mkdir(subPath.data(), dirMode) != 0)) {
            return ret;
        }
    }
    return SUCCESS;
}

uint32_t FileUtils::ForceCreateFile(const std::string &file, mode_t dirMode, mode_t fileMode)
{
    std::string::size_type posFile = file.find_last_of('/');
    if ((posFile != 0) && (posFile != std::string::npos)) {
        auto ret = ForceMakeDirectory(file.substr(0, posFile), dirMode);
        if (ret != SUCCESS) {
            return ret;
        }
    }
    if ((creat(file.data(), fileMode) == 0) || (access(file.data(), F_OK) == 0)) {
        return SUCCESS;
    }
    LOG_ERROR("FileUtils-%s: fail to create file %{public}d", __func__, errno);
    return ERR_INVAL;
}

ssize_t FileUtils::ReadFile(int fd, off_t offset, size_t size, void *data)
{
    if ((fd < 0) || (offset < 0) || (size < 0) || (data == nullptr)) {
        LOG_ERROR("FileUtils-%s: invalid params, fd %d, offset %ld, size %llu, or buf is null", __func__, fd, offset,
                  static_cast<long long>(size));
        return -1;
    }

    size_t readLen = 0;
    while (readLen < size) {
        ssize_t ret = TEMP_FAILURE_RETRY(pread(fd, data, (size - readLen), (offset + readLen)));
        if (ret < 0) {
            LOG_ERROR("FileUtils-%s: fail to read file %d, error: %d", __func__, fd, errno);
            return ret;
        } else if (ret == 0) {
            break;
        }
        readLen += ret;
    }
    return readLen;
}

ssize_t FileUtils::ReadFile(const std::string &path, off_t offset, size_t size, void *data)
{
    auto fd = TEMP_FAILURE_RETRY(open(path.data(), O_RDONLY | O_CLOEXEC));
    if (fd == -1) {
        LOG_ERROR("FileUtils-%s: fail to open file", __func__);
        return -1;
    }
    auto ret = ReadFile(fd, offset, size, data);
    close(fd);
    return ret;
}

ssize_t FileUtils::WriteFile(int fd, const void *data, off_t offset, size_t size)
{
    if ((fd < 0) || (offset < 0) || (size < 0) || (data == nullptr)) {
        LOG_ERROR("FileUtils-%s: invalid params, fd %d, offset %ld, size %llu, or buf is null", __func__, fd, offset,
                  static_cast<long long>(size));
        return -1;
    }

    size_t writeLen = 0;
    while (writeLen < size) {
        ssize_t ret = TEMP_FAILURE_RETRY(pwrite(fd, data, (size - writeLen), (offset + writeLen)));
        if ((ret < 0) || (ret == 0)) {
            LOG_ERROR("FileUtils-%s: failed to write file %d, error: %d", __func__, fd, errno);
            return ret;
        }
        writeLen += ret;
    }
    return writeLen;
}

ssize_t FileUtils::WriteFile(const std::string &path, const void *data, off_t offset, size_t size)
{
    auto fd = TEMP_FAILURE_RETRY(open(path.data(), O_WRONLY | O_TRUNC | O_CLOEXEC));
    if (fd == -1) {
        LOG_ERROR("FileUtils-%s: fail to open file", __func__);
        return -1;
    }
    auto ret = WriteFile(fd, data, offset, size);
    close(fd);
    return ret;
}

bool FileUtils::FileExists(const std::string &path)
{
    if (path.empty()) {
        LOG_ERROR("FileUtils-%s: path is empty", __func__);
        return false;
    }

    if (access(path.data(), F_OK) != 0) {
        return false;
    }
    return true;
}
}  // namespace OHOS::ObjectStore