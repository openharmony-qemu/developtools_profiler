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
#ifndef HTRACE_PLUGIN_TIME_PARSER_H
#define HTRACE_PLUGIN_TIME_PARSER_H
#include "ts_common.h"
namespace SysTuning {
namespace TraceStreamer {
class HtracePluginTimeParser {
public:
    HtracePluginTimeParser();
    HtracePluginTimeParser(const HtracePluginTimeParser&) = delete;
    HtracePluginTimeParser& operator=(const HtracePluginTimeParser&) = delete;
    ~HtracePluginTimeParser() = default;
    void UpdatePluginTimeRange(ClockId clockId, uint64_t asyncTimestamp, uint64_t syncTimestamp);
    uint64_t GetPluginStartTime() const;
    uint64_t GetPluginEndTime() const;
    uint64_t MinTs() const
    {
        return minTs_;
    }
    uint64_t MaxTs() const
    {
        return maxTs_;
    }

private:
    uint64_t syncHtracePluginStartTime_ = std::numeric_limits<uint64_t>::max();
    uint64_t syncHtracePluginEndTime_ = 0;
    uint64_t asyncHtracePluginStartTime_ = std::numeric_limits<uint64_t>::max();
    uint64_t asyncHtracePluginEndTime_ = 0;
    uint64_t minTs_ = std::numeric_limits<uint64_t>::max();
    uint64_t maxTs_ = 0;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_PLUGIN_TIME_PARSER_H
