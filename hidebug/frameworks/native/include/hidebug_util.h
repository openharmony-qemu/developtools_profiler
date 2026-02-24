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

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "hidebug_native_type.h"
namespace OHOS {
namespace HiviewDFX {

int64_t GetElapsedNanoSecondsSinceBoot();

int64_t GetRealNanoSecondsTimestamp();

template<typename T>
class CachedValue {
public:
    CachedValue() = default;
    std::pair<int, T> GetOrUpdateCachedValue(int64_t effectiveTime, std::function<int(T &)> getValue)
    {
        std::pair<int, T> ret;
        int64_t currentTime = GetElapsedNanoSecondsSinceBoot();
        if (currentTime > updateTime_.load(std::memory_order_relaxed) + effectiveTime) {
            std::unique_lock<std::mutex> lock(updateValueMutex_);
            if (currentTime > updateTime_.load(std::memory_order_acquire) + effectiveTime) {
                ret.first = getValue(ret.second);
                if (ret.first == NATIVE_SUCCESS) {
                    std::unique_lock<std::mutex> lockCacheValue(cachedValueMutex_);
                    cachedValue_ = ret.second;
                    currentTime = GetElapsedNanoSecondsSinceBoot();
                    updateTime_.store(currentTime, std::memory_order_release);
                }
                return ret;
            }
        }
        std::unique_lock<std::mutex> lockCacheValue(cachedValueMutex_);
        ret.second = cachedValue_;
        ret.first = NATIVE_SUCCESS;
        return ret;
    }

private:
    T cachedValue_;
    std::mutex cachedValueMutex_;
    std::mutex updateValueMutex_;
    std::atomic<int64_t> updateTime_ = INT64_MIN;
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

bool CheckVersionType(const std::string& type, const std::string& key);

bool CreateResourceLimitDir();

bool IsHm();

bool GetGlAndGraph(GraphicsMemorySummary& graphicMemoryInfo);

bool GetVssInfo(NativeMemInfo& nativeMemInfo);

bool GetMemInfo(NativeMemInfo& nativeMemInfo);
}
}
#endif // HIDEBUG_UTIL_H_
