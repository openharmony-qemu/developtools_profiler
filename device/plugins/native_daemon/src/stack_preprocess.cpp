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

#include "stack_preprocess.h"

#include <elf.h>
#include <type_traits>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include "cache_file_reader.h"
#include "common.h"
#include "logging.h"
#include "plugin_service_types.pb.h"
#include "elf_factory.h"
#include "utilities.h"
#include "native_hook_result_standard.pb.h"
#include "native_hook_config_standard.pb.h"
#include "google/protobuf/text_format.h"
#include "hook_common.h"
#include "trace_file_writer.h"
#include "unique_stack_table.h"
#include "dfx_elf.h"

constexpr uint32_t MAX_MATCH_CNT = 1000;
constexpr uint32_t MAX_MATCH_INTERVAL = 3600;
constexpr uint32_t LOG_PRINT_TIMES = 10000;
constexpr uint32_t WAIT_STOP_TIME = 5000;
constexpr uint32_t FINAL_SNAPSHOT_TIMEOUT_MS = 3000;
constexpr uint32_t WAIT_TIME_ONCE = 10;
#ifdef NATIVE_DAEMON_UNITTEST
constexpr uint32_t MAX_FILE_CACHE_INACTIVE_SECONDS = 2;
#else
constexpr uint32_t MAX_FILE_CACHE_INACTIVE_SECONDS = 60;
#endif
constexpr uint32_t WAIT_FILE_CACHE_STOP_CNT = 3600;
constexpr uint32_t SECOND_TO_MILLISECOND = 1000;
constexpr uint32_t MILLISECOND_TO_NANOSECOND = 1000 * 1000;
constexpr uint32_t MAX_BATCH_CNT = 40;

constexpr uint64_t JS_OFFLINE_IP_MASK = 0xFFFFFE0000000000;
constexpr uint64_t DWARF_NAPI_CALLBACK = 999999;
constexpr uint64_t STATISTICS_PERIOD_DATA_FLUSH_BATCH = 10000;
constexpr int NAPI_CALL_STACK = 2; // just for napi call stack
constexpr uint32_t FRAME_DEPTH = 2; // add two frames
constexpr uint32_t FLUSH_BASELINE_SA = (1U << 20);
#ifdef PERFORMANCE_DEBUG
constexpr uint32_t LONG_TIME_THRESHOLD = 1000000;
static std::atomic<uint64_t> timeCost = 0;
static std::atomic<uint64_t> unwindTimes = 0;
#endif
using namespace OHOS::Developtools::NativeDaemon;
using namespace OHOS::HiviewDFX;
using namespace OHOS::Developtools::Profiler;

thread_local std::vector<uint64_t> g_callStack;

thread_local std::shared_ptr<BuildStackDirector> g_director{nullptr};

#ifdef NATIVE_DAEMON_UNITTEST
std::vector<uint64_t>& StackPreprocess::GetThreadCallStack()
{
    return g_callStack;
}
#endif

static uint64_t TimespecToMilliseconds(const struct timespec& time)
{
    return static_cast<uint64_t>(time.tv_sec) * SECOND_TO_MILLISECOND +
        static_cast<uint64_t>(time.tv_nsec) / MILLISECOND_TO_NANOSECOND;
}

static struct timespec MillisecondsToTimespec(uint64_t timeMs)
{
    return {static_cast<time_t>(timeMs / SECOND_TO_MILLISECOND),
        static_cast<long>((timeMs % SECOND_TO_MILLISECOND) * MILLISECOND_TO_NANOSECOND)};
}

namespace {
struct TagMapping {
    const char* tagName;
    RecordStatisticsEvent::MemoryType type;
    bool needNodeInfo;
};

static TagMapping g_tagMappings[] = {
    {TAG_RES_GPU_VK, RecordStatisticsEvent::GPU_VK, true},
    {TAG_RES_GPU_GLES_IMAGE, RecordStatisticsEvent::GPU_GLES, true},
    {TAG_RES_GPU_GLES_BUFFER, RecordStatisticsEvent::GPU_GLES, true},
    {TAG_RES_GPU_CL_IMAGE, RecordStatisticsEvent::GPU_CL, true},
    {TAG_RES_GPU_CL_BUFFER, RecordStatisticsEvent::GPU_CL, true},
    {TAG_RES_FD_OPEN, RecordStatisticsEvent::FD, false},
    {TAG_RES_FD_EPOLL, RecordStatisticsEvent::FD, false},
    {TAG_RES_FD_EVENTFD, RecordStatisticsEvent::FD, false},
    {TAG_RES_FD_SOCKET, RecordStatisticsEvent::FD, false},
    {TAG_RES_FD_PIPE, RecordStatisticsEvent::FD, false},
    {TAG_RES_FD_DUP, RecordStatisticsEvent::FD, false},
    {TAG_RES_FD_ALL, RecordStatisticsEvent::FD, false},
    {TAG_RES_THREAD_PTHREAD, RecordStatisticsEvent::THREAD, false},
    {TAG_RES_THREAD_ALL, RecordStatisticsEvent::THREAD, false},
    {TAG_RES_ARKTS_HEAP_MASK, RecordStatisticsEvent::ARKTS_HEAP, false},
    {TAG_RES_ARKTS_STATIC_HEAP, RecordStatisticsEvent::ARKTS_STATIC_HEAP, false},
    {TAG_RES_JS_HEAP_MASK, RecordStatisticsEvent::JS_HEAP, false},
    {TAG_RES_KOTLIN_HEAP_MASK, RecordStatisticsEvent::KMP_HEAP, false},
    {TAG_RES_DMABUF_MASK, RecordStatisticsEvent::DMA, true},
    {TAG_RES_ARK_GLOBAL_HANDLE, RecordStatisticsEvent::ARK_GLOBAL_HANDLE, false},
    {TAG_RES_ARK_LOCAL_HANDLE, RecordStatisticsEvent::ARK_LOCAL_HANDLE, false},
    {TAG_RES_VMA_ARKWEB, RecordStatisticsEvent::VMA_ARKWEB, false},
    {TAG_RES_DART_HEAP, RecordStatisticsEvent::DART_HEAP, false},
    {TAG_RES_RN_HERMES_HEAP, RecordStatisticsEvent::RN_HERMES_HEAP, false},
};

bool NeedBuildIdForPath(const CallFrame& callFrame)
{
    if (callFrame.isJsFrame_) {
        return false;
    }
    std::string_view filePath = callFrame.filePath_;
    if (filePath.empty()) {
        return false;
    }
    constexpr std::array<std::string_view, 6> scriptExts = {
        ".ts", ".ets", ".js", ".mjs", ".cjs", ".abc"
    };
    for (const auto ext : scriptExts) {
        if (filePath.size() >= ext.size() && filePath.substr(filePath.size() - ext.size()) == ext) {
            return false;
        }
    }
    return true;
}
}

std::vector<uint64_t> StackPreprocess::FindAddrs(uint64_t addr, size_t size)
{
    std::vector<uint64_t> addrs = {};
    spinLock_.Lock();
    auto it = addrSet_.lower_bound(addr);
    for (; it != addrSet_.end() && *it < addr + size; ++it) {
        addrs.push_back(*it);
    }
    spinLock_.Unlock();
    return addrs;
}

void StackPreprocess::HandleDeleteAddr(uint64_t addr, size_t size)
{
    std::vector<uint64_t> addrs = FindAddrs(addr, size);
    for (auto addr : addrs) {
        SetFreeStatisticsData(addr);
    }
}

void StackPreprocess::ReconcileWithSnapshot(const std::unordered_set<uint64_t>& aliveAddrs)
{
    std::vector<uint64_t> staleAddrs;
    spinLock_.Lock();
    for (const auto& [addr, info] : allocAddrMap_) {
        if (aliveAddrs.find(addr) == aliveAddrs.end()) {
            staleAddrs.push_back(addr);
        }
    }
    for (uint64_t addr : staleAddrs) {
        SetFreeStatisticsDataLocked(addr);
    }
    spinLock_.Unlock();
}

template <typename T>
void StackPreprocess::UpdateMoveAddrRecord(
    std::unordered_map<uint32_t, T>& statisticsMap, uint64_t addr, uint64_t newAddr, size_t size)
{
    auto addrIter = allocAddrMap_.find(addr);
    if (addrIter == allocAddrMap_.end()) {
        return;
    }
    auto recordIter = statisticsMap.find(addrIter->second.stackId);
    if (recordIter == statisticsMap.end()) {
        return;
    }
    auto& record = recordIter->second;
    record.applySize += (size - allocAddrMap_[addr].mallocSize);
    if (NeedCollectAddr(record.type)) {
        auto addrPos = std::find(record.addrs.begin(), record.addrs.end(), addr);
        if (addrPos != record.addrs.end()) {
            *addrPos = newAddr;
        }
    }
    allocAddrMap_[newAddr] = AllocAddrInfo{size, record.callstackId, addrIter->second.ts};
    allocAddrMap_.erase(addr);
    addrSet_.insert(newAddr);
    addrSet_.erase(addr);
}

template <typename T>
inline void StackPreprocess::HandleMoveAddr(
    std::unordered_map<uint32_t, T>& statisticsMap, uint64_t addr, uint64_t newAddr, size_t size)
{
    spinLock_.Lock();
    UpdateMoveAddrRecord(statisticsMap, addr, newAddr, size);
    spinLock_.Unlock();
}

void StackPreprocess::HandleMoveAddr(uint64_t addr, uint64_t newAddr, size_t size)
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        HandleMoveAddr(simplifiedRecordStatisticsMap_, addr, newAddr, size);
    } else {
        HandleMoveAddr(recordStatisticsMap_, addr, newAddr, size);
    }
}

StackPreprocess::StackPreprocess(const StackDataRepeaterPtr& dataRepeater, const NativeHookConfig& hookConfig,
    clockid_t pluginDataClockId, FILE* fpHookData, bool isHookStandalone, bool isSaService, bool isProtobufSerialize)
    : dataRepeater_(dataRepeater), hookConfig_(hookConfig), pluginDataClockId_(pluginDataClockId),
      fpHookData_(fpHookData), isHookStandaloneSerialize_(isHookStandalone), isSaService_(isSaService),
      isProtobufSerialize_(isProtobufSerialize)
{
    runtime_instance = std::make_shared<VirtualRuntime>(hookConfig_);
    if (hookConfig_.malloc_free_matching_interval() > MAX_MATCH_INTERVAL) {
        PROFILER_LOG_INFO(LOG_CORE, "Not support set %d", hookConfig_.malloc_free_matching_interval());
        hookConfig_.set_malloc_free_matching_interval(MAX_MATCH_INTERVAL);
    }

    if (hookConfig_.malloc_free_matching_cnt() > MAX_MATCH_CNT) {
        PROFILER_LOG_INFO(LOG_CORE, "Not support set %d", hookConfig_.malloc_free_matching_cnt());
        hookConfig_.set_malloc_free_matching_cnt(MAX_MATCH_CNT);
    }
    PROFILER_LOG_INFO(LOG_CORE, "malloc_free_matching_interval = %d malloc_free_matching_cnt = %d\n",
        hookConfig_.malloc_free_matching_interval(), hookConfig_.malloc_free_matching_cnt());

    if (hookConfig_.statistics_interval() > 0) {
        statisticsInterval_ = std::chrono::seconds(hookConfig_.statistics_interval());
        discardDestroyedTraces_ = hookConfig_.discard_destroyed_traces();
        if (!hookConfig_.arkui_mem_trace_enable()) {
            simplifiedRecordStatisticsMap_.reserve(STATISTICS_MAP_SIZE);
        } else {
            recordStatisticsMap_.reserve(STATISTICS_MAP_SIZE);
        }
        statisticsPeriodSet_.reserve(STATISTICS_PERIOD_DATA_SIZE);
        allocAddrMap_.reserve(ALLOC_ADDRMAMP_SIZE);
    }
    InitializeFunctionPointers();
    if (hookConfig_.malloc_free_matching_interval() > 0) {
        applyAndReleaseMatchInterval_ = std::chrono::seconds(hookConfig_.malloc_free_matching_interval());
        applyAndReleaseMatchIntervallMap_.reserve(MATCH_ADDRMAMP_SIZE);
    }
    PROFILER_LOG_INFO(LOG_CORE, "statistics_interval = %d statisticsInterval_ = %lld \n",
        hookConfig_.statistics_interval(), statisticsInterval_.count());
    PROFILER_LOG_INFO(LOG_CORE, "applyAndReleaseMatchInterval_ = %lld", applyAndReleaseMatchInterval_.count());
    hookDataClockId_ = COMMON::GetClockId(hookConfig_.clock());
    PROFILER_LOG_INFO(LOG_CORE, "StackPreprocess(): pluginDataClockId = %d hookDataClockId = %d \n",
        pluginDataClockId_, hookDataClockId_);
    if (hookConfig_.save_file() && fpHookData_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "If you need to save the file, please set the file_name");
    }
    if (hookConfig_.memtrace_enable()) {
        InitializeGpuData();
    }
    PROFILER_LOG_INFO(LOG_CORE, "isHookStandaloneSerialize_ = %d", isHookStandaloneSerialize_);
#ifdef PERFORMANCE_DEBUG
    timeCost = 0;
    unwindTimes = 0;
#endif
}

StackPreprocess::~StackPreprocess()
{
    isStopTakeData_ = true;
    if (dataRepeater_) {
        dataRepeater_->Close();
    }
    if (thread_.joinable()) {
        thread_.join();
    }
    runtime_instance = nullptr;
    fpHookData_ = nullptr;
}

void StackPreprocess::InitializeGpuData()
{
    SaveMemTag(GPU_VK_INDEX + 1, TAG_RES_GPU_VK);
    SaveMemTag(GPU_GLES_IMAGE_INDEX + 1, TAG_RES_GPU_GLES_IMAGE);
    SaveMemTag(GPU_GLES_BUFFER_INDEX + 1, TAG_RES_GPU_GLES_BUFFER);
    SaveMemTag(GPU_CL_IMAGE_INDEX + 1, TAG_RES_GPU_CL_IMAGE);
    SaveMemTag(GPU_CL_BUFFER_INDEX + 1, TAG_RES_GPU_CL_BUFFER);
    SaveMemTag(FD_OPEN_INDEX + 1, TAG_RES_FD_OPEN);
    SaveMemTag(FD_EPOLL_INDEX + 1, TAG_RES_FD_EPOLL);
    SaveMemTag(FD_EVENTFD_INDEX + 1, TAG_RES_FD_EVENTFD);
    SaveMemTag(FD_SOCKET_INDEX + 1, TAG_RES_FD_SOCKET);
    SaveMemTag(FD_PIPE_INDEX + 1, TAG_RES_FD_PIPE);
    SaveMemTag(FD_DUP_INDEX + 1, TAG_RES_FD_DUP);
    SaveMemTag(FD_MASK_INDEX + 1, TAG_RES_FD_ALL);
    SaveMemTag(THREAD_PTHREAD_INDEX + 1, TAG_RES_THREAD_PTHREAD);
    SaveMemTag(THREAD_MASK_INDEX + 1, TAG_RES_THREAD_ALL);
    SaveMemTag(ARKTS_HEAP_MASK_INDEX + 1, TAG_RES_ARKTS_HEAP_MASK);
    SaveMemTag(ARKTS_STATIC_HEAP_MASK_INDEX + 1, TAG_RES_ARKTS_STATIC_HEAP);
    SaveMemTag(JS_HEAP_MASK_INDEX + 1, TAG_RES_JS_HEAP_MASK);
    SaveMemTag(KOTLIN_HEAP_MASK_INDEX + 1, TAG_RES_KOTLIN_HEAP_MASK);
    SaveMemTag(DMABUF_MASK_INDEX + 1, TAG_RES_DMABUF_MASK);
    SaveMemTag(ARK_GLOBAL_HANDLE_INDEX + 1, TAG_RES_ARK_GLOBAL_HANDLE);
    SaveMemTag(ARK_LOCAL_HANDLE_INDEX + 1, TAG_RES_ARK_LOCAL_HANDLE);
    SaveMemTag(VMA_ARKWEB_INDEX + 1, TAG_RES_VMA_ARKWEB);
    SaveMemTag(DART_HEAP_INDEX + 1, TAG_RES_DART_HEAP);
    SaveMemTag(RN_HERMES_HEAP_INDEX + 1, TAG_RES_RN_HERMES_HEAP);
}

