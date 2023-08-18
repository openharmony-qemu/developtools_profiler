/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BUFFER_WRITER_H
#define BUFFER_WRITER_H

#include <atomic>
#include <memory>
#include "event_notifier.h"
#include "plugin_module_api.h"
#include "share_memory_allocator.h"
#include "writer.h"


class BufferWriter : public Writer {
public:
    BufferWriter(std::string name,
                 std::string version,
                 uint32_t size,
                 int smbFd,
                 int eventFd,
                 uint32_t pluginId);
    ~BufferWriter();
    long Write(const void* data, size_t size) override;
    bool Flush() override;
    bool Clear() override;

    bool WriteMessage(const google::protobuf::Message& pmsg, const std::string& pluginName);

    void SetClockId(clockid_t clockId) override
    {
        clockId_ = clockId;
    }
private:
    void DoStats(long bytes);
    void Report() const;

private:
    std::string pluginName_;
    std::string pluginVersion_;
    std::shared_ptr<ShareMemoryBlock> shareMemoryBlock_;
    EventNotifierPtr eventNotifier_ = nullptr;
    std::chrono::steady_clock::time_point lastFlushTime_;
    std::atomic<uint64_t> bytesCount_ = 0;
    std::atomic<uint32_t> bytesPending_ = 0;
    std::atomic<uint64_t> writeCount_ = 0;
    std::atomic<uint32_t> flushCount_ = 0;
    uint32_t pluginId_ = 0;
    clockid_t clockId_ = CLOCK_REALTIME;
};

using BufferWriterPtr = STD_PTR(shared, BufferWriter);

#endif // BUFFER_WRITER_H
