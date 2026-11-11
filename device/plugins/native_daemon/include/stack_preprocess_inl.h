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
#ifndef STACK_PREPROCESS_INL_H
#define STACK_PREPROCESS_INL_H

// This file contains inline implementations for StackPreprocess.
// Must be included AFTER the class declaration in stack_preprocess.h.
// Functions with cpp-side inline definitions (HandleMoveAddr, FindAndUpdateFreeStatisticsLocked,
// AddAllocStatistics, UpdateAllocStatistics) are NOT included here.

namespace OHOS::Developtools::NativeDaemon {

// ========== RecordStatistic inline implementations ==========

inline uint64_t RecordStatistic::GetNodeId() const
{
    return nodeId;
}

inline void RecordStatistic::SetNodeId(uint64_t val)
{
    nodeId = val;
}

inline uint32_t RecordStatistic::GetNodeType() const
{
    return nodeType;
}

inline void RecordStatistic::SetNodeType(uint32_t val)
{
    nodeType = val;
}

// ========== SimplifiedRecordStatistics inline implementations ==========

inline uint64_t SimplifiedRecordStatistics::GetNodeId() const
{
    return 0;
}

inline void SimplifiedRecordStatistics::SetNodeId(uint64_t val)
{
    (void) val;
}

inline uint32_t SimplifiedRecordStatistics::GetNodeType() const
{
    return 0;
}

inline void SimplifiedRecordStatistics::SetNodeType(uint32_t val)
{
    (void) val;
}

// ========== StackPreprocess inline implementations ==========

inline void StackPreprocess::SetFactory(std::shared_ptr<HookRecordFactory> factory)
{
    factory_ = factory;
}

inline void StackPreprocess::SetServerCommandCallback(std::function<void(uint32_t)> callback)
{
    serverCommandCallback_ = std::move(callback);
}

inline bool StackPreprocess::IsDataSourceTerminated() const
{
    return dataSourceTerminated_;
}

inline void StackPreprocess::SetPid(int32_t pid)
{
    pid_ = pid;
    if (COMMON::GetNSpid(pid) != -1) {
        isContainer_ = true;
        runtime_instance->SetContainerInfo(true);
    }
}

inline void StackPreprocess::InitStatisticsTime()
{
    if (hookConfig_.use_file_cache_mode()) {
        lastCacheStatisticsTime_ = {0, 0};
    } else {
        lastStatisticsTime_ = std::chrono::steady_clock::now();
    }
}

inline void StackPreprocess::UpdateStatisticsTime()
{
    if (hookConfig_.use_file_cache_mode()) {
        lastCacheStatisticsTime_.tv_sec += hookConfig_.statistics_interval();
    }
}

inline void StackPreprocess::SetNmdFd(uint32_t fd)
{
    nmdFd_ = fd;
}

inline void StackPreprocess::SetFlushSize(uint64_t size)
{
    double tenth = static_cast<double>(size) / 10.0;
    flushSize_ = static_cast<uint64_t>(std::ceil(tenth));
    PROFILER_LOG_INFO(LOG_CORE, "SetFlushSize size: %" PRIu64 ", flushSize_: %" PRIu64 "", size, flushSize_);
    if (isProtobufSerialize_) {
        bufferSize_ = flushSize_ << 1;
        buffer_ = std::make_unique<uint8_t[]>(bufferSize_);
    }
}

inline void StackPreprocess::SetIsStartTakeData(bool isStartTakeData)
{
    if (isStartTakeData_ != isStartTakeData) {
        isStartTakeData_ = isStartTakeData;
    }
}

inline void StackPreprocess::SetIsStopTakeData(bool isStopTakeData)
{
    if (isStopTakeData_ != isStopTakeData) {
        isStopTakeData_ = isStopTakeData;
    }
}

inline void StackPreprocess::SetIsCacheFileParseDone(bool isCacheFileParseDone)
{
    if (isCacheFileParseDone_ != isCacheFileParseDone) {
        isCacheFileParseDone_ = isCacheFileParseDone;
    }
}

} // namespace OHOS::Developtools::NativeDaemon

#endif // STACK_PREPROCESS_INL_H
