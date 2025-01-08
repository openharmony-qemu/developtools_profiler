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
constexpr static uint32_t MAX_MATCH_CNT = 1000;
constexpr static uint32_t MAX_MATCH_INTERVAL = 3600;
constexpr static uint32_t LOG_PRINT_TIMES = 10000;
constexpr static uint32_t WAIT_STOP_TIME = 5000;
constexpr static uint32_t WAIT_TIME_ONCE = 10;
constexpr static uint32_t MAX_BATCH_CNT = 40;
constexpr static uint32_t RIGHT_MOVE_1 = 1;
constexpr static uint32_t RIGHT_MOVE_2 = 2;
constexpr static uint32_t RIGHT_MOVE_4 = 4;
constexpr static uint32_t RIGHT_MOVE_8 = 8;
constexpr static uint32_t RIGHT_MOVE_16 = 16;
constexpr static uint64_t SIZE_MASK = 0xFFFFFF0000000000;
constexpr static uint64_t JS_OFFLINE_IP_MASK = 0xFFFFFE0000000000;
constexpr static uint64_t DWARF_ERROR_ID = 999999;
constexpr static uint64_t DWARF_NAPI_CALLBACK = 999999;
static std::string JS_CALL_STACK_DEPTH_SEP = ",";   // ',' is js call stack depth separator
static std::string JS_SYMBOL_FILEPATH_SEP = "|";    // '|' is js symbol and filepath separator
constexpr static int NAPI_CALL_STACK = 2; // just for napi call stack
constexpr static uint32_t FRAME_DEPTH = 2; // add two frames
#ifdef PERFORMANCE_DEBUG
constexpr static uint32_t LONG_TIME_THRESHOLD = 1000000;
static std::atomic<uint64_t> timeCost = 0;
static std::atomic<uint64_t> unwindTimes = 0;
#endif

using namespace OHOS::Developtools::NativeDaemon;
using namespace OHOS::HiviewDFX;
using namespace OHOS::Developtools::Profiler;

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
        recordStatisticsMap_.reserve(STATISTICS_MAP_SZIE);
        statisticsPeriodData_.reserve(STATISTICS_PERIOD_DATA_SIZE);
        allocAddrMap_.reserve(ALLOC_ADDRMAMP_SIZE);
    }
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
    PROFILER_LOG_INFO(LOG_CORE, "isHookStandaloneSerialize_ = %d", isHookStandaloneSerialize_);
#if defined(__arm__)
    u64regs_.resize(PERF_REG_ARM_MAX);
#else
    u64regs_.resize(PERF_REG_ARM64_MAX);
#endif
    callFrames_.reserve(hookConfig_.max_stack_depth() + hookConfig_.max_js_stack_depth());
    if (hookConfig_.fp_unwind() && hookConfig_.js_stack_report() > 0) {
        fpJsCallStacks_.reserve(hookConfig_.max_js_stack_depth());
    }
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

void StackPreprocess::FinishTraceFile()
{
    if (isSaService_) {
        std::shared_ptr<TraceFileWriter> tfPtr = std::static_pointer_cast<TraceFileWriter>(writer_);
        tfPtr->SetDurationTime();
        tfPtr->Finish();
    }
}

void StackPreprocess::SetWriter(const std::shared_ptr<Writer>& writer)
{
    writer_ = writer;
    if (!isSaService_) {
        stackData_ = BatchNativeHookData();
    }
}

void StackPreprocess::SetWriter(const WriterStructPtr& writer)
{
    resultWriter_ = writer;
    auto ctx = resultWriter_->startReport(resultWriter_);
    if (ctx == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: get RandomWriteCtx FAILED!", __func__);
        return;
    }
    stackData_ = ProtoEncoder::BatchNativeHookData(ctx);
}


bool StackPreprocess::StartTakeResults()
{
    CHECK_NOTNULL(dataRepeater_, false, "data repeater null");

    std::thread demuxer([this] { this->TakeResults(); });
    CHECK_TRUE(demuxer.get_id() != std::thread::id(), false, "demuxer thread invalid");

    thread_ = std::move(demuxer);
    isStopTakeData_ = false;
    return true;
}

bool StackPreprocess::StopTakeResults()
{
    PROFILER_LOG_INFO(LOG_CORE, "start StopTakeResults");
    int32_t timerFd = scheduleTaskManager_.ScheduleTask(
        std::bind(&StackPreprocess::ForceStop, this), WAIT_STOP_TIME, true, false);
    if (timerFd == -1) {
        PROFILER_LOG_ERROR(LOG_CORE, "StopTakeResults ScheduleTask failed!");
        return false;
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

inline void StackPreprocess::IntervalFlushRecordStatistics()
{
    // interval reporting statistics
    if (hookConfig_.statistics_interval() > 0) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastStatisticsTime_);
        if (elapsedTime >= statisticsInterval_) {
            lastStatisticsTime_ = currentTime;
            FlushRecordStatistics();
        }
    }
}

