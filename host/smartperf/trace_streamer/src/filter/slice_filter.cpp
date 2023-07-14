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

#include "slice_filter.h"
#include <cstdint>
#include <limits>
#include <optional>

#include "args_filter.h"
#include "log.h"
#include "measure_filter.h"
#include "process_filter.h"
#include "stat_filter.h"
#include "string_to_numerical.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace SysTuning::base;
SliceFilter::SliceFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter), asyncEventMap_(INVALID_UINT64)
{
}

SliceFilter::~SliceFilter() = default;

size_t SliceFilter::BeginSlice(const std::string& comm,
                               uint64_t timeStamp,
                               uint32_t pid,
                               uint32_t threadGroupId,
                               DataIndex cat,
                               DataIndex nameIndex)
{
    InternalTid internalTid = INVALID_ITID;
    if (threadGroupId > 0) {
        internalTid = streamFilters_->processFilter_->UpdateOrCreateThreadWithPidAndName(pid, threadGroupId, comm);
        pidTothreadGroupId_[pid] = threadGroupId;
    } else {
        internalTid = streamFilters_->processFilter_->UpdateOrCreateThreadWithName(timeStamp, pid, comm);
    }
    // make a SliceData DataItem, {timeStamp, dur, internalTid, cat, nameIndex}
    struct SliceData sliceData = {timeStamp, -1, internalTid, cat, nameIndex};
    ArgsSet args;
    return StartSlice(timeStamp, pid, cat, nameIndex, args, sliceData);
}

void SliceFilter::IrqHandlerEntry(uint64_t timeStamp, uint32_t cpu, DataIndex catalog, DataIndex nameIndex)
{
    // clear ipi for current cpu and nameIndex
    irqDataLinker_.erase(cpu);
    struct SliceData sliceData = {timeStamp, 0, cpu, catalog, nameIndex};
    auto slices = traceDataCache_->GetIrqData();
    size_t index = slices->AppendInternalSlice(
        sliceData.timeStamp, sliceData.duration, sliceData.internalTid, sliceData.cat,
        GetNameASCIISumNoNum(traceDataCache_->GetDataFromDict(sliceData.name)), sliceData.name, 0, std::nullopt);
    if (irqEventMap_.count(cpu)) {
        // not match
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_ENTRY, STAT_EVENT_DATA_LOST);
        irqEventMap_.at(cpu) = {timeStamp, index};
    } else {
        irqEventMap_[cpu] = {timeStamp, index};
    }
    return;
}

void SliceFilter::IrqHandlerExit(uint64_t timeStamp, uint32_t cpu, ArgsSet args)
{
    if (!irqEventMap_.count(cpu)) {
        // not match
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_EXIT, STAT_EVENT_NOTMATCH);
        return;
    }
    uint32_t argSetId = INVALID_UINT32;
    auto slices = traceDataCache_->GetIrqData();
    argSetId = streamFilters_->argsFilter_->NewArgs(args);
    slices->SetIrqDurAndArg(irqEventMap_.at(cpu).row, timeStamp, argSetId);
    auto internalEventRow = irqDataLinker_.find(cpu);
    if (internalEventRow != irqDataLinker_.end()) {
        slices->SetArgSetId(internalEventRow->second, slices->ArgSetIdsData()[irqEventMap_.at(cpu).row]);
        slices->SetDurationEx(irqEventMap_.at(cpu).row, slices->DursData()[internalEventRow->second]);
    } else {
        slices->SetFlag(irqEventMap_.at(cpu).row, 1);
    }
    irqDataLinker_.erase(cpu);
    irqEventMap_.erase(cpu);
    return;
}

