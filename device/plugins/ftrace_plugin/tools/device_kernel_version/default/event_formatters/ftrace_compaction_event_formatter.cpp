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
#include <cinttypes>

#include "event_formatter.h"
#include "logging.h"
#include "trace_events.h"
#include "type.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_compaction_isolate_freepages,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_compaction_isolate_freepages_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_compaction_isolate_freepages_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_compaction_isolate_freepages: range=(0x%" PRIx64 " ~ 0x%" PRIx64 ") nr_scanned==0x%" PRIu64
            " nr_taken==0x%" PRIu64 "",
            msg.start_pfn(), msg.end_pfn(), msg.nr_scanned(), msg.nr_taken());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(mm_compaction_isolate_freepages) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_compaction_isolate_migratepages,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_compaction_isolate_migratepages_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_compaction_isolate_migratepages_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_compaction_isolate_migratepages: range=(0x%" PRIx64 " ~ 0x%" PRIx64 ") nr_scanned==0x%" PRIu64
            " nr_taken==0x%" PRIu64 "",
            msg.start_pfn(), msg.end_pfn(), msg.nr_scanned(), msg.nr_taken());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(mm_compaction_isolate_migratepages) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
