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
#include <unistd.h>

#include "common.h"
#include "logging.h"
#include "plugin_service_types.pb.h"
#include "dfx_elf.h"
#include "utilities.h"
#include "native_hook_result_standard.pb.h"
#include "native_hook_config_standard.pb.h"
#include "google/protobuf/text_format.h"
#include "trace_file_writer.h"


constexpr static uint32_t SC_LG_TINY_MIN = 3;
constexpr static uint32_t LG_QUANTUM = 4;
constexpr static uint32_t SC_NTINY = LG_QUANTUM - SC_LG_TINY_MIN;
constexpr static uint32_t SC_LG_TINY_MAXCLASS = (LG_QUANTUM > SC_LG_TINY_MIN ? LG_QUANTUM - 1 : -1);
constexpr static uint32_t SC_LG_NGROUP = 2;
constexpr static uint32_t LG_SIZE_CLASS_GROUP = 2;
constexpr static uint32_t NTBINS = 1;
constexpr static uint32_t LG_TINY_MAXCLASS = 3;
constexpr static uint32_t MAX_BUFFER_SIZE = 10 * 1024 * 1024;
constexpr static uint32_t MAX_MATCH_CNT = 1000;
constexpr static uint32_t MAX_MATCH_INTERVAL = 2000;
constexpr static uint32_t LOG_PRINT_TIMES = 10000;
constexpr static uint32_t MAX_BATCH_CNT = 5;
constexpr static uint32_t RIGHT_MOVE_1 = 1;
constexpr static uint32_t RIGHT_MOVE_2 = 2;
constexpr static uint32_t RIGHT_MOVE_4 = 4;
constexpr static uint32_t RIGHT_MOVE_8 = 8;
constexpr static uint32_t RIGHT_MOVE_16 = 16;
constexpr static uint64_t SIZE_MASK = 0xFFFFFF0000000000;

using namespace OHOS::Developtools::NativeDaemon;
using namespace OHOS::HiviewDFX;

StackPreprocess::StackPreprocess(const StackDataRepeaterPtr& dataRepeater,
    const NativeHookConfig& hookConfig,
    clockid_t pluginDataClockId, FILE* fpHookData,
    bool isHookStandalone) : dataRepeater_(dataRepeater), buffer_(new (std::nothrow) uint8_t[MAX_BUFFER_SIZE]),
                             hookConfig_(hookConfig), pluginDataClockId_(pluginDataClockId), fpHookData_(fpHookData),
                             isHookStandaloneSerialize_(isHookStandalone)
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
    }
    PROFILER_LOG_INFO(LOG_CORE, "statistics_interval = %d statisticsInterval_ = %lld \n",
        hookConfig_.statistics_interval(), statisticsInterval_.count());
    hookDataClockId_ = COMMON::GetClockId(hookConfig_.clock());
    PROFILER_LOG_INFO(LOG_CORE, "StackPreprocess(): pluginDataClockId = %d hookDataClockId = %d \n",
        pluginDataClockId_, hookDataClockId_);
    if (hookConfig_.save_file() && fpHookData_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "If you need to save the file, please set the file_name");
    }
    PROFILER_LOG_INFO(LOG_CORE, "isHookStandaloneSerialize_ = %d", isHookStandaloneSerialize_);
#if defined(__arm__)
    u64regs_.resize(PERF_REG_ARM_MAX);
#else
    u64regs_.resize(PERF_REG_ARM64_MAX);
#endif
    callFrames_.reserve(hookConfig_.max_stack_depth());
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

    if (isSaService_) {
        std::shared_ptr<TraceFileWriter> tfPtr = std::static_pointer_cast<TraceFileWriter>(writer_);
        tfPtr->SetDurationTime();
        tfPtr->Finish();
    }
}

void StackPreprocess::SetWriter(const std::shared_ptr<Writer>& writer)
{
    writer_ = writer;
}

bool StackPreprocess::StartTakeResults()
{
    CHECK_NOTNULL(dataRepeater_, false, "data repeater null");

    std::thread demuxer(&StackPreprocess::TakeResults, this);
    CHECK_TRUE(demuxer.get_id() != std::thread::id(), false, "demuxer thread invalid");

    thread_ = std::move(demuxer);
    isStopTakeData_ = false;
    return true;
}

bool StackPreprocess::StopTakeResults()
{
    PROFILER_LOG_INFO(LOG_CORE, "start StopTakeResults");
    if (!dataRepeater_) {
        isStopTakeData_ = true;
        return true;
    }
    CHECK_NOTNULL(dataRepeater_, false, "data repeater null");
    CHECK_TRUE(thread_.get_id() != std::thread::id(), false, "thread invalid");

    isStopTakeData_ = true;
    dataRepeater_->PutRawStack(nullptr, false);
    PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults Wait thread join");

    if (thread_.joinable()) {
        thread_.join();
    }
    PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults Wait thread join success");
    return true;
}

inline void StackPreprocess::IntervalFlushRecordStatistics(BatchNativeHookData& stackData)
{
    {
        std::lock_guard<std::mutex> guard(mtx_);
        FlushData(stackData);
    }
    // interval reporting statistics
    if (hookConfig_.statistics_interval() > 0) {
        static auto lastStatisticsTime = std::chrono::steady_clock::now();
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastStatisticsTime);
        if (elapsedTime >= statisticsInterval_) {
            lastStatisticsTime = currentTime;
            FlushRecordStatistics();
        }
    }
}

