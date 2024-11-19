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
void rpc_call_status_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_call_status, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                         const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_call_status_format();
    rpc_call_status_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_call_status, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                 size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_call_status_format();
    rpc_call_status_func(msg, data, size, format);
});

template <typename T>
void rpc_connect_status_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_connect_status, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_connect_status_format();
    rpc_connect_status_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_connect_status,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_connect_status_format();
                                                rpc_connect_status_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_socket_close_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_saddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_daddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_close, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_socket_close_format();
    rpc_socket_close_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_close,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_socket_close_format();
                                                rpc_socket_close_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_socket_connect_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_saddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_daddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_connect, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                            const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_socket_connect_format();
    rpc_socket_connect_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_connect,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_socket_connect_format();
                                                rpc_socket_connect_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_socket_error_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_saddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_daddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_error, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_socket_error_format();
    rpc_socket_error_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_error,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_socket_error_format();
                                                rpc_socket_error_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_socket_reset_connection_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_error(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_saddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_daddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_reset_connection, [](FtraceEvent &ftraceEvent, uint8_t data[],
                                                                     size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_socket_reset_connection_format();
    rpc_socket_reset_connection_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_reset_connection,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_socket_reset_connection_format();
                                                rpc_socket_reset_connection_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_socket_shutdown_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_saddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_daddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_shutdown, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                             const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_socket_shutdown_format();
    rpc_socket_shutdown_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_shutdown,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_socket_shutdown_format();
                                                rpc_socket_shutdown_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_socket_state_change_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_socket_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_sock_state(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_ino(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_saddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_daddr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_state_change, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                                 const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_socket_state_change_format();
    rpc_socket_state_change_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_state_change,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_socket_state_change_format();
                                                rpc_socket_state_change_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_task_begin_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_action(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_begin, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                        const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_task_begin_format();
    rpc_task_begin_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_begin, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_task_begin_format();
    rpc_task_begin_func(msg, data, size, format);
});

template <typename T>
void rpc_task_complete_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_action(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_complete, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                           const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_task_complete_format();
    rpc_task_complete_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_complete,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_task_complete_format();
                                                rpc_task_complete_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_task_run_action_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_action(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_run_action, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                             const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_task_run_action_format();
    rpc_task_run_action_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_run_action,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_rpc_task_run_action_format();
                                                rpc_task_run_action_func(msg, data, size, format);
                                            });

template <typename T>
void rpc_task_sleep_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_timeout(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_q_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_sleep, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                        const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_task_sleep_format();
    rpc_task_sleep_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_sleep, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_task_sleep_format();
    rpc_task_sleep_func(msg, data, size, format);
});

template <typename T>
void rpc_task_wakeup_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_timeout(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_runstate(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_q_name(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_wakeup, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                         const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_task_wakeup_format();
    rpc_task_wakeup_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_wakeup, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                                 size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_rpc_task_wakeup_format();
    rpc_task_wakeup_func(msg, data, size, format);
});

template <typename T>
void svc_process_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_vers(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_proc(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_service(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_procedure(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_process, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                     const EventFormat &format) {
    auto msg = ftraceEvent.mutable_svc_process_format();
    svc_process_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_process, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                             size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_svc_process_format();
    svc_process_func(msg, data, size, format);
});

template <typename T>
void svc_send_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_server(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_netns_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_send, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                  const EventFormat &format) {
    auto msg = ftraceEvent.mutable_svc_send_format();
    svc_send_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_send, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                          size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_svc_send_format();
    svc_send_func(msg, data, size, format);
});

template <typename T>
void svc_wake_up_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_pid(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_wake_up, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                     const EventFormat &format) {
    auto msg = ftraceEvent.mutable_svc_wake_up_format();
    svc_wake_up_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_wake_up, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                             size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_svc_wake_up_format();
    svc_wake_up_func(msg, data, size, format);
});

template <typename T>
void svc_xprt_dequeue_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_server(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_flags(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
    msg->set_netns_ino(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_wakeup(FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_xprt_dequeue, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_svc_xprt_dequeue_format();
    svc_xprt_dequeue_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_xprt_dequeue,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_svc_xprt_dequeue_format();
                                                svc_xprt_dequeue_func(msg, data, size, format);
                                            });

template <typename T>
void xprt_lookup_rqst_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
    msg->set_addr(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
    msg->set_port(FtraceFieldParser::ParseStrField(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xprt_lookup_rqst, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                          const EventFormat &format) {
    auto msg = ftraceEvent.mutable_xprt_lookup_rqst_format();
    xprt_lookup_rqst_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xprt_lookup_rqst,
                                             [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                const EventFormat &format) {
                                                auto msg = ftraceEvent.mutable_xprt_lookup_rqst_format();
                                                xprt_lookup_rqst_func(msg, data, size, format);
                                            });

template <typename T>
void xprt_transmit_func(T &msg, uint8_t data[], size_t size, const EventFormat &format)
{
    int i = 0;
    msg->set_task_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_client_id(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_xid(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_seqno(FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size));
    msg->set_status(FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size));
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xprt_transmit, [](FtraceEvent &ftraceEvent, uint8_t data[], size_t size,
                                                       const EventFormat &format) {
    auto msg = ftraceEvent.mutable_xprt_transmit_format();
    xprt_transmit_func(msg, data, size, format);
});
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xprt_transmit, [](ProtoEncoder::FtraceEvent &ftraceEvent, uint8_t data[],
                                                               size_t size, const EventFormat &format) {
    auto msg = ftraceEvent.mutable_xprt_transmit_format();
    xprt_transmit_func(msg, data, size, format);
});
}  // namespace
FTRACE_NS_END
