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

#include "cpu_filter.h"
#include "args_filter.h"
#include "process_filter.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
CpuFilter::CpuFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter) : FilterBase(dataCache, filter) {}
CpuFilter::~CpuFilter() = default;
void CpuFilter::InsertSwitchEvent(uint64_t ts,
                                  uint64_t cpu,
                                  uint32_t prevPid,
                                  uint64_t prevPior,
                                  uint64_t prevState,
                                  uint32_t nextPid,
                                  uint64_t nextPior,
                                  DataIndex nextInfo)
{
    BinderTransactionInfo btInfo = {prevPid, nextPid, INVALID_UINT64, INVALID_UINT64};
    auto index = traceDataCache_->GetSchedSliceData()->AppendSchedSlice(ts, 0, cpu, nextPid, 0, nextPior);
    auto prevTidOnCpu = cpuToRowSched_.find(cpu);
    if (prevTidOnCpu != cpuToRowSched_.end()) {
        traceDataCache_->GetSchedSliceData()->Update(prevTidOnCpu->second.row, ts, prevState);
        btInfo.schedSliceRow = prevTidOnCpu->second.row;
        cpuToRowSched_.at(cpu).row = index;
    } else {
        cpuToRowSched_.insert(std::make_pair(cpu, RowPos{nextPid, index}));
    }
    if (nextPid) {
        CheckWakeupEvent(nextPid);
        auto lastRow = RowOfInternalTidInStateTable(nextPid);
        if (lastRow != INVALID_UINT64) {
            // check if there are wakeup or waking events before
            traceDataCache_->GetThreadStateData()->UpdateDuration(static_cast<TableRowId>(lastRow), ts);
        }
        auto index =
            traceDataCache_->GetThreadStateData()->AppendThreadState(ts, INVALID_TIME, cpu, nextPid, TASK_RUNNING);
        if (nextInfo != INVALID_DATAINDEX) {
            ArgsSet args;
            args.AppendArg(nextInfo_, BASE_DATA_TYPE_STRING, nextInfo);
            auto argSetId = streamFilters_->argsFilter_->NewArgs(args);
            traceDataCache_->GetThreadStateData()->SetArgSetId(index, argSetId);
        }
        (void)RemberInternalTidInStateTable(nextPid, index, TASK_RUNNING);
        if (cpuToRowThreadState_.find(cpu) == cpuToRowThreadState_.end()) {
            cpuToRowThreadState_.insert(std::make_pair(cpu, index));
        } else {
            // only one thread on run on a cpu at a certain time
            if (traceDataCache_->GetThreadStateData()->ItidsData()[cpuToRowThreadState_.at(cpu)] != prevPid) {
                if (!traceDataCache_->GetThreadStateData()->End(static_cast<TableRowId>(cpuToRowThreadState_.at(cpu)),
                                                                ts)) {
                    ClearInternalTidInStateTable(
                        traceDataCache_->GetThreadStateData()->ItidsData()[cpuToRowThreadState_.at(cpu)]);
                }
            }
            cpuToRowThreadState_.at(cpu) = index;
        }
    }

    if (prevPid) {
        auto lastRow = RowOfInternalTidInStateTable(prevPid);
        if (lastRow != INVALID_UINT64) {
            CheckWakeupEvent(prevPid);
            traceDataCache_->GetThreadStateData()->UpdateDuration(static_cast<TableRowId>(lastRow), ts);
            streamFilters_->processFilter_->AddCpuStateCount(prevPid);
            auto thread = traceDataCache_->GetThreadData(prevPid);
            if (thread && !thread->switchCount_) {
                thread->switchCount_ = 1;
            }
        }
        auto threadStateRow =
            traceDataCache_->GetThreadStateData()->AppendThreadState(ts, INVALID_TIME, INVALID_CPU, prevPid, prevState);
        btInfo.threadStateRow = threadStateRow;
        if (prevState == TASK_UNINTERRUPTIBLE || prevState == TASK_DK) {
            if (!pidToThreadSliceRow.count(prevPid)) {
                pidToThreadSliceRow.emplace(std::make_pair(prevPid, threadStateRow));
            } else {
                pidToThreadSliceRow.at(prevPid) = threadStateRow;
            }
        }
        (void)RemberInternalTidInStateTable(prevPid, threadStateRow, prevState);
    }
    if (traceDataCache_->BinderRunnableTraceEnabled() && iTidToTransaction_.find(prevPid) != iTidToTransaction_.end()) {
        uint64_t transactionId = iTidToTransaction_.at(prevPid);
        auto iter = transactionIdToInfo_.find(transactionId);
        if (prevState != TASK_FOREGROUND || iter == transactionIdToInfo_.end() || iter->second.iTidFrom != prevPid ||
            btInfo.schedSliceRow == INVALID_UINT64 || btInfo.threadStateRow == INVALID_UINT64) {
            TransactionClear(prevState, transactionId);
            return;
        }
        transactionIdToInfo_[transactionId] = btInfo;
    }
}

