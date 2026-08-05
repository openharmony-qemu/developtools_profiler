/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#ifndef CACHE_FILE_WRITER_H
#define CACHE_FILE_WRITER_H

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include "hook_common.h"
#include "nocopyable.h"

// Callback type for file event in filecache mode
using FileEventCallback = std::function<bool(uint16_t fileIndex, uint16_t eventType, int fd)>;

class CacheFileWriter {
public:
    explicit CacheFileWriter(uint32_t cacheMemorySize);
    ~CacheFileWriter();

    void Prepare(FileEventCallback callback);
    bool AddCacheFile(uint16_t fileIndex, int fd);
    void EnterStopMode();
    bool Write(const void* data, size_t size);
    bool WriteWithPayload(const void* data, size_t size, const void* payload, size_t payloadSize);
    bool WriteWithExtraData(const void* data, size_t size, const void* payload, size_t payloadSize,
        const void* firstExtra, size_t firstExtraSize, const void* secondExtra, size_t secondExtraSize);
    bool Flush();
    void SetCacheFileWriterEnd();
    void UpdateBackpressureState(const BackpressureStateMsg& state);

private:
    bool EnsureWritableCacheFile();
    bool FlushEndedCacheFile();
    bool NotifyStopEvent();
    bool ReturnCurrentCacheFile();
    bool SwitchToNextCacheFile();
    bool WaitForNextCacheFile();
    bool WriteToMemoryCache(const void* data, size_t size);
    bool FlushMemoryCacheToFile();
    bool WriteToFile(const uint8_t* data, size_t size);
    bool NotifyFileEvent(uint16_t fileIndex, uint16_t eventType, int fd);
    void CloseCacheFile(int& fd);
    void ApplyBackpressureDelay();
    void AdaptiveWait(uint32_t delayNs);
    bool ShouldApplyBackpressure();
    uint8_t* AllocateMmapCache(uint32_t capacity);
    void ReleaseMmapCache();

private:
    std::mutex writeMutex_;
    FileEventCallback fileEventCallback_ = nullptr;
    uint8_t* memoryCache_ = nullptr;
    uint32_t memoryCacheCapacity_ = 0;
    size_t memoryCacheSize_ = 0;
    std::array<int, FILE_CACHE_FD_COUNT> pendingFileFds_ {{-1, -1, -1}};
    int currentFileFd_ = -1;
    uint16_t currentFileIndex_ = INVALID_CACHE_FILE_INDEX;
    uint16_t nextFileIndex_ = 0;
    uint64_t currentFileSize_ = 0;
    std::atomic<bool> initialized_ = false;
    bool isCacheFileEnded_ = false;
    bool stopModeEnabled_ = false;
    uint32_t memoryCacheDataSizeLimit_;
    
    std::atomic<uint64_t> delayCount_{0};
    std::atomic<uint32_t> currentBackpressureDelay_{0};

    DISALLOW_COPY_AND_MOVE(CacheFileWriter);
};

#endif // CACHE_FILE_WRITER_H
