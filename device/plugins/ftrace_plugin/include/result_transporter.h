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
 *
 * Description: StreamTransporter class define
 */
#ifndef RESULT_TRANSPORTER_H
#define RESULT_TRANSPORTER_H
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fcntl.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "ftrace_namespace.h"
#include "plugin_module_api.h"
#include "trace_plugin_result.pb.h"

FTRACE_NS_BEGIN
using ResultPtr = std::unique_ptr<TracePluginResult>;

class ResultTransporter {
public:
    using WriterStructPtr = std::unique_ptr<WriterStruct>::pointer;
    ResultTransporter(const std::string& name, WriterStructPtr writer);
    ~ResultTransporter();

    bool Submit(ResultPtr&& result);
    void SetFlushInterval(int ms);
    void SetFlushThreshold(uint32_t nbytes);
    void Flush();

protected:
    long Write(ResultPtr&& packet);

private:
    DISALLOW_COPY_AND_MOVE(ResultTransporter);
    bool IsFlushTime() const;

    std::string name_;
    std::atomic<uint64_t> bytesCount_ = 0;
    std::atomic<uint32_t> bytesPending_ = 0;
    std::atomic<uint64_t> timeCostUs_ = 0;
    std::atomic<uint32_t> writeCount_ = 0;
    std::atomic<uint32_t> flushThreshold_ = 0;
    std::chrono::milliseconds flushInterval_;

    WriterStructPtr writer_ = nullptr;
    std::vector<char> buffer_;
    std::mutex mutex_;
};
FTRACE_NS_END
#endif // RESULT_TRANSPORTER_H