void SliceFilter::IpiHandlerEntry(uint64_t timeStamp, uint32_t cpu, DataIndex catalog, DataIndex nameIndex)
{
    irqDataLinker_.erase(cpu);
    struct SliceData sliceData = {timeStamp, 0, cpu, catalog, nameIndex};
    auto slices = traceDataCache_->GetIrqData();
    size_t index = slices->AppendInternalSlice(
        sliceData.timeStamp, sliceData.duration, sliceData.internalTid, sliceData.cat,
        GetNameASCIISumNoNum(traceDataCache_->GetDataFromDict(sliceData.name)), sliceData.name, 0, std::nullopt);
    if (ipiEventMap_.count(cpu)) {
        // not match
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_IRQ_HANDLER_ENTRY, STAT_EVENT_DATA_LOST);
        ipiEventMap_.at(cpu) = {timeStamp, index};
    } else {
        ipiEventMap_[cpu] = {timeStamp, index};
    }
    return;
}
void SliceFilter::IpiHandlerExit(uint64_t timeStamp, uint32_t cpu)
{
    if (!ipiEventMap_.count(cpu)) {
        // not match
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST);
        return;
    }
    auto slices = traceDataCache_->GetIrqData();
    slices->SetDurationWithFlag(ipiEventMap_.at(cpu).row, timeStamp);
    irqDataLinker_.emplace(cpu, ipiEventMap_.at(cpu).row);
    ipiEventMap_.erase(cpu);
}
void SliceFilter::SoftIrqEntry(uint64_t timeStamp, uint32_t cpu, DataIndex catalog, DataIndex nameIndex)
{
    struct SliceData sliceData = {timeStamp, 0, cpu, catalog, nameIndex};
    auto slices = traceDataCache_->GetIrqData();
    size_t index = slices->AppendInternalSlice(
        sliceData.timeStamp, sliceData.duration, sliceData.internalTid, sliceData.cat,
        GetNameASCIISumNoNum(traceDataCache_->GetDataFromDict(sliceData.name)), sliceData.name, 0, std::nullopt);
    if (softIrqEventMap_.count(cpu)) {
        // not match
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SOFTIRQ_ENTRY, STAT_EVENT_DATA_LOST);
        softIrqEventMap_.at(cpu) = {timeStamp, index};
    } else {
        softIrqEventMap_[cpu] = {timeStamp, index};
    }
    return;
}

void SliceFilter::SoftIrqExit(uint64_t timeStamp, uint32_t cpu, ArgsSet args)
{
    if (!softIrqEventMap_.count(cpu)) {
        // not match
        streamFilters_->statFilter_->IncreaseStat(TRACE_EVENT_SOFTIRQ_EXIT, STAT_EVENT_DATA_LOST);
        return;
    }
    uint32_t argSetId = INVALID_UINT32;
    auto slices = traceDataCache_->GetIrqData();
    argSetId = streamFilters_->argsFilter_->NewArgs(args);
    slices->SetIrqDurAndArg(softIrqEventMap_.at(cpu).row, timeStamp, argSetId);
    softIrqEventMap_.erase(cpu);
    return;
}

void SliceFilter::RememberSliceData(InternalTid internalTid,
                                    std::unordered_map<InternalTid, StackOfSlices>& stackMap,
                                    SliceData& slice,
                                    uint32_t depth,
                                    uint64_t index)
{
    if (stackMap.find(internalTid) == stackMap.end()) {
        auto& sliceStack = stackMap[internalTid].sliceStack; // this can be a empty call, but it does not matter
        slice.depth = depth;
        slice.index = index;
        sliceStack.push_back(slice);
    } else {
        auto& sliceStack = stackMap.at(internalTid).sliceStack; // this can be a empty call, but it does not matter
        slice.depth = depth;
        slice.index = index;
        sliceStack.push_back(slice);
    }
}
size_t SliceFilter::AsyncBinder(uint64_t timeStamp, uint32_t pid, DataIndex cat, DataIndex nameIndex, ArgsSet& args)
{
    InternalTid internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(timeStamp, pid);
    struct SliceData sliceData = {timeStamp, 0, internalTid, cat, nameIndex};
    return StartSlice(timeStamp, pid, cat, nameIndex, args, std::move(sliceData));
}
uint8_t SliceFilter::CurrentDepth(InternalTid internalTid)
{
    if (depthHolder_.find(internalTid) == depthHolder_.end()) {
        return 0;
    }
    auto& depthMap = depthHolder_.at(internalTid);
    auto depthSize = depthMap.size();
    for (int32_t i = depthSize - 1; i >= 0; i--) {
        if (depthMap.at(i)) {
            return i;
        }
    }
    return 0;
}
uint8_t SliceFilter::UpdateDepth(bool increase, InternalTid internalTid, int32_t depth)
{
    if (increase) {
        if (depthHolder_.find(internalTid) == depthHolder_.end()) {
            StackOnDepth tmp;
            tmp.insert(std::make_pair(0, true));
            depthHolder_.insert(std::make_pair(internalTid, tmp));
            return 0;
        }
        auto& depthMap = depthHolder_.at(internalTid);
        auto depthSize = depthMap.size();
        auto lastIndex = 0;
        for (int32_t i = depthSize - 1; i >= 0; i--) {
            if (depthMap.at(i) && (i == depthSize - 1)) {
                depthMap.insert(std::make_pair(depthSize, true));
                return depthSize;
            }
            if (depthMap.at(i)) {
                break;
            }
            lastIndex = i;
        }

        if (!depthMap.at(lastIndex)) {
            depthMap.at(lastIndex) = true;
            return lastIndex;
        }
    } else {
        if (depthHolder_.find(internalTid) == depthHolder_.end()) {
            TS_LOGE("internalTid not found");
            return 0;
        }
        auto& depthMap = depthHolder_.at(internalTid);
        if (depthMap.find(depth) == depthMap.end()) {
            return 0;
        }
        depthMap.at(depth) = false;
    }
    return depth;
}

