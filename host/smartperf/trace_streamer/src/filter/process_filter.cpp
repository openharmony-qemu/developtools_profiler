/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "process_filter.h"
#include <cinttypes>
#include <limits>
#include <string_view>
#include <utility>

using CustomPair = std::pair<uint32_t, uint32_t>;
namespace SysTuning {
namespace TraceStreamer {
namespace {
const uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();
}
ProcessFilter::ProcessFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter)
{
    tidMappingSet_.insert(CustomPair(0, 0));
    pidToInternalPidMap_.insert(CustomPair(0, 0));
}

ProcessFilter::~ProcessFilter() {}

uint32_t ProcessFilter::UpdateOrCreateThreadWithName(uint64_t timeStamp, uint32_t tid, std::string_view name)
{
    DataIndex nameIndex = traceDataCache_->GetDataIndex(name);
    return UpdateOrCreateThreadWithNameIndex(timeStamp, tid, nameIndex);
}

void ProcessFilter::AddProcessMemory(uint32_t ipid)
{
    traceDataCache_->GetProcessData(ipid)->memSize_ = 1;
}

void ProcessFilter::AddThreadSliceNum(uint32_t itid)
{
    traceDataCache_->GetThreadData(itid)->sliceSize_ = 1;
}
void ProcessFilter::AddProcessSliceNum(uint32_t ipid)
{
    traceDataCache_->GetProcessData(ipid)->sliceSize_ = 1;
}

void ProcessFilter::AddCpuStateCount(uint32_t itid)
{
    auto thread = traceDataCache_->GetThreadData(itid);
    if (thread) {
        thread->cpuStatesCount_++;
    }
}
uint32_t ProcessFilter::UpdateOrCreateThread(uint64_t timeStamp, uint32_t tid)
{
    return UpdateOrCreateThreadWithNameIndex(timeStamp, tid, 0);
}
uint32_t ProcessFilter::UpdateOrCreateThreadWithPidAndName(uint32_t tid, uint32_t pid, std::string_view name)
{
    uint32_t internalTid = GetOrCreateThreadWithPid(tid, pid);
    auto thread = traceDataCache_->GetThreadData(internalTid);
    auto nameIndex = traceDataCache_->GetDataIndex(name);
    thread->nameIndex_ = nameIndex;
    // When the process ID is equal to the thread ID, the process name is also equal to the thread name
    if (tid == pid) {
        UpdateOrCreateProcessWithName(pid, name);
    }
    return internalTid;
}

uint32_t ProcessFilter::GetOrCreateThreadWithPid(uint32_t tid, uint32_t pid)
{
    TraceStdtype::Thread* thread = nullptr;
    uint32_t internalTid = INVALID_ID;
    if (pid == 0) {
        internalTid = GetInternalTid(tid);
    } else {
        internalTid = GetInternalTid(tid, pid);
    }
    if (internalTid != INVALID_ID) {
        thread = traceDataCache_->GetThreadData(internalTid);
    } else {
        std::tie(internalTid, thread) = NewThread(tid);
    }

    if (thread->internalPid_ == INVALID_UINT32 && pid != 0) {
        std::tie(thread->internalPid_, std::ignore) = CreateProcessMaybe(pid, thread->startT_);
    }

    return internalTid;
}

uint32_t ProcessFilter::UpdateOrCreateProcessWithName(uint32_t pid, std::string_view name)
{
    uint32_t internalPid = 0;
    TraceStdtype::Process* process = nullptr;
    std::tie(internalPid, process) = CreateProcessMaybe(pid, 0);
    if (process && name != "" && process->cmdLine_ != name) {
        process->cmdLine_ = std::string(name);
    }
    // update main thread name
    auto internalTid = GetInternalTid(pid, pid);
    if (internalTid != INVALID_ID) {
        auto thread = traceDataCache_->GetThreadData(internalTid);
        thread->nameIndex_ = traceDataCache_->GetDataIndex(process->cmdLine_);
    }
    return internalPid;
}

uint32_t ProcessFilter::UpdateOrCreateThreadWithNameIndex(uint64_t timeStamp, uint32_t tid, DataIndex threadNameIndex)
{
    TraceStdtype::Thread* thread = nullptr;
    auto& internalTids = GetInternalTids(tid);
    if (internalTids.size()) {
        if (!threadNameIndex) {
            return internalTids.back();
        }
        for (auto i : internalTids) {
            thread = traceDataCache_->GetThreadData(i);
            if (thread && threadNameIndex != thread->nameIndex_) {
                thread->nameIndex_ = threadNameIndex;
            }
        }
    } else {
        InternalTid internalTid = INVALID_ITID;
        std::tie(internalTid, thread) = NewThread(tid);
        if (!thread) {
            return INVALID_ID;
        }
        if (threadNameIndex != thread->nameIndex_) {
            thread->nameIndex_ = threadNameIndex;
        }
        if (timeStamp < thread->startT_) {
            thread->startT_ = timeStamp;
        }
        return internalTid;
    }
    return internalTids.back();
}
uint32_t ProcessFilter::GetInternalTid(uint32_t tid, uint32_t pid) const
{
    uint32_t internalTid = INVALID_ID;
    auto tidsPair = tidMappingSet_.equal_range(tid);
    for (auto it = tidsPair.first; it != tidsPair.second; it++) {
        uint32_t iterItid = it->second;
        auto iterThread = traceDataCache_->GetThreadData(iterItid);
        if (iterThread->internalPid_ == INVALID_UINT32) {
            internalTid = iterItid;
            continue;
        }

        const auto& iterProcess = traceDataCache_->GetConstProcessData(iterThread->internalPid_);
        if (iterProcess.pid_ == pid) {
            internalTid = iterItid;
            break;
        }
    }

    return internalTid;
}

uint32_t ProcessFilter::GetInternalTid(uint32_t tid) const
{
    auto itRange = tidMappingSet_.equal_range(tid);
    if (itRange.first != itRange.second) {
        auto internalTid = std::prev(itRange.second)->second;
        return internalTid;
    }
    return INVALID_ID;
}

std::vector<InternalTid>& ProcessFilter::GetInternalTids(uint32_t tid)
{
    tmpTids_.clear();
    auto itRange = tidMappingSet_.equal_range(tid);
    auto it = itRange.first;
    while (it != itRange.second) {
        tmpTids_.push_back(it->second);
        it++;
    }
    return tmpTids_;
}

bool ProcessFilter::isThreadNameEmpty(uint32_t tid) const
{
    auto internalTid = GetInternalTid(tid);
    if (internalTid != INVALID_ID) {
        auto thread = traceDataCache_->GetThreadData(internalTid);
        if (thread->nameIndex_) {
            return false;
        }
    }
    return true;
}

InternalPid ProcessFilter::GetInternalPid(uint32_t pid) const
{
    auto it = pidToInternalPidMap_.find(pid);
    if (it != pidToInternalPidMap_.end()) {
        return it->second;
    }
    return INVALID_ID;
}

InternalPid ProcessFilter::GetOrCreateInternalPid(uint64_t timeStamp, uint32_t pid)
{
    auto ipid = GetInternalPid(pid);
    if (ipid != INVALID_ID) {
        return ipid;
    }

    uint32_t internalPid = 0;
    TraceStdtype::Process* process = nullptr;
    std::tie(internalPid, process) = CreateProcessMaybe(pid, timeStamp);
    return internalPid;
}
std::tuple<uint32_t, TraceStdtype::Thread*> ProcessFilter::NewThread(uint32_t tid)
{
    uint32_t internalTid = traceDataCache_->NewInternalThread(tid);
    tidMappingSet_.emplace(tid, internalTid);
    auto thread = traceDataCache_->GetThreadData(internalTid);

    return std::make_tuple(internalTid, thread);
}

std::tuple<uint32_t, TraceStdtype::Process*> ProcessFilter::NewProcess(uint32_t pid)
{
    uint32_t internalPid = traceDataCache_->GetProcessInternalPid(pid);
    pidToInternalPidMap_.emplace(pid, internalPid);
    auto process = traceDataCache_->GetProcessData(internalPid);

    return std::make_tuple(internalPid, process);
}

std::tuple<uint32_t, TraceStdtype::Process*> ProcessFilter::CreateProcessMaybe(uint32_t pid, uint64_t startT)
{
    uint32_t internalPid = INVALID_ID;
    TraceStdtype::Process* process = nullptr;
    auto it = pidToInternalPidMap_.find(pid);
    if (it != pidToInternalPidMap_.end()) {
        internalPid = it->second;
        process = traceDataCache_->GetProcessData(internalPid);
    } else {
        std::tie(internalPid, process) = NewProcess(pid);
        void(GetOrCreateThreadWithPid(pid, pid));
    }

    if (process->startT_ == 0) {
        process->startT_ = startT;
    }

    return std::make_tuple(internalPid, process);
}
void ProcessFilter::Clear()
{
    tidMappingSet_.clear();
    pidToInternalPidMap_.clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