bool StackPreprocess::HandleNoStackEvent(RawStackPtr& rawData, BatchNativeHookData& stackData)
{
    if (rawData->stackConext->type == MMAP_FILE_TYPE) {
        BaseStackRawData* mmapRawData = rawData->stackConext;
        std::string filePath(reinterpret_cast<char *>(rawData->data));
        COMMON::AdaptSandboxPath(filePath, rawData->stackConext->pid);
        PROFILER_LOG_DEBUG(LOG_CORE, "MMAP_FILE_TYPE curMmapAddr=%p, MAP_FIXED=%d, "
                    "PROT_EXEC=%d, offset=%" PRIu64 ", filePath=%s",
                    mmapRawData->addr, mmapRawData->mmapArgs.flags & MAP_FIXED,
                    mmapRawData->mmapArgs.flags & PROT_EXEC, mmapRawData->mmapArgs.offset, filePath.data());
        {
            std::lock_guard<std::mutex> guard(mtx_);
            runtime_instance->HandleMapInfo(reinterpret_cast<uint64_t>(mmapRawData->addr),
                mmapRawData->mallocSize, mmapRawData->mmapArgs.flags, mmapRawData->mmapArgs.offset, filePath);
        }
        flushBasicData_ = true;
    } else if (rawData->stackConext->type == THREAD_NAME_MSG) {
        std::string threadName = reinterpret_cast<char*>(rawData->data);
        ReportThreadNameMap(rawData->stackConext->tid, threadName, stackData);
    } else {
        return false;
    }
    return true;
}

void StackPreprocess::TakeResultsFromShmem(const std::shared_ptr<EventNotifier>& eventNotifier,
                                           const std::shared_ptr<ShareMemoryBlock>& shareMemoryBlock)
{
    eventNotifier->Take();
    StackDataRepeater::RawStack rawStack;
    RawStackPtr rawData(&rawStack, [](StackDataRepeater::RawStack* del) {});
    while (!isStopTakeData_) {
        BatchNativeHookData stackData;
        bool ret = shareMemoryBlock->TakeData(
            [&](const int8_t data[], uint32_t size) -> bool {
                if (size == sizeof(uint64_t)) {
                    uint64_t addr = *reinterpret_cast<uint64_t *>(const_cast<int8_t *>(data));
                    SetFreeStatisticsData(addr);
                    return true;
                }
                CHECK_TRUE(size >= sizeof(BaseStackRawData), false, "stack data invalid!");
                rawData->stackConext = reinterpret_cast<BaseStackRawData *>(const_cast<int8_t *>(data));
                rawData->data = reinterpret_cast<uint8_t*>(const_cast<int8_t *>(data)) + sizeof(BaseStackRawData);
                rawData->fpDepth = (size - sizeof(BaseStackRawData)) / sizeof(uint64_t);
                if (isStopTakeData_) {
                    return false;
                } else if (rawData->stackConext->type == MEMORY_TAG) {
                    std::string tagName = reinterpret_cast<char*>(rawData->data);
                    SaveMemTag(rawData->stackConext->tagId, tagName);
                    return true;
                } else if (HandleNoStackEvent(rawData, stackData)) {
                    if (rawData->stackConext->type == THREAD_NAME_MSG) {
                        FlushData(stackData);
                    }
                    return true;
                } else if (rawData->stackConext->type == MUNMAP_MSG) {
                    std::lock_guard<std::mutex> guard(mtx_);
                    runtime_instance->RemoveMaps(reinterpret_cast<uint64_t>(rawData->stackConext->addr));
                }
                {
                    std::lock_guard<std::mutex> guard(mtx_);
                    runtime_instance->UpdateThread(rawData->stackConext->pid, rawData->stackConext->tid);
                }
                SetHookData(rawData, stackData);
                IntervalFlushRecordStatistics(stackData);
                return true;
        });
        if (!ret) {
            break;
        }
    }
}