void SliceFilter::CloseUnMatchedSlice(int64_t ts, SlicesStack& stack, InternalTid itid)
{
    auto slices = traceDataCache_->GetInternalSlicesData();
    bool incomplete = false;
    for (int32_t i = stack.size() - 1; i >= 0; i--) {
        uint32_t sliceIdx = stack[i].index;
        int64_t startTs = slices->TimeStampData()[sliceIdx];
        int64_t dur = slices->DursData()[sliceIdx];
        int64_t endTs = startTs + dur;
        if (dur == -1) {
            incomplete = true;
            continue;
        }
        if (incomplete) {
            if (ts <= endTs) {
                continue;
            }
            for (int32_t j = stack.size() - 1; j > i; --j) {
                uint32_t childIdx = stack[i].index;
                slices->SetDur(childIdx, endTs - slices->TimeStampData()[childIdx]);
                stack.pop_back();
            }
            stack.pop_back();
            incomplete = false;
            continue;
        }
        if (endTs <= ts) {
            stack.pop_back();
        }
    }
}

int32_t SliceFilter::MatchingIncompleteSliceIndex(const SlicesStack& stack, DataIndex category, DataIndex name)
{
    auto slices = traceDataCache_->GetInternalSlicesData();
    for (int32_t i = stack.size() - 1; i >= 0; i--) {
        uint32_t sliceIdx = stack[i].index;
        if (slices->DursData()[sliceIdx] != -1) {
            continue;
        }
        const DataIndex& categoryLast = slices->CatsData()[sliceIdx];
        if (category != INVALID_UINT64 && (categoryLast != INVALID_UINT64 && category != categoryLast)) {
            continue;
        }
        const DataIndex& nameLast = slices->NamesData()[sliceIdx];
        if (name != INVALID_UINT64 && nameLast != INVALID_UINT64 && name != nameLast) {
            continue;
        }
        return static_cast<int32_t>(i);
    }
    return -1;
}
size_t SliceFilter::StartSlice(uint64_t timeStamp,
                               uint32_t pid,
                               DataIndex cat,
                               DataIndex nameIndex,
                               ArgsSet& args,
                               SliceData sliceData)
{
    InternalTid internalTid = sliceData.internalTid;
    auto& sliceStack = binderStackMap_[internalTid];
    auto& stack = sliceStack.sliceStack;
    if (sliceStack.isAsyncEvent) {
        sliceStack.asyncEventCount++;
        sliceStack.asyncEventLastBeginTs = timeStamp;
        if (!stack.empty()) {
            return SIZE_MAX;
        }
    }
    // keep slice of thread
    CloseUnMatchedSlice(timeStamp, stack, internalTid);
    uint32_t depth = stack.size();
    auto slices = traceDataCache_->GetInternalSlicesData();
    uint32_t parentId = depth == 0 ? INVALID_UINT32 : slices->IdsData()[stack.back().index];
    size_t index = slices->AppendInternalSlice(sliceData.timeStamp, sliceData.duration, internalTid, sliceData.cat,
                                               GetNameASCIISumNoNum(traceDataCache_->GetDataFromDict(sliceData.name)),
                                               sliceData.name, 0, parentId);
    if (depth >= std::numeric_limits<uint8_t>::max()) {
        return SIZE_MAX;
    }
    slices->SetDepth(index, depth);

    uint32_t argSetId = INVALID_UINT32;
    if (args.valuesMap_.size()) {
        if (args.inserted_) {
            argSetId = args.argSetId_;
        } else {
            argSetId = streamFilters_->argsFilter_->NewArgs(args);
            sliceRowToArgsSetId_[index] = argSetId;
            argsSetIdToSliceRow_[argSetId] = static_cast<uint32_t>(index);
            args.argSetId_ = argSetId;
            args.inserted_ = true;
        }
        // set ArgSetId here
        slices->SetArgSetId(index, argSetId);
    }
    sliceData.argSetId = argSetId;
    RememberSliceData(internalTid, binderStackMap_, sliceData, depth, index);
    return index;
}
size_t SliceFilter::BeginBinder(uint64_t timeStamp, uint32_t pid, DataIndex cat, DataIndex nameIndex, ArgsSet args)
{
    InternalTid internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(timeStamp, pid);
    struct SliceData sliceData = {timeStamp, -1, internalTid, cat, nameIndex};
    return StartSlice(timeStamp, pid, cat, nameIndex, args, std::move(sliceData));
}

