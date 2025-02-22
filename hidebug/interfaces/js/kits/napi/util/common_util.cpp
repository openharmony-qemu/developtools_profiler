/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "common_util.h"

#include <cstring>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
#include <vector>

#include <sys/stat.h>
#include <sys/xattr.h>

#include "application_context.h"
#include "parameters.h"

#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {
namespace {
#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HiDebug_COMMON_UTIL"
}
std::string GetProcessDir(DirectoryType type)
{
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    if (!context) {
        return "";
    }
    switch (type) {
        case CACHE:
            return context->GetCacheDir();
        case FILE:
            return context->GetFilesDir();
        default:
            return "";
    }
}

std::vector<std::string> SplitStr(const std::string& origin, char delimiter,
    const std::function<bool(std::string&)>& filter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(origin);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!filter || filter(token)) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

bool GetXAttr(const std::string& fileName, const std::string& key, std::string& value, size_t maxLength)
{
    value = std::string(maxLength + 1, '\0');
    if (getxattr(fileName.c_str(), key.c_str(), value.data(), value.length() - 1) == -1) {
        HILOG_ERROR(LOG_CORE, "failed to getxattr %{public}s from %{public}s because of %{public}s.",
            key.c_str(), fileName.c_str(), strerror(errno));
        return false;
    }
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
        if (access(path.c_str(), W_OK) == 0) {
            return true;
        }
        return false;
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
}
}