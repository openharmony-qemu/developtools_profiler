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

#ifndef ANIMATION_FILTER_H
#define ANIMATION_FILTER_H

#include <unordered_map>
#include "common_types.h"
#include "filter_base.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class AnimationFilter : private FilterBase {
public:
    AnimationFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    ~AnimationFilter() override;
    bool UpdateDeviceInfoEvent(const TracePoint& point, const BytraceLine& line);
    bool BeginDynamicFrameEvent(const TracePoint& point, size_t callStackRow);
    void StartAnimationEvent(const BytraceLine& line, size_t callStackRow);
    bool FinishAnimationEvent(const BytraceLine& line, size_t callStackRow);
    void UpdateDynamicFrameInfo();
    void Clear();

private:
    bool UpdateDeviceFps(const BytraceLine& line);
    bool UpdateDeviceScreenSize(const TracePoint& point);
    bool UpdateDynamicEndTime(const uint64_t curFrameRow, uint64_t curStackRow);
    const std::string generateVsyncCmd_ = "H:GenerateVsyncCount";
    const std::string leashWindowCmd_ = "H:RSUniRender::Process:[leashWindow";
    const std::string rsUniProcessCmd_ = "H:RSUniRender::Process:[";
    const DataIndex rsDoCompCmd_ = traceDataCache_->GetDataIndex("H:RSMainThread::DoComposition");
    const DataIndex entryViewCmd_ = traceDataCache_->GetDataIndex("H:RSUniRender::Process:[EntryView]");
    // for update dynamicFrameInfo at the end, first is callStackRow, second is dynamicFramRow
    std::map<uint64_t, uint64_t> callStackRowMap_ = {};
    // for update animationInfo, first is callStackRow, second is animationRow
    std::unordered_map<uint64_t, uint64_t> animationCallIds_ = {};
    uint64_t generateFirstTime_ = INVALID_UINT64;
    uint8_t generateVsyncCnt_ = 0;
    DynamicFrame* dynamicFrame_ = nullptr;
    CallStack* callStackSlice_ = nullptr;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // ANIMATION_FILTER_H