void StackPreprocess::TakeResults()
{
    if (!dataRepeater_) {
        return;
    }

    size_t minStackDepth = hookConfig_.max_stack_depth() > MIN_STACK_DEPTH
        ? MIN_STACK_DEPTH : hookConfig_.max_stack_depth();
    if (hookConfig_.blocked()) {
        minStackDepth = static_cast<size_t>(hookConfig_.max_stack_depth());
    }
    minStackDepth += FILTER_STACK_DEPTH;
    PROFILER_LOG_INFO(LOG_CORE, "TakeResults thread %d, start!", gettid());
    while (1) {
        BatchNativeHookData stackData;
        RawStackPtr batchRawStack[MAX_BATCH_CNT] = {nullptr};
        auto result = dataRepeater_->TakeRawData(hookConfig_.malloc_free_matching_interval(), hookDataClockId_,
            MAX_BATCH_CNT, batchRawStack);
        if (!result || isStopTakeData_) {
            break;
        }
        for (unsigned int i = 0; i < MAX_BATCH_CNT; i++) {
            auto rawData = batchRawStack[i];
            if (!rawData || isStopTakeData_) {
                break;
            }
            if (HandleNoStackEvent(rawData, stackData)) {
                continue;
            } else if (rawData->stackConext->type == MUNMAP_MSG) {
                std::lock_guard<std::mutex> guard(mtx_);
                runtime_instance->RemoveMaps(reinterpret_cast<uint64_t>(rawData->stackConext->addr));
            }

            if (!rawData->reportFlag) {
                ignoreCnts_++;
                if (ignoreCnts_ % LOG_PRINT_TIMES == 0) {
                    PROFILER_LOG_INFO(LOG_CORE, "ignoreCnts_ = %d quene size = %zu\n",
                                      ignoreCnts_, dataRepeater_->Size());
                }
                continue;
            }
            eventCnts_++;
            if (eventCnts_ % LOG_PRINT_TIMES == 0) {
                PROFILER_LOG_INFO(LOG_CORE, "eventCnts_ = %d quene size = %zu\n", eventCnts_, dataRepeater_->Size());
            }
            callFrames_.clear();
            if (hookConfig_.fp_unwind()) {
#if defined(__aarch64__)
                uintptr_t pacMask = 0xFFFFFF8000000000;
#else
                uintptr_t pacMask = 0;
#endif
                uint64_t* fpIp = reinterpret_cast<uint64_t *>(rawData->data);
                for (uint8_t idx = 0; idx < rawData->fpDepth ; ++idx) {
                    if (fpIp[idx] == 0) {
                        break;
                    }
                    callFrames_.emplace_back(fpIp[idx] & (~pacMask));
                }
            } else {
#if defined(__arm__)
                uint32_t *regAddrArm = reinterpret_cast<uint32_t *>(rawData->data);
                u64regs_.assign(regAddrArm, regAddrArm + PERF_REG_ARM_MAX);
#else
                if (memcpy_s(u64regs_.data(), sizeof(uint64_t) * PERF_REG_ARM64_MAX, rawData->data,
                    sizeof(uint64_t) * PERF_REG_ARM64_MAX) != EOK) {
                    PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s regs failed");
                }
#endif
            }
#ifdef PERFORMANCE_DEBUG
            struct timespec start = {};
            clock_gettime(CLOCK_REALTIME, &start);
            size_t realFrameDepth = callFrames_.size();
#endif
            size_t stackDepth = ((size_t)hookConfig_.max_stack_depth() > MAX_CALL_FRAME_UNWIND_SIZE)
                        ? MAX_CALL_FRAME_UNWIND_SIZE
                        : hookConfig_.max_stack_depth() + FILTER_STACK_DEPTH;
            if (rawData->reduceStackFlag) {
                stackDepth = minStackDepth;
            }
            {
                std::lock_guard<std::mutex> guard(mtx_);
                bool ret = runtime_instance->UnwindStack(u64regs_, rawData->stackData, rawData->stackSize,
                    rawData->stackConext->pid, rawData->stackConext->tid, callFrames_, stackDepth);
                if (!ret) {
                    PROFILER_LOG_ERROR(LOG_CORE, "unwind fatal error");
                    continue;
                }
            }

            if (hookConfig_.save_file() && hookConfig_.file_name() != "" && isHookStandaloneSerialize_) {
                SetHookData(rawData, callFrames_, stackData);
            } else if (hookConfig_.save_file() && hookConfig_.file_name() != "") {
                WriteFrames(rawData, callFrames_);
            } else if (!hookConfig_.save_file()) {
                SetHookData(rawData, callFrames_, stackData);
            }
#ifdef PERFORMANCE_DEBUG
            struct timespec end = {};
            clock_gettime(CLOCK_REALTIME, &end);
            uint64_t curTimeCost = (end.tv_sec - start.tv_sec) * MAX_MATCH_CNT * MAX_MATCH_CNT * MAX_MATCH_CNT +
                (end.tv_nsec - start.tv_nsec);
            if (curTimeCost >= LONG_TIME_THRESHOLD) {
                PROFILER_LOG_ERROR(LOG_CORE, "bigTimeCost %" PRIu64 " event=%d, realFrameDepth=%zu, "
                    "callFramesDepth=%zu\n",
                    curTimeCost, rawData->stackConext->type, realFrameDepth, callFrames_.size());
            }
            timeCost += curTimeCost;
            unwindTimes++;
            if (unwindTimes % LOG_PRINT_TIMES == 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "unwindTimes %" PRIu64" cost time = %" PRIu64" mean cost = %" PRIu64"\n",
                    unwindTimes.load(), timeCost.load(), timeCost.load() / unwindTimes.load());
            }
#endif
        }
        if (hookConfig_.save_file() && hookConfig_.file_name() != "" && !isHookStandaloneSerialize_) {
            continue;
        }
        IntervalFlushRecordStatistics(stackData);
    }
    PROFILER_LOG_INFO(LOG_CORE, "TakeResults thread %d, exit!", gettid());
}

inline void StackPreprocess::ReportThreadNameMap(uint32_t tid, const std::string& tname,
                                                 BatchNativeHookData& batchNativeHookData)
{
    std::lock_guard<std::mutex> guard(mtx_);
    auto it = threadNameMap_.find(tid);
    if (it == threadNameMap_.end() || it->second != tname) {
        threadNameMap_[tid] = tname;
        auto hookData = batchNativeHookData.add_events();
        auto* thread = hookData->mutable_thread_name_map();
        thread->set_id(tid);
        thread->set_name(tname);
        thread->set_pid(pid_);
    }
}

inline void StackPreprocess::FillOfflineCallStack(std::vector<CallFrame>& callFrames, size_t idx)
{
    for (; idx < callFrames.size(); ++idx) {
        callStack_.push_back(callFrames[idx].ip_);
    }
}

inline void StackPreprocess::FillCallStack(std::vector<CallFrame>& callFrames,
    BatchNativeHookData& batchNativeHookData, size_t idx)
{
    for (; idx < callFrames.size(); ++idx) {
        ReportFrameMap(callFrames[idx], batchNativeHookData);
        // for call stack id
        callStack_.push_back(callFrames[idx].callFrameId_);
    }
}

inline uint32_t StackPreprocess::FindCallStackId(std::vector<uint64_t>& callStack)
{
    if (hookConfig_.response_library_mode()) {
        auto itStack = responseLibraryMap_.find(callStack[0]);
        if (itStack != responseLibraryMap_.end()) {
            return itStack->second;
        }
    } else {
        auto itStack = callStackMap_.find(callStack);
        if (itStack != callStackMap_.end()) {
            return itStack->second;
        }
    }
    return 0;
}

/**
 * @return '0' is invalid stack id, '> 0' is valid stack id
 */