void StackPreprocess::FinishTraceFile()
{
    if (isSaService_ && (writer_ != nullptr)) {
        std::shared_ptr<TraceFileWriter> tfPtr = std::static_pointer_cast<TraceFileWriter>(writer_);
        tfPtr->SetDurationTime();
        tfPtr->Finish();
    }
}

void StackPreprocess::SetWriter(const std::shared_ptr<Writer>& writer)
{
    writer_ = writer;
    if ((!isSaService_) || (isHookStandaloneSerialize_)) {
        stackData_ = BatchNativeHookData();
    }
}

void StackPreprocess::SetWriterForFdSa(uint32_t fd)
{
    writer_ = std::make_shared<TraceFileWriter>(fd);
    std::shared_ptr<TraceFileWriter> tfPtr = std::static_pointer_cast<TraceFileWriter>(writer_);
    tfPtr->SetTimeSource();
    if ((!isSaService_) || (isHookStandaloneSerialize_)) {
        stackData_ = BatchNativeHookData();
    }
}

void StackPreprocess::SetWriter(const WriterStructPtr& writer)
{
    if (writer == nullptr) {
        return;
    }
    resultWriter_ = writer;
    auto ctx = resultWriter_->startReport(resultWriter_);
    if (ctx == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get RandomWriteCtx FAILED!", __func__);
        return;
    }
    if (isHookStandaloneSerialize_) {
        stackData_ = BatchNativeHookData();
    } else {
        stackData_ = ProtoEncoder::BatchNativeHookData(ctx);
    }
}

bool StackPreprocess::StartTakeResults()
{
    CHECK_NOTNULL(dataRepeater_, false, "data repeater null");

    std::weak_ptr<StackPreprocess> stackPreprocessPtr(shared_from_this());
    std::thread demuxer([stackPreprocessPtr] {
        if (auto ptr = stackPreprocessPtr.lock(); ptr != nullptr) {
            ptr->TakeResults();
        }
    });
    CHECK_TRUE(demuxer.get_id() != std::thread::id(), false, "demuxer thread invalid");

    thread_ = std::move(demuxer);
    isStopTakeData_ = false;
    SetIsStartTakeData(false);
    dataSourceTerminated_ = false;
    return true;
}

void StackPreprocess::SetStopTakeData()
{
    if (hookConfig_.use_file_cache_mode()) {
        SetIsStopTakeData(true);
    }
}

void StackPreprocess::SetDataSourceTerminated(bool dataSourceTerminated)
{
    dataSourceTerminated_ = dataSourceTerminated;
    PROFILER_LOG_INFO(LOG_CORE, "SetDataSourceTerminated: pid %d, terminated %d, hasRepeater %d",
        pid_, dataSourceTerminated, dataRepeater_ != nullptr);
    if (!dataSourceTerminated) {
        return;
    }
    if (dataRepeater_ == nullptr && !hookConfig_.use_file_cache_mode()) {
        SetIsStopTakeData(true);
    }
}

void StackPreprocess::UpdateLastCacheFileActivityTime()
{
    if (!waitingCacheFileStop_) {
        return;
    }
    std::lock_guard<std::mutex> lock(cacheFileActivityTimeMtx_);
    lastCacheFileActivityTime_ = std::chrono::steady_clock::now();
}

bool StackPreprocess::IsCacheFileInactiveWithTime(uint32_t seconds)
{
    std::lock_guard<std::mutex> lock(cacheFileActivityTimeMtx_);
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - lastCacheFileActivityTime_);
    return duration.count() > seconds;
}

void StackPreprocess::StopCacheFileTask()
{
    uint32_t waitCnt = 0;
    waitingCacheFileStop_ = true;
    UpdateLastCacheFileActivityTime();
    while (!isStopTakeData_ && waitCnt < WAIT_FILE_CACHE_STOP_CNT) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SECOND_TO_MILLISECOND));
        waitCnt++;
        // Check if there's inactivity for a max time
        if (IsCacheFileInactiveWithTime(MAX_FILE_CACHE_INACTIVE_SECONDS)) {
            PROFILER_LOG_INFO(LOG_CORE, "StopCacheFileTask: inactivity for %u seconds, stopping",
                MAX_FILE_CACHE_INACTIVE_SECONDS);
            break;
        }
    }
    ForceStop();
    waitingCacheFileStop_ = false;
}

bool StackPreprocess::StopTakeResults()
{
#ifdef PERFORMANCE_DEBUG
    if (unwindTimes > 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "Performance metrics - Times: %" PRIu64
                    " Total Cost: %" PRIu64 " Mean Cost: %" PRIu64,
                    unwindTimes.load(), timeCost.load(), timeCost.load() / unwindTimes.load());
    }
#endif
    PROFILER_LOG_INFO(LOG_CORE, "start StopTakeResults");
    if (!hookConfig_.use_file_cache_mode()) {
        if (dataSourceTerminated_) {
            PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults: data source already terminated, pid %d", pid_);
            if (!dataRepeater_) {
                ForceStop();
            }
        } else if (isStartTakeData_) {
            PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults: schedule timeout force stop, pid %d", pid_);
            int32_t timerFd = scheduleTaskManager_.ScheduleTask(
                std::bind(&StackPreprocess::ForceStop, this), WAIT_STOP_TIME, true);
            if (timerFd == -1) {
                PROFILER_LOG_ERROR(LOG_CORE, "StopTakeResults ScheduleTask failed!");
                return false;
            }
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults: no active data, force stop immediately, pid %d", pid_);
            ForceStop();
        }
    } else {
        PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults: enter file cache stop task, pid %d", pid_);
        if (pid_ < 0) {
            isStopTakeData_ = true;
        }
        StopCacheFileTask();
    }

    if (!dataRepeater_) {
        while (!isStopTakeData_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME_ONCE));
        }
        return true;
    }
    CHECK_NOTNULL(dataRepeater_, false, "data repeater null");
    CHECK_TRUE(thread_.get_id() != std::thread::id(), false, "thread invalid");

    PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults Wait thread join");

    if (thread_.joinable()) {
        thread_.join();
    }
    PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults Wait thread join success");
    return true;
}

void StackPreprocess::WaitForFinalSnapshot()
{
    if (!serverCommandCallback_) {
        return;
    }
    PROFILER_LOG_INFO(LOG_CORE, "WaitForFinalSnapshot: sending final snapshot request");
    waitingFinalSnapshot_ = true;
    finalSnapshotDone_ = false;
    serverCommandCallback_(FINAL_SNAPSHOT_REQUEST_CMD);
    std::unique_lock<std::mutex> lock(snapshotMtx_);
    bool completed = snapshotCv_.wait_for(lock,
        std::chrono::milliseconds(FINAL_SNAPSHOT_TIMEOUT_MS),
        [this]() { return finalSnapshotDone_.load(); });
    waitingFinalSnapshot_ = false;
    if (!completed) {
        PROFILER_LOG_WARN(LOG_CORE, "WaitForFinalSnapshot: timed out");
    }
}


inline void StackPreprocess::IntervalFlushRecordStatistics(const struct timespec& eventTime)
{
    // interval reporting statistics
    if (hookConfig_.statistics_interval() > 0) {
        if (hookConfig_.use_file_cache_mode()) {
            uint64_t lastTimeMs = TimespecToMilliseconds(lastCacheStatisticsTime_);
            uint64_t eventTimeMs = TimespecToMilliseconds(eventTime);
            if (lastTimeMs == 0 && eventTimeMs != 0) {
                lastCacheStatisticsTime_ = eventTime;
                return;
            }
            uint64_t intervalMs = static_cast<uint64_t>(hookConfig_.statistics_interval()) * SECOND_TO_MILLISECOND;
            uint64_t elapsedTimeMs = eventTimeMs > lastTimeMs ? (eventTimeMs - lastTimeMs) : 0;
            if (elapsedTimeMs >= intervalMs) {
                uint64_t intervalCount = elapsedTimeMs / intervalMs;
                lastCacheStatisticsTime_ = MillisecondsToTimespec(lastTimeMs + intervalCount * intervalMs);
                if (hookConfig_.allocator_sampling_enable() && serverCommandCallback_) {
                    serverCommandCallback_(SNAPSHOT_REQUEST_CMD);
                } else {
                    FlushRecordStatistics();
                }
            }
        } else {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastStatisticsTime_);
            if (elapsedTime >= statisticsInterval_) {
                lastStatisticsTime_ = currentTime;
                if (hookConfig_.allocator_sampling_enable() && serverCommandCallback_) {
                    serverCommandCallback_(SNAPSHOT_REQUEST_CMD);
                } else {
                    FlushRecordStatistics();
                }
            }
        }
    }
}

void StackPreprocess::IntervalFlushApplyAndReleaseMatchData()
{
    // interval reporting apply and release match data
    if (hookConfig_.malloc_free_matching_interval() > 0) {
        static auto lastStatisticsTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastStatisticsTime);
        if (elapsedTime >= applyAndReleaseMatchInterval_) {
            lastStatisticsTime = currentTime;
            FlushRecordApplyAndReleaseMatchData();
        }
    }
}

bool StackPreprocess::HandleNoStackEvent(HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    if (hookRecord->GetType() == MMAP_FILE_TYPE) {
        BaseStackRawData* mmapRawData = rawData->stackContext;
        std::string filePath(reinterpret_cast<char *>(rawData->data));
        COMMON::AdaptSandboxPath(filePath, rawData->stackContext->pid);
        std::lock_guard<std::mutex> guard(mtx_);
        runtime_instance->HandleMapInfo({reinterpret_cast<uint64_t>(mmapRawData->addr),
            mmapRawData->mallocSize, mmapRawData->mmapArgs.flags, mmapRawData->mmapArgs.offset}, filePath,
            rawData->stackContext->pid, rawData->stackContext->tid);
        flushBasicData_ = true;
    } else if (hookRecord->GetType() == THREAD_NAME_MSG) {
        std::string threadName = reinterpret_cast<char*>(rawData->data);
        ReportThreadNameMap(rawData->stackContext->tid, threadName);
    } else {
        return false;
    }
    return true;
}

void StackPreprocess::ForceStop()
{
    isStopTakeData_ = true;
    if (dataRepeater_ != nullptr) {
        dataRepeater_->Close();
    }
}

bool StackPreprocess::DrainPendingShmemData(const std::shared_ptr<ShareMemoryBlock>& shareMemoryBlock)
{
    CHECK_NOTNULL(shareMemoryBlock, false, "shareMemoryBlock is null!");
    InitializeDirector();

    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;
    bool drained = false;
    while (!isStopTakeData_) {
        bool ret = shareMemoryBlock->TakeData(
            [&](const int8_t data[], uint32_t size) -> bool {
                return ProcessRawData(data, size, rawData, hookRecord);
            });
        if (!ret) {
            break;
        }
        drained = true;
    }
    return drained;
}

void StackPreprocess::TakeResultsFromShmem(const std::shared_ptr<EventNotifier>& eventNotifier,
                                           const std::shared_ptr<ShareMemoryBlock>& shareMemoryBlock)
{
    eventNotifier->Take();
    DrainPendingShmemData(shareMemoryBlock);
}

bool StackPreprocess::ParseRecordsFromBuffer(uint8_t* buffer, uint32_t totalAvail, ParseContext& ctx,
    RawStackPtr& rawData, std::shared_ptr<HookRecord>& hookRecord, bool isCrashFile)
{
    while (ctx.offset + sizeof(uint32_t) <= totalAvail) {
        uint32_t totalSize = 0;
        if (isCrashFile) {
            if (!GetRecordHeadWithRetry(buffer, totalAvail, ctx.offset, totalSize, isCrashFile)) {
                return true;
            }
        } else {
            RecordHeadStatus headStatus = GetRecordHead(buffer, ctx.offset, isCrashFile, totalSize);
            if (headStatus == RECORD_HEAD_INVALID) {
                DumpInvalidRecordHead(buffer, totalAvail, ctx.offset, ctx.readfileSize);
                return false;
            }
            if (headStatus == RECORD_HEAD_END) {
                return true;
            }
        }

        if (ctx.offset + sizeof(uint32_t) + totalSize > totalAvail) {
            break;
        }

        ctx.offset += sizeof(uint32_t);
        ctx.readfileSize += sizeof(uint32_t) + totalSize;

        const int8_t* dataPtr = reinterpret_cast<const int8_t*>(buffer + ctx.offset);
        if (validLiveDataPtr_ != nullptr && IsBaseStackRawDataRecord(dataPtr, totalSize)) {
            auto* tmpRawData = reinterpret_cast<const BaseStackRawData*>(dataPtr);
            if (IsArkGlobalHandleRecord(tmpRawData) && ShouldSkipRecord(tmpRawData, *validLiveDataPtr_) &&
                !(IsFreeRecord(tmpRawData) && orphanFreeWhitelistPtr_ != nullptr &&
                  InOrphanFreeWhitelist(tmpRawData, *orphanFreeWhitelistPtr_))) {
                ctx.offset += totalSize;
                continue;
            }
        }
        if (!ProcessRawData(dataPtr, totalSize, rawData, hookRecord)) {
            if (isStopTakeData_) {
                ctx.offset += totalSize;
                break;
            }
            PROFILER_LOG_ERROR(LOG_CORE, "ParseRecordsFromBuffer ProcessRawData failed, skip current record");
            ctx.offset += totalSize;
            continue;
        }

        ctx.offset += totalSize;
    }
    return true;
}

