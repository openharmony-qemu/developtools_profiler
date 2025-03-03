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

#ifndef HIDEBUG_UTIL_H_
#define HIDEBUG_UTIL_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace OHOS {
namespace HiviewDFX {

int64_t GetElapsedNanoSecondsSinceBoot();

int64_t GetRealNanoSecondsTimestamp();

template<typename T>
class CachedValue {
public:
    CachedValue() = default;
    std::optional<T> GetOrUpdateCachedValue(int64_t effectiveTime, std::function<bool(T &)> getValue)
    {
        int64_t currentTime = GetElapsedNanoSecondsSinceBoot();
        if (currentTime <= expirationTime_) {
            return cachedValue_;
        }
        if (getValue(cachedValue_)) {
            expirationTime_ = currentTime + effectiveTime;
            return cachedValue_;
        }
        return {};
    };

private:
    T cachedValue_;
    int64_t expirationTime_ = -1;
};

enum class DirectoryType {
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

class SmartFile {
public:
    static std::unique_ptr<SmartFile> OpenFile(const std::string& path, const std::string& mode);
    ~SmartFile();
    SmartFile(SmartFile&& other) = delete;
    SmartFile& operator=(SmartFile&& other) = delete;
    SmartFile(const SmartFile&) = delete;
    SmartFile& operator=(const SmartFile&) = delete;
    bool Write(const void *__restrict dataPtr, size_t itemSize, size_t dataNum);
    size_t Read(void *__restrict dataPtr, size_t itemSize, size_t dataNum);
private:
    explicit SmartFile(FILE *file) : file_(file) {}
    FILE *file_;
};

bool IsBetaVersion();

bool IsDebuggableHap();

bool IsDeveloperOptionsEnabled();
}
}
#endif // HIDEBUG_UTIL_H_
