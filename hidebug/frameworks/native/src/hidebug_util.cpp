/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "hidebug_util.h"

#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <thread>
#include <vector>

#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "application_context.h"
#include "directory_ex.h"
#include "file_ex.h"
#include "parameters.h"
#include "storage_acl.h"

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "CommonUtil"

int64_t GetNanoSecondsTimestamp(clockid_t clockId)
{
    struct timespec times{};
    if (clock_gettime(clockId, &times) == -1) {
        return -1;
    }
    constexpr int64_t secondToNanosecond = 1 * 1000 * 1000 * 1000;
    return times.tv_sec * secondToNanosecond + times.tv_nsec;
}
}

int64_t GetElapsedNanoSecondsSinceBoot()
{
    return GetNanoSecondsTimestamp(CLOCK_BOOTTIME);
}

int64_t GetRealNanoSecondsTimestamp()
{
    return GetNanoSecondsTimestamp(CLOCK_REALTIME);
}

std::string GetProcessDir(DirectoryType type)
{
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (!context) {
        return "";
    }
    switch (type) {
        case DirectoryType::CACHE:
            return context->GetCacheDir();
        case DirectoryType::FILE:
            return context->GetFilesDir();
        default:
            return "";
    }
}

std::vector<std::string> SplitStr(const std::string& origin, char delimiter,
    const std::function<bool(std::string&)>& filter)
{
    std::vector<std::string> tokens;
    size_t index = 0;
    for (size_t i = 0; i <= origin.length(); ++i) {
        if (i == origin.length() || origin[i] == delimiter) {
            std::string token = origin.substr(index, i - index);
            if (!filter || filter(token)) {
                tokens.emplace_back(std::move(token));
            }
            index = i + 1;
        }
    }
    return tokens;
}

bool GetXAttr(const std::string& fileName, const std::string& key, std::string& value, size_t maxLength)
{
    std::string readValue = std::string(maxLength + 1, '\0');
    if (getxattr(fileName.c_str(), key.c_str(), readValue.data(), maxLength) == -1) {
        HILOG_ERROR(LOG_CORE, "failed to getxattr %{public}s from %{public}s because of %{public}s.",
            key.c_str(), fileName.c_str(), strerror(errno));
        return false;
    }
    value = readValue.c_str();
    return true;
}

bool SetXAttr(const std::string& fileName, const std::string& key, const std::string& value)
{
    if (setxattr(fileName.c_str(), key.c_str(), value.c_str(), value.size(), 0) != 0) {
        HILOG_ERROR(LOG_CORE, "failed to setxattr %{public}s to %{public}s because of %{public}s.",
            key.c_str(), fileName.c_str(), strerror(errno));
        return false;
    }
    return true;
}

bool IsLegalPath(const std::string& path)
{
    return !path.empty() && path.find("./") == std::string::npos;
}

uint64_t GetFileSize(const std::string& path)
{
    struct stat statBuf{};
    if (stat(path.c_str(), &statBuf) == 0) {
        return statBuf.st_size;
    }
    return 0;
}

bool CreateFile(const std::string &path)
{
    if (access(path.c_str(), F_OK) == 0) {
        return access(path.c_str(), W_OK) == 0;
    }
    const mode_t defaultMode = S_IRUSR | S_IWUSR | S_IRGRP; // -rw-r-----
    int fd = creat(path.c_str(), defaultMode);
    if (fd == -1) {
        HILOG_ERROR(LOG_CORE, "file create failed, errno = %{public}d", errno);
        return false;
    }
    close(fd);
    return true;
}

bool CreateDirectory(const std::string &path, unsigned mode)
{
    std::vector<std::string> subPaths = SplitStr(path, '/', [](std::string& subPath) {
        return !subPath.empty();
    });
    std::string currentPath;
    for (const auto& subPath : subPaths) {
        currentPath += "/" + subPath;
        if (mkdir(currentPath.c_str(), mode) != 0 && errno != EEXIST) {
            HILOG_ERROR(LOG_CORE, "directory %{public}s create failed, errno = %{public}d", currentPath.c_str(), errno);
            return false;
        }
    }
    return true;
}

std::unique_ptr<SmartFile> SmartFile::OpenFile(const std::string& path, const std::string& mode)
{
    if (!IsLegalPath(path)) {
        HILOG_ERROR(LOG_CORE, "illegal file path %{public}s .", path.c_str());
        return nullptr;
    }
    auto file = fopen(path.c_str(), mode.c_str());
    if (file == nullptr) {
        HILOG_ERROR(LOG_CORE, "can not open file %{public}s for %{public}s because of %{public}s.", path.c_str(),
            mode.c_str(), strerror(errno));
        return nullptr;
    }
    return std::unique_ptr<SmartFile>(new (std::nothrow) SmartFile(file));
}

SmartFile::~SmartFile()
{
    if (fclose(file_) != 0) {
        HILOG_ERROR(LOG_CORE, "failed close file because of %{public}s.", strerror(errno));
    }
}

bool SmartFile::Write(const void *__restrict dataPtr, size_t itemSize, size_t dataNum)
{
    errno = 0;
    if (fwrite(dataPtr, itemSize, dataNum, file_) != dataNum) {
        HILOG_ERROR(LOG_CORE, "failed to write file because of %{public}s.", strerror(errno));
        return false;
    }
    return true;
}

size_t SmartFile::Read(void *__restrict dataPtr, size_t itemSize, size_t dataNum)
{
    return fread(dataPtr, itemSize, dataNum, file_);
}

bool IsBetaVersion()
{
    return OHOS::system::GetParameter("const.logsystem.versiontype", "") == "beta";
}

bool IsDebuggableHap()
{
    const char* debuggableEnv = getenv("HAP_DEBUGGABLE");
    return debuggableEnv != nullptr && strcmp(debuggableEnv, "true") == 0;
}

bool IsDeveloperOptionsEnabled()
{
    return OHOS::system::GetBoolParameter("const.security.developermode.state", false);
}

bool CheckVersionType(const std::string& type, const std::string& key)
{
    auto versionType = OHOS::system::GetParameter(key, "unknown");
    return (versionType.find(type) != std::string::npos);
}

bool CreateResourceLimitDir()
{
    constexpr mode_t defaultLogDirMode = 0770;
    const std::string resourceLimitDir = "/data/storage/el2/log/resourcelimit/";
    if (!OHOS::FileExists(resourceLimitDir)) {
        OHOS::ForceCreateDirectory(resourceLimitDir);
        OHOS::ChangeModeDirectory(resourceLimitDir, defaultLogDirMode);
    }
    if (OHOS::StorageDaemon::AclSetAccess(resourceLimitDir, "g:1201:rwx") != 0) {
        HILOG_ERROR(LOG_CORE, "CreateSanBoxDir Failed to AclSetAccess");
        return false;
    }
    return true;
}
}
}