inline uint32_t StackPreprocess::SetCallStackMap(BatchNativeHookData& batchNativeHookData)
{
    auto hookData = batchNativeHookData.add_events();
    StackMap* stackmap = hookData->mutable_stack_map();
    uint32_t stackId = 0;
    if (hookConfig_.response_library_mode()) {
        stackId = responseLibraryMap_.size() + 1;
    } else {
        stackId = callStackMap_.size() + 1;
    }
    stackmap->set_id(stackId);
    stackmap->set_pid(pid_);
    // offline symbolization use ip, other use frame_map_id
    if (hookConfig_.offline_symbolization()) {
        for (size_t i = 0; i < callStack_.size(); i++) {
            stackmap->add_ip(callStack_[i]);
        }
    } else {
        for (size_t i = 0; i < callStack_.size(); i++) {
            stackmap->add_frame_map_id(callStack_[i]);
        }
    }
    if (hookConfig_.response_library_mode()) {
        responseLibraryMap_[callStack_[0]] = stackId;
    } else {
        callStackMap_[callStack_] = stackId;
    }
    return stackId;
}

/**
 * @return '0' is invalid stack id, '> 0' is valid stack id
 */
inline uint32_t StackPreprocess::GetCallStackId(const RawStackPtr& rawStack,
    std::vector<CallFrame>& callFrames,
    BatchNativeHookData& batchNativeHookData)
{
    // ignore the first two frame if dwarf unwind
    size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
    // if free_stack_report or munmap_stack_report is false, don't need to record.
    if ((rawStack->stackConext->type == FREE_MSG) && !hookConfig_.free_stack_report()) {
        return 0;
    } else if ((rawStack->stackConext->type == MUNMAP_MSG) && !hookConfig_.munmap_stack_report()) {
        return 0;
    }
    callStack_.clear();
    callStack_.reserve(callFrames.size());
    if (!hookConfig_.offline_symbolization()) {
        FillCallStack(callFrames, batchNativeHookData, idx);
    } else {
        FillOfflineCallStack(callFrames, idx);
    }
    // return call stack id
    std::lock_guard<std::mutex> guard(mtx_);
    uint32_t stackId = FindCallStackId(callStack_);
    if (stackId > 0) {
        return stackId;
    } else {
        return SetCallStackMap(batchNativeHookData);
    }
}

template <typename T>
void StackPreprocess::SetEventFrame(const RawStackPtr& rawStack,
    std::vector<CallFrame>& callFrames,
    BatchNativeHookData& batchNativeHookData,
    T* event, uint32_t stackMapId)
{
    // ignore the first two frame if dwarf unwind
    size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
    event->set_pid(rawStack->stackConext->pid);
    event->set_tid(rawStack->stackConext->tid);
    event->set_addr((uint64_t)rawStack->stackConext->addr);

    if (hookConfig_.callframe_compress() && stackMapId != 0) {
        event->set_stack_id(stackMapId);
    } else {
        for (; idx < callFrames.size(); ++idx) {
            Frame* frame = event->add_frame_info();
            SetFrameInfo(*frame, callFrames[idx]);
        }
    }
}

void StackPreprocess::SetAllocStatisticsFrame(const RawStackPtr& rawStack,
    std::vector<CallFrame>& callFrames,
    BatchNativeHookData& batchNativeHookData)
{
    // ignore the first two frame if dwarf unwind
    size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
    callStack_.clear();
    callStack_.reserve(callFrames.size() + 1);
    callStack_.push_back(rawStack->stackConext->mallocSize | SIZE_MASK);
    if (!hookConfig_.offline_symbolization()) {
        FillCallStack(callFrames, batchNativeHookData, idx);
    } else {
        FillOfflineCallStack(callFrames, idx);
    }

    std::lock_guard<std::mutex> guard(mtx_);
    // by call stack id set alloc statistics data.
    uint32_t stackId = FindCallStackId(callStack_);
    if (stackId > 0) {
        SetAllocStatisticsData(rawStack, stackId, true);
    } else {
        stackId = SetCallStackMap(batchNativeHookData);
        SetAllocStatisticsData(rawStack, stackId);
    }
}

