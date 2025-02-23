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

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include <cstdint>
#include <functional>
#include <string>

namespace OHOS {
namespace HiviewDFX {

enum DirectoryType {
    CACHE = 0,
    FILE = 1,
};

std::string GetProcessDir(DirectoryType type);

std::vector<std::string> SplitStr(const std::string& origin, char delimiter,
    const std::function<bool(std::string&)>& filter = nullptr);

bool GetXAttr(const std::string& fileName, const std::string& key, std::string& value, size_t maxLength);

bool SetXAttr(const std::string& fileName, const std::string& key, const std::string& value);

bool CreateFile(const std::string &path);

bool CreateDirectory(const std::string &path, unsigned mode);

bool IsLegalPath(const std::string& path);

uint64_t GetFileSize(const std::string& path);

bool IsBetaVersion();

bool IsDebuggableHap();

bool IsDeveloperOptionsEnabled();
}
}

#endif //COMMON_UTIL_H
