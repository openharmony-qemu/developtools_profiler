/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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


#ifndef STACK_WRITER_H
#define STACK_WRITER_H

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <mutex>
#include "event_notifier.h"
#include "share_memory_block.h"
#include "writer.h"
#include "cache_file_writer.h"

class StackWriter : public Writer {
public:
    StackWriter(std::string name,
                 uint32_t size,
                 int smbFd,
                 int eventFd,
                 bool blocked = false,
                 bool isSaMode = false,
                 bool useFileCacheMode = false);
    ~StackWriter();
    long Write(const void* data, size_t size) override;
    bool WriteTimeout(const void* data, size_t size);
    bool WriteSync(const void* data, size_t size, const std::function<bool()>& callback);
    bool WriteWithPayloadTimeout(const void* data, size_t size, const void* payload, size_t payloadSize,
                                 const std::function<bool()>& callback);
    bool WriteWithExtraDataTimeout(const void* data, size_t size, const void* payload, size_t payloadSize,
        const void* firstExtra, size_t firstExtraSize, const void* secondExtra, size_t secondExtraSize,
        const std::function<bool()>& callback);
    bool Flush() override;
    bool PrepareFlush();
    bool IsFileCacheMode();
    void PrepareCacheFileWriter(FileEventCallback callback);
    bool AddFileCacheDescriptor(uint16_t fileIndex, int fd);
    void EnterStopFileCacheMode();
    void StopFileCache();

private:
    void DoStats(long bytes);
    void Report() const;

private:
    std::string pluginName_;
    bool blocked_ = false;
    bool useFileCache_ = false;
    std::shared_ptr<ShareMemoryBlock> shareMemoryBlock_ = nullptr;
    std::shared_ptr<CacheFileWriter> cacheFileWriter_ = nullptr;
    EventNotifierPtr eventNotifier_ = nullptr;
    std::chrono::steady_clock::time_point lastFlushTime_;
    std::atomic<uint64_t> bytesCount_ = 0;
    std::atomic<uint32_t> bytesPending_ = 0;
    std::atomic<uint64_t> writeCount_ = 0;
    std::atomic<uint32_t> flushCount_ = 0;
    std::atomic<uint64_t> dataCount_ = 0;
    std::mutex flushMutex_;
};


#endif // STACK_WRITER_H
