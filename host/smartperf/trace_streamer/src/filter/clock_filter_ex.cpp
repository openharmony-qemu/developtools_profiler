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
#include "clock_filter_ex.h"
#include <algorithm>
#include <map>
#include <vector>
#include "filter_base.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"
#include "string_help.h"
namespace SysTuning {
namespace TraceStreamer {
ClockFilterEx::ClockFilterEx(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter), dataCache_(dataCache)
{
}
ClockFilterEx::~ClockFilterEx() {}

void ClockFilterEx::AddClockSnapshot(const std::vector<SnapShot>& snapShot)
{
    ClockId srcId, desId;
    const int32_t theDataBeforeLast = 2;
    for (srcId = 0; srcId < snapShot.size() - 1; ++srcId) {
        ClockId srcClockId = snapShot[srcId].clockId;
        uint64_t srcTs = snapShot[srcId].ts;
        (void)traceDataCache_->GetClockSnapshotData()->AppendNewSnapshot(
            srcClockId, srcTs,
            dataCache_->GetConstStatAndInfo().clockid2ClockNameMap_.at(static_cast<BuiltinClocks>(srcClockId)));
        for (desId = srcId + 1; desId < snapShot.size(); ++desId) {
            ClockId desClockId = snapShot[desId].clockId;
            uint64_t desTs = snapShot[desId].ts;
            if ((srcId == snapShot.size() - theDataBeforeLast) and (desId == snapShot.size() - 1)) {
                traceDataCache_->GetClockSnapshotData()->AppendNewSnapshot(
                    desClockId, desTs,
                    dataCache_->GetConstStatAndInfo().clockid2ClockNameMap_.at(static_cast<BuiltinClocks>(desClockId)));
            }
            AddConvertClockMap(srcClockId, desClockId, srcTs, desTs);
            AddConvertClockMap(desClockId, srcClockId, desTs, srcTs);
        }
    }
    hasInitSnapShot_ = true;
}
} // namespace TraceStreamer
} // namespace SysTuning
