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
#include "f2fs.h"
#include "logging.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    f2fs_sync_file_enter,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_f2fs_sync_file_enter_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.f2fs_sync_file_enter_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "f2fs_sync_file_enter: dev = (%d,%d), ino = %lu, pino = %lu, i_mode = 0x%x, i_size = %" PRIu64
            ", i_nlink = %u, i_blocks = %" PRIu64 ", i_advise = 0x%x",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
            (unsigned long)msg.ino(), (unsigned long)msg.pino(), msg.mode(), msg.size(), (unsigned int)msg.nlink(),
            msg.blocks(), (unsigned char)msg.advise());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(f2fs_sync_file_enter) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    f2fs_sync_file_exit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_f2fs_sync_file_exit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.f2fs_sync_file_exit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "f2fs_sync_file_exit: dev = (%d,%d), ino = %lu, cp_reason: %s, datasync = %d, ret = %d",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
            (unsigned long)msg.ino(),
            __print_symbolic(msg.cp_reason(), {CP_NO_NEEDED, "no needed"}, {CP_NON_REGULAR, "non regular"},
                {CP_COMPRESSED, "compressed"}, {CP_HARDLINK, "hardlink"}, {CP_SB_NEED_CP, "sb needs cp"},
                {CP_WRONG_PINO, "wrong pino"}, {CP_NO_SPC_ROLL, "no space roll forward"},
                {CP_NODE_NEED_CP, "node needs cp"}, {CP_FASTBOOT_MODE, "fastboot mode"},
                {CP_SPEC_LOG_NUM, "log type is 2"}, {CP_RECOVER_DIR, "dir needs recovery"}),
            msg.datasync(), msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(f2fs_sync_file_exit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    f2fs_write_begin,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_f2fs_write_begin_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.f2fs_write_begin_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "f2fs_write_begin: dev = (%d,%d), ino = %lu, pos = %" PRIu64 ", len = %u, flags = %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
            (unsigned long)msg.ino(), msg.pos(), msg.len(), msg.flags());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(f2fs_write_begin) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    f2fs_write_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_f2fs_write_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.f2fs_write_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "f2fs_write_end: dev = (%d,%d), ino = %lu, pos = %" PRIu64 ", len = %u, copied = %u",
            ((unsigned int)((msg.dev()) >> 20)), ((unsigned int)((msg.dev()) & ((1U << 20) - 1))),
            (unsigned long)msg.ino(), msg.pos(), msg.len(), msg.copied());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(f2fs_write_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
