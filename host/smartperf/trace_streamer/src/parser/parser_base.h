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

#ifndef SRC_PARSER_BASE_H
#define SRC_PARSER_BASE_H
#include <deque>
#include <memory>
#include "trace_streamer_filters.h"
#include "ts_common.h"
namespace SysTuning {
namespace TraceStreamer {
class ParserBase {
public:
    explicit ParserBase(const TraceStreamerFilters* filter);
    virtual ~ParserBase() = default;
    virtual void ParseTraceDataSegment(std::unique_ptr<uint8_t[]>, size_t size, bool isFinish = false) = 0;

protected:
    virtual void ParseTraceDataItem(const std::string& buffer) = 0;
    std::deque<uint8_t> packagesBuffer_ = {};
    const TraceStreamerFilters* streamFilters_;
    BuiltinClocks clock_;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif
