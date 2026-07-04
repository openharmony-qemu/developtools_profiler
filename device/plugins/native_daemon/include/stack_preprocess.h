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
#include <functional>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <variant>
#include <numeric>
#include <cmath>
#include <inttypes.h>
#include "hook_record_factory.h"
#include "hook_record.h"
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
#include "common.h"
#include "stack_builder.h"
#include "unique_stack_table.h"
#include "stack_preprocess_types.h"

namespace OHOS::Developtools::NativeDaemon {
using WriterStructPtr = std::unique_ptr<WriterStruct>::pointer;

class StackPreprocess : public std::enable_shared_from_this<StackPreprocess> {
public:
    explicit StackPreprocess(const StackDataRepeaterPtr& dataRepeater, const NativeHookConfig& hookConfig,
        clockid_t pluginDataClockId, FILE* fpHookData = nullptr, bool isHookStandalone = false,
        bool isSaService = false, bool isProtobufSerialize = true);
    ~StackPreprocess();
    void SetWriter(const std::shared_ptr<Writer>& writer);
    void SetWriter(const WriterStructPtr& writer);
    void SetWriterForFdSa(uint32_t fd);
    void SetFactory(std::shared_ptr<HookRecordFactory> factory);
    void SetServerCommandCallback(std::function<void(uint32_t)> callback);
    bool StartTakeResults();
    bool StopTakeResults();
    void FinishTraceFile();
    bool FlushRecordStatistics();
    void FlushRecordApplyAndReleaseMatchData();
    void WaitForFinalSnapshot();
    void ForceStop();
    void SetDataSourceTerminated(bool dataSourceTerminated = true);
    bool DrainPendingShmemData(const std::shared_ptr<ShareMemoryBlock>& shareMemoryBlock);
    bool IsDataSourceTerminated() const;
    void SetPid(int32_t pid);
    void InitStatisticsTime();
    void UpdateStatisticsTime();
    void SetStopTakeData();
    void UpdateLastCacheFileActivityTime();
    void SaveMemTag(uint32_t tagId, const std::string& tagName);
    bool GetMemTag(uint32_t tagId, std::string& tagName);
    void SaveJsRawStack(uint64_t jsChainId, const char* jsRawStack);
    const char* GetJsRawStack(uint64_t jsChainId);
    void ReportBasicData();
    void WriteHookConfig();
    bool BuildCallFrames(const std::shared_ptr<HookRecord>& hookRecord, RawStackPtr& rawData,
        std::vector<CallFrame>& callFrames);
    void TakeResultsFromShmem(const std::shared_ptr<EventNotifier>&, const std::shared_ptr<ShareMemoryBlock>&);
    void TakeResultsFromCacheFile(int fd);
    void InitializeDirector();
#ifdef NATIVE_DAEMON_UNITTEST
    static std::shared_ptr<BuildStackDirector> GetDirectorForTest();
    static void SetDirectorForTest(std::shared_ptr<BuildStackDirector> director);
    static void ClearThreadLocalVariables();
#endif
    bool ProcessRawData(const int8_t data[], uint32_t size,
        RawStackPtr rawData, std::shared_ptr<HookRecord> hookRecord);
    void SetNmdFd(uint32_t fd);
    void SetFlushSize(uint64_t size);
    void SetIsStartTakeData(bool isStartTakeData);
    void SetIsStopTakeData(bool isStopTakeData);
    void SetIsCacheFileParseDone(bool isCacheFileParseDone);
private:
    using CallFrame = OHOS::Developtools::NativeDaemon::CallFrame;
    using DmaFdSizeMap = std::unordered_map<uint32_t, size_t>;
    struct ElfSymbolTable {
        uint64_t textVaddr;
        uint32_t textOffset;
        uint32_t symEntSize;
        std::vector<uint8_t> strTable;
        std::vector<uint8_t> symTable;
    };