void StackPreprocess::SetAllocStatisticsFrame(const RawStackPtr& rawStack,
    BatchNativeHookData& batchNativeHookData)
{
    callStack_.resize(rawStack->fpDepth + 1);
    callStack_[0] = (rawStack->stackConext->mallocSize | SIZE_MASK);
    if (memcpy_s(callStack_.data() + 1, sizeof(uint64_t) * rawStack->fpDepth,
                 rawStack->data, sizeof(uint64_t) * rawStack->fpDepth) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s callStack_ failed");
        return;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    // by call stack id set alloc statistics data.
    uint32_t stackId = FindCallStackId(callStack_);
    if (stackId > 0) {
        SetAllocStatisticsData(rawStack, stackId, true);
    } else {
        stackId = SetCallStackMap(batchNativeHookData);
        SetAllocStatisticsData(rawStack, stackId);
    }
}

void StackPreprocess::SetHookData(RawStackPtr rawStack, BatchNativeHookData& batchNativeHookData)
{
    if (flushBasicData_) {
        SetMapsInfo(rawStack->stackConext->pid);
        flushBasicData_ = false;
    }
    // statistical reporting must is compressed and accurate.
    switch (rawStack->stackConext->type) {
        case FREE_MSG:
        case MUNMAP_MSG:
        case MEMORY_UNUSING_MSG: {
            SetFreeStatisticsData((uint64_t)rawStack->stackConext->addr);
            break;
        }
        case MALLOC_MSG:
            rawStack->stackConext->mallocSize = ComputeAlign(rawStack->stackConext->mallocSize);
        case MMAP_MSG:
        case MMAP_FILE_PAGE_MSG:
        case MEMORY_USING_MSG: {
            SetAllocStatisticsFrame(rawStack, batchNativeHookData);
            break;
        }
        case PR_SET_VMA_MSG: {
            break;
        }
        default: {
            PROFILER_LOG_ERROR(LOG_CORE, "statistics event type: error");
            break;
        }
    }
    return;
}

void StackPreprocess::SetHookData(RawStackPtr rawStack,
    std::vector<CallFrame>& callFrames, BatchNativeHookData& batchNativeHookData)
{
    if (hookConfig_.offline_symbolization() && flushBasicData_) {
        SetMapsInfo(rawStack->stackConext->pid);
        flushBasicData_ = false;
    }

    // statistical reporting must is compressed and accurate.
    if (hookConfig_.statistics_interval() > 0) {
        switch (rawStack->stackConext->type) {
            case FREE_MSG:
            case MUNMAP_MSG:
            case MEMORY_UNUSING_MSG: {
                SetFreeStatisticsData((uint64_t)rawStack->stackConext->addr);
                break;
            }
            case MALLOC_MSG:
                rawStack->stackConext->mallocSize = ComputeAlign(rawStack->stackConext->mallocSize);
            case MMAP_MSG:
            case MMAP_FILE_PAGE_MSG:
            case MEMORY_USING_MSG: {
                SetAllocStatisticsFrame(rawStack, callFrames, batchNativeHookData);
                break;
            }
            case PR_SET_VMA_MSG: {
                break;
            }
            default: {
                PROFILER_LOG_ERROR(LOG_CORE, "statistics event type:%d error", rawStack->stackConext->type);
                break;
            }
        }
        return;
    }

    uint32_t stackMapId = 0;
    if (hookConfig_.callframe_compress() &&
        !(rawStack->stackConext->type == MEMORY_TAG || rawStack->stackConext->type == PR_SET_VMA_MSG)) {
        stackMapId = GetCallStackId(rawStack, callFrames, batchNativeHookData);
    }

    if ((!hookConfig_.callframe_compress() || stackMapId == 0) && hookConfig_.string_compressed()) {
        size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
        for (; idx < callFrames.size(); ++idx) {
            ReportSymbolNameMap(callFrames[idx], batchNativeHookData);
            ReportFilePathMap(callFrames[idx], batchNativeHookData);
        }
    }

    NativeHookData* hookData = batchNativeHookData.add_events();
    hookData->set_tv_sec(rawStack->stackConext->ts.tv_sec);
    hookData->set_tv_nsec(rawStack->stackConext->ts.tv_nsec);

    if (rawStack->stackConext->type == MALLOC_MSG) {
        AllocEvent* allocEvent = hookData->mutable_alloc_event();
#ifdef USE_JEMALLOC
        allocEvent->set_size(static_cast<uint64_t>(ComputeAlign(rawStack->stackConext->mallocSize)));
#else
        allocEvent->set_size(static_cast<uint64_t>(rawStack->stackConext->mallocSize));
#endif
        allocEvent->set_thread_name_id(rawStack->stackConext->tid);
        SetEventFrame(rawStack, callFrames, batchNativeHookData, allocEvent, stackMapId);
    } else if (rawStack->stackConext->type == FREE_MSG) {
        FreeEvent* freeEvent = hookData->mutable_free_event();
        freeEvent->set_thread_name_id(rawStack->stackConext->tid);
        SetEventFrame(rawStack, callFrames, batchNativeHookData, freeEvent, stackMapId);
    } else if (rawStack->stackConext->type == MMAP_MSG) {
        MmapEvent* mmapEvent = hookData->mutable_mmap_event();
        mmapEvent->set_size(static_cast<uint64_t>(rawStack->stackConext->mallocSize));
        mmapEvent->set_thread_name_id(rawStack->stackConext->tid);
        SetEventFrame(rawStack, callFrames, batchNativeHookData, mmapEvent, stackMapId);
    } else if (rawStack->stackConext->type == MMAP_FILE_PAGE_MSG) {
        MmapEvent* mmapEvent = hookData->mutable_mmap_event();
        mmapEvent->set_size(static_cast<uint64_t>(rawStack->stackConext->mallocSize));
        mmapEvent->set_thread_name_id(rawStack->stackConext->tid);
        const std::string prefix = "FilePage:";
        std::string tagName;
        if (GetMemTag(rawStack->stackConext->tagId, tagName)) {
            mmapEvent->set_type(prefix + tagName);
        }
        SetEventFrame(rawStack, callFrames, batchNativeHookData, mmapEvent, stackMapId);
    } else if (rawStack->stackConext->type == MUNMAP_MSG) {
        MunmapEvent* munmapEvent = hookData->mutable_munmap_event();
        munmapEvent->set_size(static_cast<uint64_t>(rawStack->stackConext->mallocSize));
        munmapEvent->set_thread_name_id(rawStack->stackConext->tid);
        SetEventFrame(rawStack, callFrames, batchNativeHookData, munmapEvent, stackMapId);
    } else if (rawStack->stackConext->type == PR_SET_VMA_MSG) {
        MemTagEvent* tagEvent = hookData->mutable_tag_event();
        const std::string prefix = "Anonymous:";
        std::string tagName(reinterpret_cast<char*>(rawStack->data));
        tagEvent->set_tag(prefix + tagName);
        tagEvent->set_size(rawStack->stackConext->mallocSize);
        tagEvent->set_addr((uint64_t)rawStack->stackConext->addr);
        tagEvent->set_pid(pid_);
    } else if (rawStack->stackConext->type == MEMORY_USING_MSG) {
        MmapEvent* mmapEvent = hookData->mutable_mmap_event();
        mmapEvent->set_size(static_cast<uint64_t>(rawStack->stackConext->mallocSize));
        mmapEvent->set_thread_name_id(rawStack->stackConext->tid);
        std::string tagName;
        if (GetMemTag(rawStack->stackConext->tagId, tagName)) {
            mmapEvent->set_type(tagName);
        }
        SetEventFrame(rawStack, callFrames, batchNativeHookData, mmapEvent, stackMapId);
    } else if (rawStack->stackConext->type == MEMORY_UNUSING_MSG) {
        MunmapEvent* munmapEvent = hookData->mutable_munmap_event();
        munmapEvent->set_size(static_cast<uint64_t>(rawStack->stackConext->mallocSize));
        munmapEvent->set_thread_name_id(rawStack->stackConext->tid);
        SetEventFrame(rawStack, callFrames, batchNativeHookData, munmapEvent, stackMapId);
    }
}

inline bool StackPreprocess::SetFreeStatisticsData(uint64_t addr)
{
    // through the addr lookup record
    std::lock_guard<std::mutex> guard(mtex_);
    auto addrIter = allocAddrMap_.find(addr);
    if (addrIter != allocAddrMap_.end()) {
        auto& record = addrIter->second.second;
        ++record->releaseCount;
        record->releaseSize += addrIter->second.first;
        statisticsPeriodData_[record->callstackId] = record;
        allocAddrMap_.erase(addr);
        return true;
    }
    return false;
}

inline void StackPreprocess::SetAllocStatisticsData(const RawStackPtr& rawStack, size_t stackId, bool isExists)
{
    // if the record exists, it is updated.Otherwise Add
    if (isExists) {
        auto recordIter = recordStatisticsMap_.find(stackId);
        if (recordIter != recordStatisticsMap_.end()) {
            auto& record = recordIter->second;
            ++record.applyCount;
            record.applySize += rawStack->stackConext->mallocSize;
            std::lock_guard<std::mutex> guard(mtex_);
            allocAddrMap_[(uint64_t)rawStack->stackConext->addr] =
                std::pair(rawStack->stackConext->mallocSize, &recordIter->second);
            statisticsPeriodData_[stackId] = &recordIter->second;
        }
    } else {
        RecordStatistic record;
        record.pid = rawStack->stackConext->pid;
        record.callstackId = stackId;
        record.applyCount = 1;
        record.applySize = rawStack->stackConext->mallocSize;
        switch (rawStack->stackConext->type) {
            case MALLOC_MSG: {
                record.type = RecordStatisticsEvent::MALLOC;
                break;
            }
            case MMAP_MSG: {
                record.type = RecordStatisticsEvent::MMAP;
                break;
            }
            case MMAP_FILE_PAGE_MSG: {
                record.type = RecordStatisticsEvent::FILE_PAGE_MSG;
                break;
            }
            case MEMORY_USING_MSG: {
                record.type = RecordStatisticsEvent::MEMORY_USING_MSG;
                record.tagId = rawStack->stackConext->tagId;
                break;
            }
            default: {
                PROFILER_LOG_ERROR(LOG_CORE, "SetAllocStatisticsData event type error");
                break;
            }
        }

        auto [recordIter, stat] = recordStatisticsMap_.emplace(stackId, record);
        std::lock_guard<std::mutex> guard(mtex_);
        allocAddrMap_[(uint64_t)rawStack->stackConext->addr] =
            std::pair(rawStack->stackConext->mallocSize, &recordIter->second);
        statisticsPeriodData_[stackId] = &recordIter->second;
    }
}

void StackPreprocess::WriteFrames(RawStackPtr rawStack, const std::vector<CallFrame>& callFrames)
{
    CHECK_TRUE(fpHookData_ != nullptr, NO_RETVAL, "fpHookData_ is nullptr, please check file_name(%s)",
        hookConfig_.file_name().c_str());
    if (rawStack->stackConext->type == PR_SET_VMA_MSG) {
        const std::string prefix = "Anonymous:";
        std::string tagName;
        GetMemTag(rawStack->stackConext->tagId, tagName);
        fprintf(fpHookData_, "prctl;%u;%u;%" PRId64 ";%ld;0x%" PRIx64 ":tag:%s\n",
            rawStack->stackConext->pid, rawStack->stackConext->tid,
            (int64_t)rawStack->stackConext->ts.tv_sec, rawStack->stackConext->ts.tv_nsec,
            (uint64_t)rawStack->stackConext->addr, (prefix + tagName).c_str());
        return;
    }
    std::string tag = "";
    switch (rawStack->stackConext->type) {
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

    fprintf(fpHookData_, "%s;%u;%u;%" PRId64 ";%ld;0x%" PRIx64 ";%zu\n", tag.c_str(),
        rawStack->stackConext->pid, rawStack->stackConext->tid, (int64_t)rawStack->stackConext->ts.tv_sec,
        rawStack->stackConext->ts.tv_nsec, (uint64_t)rawStack->stackConext->addr, rawStack->stackConext->mallocSize);
    size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
    for (; idx < callFrames.size(); ++idx) {
        (void)fprintf(fpHookData_, "0x%" PRIx64 ";0x%" PRIx64 ";%s;%s;0x%" PRIx64 ";%" PRIu64 "\n",
            callFrames[idx].ip_, callFrames[idx].sp_, std::string(callFrames[idx].symbolName_).c_str(),
            std::string(callFrames[idx].filePath_).c_str(), callFrames[idx].offset_, callFrames[idx].symbolOffset_);
    }
}

inline void StackPreprocess::SetFrameInfo(Frame& frame, CallFrame& callFrame)
{
    frame.set_ip(callFrame.ip_);
    if (hookConfig_.offline_symbolization()) {
        return;
    }
    frame.set_sp(callFrame.sp_);
    frame.set_offset(callFrame.offset_);
    frame.set_symbol_offset(callFrame.symbolOffset_);

    if (callFrame.symbolNameId_ != 0 && callFrame.filePathId_ != 0) {
        frame.set_symbol_name_id(callFrame.symbolNameId_);
        frame.set_file_path_id(callFrame.filePathId_);
    } else {
        frame.set_symbol_name(std::string(callFrame.symbolName_));
        frame.set_file_path(std::string(callFrame.filePath_));
    }
}

inline void StackPreprocess::ReportSymbolNameMap(CallFrame& callFrame, BatchNativeHookData& batchNativeHookData)
{
    if (callFrame.needReport_ & SYMBOL_NAME_ID_REPORT) {
        auto hookData = batchNativeHookData.add_events();
        SymbolMap* symbolMap = hookData->mutable_symbol_name();
        symbolMap->set_id(callFrame.symbolNameId_);
        symbolMap->set_name(std::string(callFrame.symbolName_));
        symbolMap->set_pid(pid_);
    }
}

inline void StackPreprocess::ReportFilePathMap(CallFrame& callFrame, BatchNativeHookData& batchNativeHookData)
{
    if (callFrame.needReport_ & FILE_PATH_ID_REPORT) {
        auto hookData = batchNativeHookData.add_events();
        FilePathMap* filePathMap = hookData->mutable_file_path();
        filePathMap->set_id(callFrame.filePathId_);
        filePathMap->set_name(std::string(callFrame.filePath_));
        filePathMap->set_pid(pid_);
    }
}

inline void StackPreprocess::ReportFrameMap(CallFrame& callFrame, BatchNativeHookData& batchNativeHookData)
{
    if (callFrame.needReport_ & CALL_FRAME_REPORT) {
        ReportSymbolNameMap(callFrame, batchNativeHookData);
        ReportFilePathMap(callFrame, batchNativeHookData);
        auto hookData = batchNativeHookData.add_events();
        FrameMap* frameMap = hookData->mutable_frame_map();
        Frame* frame = frameMap->mutable_frame();
        SetFrameInfo(*frame, callFrame);
        frameMap->set_id(callFrame.callFrameId_);
        frameMap->set_pid(pid_);
    }
}

void StackPreprocess::SetMapsInfo(pid_t pid)
{
    std::lock_guard<std::mutex> guard(mtx_);
    for (auto& itemSoBegin : runtime_instance->GetOfflineMaps()) {
        auto& maps = runtime_instance->GetMapsCache();
        auto mapsIter = maps.find(itemSoBegin);
        if (mapsIter == maps.end()) {
            continue;
        }

        ElfSymbolTable symbolInfo;
        auto& curMemMaps = mapsIter->second;
        GetSymbols(curMemMaps->name_, symbolInfo);
        if (symbolInfo.symEntSize == 0) {
            continue;
        }
        BatchNativeHookData stackData;
        NativeHookData* hookData = stackData.add_events();
        FilePathMap* filepathMap = hookData->mutable_file_path();
        filepathMap->set_id(curMemMaps->filePathId_);
        filepathMap->set_name(curMemMaps->name_);
        filepathMap->set_pid(pid_);
        SetSymbolInfo(curMemMaps->filePathId_, symbolInfo, stackData);

        for (auto& map : curMemMaps->GetMaps()) {
            if (map->prots & PROT_EXEC) {
                NativeHookData* nativeHookData = stackData.add_events();
                MapsInfo* mapSerialize = nativeHookData->mutable_maps_info();
                mapSerialize->set_pid(pid);
                mapSerialize->set_start(map->begin);
                mapSerialize->set_end(map->end);
                mapSerialize->set_offset(map->offset);
                mapSerialize->set_file_path_id(curMemMaps->filePathId_);
            }
        }
        FlushData(stackData);
    }
    runtime_instance->ClearOfflineMaps();
}

void StackPreprocess::SetSymbolInfo(uint32_t filePathId, ElfSymbolTable& symbolInfo,
    BatchNativeHookData& batchNativeHookData)
{
    if (symbolInfo.symEntSize == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "SetSymbolInfo get symbolInfo failed");
        return;
    }
    NativeHookData* hookData = batchNativeHookData.add_events();
    SymbolTable* symTable = hookData->mutable_symbol_tab();
    symTable->set_file_path_id(filePathId);
    symTable->set_text_exec_vaddr(symbolInfo.textVaddr);
    symTable->set_text_exec_vaddr_file_offset(symbolInfo.textOffset);
    symTable->set_sym_entry_size(symbolInfo.symEntSize);
    symTable->set_sym_table(symbolInfo.symTable.data(), symbolInfo.symTable.size());
    symTable->set_str_table(symbolInfo.strTable.data(), symbolInfo.strTable.size());
    symTable->set_pid(pid_);
}

void StackPreprocess::FlushData(BatchNativeHookData& stackData)
{
    if (stackData.events().size() > 0) {
        size_t length = stackData.ByteSizeLong();
        stackData.SerializeToArray(buffer_.get(), length);
        if (length < MAX_BUFFER_SIZE) {
            if (isHookStandaloneSerialize_) {
                std::string str;
                ForStandard::BatchNativeHookData StandardStackData;
                StandardStackData.ParseFromArray(buffer_.get(), length);
                google::protobuf::TextFormat::PrintToString(StandardStackData, &str);
                size_t n = fwrite(str.data(), 1, str.size(), fpHookData_);
                fflush(fpHookData_);
                PROFILER_LOG_DEBUG(LOG_CORE, "Flush Data fwrite n = %zu str.size() = %zu", n, str.size());
            } else {
                Flush(buffer_.get(), length);
            }
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "the data is larger than MAX_BUFFER_SIZE, flush failed");
        }
    }
}