bool CpuFilter::InsertBlockedReasonEvent(uint64_t ts,
                                         uint64_t cpu,
                                         uint32_t iTid,
                                         bool iowait,
                                         DataIndex caller,
                                         uint32_t delay)
{
    if (pidToThreadSliceRow.count(iTid)) {
        // ArgSet
        ArgsSet args;
        args.AppendArg(ioWait_, BASE_DATA_TYPE_INT, iowait);
        args.AppendArg(caller_, BASE_DATA_TYPE_STRING, caller);
        if (delay != INVALID_UINT32) {
            args.AppendArg(delay_, BASE_DATA_TYPE_INT, delay);
        }
        auto argSetId = streamFilters_->argsFilter_->NewArgs(args);
        auto row = pidToThreadSliceRow.at(iTid);
        traceDataCache_->GetThreadStateData()->SetArgSetId(row, argSetId);
        if (iowait) {
            auto state = traceDataCache_->GetThreadStateData()->StatesData()[row];
            if (state == TASK_UNINTERRUPTIBLE) {
                traceDataCache_->GetThreadStateData()->UpdateState(row, TASK_UNINTERRUPTIBLE_IO);
            } else if (state == TASK_DK) { // state == TASK_DK
                traceDataCache_->GetThreadStateData()->UpdateState(row, TASK_DK_IO);
            }
        } else {
            auto state = traceDataCache_->GetThreadStateData()->StatesData()[row];
            if (state == TASK_UNINTERRUPTIBLE) {
                traceDataCache_->GetThreadStateData()->UpdateState(row, TASK_UNINTERRUPTIBLE_NIO);
            } else if (state == TASK_DK) { // state == TASK_DK
                traceDataCache_->GetThreadStateData()->UpdateState(row, TASK_DK_NIO);
            }
        }
        pidToThreadSliceRow.erase(iTid);
    }
    return true;
}
bool CpuFilter::InsertProcessExitEvent(uint64_t ts, uint64_t cpu, uint32_t pid)
{
    UNUSED(cpu);
    auto thread = traceDataCache_->GetThreadData(static_cast<InternalTid>(pid));
    if (thread) {
        thread->endT_ = ts;
        return true;
    }
    return false;
}

bool CpuFilter::InsertProcessFreeEvent(uint64_t ts, uint32_t pid)
{
    auto thread = traceDataCache_->GetThreadData(static_cast<InternalTid>(pid));
    if (thread) {
        thread->endT_ = ts;
        return true;
    }
    return false;
}

void CpuFilter::Finish() const
{
    auto size = traceDataCache_->ThreadSize();
    for (auto i = 0; i < size; i++) {
        auto thread = traceDataCache_->GetThreadData(i);
        if (thread->internalPid_ != INVALID_UINT32) {
            traceDataCache_->GetProcessData(thread->internalPid_)->threadCount_++;
            traceDataCache_->GetProcessData(thread->internalPid_)->cpuStatesCount_ += thread->cpuStatesCount_;
            traceDataCache_->GetProcessData(thread->internalPid_)->sliceSize_ += thread->sliceSize_;
            traceDataCache_->GetProcessData(thread->internalPid_)->switchCount_ += thread->switchCount_;
            continue;
        }
        auto ipid = traceDataCache_->AppendNewProcessData(
            thread->tid_, traceDataCache_->GetDataFromDict(thread->nameIndex_), thread->startT_);
        thread->internalPid_ = ipid;
        traceDataCache_->GetProcessData(thread->internalPid_)->threadCount_++;
        traceDataCache_->GetProcessData(thread->internalPid_)->cpuStatesCount_ += thread->cpuStatesCount_;
        traceDataCache_->GetProcessData(thread->internalPid_)->sliceSize_ += thread->sliceSize_;
        traceDataCache_->GetProcessData(thread->internalPid_)->switchCount_ += thread->switchCount_;
    }
    auto threadState = traceDataCache_->GetConstThreadStateData();
    size = threadState.Size();
    auto rowData = threadState.ItidsData();
    for (auto i = 0; i < size; i++) {
        auto thread = traceDataCache_->GetThreadData(rowData[i]);
        if (thread->internalPid_ == INVALID_UINT32) {
            continue;
        }
        auto process = traceDataCache_->GetProcessData(thread->internalPid_);
        traceDataCache_->GetThreadStateData()->UpdateTidAndPid(i, thread->tid_, process->pid_);
    }
    auto slice = traceDataCache_->GetConstSchedSliceData();
    size = slice.Size();
    for (auto i = 0; i < size; i++) {
        traceDataCache_->GetSchedSliceData()->AppendInternalPid(
            traceDataCache_->GetThreadData(slice.InternalTidsData()[i])->internalPid_);
    }
}
void CpuFilter::Clear()
{
    cpuToRowThreadState_.clear();
    cpuToRowSched_.clear();
    lastWakeUpMsg_.clear();
    internalTidToRowThreadState_.clear();
    iTidToTransaction_.clear();
    transactionIdToInfo_.clear();
}
void CpuFilter::InsertWakeupEvent(uint64_t ts, uint32_t internalTid, bool isWaking)
{
    if (!isWaking && !toRunnableTid_.count(internalTid)) {
        toRunnableTid_[internalTid] = ts;
    }
}
uint64_t CpuFilter::RemberInternalTidInStateTable(uint32_t uid, uint64_t row, uint64_t state)
{
    if (internalTidToRowThreadState_.find(uid) != internalTidToRowThreadState_.end()) {
        internalTidToRowThreadState_.at(uid) = TPthread{row, state};
    } else {
        internalTidToRowThreadState_.insert(std::make_pair(uid, TPthread{row, state}));
    }
    return 0;
}
uint64_t CpuFilter::RowOfInternalTidInStateTable(uint32_t uid) const
{
    auto row = internalTidToRowThreadState_.find(uid);
    if (row != internalTidToRowThreadState_.end()) {
        return (*row).second.row_;
    }
    return INVALID_UINT64;
}
void CpuFilter::ClearInternalTidInStateTable(uint32_t uid)
{
    auto row = internalTidToRowThreadState_.find(uid);
    if (row != internalTidToRowThreadState_.end()) {
        internalTidToRowThreadState_.erase(row);
    }
}
uint64_t CpuFilter::StateOfInternalTidInStateTable(uint32_t uid) const
{
    auto row = internalTidToRowThreadState_.find(uid);
    if (row != internalTidToRowThreadState_.end()) {
        return (*row).second.state_;
    }
    return TASK_INVALID;
}

