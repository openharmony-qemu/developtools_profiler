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

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
template <typename T>
void mm_compaction_isolate_freepages_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_start_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_end_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_scanned(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_taken(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_isolate_freepages, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                         size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_freepages_format();
    mm_compaction_isolate_freepages_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_isolate_freepages, [](ProtoEncoder::FtraceEvent &ftraceEvent,
                                                                                 uint8_t data[], size_t size,
                                                                                 const EventFormat &format) {
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_freepages_format();
    mm_compaction_isolate_freepages_func(msg, data, size, format);
});

template <typename T>
void mm_compaction_isolate_migratepages_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_start_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_end_pfn(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_scanned(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_nr_taken(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(mm_compaction_isolate_migratepages, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                            size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_migratepages_format();
    mm_compaction_isolate_migratepages_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(mm_compaction_isolate_migratepages,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
    auto msg = ftraceEvent.mutable_mm_compaction_isolate_migratepages_format();
    mm_compaction_isolate_migratepages_func(msg, data, size, format);
});
}  // namespace
FTRACE_NS_END