void StackPreprocess::TakeResultsFromCacheFile(int fd)
{
    if (fd < 0) {
        PROFILER_LOG_INFO(LOG_CORE, "TakeResultsFromCacheFile parse all cache file done.");
        SetIsStopTakeData(true);
        return;
    }
    InitializeDirector();
    uint32_t dataSize = 0;
    if (!ReadCacheFileHeader(fd, dataSize)) {
        PROFILER_LOG_ERROR(LOG_CORE, "TakeResultsFromCacheFile read data size header failed, fd=%d errno=%d",
            fd, errno);
        close(fd);
        return;
    }
    PROFILER_LOG_INFO(LOG_CORE, "ReadCacheFileHeader fd=%d dataSize=%u", fd, dataSize);
    if (!SanitizeCacheDataSize(dataSize)) {
        PROFILER_LOG_WARN(LOG_CORE, "TakeResultsFromCacheFile dataSize exceeds max, fallback to crash mode");
    }
    CacheFileReadContext ctx;
    ctx.fd = fd;
    ctx.isCrashFile = (dataSize == 0);
    ctx.bytesToRead = ctx.isCrashFile ? MAX_CACHE_DATA_SIZE : dataSize;
    auto readBuffer = std::make_unique<uint8_t[]>(FILE_CACHE_READ_BUFFER_SIZE);
    ctx.readBuffer = readBuffer.get();
    bool ret = false;
    bool hasError = false;
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;
    PROFILER_LOG_INFO(LOG_CORE, "TakeResultsFromCacheFile dataSize=%u crashFile=%d",
        dataSize, static_cast<int>(ctx.isCrashFile));

    while (!isStopTakeData_ && !hasError) {
        uint32_t maxRead = GetCacheFileReadSize(ctx);
        if (ctx.readDone) {
            ret = true;
            break;
        }
        ssize_t bytesRead = read(ctx.fd, ctx.readBuffer + ctx.remainSize, maxRead);
        if (bytesRead < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "TakeResultsFromCacheFile read failed errno=%d", errno);
            hasError = true;
            break;
        }
        if (bytesRead == 0) {
            if (ctx.remainSize != 0 && !ctx.isCrashFile) {
                PROFILER_LOG_ERROR(LOG_CORE, "TakeResultsFromCacheFile incomplete record at EOF, remainSize=%u",
                    ctx.remainSize);
                hasError = true;
            } else {
                ret = true;
            }
            break;
        }

        uint32_t totalAvail = ctx.remainSize + static_cast<uint32_t>(bytesRead);
        ParseContext parseCtx = {0, ctx.readfileSize};
        UpdateLastCacheFileActivityTime();
        if (!ParseRecordsFromBuffer(ctx.readBuffer, totalAvail, parseCtx, rawData, hookRecord,
                                    ctx.isCrashFile)) {
            ret = ctx.isCrashFile;
            hasError = !ctx.isCrashFile;
            break;
        }

        ctx.readfileSize = parseCtx.readfileSize;
        ctx.remainSize = totalAvail - parseCtx.offset;
        if (ctx.remainSize > 0 && parseCtx.offset == 0) {
            if (!HandleOversizeRecord(ctx)) {
                hasError = true;
                break;
            }
            if (ctx.readDone) {
                ret = true;
                break;
            }
            continue;
        }
        if (ctx.remainSize > 0 &&
            memmove_s(ctx.readBuffer, FILE_CACHE_READ_BUFFER_SIZE, ctx.readBuffer + parseCtx.offset,
                      ctx.remainSize) != EOK) {
            PROFILER_LOG_ERROR(LOG_CORE, "TakeResultsFromCacheFile memmove_s failed");
            hasError = true;
            break;
        }
    }

    close(fd);
    PROFILER_LOG_INFO(LOG_CORE, "TakeResultsFromCacheFile [%llu]completed, ret %d.",
        static_cast<unsigned long long>(ctx.readfileSize), ret);
}

void StackPreprocess::InitializeDirector()
{
    if (g_director == nullptr) {
        g_director = std::make_shared<BuildStackDirector>(&hookConfig_, runtime_instance, mtx_);
    }
}

#ifdef NATIVE_DAEMON_UNITTEST
std::shared_ptr<BuildStackDirector> StackPreprocess::GetDirectorForTest()
{
    return g_director;
}

void StackPreprocess::SetDirectorForTest(std::shared_ptr<BuildStackDirector> director)
{
    g_director = director;
}

void StackPreprocess::ClearThreadLocalVariables()
{
    g_callStack.clear();
    g_director = nullptr;
}
#endif

bool StackPreprocess::ProcessRawData(const int8_t data[], uint32_t size,
                                              RawStackPtr rawData, std::shared_ptr<HookRecord> hookRecord)
{
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    if (size >= sizeof(uint32_t)) {
        uint32_t marker = *reinterpret_cast<const uint32_t*>(data);
        if (marker == ALIVE_ADDR_SNAPSHOT_MARKER) {
            const uint64_t* addrs = reinterpret_cast<const uint64_t*>(
                data + sizeof(uint32_t));
            size_t addrCount = (size - sizeof(uint32_t)) / sizeof(uint64_t);
            snapshotAddrs_.insert(addrs, addrs + addrCount);
            return true;
        }
        if (marker == ALIVE_ADDR_SNAPSHOT_END_MARKER) {
            ReconcileWithSnapshot(snapshotAddrs_);
            snapshotAddrs_.clear();
            FlushRecordStatistics();
            if (waitingFinalSnapshot_) {
                finalSnapshotDone_ = true;
                snapshotCv_.notify_one();
            }
            return true;
        }
    }

    if (size == sizeof(uint64_t)) {
        return ProcessAddressData(data);
    }

    if (!ValidateAndPrepareData(data, size, hookRecord, rawData)) {
        return false;
    }
    if (hookRecord->GetType() == FREE_MSG_SIMP) {
        uint64_t addr = hookRecord->GetAddr();
        factory_->SaveHookRecord(std::move(hookRecord));
        return ProcessAddressData(reinterpret_cast<const int8_t*>(&addr));
    }
    DmaFdSizeMap dmaFdSizeMap;
    uint32_t dmaFdSizePid = 0;
    if (!ResolveDmaSize(hookRecord, dmaFdSizeMap, dmaFdSizePid)) {
        factory_->SaveHookRecord(std::move(hookRecord));
        return true;
    }
    uint16_t type = hookRecord->GetType();

    // jemalloc may report duplicate malloc addresses; reconcile before expensive unwinding
    if (type == MALLOC_MSG && hookConfig_.statistics_interval() > 0) {
        uint64_t addr = reinterpret_cast<uint64_t>(rawData->stackContext->addr);
        if (!ReconcileDuplicateMalloc(addr, rawData->stackContext->ts)) {
            return true;
        }
    }

    SetIsStartTakeData(true);
    if (isStopTakeData_) {
        return false;
    } else if (type == MEMORY_TAG) {
        return true;
    } else if (type == FREE_ARKTS) {
        size_t size = hookRecord->GetRawStack()->stackContext->mallocSize;
        HandleDeleteAddr(reinterpret_cast<uint64_t>(hookRecord->GetAddr()), size);
    } else if (type == MALLOC_ARKTS) {
        size_t size = hookRecord->GetRawStack()->stackContext->mallocSize;
        auto addr = hookRecord->GetRawStack()->stackContext->addr;
        auto newAddr = hookRecord->GetRawStack()->stackContext->newAddr;
        HandleMoveAddr(reinterpret_cast<uint64_t>(addr), reinterpret_cast<uint64_t>(newAddr), size);
    } else if (HandleNoStackEvent(hookRecord)) {
        return true;
    } else if (type == MUNMAP_MSG) {
        {
            std::lock_guard<std::mutex> guard(mtx_);
            runtime_instance->RemoveMaps(reinterpret_cast<uint64_t>(hookRecord->GetAddr()));
        }
        if (rawData->stackSize == 0 && rawData->fpDepth == 0) {
            return ProcessStacklessMunmap(hookRecord, rawData);
        }
    } else if (type == NMD_MSG) {
        return ProcessNmdMsg(rawData);
    }  else if (type == END_MSG) {
        return ProcessEndMsg();
    } else if (type == JS_STACK_MSG) {
        SaveJsRawStack(rawData->stackContext->jsChainId, reinterpret_cast<char*>(rawData->data));
        return true;
    }
    return ProcessRegularStackData(hookRecord, rawData);
}

bool StackPreprocess::ProcessAddressData(const int8_t data[])
{
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    uint64_t addr = *reinterpret_cast<uint64_t *>(const_cast<int8_t *>(data));
    SetFreeStatisticsData(addr);

#ifdef PERFORMANCE_DEBUG
    LogPerformanceMetrics(start, "address processing");
    unwindTimes++;
    if (unwindTimes % LOG_PRINT_TIMES == 0 && unwindTimes > 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "Performance metrics - Times: %" PRIu64
                    " Total Cost: %" PRIu64 " Mean Cost: %" PRIu64,
                    unwindTimes.load(), timeCost.load(), timeCost.load() / unwindTimes.load());
    }
#endif
    return true;
}

bool StackPreprocess::ValidateAndPrepareData(const int8_t data[], uint32_t size,
                                             std::shared_ptr<HookRecord>& hookRecord,
                                             RawStackPtr& rawData)
{
    CHECK_TRUE(size >= sizeof(BaseStackRawData), false, "stack data invalid!");
    auto saveMemTagCallBack = [this] (uint32_t tagId, const std::string& tagName) {
        this->SaveMemTag(tagId, tagName);
    };
    hookRecord = factory_->GetHookRecord(data, size, false, saveMemTagCallBack);
    CHECK_NOTNULL(hookRecord, false, "hookRecord invalid!");
    rawData = hookRecord->GetRawStack();
    return true;
}

bool StackPreprocess::ProcessNmdMsg(RawStackPtr& rawData)
{
    const char* nmdResult = reinterpret_cast<const char*>(rawData->data);
    lseek(nmdFd_, 0, SEEK_END);
    (void)write(nmdFd_, nmdResult, strlen(nmdResult));
    return true;
}

bool StackPreprocess::ProcessEndMsg()
{
    if (hookConfig_.use_file_cache_mode()) {
        PROFILER_LOG_INFO(LOG_CORE, "Received END_MSG, stopping cache file data collection");
        SetIsStopTakeData(true);
        return false;
    }
    endMsgCount_++;
    int sharedMemCount = hookConfig_.offline_symbolization() ? SHARED_MEMORY_NUM : 1;
    if (endMsgCount_ == sharedMemCount) {
        PROFILER_LOG_INFO(LOG_CORE, "Received all END_MSG, stopping data collection");
        isStopTakeData_ = true;
        return false;
    }
    return true;
}

bool StackPreprocess::ProcessStacklessMunmap(std::shared_ptr<HookRecord>& hookRecord, RawStackPtr& rawData)
{
    std::visit([&](auto& stackData) {
        if (hookConfig_.malloc_free_matching_interval() == 0) {
            SetHookData(hookRecord, stackData);
            return;
        }
        ReportEventBaseData eventData(rawData->stackContext, 0);
        uint64_t addr = reinterpret_cast<uint64_t>(hookRecord->GetAddr());
        std::lock_guard<std::mutex> guard(mtx_);
        auto iter = applyAndReleaseMatchIntervallMap_.find(addr);
        if (iter != applyAndReleaseMatchIntervallMap_.end()) {
            applyAndReleaseMatchPeriodListData_.erase(iter->second);
            applyAndReleaseMatchIntervallMap_.erase(addr);
        } else {
            releasedPeriodListData_.emplace_back(eventData);
        }
    }, stackData_);
    if (hookRecord != nullptr) {
        factory_->SaveHookRecord(std::move(hookRecord));
        hookRecord = nullptr;
    }
    return true;
}

bool StackPreprocess::IsDmaUsingRecord(const HookRecordPtr& hookRecord) const
{
    if (hookRecord == nullptr) {
        return false;
    }
    auto rawData = hookRecord->GetRawStack();
    if (rawData == nullptr || rawData->stackContext == nullptr) {
        return false;
    }
    return rawData->stackContext->type == MEMORY_USING_MSG &&
        rawData->stackContext->tagId == DMABUF_MASK_INDEX + 1;
}

bool StackPreprocess::PassesSizeFilter(size_t size) const
{
    return size > 0 && hookConfig_.filter_size() >= 0 &&
        size >= static_cast<size_t>(hookConfig_.filter_size());
}

uint32_t StackPreprocess::GetDmaFd(uint64_t addr) const
{
    return static_cast<uint32_t>(addr & ~static_cast<uint64_t>(RES_DMABUF_MASK));
}

bool StackPreprocess::ParseDmaInfoLine(const std::string& line, uint32_t targetPid,
                                       DmaFdSizeMap& dmaFdSizeMap) const
{
    std::istringstream iss(line);
    std::string col;
    uint32_t pid = 0;
    uint32_t fd = 0;
    size_t size = 0;
    if (!(iss >> col >> pid >> fd >> size)) {
        return false;
    }
    if (pid == targetPid && size > 0) {
        dmaFdSizeMap[fd] = size;
    }
    return true;
}

StackPreprocess::DmaFdSizeMap StackPreprocess::LoadDmaFdSizeMap(uint32_t pid) const
{
    DmaFdSizeMap dmaFdSizeMap;
    char fileName[MAX_HOOK_PATH + 1] = {0};
    if (snprintf_s(fileName, sizeof(fileName), sizeof(fileName) - 1,
        "/proc/%u/mm_dmabuf_info", pid) < 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "LoadDmaFdSizeMap snprintf_s failed");
        return dmaFdSizeMap;
    }
    std::ifstream file(fileName);
    if (!file) {
        PROFILER_LOG_ERROR(LOG_CORE, "LoadDmaFdSizeMap open %s failed errno=%d", fileName, errno);
        return dmaFdSizeMap;
    }
    std::string line;
    while (std::getline(file, line)) {
        ParseDmaInfoLine(line, pid, dmaFdSizeMap);
    }
    return dmaFdSizeMap;
}

bool StackPreprocess::ResolveDmaSize(HookRecordPtr& hookRecord, DmaFdSizeMap& dmaFdSizeMap,
                                     uint32_t& dmaFdSizePid)
{
    if (!IsDmaUsingRecord(hookRecord)) {
        return true;
    }
    auto rawData = hookRecord->GetRawStack();
    uint32_t pid = rawData->stackContext->pid;
    if (dmaFdSizePid != pid) {
        dmaFdSizeMap = LoadDmaFdSizeMap(pid);
        dmaFdSizePid = pid;
    }
    uint32_t fd = GetDmaFd(reinterpret_cast<uint64_t>(rawData->stackContext->addr));
    auto iter = dmaFdSizeMap.find(fd);
    if (iter == dmaFdSizeMap.end() || !PassesSizeFilter(iter->second)) {
        return false;
    }
    rawData->stackContext->mallocSize = iter->second;
    return true;
}

bool StackPreprocess::GetPcsByStackId(StackId stackIdUnion, std::vector<uintptr_t>& pcs, const char* errorMsg)
{
    if (stackIdUnion.value > 0) {
        pcs.reserve(MAX_CALL_FRAME_UNWIND_SIZE);
        auto table = OHOS::HiviewDFX::UniqueStackTable::Instance();
        if (!table->GetPcsByStackId(stackIdUnion, pcs, true)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s", errorMsg);
            return false;
        }
    }
    return true;
}

bool StackPreprocess::BuildNestedAsyncCallFrames(const AsyncStackData* asyncData,
    const std::shared_ptr<HookRecord>& hookRecord, std::vector<CallFrame>& callFrames)
{
    std::vector<uintptr_t> syncStackPcs;
    GetPcsByStackId({asyncData->syncStackId}, syncStackPcs,
        "BuildCallFrames Get syncStackPcs Failed");

    std::vector<std::vector<uintptr_t>> nestedAsyncPcs;
    uint32_t extendCount = asyncData->nestedAsyncCount <= MAX_ASYNC_CHAIN_LAYERS ?
        asyncData->nestedAsyncCount : 0;
    nestedAsyncPcs.reserve(extendCount);
    for (uint32_t i = 0; i < extendCount; i++) {
        std::vector<uintptr_t> asyncPcs;
        GetPcsByStackId({asyncData->nestedAsyncIds[i]}, asyncPcs, "BuildCallFrames Get nestedAsyncPcs Failed");
        nestedAsyncPcs.push_back(std::move(asyncPcs));
    }

    if (!syncStackPcs.empty() || !nestedAsyncPcs.empty()) {
        g_director->ConstructNestedCallFrames(hookRecord, syncStackPcs, nestedAsyncPcs);
    }
    return true;
}

