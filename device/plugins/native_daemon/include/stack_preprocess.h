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
#ifndef STACK_PREPROCESS_H
#define STACK_PREPROCESS_H

#include <chrono>
#include <thread>
#include <unordered_map>
#include <list>
#include <algorithm>

#include "logging.h"
#include "nocopyable.h"
#include "stack_data_repeater.h"
#include "buffer_writer.h"
#include "virtual_runtime.h"
#include "hook_common.h"
#include "native_hook_config.pb.h"
#include "native_hook_result.pb.h"
#include "safe_map.h"

class StackPreprocess {
public:
    struct RecordStatistic {
        uint32_t pid {0};
        uint32_t callstackId {0};
        uint32_t tagId {0};
        RecordStatisticsEvent::MemoryType type {RecordStatisticsEvent::MALLOC};
        uint64_t applyCount {0};
        uint64_t releaseCount {0};
        uint64_t applySize {0};
        uint64_t releaseSize {0};
    };

    explicit StackPreprocess(const StackDataRepeaterPtr& dataRepeater, const NativeHookConfig& hookConfig,
        clockid_t pluginDataClockId, bool isHookStandalone = false);
    explicit StackPreprocess(bool fpUnwind);
    ~StackPreprocess();
    void SetWriter(const std::shared_ptr<BufferWriter>& writer);
    bool StartTakeResults();
    bool StopTakeResults();
    bool FlushRecordStatistics();
    void SetSerializeMode(bool protobufSerialize);
    void SaveMemTag(uint32_t tagId, const std::string& tagName);
    bool GetMemTag(uint32_t tagId, std::string& tagName);
    void SaveThreadName(uint32_t tid, const std::string& tname);

private:
    using CallFrame = OHOS::Developtools::NativeDaemon::CallFrame;
    struct ElfSymbolTable {
        uint64_t textVaddr;
        uint32_t textOffset;
        uint32_t symEntSize;
        std::vector<uint8_t> strTable;
        std::vector<uint8_t> symTable;
    };

    enum RecordStatisticsLimit : std::size_t {
        STATISTICS_MAP_SZIE = 2048,
        STATISTICS_PERIOD_DATA_SIZE = 256,
        ALLOC_ADDRMAMP_SIZE = 2048,
    };

private:
    void TakeResults();
    void SetHookData(RawStackPtr RawStack, std::vector<CallFrame>& callFrames,
        BatchNativeHookData& batchNativeHookData);
    void WriteFrames(RawStackPtr RawStack, const std::vector<CallFrame>& callFrames);
    void SetFrameInfo(Frame& frame, CallFrame& callFrame);
    void ReportSymbolNameMap(CallFrame& callFrame, BatchNativeHookData& batchNativeHookData);
    void ReportFilePathMap(CallFrame& callFrame, BatchNativeHookData& batchNativeHookData);
    void ReportFrameMap(CallFrame& callFrame, BatchNativeHookData& batchNativeHookData);
    void ReportThreadNameMap(uint32_t tid, const std::string& tname, BatchNativeHookData& batchNativeHookData);
    void SetMapsInfo(pid_t pid);
    void SetSymbolInfo(uint32_t filePathId, ElfSymbolTable& symbolInfo,
        BatchNativeHookData& batchNativeHookData);
    void FlushData(BatchNativeHookData& stackData);
    void Flush(const uint8_t* src, size_t size);
    void GetSymbols(const std::string& filePath, ElfSymbolTable& symbols);

    void FillOfflineCallStack(std::vector<CallFrame>& callFrames, size_t idx);
    void FillCallStack(std::vector<CallFrame>& callFrames,
        BatchNativeHookData& batchNativeHookData, size_t idx);
    uint32_t SetCallStackMap(BatchNativeHookData& batchNativeHookData);
    uint32_t GetCallStackId(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames,
        BatchNativeHookData& batchNativeHookData);
    template <typename T>
    void SetEventFrame(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames,
        BatchNativeHookData& batchNativeHookData, T* event, uint32_t stackId);
    void SetAllocStatisticsFrame(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames,
        BatchNativeHookData& batchNativeHookData);
    bool SetFreeStatisticsData(uint64_t addr);
    void SetAllocStatisticsData(const RawStackPtr& rawStack, size_t stackId, bool isExists = false);
private:
    std::shared_ptr<BufferWriter> writer_ = nullptr;
    StackDataRepeaterPtr dataRepeater_ = nullptr;
    std::thread thread_ {};
    std::unique_ptr<uint8_t[]> buffer_;
    bool isStopTakeData_ = false;
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime_instance;
    DISALLOW_COPY_AND_MOVE(StackPreprocess);
    OHOS::SafeMap<uint32_t, std::string> memTagMap_ = {};
    std::unordered_map<uint32_t, std::string> threadNameMap_ = {};
    NativeHookConfig hookConfig_;
    std::unique_ptr<FILE, decltype(&fclose)> fpHookData_;
    uint32_t ignoreCnts_ = 0;
    uint32_t eventCnts_ = 0;
    bool flushBasicData_ {true};
    std::vector<u64> u64regs_;
    std::vector<CallFrame> callFrames_;
    std::vector<uint64_t> callStack_;
    // Key is callStack_, value is call stack id
    std::map<std::vector<uint64_t>, uint32_t> callStackMap_;
    std::chrono::seconds statisticsInterval_ {0};
    std::chrono::steady_clock::time_point lastStatisticsTime_;
    // Key is call stack id, value is recordstatistic data
    std::unordered_map<uint32_t, RecordStatistic> recordStatisticsMap_ {STATISTICS_MAP_SZIE};
    // Key is call stack id, value is recordstatistic data pointer
    std::unordered_map<uint32_t, RecordStatistic*> statisticsPeriodData_ {STATISTICS_PERIOD_DATA_SIZE};
    // Key is alloc or mmap address, value first is mallocsize, second is recordstatistic data pointer
    std::unordered_map<uint64_t, std::pair<uint64_t, RecordStatistic*>> allocAddrMap_ {ALLOC_ADDRMAMP_SIZE};
    bool isProtobufSerialize_ = true;
    // used for plugin data
    clockid_t pluginDataClockId_ = CLOCK_REALTIME;
    // used for clac wait time in StackDataRepeater::TakeRawData() or statistics HookData
    clockid_t hookDataClockId_ = CLOCK_REALTIME;
    bool isHookStandaloneSerialize_ = false;
};

#endif // STACK_PREPROCESS_H