    enum RecordStatisticsLimit : std::size_t {
        STATISTICS_MAP_SIZE = 100000,
        STATISTICS_PERIOD_DATA_SIZE = 100000,
        ALLOC_ADDRMAMP_SIZE = 100000,
        MATCH_ADDRMAMP_SIZE = 100000,
    };

    struct ParseContext {
        uint32_t offset;
        uint64_t readfileSize;
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
    bool CheckIsFileCacheFinsh();
    bool IsCacheFileInactiveWithTime(uint32_t seconds);
    void StopCacheFileTask();
    bool ProcessBatchData();
    bool ProcessSingleRecord(HookRecordPtr& hookRecord);
    void BuildAndReportStack(HookRecordPtr& hookRecord);
    void CleanupBatchRecords(HookRecordPtr batchRawStack[], uint32_t maxCount);
    bool HandleSimpleMessages(HookRecordPtr& hookRecord);
    template <typename T>
    void SetHookData(HookRecordPtr hookRecord, T& stackData);
    template <typename T>
    void SetHookData(HookRecordPtr hookRecord, std::vector<CallFrame>& callFrames, T& stackData);
    void WriteFrames(HookRecordPtr hookRecord, const std::vector<CallFrame>& callFrames);
    template <typename T>
    void SetFrameInfo(T& frame, CallFrame& callFrame);
    template <typename T>
    void ReportSymbolNameMap(CallFrame& callFrame, T& stackData);
    template <typename T>
    void ReportFilePathMap(CallFrame& callFrame, T& stackData);
    template <typename T>
    void ReportFrameMap(CallFrame& callFrame, T& stackData);
    void ReportThreadNameMap(uint32_t tid, const std::string& tname);
    bool ParseRecordsFromBuffer(uint8_t* buffer, uint32_t totalAvail, ParseContext& ctx,
        RawStackPtr& rawData, std::shared_ptr<HookRecord>& hookRecord);
    void SetMapsInfo();
    template <typename T>
    void SetSymbolInfo(uint32_t filePathId, ElfSymbolTable& symbolInfo, T& batchNativeHookData);
    template <typename T>
    void FlushCheck(T& stackData);
    void FlushData(BatchNativeHookData& stackData);
    void FlushData(OHOS::Developtools::Profiler::ProtoEncoder::BatchNativeHookData& stackData);
    void Flush(const uint8_t* src, size_t size);
    void GetSymbols(const std::string& filePath, ElfSymbolTable& symbols);
    std::string GetBuildId(const std::string& soName);
    template <typename T>
    void ProcessStackData(std::shared_ptr<HookRecord>& hookRecord, std::vector<CallFrame>& callFrames, T& stackData);
    template <typename T>
    void FillOfflineCallStack(std::vector<CallFrame>& callFrames, size_t idx, T& stackData);
    template <typename T>
    void FillCallStack(std::vector<CallFrame>& callFrames, size_t idx, T& stackData);
    template <typename T>
    uint32_t SetCallStackMap(T& stackData, const HookRecordPtr& hookRecord);
    template <typename T>
    uint32_t GetCallStackId(const HookRecordPtr& hookRecord, std::vector<CallFrame>& callFrames, T& stackData);
    uint32_t FindCallStackId(std::vector<uint64_t>& callStack, const HookRecordPtr& hookRecord);
    template <typename T>
    void SetEventFrame(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames,
        T* event, uint32_t stackId, const std::string& type = "");
    template <typename T>
    void SetEventFrame(const ReportEventBaseData& rawStack, T* event, uint32_t stackMapId,
                       const std::string& type = "");
    template <typename T>
    void SetAllocStatisticsFrame(const HookRecordPtr& hookRecord, std::vector<CallFrame>& callFrames, T& stackData);
    template <typename T>
    void SetAllocStatisticsFrame(const HookRecordPtr& hookRecord, T& stackData);
    template <typename T>
    void SetApplyAndReleaseMatchFrame(HookRecordPtr hookRecord, std::vector<CallFrame>& callFrames, T& stackData);
    void IntervalFlushRecordStatistics(const struct timespec& eventTime);
    void IntervalFlushApplyAndReleaseMatchData();
    bool HandleNoStackEvent(HookRecordPtr& hookRecord);
    bool SetFreeStatisticsData(uint64_t addr);
    bool SetFreeStatisticsDataLocked(uint64_t addr);
    template <typename T>
    void RemoveFreeAddrFromStatisticsLocked(std::unordered_map<uint32_t, T>& statisticsMap,
        uint64_t addr, uint32_t stackId);
    void RemoveFreeAddrFromStatisticsLocked(uint64_t addr, uint32_t stackId);
    void ReconcileWithSnapshot(const std::unordered_set<uint64_t>& aliveAddrs);
    bool ReconcileDuplicateMalloc(uint64_t addr, const struct timespec& newTs);
    void SetAllocStatisticsData(const HookRecordPtr& hookRecord, size_t stackId, bool isExists = false);
    void UpdateAllocStatistics(const HookRecordPtr& hookRecord, size_t stackId);
    template <typename T>
    void UpdateAllocStatistics(std::unordered_map<uint32_t, T>& statisticsMap,
        const HookRecordPtr& hookRecord, size_t stackId);
    void UpdateAllocSimplifiedStatistics(const HookRecordPtr& hookRecord, size_t stackId);
    bool ResolveTagType(const std::string& tagName, RecordStatisticsEvent::MemoryType& type);
    static bool NeedCollectAddr(RecordStatisticsEvent::MemoryType type);
    template <typename T>
    void FillRecordFromType(T& record, const HookRecordPtr& hookRecord);
    template <typename T>
    void RegisterAllocAddress(std::unordered_map<uint32_t, T>& statisticsMap,
        const HookRecordPtr& hookRecord, size_t stackId);
    template <typename T>
    void AddAllocStatistics(std::unordered_map<uint32_t, T>& statisticsMap,
        const HookRecordPtr& hookRecord, size_t stackId);
    void AddAllocStatistics(const HookRecordPtr& hookRecord, size_t stackId);
    bool TryProcessCachedAsyncStatistics(std::shared_ptr<HookRecord>& hookRecord);
    uint32_t FindAsyncStatisticsStackIdLocked(const HookRecordPtr& hookRecord);
    void HandleDeleteAddr(uint64_t addr, size_t size);
    template <typename T>
    void HandleMoveAddr(std::unordered_map<uint32_t, T>& statisticsMap, uint64_t addr, uint64_t new_addr, size_t size);
    std::vector<uint64_t> FindAddrs(uint64_t addr, size_t size);
    void HandleMoveAddr(uint64_t addr, uint64_t new_addr, size_t size);
    void InitializeGpuData();
    void ReportOfflineSymbolizationData();

