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
#include "elf_parser.h"
#include "utilities.h"
#include "native_hook_result_standard.pb.h"
#include "native_hook_config_standard.pb.h"
#include "google/protobuf/text_format.h"
static std::atomic<uint64_t> timeCost = 0;
static std::atomic<uint64_t> unwindTimes = 0;

constexpr static uint32_t MAX_BUFFER_SIZE = 10 * 1024 * 1024;
constexpr static uint32_t MAX_MATCH_CNT = 1000;
constexpr static uint32_t MAX_MATCH_INTERVAL = 2000;
constexpr static uint32_t LOG_PRINT_TIMES = 10000;
constexpr static uint32_t FUNCTION_MAP_LOG_PRINT = 100;
constexpr static uint32_t FILE_MAP_LOG_PRINT = 10;
constexpr static uint32_t MAX_BATCH_CNT = 5;
constexpr static uint32_t LONG_TIME_THRESHOLD = 1000000;

using namespace OHOS::Developtools::NativeDaemon;

StackPreprocess::StackPreprocess(const StackDataRepeaterPtr& dataRepeater,
    const NativeHookConfig& hookConfig,
    clockid_t pluginDataClockId,
    bool isHookStandalone) : dataRepeater_(dataRepeater), buffer_(new (std::nothrow) uint8_t[MAX_BUFFER_SIZE]),
                             hookConfig_(hookConfig), fpHookData_(nullptr, nullptr),
                             pluginDataClockId_(pluginDataClockId), isHookStandaloneSerialize_(isHookStandalone)
{
    runtime_instance = std::make_shared<VirtualRuntime>(hookConfig_);

    if (hookConfig_.malloc_free_matching_interval() > MAX_MATCH_INTERVAL) {
        HILOG_INFO(LOG_CORE, "Not support set %d", hookConfig_.malloc_free_matching_interval());
        hookConfig_.set_malloc_free_matching_interval(MAX_MATCH_INTERVAL);
    }

    if (hookConfig_.malloc_free_matching_cnt() > MAX_MATCH_CNT) {
        HILOG_INFO(LOG_CORE, "Not support set %d", hookConfig_.malloc_free_matching_cnt());
        hookConfig_.set_malloc_free_matching_cnt(MAX_MATCH_CNT);
    }
    HILOG_INFO(LOG_CORE, "malloc_free_matching_interval = %d malloc_free_matching_cnt = %d\n",
        hookConfig_.malloc_free_matching_interval(), hookConfig_.malloc_free_matching_cnt());

    if (hookConfig_.statistics_interval() > 0) {
        statisticsInterval_ = std::chrono::seconds(hookConfig_.statistics_interval());
    }
    HILOG_INFO(LOG_CORE, "statistics_interval = %d statisticsInterval_ = %lld \n",
        hookConfig_.statistics_interval(), statisticsInterval_.count());
    hookDataClockId_ = COMMON::GetClockId(hookConfig_.clock());
    HILOG_INFO(LOG_CORE, "StackPreprocess(): pluginDataClockId = %d hookDataClockId = %d \n",
        pluginDataClockId_, hookDataClockId_);
    // create file
    if (hookConfig_.save_file()) {
        if (hookConfig_.file_name() != "") {
            HILOG_DEBUG(LOG_CORE, "save file name = %s", hookConfig_.file_name().c_str());
            FILE *fp = fopen(hookConfig_.file_name().c_str(), "wb+");
            if (fp) {
                fpHookData_.reset();
                fpHookData_ = std::unique_ptr<FILE, decltype(&fclose)>(fp, fclose);
                if (isHookStandaloneSerialize_) {
                    std::string str;
                    size_t length = hookConfig_.ByteSizeLong();
                    hookConfig_.SerializeToArray(buffer_.get(), length);
                    ForStandard::NativeHookConfig StandardNativeHookConfig;
                    StandardNativeHookConfig.ParseFromArray(buffer_.get(), length);
                    google::protobuf::TextFormat::PrintToString(StandardNativeHookConfig, &str);
                    size_t n = fwrite(str.data(), 1, str.size(), fp);
                    HILOG_DEBUG(LOG_CORE, "Flush hookConfig_ fwrite n = %zu strbuf.size() = %zu", n, str.size());
                }
            } else {
                fpHookData_.reset();
            }
        } else {
            HILOG_WARN(LOG_CORE, "If you need to save the file, please set the file_name");
        }
    }
    HILOG_INFO(LOG_CORE, "isHookStandaloneSerialize_ = %d\n", isHookStandaloneSerialize_);
#if defined(__arm__)
    u64regs_.resize(PERF_REG_ARM_MAX);
#else
    u64regs_.resize(PERF_REG_ARM64_MAX);
#endif
    callFrames_.reserve(hookConfig_.max_stack_depth());
}