size_t SliceFilter::CompleteSlice(uint64_t timeStamp,
                                  uint32_t pid,
                                  uint32_t threadGroupId,
                                  DataIndex category,
                                  DataIndex name,
                                  ArgsSet args)
{
    InternalTid internalTid = INVALID_ITID;
    if (threadGroupId > 0) {
        internalTid = streamFilters_->processFilter_->GetOrCreateThreadWithPid(pid, threadGroupId);
    } else {
        internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(timeStamp, pid);
    }
    if (binderStackMap_.find(internalTid) == binderStackMap_.end()) {
        return SIZE_MAX;
    }
    auto& stackInfo = binderStackMap_[internalTid];
    SlicesStack& stack = stackInfo.sliceStack;
    CloseUnMatchedSlice(timeStamp, stack, internalTid);
    if (stack.empty()) {
        TS_LOGE("a slice end do not match a slice start event");
        callEventDisMatchCount++;
        return SIZE_MAX;
    }
    auto stackIdx = MatchingIncompleteSliceIndex(stack, category, name);
    if (stackIdx < 0) {
        TS_LOGE("MatchingIncompleteSliceIndex failed");
        return SIZE_MAX;
    }
    auto lastRow = stack[stackIdx].index;
    auto slices = traceDataCache_->GetInternalSlicesData();
    slices->SetDuration(lastRow, timeStamp);

    auto argSize = sliceRowToArgsSetId_.count(lastRow);
    size_t argSetId = 0;
    if (args.valuesMap_.size()) {
        if (!argSize) {
            argSetId = streamFilters_->argsFilter_->NewArgs(args);
            sliceRowToArgsSetId_[lastRow] = argSetId;
            slices->SetArgSetId(lastRow, argSetId);
        } else {
            argSetId = sliceRowToArgsSetId_.at(lastRow);
        }
        streamFilters_->argsFilter_->AppendArgs(args, argSetId);
    }
    if (stackInfo.isAsyncEvent) {
        ArgsSet args;
        args.AppendArg(asyncBeginCountId_, BASE_DATA_TYPE_INT, stackInfo.asyncEventCount);
        args.AppendArg(asyncBeginTsId_, BASE_DATA_TYPE_INT, stackInfo.asyncEventLastBeginTs);
        if (!argSetId) {
            argSetId = streamFilters_->argsFilter_->NewArgs(args);
            sliceRowToArgsSetId_[lastRow] = argSetId;
            slices->SetArgSetId(lastRow, argSetId);
        } else {
            streamFilters_->argsFilter_->AppendArgs(args, argSetId);
        }
    }
    if (stackIdx == stack.size() - 1) {
        stack.pop_back();
    }
    streamFilters_->processFilter_->AddThreadSliceNum(internalTid);
    return lastRow;
}
size_t SliceFilter::EndBinder(uint64_t timeStamp, uint32_t pid, DataIndex category, DataIndex name, ArgsSet args)
{
    return CompleteSlice(timeStamp, pid, 0, category, name, args);
}
std::tuple<uint64_t, uint32_t> SliceFilter::AddArgs(uint32_t tid, DataIndex key1, DataIndex key2, ArgsSet& args)
{
    InternalTid internalTid = streamFilters_->processFilter_->GetInternalTid(tid);
    if (binderStackMap_.find(internalTid) == binderStackMap_.end()) {
        return std::make_tuple(INVALID_UINT32, INVALID_UINT32);
    }
    auto& stack = binderStackMap_[internalTid];
    auto idx = MatchingIncompleteSliceIndex(stack.sliceStack, key1, key2);
    if (idx < 0) {
        return std::make_tuple(INVALID_UINT32, INVALID_UINT32);
    }
    uint32_t argSetId = stack.sliceStack[idx].argSetId;
    if (argSetId == INVALID_UINT32) {
        argSetId = streamFilters_->argsFilter_->NewArgs(args);
        sliceRowToArgsSetId_[stack.sliceStack[idx].index] = argSetId;
        stack.sliceStack[idx].argSetId = argSetId;
    } else {
        streamFilters_->argsFilter_->AppendArgs(args, argSetId);
    }
    return std::make_tuple(stack.sliceStack[idx].index, argSetId);
}
uint64_t SliceFilter::StartAsyncSlice(uint64_t timeStamp,
                                      uint32_t pid,
                                      uint32_t threadGroupId,
                                      uint64_t cookie,
                                      DataIndex nameIndex)
{
    UNUSED(pid);
    InternalPid internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(timeStamp, threadGroupId);

    auto lastFilterId = asyncEventMap_.Find(internalTid, cookie, nameIndex);
    auto slices = traceDataCache_->GetInternalSlicesData();
    if (lastFilterId != INVALID_UINT64) {
        asyncEventDisMatchCount++;
        return INVALID_UINT64;
    }
    asyncEventSize_++;
    // a pid, cookie and function name determain a callstack
    asyncEventMap_.Insert(internalTid, cookie, nameIndex, asyncEventSize_);
    // the IDE need a depth to paint call slice in different position of the canvas, the depth of async call
    // do not mean the parent-to-child relationship, it is different from no-async call
    uint8_t depth = 0;
    size_t index = slices->AppendInternalAsyncSlice(timeStamp, -1, internalTid, INVALID_UINT64,
                                                    GetNameASCIISumNoNum(traceDataCache_->GetDataFromDict(nameIndex)),
                                                    nameIndex, depth, cookie, std::nullopt);
    asyncEventFilterMap_.insert(std::make_pair(asyncEventSize_, AsyncEvent{timeStamp, index}));
    return index;
}

