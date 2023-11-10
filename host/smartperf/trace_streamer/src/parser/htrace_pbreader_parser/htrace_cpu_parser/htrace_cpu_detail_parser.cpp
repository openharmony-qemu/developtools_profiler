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
#include "htrace_cpu_detail_parser.h"
#include "htrace_event_parser.h"
#include "stat_filter.h"
#include "trace_plugin_result.pbreader.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceCpuDetailParser::HtraceCpuDetailParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : eventParser_(std::make_unique<HtraceEventParser>(dataCache, ctx))
{
}

HtraceCpuDetailParser::~HtraceCpuDetailParser() = default;
void HtraceCpuDetailParser::Parse(HtraceDataSegment& tracePacket, BuiltinClocks clock, bool& haveSplitSeg)
{
    eventParser_->ParseDataItem(tracePacket, clock, haveSplitSeg);
}
void HtraceCpuDetailParser::FilterAllEvents()
{
    eventParser_->FilterAllEvents();
    eventParser_->Clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