bool StackPreprocess::BuildRegularAsyncCallFrames(const AsyncStackData* asyncData,
    const std::shared_ptr<HookRecord>& hookRecord, std::vector<CallFrame>& callFrames)
{
    std::vector<uintptr_t> syncStackPcs;
    GetPcsByStackId({asyncData->syncStackId}, syncStackPcs,
        "BuildCallFrames Get syncStackPcs Failed");

    std::vector<uintptr_t> asyncStackPcs;
    GetPcsByStackId({asyncData->asyncStackId}, asyncStackPcs,
        "BuildCallFrames Get asyncStackPcs Failed");

    if (!syncStackPcs.empty() || !asyncStackPcs.empty()) {
        g_director->ConstructCallFrames(hookRecord, syncStackPcs, asyncStackPcs);
    }
    return true;
}

bool StackPreprocess::BuildCallFrames(const std::shared_ptr<HookRecord>& hookRecord,
    RawStackPtr& rawData,
    std::vector<CallFrame>& callFrames)
{
    if (!hookConfig_.async_stack_enable()) {
        g_director->ConstructCallFrames(hookRecord);
        return true;
    }
    CHECK_NOTNULL(rawData, false, "rawData is null");
    CHECK_NOTNULL(rawData->stackContext, false, "rawData stackContext is null");
    uint16_t msgType = rawData->stackContext->type;
    if (msgType == FREE_ARKTS || msgType == MALLOC_ARKTS || msgType == PR_SET_VMA_MSG) {
        return true;
    }
    const auto* asyncData = reinterpret_cast<const AsyncStackData*>(rawData->stackContext);

    if (hookConfig_.nested_async_stack_enable() && asyncData->nestedAsyncCount > 0) {
        return BuildNestedAsyncCallFrames(asyncData, hookRecord, callFrames);
    }

    return BuildRegularAsyncCallFrames(asyncData, hookRecord, callFrames);
}

bool StackPreprocess::ProcessRegularStackData(std::shared_ptr<HookRecord>& hookRecord,
                                              RawStackPtr& rawData)
{
    if (rawData == nullptr) {
        return false;
    }
#ifdef PERFORMANCE_DEBUG
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
#endif

    if (TryProcessCachedAsyncStatistics(hookRecord)) {
        return true;
    }

    if (rawData->stackContext->jsChainId > 0) {
        rawData->jsStackData = GetJsRawStack(rawData->stackContext->jsChainId);
    }

    std::vector<CallFrame>& callFrames = g_director->GetCallFrames();
    callFrames.clear();
    if (!BuildCallFrames(hookRecord, rawData, callFrames)) {
        PROFILER_LOG_ERROR(LOG_CORE, "ProcessRegularStackData: BuildCallFrames failed for type %u",
                           hookRecord->GetType());
        return false;
    }

    ReportOfflineSymbolizationData();

    std::visit(
        [&](auto& stackData) {
            ProcessStackData(hookRecord, callFrames, stackData);
        }, stackData_);

    IntervalFlushRecordStatistics(rawData->stackContext->ts);

#ifdef PERFORMANCE_DEBUG
    LogPerformanceMetrics(start, "stack processing", hookRecord->GetType(), rawData->fpDepth);
#endif

    if (hookRecord != nullptr) {
        factory_->SaveHookRecord(std::move(hookRecord));
        hookRecord = nullptr;
    }
    return true;
}

inline bool StackPreprocess::IsStackIdExistsInStatistics(uint32_t stackId)
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        return simplifiedRecordStatisticsMap_.find(stackId) != simplifiedRecordStatisticsMap_.end();
    } else {
        return recordStatisticsMap_.find(stackId) != recordStatisticsMap_.end();
    }
}

bool StackPreprocess::TryProcessCachedAsyncStatistics(std::shared_ptr<HookRecord>& hookRecord)
{
    uint16_t type = hookRecord->GetType();
    bool isStatisticsType = (type == MALLOC_MSG || type == MMAP_MSG ||
        type == MMAP_FILE_PAGE_MSG || type == MEMORY_USING_MSG);
    if (hookConfig_.statistics_interval() == 0 ||
        !hookConfig_.async_stack_enable() || !isStatisticsType) {
        return false;
    }
    if (hookConfig_.save_file() && !hookConfig_.file_name().empty() && !isHookStandaloneSerialize_) {
        return false;
    }
    auto rawData = hookRecord->GetRawStack();
    if (type == MALLOC_MSG) {
        rawData->stackContext->mallocSize = ComputeAlign(rawData->stackContext->mallocSize);
    }
    spinLock_.Lock();
    uint32_t stackId = FindAsyncStatisticsStackIdLocked(hookRecord);
    if (stackId == 0) {
        spinLock_.Unlock();
        return false;
    }
    bool isExists = IsStackIdExistsInStatistics(stackId);
    SetAllocStatisticsData(hookRecord, stackId, isExists);
    spinLock_.Unlock();
    ReportOfflineSymbolizationData();
    IntervalFlushRecordStatistics(rawData->stackContext->ts);
    if (hookRecord != nullptr) {
        factory_->SaveHookRecord(std::move(hookRecord));
        hookRecord = nullptr;
    }
    return true;
}

template <typename T>
void StackPreprocess::ProcessStackData(std::shared_ptr<HookRecord>& hookRecord,
                                       std::vector<CallFrame>& callFrames,
                                       T& stackData)
{
    if (hookConfig_.save_file() && hookConfig_.file_name() != "" && isHookStandaloneSerialize_) {
        SetHookData(hookRecord, callFrames, stackData);
    } else if (hookConfig_.save_file() && hookConfig_.file_name() != "") {
        WriteFrames(hookRecord, callFrames);
    } else if (!hookConfig_.save_file()) {
        SetHookData(hookRecord, callFrames, stackData);
    }
}

#ifdef PERFORMANCE_DEBUG
void StackPreprocess::LogPerformanceMetrics(const struct timespec& start,
                                            const std::string& operation)
{
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t curTimeCost = (end.tv_sec - start.tv_sec) * MAX_MATCH_CNT * MAX_MATCH_CNT * MAX_MATCH_CNT +
        (end.tv_nsec - start.tv_nsec);
    timeCost += curTimeCost;
}
#endif

#ifdef PERFORMANCE_DEBUG
void StackPreprocess::LogPerformanceMetrics(const struct timespec& start,
                                            const std::string& operation,
                                            uint16_t type,
                                            uint32_t fpDepth)
{
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t curTimeCost = (end.tv_sec - start.tv_sec) * MAX_MATCH_CNT * MAX_MATCH_CNT * MAX_MATCH_CNT +
        (end.tv_nsec - start.tv_nsec);

    if (curTimeCost >= LONG_TIME_THRESHOLD) {
        PROFILER_LOG_ERROR(LOG_CORE, "Long time cost for %s: %" PRIu64 " event=%d fpDepth=%u",
                    operation.c_str(), curTimeCost, type, fpDepth);
    }

    timeCost += curTimeCost;
}
#endif

bool StackPreprocess::ProcessBatchData()
{
    if (dataRepeater_ == nullptr) {
        return false;
    }
    HookRecordPtr batchRawStack[MAX_BATCH_CNT] = {nullptr};
    struct timespec latestEventTime = {0, 0};
    uint32_t waitDuration = 0;
    if (hookConfig_.statistics_interval() > 0) {
        auto currentTime = std::chrono::steady_clock::now();
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastStatisticsTime_);
        int tempDuration = std::chrono::duration_cast<std::chrono::milliseconds>(statisticsInterval_).count() -
                           timeDiff.count();
        waitDuration = tempDuration > 0 ? static_cast<uint32_t>(tempDuration) : 0;
    }

    bool isTimeOut = false;
    auto result = dataRepeater_->TakeRawData(waitDuration, hookDataClockId_, MAX_BATCH_CNT, batchRawStack,
                                             hookConfig_.statistics_interval(), isTimeOut);

    if (hookConfig_.statistics_interval() > 0 && isTimeOut && result == nullptr) {
        if (!hookConfig_.use_file_cache_mode()) {
            IntervalFlushRecordStatistics(latestEventTime);
        }
        return true;
    }

    if (!result) {
        return false;
    }

    DmaFdSizeMap dmaFdSizeMap;
    uint32_t dmaFdSizePid = 0;
    for (unsigned int i = 0; i < MAX_BATCH_CNT; i++) {
        if (!ResolveDmaSize(batchRawStack[i], dmaFdSizeMap, dmaFdSizePid)) {
            continue;
        }
        if (!ProcessSingleRecord(batchRawStack[i])) {
            break;
        }
        if (batchRawStack[i] && batchRawStack[i]->GetRawStack() && batchRawStack[i]->GetRawStack()->stackContext &&
            batchRawStack[i]->GetRawStack()->stackContext->type != MMAP_FILE_TYPE) {
            latestEventTime = batchRawStack[i]->GetRawStack()->stackContext->ts;
        }
    }

    CleanupBatchRecords(batchRawStack, MAX_BATCH_CNT);

    if (hookConfig_.save_file() && hookConfig_.file_name() != "" && !isHookStandaloneSerialize_) {
        return true;
    }

    if ((hookConfig_.statistics_interval() == 0) && (hookConfig_.malloc_free_matching_interval() == 0)) {
        std::visit([&](auto& stackData) {
            FlushCheck(stackData);
        }, stackData_);
    }

    IntervalFlushRecordStatistics(latestEventTime);
    IntervalFlushApplyAndReleaseMatchData();
    return true;
}

bool StackPreprocess::ProcessSingleRecord(HookRecordPtr& hookRecord)
{
    if (!hookRecord || isStopTakeData_) {
        return false;
    }

    if (HandleSimpleMessages(hookRecord)) {
        return true;
    }

    auto rawData = hookRecord->GetRawStack();
    if (rawData->stackContext == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "StackPreprocess take results rawData->stackContext is nullptr");
        return true;
    }
    uint16_t type = hookRecord->GetType();
    SetIsStartTakeData(true);
    if (type == NMD_MSG || type == END_MSG) {
        if (type == END_MSG) {
            return ProcessEndMsg();
        }
        return false;
    }

    if (HandleNoStackEvent(hookRecord)) {
        return true;
    }

    if (type == MUNMAP_MSG) {
        std::lock_guard<std::mutex> guard(mtx_);
        runtime_instance->RemoveMaps(reinterpret_cast<uint64_t>(hookRecord->GetAddr()));
    }

    if (!rawData->reportFlag) {
        ignoreCnts_++;
        if (ignoreCnts_ % LOG_PRINT_TIMES == 0) {
            PROFILER_LOG_INFO(LOG_CORE, "ignoreCnts_ = %d quene size = %zu\n", ignoreCnts_, dataRepeater_->Size());
        }
        return true;
    }

    eventCnts_++;
    if (eventCnts_ % LOG_PRINT_TIMES == 0) {
        PROFILER_LOG_INFO(LOG_CORE, "eventCnts_ = %d quene size = %zu\n", eventCnts_, dataRepeater_->Size());
    }

    BuildAndReportStack(hookRecord);
    return true;
}

void StackPreprocess::BuildAndReportStack(HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    uint16_t type = hookRecord->GetType();

    std::vector<CallFrame>& callFrames = g_director->GetCallFrames();
    callFrames.clear();
    if (!BuildCallFrames(hookRecord, rawData, callFrames)) {
        PROFILER_LOG_ERROR(LOG_CORE, "BuildAndReportStack: BuildCallFrames failed for type %u", type);
        return;
    }

    if (((hookConfig_.fp_unwind()) || rawData->stackSize > 0) && (type != PR_SET_VMA_MSG)) {
        ReportOfflineSymbolizationData();
    }

    std::visit([&](auto& stackData) {
        if (hookConfig_.save_file() && hookConfig_.file_name() != "" && isHookStandaloneSerialize_) {
            SetHookData(hookRecord, callFrames, stackData);
        } else if (hookConfig_.save_file() && hookConfig_.file_name() != "") {
            WriteFrames(hookRecord, callFrames);
        } else if (!hookConfig_.save_file()) {
            if (hookConfig_.malloc_free_matching_interval() > 0) {
                SetApplyAndReleaseMatchFrame(hookRecord, callFrames, stackData);
            } else {
                SetHookData(hookRecord, callFrames, stackData);
            }
        }
    }, stackData_);
}

void StackPreprocess::CleanupBatchRecords(HookRecordPtr batchRawStack[], uint32_t maxCount)
{
    for (unsigned int i = 0; i < maxCount; i++) {
        if (!batchRawStack[i]) {
            break;
        }
        factory_->SaveHookRecord(std::move(batchRawStack[i]));
    }
}

bool StackPreprocess::HandleSimpleMessages(HookRecordPtr& hookRecord)
{
    uint16_t type = hookRecord->GetType();

    if (type == FREE_MSG_SIMP) {
        SetFreeStatisticsData(hookRecord->GetAddr());
        return true;
    }

    if (type == FREE_ARKTS) {
        size_t size = hookRecord->GetRawStack()->stackContext->mallocSize;
        HandleDeleteAddr(reinterpret_cast<uint64_t>(hookRecord->GetAddr()), size);
        return false;
    }

    if (type == MALLOC_ARKTS) {
        size_t size = hookRecord->GetRawStack()->stackContext->mallocSize;
        auto addr = hookRecord->GetRawStack()->stackContext->addr;
        auto newAddr = hookRecord->GetRawStack()->stackContext->newAddr;
        HandleMoveAddr(reinterpret_cast<uint64_t>(addr), reinterpret_cast<uint64_t>(newAddr), size);
        return false;
    }

    return false;
}

bool StackPreprocess::CheckIsFileCacheFinsh()
{
    if (hookConfig_.use_file_cache_mode() && dataRepeater_ != nullptr &&
        dataRepeater_->Size() == 0 && isCacheFileParseDone_) {
        return true;
    }
    return false;
}

void StackPreprocess::TakeResults()
{
    if (!dataRepeater_) {
        return;
    }
    PROFILER_LOG_INFO(LOG_CORE, "TakeResults thread %d, start!", gettid());
    if (g_director == nullptr) {
        g_director = std::make_shared<BuildStackDirector>(&hookConfig_, runtime_instance, mtx_);
    }
    while (!isStopTakeData_) {
        if (!ProcessBatchData()) {
            break;
        }
        if (CheckIsFileCacheFinsh()) {
            SetIsStopTakeData(true);
        }
    } // while
    PROFILER_LOG_INFO(LOG_CORE, "TakeResults thread %d, exit!", gettid());
}

inline void StackPreprocess::ReportThreadNameMap(uint32_t tid, const std::string& tname)
{
    std::lock_guard<std::mutex> guard(mtx_);
    auto it = threadNameMap_.find(tid);
    if (it == threadNameMap_.end() || it->second != tname) {
        threadNameMap_[tid] = tname;
        std::visit([&](auto& stackData) {
            auto hookData = stackData.add_events();
            auto thread = hookData->mutable_thread_name_map();
            thread->set_id(tid);
            thread->set_name(tname);
            thread->set_pid(pid_);
            FlushCheck(stackData);
            }, stackData_);
    }
}

template <typename T>
inline void StackPreprocess::FillOfflineCallStack(std::vector<CallFrame>& callFrames, size_t idx, T& stackData,
    std::vector<uint64_t>& callStack)
{
    for (; idx < callFrames.size(); ++idx) {
        if (callFrames[idx].isJsFrame_) {
            ReportFrameMap(callFrames[idx], stackData);
            callStack.push_back(callFrames[idx].callFrameId_ | JS_OFFLINE_IP_MASK);
            continue;
        }
        callStack.push_back(callFrames[idx].ip_);
    }
}