void CpuFilter::CheckWakeupEvent(uint32_t internalTid)
{
    if (toRunnableTid_.count(internalTid)) {
        uint64_t lastrow = RowOfInternalTidInStateTable(internalTid);
        auto lastState = StateOfInternalTidInStateTable(internalTid);
        if (lastState == TASK_RUNNING) {
            toRunnableTid_.erase(internalTid);
            return;
        }
        if (lastrow != INVALID_UINT64) {
            traceDataCache_->GetThreadStateData()->UpdateDuration(static_cast<TableRowId>(lastrow),
                                                                  toRunnableTid_.at(internalTid));
        }
        auto index = traceDataCache_->GetThreadStateData()->AppendThreadState(
            toRunnableTid_.at(internalTid), INVALID_TIME, INVALID_CPU, internalTid, TASK_RUNNABLE);
        (void)RemberInternalTidInStateTable(internalTid, index, TASK_RUNNABLE);
        toRunnableTid_.erase(internalTid);
    }
    return;
}

void CpuFilter::InsertRunnableBinderEvent(uint32_t transactionId, uint32_t iTid)
{
    if (iTidToTransaction_.find(iTid) != iTidToTransaction_.end()) {
        iTidToTransaction_.erase(iTid);
    }
    iTidToTransaction_.emplace(iTid, transactionId);
    if (transactionIdToInfo_.find(transactionId) != transactionIdToInfo_.end()) {
        transactionIdToInfo_.erase(transactionId);
    }
    transactionIdToInfo_.emplace(transactionId,
                                 BinderTransactionInfo{iTid, INVALID_UINT32, INVALID_UINT64, INVALID_UINT64});
}

void CpuFilter::InsertRunnableBinderRecvEvent(uint32_t transactionId, uint32_t iTid)
{
    auto iter = transactionIdToInfo_.find(transactionId);
    if (iter == transactionIdToInfo_.end()) {
        return;
    }
    if (iter->second.iTidTo == iTid && iter->second.schedSliceRow != INVALID_UINT64 &&
        iter->second.threadStateRow != INVALID_UINT64) {
        traceDataCache_->GetSchedSliceData()->UpdateEndState(iter->second.schedSliceRow, TASK_RUNNABLE_BINDER);
        traceDataCache_->GetThreadStateData()->UpdateState(iter->second.threadStateRow, TASK_RUNNABLE_BINDER);
    }
    TransactionClear(INVALID_UINT32, transactionId);
}

void CpuFilter::TransactionClear(uint32_t iTidFrom, uint32_t transactionId)
{
    if (iTidToTransaction_.find(iTidFrom) != iTidToTransaction_.end()) {
        iTidToTransaction_.erase(iTidFrom);
    }
    auto iter = transactionIdToInfo_.find(transactionId);
    if (iter != transactionIdToInfo_.end()) {
        if (iTidToTransaction_.find(iter->second.iTidFrom) != iTidToTransaction_.end()) {
            iTidToTransaction_.erase(iter->second.iTidFrom);
        }
        transactionIdToInfo_.erase(transactionId);
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