    RandomWriteCtx* StartReport();
    int32_t FinishReport();
    void FillNapiStack(std::string& tagName, std::vector<CallFrame>& callFrames, uint64_t napiIndex);
    bool ProcessAddressData(const int8_t data[]);
    bool ValidateAndPrepareData(const int8_t data[],
        uint32_t size, std::shared_ptr<HookRecord>& hookRecord, RawStackPtr& rawData);
    bool ProcessNmdMsg(RawStackPtr& rawData);
    bool ProcessEndMsg();
    bool ProcessStacklessMunmap(std::shared_ptr<HookRecord>& hookRecord, RawStackPtr& rawData);
    bool ProcessRegularStackData(std::shared_ptr<HookRecord>& hookRecord, RawStackPtr& rawData);
    bool ResolveDmaSize(HookRecordPtr& hookRecord, DmaFdSizeMap& dmaFdSizeMap, uint32_t& dmaFdSizePid);
    bool IsDmaUsingRecord(const HookRecordPtr& hookRecord) const;
    bool ParseDmaInfoLine(const std::string& line, uint32_t targetPid, DmaFdSizeMap& dmaFdSizeMap) const;
    DmaFdSizeMap LoadDmaFdSizeMap(uint32_t pid) const;
    bool PassesSizeFilter(size_t size) const;
    uint32_t GetDmaFd(uint64_t addr) const;
    bool GetPcsByStackId(StackId stackId, std::vector<uintptr_t>& pcs, const std::string& errorMsg);
    bool BuildNestedAsyncCallFrames(const AsyncStackData* asyncData,
        const std::shared_ptr<HookRecord>& hookRecord, std::vector<CallFrame>& callFrames);
    bool BuildRegularAsyncCallFrames(const AsyncStackData* asyncData,
        const std::shared_ptr<HookRecord>& hookRecord, std::vector<CallFrame>& callFrames);
    void LogPerformanceMetrics(const timespec& start, const std::string& operation, uint16_t type, uint32_t fpDepth);
    void LogPerformanceMetrics(const struct timespec& start, const std::string& operation);
    void CleanupFullyReleasedStack(uint32_t stackId);
    void InitializeFunctionPointers();
    uint32_t GetNewStackId();
    bool IsStackIdExistsInStatistics(uint32_t stackId);

