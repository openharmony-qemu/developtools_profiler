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

#ifndef SLICE_FILTER_H
#define SLICE_FILTER_H

#include <cstdint>
#include <vector>
#include "args_set.h"
#include "filter_base.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"
#include "triple_map.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
struct SliceData {
    uint64_t timeStamp;
    int32_t duration;
    InternalTid internalTid;
    DataIndex cat;
    DataIndex name;
    uint32_t depth;
    uint64_t index;
    uint32_t argSetId;
};
struct AsyncEvent {
    uint64_t timeStamp;
    size_t row;
};
class SliceFilter : private FilterBase {
public:
    SliceFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    ~SliceFilter() override;

    size_t BeginSlice(const std::string& comm,
                      uint64_t timeStamp,
                      uint32_t pid,
                      uint32_t threadGroupId,
                      DataIndex cat,
                      DataIndex nameIndex);
    size_t BeginBinder(uint64_t timeStamp, uint32_t pid, DataIndex cat, DataIndex nameIndex, ArgsSet args = ArgsSet());
    size_t StartSlice(uint64_t timeStamp,
                      uint32_t pid,
                      DataIndex cat,
                      DataIndex nameIndex,
                      ArgsSet& args,
                      SliceData sliceData = SliceData());
    size_t AsyncBinder(uint64_t timeStamp, uint32_t pid, DataIndex cat, DataIndex nameIndex, ArgsSet& args);
    size_t EndBinder(uint64_t timeStamp,
                     uint32_t pid,
                     DataIndex category = INVALID_UINT64,
                     DataIndex name = INVALID_UINT64,
                     ArgsSet args = {});
    size_t CompleteSlice(uint64_t timeStamp,
                         uint32_t pid,
                         uint32_t threadGroupId,
                         DataIndex category = INVALID_UINT64,
                         DataIndex name = INVALID_UINT64,
                         ArgsSet args = {});
    size_t EndSlice(uint64_t timeStamp,
                    uint32_t pid,
                    uint32_t threadGroupId,
                    DataIndex category = INVALID_UINT64,
                    DataIndex name = INVALID_UINT64);
    uint64_t
        StartAsyncSlice(uint64_t timeStamp, uint32_t pid, uint32_t threadGroupId, uint64_t cookie, DataIndex nameIndex);
    uint64_t FinishAsyncSlice(uint64_t timeStamp,
                              uint32_t pid,
                              uint32_t threadGroupId,
                              uint64_t cookie,
                              DataIndex nameIndex);
    void IrqHandlerEntry(uint64_t timeStamp, uint32_t cpu, DataIndex catalog, DataIndex nameIndex);
    std::tuple<uint64_t, uint32_t> AddArgs(uint32_t tid, DataIndex key1, DataIndex key2, ArgsSet& args);
    void IrqHandlerExit(uint64_t timeStamp, uint32_t cpu, ArgsSet args);
    void IpiHandlerEntry(uint64_t timeStamp, uint32_t cpu, DataIndex catalog, DataIndex nameIndex);
    void IpiHandlerExit(uint64_t timeStamp, uint32_t cpu);
    void SoftIrqEntry(uint64_t timeStamp, uint32_t cpu, DataIndex catalog, DataIndex nameIndex);
    void SoftIrqExit(uint64_t timeStamp, uint32_t cpu, ArgsSet args);
    void Clear();

private:
    struct StackInfo {
        bool isAsyncEvent = false;
        uint32_t asyncEventCount = 0;
        uint32_t asyncEventLastBeginTs = 0;
        std::vector<SliceData> sliceStack;
    };
    using SlicesStack = std::vector<SliceData>;
    using StackOfSlices = StackInfo;
    using StackOnDepth = std::map<uint32_t, bool>;
    void RememberSliceData(InternalTid internalTid,
                           std::unordered_map<InternalTid, StackOfSlices>& stackMap,
                           SliceData& slice,
                           uint32_t depth,
                           uint64_t index);
    uint8_t UpdateDepth(bool increase, InternalTid internalTid, int32_t depth = -1);
    void CloseUnMatchedSlice(int64_t ts, SlicesStack& stack, InternalTid itid);
    int32_t MatchingIncompleteSliceIndex(const SlicesStack& stack, DataIndex category, DataIndex name);
    uint8_t CurrentDepth(InternalTid internalTid);

private:
    // The parameter list is tid, cookid, functionName, asyncCallId.
    TripleMap<uint32_t, uint64_t, DataIndex, uint64_t> asyncEventMap_;
    // this is only used to calc the layer of the async event in same time range
    std::map<uint32_t, int8_t> asyncNoEndingEventMap_ = {};
    //  irq map, key1 is cpu, key2
    struct IrqRecords {
        uint64_t ts;
        size_t row;
    };
    std::unordered_map<uint32_t, IrqRecords> irqEventMap_ = {};
    std::unordered_map<uint32_t, IrqRecords> ipiEventMap_ = {};
    //  irq map, key1 is cpu, key2
    std::unordered_map<uint32_t, IrqRecords> softIrqEventMap_ = {};
    std::map<uint64_t, AsyncEvent> asyncEventFilterMap_ = {};
    std::unordered_map<InternalTid, StackOfSlices> sliceStackMap_ = {};
    std::unordered_map<InternalTid, StackOfSlices>& binderStackMap_ = sliceStackMap_;
    std::unordered_map<InternalTid, StackOnDepth> depthHolder_ = {};
    std::unordered_map<uint32_t, uint32_t> pidTothreadGroupId_ = {};
    uint64_t asyncEventSize_ = 0;
    uint64_t asyncEventDisMatchCount = 0;
    uint64_t callEventDisMatchCount = 0;
    std::unordered_map<uint32_t, uint32_t> sliceRowToArgsSetId_ = {};
    std::unordered_map<uint32_t, uint32_t> argsSetIdToSliceRow_ = {};
    std::unordered_map<uint32_t, uint32_t> tidToArgsSetId_ = {};
    struct SliceInfo {
        uint32_t row;
        ArgsSet args_tracker;
    };
    std::unordered_map<FilterId, std::vector<SliceInfo>> argsSet_ = {};
    DataIndex asyncBeginCountId_ = traceDataCache_->GetDataIndex("legacy_unnestable_begin_count");
    DataIndex asyncBeginTsId_ = traceDataCache_->GetDataIndex("legacy_unnestable_last_begin_ts");
    DataIndex ipiId_ = traceDataCache_->GetDataIndex("IPI");
    std::map<uint32_t, uint32_t> irqDataLinker_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // SLICE_FILTER_H