uint64_t SliceFilter::FinishAsyncSlice(uint64_t timeStamp,
                                       uint32_t pid,
                                       uint32_t threadGroupId,
                                       uint64_t cookie,
                                       DataIndex nameIndex)
{
    UNUSED(pid);
    InternalPid internalTid = streamFilters_->processFilter_->UpdateOrCreateThread(timeStamp, threadGroupId);
    auto lastFilterId = asyncEventMap_.Find(internalTid, cookie, nameIndex);
    auto slices = traceDataCache_->GetInternalSlicesData();
    if (lastFilterId == INVALID_UINT64) { // if failed
        asyncEventDisMatchCount++;
        return INVALID_UINT64;
    }
    if (asyncEventFilterMap_.find(lastFilterId) == asyncEventFilterMap_.end()) {
        TS_LOGE("logic error");
        asyncEventDisMatchCount++;
        return INVALID_UINT64;
    }
    // update timeStamp
    asyncEventFilterMap_.at(lastFilterId).timeStamp = timeStamp;
    auto lastRow = asyncEventFilterMap_.at(lastFilterId).row;
    slices->SetDuration(asyncEventFilterMap_.at(lastFilterId).row, timeStamp);
    asyncEventFilterMap_.erase(lastFilterId);
    asyncEventMap_.Erase(internalTid, cookie, nameIndex);
    streamFilters_->processFilter_->AddThreadSliceNum(internalTid);
    return lastRow;
}

size_t
    SliceFilter::EndSlice(uint64_t timeStamp, uint32_t pid, uint32_t threadGroupId, DataIndex category, DataIndex name)
{
    return CompleteSlice(timeStamp, pid, threadGroupId, category, name);
}

void SliceFilter::Clear()
{
    asyncEventMap_.Clear();
    asyncNoEndingEventMap_.clear();
    irqEventMap_.clear();
    softIrqEventMap_.clear();
    asyncEventFilterMap_.clear();
    sliceStackMap_.clear();
    depthHolder_.clear();
    sliceRowToArgsSetId_.clear();
    argsSetIdToSliceRow_.clear();
    argsSetIdToSliceRow_.clear();
    argsSet_.clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
