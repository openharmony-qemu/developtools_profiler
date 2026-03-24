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

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>
#include "event_notifier.h"

// Callback type for file event in filecache mode
using FileEventCallback = std::function<void(uint16_t fileIndex, uint16_t eventType, int fd)>;

class CacheFileWriter {
public:
    CacheFileWriter(int eventFd, uint32_t cacheMemorySize);
    ~CacheFileWriter();

    void Prepare(FileEventCallback callback);
    bool Write(const void* data, size_t size);
    bool WriteWithPayload(const void* data, size_t size, const void* payload, size_t payloadSize);
    bool Flush();
    void SetCacheFileWriterEnd();

private:
    void WriteToMemoryCache(const void* data, size_t size);
    bool FlushMemoryCacheToFile();
    bool CreateNewCacheFile();
    std::string GetCacheFilePath(uint16_t index) const;
    bool WriteToFile(const uint8_t* data, size_t size);
    void HandleFileCacheEnd();
    bool CheckIsDiskUsageExceedLimit();
    void HandleFileSizeExceedLimit();
    void NotifyFileEvent(uint16_t fileIndex, uint16_t eventType);
    bool CleanOldCacheFiles();

private:
    std::mutex writeMutex_;
    FileEventCallback fileEventCallback_ = nullptr;
    EventNotifierPtr eventNotifier_ = nullptr;
    std::vector<uint8_t> memoryCache_;
    int currentFileFd_ = -1;
    std::string cacheFilePath_;
    uint16_t currentFileIndex_ = 0;
    uint64_t currentFileSize_ = 0;
    uint64_t totalBytesWritten_ = 0;
    std::atomic<bool> initialized_ = false;
    std::atomic<bool> isTotalFileSizeLimit_ = false;
    bool isCacheFileEnded_ = false;
    std::string cacheFileDir_;
    uint32_t cacheMemorySize_;

    DISALLOW_COPY_AND_MOVE(CacheFileWriter);
};

#endif // CACHE_FILE_WRITER_H