/* THIS FILE IS GENERATE BY ftrace_cpp_generator.py, PLEASE DON'T EDIT IT!
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include "sub_event_parser.h"

#include <string>
#include <vector>

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
struct MmLruActivateData {
    uint64_t page {};
    uint64_t pfn {};
};
__attribute__((noinline)) void ParseMmLruActivateData(MmLruActivateData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.page = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmLruActivate(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    MmLruActivateData eventData {};
    ParseMmLruActivateData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_lru_activate_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_page(eventData.page);
    msg->set_pfn(eventData.pfn);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_lru_activate, ParseMmLruActivate<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_lru_activate, ParseMmLruActivate<ProtoEncoder::FtraceEvent>);

struct MmLruInsertionData {
    uint64_t page {};
    uint64_t pfn {};
    int32_t lru {};
    uint64_t flags {};
};
__attribute__((noinline)) void ParseMmLruInsertionData(MmLruInsertionData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    if (data == nullptr || size == 0 || format.fields.empty()) {
        return;
    }
    int i = 0;
    eventData.page = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pfn = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.lru = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseMmLruInsertion(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        return;
    }
    MmLruInsertionData eventData {};
    ParseMmLruInsertionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_mm_lru_insertion_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_page(eventData.page);
    msg->set_pfn(eventData.pfn);
    msg->set_lru(eventData.lru);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_lru_insertion, ParseMmLruInsertion<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_lru_insertion, ParseMmLruInsertion<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
