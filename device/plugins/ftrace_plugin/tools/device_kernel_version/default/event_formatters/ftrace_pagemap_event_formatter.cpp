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

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_lru_activate,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_lru_activate_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_lru_activate_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "mm_lru_activate: page=%p pfn=%" PRIu64 "", msg.page(), msg.pfn());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(mm_lru_activate) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_lru_insertion,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_lru_insertion_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_lru_insertion_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_lru_insertion: page=%p pfn=%" PRIu64 " lru=%d flags=%s%s%s%s%s%s", msg.page(), msg.pfn(), msg.lru(),
            msg.flags() & 0x0001u ? "M" : " ", msg.flags() & 0x0002u ? "a" : "f", msg.flags() & 0x0008u ? "s" : " ",
            msg.flags() & 0x0010u ? "b" : " ", msg.flags() & 0x0020u ? "d" : " ", msg.flags() & 0x0040u ? "B" : " ");
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(mm_lru_insertion) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
