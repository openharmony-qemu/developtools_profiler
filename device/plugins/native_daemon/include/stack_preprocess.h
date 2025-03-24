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
#include <mutex>
#include <variant>

#include "logging.h"
#include "nocopyable.h"
#include "stack_data_repeater.h"
#include "buffer_writer.h"
#include "virtual_runtime.h"
#include "hook_common.h"
#include "native_hook_config.pb.h"
#include "native_hook_result.pb.h"
#include "native_hook_result.pbencoder.h"
#include "safe_map.h"
#include "schedule_task_manager.h"

using WriterStructPtr = std::unique_ptr<WriterStruct>::pointer;
class StackPreprocess : public std::enable_shared_from_this<StackPreprocess> {
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
        clockid_t pluginDataClockId, FILE* fpHookData = nullptr, bool isHookStandalone = false,
        bool isSaService = false, bool isProtobufSerialize = true);
    ~StackPreprocess();
    void SetWriter(const std::shared_ptr<Writer>& writer);
    void SetWriter(const WriterStructPtr& writer);
    bool StartTakeResults();
    bool StopTakeResults();
    void FinishTraceFile();
    bool FlushRecordStatistics();
    void FlushRecordApplyAndReleaseMatchData();
    void ForceStop();
    inline void SetPid(int32_t pid)
    {
        pid_ = pid;
    }
    inline void InitStatisticsTime()
    {
        lastStatisticsTime_ = std::chrono::steady_clock::now();
    }
    void SaveMemTag(uint32_t tagId, const std::string& tagName);
    bool GetMemTag(uint32_t tagId, std::string& tagName);
    void SaveJsRawStack(uint64_t jsChainId, const char* jsRawStack);
    const char* GetJsRawStack(uint64_t jsChainId);
    void ReportBasicData();
    void WriteHookConfig();
    void TakeResultsFromShmem(const std::shared_ptr<EventNotifier>&, const std::shared_ptr<ShareMemoryBlock>&);
    void SetNmdFd(uint32_t fd)
    {
        nmdFd_ = fd;
    }
    void SetFlushSize(uint64_t size)
    {
        double tenth = static_cast<double>(size) / 10.0;
        flushSize_ = static_cast<uint64_t>(std::ceil(tenth));
        PROFILER_LOG_INFO(LOG_CORE, "SetFlushSize size: %" PRIu64 ", flushSize_: %" PRIu64 "", size, flushSize_);
        if (isProtobufSerialize_) {
            bufferSize_ = flushSize_ << 1;
            buffer_ = std::make_unique<uint8_t[]>(bufferSize_);
        }
    }

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
        STATISTICS_MAP_SZIE = 100000,
        STATISTICS_PERIOD_DATA_SIZE = 100000,
        ALLOC_ADDRMAMP_SIZE = 100000,
        MATCH_ADDRMAMP_SIZE = 100000,
    };

    struct ScopedLockFile {
        ScopedLockFile(FILE* fpHook): fpHookData(fpHook)
        {
            flockfile(fpHookData);
        }
        ~ScopedLockFile()
        {
            funlockfile(fpHookData);
        }
        FILE* fpHookData {nullptr};
    };

private:
    void TakeResults();
    template <typename T>
    void SetHookData(RawStackPtr rawStack, T& stackData);
    template <typename T>
    void SetHookData(RawStackPtr rawStack, std::vector<CallFrame>& callFrames, T& stackData);
    void WriteFrames(RawStackPtr RawStack, const std::vector<CallFrame>& callFrames);
    template <typename T>
    void SetFrameInfo(T& frame, CallFrame& callFrame);
    template <typename T>
    void ReportSymbolNameMap(CallFrame& callFrame, T& stackData);
    template <typename T>
    void ReportFilePathMap(CallFrame& callFrame, T& stackData);
    template <typename T>
    void ReportFrameMap(CallFrame& callFrame, T& stackData);
    void ReportThreadNameMap(uint32_t tid, const std::string& tname);
    void SetMapsInfo();
    template <typename T>
    void SetSymbolInfo(uint32_t filePathId, ElfSymbolTable& symbolInfo, T& batchNativeHookData);
    template <typename T>
    void FlushCheck(T& stackData);
    void FlushData(BatchNativeHookData& stackData);
    void FlushData(OHOS::Developtools::Profiler::ProtoEncoder::BatchNativeHookData& stackData);
    void Flush(const uint8_t* src, size_t size);
    void GetSymbols(const std::string& filePath, ElfSymbolTable& symbols);
    template <typename T>
    void FillOfflineCallStack(std::vector<CallFrame>& callFrames, size_t idx, T& stackData);
    template <typename T>
    void FillCallStack(std::vector<CallFrame>& callFrames, size_t idx, T& stackData);
    template <typename T>
    uint32_t SetCallStackMap(T& stackData);
    template <typename T>
    uint32_t GetCallStackId(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames, T& stackData);
    uint32_t FindCallStackId(std::vector<uint64_t>& callStack);
    template <typename T>
    void SetEventFrame(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames,
        T* event, uint32_t stackId, const std::string& type = "");
    template <typename T>
    void SetEventFrame(const ReportEventBaseData& rawStack, T* event, uint32_t stackMapId,
                       const std::string& type = "");
    template <typename T>
    void SetAllocStatisticsFrame(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames, T& stackData);
    template <typename T>
    void SetAllocStatisticsFrame(const RawStackPtr& rawStack, T& stackData);
    template <typename T>
    void SetApplyAndReleaseMatchFrame(RawStackPtr rawStack, std::vector<CallFrame>& callFrames, T& stackData);
    void IntervalFlushRecordStatistics();
    void IntervalFlushApplyAndReleaseMatchData();
    bool HandleNoStackEvent(RawStackPtr& rawStack);
    bool SetFreeStatisticsData(uint64_t addr);
    void SetAllocStatisticsData(const RawStackPtr& rawStack, size_t stackId, bool isExists = false);
    unsigned LgFloor(unsigned long x);
    uint64_t PowCeil(uint64_t x);
    size_t ComputeAlign(size_t size);
    void ReportOfflineSymbolizationData();

    RandomWriteCtx* StartReport();
    int32_t FinishReport();
    void FillFpNativeIp(RawStackPtr& rawData);
    void FillFpJsData(RawStackPtr& rawData);
    void FillDwarfErrorStack();
    void FillNapiStack(std::string& tagName, std::vector<CallFrame>& callFrames, uint64_t napiIndex);
