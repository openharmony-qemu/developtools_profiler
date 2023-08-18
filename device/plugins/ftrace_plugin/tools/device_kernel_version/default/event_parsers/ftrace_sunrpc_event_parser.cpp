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
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_bind_status,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_bind_status_format();
        msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_call_status,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_call_status_format();
        msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_connect_status,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_connect_status_format();
        msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_socket_close,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_socket_close_format();
        msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dstaddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_dstport(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_socket_connect,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_socket_connect_format();
        msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dstaddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_dstport(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_socket_error,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_socket_error_format();
        msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dstaddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_dstport(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_socket_reset_connection,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_socket_reset_connection_format();
        msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dstaddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_dstport(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_socket_shutdown,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_socket_shutdown_format();
        msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dstaddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_dstport(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_socket_state_change,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_socket_state_change_format();
        msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_dstaddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_dstport(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_task_begin,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_task_begin_format();
        msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_action(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_task_complete,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_task_complete_format();
        msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_action(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_task_run_action,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_task_run_action_format();
        msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_action(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_task_sleep,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_task_sleep_format();
        msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timeout(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_q_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    rpc_task_wakeup,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_rpc_task_wakeup_format();
        msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_timeout(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_q_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    svc_handle_xprt,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_svc_handle_xprt_format();
        msg->set_xprt(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    svc_process,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_svc_process_format();
        msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_vers(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_proc(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_service(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    svc_recv,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_svc_recv_format();
        msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_len(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    svc_send,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_svc_send_format();
        msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    svc_wake_up,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_svc_wake_up_format();
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    svc_xprt_dequeue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_svc_xprt_dequeue_format();
        msg->set_xprt(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_wakeup(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    svc_xprt_do_enqueue,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_svc_xprt_do_enqueue_format();
        msg->set_xprt(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    xprt_complete_rqst,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_xprt_complete_rqst_format();
        msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_port(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    xprt_lookup_rqst,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_xprt_lookup_rqst_format();
        msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_port(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    xprt_transmit,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_xprt_transmit_format();
        msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_port(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    xs_tcp_data_ready,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_xs_tcp_data_ready_format();
        msg->set_err(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
        msg->set_total(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_port(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    });

REGISTER_FTRACE_EVENT_PARSE_FUNCTION(
    xs_tcp_data_recv,
    [](FtraceEvent& ftraceEvent, uint8_t data[], size_t size, const EventFormat& format) {
        int i = 0;
        auto msg = ftraceEvent.mutable_xs_tcp_data_recv_format();
        msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_port(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
        msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_copied(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
        msg->set_reclen(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
        msg->set_offset(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    });
} // namespace
FTRACE_NS_END