StackPreprocess::StackPreprocess(bool fpUnwind): fpHookData_(nullptr, nullptr) {}

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

void StackPreprocess::SetWriter(const std::shared_ptr<BufferWriter>& writer)
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
    HILOG_INFO(LOG_CORE, "start StopTakeResults");
    CHECK_NOTNULL(dataRepeater_, false, "data repeater null");
    CHECK_TRUE(thread_.get_id() != std::thread::id(), false, "thread invalid");

    isStopTakeData_ = true;
    dataRepeater_->PutRawStack(nullptr, false);
    HILOG_INFO(LOG_CORE, "StopTakeResults Wait thread join");

    if (thread_.joinable()) {
        thread_.join();
    }
    HILOG_INFO(LOG_CORE, "StopTakeResults Wait thread join success");
    return true;
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
    HILOG_INFO(LOG_CORE, "TakeResults thread %d, start!", gettid());
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
            if (rawData->stackConext->type == MMAP_FILE_TYPE) {
                BaseStackRawData* mmapRawData = rawData->stackConext;
                std::string filePath(reinterpret_cast<char *>(rawData->data));
                COMMON::AdaptSandboxPath(filePath, rawData->stackConext->pid);
                HILOG_DEBUG(LOG_CORE, "MMAP_FILE_TYPE curMmapAddr=%p, MAP_FIXED=%d, "
                    "PROT_EXEC=%d, offset=%" PRIu64 ", filePath=%s",
                    mmapRawData->addr, mmapRawData->mmapArgs.flags & MAP_FIXED,
                    mmapRawData->mmapArgs.flags & PROT_EXEC, mmapRawData->mmapArgs.offset, filePath.data());
                runtime_instance->HandleMapInfo(reinterpret_cast<uint64_t>(mmapRawData->addr), mmapRawData->mallocSize,
                    mmapRawData->mmapArgs.flags, mmapRawData->mmapArgs.offset, filePath);
                flushBasicData_ = true;
                continue;
            } else if (rawData->stackConext->type == MUNMAP_MSG) {
                runtime_instance->RemoveMaps(reinterpret_cast<uint64_t>(rawData->stackConext->addr));
            }
            if (rawData->stackConext->type == THREAD_NAME_MSG) {
                std::string threadName = reinterpret_cast<char*>(rawData->data);
                ReportThreadNameMap(rawData->stackConext->tid, threadName, stackData);
                continue;
            }

            if (!rawData->reportFlag) {
                ignoreCnts_++;
                if (ignoreCnts_ % LOG_PRINT_TIMES == 0) {
                    HILOG_INFO(LOG_CORE, "ignoreCnts_ = %d quene size = %zu\n", ignoreCnts_, dataRepeater_->Size());
                }
                continue;
            }
            eventCnts_++;
            if (eventCnts_ % LOG_PRINT_TIMES == 0) {
                HILOG_INFO(LOG_CORE, "eventCnts_ = %d quene size = %zu\n", eventCnts_, dataRepeater_->Size());
            }
            callFrames_.clear();
            if (hookConfig_.fp_unwind()) {
                uint64_t* fpIp = reinterpret_cast<uint64_t *>(rawData->data);
                for (uint8_t idx = 0; idx < rawData->fpDepth ; ++idx) {
                    if (fpIp[idx] == 0) {
                        break;
                    }
                    callFrames_.emplace_back(fpIp[idx]);
                }
            } else {
#if defined(__arm__)
                uint32_t *regAddrArm = reinterpret_cast<uint32_t *>(rawData->data);
                u64regs_.assign(regAddrArm, regAddrArm + PERF_REG_ARM_MAX);
#else
                if (memcpy_s(u64regs_.data(), sizeof(uint64_t) * PERF_REG_ARM64_MAX, rawData->data,
                    sizeof(uint64_t) * PERF_REG_ARM64_MAX) != EOK) {
                    HILOG_ERROR(LOG_CORE, "memcpy_s regs failed");
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
            bool ret = runtime_instance->UnwindStack(u64regs_, rawData->stackData, rawData->stackSize,
                rawData->stackConext->pid, rawData->stackConext->tid, callFrames_, stackDepth);
            if (!ret) {
                HILOG_ERROR(LOG_CORE, "unwind fatal error");
                continue;
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
                HILOG_ERROR(LOG_CORE, "bigTimeCost %" PRIu64 " event=%d, realFrameDepth=%zu, "
                    "callFramesDepth=%zu\n",
                    curTimeCost, rawData->stackConext->type, realFrameDepth, callFrames_.size());
            }
            timeCost += curTimeCost;
            unwindTimes++;
            if (unwindTimes % LOG_PRINT_TIMES == 0) {
                HILOG_ERROR(LOG_CORE, "unwindTimes %" PRIu64" cost time = %" PRIu64" mean cost = %" PRIu64"\n",
                    unwindTimes.load(), timeCost.load(), timeCost.load() / unwindTimes.load());
            }
#endif
        }
        if (hookConfig_.save_file() && hookConfig_.file_name() != "" && !isHookStandaloneSerialize_) {
            continue;
        }
        FlushData(stackData);

        // interval reporting statistics
        if (hookConfig_.statistics_interval() > 0) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastStatisticsTime_);
            if (elapsedTime >= statisticsInterval_) {
                lastStatisticsTime_ = currentTime;
                FlushRecordStatistics();
            }
        }
    }
    HILOG_INFO(LOG_CORE, "TakeResults thread %d, exit!", gettid());
}