private:
    std::chrono::steady_clock::time_point lastStatisticsTime_ = std::chrono::steady_clock::now();
    std::shared_ptr<Writer> writer_ = nullptr;
    StackDataRepeaterPtr dataRepeater_ = nullptr;
    std::thread thread_ {};
    std::unique_ptr<uint8_t[]> buffer_;
    std::atomic_bool isStopTakeData_ = false;
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime_instance;
    DISALLOW_COPY_AND_MOVE(StackPreprocess);
    OHOS::SafeMap<uint32_t, std::string> memTagMap_ = {};
    std::unordered_map<uint32_t, std::string> threadNameMap_ = {};
    NativeHookConfig hookConfig_;
    uint32_t ignoreCnts_ = 0;
    uint32_t eventCnts_ = 0;
    bool flushBasicData_ {true};
    std::vector<u64> u64regs_;
    std::vector<CallFrame> callFrames_;
    std::vector<uint64_t> callStack_;
    // Key is callStack_, value is call stack id
    std::map<std::vector<uint64_t>, uint32_t> callStackMap_;
    std::chrono::seconds statisticsInterval_ {0};
    // Key is call stack id, value is recordstatistic data
    std::unordered_map<uint32_t, RecordStatistic> recordStatisticsMap_;
    // Key is call stack id, value is recordstatistic data pointer
    std::unordered_map<uint32_t, RecordStatistic*> statisticsPeriodData_;
    // Key is alloc or mmap address, value first is mallocsize, second is recordstatistic data pointer
    std::unordered_map<uint64_t, std::pair<uint64_t, RecordStatistic*>> allocAddrMap_;
    // Key is alloc or mmap address, value is ReportEventBaseData list iterator
    std::unordered_map<uint64_t, std::list<ReportEventBaseData>::iterator> applyAndReleaseMatchIntervallMap_;
    std::list<ReportEventBaseData> applyAndReleaseMatchPeriodListData_;
    std::chrono::seconds applyAndReleaseMatchInterval_{0};
    // used for plugin data
    clockid_t pluginDataClockId_ = CLOCK_REALTIME;
    // used for clac wait time in StackDataRepeater::TakeRawData() or statistics HookData
    clockid_t hookDataClockId_ = CLOCK_REALTIME;
    FILE* fpHookData_ {nullptr};
    bool isHookStandaloneSerialize_ {false};
    int32_t pid_ {-1};
    std::mutex mtx_;
    bool isSaService_{false};
    std::mutex allocAddrMapMtx_;
    bool isProtobufSerialize_{true};
    WriterStructPtr resultWriter_{nullptr};
    std::variant<BatchNativeHookData, OHOS::Developtools::Profiler::ProtoEncoder::BatchNativeHookData> stackData_;
    uint64_t flushSize_{0};
    uint64_t bufferSize_{0};
    bool statisticsModelFlushCallstack_{false};
    OHOS::Developtools::Profiler::ProtoEncoder::ProfilerPluginData profilerPluginData_;
    // Key is js stack id , value is js raw stack pointer
    std::map<uint64_t, const char*> jsStackMap_ = {};
    std::set<std::string> jsStackSet_ = {};
    bool unwindFailReport_ = true;
    std::vector<std::string> prctlPeriodTags_; // applyAndReleaseMatchInterval mode used
    std::vector<std::string> fpJsCallStacks_;
    std::atomic<uint64_t> napiIndex_{1};
    ScheduleTaskManager scheduleTaskManager_;
    uint32_t nmdFd_ = 0;
    uint32_t dataFlushSize_ = 0;
};

#endif // STACK_PREPROCESS_H