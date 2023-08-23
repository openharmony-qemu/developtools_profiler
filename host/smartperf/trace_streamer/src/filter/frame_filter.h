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

#ifndef FRAME_FILTER_H
#define FRAME_FILTER_H
#include <vector>
#include "double_map.h"
#include "filter_base.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"
namespace SysTuning {
namespace TraceStreamer {
class FrameFilter : private FilterBase {
public:
    FrameFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    ~FrameFilter() override;
    void BeginVsyncEvent(uint64_t ts,
                         uint32_t ipid,
                         uint32_t itid,
                         uint64_t expectStart,
                         uint64_t expectEnd,
                         uint32_t vsyncId,
                         uint32_t callStackSliceId);
    bool MarkRSOnvsyncEvent(uint64_t ts, uint32_t itid);
    bool EndOnVsyncEvent(uint64_t ts, uint32_t itid);
    bool BeginRSTransactionData(uint64_t ts, uint32_t itid, uint32_t franeNum);
    typedef struct {
        uint32_t sourceItid;
        uint32_t frameNum;
    } FrameMap;
    bool BeginProcessCommandUni(uint64_t ts, uint32_t itid, const std::vector<FrameMap>& frame, uint32_t sliceIndex);
    bool EndVsyncEvent(uint64_t ts, uint32_t itid);
    bool StartFrameQueue(uint64_t ts, uint32_t itid);
    bool EndFrameQueue(uint64_t ts, uint32_t itid);
    void Finish();

private:
    class FrameSlice {
    public:
        FrameSlice() {}
        uint64_t startTs_ = INVALID_UINT64;
        // @deprecated it will be deleted later
        uint64_t expectedStartTs_ = INVALID_UINT64;
        // if a frame experience video lag, is depend on if the real end ts is later then the expected ts, rather then
        // the dur. noted at 2023/4/6
        uint64_t expectedEndTs_ = INVALID_UINT64;
        // @deprecated it will be deleted later
        uint64_t expectedDur_ = INVALID_UINT64;
        uint64_t endTs_ = INVALID_UINT64;
        bool gpuEnd_ = true;
        TraceStdtype::FrameSlice::FrameSliceType frameType_ = TraceStdtype::FrameSlice::ACTURAL_SLICE;
        uint32_t vsyncId_ = INVALID_UINT32;
        uint64_t frameQueueStartTs_ = INVALID_UINT64;
        bool vsyncEnd_ = false;
        bool isRsMainThread_ = false;
        uint32_t frameNum_ = INVALID_UINT32;
        uint64_t callStackSliceId_ = INVALID_UINT64;
        uint64_t frameSliceRow_ = INVALID_UINT64;
        uint64_t frameExpectedSliceRow_ = INVALID_UINT64;
        std::vector<uint64_t> sourceSlice_ = {};
        std::vector<uint64_t> sourceExpectedSlice_ = {};
        uint64_t dstFrameSliceId_ = INVALID_UINT64;
        uint64_t dstExpectedFrameSliceId_ = INVALID_UINT64;
    };
    std::unordered_map<uint32_t /* tid */, std::vector<std::shared_ptr<FrameSlice>>> vsyncRenderSlice_ = {};
    std::unordered_map<uint32_t /* tid */, std::unordered_map<uint32_t /* frameNum */, std::shared_ptr<FrameSlice>>>
        dstRenderSlice_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif
