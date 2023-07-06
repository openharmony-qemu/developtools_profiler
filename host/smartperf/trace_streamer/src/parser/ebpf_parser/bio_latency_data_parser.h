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
#ifndef BIO_LATENCY_DATA_PARSER_H
#define BIO_LATENCY_DATA_PARSER_H
#include "clock_filter_ex.h"
#include "double_map.h"
#include "ebpf_base.h"
#include "htrace_plugin_time_parser.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace SysTuning::EbpfStdtype;
class BioLatencyDataParser : virtual public EbpfBase {
public:
    BioLatencyDataParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~BioLatencyDataParser();
    void ParseBioLatencyEvent();

protected:
    std::unique_ptr<HtracePluginTimeParser> timeParser_;

private:
    uint64_t currentCallId_ = 0;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // BIO_LATENCY_DATA_PARSER_H
