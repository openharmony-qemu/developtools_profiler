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
#include "event_notifier.h"
#include "share_memory_block.h"
#include "writer.h"

class StackWriter : public Writer {
public:
    StackWriter(std::string name,
                 uint32_t size,
                 int smbFd,
                 int eventFd,
                 bool blocked = false);
    ~StackWriter();
    long Write(const void* data, size_t size) override;
    long WriteTimeout(const void* data, size_t size);
    long WriteWithPayloadTimeout(const void* data, size_t size, const void* payload, size_t payloadSize);
    bool Flush() override;
    bool Clear() override;

private:
    void DoStats(long bytes);
    void Report() const;

private:
    std::string pluginName_;
    bool blocked_ = false;
    std::shared_ptr<ShareMemoryBlock> shareMemoryBlock_;
    EventNotifierPtr eventNotifier_ = nullptr;
    std::chrono::steady_clock::time_point lastFlushTime_;
    std::atomic<uint64_t> bytesCount_ = 0;
    std::atomic<uint32_t> bytesPending_ = 0;
    std::atomic<uint64_t> writeCount_ = 0;
    std::atomic<uint32_t> flushCount_ = 0;
};


#endif // STACK_WRITER_H