inline void StackPreprocess::ReportThreadNameMap(uint32_t tid, const std::string& tname,
                                                 BatchNativeHookData& batchNativeHookData)
{
    auto it = threadNameMap_.find(tid);
    if (it == threadNameMap_.end() || it->second != tname) {
        threadNameMap_[tid] = tname;
        auto hookData = batchNativeHookData.add_events();
        auto* thread = hookData->mutable_thread_name_map();
        thread->set_id(tid);
        thread->set_name(tname);
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

/**
 * @return '0' is invalid stack id, '> 0' is valid stack id
 */
inline uint32_t StackPreprocess::SetCallStackMap(BatchNativeHookData& batchNativeHookData)
{
    auto hookData = batchNativeHookData.add_events();
    StackMap* stackmap = hookData->mutable_stack_map();
    uint32_t stackId = callStackMap_.size() + 1;
    stackmap->set_id(stackId);
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
    callStackMap_[callStack_] = stackId;
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
    auto itStack = callStackMap_.find(callStack_);
    if (itStack != callStackMap_.end()) {
        return itStack->second;
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
    } else if (hookConfig_.string_compressed()) {
        for (; idx < callFrames.size(); ++idx) {
            ReportSymbolNameMap(callFrames[idx], batchNativeHookData);
            ReportFilePathMap(callFrames[idx], batchNativeHookData);
            Frame* frame = event->add_frame_info();
            SetFrameInfo(*frame, callFrames[idx]);
        }
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
    callStack_.reserve(callFrames.size());
    if (!hookConfig_.offline_symbolization()) {
        FillCallStack(callFrames, batchNativeHookData, idx);
    } else {
        FillOfflineCallStack(callFrames, idx);
    }
    // by call stack id set alloc statistics data.
    auto itStack = callStackMap_.find(callStack_);
    if (itStack != callStackMap_.end()) {
        SetAllocStatisticsData(rawStack, itStack->second, true);
    } else {
        auto stackId = SetCallStackMap(batchNativeHookData);
        SetAllocStatisticsData(rawStack, stackId);
    }
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
            case MMAP_MSG:
            case MMAP_FILE_PAGE_MSG:
            case MEMORY_USING_MSG: {
                SetAllocStatisticsFrame(rawStack, callFrames, batchNativeHookData);
                break;
            }
            default: {
                HILOG_ERROR(LOG_CORE, "statistics event type:%d error", rawStack->stackConext->type);
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

    NativeHookData* hookData = batchNativeHookData.add_events();
    hookData->set_tv_sec(rawStack->stackConext->ts.tv_sec);
    hookData->set_tv_nsec(rawStack->stackConext->ts.tv_nsec);

    if (rawStack->stackConext->type == MALLOC_MSG) {
        AllocEvent* allocEvent = hookData->mutable_alloc_event();
        allocEvent->set_size(static_cast<uint64_t>(rawStack->stackConext->mallocSize));
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
                HILOG_ERROR(LOG_CORE, "SetAllocStatisticsData event type error");
                break;
            }
        }

        auto [recordIter, stat] = recordStatisticsMap_.emplace(stackId, record);
        allocAddrMap_[(uint64_t)rawStack->stackConext->addr] =
            std::pair(rawStack->stackConext->mallocSize, &recordIter->second);
        statisticsPeriodData_[stackId] = &recordIter->second;
    }
}

void StackPreprocess::WriteFrames(RawStackPtr rawStack, const std::vector<CallFrame>& callFrames)
{
    CHECK_TRUE(fpHookData_.get() != nullptr, NO_RETVAL, "fpHookData_ is nullptr, please check file_name(%s)",
        hookConfig_.file_name().c_str());
    if (rawStack->stackConext->type == PR_SET_VMA_MSG) {
        const std::string prefix = "Anonymous:";
        std::string tagName;
        GetMemTag(rawStack->stackConext->tagId, tagName);
        fprintf(fpHookData_.get(), "prctl;%u;%u;%" PRId64 ";%ld;0x%" PRIx64 ":tag:%s\n",
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

    fprintf(fpHookData_.get(), "%s;%u;%u;%" PRId64 ";%ld;0x%" PRIx64 ";%zu\n", tag.c_str(),
        rawStack->stackConext->pid, rawStack->stackConext->tid, (int64_t)rawStack->stackConext->ts.tv_sec,
        rawStack->stackConext->ts.tv_nsec, (uint64_t)rawStack->stackConext->addr, rawStack->stackConext->mallocSize);
    size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
    for (; idx < callFrames.size(); ++idx) {
        (void)fprintf(fpHookData_.get(), "0x%" PRIx64 ";0x%" PRIx64 ";%s;%s;0x%" PRIx64 ";%" PRIu64 "\n",
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
    }
}

inline void StackPreprocess::ReportFilePathMap(CallFrame& callFrame, BatchNativeHookData& batchNativeHookData)
{
    if (callFrame.needReport_ & FILE_PATH_ID_REPORT) {
        auto hookData = batchNativeHookData.add_events();
        FilePathMap* filePathMap = hookData->mutable_file_path();
        filePathMap->set_id(callFrame.filePathId_);
        filePathMap->set_name(std::string(callFrame.filePath_));
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
    }
}

void StackPreprocess::SetMapsInfo(pid_t pid)
{
    for (auto& itemSoBegin : runtime_instance->GetOfflineMaps()) {
        auto& maps = runtime_instance->GetMapsCache();
        auto mapsIter = maps.find(itemSoBegin);
        if (mapsIter == maps.end()) {
            continue;
        }

        ElfSymbolTable symbolInfo;
        auto& curMemMaps = mapsIter->second;
        GetSymbols(curMemMaps.name_, symbolInfo);
        if (symbolInfo.symEntSize == 0) {
            continue;
        }
        BatchNativeHookData stackData;
        NativeHookData* hookData = stackData.add_events();
        FilePathMap* filepathMap = hookData->mutable_file_path();
        filepathMap->set_id(curMemMaps.filePathId_);
        filepathMap->set_name(curMemMaps.name_);
        SetSymbolInfo(curMemMaps.filePathId_, symbolInfo, stackData);

        for (auto& map : curMemMaps.maps_) {
            if (map.type_ & PROT_EXEC) {
                NativeHookData* nativeHookData = stackData.add_events();
                MapsInfo* mapSerialize = nativeHookData->mutable_maps_info();
                mapSerialize->set_pid(pid);
                mapSerialize->set_start(map.begin_);
                mapSerialize->set_end(map.end_);
                mapSerialize->set_offset(map.pageoffset_);
                mapSerialize->set_file_path_id(curMemMaps.filePathId_);
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
        HILOG_ERROR(LOG_CORE, "SetSymbolInfo get symbolInfo failed");
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
                size_t n = fwrite(str.data(), 1, str.size(), fpHookData_.get());
                fflush(fpHookData_.get());
                HILOG_DEBUG(LOG_CORE, "Flush Data fwrite n = %zu str.size() = %zu", n, str.size());
            } else {
                Flush(buffer_.get(), length);
            }
        } else {
            HILOG_ERROR(LOG_CORE, "the data is larger than MAX_BUFFER_SIZE, flush failed");
        }
    }
}

void StackPreprocess::Flush(const uint8_t* src, size_t size)
{
    if (src == nullptr) {
        HILOG_ERROR(LOG_CORE, "Flush src is nullptr");
        return;
    }
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
    writer_->WriteMessage(pluginData, "nativehook");
    writer_->Flush();
}

void StackPreprocess::GetSymbols(const std::string& filePath, ElfSymbolTable& symbols)
{
    using OHOS::Developtools::NativeDaemon::ELF::ElfFile;
    std::unique_ptr<ElfFile> elfPtr = ElfFile::MakeUnique(filePath);
    if (elfPtr == nullptr) {
        HILOG_ERROR(LOG_CORE, "GetSymbols elfPtr is nullptr");
        return;
    }

    symbols.textVaddr = (std::numeric_limits<uint64_t>::max)();
    for (auto &item : elfPtr->phdrs_) {
        if ((item->type_ == PT_LOAD) && (item->flags_ & PF_X)) {
            // find the min addr
            if (symbols.textVaddr != (std::min)(symbols.textVaddr, item->vaddr_)) {
                symbols.textVaddr = (std::min)(symbols.textVaddr, item->vaddr_);
                symbols.textOffset = item->offset_;
            }
        }
    }
    if (symbols.textVaddr == (std::numeric_limits<uint64_t>::max)()) {
        HILOG_ERROR(LOG_CORE, "GetSymbols get textVaddr failed");
        return;
    }

    std::string symSecName;
    std::string strSecName;
    if (elfPtr->shdrs_.find(".symtab") != elfPtr->shdrs_.end()) {
        symSecName = ".symtab";
        strSecName = ".strtab";
    } else if (elfPtr->shdrs_.find(".dynsym") != elfPtr->shdrs_.end()) {
        symSecName = ".dynsym";
        strSecName = ".dynstr";
    } else {
        return;
    }
    const auto &sym = elfPtr->shdrs_[static_cast<const std::string>(symSecName)];
    const uint8_t* symData = elfPtr->GetSectionData(sym->secIndex_);
    const auto &str = elfPtr->shdrs_[static_cast<const std::string>(strSecName)];
    const uint8_t* strData = elfPtr->GetSectionData(str->secIndex_);

    if (sym->secSize_ == 0 || str->secSize_ == 0) {
        HILOG_ERROR(LOG_CORE, "GetSymbols get section size failed, \
            sym size: %" PRIu64 ", str size: %" PRIu64 "", sym->secSize_, str->secSize_);
        return;
    }
    symbols.symEntSize = sym->secEntrySize_;
    symbols.symTable.resize(sym->secSize_);
    std::copy(symData, symData + sym->secSize_, symbols.symTable.data());
    symbols.strTable.resize(str->secSize_);
    std::copy(strData, strData + str->secSize_, symbols.strTable.data());
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
    FlushData(statisticsData);
    statisticsPeriodData_.clear();

    return true;
}

void StackPreprocess::SaveMemTag(uint32_t tagId, const std::string& tagName)
{
    std::string temp;
    bool res = memTagMap_.Find(tagId, temp);
    if (!res) {
        memTagMap_[tagId] = tagName;
    }
}

bool StackPreprocess::GetMemTag(uint32_t tagId, std::string& tagName)
{
    return memTagMap_.Find(tagId, tagName);
}