    template <typename T>
    bool FlushRecordStatisticsImpl(std::unordered_map<uint32_t, T>& statisticsMap);

    uint32_t FindAsyncCallStackId(const HookRecordPtr& hookRecord);
    uint32_t FindSimplifiedAsyncCallStackId(const HookRecordPtr& hookRecord);
    uint32_t FindFullAsyncCallStackId(const HookRecordPtr& hookRecord);
    uint32_t FindFullAsyncStatisticsStackId(const HookRecordPtr& hookRecord);

    uint32_t FindSyncCallStackId(std::vector<uint64_t>& callStack, const HookRecordPtr& hookRecord);
    uint32_t FindSimplifiedSyncCallStackId(std::vector<uint64_t>& callStack, const HookRecordPtr& hookRecord);

    void AddCallStackIndex(uint32_t stackId, RawStackPtr rawData);
    void AddSimplifiedCallStackIndex(uint32_t stackId, RawStackPtr rawData);

    template <typename T>
    void UpdateMoveAddrRecord(std::unordered_map<uint32_t, T>& statisticsMap,
        uint64_t addr, uint64_t newAddr, size_t size);
    template <typename T>
    void FindAndUpdateFreeStatisticsLocked(std::unordered_map<uint32_t, T>& statisticsMap,
        uint32_t stackId, uint64_t mallocSize);
    void FindAndUpdateFreeStatisticsLocked(uint32_t stackId, uint64_t mallocSize);

    template <typename T, typename StackData>
    void FlushStatisticsEntry(T* statistics, uint32_t stackId, StackData& stackData);
private:
    std::chrono::steady_clock::time_point lastStatisticsTime_ = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point lastCacheFileActivityTime_ = std::chrono::steady_clock::now();
    std::mutex cacheFileActivityTimeMtx_;
    struct timespec lastCacheStatisticsTime_ = {0, 0};
    std::shared_ptr<Writer> writer_ = nullptr;
    StackDataRepeaterPtr dataRepeater_ = nullptr;
    std::thread thread_ {};
    std::unique_ptr<uint8_t[]> buffer_;
    std::atomic_bool isStopTakeData_ = false;
    std::atomic_bool isStartTakeData_ = false;
    std::atomic_bool isCacheFileParseDone_ = false;
    std::atomic_bool dataSourceTerminated_ = false;
    std::atomic_bool waitingCacheFileStop_ = false;
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime_instance;
    DISALLOW_COPY_AND_MOVE(StackPreprocess);
    OHOS::SafeMap<uint32_t, std::string> memTagMap_ = {};
    std::unordered_map<uint32_t, std::string> threadNameMap_ = {};
    NativeHookConfig hookConfig_;
    uint32_t ignoreCnts_ = 0;
    uint32_t eventCnts_ = 0;
    bool flushBasicData_ {true};
    std::vector<u64> u64regs_;
    std::unordered_map<CallStackKey, uint32_t, CallStackKeyHash> callStackIndex_;
    std::unordered_map<SimplifiedCallStackKey, uint32_t, SimplifiedCallStackKeyHash>
        simplifiedCallStackIndex_;

