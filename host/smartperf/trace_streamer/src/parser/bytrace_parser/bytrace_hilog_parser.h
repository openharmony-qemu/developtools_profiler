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
#ifndef BYTRACE_HILOG_PARSER_H
#define BYTRACE_HILOG_PARSER_H

#include "common_types.h"
#include "string_help.h"
#include "log.h"
#include "string_to_numerical.h"
#include "parser_base.h"
#include "event_parser_base.h"
#include "trace_data_cache.h"

namespace SysTuning {
namespace TraceStreamer {
class BytraceHilogParser : public EventParserBase {
public:
    BytraceHilogParser(TraceDataCache* dataCache, const TraceStreamerFilters* filters);
    ~BytraceHilogParser();
    void ParseHilogDataItem(const std::string& buffer, const uint64_t lineSeq);
    void FilterAllHilogData();

private:
    bool HilogTimeStrToTimestamp(std::string& timeStr, uint64_t& timeStamp) const;
    void FilterHilogData(std::unique_ptr<HilogLine> bufLine);
    void BeginFilterHilogData(HilogLine* hilogData);

    const std::regex hilogMatcher_ = std::regex(R"( *(\w+ )?([\-\d: ]+\.\d+) +(\d+) +(\d+) +([FEWID]) +(.+?): +(.+))");
    enum HILOG_MATCH_SEQ {
        HILOG_MATCH_SEQ_ZONE = 1,
        HILOG_MATCH_SEQ_TIME = 2,
        HILOG_MATCH_SEQ_PID = 3,
        HILOG_MATCH_SEQ_TID = 4,
        HILOG_MATCH_SEQ_LEVEL = 5,
        HILOG_MATCH_SEQ_TAG = 6,
        HILOG_MATCH_SEQ_CONTENT = 7,
    };

    std::vector<std::unique_ptr<HilogLine>> hilogList_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // BYTRACE_HILOG_PARSER_H