inline void StackPreprocess::IntervalFlushApplyAndReleaseMatchData()
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

bool StackPreprocess::HandleNoStackEvent(RawStackPtr& rawData)
{
    if (rawData->stackConext->type == MMAP_FILE_TYPE) {
        BaseStackRawData* mmapRawData = rawData->stackConext;
        std::string filePath(reinterpret_cast<char *>(rawData->data));
        COMMON::AdaptSandboxPath(filePath, rawData->stackConext->pid);
        PROFILER_LOG_DEBUG(LOG_CORE, "MMAP_FILE_TYPE curMmapAddr=%p, MAP_FIXED=%d, "
                    "PROT_EXEC=%d, offset=%" PRIu64 ", filePath=%s",
                    mmapRawData->addr, mmapRawData->mmapArgs.flags & MAP_FIXED,
                    mmapRawData->mmapArgs.flags & PROT_EXEC, mmapRawData->mmapArgs.offset, filePath.data());
        std::lock_guard<std::mutex> guard(mtx_);
        runtime_instance->HandleMapInfo({reinterpret_cast<uint64_t>(mmapRawData->addr),
            mmapRawData->mallocSize, mmapRawData->mmapArgs.flags, mmapRawData->mmapArgs.offset}, filePath,
            rawData->stackConext->pid, rawData->stackConext->tid);
        flushBasicData_ = true;
    } else if (rawData->stackConext->type == THREAD_NAME_MSG) {
        std::string threadName = reinterpret_cast<char*>(rawData->data);
        ReportThreadNameMap(rawData->stackConext->tid, threadName);
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

void StackPreprocess::TakeResultsFromShmem(const std::shared_ptr<EventNotifier>& eventNotifier,
                                           const std::shared_ptr<ShareMemoryBlock>& shareMemoryBlock)
{
    eventNotifier->Take();
    StackDataRepeater::RawStack rawStack;
    RawStackPtr rawData(&rawStack, [](StackDataRepeater::RawStack* del) {});
    while (!isStopTakeData_) {
        bool ret = shareMemoryBlock->TakeData(
            [&](const int8_t data[], uint32_t size) -> bool {
#ifdef PERFORMANCE_DEBUG
            struct timespec start = {};
            clock_gettime(CLOCK_REALTIME, &start);
#endif
            if (size == sizeof(uint64_t)) {
                uint64_t addr = *reinterpret_cast<uint64_t *>(const_cast<int8_t *>(data));
                SetFreeStatisticsData(addr);
#ifdef PERFORMANCE_DEBUG
                struct timespec end = {};
                clock_gettime(CLOCK_REALTIME, &end);
                uint64_t curTimeCost = (end.tv_sec - start.tv_sec) * MAX_MATCH_CNT * MAX_MATCH_CNT * MAX_MATCH_CNT +
                    (end.tv_nsec - start.tv_nsec);
                timeCost += curTimeCost;
                unwindTimes++;
                if (unwindTimes % LOG_PRINT_TIMES == 0) {
                    PROFILER_LOG_ERROR(LOG_CORE,
                                       "unwindTimes %" PRIu64" cost time = %" PRIu64" mean cost = %" PRIu64"\n",
                                       unwindTimes.load(), timeCost.load(), timeCost.load() / unwindTimes.load());
                }
#endif
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
            } else if (HandleNoStackEvent(rawData)) {
                return true;
            } else if (rawData->stackConext->type == MUNMAP_MSG) {
                std::lock_guard<std::mutex> guard(mtx_);
                runtime_instance->RemoveMaps(reinterpret_cast<uint64_t>(rawData->stackConext->addr));
            } else if (rawData->stackConext->type == NMD_MSG) {
                const char* nmdResult = reinterpret_cast<const char*>(rawData->data);
                lseek(nmdFd_, 0, SEEK_END);
                (void)write(nmdFd_, nmdResult, strlen(nmdResult));
                return true;
            }  else if (rawData->stackConext->type == END_MSG) {
                isStopTakeData_ = true;
                return true;
            }
            {
                std::lock_guard<std::mutex> guard(mtx_);
                runtime_instance->UpdateThread(rawData->stackConext->pid, rawData->stackConext->tid);
            }
            ReportOfflineSymbolizationData();
            std::visit([&](auto& stackData) {
                SetHookData(rawData, stackData);
                FlushCheck(stackData);
                }, stackData_);
            IntervalFlushRecordStatistics();
#ifdef PERFORMANCE_DEBUG
            struct timespec end = {};
            clock_gettime(CLOCK_REALTIME, &end);
            uint64_t curTimeCost = (end.tv_sec - start.tv_sec) * MAX_MATCH_CNT * MAX_MATCH_CNT * MAX_MATCH_CNT +
                (end.tv_nsec - start.tv_nsec);
            if (curTimeCost >= LONG_TIME_THRESHOLD) {
                PROFILER_LOG_ERROR(LOG_CORE, "bigTimeCost %" PRIu64 " event=%d fpDepth=%u",
                            curTimeCost, rawData->stackConext->type, rawData->fpDepth);
            }
            timeCost += curTimeCost;
            unwindTimes++;
            if (unwindTimes % LOG_PRINT_TIMES == 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "unwindTimes %" PRIu64" cost time = %" PRIu64" mean cost = %" PRIu64"\n",
                    unwindTimes.load(), timeCost.load(), timeCost.load() / unwindTimes.load());
            }
#endif
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
        RawStackPtr batchRawStack[MAX_BATCH_CNT] = {nullptr};
        if (isStopTakeData_) {
            break;
        }
        uint32_t during = 0;
        if (hookConfig_.statistics_interval() > 0) {
            auto currentTime = std::chrono::steady_clock::now();
            auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastStatisticsTime_);
            int tempDuring =
                std::chrono::duration_cast<std::chrono::milliseconds>(statisticsInterval_).count() - timeDiff.count();
            during = tempDuring > 0 ? static_cast<uint32_t>(tempDuring) : 0;
        }
        bool isTimeOut = false;
        auto result = dataRepeater_->TakeRawData(during, hookDataClockId_, MAX_BATCH_CNT, batchRawStack,
                                                 hookConfig_.statistics_interval(), isTimeOut);
        if (hookConfig_.statistics_interval() > 0 && isTimeOut && result == nullptr) {  // statistics mode
            IntervalFlushRecordStatistics();
            continue;
        }
        if (!result) {
            break;
        }
        for (unsigned int i = 0; i < MAX_BATCH_CNT; i++) {
            auto rawData = batchRawStack[i];
            if (!rawData || isStopTakeData_) {
                break;
            }
            if (rawData->baseStackData == nullptr) {
                if (rawData->freeData) {
                    SetFreeStatisticsData(rawData->freeData);
                }
                continue;
            }
            if (rawData->stackConext->type == NMD_MSG) {
                continue;
            } else if (rawData->stackConext->type == END_MSG) {
                isStopTakeData_ = true;
                break;
            }
#ifdef PERFORMANCE_DEBUG
            struct timespec start = {};
            clock_gettime(CLOCK_REALTIME, &start);
#endif
            if (HandleNoStackEvent(rawData)) {
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
                FillFpNativeIp(rawData);
                if (rawData->stackConext->jsChainId > 0 && rawData->jsStackData && hookConfig_.js_stack_report() > 0) {
                    FillFpJsData(rawData);
                }
            } else if (rawData->stackConext->type != PR_SET_VMA_MSG) {
                if (rawData->stackSize == 0) {
                    FillDwarfErrorStack();
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
            }
#ifdef PERFORMANCE_DEBUG
            size_t realFrameDepth = callFrames_.size();
#endif
            size_t stackDepth = ((size_t)hookConfig_.max_stack_depth() > MAX_CALL_FRAME_UNWIND_SIZE)
                        ? MAX_CALL_FRAME_UNWIND_SIZE
                        : hookConfig_.max_stack_depth() + FILTER_STACK_DEPTH;
            if (rawData->reduceStackFlag) {
                stackDepth = minStackDepth;
            }
            if ((hookConfig_.fp_unwind()) || rawData->stackSize > 0) {
                std::lock_guard<std::mutex> guard(mtx_);
                if (rawData->stackConext->type != PR_SET_VMA_MSG) {
                    bool ret = runtime_instance->UnwindStack(u64regs_, rawData->stackData, rawData->stackSize,
                    rawData->stackConext->pid, rawData->stackConext->tid, callFrames_, stackDepth);
                    if (!ret) {
                        PROFILER_LOG_ERROR(LOG_CORE, "unwind fatal error");
                        continue;
                    }
                }
            }
            if ((hookConfig_.fp_unwind()) || rawData->stackSize > 0) {
                ReportOfflineSymbolizationData();
            }
            std::visit([&](auto& stackData) {
                if (hookConfig_.save_file() && hookConfig_.file_name() != "" && isHookStandaloneSerialize_) {
                    SetHookData(rawData, callFrames_, stackData);
                } else if (hookConfig_.save_file() && hookConfig_.file_name() != "") {
                    WriteFrames(rawData, callFrames_);
                } else if (!hookConfig_.save_file()) {
                    if (hookConfig_.malloc_free_matching_interval() > 0) {
                        SetApplyAndReleaseMatchFrame(rawData, callFrames_, stackData);
                    } else {
                        SetHookData(rawData, callFrames_, stackData);
                    }
                }
                }, stackData_);

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
        } // for
        for (unsigned int i = 0; i < MAX_BATCH_CNT; i++) {
            if (!batchRawStack[i]) {
                break;
            }
            dataRepeater_->ReturnRawStack(std::move(batchRawStack[i]));
        }
        if (hookConfig_.save_file() && hookConfig_.file_name() != "" && !isHookStandaloneSerialize_) {
            continue;
        }
        if (hookConfig_.statistics_interval() == 0) {
            std::visit([&](auto& stackData) {
                FlushCheck(stackData);
                }, stackData_);
        }
        IntervalFlushRecordStatistics();
        IntervalFlushApplyAndReleaseMatchData();
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
inline void StackPreprocess::FillOfflineCallStack(std::vector<CallFrame>& callFrames, size_t idx, T& stackData)
{
    for (; idx < callFrames.size(); ++idx) {
        if (callFrames[idx].isJsFrame_) {
            ReportFrameMap(callFrames[idx], stackData);
            callStack_.push_back(callFrames[idx].callFrameId_ | JS_OFFLINE_IP_MASK);
            continue;
        }
        callStack_.push_back(callFrames[idx].ip_);
    }
}

template <typename T>
inline void StackPreprocess::FillCallStack(std::vector<CallFrame>& callFrames, size_t idx, T& stackData)
{
    for (; idx < callFrames.size(); ++idx) {
        ReportFrameMap(callFrames[idx], stackData);
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
template <typename T>
inline uint32_t StackPreprocess::SetCallStackMap(T& stackData)
{
    uint32_t stackId = 0;
    auto hookData = stackData.add_events();
    auto stackmap = hookData->mutable_stack_map();
    if (hookConfig_.response_library_mode()) {
        stackId = responseLibraryMap_.size() + 1;
    } else {
        stackId = callStackMap_.size() + 1;
    }
    stackmap->set_id(stackId);
    // offline symbolization use ip, other use frame_map_id
    if (hookConfig_.offline_symbolization()) {
        if constexpr (std::is_same<T, ProtoEncoder::BatchNativeHookData>::value) {
            stackmap->add_ip(callStack_);
        } else {
            for (size_t i = 0; i < callStack_.size(); i++) {
                stackmap->add_ip(callStack_[i]);
            }
        }
    } else {
        if constexpr (std::is_same<T, ProtoEncoder::BatchNativeHookData>::value) {
            stackmap->add_frame_map_id(callStack_);
        } else {
            for (size_t i = 0; i < callStack_.size(); i++) {
                stackmap->add_frame_map_id(callStack_[i]);
            }
        }
    }
    stackmap->set_pid(pid_);
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
template <typename T>
inline uint32_t StackPreprocess::GetCallStackId(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames,
    T& stackData)
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
    bool isNapi = false;
    if (rawStack->stackConext->type == MEMORY_USING_MSG && hookConfig_.js_stack_report() == NAPI_CALL_STACK) {
        std::string tagName;
        GetMemTag(rawStack->stackConext->tagId, tagName);
        if (tagName.find("napi") != std::string::npos) {
            callStack_.reserve(callFrames.size() + 1);  // 1 : insert a frame
            callStack_.push_back((DWARF_NAPI_CALLBACK + napiIndex_) | JS_OFFLINE_IP_MASK);
            isNapi = true;
        }
    } else {
        callStack_.reserve(callFrames.size());
    }
    if (!hookConfig_.offline_symbolization()) {
        FillCallStack(callFrames, idx, stackData);
    } else {
        if ((!hookConfig_.fp_unwind()) && rawStack->stackSize == 0) {
            idx = 0;
        }
        FillOfflineCallStack(callFrames, idx, stackData);
    }
    if (isNapi) {
        // insert a frame
        std::string tagName;
        GetMemTag(rawStack->stackConext->tagId, tagName);
        FillNapiStack(tagName, callFrames, napiIndex_);
        ReportFrameMap(callFrames.back(), stackData);
        ++napiIndex_;
    }
    // return call stack id
    std::lock_guard<std::mutex> guard(mtx_);
    uint32_t stackId = FindCallStackId(callStack_);
    if (stackId > 0) {
        return stackId;
    } else {
        return SetCallStackMap(stackData);
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

    if constexpr (!std::is_same<T, ::FreeEvent>::value && !std::is_same<T, ProtoEncoder::FreeEvent>::value) {
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

template <typename T>
void StackPreprocess::SetEventFrame(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames,
    T* event, uint32_t stackMapId, const std::string& type)
{
    // ignore the first two frame if dwarf unwind
    size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
    event->set_pid(rawStack->stackConext->pid);
    event->set_tid(rawStack->stackConext->tid);
    event->set_addr((uint64_t)rawStack->stackConext->addr);

    if constexpr (std::is_same<T, ::MmapEvent>::value || std::is_same<T, ProtoEncoder::MmapEvent>::value) {
        event->set_type(type);
    }

    if constexpr (!std::is_same<T, ::FreeEvent>::value && !std::is_same<T, ProtoEncoder::FreeEvent>::value) {
        auto size = static_cast<uint64_t>(rawStack->stackConext->mallocSize);
#ifdef USE_JEMALLOC
    if constexpr (std::is_same<T, ::AllocEvent>::value || std::is_same<T, ProtoEncoder::AllocEvent>::value) {
        size = static_cast<uint64_t>(ComputeAlign(size));
    }
#endif
        event->set_size(size);
    }

    if (hookConfig_.callframe_compress() && stackMapId != 0) {
        event->set_thread_name_id(rawStack->stackConext->tid);
        event->set_stack_id(stackMapId);
    } else {
        for (; idx < callFrames.size(); ++idx) {
            auto frame = event->add_frame_info();
            SetFrameInfo(*frame, callFrames[idx]);
        }
        event->set_thread_name_id(rawStack->stackConext->tid);
    }
}

void StackPreprocess::FillNapiStack(std::string& tagName, std::vector<CallFrame>& callFrames, uint64_t napiIndex)
{
    #if defined(__aarch64__)
    uintptr_t pacMask = 0xFFFFFF8000000000;
#else
    uintptr_t pacMask = 0;
#endif
    CallFrame& jsCallFrame = callFrames_.emplace_back(0 & (~pacMask));
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
void StackPreprocess::SetAllocStatisticsFrame(const RawStackPtr& rawStack, std::vector<CallFrame>& callFrames,
    T& stackData)
{
    // ignore the first two frame if dwarf unwind
    size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
    callStack_.clear();
    bool isNapi = false;
    if (hookConfig_.js_stack_report() == NAPI_CALL_STACK) {
        std::string tagName;
        GetMemTag(rawStack->stackConext->tagId, tagName);
        if (tagName.find("napi") != std::string::npos) {
            callStack_.reserve(callFrames.size() + FRAME_DEPTH);  // insert a frame
            callStack_.push_back((DWARF_NAPI_CALLBACK + napiIndex_) | JS_OFFLINE_IP_MASK);
            isNapi = true;
        }
    } else {
        callStack_.reserve(callFrames.size() + 1);
    }
    callStack_.push_back(rawStack->stackConext->mallocSize | SIZE_MASK);
    if (!hookConfig_.offline_symbolization()) {
        FillCallStack(callFrames, idx, stackData);
    } else {
        FillOfflineCallStack(callFrames, idx, stackData);
    }
    // insert a frame
    if (isNapi) {
        std::string tagName;
        GetMemTag(rawStack->stackConext->tagId, tagName);
        FillNapiStack(tagName, callFrames, napiIndex_);
        ReportFrameMap(callFrames.back(), stackData);
        ++napiIndex_;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    // by call stack id set alloc statistics data.
    uint32_t stackId = FindCallStackId(callStack_);
    if (stackId > 0) {
        SetAllocStatisticsData(rawStack, stackId, true);
    } else {
        stackId = SetCallStackMap(stackData);
        statisticsModelFlushCallstack_ = true;
        SetAllocStatisticsData(rawStack, stackId);
    }
}

template <typename T>
void StackPreprocess::SetAllocStatisticsFrame(const RawStackPtr& rawStack, T& stackData)
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
        stackId = SetCallStackMap(stackData);
        statisticsModelFlushCallstack_ = true;
        SetAllocStatisticsData(rawStack, stackId);
    }
}

template <typename T>
void StackPreprocess::SetHookData(RawStackPtr rawStack, T& stackData)
{
    if (hookConfig_.statistics_interval() > 0) {
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
                SetAllocStatisticsFrame(rawStack, stackData);
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
}

void StackPreprocess::ReportOfflineSymbolizationData()
{
    if (hookConfig_.offline_symbolization() && flushBasicData_) {
        SetMapsInfo();
        flushBasicData_ = false;
    }
}

template <typename T>
void StackPreprocess::SetApplyAndReleaseMatchFrame(RawStackPtr rawStack, std::vector<CallFrame>& callFrames,
                                                   T& stackData)
{
    uint32_t stackMapId = 0;
    if (rawStack->stackConext->type != PR_SET_VMA_MSG) {
        stackMapId = GetCallStackId(rawStack, callFrames, stackData);
    } else {
        rawStack->stackConext->tagId = prctlPeriodTags_.size();
        prctlPeriodTags_.emplace_back(reinterpret_cast<char*>(rawStack->data));
        applyAndReleaseMatchPeriodListData_.emplace_back(rawStack->stackConext);
    }
    if (rawStack->stackConext->type == MALLOC_MSG) {
        rawStack->stackConext->mallocSize = ComputeAlign(rawStack->stackConext->mallocSize);
    } else if (rawStack->stackConext->type == PR_SET_VMA_MSG) {
        return;
    }
    uint64_t addr = reinterpret_cast<uint64_t>(rawStack->stackConext->addr);
    auto iter = applyAndReleaseMatchIntervallMap_.find(addr);
    if (iter != applyAndReleaseMatchIntervallMap_.end()) {
        applyAndReleaseMatchPeriodListData_.erase(iter->second);
        applyAndReleaseMatchIntervallMap_.erase(addr);
    } else {
        applyAndReleaseMatchPeriodListData_.emplace_back(rawStack->stackConext, stackMapId);
        applyAndReleaseMatchIntervallMap_.emplace(addr, std::prev(applyAndReleaseMatchPeriodListData_.end()));
    }
}

template <typename T>
void StackPreprocess::SetHookData(RawStackPtr rawStack, std::vector<CallFrame>& callFrames, T& stackData)
{
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
                SetAllocStatisticsFrame(rawStack, callFrames, stackData);
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
        stackMapId = GetCallStackId(rawStack, callFrames, stackData);
    }

    if ((!hookConfig_.callframe_compress() || stackMapId == 0) && hookConfig_.string_compressed()) {
        size_t idx = hookConfig_.fp_unwind() ? 0 : FILTER_STACK_DEPTH;
        for (; idx < callFrames.size(); ++idx) {
            ReportSymbolNameMap(callFrames[idx], stackData);
            ReportFilePathMap(callFrames[idx], stackData);
        }
    }

    auto hookData = stackData.add_events();
    hookData->set_tv_sec(rawStack->stackConext->ts.tv_sec);
    hookData->set_tv_nsec(rawStack->stackConext->ts.tv_nsec);

    if (rawStack->stackConext->type == MALLOC_MSG) {
        auto allocEvent = hookData->mutable_alloc_event();
        SetEventFrame(rawStack, callFrames, allocEvent, stackMapId);
    } else if (rawStack->stackConext->type == FREE_MSG) {
        auto freeEvent = hookData->mutable_free_event();
        SetEventFrame(rawStack, callFrames, freeEvent, stackMapId);
    } else if (rawStack->stackConext->type == MMAP_MSG) {
        auto mmapEvent = hookData->mutable_mmap_event();
        SetEventFrame(rawStack, callFrames, mmapEvent, stackMapId);
    } else if (rawStack->stackConext->type == MMAP_FILE_PAGE_MSG) {
        auto mmapEvent = hookData->mutable_mmap_event();
        const std::string prefix = "FilePage:";
        std::string tagName;
        if (GetMemTag(rawStack->stackConext->tagId, tagName)) {
            tagName = prefix + tagName;
        }
        SetEventFrame(rawStack, callFrames, mmapEvent, stackMapId, tagName);
    } else if (rawStack->stackConext->type == MUNMAP_MSG) {
        auto munmapEvent = hookData->mutable_munmap_event();
        SetEventFrame(rawStack, callFrames, munmapEvent, stackMapId);
    } else if (rawStack->stackConext->type == PR_SET_VMA_MSG) {
        auto tagEvent = hookData->mutable_tag_event();
        const std::string prefix = "Anonymous:";
        std::string tagName(reinterpret_cast<char*>(rawStack->data));
        tagEvent->set_addr((uint64_t)rawStack->stackConext->addr);
        tagEvent->set_size(rawStack->stackConext->mallocSize);
        tagEvent->set_tag(prefix + tagName);
        tagEvent->set_pid(pid_);
    } else if (rawStack->stackConext->type == MEMORY_USING_MSG) {
        auto mmapEvent = hookData->mutable_mmap_event();
        std::string tagName;
        GetMemTag(rawStack->stackConext->tagId, tagName);
        SetEventFrame(rawStack, callFrames, mmapEvent, stackMapId, tagName);
    } else if (rawStack->stackConext->type == MEMORY_UNUSING_MSG) {
        auto munmapEvent = hookData->mutable_munmap_event();
        SetEventFrame(rawStack, callFrames, munmapEvent, stackMapId);
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
                PROFILER_LOG_ERROR(LOG_CORE, "SetAllocStatisticsData event type error");
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
    CHECK_TRUE(fpHookData_ != nullptr, NO_RETVAL, "fpHookData_ is nullptr, please check file_name(%s)",
        hookConfig_.file_name().c_str());
    if (rawStack->stackConext->type == PR_SET_VMA_MSG) {
        const std::string prefix = "Anonymous:";
        std::string tagName(reinterpret_cast<char*>(rawStack->data));
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

        ElfSymbolTable symbolInfo;
        auto& curMemMaps = mapsIter->second;
        GetSymbols(curMemMaps->name_, symbolInfo);
        if (symbolInfo.symEntSize == 0) {
            continue;
        }
        std::visit([&](auto& stackData) {
            auto hookData = stackData.add_events();
            auto filepathMap = hookData->mutable_file_path();
            filepathMap->set_id(curMemMaps->filePathId_);
            filepathMap->set_name(curMemMaps->name_);
            filepathMap->set_pid(pid_);
            SetSymbolInfo(curMemMaps->filePathId_, symbolInfo, stackData);

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
            }, stackData_);
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
        stackData.SerializeToArray(buffer_.get(), length);
        if (length < bufferSize_) {
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
    if (!isSaService_) {
        resultWriter_->finishReport(resultWriter_, messageLen);
        resultWriter_->flush(resultWriter_);
        ctx = resultWriter_->startReport(resultWriter_);
    } else {
        profilerPluginData_.finishAdd_data(messageLen);
        FinishReport();
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
    writer_->Write(src, size);
    writer_->Flush();

    std::get<::BatchNativeHookData>(stackData_).clear_events();
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
    std::visit([&](auto& stackData) {
        FlushData(stackData);
        }, stackData_);
    std::visit([&](auto& stackData) {
        struct timespec ts;
        clock_gettime(hookDataClockId_, &ts);
        for (auto [addr, statistics] : statisticsPeriodData_) {
            auto hookData = stackData.add_events();
            hookData->set_tv_sec(ts.tv_sec);
            hookData->set_tv_nsec(ts.tv_nsec);
            auto recordEvent = hookData->mutable_statistics_event();
            recordEvent->set_pid(statistics->pid);
            recordEvent->set_callstack_id(statistics->callstackId);
            recordEvent->set_type(statistics->type);
            recordEvent->set_apply_count(statistics->applyCount);
            recordEvent->set_release_count(statistics->releaseCount);
            recordEvent->set_apply_size(statistics->applySize);
            recordEvent->set_release_size(statistics->releaseSize);
        }
        FlushData(stackData);
        }, stackData_);
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

void StackPreprocess::SaveJsRawStack(uint64_t jsChainId, const char* jsRawStack)
{
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
    auto iter = jsStackMap_.find(jsChainId);
    if (iter != jsStackMap_.end()) {
        return iter->second;
    }
    return nullptr;
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
    const size_t configSize = hookConfig_.ByteSizeLong();
    auto buffer = std::make_unique<uint8_t[]>(configSize);
    hookConfig_.SerializeToArray(buffer.get(), configSize);

    writer_->ResetPos();
    profilerPluginData_.Reset(writer_->GetCtx());
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
    stackData_ = ProtoEncoder::BatchNativeHookData(ctx);
}

RandomWriteCtx* StackPreprocess::StartReport()
{
    writer_->ResetPos();
    profilerPluginData_.Reset(writer_->GetCtx());
    profilerPluginData_.set_name("nativehook");
    profilerPluginData_.set_version("1.02");
    profilerPluginData_.set_status(0);
    return profilerPluginData_.startAdd_data();
}

void StackPreprocess::FinishReport()
{
    struct timespec ts;
    clock_gettime(pluginDataClockId_, &ts);
    profilerPluginData_.set_clock_id(static_cast<ProfilerPluginData_ClockId>(pluginDataClockId_));
    profilerPluginData_.set_tv_sec(ts.tv_sec);
    profilerPluginData_.set_tv_nsec(ts.tv_nsec);

    int32_t len = profilerPluginData_.Finish();
    if (writer_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s: the writer is nullptr!", __func__);
        return;
    }
    writer_->FinishReport(len);
}

void StackPreprocess::FillFpNativeIp(RawStackPtr& rawData)
{
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
}

void StackPreprocess::FillFpJsData(RawStackPtr& rawData)
{
    if (hookConfig_.statistics_interval() > 0) {
        switch (rawData->stackConext->type) {
            case FREE_MSG:
            case MUNMAP_MSG:
            case MEMORY_UNUSING_MSG:
                 return;
            default:
                break;
        }
    }
    fpJsCallStacks_.clear();
    /**
      *     jsStackData:
      *              ts_malloc1|entry/src/main/ets/pages/Index.ets:5:5,ts_malloc2|entry/src/main/ets/pages/Index.ets:8:5
      *                        |                                      |
      *                        JS_SYMBOL_FILEPATH_SEP                 JS_CALL_STACK_DEPTH_SEP
      *     jsCallStack:
      *                  ts_malloc1|entry/src/main/ets/pages/Index.ets:5:5
      *                            / \
      *                           |   |
      *     jsSymbolFilePathSepPos    |
      *                               jsFilePathPos = jsSymbolFilePathSepPos + 1
      */
    AdvancedSplitString(rawData->jsStackData, JS_CALL_STACK_DEPTH_SEP, fpJsCallStacks_);
    for (std::string& jsCallStack: fpJsCallStacks_) {
        std::string::size_type jsSymbolFilePathSepPos = jsCallStack.find_first_of(JS_SYMBOL_FILEPATH_SEP);
        if (jsSymbolFilePathSepPos == std::string::npos) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: jsCallStack find FAILED!", __func__);
            continue;
        }
        std::string::size_type jsFilePathPos = jsSymbolFilePathSepPos + 1;
        jsCallStack[jsSymbolFilePathSepPos] = '\0'; // "ts_malloc1'\0'entry/src/main/ets/pages/Index.ets:5:5"
        CallFrame& jsCallFrame = callFrames_.emplace_back(0, 0, true);
        jsCallFrame.symbolName_ = StringViewMemoryHold::GetInstance().HoldStringView(jsCallStack.c_str());
        jsCallFrame.filePath_ = StringViewMemoryHold::GetInstance().HoldStringView(jsCallStack.c_str() + jsFilePathPos);
        if (hookConfig_.offline_symbolization()) {
            DfxSymbol symbol;
            if (!runtime_instance->ArktsGetSymbolCache(jsCallFrame, symbol)) {
                symbol.filePathId_ = runtime_instance->FillArkTsFilePath(jsCallFrame.filePath_);
                symbol.symbolName_ = jsCallFrame.symbolName_;
                symbol.module_ = jsCallFrame.filePath_;
                symbol.symbolId_ = runtime_instance->GetJsSymbolCacheSize();
                runtime_instance->FillSymbolNameId(jsCallFrame, symbol);
                runtime_instance->FillFileSet(jsCallFrame, symbol);
                jsCallFrame.needReport_ |= CALL_FRAME_REPORT;
                runtime_instance->FillJsSymbolCache(jsCallFrame, symbol);
            }
            jsCallFrame.callFrameId_ = symbol.symbolId_;
            jsCallFrame.symbolNameId_ = symbol.symbolNameId_;
            jsCallFrame.filePathId_ = symbol.filePathId_;
            jsCallFrame.filePath_ = symbol.module_;
            jsCallFrame.symbolName_ = symbol.symbolName_;
        }
    }
}

void StackPreprocess::FillDwarfErrorStack()
{
#if defined(__aarch64__)
    uintptr_t pacMask = 0xFFFFFF8000000000;
#else
    uintptr_t pacMask = 0;
#endif
    CallFrame& jsCallFrame = callFrames_.emplace_back(0 & (~pacMask));
    jsCallFrame.symbolName_ = "UnwindErrorDwarf";
    jsCallFrame.isJsFrame_ = true;
    jsCallFrame.needReport_ |= CALL_FRAME_REPORT;
    jsCallFrame.needReport_ |= SYMBOL_NAME_ID_REPORT;
    jsCallFrame.needReport_ |= FILE_PATH_ID_REPORT;
    jsCallFrame.callFrameId_ = DWARF_ERROR_ID;
    jsCallFrame.symbolNameId_ = DWARF_ERROR_ID;
    jsCallFrame.filePathId_ = DWARF_ERROR_ID;
    jsCallFrame.filePath_ = "no-file-path";
}

void StackPreprocess::FlushRecordApplyAndReleaseMatchData()
{
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
                SetEventFrame(rawStack, allocEvent, rawStack.stackMapId);
            } else if (rawStack.type == FREE_MSG) {
                auto freeEvent = hookData->mutable_free_event();
                SetEventFrame(rawStack, freeEvent, rawStack.stackMapId);
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
            } else if (rawStack.type == MUNMAP_MSG) {
                auto munmapEvent = hookData->mutable_munmap_event();
                SetEventFrame(rawStack, munmapEvent, rawStack.stackMapId);
            } else if (rawStack.type == PR_SET_VMA_MSG) {
                auto tagEvent = hookData->mutable_tag_event();
                const std::string prefix = "Anonymous:";
                tagEvent->set_addr(rawStack.addr);
                tagEvent->set_size(rawStack.mallocSize);
                tagEvent->set_tag(prefix + prctlPeriodTags_[rawStack.tagId]);
                tagEvent->set_pid(pid_);
            } else if (rawStack.type == MEMORY_USING_MSG) {
                auto mmapEvent = hookData->mutable_mmap_event();
                std::string tagName;
                GetMemTag(rawStack.tagId, tagName);
                SetEventFrame(rawStack, mmapEvent, rawStack.stackMapId, tagName);
            } else if (rawStack.type == MEMORY_UNUSING_MSG) {
                auto munmapEvent = hookData->mutable_munmap_event();
                SetEventFrame(rawStack, munmapEvent, rawStack.stackMapId);
            }
        }
        FlushData(stackData);
        }, stackData_);
    applyAndReleaseMatchPeriodListData_.clear();
    applyAndReleaseMatchIntervallMap_.clear();
    prctlPeriodTags_.clear();
}