template <typename T>
inline void StackPreprocess::FillCallStack(std::vector<CallFrame>& callFrames, size_t idx, T& stackData,
    std::vector<uint64_t>& callStack)
{
    for (; idx < callFrames.size(); ++idx) {
        ReportFrameMap(callFrames[idx], stackData);
        // for call stack id
        callStack.push_back(callFrames[idx].callFrameId_);
    }
}

uint64_t StackPreprocess::GetAsyncDedupMallocSize(const RawStackPtr& rawData) const
{
    if (hookConfig_.statistics_interval() == 0 || rawData == nullptr || rawData->stackContext == nullptr) {
        return 0;
    }
    return rawData->stackContext->mallocSize;
}

uint32_t StackPreprocess::GetDedupTagId(const RawStackPtr& rawData) const
{
    if (hookConfig_.statistics_interval() == 0 || !hookConfig_.arkui_mem_trace_enable() ||
        rawData == nullptr || rawData->stackContext == nullptr) {
        return 0;
    }
    return rawData->stackContext->tagId;
}

uint32_t StackPreprocess::FindAsyncCallStackId(const HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    const auto* asyncData = reinterpret_cast<const AsyncStackData*>(rawData->stackContext);
    uint64_t asyncHash = asyncData->syncStackId ^ (asyncData->asyncStackId << 32);
    AsyncCallStackKey asyncKey = {asyncHash, GetAsyncDedupMallocSize(rawData), GetDedupTagId(rawData)};
    auto it = asyncCallStackIndex_.find(asyncKey);
    return it != asyncCallStackIndex_.end() ? it->second : 0;
}

uint32_t StackPreprocess::FindSimplifiedAsyncCallStackId(const HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    const auto* asyncData = reinterpret_cast<const AsyncStackData*>(rawData->stackContext);
    uint64_t asyncHash = asyncData->syncStackId ^ (asyncData->asyncStackId << 32);
    SimplifiedAsyncCallStackKey asyncKey = {asyncHash, GetAsyncDedupMallocSize(rawData), GetDedupTagId(rawData)};
    auto it = simplifiedAsyncCallStackIndex_.find(asyncKey);
    return it != simplifiedAsyncCallStackIndex_.end() ? it->second : 0;
}

uint32_t StackPreprocess::FindFullAsyncCallStackId(const HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    const auto* asyncData = reinterpret_cast<const AsyncStackData*>(rawData->stackContext);
    uint64_t asyncHash = asyncData->syncStackId ^ (asyncData->asyncStackId << 32);
    AsyncCallStackKey asyncKey = {asyncHash, GetAsyncDedupMallocSize(rawData), GetDedupTagId(rawData)};
    auto it = asyncCallStackIndex_.find(asyncKey);
    return it != asyncCallStackIndex_.end() ? it->second : 0;
}

uint32_t StackPreprocess::FindFullAsyncStatisticsStackId(const HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    CHECK_NOTNULL(rawData, 0, "rawData is nullptr");
    CHECK_NOTNULL(rawData->stackContext, 0, "stackContext is nullptr");
    const auto* asyncData = reinterpret_cast<const AsyncStackData*>(rawData->stackContext);
    uint64_t stackHash = asyncData->syncStackId ^ (asyncData->asyncStackId << 32);
    AsyncCallStackKey key = {stackHash, GetAsyncDedupMallocSize(rawData), GetDedupTagId(rawData)};
    auto it = asyncCallStackIndex_.find(key);
    return it != asyncCallStackIndex_.end() ? it->second : 0;
}


uint32_t StackPreprocess::FindSyncCallStackId(std::vector<uint64_t>& callStack, const HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    uint64_t stackHash = CallStackHash{}(callStack);
    uint32_t stackSize = static_cast<uint32_t>(callStack.size());
    uint64_t nodeId = hookConfig_.arkui_mem_trace_enable() ? rawData->stackContext->nodeId : 0;
    uint32_t nodeType = hookConfig_.arkui_mem_trace_enable() ? rawData->stackContext->nodeType : 0;
    CallStackKey key = {stackHash, nodeId, stackSize, nodeType, GetDedupTagId(rawData)};
    auto itStack = callStackIndex_.find(key);
    return itStack != callStackIndex_.end() ? itStack->second : 0;
}

uint32_t StackPreprocess::FindSimplifiedSyncCallStackId(
    std::vector<uint64_t>& callStack, const HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    uint64_t stackHash = CallStackHash{}(callStack);
    uint32_t stackSize = static_cast<uint32_t>(callStack.size());
    SimplifiedCallStackKey key = {stackHash, stackSize, GetDedupTagId(rawData)};
    auto itStack = simplifiedCallStackIndex_.find(key);
    return itStack != simplifiedCallStackIndex_.end() ? itStack->second : 0;
}

uint32_t StackPreprocess::FindCallStackId(std::vector<uint64_t>& callStack, const HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();

    if (hookConfig_.async_stack_enable() && rawData->stackContext->type != FREE_ARKTS &&
        rawData->stackContext->type != MALLOC_ARKTS && rawData->stackContext->type != PR_SET_VMA_MSG) {
            return (this->*findAsyncCallStackIdFunc_)(hookRecord);
        }
    return (this->*findSyncCallStackIdFunc_)(callStack, hookRecord);
}

uint32_t StackPreprocess::FindAsyncStatisticsStackIdLocked(const HookRecordPtr& hookRecord)
{
    auto rawData = hookRecord->GetRawStack();
    CHECK_NOTNULL(rawData, 0, "rawData is nullptr");
    CHECK_NOTNULL(rawData->stackContext, 0, "stackContext is nullptr");
    return (this->*findAsyncStatisticsStackIdFunc_)(hookRecord);
}

inline uint32_t StackPreprocess::GetNewStackId()
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        return hookConfig_.async_stack_enable() ? simplifiedAsyncCallStackIndex_.size() + 1 :
            simplifiedCallStackIndex_.size() + 1;
    } else {
        return hookConfig_.async_stack_enable() ? asyncCallStackIndex_.size() + 1 :
            callStackIndex_.size() + 1;
    }
}

void StackPreprocess::AddCallStackIndex(uint32_t stackId, RawStackPtr rawData)
{
    if (hookConfig_.async_stack_enable() && rawData->stackContext->type != FREE_ARKTS &&
        rawData->stackContext->type != MALLOC_ARKTS && rawData->stackContext->type != PR_SET_VMA_MSG) {
        const auto* asyncData = reinterpret_cast<const AsyncStackData*>(rawData->stackContext);
        uint64_t stackHash = asyncData->syncStackId ^ (asyncData->asyncStackId << 32);
        AsyncCallStackKey key = {stackHash, GetAsyncDedupMallocSize(rawData), GetDedupTagId(rawData)};
            asyncCallStackIndex_[key] = stackId;
    } else {
        auto& callStack = g_callStack;
        uint64_t stackHash = CallStackHash{}(callStack);
        uint32_t stackSize = static_cast<uint32_t>(callStack.size());
        uint64_t nodeId = hookConfig_.arkui_mem_trace_enable() ? rawData->stackContext->nodeId : 0;
        uint32_t nodeType = hookConfig_.arkui_mem_trace_enable() ? rawData->stackContext->nodeType : 0;
        CallStackKey key = {stackHash, nodeId, stackSize, nodeType, GetDedupTagId(rawData)};
        callStackIndex_[key] = stackId;
    }
}

void StackPreprocess::AddSimplifiedCallStackIndex(uint32_t stackId, RawStackPtr rawData)
{
    if (hookConfig_.async_stack_enable() && rawData->stackContext->type != FREE_ARKTS &&
        rawData->stackContext->type != MALLOC_ARKTS && rawData->stackContext->type != PR_SET_VMA_MSG) {
        const auto* asyncData = reinterpret_cast<const AsyncStackData*>(rawData->stackContext);
        uint64_t stackHash = asyncData->syncStackId ^ (asyncData->asyncStackId << 32);
        SimplifiedAsyncCallStackKey asyncKey = {stackHash, GetAsyncDedupMallocSize(rawData), GetDedupTagId(rawData)};
        simplifiedAsyncCallStackIndex_[asyncKey] = stackId;
    } else {
        auto& callStack = g_callStack;
        uint64_t stackHash = CallStackHash{}(callStack);
        uint32_t stackSize = static_cast<uint32_t>(callStack.size());
        SimplifiedCallStackKey key = {stackHash, stackSize, GetDedupTagId(rawData)};
        simplifiedCallStackIndex_[key] = stackId;
    }
}

/**
 * @return '0' is invalid stack id, '> 0' is valid stack id
 */
template <typename T>
uint32_t StackPreprocess::SetCallStackMap(T& stackData, const HookRecordPtr& hookRecord,
    std::vector<uint64_t>& callStack)
{
    std::lock_guard<std::mutex> guard(mtx_);
    uint32_t stackId = 0;
    auto hookData = stackData.add_events();
    auto stackmap = hookData->mutable_stack_map();
    stackId = GetNewStackId();
    stackmap->set_id(stackId);
    // offline symbolization use ip, other use frame_map_id
    if (hookConfig_.offline_symbolization()) {
        if constexpr (std::is_same<T, ProtoEncoder::BatchNativeHookData>::value) {
            stackmap->add_ip(callStack);
        } else {
            stackmap->mutable_ip()->Reserve(callStack.size());
            for (size_t i = 0; i < callStack.size(); i++) {
                stackmap->add_ip(callStack[i]);
            }
        }
    } else {
        if constexpr (std::is_same<T, ProtoEncoder::BatchNativeHookData>::value) {
            stackmap->add_frame_map_id(callStack);
        } else {
            stackmap->mutable_frame_map_id()->Reserve(callStack.size());
            for (size_t i = 0; i < callStack.size(); i++) {
                stackmap->add_frame_map_id(callStack[i]);
            }
        }
    }
    stackmap->set_pid(pid_);
    auto rawData = hookRecord->GetRawStack();
    CHECK_NOTNULL(rawData, 0, "StackPreprocess rawData is nullptr");

    (this->*addCallStackIndexFunc_)(stackId, rawData);
    return stackId;
}

/**
 * @return '0' is invalid stack id, '> 0' is valid stack id
 */
template <typename T>
inline uint32_t StackPreprocess::GetCallStackId(const HookRecordPtr& hookRecord, std::vector<CallFrame>& callFrames,
    T& stackData, std::vector<uint64_t>& callStack)
{
    auto rawStack = hookRecord->GetRawStack();
    // ignore the first two frame if dwarf unwind
    size_t idx = (hookConfig_.async_stack_enable() || !hookConfig_.fp_unwind()) ? FILTER_STACK_DEPTH : 0;
    // if free_stack_report or munmap_stack_report is false, don't need to record.
    uint16_t type = hookRecord->GetType();
    if ((type == FREE_MSG) && !hookConfig_.free_stack_report()) {
        return 0;
    } else if ((type == MUNMAP_MSG) && !hookConfig_.munmap_stack_report()) {
        return 0;
    }
    callStack.clear();
    bool isNapi = false;
    if (type == MEMORY_USING_MSG && hookConfig_.js_stack_report() == NAPI_CALL_STACK) {
        std::string tagName;
        GetMemTag(rawStack->stackContext->tagId, tagName);
        if (tagName.find("napi") != std::string::npos) {
            callStack.reserve(callFrames.size() + 1);  // 1 : insert a frame
            if (!hookConfig_.offline_symbolization()) {
                callStack.push_back(DWARF_NAPI_CALLBACK + napiIndex_);
            } else {
                // just for offline symbolization
                callStack.push_back((DWARF_NAPI_CALLBACK + napiIndex_) | JS_OFFLINE_IP_MASK);
            }
            isNapi = true;
        }
    } else {
        callStack.reserve(callFrames.size());
    }
    if (!hookConfig_.offline_symbolization()) {
        FillCallStack(callFrames, idx, stackData, callStack);
    } else {
        if ((!hookConfig_.fp_unwind()) && rawStack->stackSize == 0) {
            idx = 0;
        }
        FillOfflineCallStack(callFrames, idx, stackData, callStack);
    }
    if (isNapi) {
        // insert a frame
        std::string tagName;
        GetMemTag(rawStack->stackContext->tagId, tagName);
        FillNapiStack(tagName, callFrames, napiIndex_);
        ReportFrameMap(callFrames.back(), stackData);
        ++napiIndex_;
    }
    // return call stack id
    uint32_t stackId = FindCallStackId(callStack, hookRecord);
    if (stackId > 0) {
        return stackId;
    } else {
        return SetCallStackMap(stackData, hookRecord, callStack);
    }
}

template <typename T>
void StackPreprocess::SetEventFrame(const ReportEventBaseData& rawStack,
    T* event, uint32_t stackMapId, const std::string& type)
{
    event->set_pid(pid_);
    event->set_tid(rawStack.tid);
    event->set_addr(rawStack.addr);
    if constexpr (std::is_same<T, ::MmapEvent>::value || std::is_same<T, ProtoEncoder::MmapEvent>::value) {
        event->set_type(type);
    }

    if constexpr (!std::is_same<T, ::FreeEvent>::value && !std::is_same<T, ProtoEncoder::FreeEvent>::value &&
                  !std::is_same<T, ::TraceFreeEvent>::value && !std::is_same<T, ProtoEncoder::TraceFreeEvent>::value) {
        auto size = static_cast<uint64_t>(rawStack.mallocSize);
#ifdef USE_JEMALLOC
    if constexpr (std::is_same<T, ::AllocEvent>::value || std::is_same<T, ProtoEncoder::AllocEvent>::value) {
        size = static_cast<uint64_t>(ComputeAlign(size));
    }
#endif
        event->set_size(size);
    }
    if (hookConfig_.callframe_compress() && stackMapId != 0) {
        event->set_thread_name_id(rawStack.tid);
        event->set_stack_id(stackMapId);
    }
    event->set_thread_name_id(rawStack.tid);
}

void StackPreprocess::FillNapiStack(std::string& tagName, std::vector<CallFrame>& callFrames, uint64_t napiIndex)
{
    CallFrame& jsCallFrame = callFrames.emplace_back(0);
    jsCallFrame.symbolName_ = tagName;
    jsCallFrame.isJsFrame_ = true;
    jsCallFrame.needReport_ |= CALL_FRAME_REPORT;
    jsCallFrame.needReport_ |= SYMBOL_NAME_ID_REPORT;
    jsCallFrame.needReport_ |= FILE_PATH_ID_REPORT;
    jsCallFrame.callFrameId_ = DWARF_NAPI_CALLBACK + napiIndex;
    jsCallFrame.symbolNameId_ = DWARF_NAPI_CALLBACK + napiIndex;
    jsCallFrame.filePathId_ = DWARF_NAPI_CALLBACK + napiIndex;
    jsCallFrame.filePath_ = "no-napi-file-path";
}

