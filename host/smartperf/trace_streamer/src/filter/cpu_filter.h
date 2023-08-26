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

#ifndef CPU_FILTER_H
#define CPU_FILTER_H

#include <cstdint>
#include <limits>
#include <map>
#include <string_view>
#include <tuple>
#include <unordered_map>

#include "filter_base.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
class TraceStreamerFilters;
class CpuFilter : private FilterBase {
public:
    CpuFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    CpuFilter(const CpuFilter&) = delete;
    CpuFilter& operator=(const CpuFilter&) = delete;
    ~CpuFilter() override;

public:
    void InsertSwitchEvent(uint64_t ts,
                           uint64_t cpu,
                           uint32_t prevPid,
                           uint64_t prevPior,
                           uint64_t prevState,
                           uint32_t nextPid,
                           uint64_t nextPior,
                           DataIndex nextInfo);
    bool InsertBlockedReasonEvent(uint64_t ts,
                                  uint64_t cpu,
                                  uint32_t iTid,
                                  bool iowait,
                                  DataIndex caller,
                                  uint32_t delay);
    void InsertWakeupEvent(uint64_t ts, uint32_t internalTid, bool isWaking = false);
    bool InsertProcessExitEvent(uint64_t ts, uint64_t cpu, uint32_t pid);
    bool InsertProcessFreeEvent(uint64_t ts, uint32_t pid);
    void InsertRunnableBinderEvent(uint32_t transactionId, uint32_t iTid);
    void InsertRunnableBinderRecvEvent(uint32_t transactionId, uint32_t iTid);
    void Finish() const;
    void Clear();

private:
    void CheckWakeupEvent(uint32_t internalTid);
    uint64_t RemberInternalTidInStateTable(uint32_t uid, uint64_t row, uint64_t state = TASK_INVALID);
    uint64_t RowOfInternalTidInStateTable(uint32_t uid) const;
    void ClearInternalTidInStateTable(uint32_t uid);
    uint64_t StateOfInternalTidInStateTable(uint32_t uid) const;
    void TransactionClear(uint32_t iTidFrom, uint32_t transactionId);
    std::map<uint64_t, uint64_t> cpuToRowThreadState_ = {};
    typedef struct {
        uint32_t iTid;
        uint64_t row;
    } RowPos;
    std::map<uint64_t, RowPos> cpuToRowSched_ = {};

    std::map<uint64_t, uint64_t> lastWakeUpMsg_ = {};
    std::map<uint32_t, uint64_t> pidToThreadSliceRow = {};
    struct TPthread {
        uint64_t row_;
        uint64_t state_;
    };
    std::map<uint32_t, TPthread> internalTidToRowThreadState_ = {};
    const DataIndex ioWait_ = traceDataCache_->GetDataIndex("iowait");
    const DataIndex nextInfo_ = traceDataCache_->GetDataIndex("next_info");
    const DataIndex caller_ = traceDataCache_->GetDataIndex("caller");
    const DataIndex delay_ = traceDataCache_->GetDataIndex("delay");
    std::map<uint64_t, uint64_t> toRunnableTid_ = {};

    struct BinderTransactionInfo {
        uint32_t iTidFrom;
        uint32_t iTidTo;
        uint64_t schedSliceRow;
        uint64_t threadStateRow;
    };
    std::unordered_map<uint32_t, uint32_t> iTidToTransaction_;
    std::unordered_map<uint32_t, BinderTransactionInfo> transactionIdToInfo_;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // CPU_FILTER_H
