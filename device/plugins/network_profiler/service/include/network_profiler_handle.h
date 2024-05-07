/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#ifndef NETWORK_PROFILER_HANDLE_H
#define NETWORK_PROFILER_HANDLE_H

#include <chrono>
#include <variant>

#include "logging.h"

#include "buffer_writer.h"
#include "network_profiler_result.pb.h"
#include "network_profiler_result.pbencoder.h"

using WriterStructPtr = std::unique_ptr<WriterStruct>::pointer;
namespace OHOS::Developtools::Profiler {
class NetworkProfilerManager;

class NetworkProfilerHandle {
public:
    explicit NetworkProfilerHandle(clockid_t pluginDataClockId, uint32_t bufferSize, bool isProtobufSerialize = true);
    ~NetworkProfilerHandle();

    void SetWriter(const std::shared_ptr<Writer>& writer);
    void SetWriter(const WriterStructPtr& writer);
    void SerializeData(const int8_t data[], uint32_t size);
    void SetTargetProcessInfo(uint32_t pid, const std::string& name)
    {
        pid_ = pid;
        processName_ = name;
    }

private:
    template <typename T>
    void FlushCheck(T& protoData);
    void FlushData(::NetworkProfilerResult& data);
    void FlushData(ProtoEncoder::NetworkProfilerResult& data);
    template <typename T>
    void SerializeDataImpl(T& protoData, const int8_t data[], uint32_t size);
    template <typename T>
    void SerializeBaseData(T& baseDataProto, const int8_t* data);
    template <typename T>
    void SerializeTraceData(T& protoData, const int8_t data[], uint32_t size);
    template <typename T>
    void SerializeThreadData(T& protoData, const int8_t data[], uint32_t size);
    template <typename T>
    void SerializeRawData(T& protoData, const int8_t data[], uint32_t size);

private:
    std::unique_ptr<uint8_t[]> buffer_{nullptr};
    std::shared_ptr<Writer> writer_{nullptr};
    WriterStructPtr writerStruct_{nullptr};
    bool isProtobufSerialize_{true};
    clockid_t pluginDataClockId_ = CLOCK_REALTIME;
    std::variant<::NetworkProfilerResult, ProtoEncoder::NetworkProfilerResult> protoData_;
    uint64_t flushSize_{0};
    uint32_t flushCount_{0};
    uint32_t bufferSize_{0};
    uint32_t pid_{0};
    std::string processName_;
};
} // OHOS::Developtools::Profiler

#endif // NETWORK_PROFILER_HANDLE_H