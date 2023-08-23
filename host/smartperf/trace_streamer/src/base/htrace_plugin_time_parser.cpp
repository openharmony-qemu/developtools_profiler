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
#include "htrace_plugin_time_parser.h"
#include "clock_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtracePluginTimeParser::HtracePluginTimeParser() {}
void HtracePluginTimeParser::UpdatePluginTimeRange(ClockId clockId, uint64_t asyncTimestamp, uint64_t syncTimestamp)
{
    minTs_ = std::min(minTs_, asyncTimestamp);
    maxTs_ = std::max(maxTs_, asyncTimestamp);
    if (clockId == g_primaryClockId) {
        syncHtracePluginStartTime_ = std::min(syncHtracePluginStartTime_, syncTimestamp);
        syncHtracePluginEndTime_ = std::max(syncHtracePluginEndTime_, syncTimestamp);
        return;
    }
    if (syncTimestamp != asyncTimestamp) {
        syncHtracePluginStartTime_ = std::min(syncHtracePluginStartTime_, syncTimestamp);
        syncHtracePluginEndTime_ = std::max(syncHtracePluginEndTime_, syncTimestamp);
    } else {
        asyncHtracePluginStartTime_ = std::min(asyncHtracePluginStartTime_, syncTimestamp);
        asyncHtracePluginEndTime_ = std::max(asyncHtracePluginEndTime_, syncTimestamp);
    }
}
uint64_t HtracePluginTimeParser::GetPluginStartTime() const
{
    if (syncHtracePluginStartTime_ != std::numeric_limits<uint64_t>::max()) {
        return syncHtracePluginStartTime_;
    } else if (asyncHtracePluginStartTime_ != std::numeric_limits<uint64_t>::max()) {
        return asyncHtracePluginStartTime_;
    }
    return std::numeric_limits<uint64_t>::max();
}

uint64_t HtracePluginTimeParser::GetPluginEndTime() const
{
    if (syncHtracePluginEndTime_ != 0) {
        return syncHtracePluginEndTime_;
    } else if (asyncHtracePluginEndTime_ != 0) {
        return asyncHtracePluginEndTime_;
    }
    return 0;
}
} // namespace TraceStreamer
} // namespace SysTuning
