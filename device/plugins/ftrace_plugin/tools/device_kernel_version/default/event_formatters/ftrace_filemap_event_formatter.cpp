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
    file_check_and_advance_wb_err,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_file_check_and_advance_wb_err_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.file_check_and_advance_wb_err_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "file_check_and_advance_wb_err: file=%p dev=%d:%d ino=0x%" PRIx64 " old=0x%x new=0x%x", msg.file(),
            ((unsigned int)((msg.s_dev()) >> 20)), ((unsigned int)((msg.s_dev()) & ((1U << 20) - 1))), msg.i_ino(),
            msg.old(), msg.seq_new());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(file_check_and_advance_wb_err) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    filemap_set_wb_err,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_filemap_set_wb_err_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.filemap_set_wb_err_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "filemap_set_wb_err: dev=%d:%d ino=0x%" PRIx64 " errseq=0x%x", ((unsigned int)((msg.s_dev()) >> 20)),
            ((unsigned int)((msg.s_dev()) & ((1U << 20) - 1))), msg.i_ino(), msg.errseq());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(filemap_set_wb_err) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_filemap_add_to_page_cache,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_filemap_add_to_page_cache_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_filemap_add_to_page_cache_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_filemap_add_to_page_cache: dev %" PRIu64 ":%" PRIu64 " ino %p page=%s pfn=%" PRIu64 " ofs=%" PRIu64 "",
            (((msg.s_dev()) >> 20)), (((msg.s_dev()) & ((1U << 20) - 1))), msg.i_ino(), "0000000000000000", msg.pfn(),
            msg.index() << 12);
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(mm_filemap_add_to_page_cache) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    mm_filemap_delete_from_page_cache,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_mm_filemap_delete_from_page_cache_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.mm_filemap_delete_from_page_cache_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "mm_filemap_delete_from_page_cache: dev %" PRIu64 ":%" PRIu64 " ino %p page=%s pfn=%" PRIu64 " ofs=%" PRIu64
            "",
            (((msg.s_dev()) >> 20)), (((msg.s_dev()) & ((1U << 20) - 1))), msg.i_ino(), "0000000000000000", msg.pfn(),
            msg.index() << 12);
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(mm_filemap_delete_from_page_cache) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
