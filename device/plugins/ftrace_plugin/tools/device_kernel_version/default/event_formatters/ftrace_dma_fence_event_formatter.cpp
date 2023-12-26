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
    dma_fence_destroy,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dma_fence_destroy_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dma_fence_destroy_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dma_fence_destroy: driver=%s timeline=%s context=%u seqno=%u", msg.driver().c_str(),
            msg.timeline().c_str(), msg.context(), msg.seqno());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(dma_fence_destroy) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dma_fence_emit,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dma_fence_emit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dma_fence_emit_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dma_fence_emit: driver=%s timeline=%s context=%u seqno=%u", msg.driver().c_str(), msg.timeline().c_str(),
            msg.context(), msg.seqno());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(dma_fence_emit) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dma_fence_enable_signal,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dma_fence_enable_signal_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dma_fence_enable_signal_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dma_fence_enable_signal: driver=%s timeline=%s context=%u seqno=%u", msg.driver().c_str(),
            msg.timeline().c_str(), msg.context(), msg.seqno());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(dma_fence_enable_signal) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dma_fence_init,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dma_fence_init_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dma_fence_init_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dma_fence_init: driver=%s timeline=%s context=%u seqno=%u", msg.driver().c_str(), msg.timeline().c_str(),
            msg.context(), msg.seqno());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(LOG_CORE, "maybe, the contents of print event(dma_fence_init) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dma_fence_signaled,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dma_fence_signaled_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dma_fence_signaled_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dma_fence_signaled: driver=%s timeline=%s context=%u seqno=%u", msg.driver().c_str(),
            msg.timeline().c_str(), msg.context(), msg.seqno());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(dma_fence_signaled) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dma_fence_wait_end,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dma_fence_wait_end_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dma_fence_wait_end_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dma_fence_wait_end: driver=%s timeline=%s context=%u seqno=%u", msg.driver().c_str(),
            msg.timeline().c_str(), msg.context(), msg.seqno());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(dma_fence_wait_end) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    dma_fence_wait_start,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_dma_fence_wait_start_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.dma_fence_wait_start_format();
        char buffer[BUFFER_SIZE];
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "dma_fence_wait_start: driver=%s timeline=%s context=%u seqno=%u", msg.driver().c_str(),
            msg.timeline().c_str(), msg.context(), msg.seqno());
        if (len >= BUFFER_SIZE - 1) {
            HILOG_WARN(
                LOG_CORE, "maybe, the contents of print event(dma_fence_wait_start) msg had be cut off in outfile");
        }
        return std::string(buffer);
    });
} // namespace
FTRACE_NS_END