template <typename T>
void StackPreprocess::SetAllocStatisticsFrame(const HookRecordPtr& hookRecord, std::vector<CallFrame>& callFrames,
    T& stackData, std::vector<uint64_t>& callStack)
{
    // ignore the first two frame if dwarf unwind
    auto rawStack = hookRecord->GetRawStack();
    size_t idx = (hookConfig_.async_stack_enable() || !hookConfig_.fp_unwind()) ? FILTER_STACK_DEPTH : 0;
    callStack.clear();
    bool isNapi = false;
    if (hookConfig_.js_stack_report() == NAPI_CALL_STACK) {
        std::string tagName;
        GetMemTag(rawStack->stackContext->tagId, tagName);
        if (tagName.find("napi") != std::string::npos) {
            callStack.reserve(callFrames.size() + FRAME_DEPTH);  // insert a frame
            if (!hookConfig_.offline_symbolization()) {
                callStack.push_back(DWARF_NAPI_CALLBACK + napiIndex_);
            } else {
                // just for offline symbolization
                callStack.push_back((DWARF_NAPI_CALLBACK + napiIndex_) | JS_OFFLINE_IP_MASK);
            }
            isNapi = true;
        }
    } else {
        callStack.reserve(callFrames.size() + 1);
    }
    callStack.push_back(rawStack->stackContext->mallocSize | SIZE_MASK);
    if (!hookConfig_.offline_symbolization()) {
        FillCallStack(callFrames, idx, stackData, callStack);
    } else {
        FillOfflineCallStack(callFrames, idx, stackData, callStack);
    }
    // insert a frame
    if (isNapi) {
        std::string tagName;
        GetMemTag(rawStack->stackContext->tagId, tagName);
        FillNapiStack(tagName, callFrames, napiIndex_);
        ReportFrameMap(callFrames.back(), stackData);
        ++napiIndex_;
    }
    spinLock_.Lock();
    uint32_t stackId = FindCallStackId(callStack, hookRecord);
    if (stackId > 0) {
        SetAllocStatisticsData(hookRecord, stackId, IsStackIdExistsInStatistics(stackId));
    } else {
        stackId = SetCallStackMap(stackData, hookRecord, callStack);
        statisticsModelFlushCallstack_ = true;
        SetAllocStatisticsData(hookRecord, stackId);
    }
    if constexpr (std::is_same<T, ::BatchNativeHookData>::value) {
        if (stackData.events().size() > STATISTICS_PERIOD_DATA_FLUSH_BATCH) {
            FlushData(stackData);
        }
    }
    spinLock_.Unlock();
}

template <typename T>
void StackPreprocess::SetAllocStatisticsFrame(const HookRecordPtr& hookRecord, T& stackData,
    std::vector<uint64_t>& callStack)
{
    auto rawStack = hookRecord->GetRawStack();
    callStack.resize(rawStack->fpDepth + 1);
    callStack[0] = (rawStack->stackContext->mallocSize | SIZE_MASK);
    uint64_t* fpIp = reinterpret_cast<uint64_t *>(rawStack->data);
    for (uint8_t idx = 0; idx < rawStack->fpDepth ; ++idx) {
        if (fpIp[idx] != 0) {
            fpIp[idx] = StripPac(fpIp[idx], 0);
        }
    }
    if (memcpy_s(callStack.data() + 1, sizeof(uint64_t) * rawStack->fpDepth,
                 rawStack->data, sizeof(uint64_t) * rawStack->fpDepth) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s g_callStack failed");
        return;
    }
    // by call stack id set alloc statistics data.
    spinLock_.Lock();
    uint32_t stackId = FindCallStackId(callStack, hookRecord);
    if (stackId > 0) {
        SetAllocStatisticsData(hookRecord, stackId, true);
    } else {
        stackId = SetCallStackMap(stackData, hookRecord, callStack);
        statisticsModelFlushCallstack_ = true;
        SetAllocStatisticsData(hookRecord, stackId);
    }
    spinLock_.Unlock();
}

template <typename T>
void StackPreprocess::SetHookData(HookRecordPtr hookRecord, T& stackData)
{
    auto rawStack = hookRecord->GetRawStack();
    if (hookConfig_.statistics_interval() > 0) {
        // statistical reporting must is compressed and accurate.
        switch (hookRecord->GetType()) {
            case FREE_MSG:
            case MUNMAP_MSG:
            case MEMORY_UNUSING_MSG: {
                SetFreeStatisticsData(hookRecord->GetAddr());
                break;
            }
            case MALLOC_MSG:
                rawStack->stackContext->mallocSize = ComputeAlign(rawStack->stackContext->mallocSize);
            case MMAP_MSG:
            case MMAP_FILE_PAGE_MSG:
            case MEMORY_USING_MSG: {
                auto* cs = &g_callStack;
                asm volatile("" : "+r"(cs) : : "memory");
                SetAllocStatisticsFrame(hookRecord, stackData, *cs);
                break;
            }
            case PR_SET_VMA_MSG: {
                break;
            }
            case JS_STACK_MSG: {
                break;
            }
            case MALLOC_ARKTS: {
                break;
            }
            case FREE_ARKTS: {
                break;
            }
            default: {
                PROFILER_LOG_ERROR(LOG_CORE, "statistics event type: error");
                break;
            }
        }
        return;
    }
    auto hookData = stackData.add_events();
    hookData->set_tv_sec(rawStack->stackContext->ts.tv_sec);
    hookData->set_tv_nsec(rawStack->stackContext->ts.tv_nsec);
    std::vector<CallFrame> callFrames;
    SerializeInfo hookInfo = {&callFrames, 0, "", rawStack->stackContext->nodeType,
                              rawStack->stackContext->nodeId, &hookConfig_};
    hookRecord->SerializeData(hookData, hookInfo);
}

void StackPreprocess::ReportOfflineSymbolizationData()
{
    if (hookConfig_.offline_symbolization() && (flushBasicData_ || hookConfig_.async_stack_enable())) {
        SetMapsInfo();
        flushBasicData_ = false;
    }
}

template <typename T>
void StackPreprocess::SetApplyAndReleaseMatchFrame(HookRecordPtr hookRecord, std::vector<CallFrame>& callFrames,
                                                   T& stackData)
{
    uint32_t stackMapId = 0;
    auto rawStack = hookRecord->GetRawStack();
    uint16_t type = hookRecord->GetType();
    if (type != PR_SET_VMA_MSG) {
        auto* cs = &g_callStack;
        asm volatile("" : "+r"(cs) : : "memory");
        stackMapId = GetCallStackId(hookRecord, callFrames, stackData, *cs);
    } else {
        rawStack->stackContext->tagId = prctlPeriodTags_.size();
        prctlPeriodTags_.emplace_back(reinterpret_cast<char*>(rawStack->data));
        applyAndReleaseMatchPeriodListData_.emplace_back(rawStack->stackContext);
    }
    if (type == MALLOC_MSG) {
        rawStack->stackContext->mallocSize = ComputeAlign(rawStack->stackContext->mallocSize);
    } else if (type == PR_SET_VMA_MSG) {
        return;
    }
    uint64_t addr = reinterpret_cast<uint64_t>(hookRecord->GetAddr());
    bool isReleasedType = ((type == FREE_MSG) || (type == MUNMAP_MSG));
    if (isReleasedType) {
        auto iter = applyAndReleaseMatchIntervallMap_.find(addr);
        if (iter != applyAndReleaseMatchIntervallMap_.end()) {
            applyAndReleaseMatchPeriodListData_.erase(iter->second);
            applyAndReleaseMatchIntervallMap_.erase(addr);
        } else {
            releasedPeriodListData_.emplace_back(rawStack->stackContext, stackMapId);
        }
    } else {
        applyAndReleaseMatchPeriodListData_.emplace_back(rawStack->stackContext, stackMapId);
        applyAndReleaseMatchIntervallMap_.emplace(addr, std::prev(applyAndReleaseMatchPeriodListData_.end()));
    }
}

template <typename T>
void StackPreprocess::SetHookData(HookRecordPtr hookRecord, std::vector<CallFrame>& callFrames, T& stackData)
{
    // statistical reporting must is compressed and accurate.
    auto rawStack = hookRecord->GetRawStack();
    uint16_t type = hookRecord->GetType();
    if (hookConfig_.statistics_interval() > 0) {
        switch (type) {
            case FREE_MSG:
            case MUNMAP_MSG:
            case MEMORY_UNUSING_MSG: {
                SetFreeStatisticsData(hookRecord->GetAddr());
                break;
            }
            case MALLOC_MSG:
                rawStack->stackContext->mallocSize = ComputeAlign(rawStack->stackContext->mallocSize);
            case MMAP_MSG:
            case MMAP_FILE_PAGE_MSG:
            case MEMORY_USING_MSG: {
                auto* cs = &g_callStack;
                asm volatile("" : "+r"(cs) : : "memory");
                SetAllocStatisticsFrame(hookRecord, callFrames, stackData, *cs);
                break;
            }
            case PR_SET_VMA_MSG: {
                break;
            }
            case MALLOC_ARKTS: {
                break;
            }
            case FREE_ARKTS: {
                break;
            }
            default: {
                PROFILER_LOG_ERROR(LOG_CORE, "statistics event type:%d error", type);
                break;
            }
        }
        return;
    }

    uint32_t stackMapId = 0;
    if (hookConfig_.callframe_compress() &&
        !(type == MEMORY_TAG || type == PR_SET_VMA_MSG)) {
        auto* cs = &g_callStack;
        asm volatile("" : "+r"(cs) : : "memory");
        stackMapId = GetCallStackId(hookRecord, callFrames, stackData, *cs);
    }

    if ((!hookConfig_.callframe_compress() || stackMapId == 0) && hookConfig_.string_compressed()) {
        size_t idx = (hookConfig_.async_stack_enable() || !hookConfig_.fp_unwind()) ? FILTER_STACK_DEPTH : 0;
        for (; idx < callFrames.size(); ++idx) {
            ReportSymbolNameMap(callFrames[idx], stackData);
            ReportFilePathMap(callFrames[idx], stackData);
        }
    }

    auto hookData = stackData.add_events();
    hookData->set_tv_sec(rawStack->stackContext->ts.tv_sec);
    hookData->set_tv_nsec(rawStack->stackContext->ts.tv_nsec);
    std::string tagName = "";
    if (type == MMAP_FILE_PAGE_MSG || type == MEMORY_USING_MSG || type == MEMORY_UNUSING_MSG ||
        type == FREE_ARKTS || type == MALLOC_ARKTS) {
        GetMemTag(rawStack->stackContext->tagId, tagName);
    }

    SerializeInfo hookInfo = {&callFrames, stackMapId, tagName,
                              rawStack->stackContext->nodeType,
                              rawStack->stackContext->nodeId,
                              &hookConfig_};
    hookRecord->SerializeData(hookData, hookInfo);
}

template <typename T>
inline void StackPreprocess::FindAndUpdateFreeStatisticsLocked(
    std::unordered_map<uint32_t, T>& statisticsMap, uint32_t stackId, uint64_t mallocSize)
{
    auto recordIter = statisticsMap.find(stackId);
    if (recordIter != statisticsMap.end()) {
        auto& record = recordIter->second;
        ++record.releaseCount;
        record.releaseSize += mallocSize;
        statisticsPeriodSet_.insert(record.callstackId);
    }
}

inline void StackPreprocess::FindAndUpdateFreeStatisticsLocked(uint32_t stackId, uint64_t mallocSize)
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        FindAndUpdateFreeStatisticsLocked(simplifiedRecordStatisticsMap_, stackId, mallocSize);
    } else {
        FindAndUpdateFreeStatisticsLocked(recordStatisticsMap_, stackId, mallocSize);
    }
}

template <typename T>
inline void StackPreprocess::RemoveFreeAddrFromStatisticsLocked(
    std::unordered_map<uint32_t, T>& statisticsMap, uint64_t addr, uint32_t stackId)
{
    auto recordIter = statisticsMap.find(stackId);
    if (recordIter != statisticsMap.end()) {
        auto& addrs = recordIter->second.addrs;
        addrs.erase(std::remove(addrs.begin(), addrs.end(), addr), addrs.end());
    }
}

void StackPreprocess::RemoveFreeAddrFromStatisticsLocked(uint64_t addr, uint32_t stackId)
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        RemoveFreeAddrFromStatisticsLocked(simplifiedRecordStatisticsMap_, addr, stackId);
    } else {
        RemoveFreeAddrFromStatisticsLocked(recordStatisticsMap_, addr, stackId);
    }
}

bool StackPreprocess::SetFreeStatisticsDataLocked(uint64_t addr)
{
    auto addrIter = allocAddrMap_.find(addr);
    if (addrIter == allocAddrMap_.end()) {
        return false;
    }
    FindAndUpdateFreeStatisticsLocked(addrIter->second.stackId, addrIter->second.mallocSize);
    RemoveFreeAddrFromStatisticsLocked(addr, addrIter->second.stackId);
    allocAddrMap_.erase(addr);
    auto addrSetIter = addrSet_.find(addr);
    if (addrSetIter != addrSet_.end()) {
        addrSet_.erase(addrSetIter);
    }
    return true;
}

bool StackPreprocess::SetFreeStatisticsData(uint64_t addr)
{
    spinLock_.Lock();
    bool result = SetFreeStatisticsDataLocked(addr);
    spinLock_.Unlock();
    return result;
}

bool StackPreprocess::ReconcileDuplicateMalloc(uint64_t addr, const struct timespec& newTs)
{
    spinLock_.Lock();
    auto it = allocAddrMap_.find(addr);
    if (it == allocAddrMap_.end()) {
        spinLock_.Unlock();
        return true;
    }
    const auto& oldTs = it->second.ts;
    bool isNewer = TsLess(oldTs, newTs);
    if (!isNewer) {
        spinLock_.Unlock();
        return false;
    }
    SetFreeStatisticsDataLocked(addr);
    spinLock_.Unlock();
    return true;
}

template <typename T>
void StackPreprocess::UpdateAllocStatistics(
    std::unordered_map<uint32_t, T>& statisticsMap, const HookRecordPtr& hookRecord, size_t stackId)
{
    auto recordIter = statisticsMap.find(stackId);
    if (recordIter != statisticsMap.end()) {
        auto& record = recordIter->second;
        auto rawStack = hookRecord->GetRawStack();
        ++record.applyCount;
        record.applySize += rawStack->stackContext->mallocSize;
        if constexpr (std::is_same_v<T, RecordStatistic>) {
            record.ts = rawStack->stackContext->ts;
        }
        if (NeedCollectAddr(record.type)) {
            record.addrs.push_back(hookRecord->GetAddr());
        }
        allocAddrMap_[hookRecord->GetAddr()] =
            AllocAddrInfo{rawStack->stackContext->mallocSize, stackId, rawStack->stackContext->ts};
        if (hookRecord->GetType() == MEMORY_USING_MSG) {
            addrSet_.insert(hookRecord->GetAddr());
        }
        statisticsPeriodSet_.insert(stackId);
    }
}

void StackPreprocess::UpdateAllocStatistics(const HookRecordPtr& hookRecord, size_t stackId)
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        UpdateAllocStatistics(simplifiedRecordStatisticsMap_, hookRecord, stackId);
    } else {
        UpdateAllocStatistics(recordStatisticsMap_, hookRecord, stackId);
    }
}

bool StackPreprocess::ResolveTagType(const std::string& tagName, RecordStatisticsEvent::MemoryType& type)
{
    for (const auto& item : g_tagMappings) {
        if (tagName == item.tagName) {
            type = item.type;
            return item.needNodeInfo;
        }
    }
    if (tagName == "ashmem") {
        type = RecordStatisticsEvent::ASHMEM;
        return true;
    }
    if (tagName.find(".so") != std::string::npos) {
        type = RecordStatisticsEvent::SO;
    } else {
        type = RecordStatisticsEvent::MEMORY_USING_MSG;
    }
    return false;
}