void StackPreprocess::Flush(const uint8_t* src, size_t size)
{
    if (src == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "Flush src is nullptr");
        return;
    }
    if (isSaService_) {
        ProfilerPluginData pluginData;
        pluginData.set_name("nativehook");
        pluginData.set_version("1.02");
        pluginData.set_status(0);
        pluginData.set_data(src, size);
        struct timespec ts;
        clock_gettime(pluginDataClockId_, &ts);
        pluginData.set_clock_id(static_cast<ProfilerPluginData_ClockId>(pluginDataClockId_));
        pluginData.set_tv_sec(ts.tv_sec);
        pluginData.set_tv_nsec(ts.tv_nsec);
        pluginData.SerializeToArray(buffer_.get(), pluginData.ByteSizeLong());
        size = pluginData.ByteSizeLong();
    }

    writer_->Write(buffer_.get(), size);
    writer_->Flush();
}

void StackPreprocess::GetSymbols(const std::string& filePath, ElfSymbolTable& symbols)
{
    std::shared_ptr<DfxElf> elfPtr = std::make_shared<DfxElf>(filePath);
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

bool StackPreprocess::FlushRecordStatistics()
{
    if (statisticsPeriodData_.empty()) {
        return false;
    }
    struct timespec ts;
    clock_gettime(hookDataClockId_, &ts);
    BatchNativeHookData statisticsData;
    for (auto [addr, statistics] : statisticsPeriodData_) {
        NativeHookData* hookData = statisticsData.add_events();
        hookData->set_tv_sec(ts.tv_sec);
        hookData->set_tv_nsec(ts.tv_nsec);
        RecordStatisticsEvent* recordEvent = hookData->mutable_statistics_event();
        recordEvent->set_pid(statistics->pid);
        recordEvent->set_callstack_id(statistics->callstackId);
        recordEvent->set_type(statistics->type);
        recordEvent->set_apply_count(statistics->applyCount);
        recordEvent->set_release_count(statistics->releaseCount);
        recordEvent->set_apply_size(statistics->applySize);
        recordEvent->set_release_size(statistics->releaseSize);

        std::string tagName;
        if (statistics->type == RecordStatisticsEvent::MEMORY_USING_MSG && GetMemTag(statistics->tagId, tagName)) {
            recordEvent->set_tag_name(tagName);
        }
    }
    {
        std::lock_guard<std::mutex> guard(mtx_);
        FlushData(statisticsData);
    }
    statisticsPeriodData_.clear();

    return true;
}

void StackPreprocess::SaveMemTag(uint32_t tagId, const std::string& tagName)
{
    std::string temp;
    bool res = memTagMap_.Find(tagId, temp);
    if (!res) {
        memTagMap_.EnsureInsert(tagId, tagName);
    }
}

bool StackPreprocess::GetMemTag(uint32_t tagId, std::string& tagName)
{
    return memTagMap_.Find(tagId, tagName);
}

unsigned StackPreprocess::LgFloor(unsigned long val)
{
    val |= (val >> RIGHT_MOVE_1);
    val |= (val >> RIGHT_MOVE_2);
    val |= (val >> RIGHT_MOVE_4);
    val |= (val >> RIGHT_MOVE_8);
    val |= (val >> RIGHT_MOVE_16);
    if (sizeof(val) > 4) {              // 4: sizeThreshold
        int constant = sizeof(val) * 4; // 4: sizeThreshold
        val |= (val >> constant);
    }
    val++;
    if (val == 0) {
        return 8 * sizeof(val) - 1; // 8: 8byte
    }
    return __builtin_ffsl(val) - 2; // 2: adjustment
}

uint64_t StackPreprocess::PowCeil(uint64_t val)
{
    size_t msbIndex = LgFloor(val - 1);
    return 1ULL << (msbIndex + 1);
}

size_t StackPreprocess::ComputeAlign(size_t size)
{
    if (size == 0) {
        return 0;
    }
    unsigned index = 0;
    if (size <= (size_t(1) << SC_LG_TINY_MAXCLASS)) {
        unsigned lgTmin = SC_LG_TINY_MAXCLASS - SC_NTINY + 1;
        unsigned lgCeil = LgFloor(PowCeil(size));
        index = (lgCeil < lgTmin) ? 0 : lgCeil - lgTmin;
    } else {
        unsigned floor = LgFloor((size << 1) - 1);
        unsigned shift = (floor < SC_LG_NGROUP + LG_QUANTUM) ? 0 : floor - (SC_LG_NGROUP + LG_QUANTUM);
        unsigned grp = shift << SC_LG_NGROUP;
        unsigned lgDelta = (floor < SC_LG_NGROUP + LG_QUANTUM + 1) ? LG_QUANTUM : floor - SC_LG_NGROUP - 1;
        size_t deltaInverseMask = size_t(-1) << lgDelta;
        unsigned mod = ((((size - 1) & deltaInverseMask) >> lgDelta)) & ((size_t(1) << SC_LG_NGROUP) - 1);
        index = SC_NTINY + grp + mod;
    }

    if (index < NTBINS) {
        return (size_t(1) << (LG_TINY_MAXCLASS - NTBINS + 1 + index));
    }
    size_t reducedIndex = index - NTBINS;
    size_t grpVal = reducedIndex >> LG_SIZE_CLASS_GROUP;
    size_t modVal = reducedIndex & ((size_t(1) << LG_SIZE_CLASS_GROUP) - 1);
    size_t grpSizeMask = ~((!!grpVal) - 1);
    size_t grpSize = ((size_t(1) << (LG_QUANTUM + (LG_SIZE_CLASS_GROUP - 1))) << grpVal) & grpSizeMask;
    size_t shiftVal = (grpVal == 0) ? 1 : grpVal;
    size_t lgDeltaVal = shiftVal + (LG_QUANTUM - 1);
    size_t modSize = (modVal + 1) << lgDeltaVal;
    size_t usize = grpSize + modSize;
    return usize;
}

void StackPreprocess::WriteHookConfig()
{
    std::shared_ptr<TraceFileWriter> tfPtr = std::static_pointer_cast<TraceFileWriter>(writer_);
    hookConfig_.SerializeToArray(buffer_.get(), hookConfig_.ByteSizeLong());
    tfPtr->WriteStandalonePluginData(
        "nativehook_config",
        std::string(reinterpret_cast<char*>(buffer_.get()), hookConfig_.ByteSizeLong()));
}