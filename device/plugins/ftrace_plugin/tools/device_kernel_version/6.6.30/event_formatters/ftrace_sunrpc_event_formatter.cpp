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
    rpc_call_status, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_call_status_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_call_status_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "rpc_call_status: task:%08x@%08x status=%d",
                             msg.task_id(), msg.client_id(), msg.status());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_call_status) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_call_status) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_connect_status,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_connect_status_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_connect_status_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "rpc_connect_status: task:%08x@%08x status=%d",
                             msg.task_id(), msg.client_id(), msg.status());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_connect_status) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_connect_status) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_socket_close, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_socket_close_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_socket_close_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                             "rpc_socket_close: socket:[%" PRIu64
                        "] srcaddr=%pISpc dstaddr=%pISpc state=%u (%s) sk_state=%u (%s)",
                             msg.ino(), msg.saddr(), msg.daddr(), msg.socket_state(),
                             __print_symbolic(msg.socket_state(), {0, "FREE"}, {1, "UNCONNECTED"}, {2, "CONNECTING"},
                                              {3, "CONNECTED"}, {4, "DISCONNECTING"}),
                             msg.sock_state(),
                             __print_symbolic(msg.sock_state(), {1, "ESTABLISHED"}, {2, "SYN_SENT"}, {3, "SYN_RECV"},
                                              {4, "FIN_WAIT1"}, {5, "FIN_WAIT2"}, {6, "TIME_WAIT"}, {7, "CLOSE"},
                                              {8, "CLOSE_WAIT"}, {9, "LAST_ACK"}, {10, "LISTEN"}, {11, "CLOSING"}));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_socket_close) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_socket_close) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_socket_connect,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_socket_connect_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_socket_connect_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                             "rpc_socket_connect: error=%d socket:[%" PRIu64
                        "] srcaddr=%pISpc dstaddr=%pISpc state=%u (%s) sk_state=%u (%s)",
                             msg.error(), msg.ino(), msg.saddr(), msg.daddr(), msg.socket_state(),
                             __print_symbolic(msg.socket_state(), {0, "FREE"}, {1, "UNCONNECTED"}, {2, "CONNECTING"},
                                              {3, "CONNECTED"}, {4, "DISCONNECTING"}),
                             msg.sock_state(),
                             __print_symbolic(msg.sock_state(), {1, "ESTABLISHED"}, {2, "SYN_SENT"}, {3, "SYN_RECV"},
                                              {4, "FIN_WAIT1"}, {5, "FIN_WAIT2"}, {6, "TIME_WAIT"}, {7, "CLOSE"},
                                              {8, "CLOSE_WAIT"}, {9, "LAST_ACK"}, {10, "LISTEN"}, {11, "CLOSING"}));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_socket_connect) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_socket_connect) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_socket_error, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_socket_error_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_socket_error_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                             "rpc_socket_error: error=%d socket:[%" PRIu64
                        "] srcaddr=%pISpc dstaddr=%pISpc state=%u (%s) sk_state=%u (%s)",
                             msg.error(), msg.ino(), msg.saddr(), msg.daddr(), msg.socket_state(),
                             __print_symbolic(msg.socket_state(), {0, "FREE"}, {1, "UNCONNECTED"}, {2, "CONNECTING"},
                                              {3, "CONNECTED"}, {4, "DISCONNECTING"}),
                             msg.sock_state(),
                             __print_symbolic(msg.sock_state(), {1, "ESTABLISHED"}, {2, "SYN_SENT"}, {3, "SYN_RECV"},
                                              {4, "FIN_WAIT1"}, {5, "FIN_WAIT2"}, {6, "TIME_WAIT"}, {7, "CLOSE"},
                                              {8, "CLOSE_WAIT"}, {9, "LAST_ACK"}, {10, "LISTEN"}, {11, "CLOSING"}));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_socket_error) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_socket_error) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_socket_reset_connection,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_socket_reset_connection_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_socket_reset_connection_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                             "rpc_socket_reset_connection: error=%d socket:[%" PRIu64
                        "] srcaddr=%pISpc dstaddr=%pISpc state=%u (%s) sk_state=%u (%s)",
                             msg.error(), msg.ino(), msg.saddr(), msg.daddr(), msg.socket_state(),
                             __print_symbolic(msg.socket_state(), {0, "FREE"}, {1, "UNCONNECTED"}, {2, "CONNECTING"},
                                              {3, "CONNECTED"}, {4, "DISCONNECTING"}),
                             msg.sock_state(),
                             __print_symbolic(msg.sock_state(), {1, "ESTABLISHED"}, {2, "SYN_SENT"}, {3, "SYN_RECV"},
                                              {4, "FIN_WAIT1"}, {5, "FIN_WAIT2"}, {6, "TIME_WAIT"}, {7, "CLOSE"},
                                              {8, "CLOSE_WAIT"}, {9, "LAST_ACK"}, {10, "LISTEN"}, {11, "CLOSING"}));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(
                LOG_CORE,
                "maybe, the contents of print event(rpc_socket_reset_connection) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "the contents of event(rpc_socket_reset_connection) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_socket_shutdown,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_socket_shutdown_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_socket_shutdown_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                             "rpc_socket_shutdown: socket:[%" PRIu64
                        "] srcaddr=%pISpc dstaddr=%pISpc state=%u (%s) sk_state=%u (%s)",
                             msg.ino(), msg.saddr(), msg.daddr(), msg.socket_state(),
                             __print_symbolic(msg.socket_state(), {0, "FREE"}, {1, "UNCONNECTED"}, {2, "CONNECTING"},
                                              {3, "CONNECTED"}, {4, "DISCONNECTING"}),
                             msg.sock_state(),
                             __print_symbolic(msg.sock_state(), {1, "ESTABLISHED"}, {2, "SYN_SENT"}, {3, "SYN_RECV"},
                                              {4, "FIN_WAIT1"}, {5, "FIN_WAIT2"}, {6, "TIME_WAIT"}, {7, "CLOSE"},
                                              {8, "CLOSE_WAIT"}, {9, "LAST_ACK"}, {10, "LISTEN"}, {11, "CLOSING"}));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_socket_shutdown) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_socket_shutdown) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_socket_state_change,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_socket_state_change_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_socket_state_change_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                             "rpc_socket_state_change: socket:[%" PRIu64
                        "] srcaddr=%pISpc dstaddr=%pISpc state=%u (%s) sk_state=%u (%s)",
                             msg.ino(), msg.saddr(), msg.daddr(), msg.socket_state(),
                             __print_symbolic(msg.socket_state(), {0, "FREE"}, {1, "UNCONNECTED"}, {2, "CONNECTING"},
                                              {3, "CONNECTED"}, {4, "DISCONNECTING"}),
                             msg.sock_state(),
                             __print_symbolic(msg.sock_state(), {1, "ESTABLISHED"}, {2, "SYN_SENT"}, {3, "SYN_RECV"},
                                              {4, "FIN_WAIT1"}, {5, "FIN_WAIT2"}, {6, "TIME_WAIT"}, {7, "CLOSE"},
                                              {8, "CLOSE_WAIT"}, {9, "LAST_ACK"}, {10, "LISTEN"}, {11, "CLOSING"}));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(
                LOG_CORE, "maybe, the contents of print event(rpc_socket_state_change) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_socket_state_change) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_task_begin, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_task_begin_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_task_begin_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.action()) > 0) {
            functionStr = kernelSymbols[msg.action()];
        }
        if (functionStr != "") {
            len = snprintf_s(
                buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "rpc_task_begin: task:%08x@%08x flags=%s runstate=%s status=%d action=%s", msg.task_id(),
                msg.client_id(),
                __print_flags(msg.flags(), "|", {0x0001, "ASYNC"}, {0x0002, "SWAPPER"}, {0x0004, "MOVEABLE"},
                              {0x0010, "NULLCREDS"}, {0x0020, "MAJORSEEN"}, {0x0080, "DYNAMIC"},
                              {0x0100, "NO_ROUND_ROBIN"}, {0x0200, "SOFT"}, {0x0400, "SOFTCONN"}, {0x0800, "SENT"},
                              {0x1000, "TIMEOUT"}, {0x2000, "NOCONNECT"}, {0x4000, "NORTO"}, {0x8000, "CRED_NOREF"}),
                __print_flags(msg.runstate(), "|", {(1UL << 0), "RUNNING"}, {(1UL << 1), "QUEUED"},
                              {(1UL << 2), "ACTIVE"}, {(1UL << 3), "NEED_XMIT"}, {(1UL << 4), "NEED_RECV"},
                              {(1UL << 5), "MSG_PIN_WAIT"}, {(1UL << 6), "SIGNALLED"}),
                msg.status(), functionStr.c_str());
        } else {
            len = snprintf_s(
                buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "rpc_task_begin: task:%08x@%08x flags=%s runstate=%s status=%d action=%p", msg.task_id(),
                msg.client_id(),
                __print_flags(msg.flags(), "|", {0x0001, "ASYNC"}, {0x0002, "SWAPPER"}, {0x0004, "MOVEABLE"},
                              {0x0010, "NULLCREDS"}, {0x0020, "MAJORSEEN"}, {0x0080, "DYNAMIC"},
                              {0x0100, "NO_ROUND_ROBIN"}, {0x0200, "SOFT"}, {0x0400, "SOFTCONN"}, {0x0800, "SENT"},
                              {0x1000, "TIMEOUT"}, {0x2000, "NOCONNECT"}, {0x4000, "NORTO"}, {0x8000, "CRED_NOREF"}),
                __print_flags(msg.runstate(), "|", {(1UL << 0), "RUNNING"}, {(1UL << 1), "QUEUED"},
                              {(1UL << 2), "ACTIVE"}, {(1UL << 3), "NEED_XMIT"}, {(1UL << 4), "NEED_RECV"},
                              {(1UL << 5), "MSG_PIN_WAIT"}, {(1UL << 6), "SIGNALLED"}),
                msg.status(), msg.action());
        }
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_task_begin) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_task_begin) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_task_complete,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_task_complete_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_task_complete_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.action()) > 0) {
            functionStr = kernelSymbols[msg.action()];
        }
        if (functionStr != "") {
            len = snprintf_s(
                buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "rpc_task_complete: task:%08x@%08x flags=%s runstate=%s status=%d action=%s", msg.task_id(),
                msg.client_id(),
                __print_flags(msg.flags(), "|", {0x0001, "ASYNC"}, {0x0002, "SWAPPER"}, {0x0004, "MOVEABLE"},
                              {0x0010, "NULLCREDS"}, {0x0020, "MAJORSEEN"}, {0x0080, "DYNAMIC"},
                              {0x0100, "NO_ROUND_ROBIN"}, {0x0200, "SOFT"}, {0x0400, "SOFTCONN"}, {0x0800, "SENT"},
                              {0x1000, "TIMEOUT"}, {0x2000, "NOCONNECT"}, {0x4000, "NORTO"}, {0x8000, "CRED_NOREF"}),
                __print_flags(msg.runstate(), "|", {(1UL << 0), "RUNNING"}, {(1UL << 1), "QUEUED"},
                              {(1UL << 2), "ACTIVE"}, {(1UL << 3), "NEED_XMIT"}, {(1UL << 4), "NEED_RECV"},
                              {(1UL << 5), "MSG_PIN_WAIT"}, {(1UL << 6), "SIGNALLED"}),
                msg.status(), functionStr.c_str());
        } else {
            len = snprintf_s(
                buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "rpc_task_complete: task:%08x@%08x flags=%s runstate=%s status=%d action=%p", msg.task_id(),
                msg.client_id(),
                __print_flags(msg.flags(), "|", {0x0001, "ASYNC"}, {0x0002, "SWAPPER"}, {0x0004, "MOVEABLE"},
                              {0x0010, "NULLCREDS"}, {0x0020, "MAJORSEEN"}, {0x0080, "DYNAMIC"},
                              {0x0100, "NO_ROUND_ROBIN"}, {0x0200, "SOFT"}, {0x0400, "SOFTCONN"}, {0x0800, "SENT"},
                              {0x1000, "TIMEOUT"}, {0x2000, "NOCONNECT"}, {0x4000, "NORTO"}, {0x8000, "CRED_NOREF"}),
                __print_flags(msg.runstate(), "|", {(1UL << 0), "RUNNING"}, {(1UL << 1), "QUEUED"},
                              {(1UL << 2), "ACTIVE"}, {(1UL << 3), "NEED_XMIT"}, {(1UL << 4), "NEED_RECV"},
                              {(1UL << 5), "MSG_PIN_WAIT"}, {(1UL << 6), "SIGNALLED"}),
                msg.status(), msg.action());
        }
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_task_complete) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_task_complete) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_task_run_action,
    [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_task_run_action_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_task_run_action_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = 0;
        std::string functionStr = "";
        auto& kernelSymbols = EventFormatter::GetInstance().kernelSymbols_;
        if (kernelSymbols.count(msg.action()) > 0) {
            functionStr = kernelSymbols[msg.action()];
        }
        if (functionStr != "") {
            len = snprintf_s(
                buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "rpc_task_run_action: task:%08x@%08x flags=%s runstate=%s status=%d action=%s", msg.task_id(),
                msg.client_id(),
                __print_flags(msg.flags(), "|", {0x0001, "ASYNC"}, {0x0002, "SWAPPER"}, {0x0004, "MOVEABLE"},
                              {0x0010, "NULLCREDS"}, {0x0020, "MAJORSEEN"}, {0x0080, "DYNAMIC"},
                              {0x0100, "NO_ROUND_ROBIN"}, {0x0200, "SOFT"}, {0x0400, "SOFTCONN"}, {0x0800, "SENT"},
                              {0x1000, "TIMEOUT"}, {0x2000, "NOCONNECT"}, {0x4000, "NORTO"}, {0x8000, "CRED_NOREF"}),
                __print_flags(msg.runstate(), "|", {(1UL << 0), "RUNNING"}, {(1UL << 1), "QUEUED"},
                              {(1UL << 2), "ACTIVE"}, {(1UL << 3), "NEED_XMIT"}, {(1UL << 4), "NEED_RECV"},
                              {(1UL << 5), "MSG_PIN_WAIT"}, {(1UL << 6), "SIGNALLED"}),
                msg.status(), functionStr.c_str());
        } else {
            len = snprintf_s(
                buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                "rpc_task_run_action: task:%08x@%08x flags=%s runstate=%s status=%d action=%p", msg.task_id(),
                msg.client_id(),
                __print_flags(msg.flags(), "|", {0x0001, "ASYNC"}, {0x0002, "SWAPPER"}, {0x0004, "MOVEABLE"},
                              {0x0010, "NULLCREDS"}, {0x0020, "MAJORSEEN"}, {0x0080, "DYNAMIC"},
                              {0x0100, "NO_ROUND_ROBIN"}, {0x0200, "SOFT"}, {0x0400, "SOFTCONN"}, {0x0800, "SENT"},
                              {0x1000, "TIMEOUT"}, {0x2000, "NOCONNECT"}, {0x4000, "NORTO"}, {0x8000, "CRED_NOREF"}),
                __print_flags(msg.runstate(), "|", {(1UL << 0), "RUNNING"}, {(1UL << 1), "QUEUED"},
                              {(1UL << 2), "ACTIVE"}, {(1UL << 3), "NEED_XMIT"}, {(1UL << 4), "NEED_RECV"},
                              {(1UL << 5), "MSG_PIN_WAIT"}, {(1UL << 6), "SIGNALLED"}),
                msg.status(), msg.action());
        }
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_task_run_action) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_task_run_action) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_task_sleep, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_task_sleep_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_task_sleep_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "rpc_task_sleep: task:%08x@%08x flags=%s runstate=%s status=%d timeout=%" PRIu64 " queue=%s", msg.task_id(),
            msg.client_id(),
            __print_flags(msg.flags(), "|", {0x0001, "ASYNC"}, {0x0002, "SWAPPER"}, {0x0004, "MOVEABLE"},
                          {0x0010, "NULLCREDS"}, {0x0020, "MAJORSEEN"}, {0x0080, "DYNAMIC"}, {0x0100, "NO_ROUND_ROBIN"},
                          {0x0200, "SOFT"}, {0x0400, "SOFTCONN"}, {0x0800, "SENT"}, {0x1000, "TIMEOUT"},
                          {0x2000, "NOCONNECT"}, {0x4000, "NORTO"}, {0x8000, "CRED_NOREF"}),
            __print_flags(msg.runstate(), "|", {(1UL << 0), "RUNNING"}, {(1UL << 1), "QUEUED"}, {(1UL << 2), "ACTIVE"},
                          {(1UL << 3), "NEED_XMIT"}, {(1UL << 4), "NEED_RECV"}, {(1UL << 5), "MSG_PIN_WAIT"},
                          {(1UL << 6), "SIGNALLED"}),
            msg.status(), msg.timeout(), msg.q_name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_task_sleep) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_task_sleep) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    rpc_task_wakeup, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_rpc_task_wakeup_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.rpc_task_wakeup_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "rpc_task_wakeup: task:%08x@%08x flags=%s runstate=%s status=%d timeout=%" PRIu64 " queue=%s",
            msg.task_id(), msg.client_id(),
            __print_flags(msg.flags(), "|", {0x0001, "ASYNC"}, {0x0002, "SWAPPER"}, {0x0004, "MOVEABLE"},
                          {0x0010, "NULLCREDS"}, {0x0020, "MAJORSEEN"}, {0x0080, "DYNAMIC"}, {0x0100, "NO_ROUND_ROBIN"},
                          {0x0200, "SOFT"}, {0x0400, "SOFTCONN"}, {0x0800, "SENT"}, {0x1000, "TIMEOUT"},
                          {0x2000, "NOCONNECT"}, {0x4000, "NORTO"}, {0x8000, "CRED_NOREF"}),
            __print_flags(msg.runstate(), "|", {(1UL << 0), "RUNNING"}, {(1UL << 1), "QUEUED"}, {(1UL << 2), "ACTIVE"},
                          {(1UL << 3), "NEED_XMIT"}, {(1UL << 4), "NEED_RECV"}, {(1UL << 5), "MSG_PIN_WAIT"},
                          {(1UL << 6), "SIGNALLED"}),
            msg.status(), msg.timeout(), msg.q_name().c_str());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(rpc_task_wakeup) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(rpc_task_wakeup) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    svc_process, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_svc_process_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.svc_process_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
                             "svc_process: addr=%s xid=0x%08x service=%s vers=%u proc=%s", msg.addr().c_str(),
                             msg.xid(), msg.service().c_str(), msg.vers(), msg.procedure().c_str());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(svc_process) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(svc_process) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    svc_send, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_svc_send_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.svc_send_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "svc_send: xid=0x%08x server=%pISpc client=%pISpc status=%d flags=%s",
            msg.xid(), __get_sockaddr(server), __get_sockaddr(client), msg.status(),
            __print_flags(msg.flags(), "|", {((((1UL))) << (0)), "SECURE"}, {((((1UL))) << (1)), "LOCAL"},
                          {((((1UL))) << (2)), "USEDEFERRAL"}, {((((1UL))) << (3)), "DROPME"},
                          {((((1UL))) << (4)), "SPLICE_OK"}, {((((1UL))) << (5)), "VICTIM"},
                          {((((1UL))) << (6)), "BUSY"}, {((((1UL))) << (7)), "DATA"}));
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE, "maybe, the contents of print event(svc_send) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(svc_send) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    svc_wake_up, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_svc_wake_up_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.svc_wake_up_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "svc_wake_up: pid=%d", msg.pid());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(svc_wake_up) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(svc_wake_up) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    svc_xprt_dequeue, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_svc_xprt_dequeue_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.svc_xprt_dequeue_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(
            buffer, BUFFER_SIZE, BUFFER_SIZE - 1,
            "svc_xprt_dequeue: server=%pISpc client=%pISpc flags=%s wakeup-us=%" PRIu64 "", __get_sockaddr(server),
            __get_sockaddr(client),
            __print_flags(msg.flags(), "|", {((((1UL))) << (0)), "BUSY"}, {((((1UL))) << (1)), "CONN"},
                          {((((1UL))) << (2)), "CLOSE"}, {((((1UL))) << (3)), "DATA"}, {((((1UL))) << (4)), "TEMP"},
                          {((((1UL))) << (5)), "DEAD"}, {((((1UL))) << (6)), "CHNGBUF"},
                          {((((1UL))) << (7)), "DEFERRED"}, {((((1UL))) << (8)), "OLD"},
                          {((((1UL))) << (9)), "LISTENER"}, {((((1UL))) << (10)), "CACHE_AUTH"},
                          {((((1UL))) << (11)), "LOCAL"}, {((((1UL))) << (12)), "KILL_TEMP"},
                          {((((1UL))) << (13)), "CONG_CTRL"}, {((((1UL))) << (14)), "HANDSHAKE"},
                          {((((1UL))) << (15)), "TLS_SESSION"}, {((((1UL))) << (16)), "PEER_AUTH"}),
            msg.wakeup());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(svc_xprt_dequeue) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(svc_xprt_dequeue) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    xprt_lookup_rqst, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_xprt_lookup_rqst_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.xprt_lookup_rqst_format();
        char buffer[BUFFER_SIZE] = {0};
        int len =
            snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "xprt_lookup_rqst: peer=[%s]:%s xid=0x%08x status=%d",
                       msg.addr().c_str(), msg.port().c_str(), msg.xid(), msg.status());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(xprt_lookup_rqst) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(xprt_lookup_rqst) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });

REGISTER_FTRACE_EVENT_FORMATTER(
    xprt_transmit, [](const ForStandard::FtraceEvent& event) -> bool { return event.has_xprt_transmit_format(); },
    [](const ForStandard::FtraceEvent& event) -> std::string {
        auto msg = event.xprt_transmit_format();
        char buffer[BUFFER_SIZE] = {0};
        int len = snprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE - 1, "xprt_transmit: xid=0x%08x status=%d", msg.xid(),
                             msg.status());
        if (len >= BUFFER_SIZE - 1) {
            PROFILER_LOG_WARN(LOG_CORE,
                              "maybe, the contents of print event(xprt_transmit) msg had be cut off in outfile");
        }
        if (len < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "the contents of event(xprt_transmit) msg snprintf_s format error ");
            return "";
        }
        return std::string(buffer);
    });
}  // namespace
FTRACE_NS_END