bool StackPreprocess::NeedCollectAddr(RecordStatisticsEvent::MemoryType type)
{
    return type == RecordStatisticsEvent::MALLOC ||
           type == RecordStatisticsEvent::ARK_GLOBAL_HANDLE ||
           type == RecordStatisticsEvent::ARK_LOCAL_HANDLE;
}

template <typename T>
void StackPreprocess::FillRecordFromType(T& record, const HookRecordPtr& hookRecord)
{
    auto rawStack = hookRecord->GetRawStack();
    switch (hookRecord->GetType()) {
        case MALLOC_MSG:
            record.type = RecordStatisticsEvent::MALLOC;
            record.SetNodeType(rawStack->stackContext->nodeType);
            record.SetNodeId(rawStack->stackContext->nodeId);
            record.addrs.push_back(hookRecord->GetAddr());
            break;
        case MMAP_MSG:
            record.type = RecordStatisticsEvent::MMAP;
            break;
        case MMAP_FILE_PAGE_MSG:
            record.type = RecordStatisticsEvent::FILE_PAGE_MSG;
            record.tagId = rawStack->stackContext->tagId;
            break;
        case MEMORY_USING_MSG: {
            std::string tagName;
            GetMemTag(rawStack->stackContext->tagId, tagName);
            RecordStatisticsEvent::MemoryType memType = RecordStatisticsEvent::MALLOC;
            bool needNodeInfo = ResolveTagType(tagName, memType);
            record.type = memType;
            if (needNodeInfo) {
                record.SetNodeType(rawStack->stackContext->nodeType);
                record.SetNodeId(rawStack->stackContext->nodeId);
            }
            record.tagId = rawStack->stackContext->tagId;
            if (NeedCollectAddr(memType)) {
                record.addrs.push_back(hookRecord->GetAddr());
            }
            break;
        }
        default:
            PROFILER_LOG_ERROR(LOG_CORE, "SetAllocStatisticsData event type error");
            break;
    }
}

template <typename T>
void StackPreprocess::RegisterAllocAddress(std::unordered_map<uint32_t, T>& statisticsMap,
    const HookRecordPtr& hookRecord, size_t stackId)
{
    T record;
    record.callstackId = stackId;
    record.applyCount = 1;
    auto rawStack = hookRecord->GetRawStack();
    record.applySize = rawStack->stackContext->mallocSize;
    FillRecordFromType(record, hookRecord);
    statisticsMap[stackId] = record;
    allocAddrMap_[hookRecord->GetAddr()] =
        AllocAddrInfo{rawStack->stackContext->mallocSize, stackId, rawStack->stackContext->ts};
    if (hookRecord->GetType() == MEMORY_USING_MSG) {
        addrSet_.insert(hookRecord->GetAddr());
    }
    statisticsPeriodSet_.insert(stackId);
}

template <typename T>
inline void StackPreprocess::AddAllocStatistics(std::unordered_map<uint32_t, T>& statisticsMap,
    const HookRecordPtr& hookRecord, size_t stackId)
{
    RegisterAllocAddress(statisticsMap, hookRecord, stackId);
}

void StackPreprocess::AddAllocStatistics(const HookRecordPtr& hookRecord, size_t stackId)
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        AddAllocStatistics(simplifiedRecordStatisticsMap_, hookRecord, stackId);
    } else {
        AddAllocStatistics(recordStatisticsMap_, hookRecord, stackId);
    }
}

void StackPreprocess::SetAllocStatisticsData(const HookRecordPtr& hookRecord, size_t stackId, bool isExists)
{
    auto rawStack = hookRecord->GetRawStack();
    // if the record exists, it is updated.Otherwise Add
    if (isExists) {
        UpdateAllocStatistics(hookRecord, stackId);
    } else {
        AddAllocStatistics(hookRecord, stackId);
    }
}

void StackPreprocess::WriteFrames(HookRecordPtr hookRecord, const std::vector<CallFrame>& callFrames)
{
    std::lock_guard<std::mutex> guard(mtx_);
    auto rawStack = hookRecord->GetRawStack();
    CHECK_TRUE(fpHookData_ != nullptr, NO_RETVAL, "fpHookData_ is nullptr, please check file_name(%s)",
        hookConfig_.file_name().c_str());
    if (hookRecord->GetType() == PR_SET_VMA_MSG) {
        const std::string prefix = "Anonymous:";
        std::string tagName(reinterpret_cast<char*>(rawStack->data));
        fprintf(fpHookData_, "prctl;%u;%u;%" PRId64 ";%ld;tag:%s\n",
            rawStack->stackContext->pid, rawStack->stackContext->tid,
            static_cast<int64_t>(rawStack->stackContext->ts.tv_sec), rawStack->stackContext->ts.tv_nsec,
            (prefix + tagName).c_str());
        return;
    }
    std::string tag = "";
    switch (hookRecord->GetType()) {
        case FREE_MSG:
            tag = "free";
            break;
        case MALLOC_MSG:
            tag = "malloc";
            break;
        case MMAP_MSG:
            tag = "mmap";
            break;
        case MUNMAP_MSG:
            tag = "munmap";
            break;
        default:
            break;
    }

    fprintf(fpHookData_, "%s;%u;%u;%" PRId64 ";%ld;%zu\n", tag.c_str(),
        rawStack->stackContext->pid, rawStack->stackContext->tid,
        static_cast<int64_t>(rawStack->stackContext->ts.tv_sec),
        rawStack->stackContext->ts.tv_nsec, rawStack->stackContext->mallocSize);
    size_t idx = (hookConfig_.async_stack_enable() || !hookConfig_.fp_unwind()) ? FILTER_STACK_DEPTH : 0;
    for (; idx < callFrames.size(); ++idx) {
        (void)fprintf(fpHookData_, "0x%" PRIx64 ";0x%" PRIx64 ";%s;%s;0x%" PRIx64 ";%" PRIu64 "\n",
            callFrames[idx].ip_, callFrames[idx].sp_, std::string(callFrames[idx].symbolName_).c_str(),
            std::string(callFrames[idx].filePath_).c_str(), callFrames[idx].offset_, callFrames[idx].symbolOffset_);
    }
}

template <typename T>
inline void StackPreprocess::SetFrameInfo(T& frame, CallFrame& callFrame)
{
    frame.set_ip(callFrame.ip_);
    if (hookConfig_.offline_symbolization()) {
        // when js mixes offline symbols, the js call stack is reported according to the online symbolization
        if (callFrame.isJsFrame_ && callFrame.symbolNameId_ != 0 && callFrame.filePathId_ != 0) {
            frame.set_sp(callFrame.sp_);
            frame.set_offset(callFrame.offset_);
            frame.set_symbol_offset(callFrame.symbolOffset_);
            frame.set_symbol_name_id(callFrame.symbolNameId_);
            frame.set_file_path_id(callFrame.filePathId_);
        }
        return;
    }
    frame.set_sp(callFrame.sp_);
    if (!(callFrame.symbolNameId_ != 0 && callFrame.filePathId_ != 0)) {
        frame.set_symbol_name(std::string(callFrame.symbolName_));
        frame.set_file_path(std::string(callFrame.filePath_));
    }
    frame.set_offset(callFrame.offset_);
    frame.set_symbol_offset(callFrame.symbolOffset_);
    frame.set_file_path(runtime_instance->ResolveMergedSoIp(callFrame.ip_, std::string(callFrame.filePath_)));
    if (callFrame.symbolNameId_ != 0 && callFrame.filePathId_ != 0) {
        frame.set_symbol_name_id(callFrame.symbolNameId_);
        frame.set_file_path_id(callFrame.filePathId_);
    }
}

template <typename T>
inline void StackPreprocess::ReportSymbolNameMap(CallFrame& callFrame, T& stackData)
{
    if (callFrame.needReport_ & SYMBOL_NAME_ID_REPORT) {
        auto hookData = stackData.add_events();
        auto symbolMap = hookData->mutable_symbol_name();
        symbolMap->set_id(callFrame.symbolNameId_);
        symbolMap->set_name(std::string(callFrame.symbolName_));
        symbolMap->set_pid(pid_);
    }
}

template <typename T>
inline void StackPreprocess::ReportFilePathMap(CallFrame& callFrame, T& stackData)
{
    if (callFrame.needReport_ & FILE_PATH_ID_REPORT) {
        auto hookData = stackData.add_events();
        auto filePathMap = hookData->mutable_file_path();
        filePathMap->set_id(callFrame.filePathId_);
        filePathMap->set_name(std::string(callFrame.filePath_));
        filePathMap->set_pid(pid_);
        if (NeedBuildIdForPath(callFrame)) {
            filePathMap->set_build_id(GetBuildId(std::string(callFrame.filePath_)));
        }
    }
}

template <typename T>
inline void StackPreprocess::ReportFrameMap(CallFrame& callFrame, T& stackData)
{
    if (callFrame.needReport_ & CALL_FRAME_REPORT) {
        if ((!hookConfig_.fp_unwind()) && callFrame.callFrameId_ == DWARF_ERROR_ID && !unwindFailReport_) {
            return;
        } else if ((!hookConfig_.fp_unwind()) && callFrame.callFrameId_ == DWARF_ERROR_ID && unwindFailReport_) {
            unwindFailReport_ = false;
        }
        std::lock_guard<std::mutex> guard(mtx_);
        ReportSymbolNameMap(callFrame, stackData);
        ReportFilePathMap(callFrame, stackData);
        auto hookData = stackData.add_events();
        auto frameMap = hookData->mutable_frame_map();
        frameMap->set_id(callFrame.callFrameId_);
        auto frame = frameMap->mutable_frame();
        SetFrameInfo(*frame, callFrame);
        frameMap->set_pid(pid_);
    }
}

void StackPreprocess::SetMapsInfo()
{
    std::lock_guard<std::mutex> guard(mtx_);
    for (auto& itemSoBegin : runtime_instance->GetOfflineMaps()) {
        auto& maps = runtime_instance->GetMapsCache();
        auto mapsIter = maps.find(itemSoBegin);
        if (mapsIter == maps.end()) {
            continue;
        }

        auto& curMemMaps = mapsIter->second;
        bool needBasicData = runtime_instance->MarkOfflineFileReported(curMemMaps->filePathId_);
        ElfSymbolTable symbolInfo;
        if (needBasicData) {
            GetSymbols(curMemMaps->name_, symbolInfo);
            if (symbolInfo.symEntSize == 0) {
                runtime_instance->ResetOfflineFileReported(curMemMaps->filePathId_);
                continue;
            }
        }
        std::visit(
            [&](auto& stackData) {
                runtime_instance->TryEmitSmoMapsInfo(stackData);
                if (needBasicData) {
                    auto hookData = stackData.add_events();
                    auto filepathMap = hookData->mutable_file_path();
                    filepathMap->set_id(curMemMaps->filePathId_);
                    filepathMap->set_name(curMemMaps->name_);
                    filepathMap->set_pid(pid_);
                    auto& runtime = runtime_instance;
                    bool isAdlt = StartsWith(std::string(curMemMaps->name_), "/system/lib/libadlt") &&
                                  EndsWith(std::string(curMemMaps->name_), ".so");
                    if (isAdlt) {
                        uint64_t base = runtime->GetAdltBase(std::string(curMemMaps->name_));
                        filepathMap->set_adlt_base(base);
                    } else {
                        filepathMap->set_adlt_base(std::numeric_limits<uint64_t>::max());
                    }
                    filepathMap->set_build_id(GetBuildId(curMemMaps->name_));
                    SetSymbolInfo(curMemMaps->filePathId_, symbolInfo, stackData);
                }
                for (auto& map : curMemMaps->GetMaps()) {
                    if (map->prots & PROT_EXEC) {
                        auto nativeHookData = stackData.add_events();
                        auto mapSerialize = nativeHookData->mutable_maps_info();
                        mapSerialize->set_pid(pid_);
                        mapSerialize->set_start(map->begin);
                        mapSerialize->set_end(map->end);
                        mapSerialize->set_offset(map->offset);
                        mapSerialize->set_file_path_id(curMemMaps->filePathId_);
                    }
                }
                FlushData(stackData);
            },
            stackData_);
    }
    runtime_instance->ClearOfflineMaps();
}

template <typename T>
void StackPreprocess::SetSymbolInfo(uint32_t filePathId, ElfSymbolTable& symbolInfo, T& batchNativeHookData)
{
    if (symbolInfo.symEntSize == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "SetSymbolInfo get symbolInfo failed");
        return;
    }
    auto hookData = batchNativeHookData.add_events();
    auto symTable = hookData->mutable_symbol_tab();
    symTable->set_file_path_id(filePathId);
    symTable->set_text_exec_vaddr(symbolInfo.textVaddr);
    symTable->set_text_exec_vaddr_file_offset(symbolInfo.textOffset);
    symTable->set_sym_entry_size(symbolInfo.symEntSize);
    symTable->set_sym_table(symbolInfo.symTable.data(), symbolInfo.symTable.size());
    symTable->set_str_table(symbolInfo.strTable.data(), symbolInfo.strTable.size());
    symTable->set_pid(pid_);
}

template <typename T>
void StackPreprocess::FlushCheck(T& stackData)
{
    if (hookConfig_.statistics_interval() > 0) {
        if (!statisticsModelFlushCallstack_) {
            return;
        }
        if constexpr (std::is_same<T, ::BatchNativeHookData>::value) {
            FlushData(stackData);
        } else {
            uint64_t dataLen = static_cast<uint64_t>(stackData.Size());
            if (dataLen > flushSize_) {
                FlushData(stackData);
            }
        }
        statisticsModelFlushCallstack_ = false;
    } else {
        FlushData(stackData);
    }
}

void StackPreprocess::FlushData(BatchNativeHookData& stackData)
{
    if (buffer_ == nullptr) {
        return;
    }
    if (stackData.events().size() > 0) {
        size_t length = stackData.ByteSizeLong();
        if (length < bufferSize_) {
            stackData.SerializeToArray(buffer_.get(), length);
            if (isHookStandaloneSerialize_) {
                std::string str;
                ForStandard::BatchNativeHookData StandardStackData;
                StandardStackData.ParseFromArray(buffer_.get(), length);
                google::protobuf::TextFormat::PrintToString(StandardStackData, &str);
                size_t n = fwrite(str.data(), 1, str.size(), fpHookData_);
                fflush(fpHookData_);
                std::get<::BatchNativeHookData>(stackData_).clear_events();
                PROFILER_LOG_DEBUG(LOG_CORE, "Flush Data fwrite n = %zu str.size() = %zu", n, str.size());
            } else {
                Flush(buffer_.get(), length);
            }
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "the data is larger than MAX_BUFFER_SIZE, flush failed");
            std::get<::BatchNativeHookData>(stackData_).clear_events();
        }
    }
}

void StackPreprocess::FlushData(ProtoEncoder::BatchNativeHookData& stackData)
{
    if (stackData.Size() == 0) {
        return;
    }

    int messageLen = stackData.Finish();
    RandomWriteCtx* ctx = nullptr;
    if ((!isSaService_) && (resultWriter_ != nullptr)) {
        resultWriter_->finishReport(resultWriter_, messageLen);
        resultWriter_->flush(resultWriter_);
        ctx = resultWriter_->startReport(resultWriter_);
    } else {
        profilerPluginData_.finishAdd_data(messageLen);
        int32_t mesgLen = FinishReport();
        dataFlushSize_ += static_cast<uint32_t>(mesgLen);
        if (dataFlushSize_ >= FLUSH_BASELINE_SA && writer_ != nullptr) {
            std::shared_ptr<TraceFileWriter> tfPtr = std::static_pointer_cast<TraceFileWriter>(writer_);
            tfPtr->UpdateSaFileHeader();
            dataFlushSize_ = 0;
        }
        ctx = StartReport();
    }

    if (ctx == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get RandomWriteCtx FAILED!", __func__);
        return;
    }
    stackData_ = ProtoEncoder::BatchNativeHookData(ctx);
}