    using FindSyncFunc = uint32_t(StackPreprocess::*)(std::vector<uint64_t>&, const HookRecordPtr&);
    FindSyncFunc findSyncCallStackIdFunc_ = &StackPreprocess::FindSimplifiedSyncCallStackId;
    std::unordered_map<AsyncCallStackKey, uint32_t, AsyncCallStackKeyHash> asyncCallStackIndex_;
    std::unordered_map<SimplifiedAsyncCallStackKey, uint32_t, SimplifiedAsyncCallStackKeyHash>
        simplifiedAsyncCallStackIndex_;

    using FindAsyncFunc = uint32_t(StackPreprocess::*)(const HookRecordPtr&);
    FindAsyncFunc findAsyncCallStackIdFunc_ = &StackPreprocess::FindSimplifiedAsyncCallStackId;
    FindAsyncFunc findAsyncStatisticsStackIdFunc_ = &StackPreprocess::FindSimplifiedAsyncCallStackId;
    using AddIndexFunc = void(StackPreprocess::*)(uint32_t, RawStackPtr);
    AddIndexFunc addCallStackIndexFunc_ = &StackPreprocess::AddSimplifiedCallStackIndex;

    // Key is callStack_, value is call stack id
    std::unordered_map<NewCallStack, uint32_t, NewCallStackHash> callStackMap_;
    std::chrono::seconds statisticsInterval_ {0};
    // Key is call stack id, value is recordstatistic data
    std::unordered_map<uint32_t, RecordStatistic> recordStatisticsMap_;
    std::unordered_map<uint32_t, SimplifiedRecordStatistics> simplifiedRecordStatisticsMap_;
    // Key is call stack id, value is recordstatistic data pointer
    std::unordered_set<uint32_t> statisticsPeriodSet_;
    // Key is alloc or mmap address, value is ReportEventBaseData list iterator
    std::unordered_map<uint64_t, std::list<ReportEventBaseData>::iterator> applyAndReleaseMatchIntervallMap_;
    std::list<ReportEventBaseData> applyAndReleaseMatchPeriodListData_;
    std::list<ReportEventBaseData> releasedPeriodListData_;
    std::chrono::seconds applyAndReleaseMatchInterval_{0};
    // used for plugin data
    clockid_t pluginDataClockId_ = CLOCK_REALTIME;
    // used for clac wait time in StackDataRepeater::TakeRawData() or statistics HookData
    clockid_t hookDataClockId_ = CLOCK_REALTIME;
    FILE* fpHookData_ {nullptr};
    bool isHookStandaloneSerialize_ {false};
    int32_t pid_ {-1};
    std::mutex mtx_;
    std::mutex jsMapMtx_;
    COMMON::SpinLock spinLock_;
    bool isSaService_{false};
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
    std::atomic<uint64_t> napiIndex_{1};
    std::atomic<int> endMsgCount_ {0};
    ScheduleTaskManager scheduleTaskManager_;
    uint32_t nmdFd_ = 0;
    uint32_t dataFlushSize_ = 0;
    std::shared_ptr<HookRecordFactory> factory_{nullptr};
    std::set<uint64_t> addrSet_;
    std::unordered_map<uint64_t, AllocAddrInfo> allocAddrMap_;
    std::function<void(uint32_t)> serverCommandCallback_;
    std::unordered_set<uint64_t> snapshotAddrs_;
    std::atomic_bool waitingFinalSnapshot_{false};
    std::atomic_bool finalSnapshotDone_{false};
    std::mutex snapshotMtx_;
    std::condition_variable snapshotCv_;
    bool discardDestroyedTraces_ = false;
};
}

#include "stack_preprocess_inl.h"

#endif // STACK_PREPROCESS_H
