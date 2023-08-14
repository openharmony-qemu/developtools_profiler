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

#include "binder.h"
#include "event_formatter.h"
#include "logging.h"
#include "trace_events.h"

FTRACE_NS_BEGIN
namespace {
const int BUFFER_SIZE = 512;

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_alloc_lru_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_alloc_lru_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_alloc_lru_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_alloc_lru_end: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_alloc_lru_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_alloc_lru_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_alloc_lru_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_alloc_lru_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_alloc_lru_start: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_alloc_lru_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_alloc_page_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_alloc_page_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_alloc_page_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_alloc_page_end: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_alloc_page_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_alloc_page_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_alloc_page_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_alloc_page_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_alloc_page_start: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_alloc_page_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_command,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_command_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_command_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_command: cmd=0x%x", msg.cmd());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_command) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_free_lru_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_free_lru_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_free_lru_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_free_lru_end: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_free_lru_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_free_lru_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_free_lru_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_free_lru_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_free_lru_start: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_free_lru_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_ioctl,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_ioctl_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_ioctl_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_ioctl: cmd=0x%x arg=0x%" PRIx64 "", msg.cmd(), msg.arg());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_ioctl) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_ioctl_done,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_ioctl_done_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_ioctl_done_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_ioctl_done: ret=%d", msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_ioctl_done) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_lock,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_lock_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_lock_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_lock: tag=%s", msg.tag().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_lock) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_locked,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_locked_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_locked_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_locked: tag=%s", msg.tag().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_locked) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_read_done,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_read_done_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_read_done_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_read_done: ret=%d", msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_read_done) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_return,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_return_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_return_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_return: cmd=0x%x", msg.cmd());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_return) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_transaction,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_transaction_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_transaction_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_transaction: transaction=%d dest_node=%d dest_proc=%d dest_thread=%d reply=%d flags=0x%x code=0x%x",
            msg.debug_id(), msg.target_node(), msg.to_proc(), msg.to_thread(), msg.reply(), msg.flags(), msg.code());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_transaction) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_transaction_alloc_buf,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_transaction_alloc_buf_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_transaction_alloc_buf_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_transaction_alloc_buf: transaction=%d data_size=%" PRIu64 " offsets_size=%" PRIu64
            " extra_buffers_size=%" PRIu64 "",
            msg.debug_id(), msg.data_size(), msg.offsets_size(), msg.extra_buffers_size());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(binder_transaction_alloc_buf) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_transaction_buffer_release,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_transaction_buffer_release_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_transaction_buffer_release_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_transaction_buffer_release: transaction=%d data_size=%" PRIu64 " offsets_size=%" PRIu64
            " extra_buffers_size=%" PRIu64 "",
            msg.debug_id(), msg.data_size(), msg.offsets_size(), msg.extra_buffers_size());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(binder_transaction_buffer_release) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_transaction_failed_buffer_release,
    [](const ForStandard::FtraceEvent& event) -> bool {
        return event.has_binder_transaction_failed_buffer_release_format();
    },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_transaction_failed_buffer_release_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_transaction_failed_buffer_release: transaction=%d data_size=%" PRIu64 " offsets_size=%" PRIu64
            " extra_buffers_size=%" PRIu64 "",
            msg.debug_id(), msg.data_size(), msg.offsets_size(), msg.extra_buffers_size());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(binder_transaction_failed_buffer_release) msg had be cut off in "
                "outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_transaction_node_to_ref,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_transaction_node_to_ref_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_transaction_node_to_ref_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_transaction_node_to_ref: transaction=%d node=%d src_ptr=0x%016llx ==> dest_ref=%d dest_desc=%d",
            msg.debug_id(), msg.node_debug_id(), (u64)msg.node_ptr(), msg.ref_debug_id(), msg.ref_desc());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(binder_transaction_node_to_ref) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_transaction_received,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_transaction_received_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_transaction_received_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_transaction_received: transaction=%d", msg.debug_id());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(binder_transaction_received) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_transaction_ref_to_node,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_transaction_ref_to_node_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_transaction_ref_to_node_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_transaction_ref_to_node: transaction=%d node=%d src_ref=%d src_desc=%d ==> dest_ptr=0x%016llx",
            msg.debug_id(), msg.node_debug_id(), msg.ref_debug_id(), msg.ref_desc(), (u64)msg.node_ptr());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(binder_transaction_ref_to_node) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_transaction_ref_to_ref,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_transaction_ref_to_ref_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_transaction_ref_to_ref_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_transaction_ref_to_ref: transaction=%d node=%d src_ref=%d src_desc=%d ==> dest_ref=%d dest_desc=%d",
            msg.debug_id(), msg.node_debug_id(), msg.src_ref_debug_id(), msg.src_ref_desc(), msg.dest_ref_debug_id(),
            msg.dest_ref_desc());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(binder_transaction_ref_to_ref) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_unlock,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_unlock_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_unlock_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_unlock: tag=%s", msg.tag().c_str());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_unlock) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_unmap_kernel_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_unmap_kernel_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_unmap_kernel_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_unmap_kernel_end: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_unmap_kernel_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_unmap_kernel_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_unmap_kernel_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_unmap_kernel_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_unmap_kernel_start: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE,
                "maybe, the contents of print event(binder_unmap_kernel_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_unmap_user_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_unmap_user_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_unmap_user_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_unmap_user_end: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_unmap_user_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_unmap_user_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_unmap_user_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_unmap_user_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_unmap_user_start: proc=%d page_index=%" PRIu64 "", msg.proc(), msg.page_index());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_unmap_user_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_update_page_range,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_update_page_range_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_update_page_range_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_update_page_range: proc=%d allocate=%d offset=%" PRIu64 " size=%" PRIu64 "", msg.proc(),
            msg.allocate(), msg.offset(), msg.size());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_update_page_range) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_wait_for_work,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_wait_for_work_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_wait_for_work_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "binder_wait_for_work: proc_work=%d transaction_stack=%d thread_todo=%d", msg.proc_work(),
            msg.transaction_stack(), msg.thread_todo());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(binder_wait_for_work) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    binder_write_done,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_binder_write_done_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.binder_write_done_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "binder_write_done: ret=%d", msg.ret());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(binder_write_done) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