void StackPreprocess::Flush(const uint8_t* src, size_t size)
{
    if (src == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "Flush src is nullptr");
        return;
    }

    if (writer_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "Flush writer_ is nullptr");
        return;
    }
    if (hookConfig_.blocked()) {
        writer_->WriteBlock(src, size);
    } else {
        writer_->Write(src, size);
    }
    writer_->Flush();

    std::get<::BatchNativeHookData>(stackData_).clear_events();
}

void StackPreprocess::GetSymbols(const std::string& filePath, ElfSymbolTable& symbols)
{
    RegularElfFactory elfFactory(filePath);
    auto elfPtr = elfFactory.Create();

    symbols.textVaddr = elfPtr->GetStartVaddr();
    symbols.textOffset = elfPtr->GetStartOffset();
    if (symbols.textVaddr == (std::numeric_limits<uint64_t>::max)()) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetSymbols get textVaddr failed");
        return;
    }

    std::string symSecName;
    std::string strSecName;
    ShdrInfo shdr;
    if (elfPtr->GetSectionInfo(shdr, ".symtab")) {
        symSecName = ".symtab";
        strSecName = ".strtab";
    } else if (elfPtr->GetSectionInfo(shdr, ".dynsym")) {
        symSecName = ".dynsym";
        strSecName = ".dynstr";
    } else {
        return;
    }
    symbols.symEntSize = shdr.entSize;
    symbols.symTable.resize(shdr.size);
    if (!elfPtr->GetSectionData(symbols.symTable.data(), shdr.size, symSecName)) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetSymbols get symbol section data failed");
        return;
    }
    if (!elfPtr->GetSectionInfo(shdr, strSecName)) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetSymbols get str section failed");
        return;
    }
    symbols.strTable.resize(shdr.size);
    if (!elfPtr->GetSectionData(symbols.strTable.data(), shdr.size, strSecName)) {
        PROFILER_LOG_ERROR(LOG_CORE, "GetSymbols get str section failed");
        return;
    }
}

void StackPreprocess::CleanupFullyReleasedStack(uint32_t stackId)
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        auto recordIt = simplifiedRecordStatisticsMap_.find(stackId);
        if (recordIt != simplifiedRecordStatisticsMap_.end()) {
            simplifiedRecordStatisticsMap_.erase(recordIt);
        }
    } else {
        auto recordIt = recordStatisticsMap_.find(stackId);
        if (recordIt != recordStatisticsMap_.end()) {
            recordStatisticsMap_.erase(recordIt);
        }
    }
}

void StackPreprocess::InitializeFunctionPointers()
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        findSyncCallStackIdFunc_ = &StackPreprocess::FindSimplifiedSyncCallStackId;
        findAsyncCallStackIdFunc_ = &StackPreprocess::FindSimplifiedAsyncCallStackId;
        findAsyncStatisticsStackIdFunc_ = &StackPreprocess::FindSimplifiedAsyncCallStackId;
        addCallStackIndexFunc_ = &StackPreprocess::AddSimplifiedCallStackIndex;
    } else {
        findSyncCallStackIdFunc_ = &StackPreprocess::FindSyncCallStackId;
        findAsyncCallStackIdFunc_ = &StackPreprocess::FindFullAsyncCallStackId;
        findAsyncStatisticsStackIdFunc_ = &StackPreprocess::FindFullAsyncStatisticsStackId;
        addCallStackIndexFunc_ = &StackPreprocess::AddCallStackIndex;
    }
}

template <typename T, typename StackData>
void StackPreprocess::FlushStatisticsEntry(T* statistics, uint32_t stackId, StackData& stackData,
    const struct timespec& timestamp)
{
    auto hookData = stackData.add_events();
    hookData->set_tv_sec(timestamp.tv_sec);
    hookData->set_tv_nsec(timestamp.tv_nsec);
    auto recordEvent = hookData->mutable_statistics_event();
    recordEvent->set_pid(pid_);
    recordEvent->set_callstack_id(stackId);
    recordEvent->set_type(statistics->type);
    recordEvent->set_apply_count(statistics->applyCount);
    recordEvent->set_release_count(statistics->releaseCount);
    recordEvent->set_apply_size(statistics->applySize);
    recordEvent->set_release_size(statistics->releaseSize);
    std::string tagName;
    GetMemTag(statistics->tagId, tagName);
    recordEvent->set_tag_name(tagName);
    recordEvent->set_node_type(statistics->GetNodeType());
    recordEvent->set_node_id(statistics->GetNodeId());
    for (auto addr : statistics->addrs) {
        recordEvent->add_addr(addr);
    }
}

template <typename T>
bool StackPreprocess::FlushRecordStatisticsImpl(std::unordered_map<uint32_t, T>& statisticsMap)
{
    spinLock_.Lock();
    std::lock_guard<std::mutex> guard(mtx_);
    if (statisticsPeriodSet_.empty()) {
        spinLock_.Unlock();
        return false;
    }
    std::visit([&](auto& stackData) { FlushData(stackData); }, stackData_);
    struct timespec timestamp;
    if (hookConfig_.use_file_cache_mode()) {
        timestamp = lastCacheStatisticsTime_;
    } else {
        clock_gettime(hookDataClockId_, &timestamp);
    }
    std::visit(
        [&](auto& stackData) {
            uint32_t count = 0;
            for (auto& stackId : statisticsPeriodSet_) {
                auto iter = statisticsMap.find(stackId);
                if (iter == statisticsMap.end()) {
                    continue;
                }
                auto statistics = &iter->second;
                bool shouldClean = discardDestroyedTraces_ &&
                    statistics->applySize == statistics->releaseSize;
                if (shouldClean && statistics->createInCurPeriod) {
                    statisticsMap.erase(iter);
                    continue;
                }
                // if the stack is not created in this period, data will not write to file.
                FlushStatisticsEntry(statistics, stackId, stackData, timestamp);
                if (statistics->applySize == statistics->releaseSize) {
                    statisticsMap.erase(iter);
                } else {
                    statistics->createInCurPeriod = false;
                }
                if (++count % STATISTICS_PERIOD_DATA_FLUSH_BATCH == 0) {
                    FlushData(stackData);
                    count = 0;
                }
            }
            FlushData(stackData);
        },
        stackData_);
    statisticsPeriodSet_.clear();
    spinLock_.Unlock();
    return true;
}

__attribute__((noinline)) bool StackPreprocess::FlushRecordStatistics()
{
    if (!hookConfig_.arkui_mem_trace_enable()) {
        return FlushRecordStatisticsImpl<SimplifiedRecordStatistics>(simplifiedRecordStatisticsMap_);
    } else {
        return FlushRecordStatisticsImpl<RecordStatistic>(recordStatisticsMap_);
    }
}

void StackPreprocess::SaveMemTag(uint32_t tagId, const std::string& tagName)
{
    memTagMap_.EnsureInsert(tagId, tagName);
}

bool StackPreprocess::GetMemTag(uint32_t tagId, std::string& tagName)
{
    return memTagMap_.Find(tagId, tagName);
}

void StackPreprocess::SaveJsRawStack(uint64_t jsChainId, const char* jsRawStack)
{
    if (jsRawStack == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> guard(jsMapMtx_);
    auto iterChainId = jsStackMap_.find(jsChainId);
    if (iterChainId == jsStackMap_.end()) {
        auto iterRawStack = jsStackSet_.find(jsRawStack);
        if (iterRawStack == jsStackSet_.end()) {
            auto iter = jsStackSet_.insert(jsRawStack);
            jsStackMap_[jsChainId] = iter.first->c_str();
        } else {
            jsStackMap_[jsChainId] = iterRawStack->c_str();
        }
    }
}

const char* StackPreprocess::GetJsRawStack(uint64_t jsChainId)
{
    std::lock_guard<std::mutex> guard(jsMapMtx_);
    auto iter = jsStackMap_.find(jsChainId);
    if (iter != jsStackMap_.end()) {
        return iter->second;
    }
    return nullptr;
}

void StackPreprocess::WriteHookConfig()
{
    const size_t configSize = hookConfig_.ByteSizeLong();
    auto buffer = std::make_unique<uint8_t[]>(configSize);
    hookConfig_.SerializeToArray(buffer.get(), configSize);
    if (writer_ != nullptr) {
        writer_->ResetPos();
        profilerPluginData_.Reset(writer_->GetCtx());
    }
    profilerPluginData_.set_name("nativehook_config");
    profilerPluginData_.set_version("1.02");
    profilerPluginData_.set_status(0);
    profilerPluginData_.set_data(buffer.get(), configSize);

    FinishReport();

    auto ctx = StartReport();
    if (ctx == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get RandomWriteCtx FAILED!", __func__);
        return;
    }
    if (isHookStandaloneSerialize_) {
        stackData_ = BatchNativeHookData();
    } else {
        stackData_ = ProtoEncoder::BatchNativeHookData(ctx);
    }
}

RandomWriteCtx* StackPreprocess::StartReport()
{
    if (writer_ != nullptr) {
        writer_->ResetPos();
        profilerPluginData_.Reset(writer_->GetCtx());
    }
    profilerPluginData_.set_name("nativehook");
    profilerPluginData_.set_version("1.02");
    profilerPluginData_.set_status(0);
    return profilerPluginData_.startAdd_data();
}

int32_t StackPreprocess::FinishReport()
{
    struct timespec ts;
    clock_gettime(pluginDataClockId_, &ts);
    profilerPluginData_.set_clock_id(static_cast<ProfilerPluginData_ClockId>(pluginDataClockId_));
    profilerPluginData_.set_tv_sec(ts.tv_sec);
    profilerPluginData_.set_tv_nsec(ts.tv_nsec);

    int32_t len = profilerPluginData_.Finish();
    if (writer_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: the writer is nullptr!", __func__);
        return 0;
    }
    writer_->FinishReport(len);
    return len;
}

void StackPreprocess::FlushRecordApplyAndReleaseMatchData()
{
    std::lock_guard<std::mutex> guard(mtx_);
    if (applyAndReleaseMatchPeriodListData_.empty()) {
        return;
    }
    std::visit([&](auto& stackData) {
        for (const auto& rawStack: applyAndReleaseMatchPeriodListData_) {
            auto hookData = stackData.add_events();
            hookData->set_tv_sec(rawStack.ts.tv_sec);
            hookData->set_tv_nsec(rawStack.ts.tv_nsec);
            if (rawStack.type == MALLOC_MSG) {
                auto allocEvent = hookData->mutable_alloc_event();
                allocEvent->set_node_type(rawStack.nodeType);
                allocEvent->set_node_id(rawStack.nodeId);
                SetEventFrame(rawStack, allocEvent, rawStack.stackMapId);
            } else if (rawStack.type == MMAP_MSG) {
                auto mmapEvent = hookData->mutable_mmap_event();
                SetEventFrame(rawStack, mmapEvent, rawStack.stackMapId);
            } else if (rawStack.type == MMAP_FILE_PAGE_MSG) {
                auto mmapEvent = hookData->mutable_mmap_event();
                const std::string prefix = "FilePage:";
                std::string tagName;
                if (GetMemTag(rawStack.tagId, tagName)) {
                    tagName = prefix + tagName;
                }
                SetEventFrame(rawStack, mmapEvent, rawStack.stackMapId, tagName);
            } else if (rawStack.type == PR_SET_VMA_MSG) {
                auto tagEvent = hookData->mutable_tag_event();
                const std::string prefix = "Anonymous:";
                tagEvent->set_addr(rawStack.addr);
                tagEvent->set_size(rawStack.mallocSize);
                tagEvent->set_tag(prefix + prctlPeriodTags_[rawStack.tagId]);
                tagEvent->set_pid(pid_);
            } else if (rawStack.type == MEMORY_USING_MSG) {
                auto traceAllocEvent = hookData->mutable_trace_alloc_event();
                traceAllocEvent->set_node_type(rawStack.nodeType);
                traceAllocEvent->set_node_id(rawStack.nodeId);
                std::string tagName;
                GetMemTag(rawStack.tagId, tagName);
                if (tagName != "") {
                    traceAllocEvent->set_tag_name(tagName);
                }
                HookRecord::SetTraceType(traceAllocEvent, tagName);
                SetEventFrame(rawStack, traceAllocEvent, rawStack.stackMapId, tagName);
            } else if (rawStack.type == MEMORY_UNUSING_MSG) {
                auto traceFreeEvent = hookData->mutable_trace_free_event();
                std::string tagName;
                GetMemTag(rawStack.tagId, tagName);
                if (tagName != "") {
                    traceFreeEvent->set_tag_name(tagName);
                }
                HookRecord::SetTraceType(traceFreeEvent, tagName);
                SetEventFrame(rawStack, traceFreeEvent, rawStack.stackMapId);
            }
        }
        FlushData(stackData);
        }, stackData_);
    applyAndReleaseMatchPeriodListData_.clear();
    applyAndReleaseMatchIntervallMap_.clear();
    prctlPeriodTags_.clear();

    if (releasedPeriodListData_.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "releasedPeriodListData_ is empty!");
        return;
    }
    std::visit([&](auto& stackData) {
        for (const auto& rawStack: releasedPeriodListData_) {
            auto hookData = stackData.add_events();
            hookData->set_tv_sec(rawStack.ts.tv_sec);
            hookData->set_tv_nsec(rawStack.ts.tv_nsec);
            if (rawStack.type == FREE_MSG) {
                auto freeEvent = hookData->mutable_free_event();
                SetEventFrame(rawStack, freeEvent, rawStack.stackMapId);
            } else if (rawStack.type == MUNMAP_MSG) {
                auto munmapEvent = hookData->mutable_munmap_event();
                SetEventFrame(rawStack, munmapEvent, rawStack.stackMapId);
            }
        }
        FlushData(stackData);
        }, stackData_);
    releasedPeriodListData_.clear();
}

std::string StackPreprocess::GetBuildId(const std::string& soName)
{
    OHOS::HiviewDFX::RegularElfFactory elfFactory(soName);
    auto elf = elfFactory.Create();
    if (elf == nullptr || !elf->IsValid()) {
        PROFILER_LOG_WARN(LOG_CORE, "GetBuildId Failed to create valid DfxElf for: %s", soName.c_str());
        return "";
    }
    std::string buildId = elf->GetBuildId();
    return buildId;
}

template void StackPreprocess::SetEventFrame<AllocEvent>(const ReportEventBaseData& rawStack,
    AllocEvent* event, uint32_t stackMapId, const std::string& type);
template void StackPreprocess::SetEventFrame<FreeEvent>(const ReportEventBaseData& rawStack,
    FreeEvent* event, uint32_t stackMapId, const std::string& type);
template void StackPreprocess::SetEventFrame<MmapEvent>(const ReportEventBaseData& rawStack,
    MmapEvent* event, uint32_t stackMapId, const std::string& type);
template void StackPreprocess::SetEventFrame<TraceAllocEvent>(const ReportEventBaseData& rawStack,
    TraceAllocEvent* event, uint32_t stackMapId, const std::string& type);
template void StackPreprocess::SetEventFrame<TraceFreeEvent>(const ReportEventBaseData& rawStack,
    TraceFreeEvent* event, uint32_t stackMapId, const std::string& type);
