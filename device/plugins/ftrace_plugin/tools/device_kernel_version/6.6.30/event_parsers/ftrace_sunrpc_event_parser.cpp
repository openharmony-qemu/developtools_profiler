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

#include <string>
#include <vector>

FTRACE_NS_BEGIN
namespace {
using namespace OHOS::Developtools::Profiler;
struct RpcCallStatusData {
    uint32_t task_id {};
    uint32_t client_id {};
    int32_t status {};
};
__attribute__((noinline)) void ParseRpcCallStatusData(RpcCallStatusData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseRpcCallStatus(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcCallStatusData eventData {};
    ParseRpcCallStatusData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_call_status_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_status(eventData.status);
}
void ParseRpcCallStatusEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcCallStatusData eventData {};
    ParseRpcCallStatusData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_call_status_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_call_status, ParseRpcCallStatus);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_call_status, ParseRpcCallStatusEncoder);

struct RpcConnectStatusData {
    uint32_t task_id {};
    uint32_t client_id {};
    int32_t status {};
};
__attribute__((noinline)) void ParseRpcConnectStatusData(RpcConnectStatusData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseRpcConnectStatus(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcConnectStatusData eventData {};
    ParseRpcConnectStatusData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_connect_status_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_status(eventData.status);
}
void ParseRpcConnectStatusEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcConnectStatusData eventData {};
    ParseRpcConnectStatusData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_connect_status_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_connect_status, ParseRpcConnectStatus);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_connect_status, ParseRpcConnectStatusEncoder);

struct RpcSocketCloseData {
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string saddr {};
    std::string daddr {};
};
__attribute__((noinline)) void ParseRpcSocketCloseData(RpcSocketCloseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.saddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.daddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRpcSocketClose(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketCloseData eventData {};
    ParseRpcSocketCloseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_close_format();
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
void ParseRpcSocketCloseEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketCloseData eventData {};
    ParseRpcSocketCloseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_close_format();
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_close, ParseRpcSocketClose);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_close, ParseRpcSocketCloseEncoder);

struct RpcSocketConnectData {
    int32_t error {};
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string saddr {};
    std::string daddr {};
};
__attribute__((noinline)) void ParseRpcSocketConnectData(RpcSocketConnectData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.saddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.daddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRpcSocketConnect(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketConnectData eventData {};
    ParseRpcSocketConnectData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_connect_format();
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
void ParseRpcSocketConnectEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketConnectData eventData {};
    ParseRpcSocketConnectData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_connect_format();
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_connect, ParseRpcSocketConnect);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_connect, ParseRpcSocketConnectEncoder);

struct RpcSocketErrorData {
    int32_t error {};
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string saddr {};
    std::string daddr {};
};
__attribute__((noinline)) void ParseRpcSocketErrorData(RpcSocketErrorData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.saddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.daddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRpcSocketError(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketErrorData eventData {};
    ParseRpcSocketErrorData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_error_format();
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
void ParseRpcSocketErrorEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketErrorData eventData {};
    ParseRpcSocketErrorData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_error_format();
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_error, ParseRpcSocketError);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_error, ParseRpcSocketErrorEncoder);

struct RpcSocketResetConnectionData {
    int32_t error {};
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string saddr {};
    std::string daddr {};
};
__attribute__((noinline)) void ParseRpcSocketResetConnectionData(RpcSocketResetConnectionData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.saddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.daddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRpcSocketResetConnection(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketResetConnectionData eventData {};
    ParseRpcSocketResetConnectionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_reset_connection_format();
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
void ParseRpcSocketResetConnectionEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketResetConnectionData eventData {};
    ParseRpcSocketResetConnectionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_reset_connection_format();
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_reset_connection, ParseRpcSocketResetConnection);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_reset_connection, ParseRpcSocketResetConnectionEncoder);

struct RpcSocketShutdownData {
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string saddr {};
    std::string daddr {};
};
__attribute__((noinline)) void ParseRpcSocketShutdownData(RpcSocketShutdownData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.saddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.daddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRpcSocketShutdown(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketShutdownData eventData {};
    ParseRpcSocketShutdownData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_shutdown_format();
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
void ParseRpcSocketShutdownEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketShutdownData eventData {};
    ParseRpcSocketShutdownData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_shutdown_format();
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_shutdown, ParseRpcSocketShutdown);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_shutdown, ParseRpcSocketShutdownEncoder);

struct RpcSocketStateChangeData {
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string saddr {};
    std::string daddr {};
};
__attribute__((noinline)) void ParseRpcSocketStateChangeData(RpcSocketStateChangeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.saddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.daddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRpcSocketStateChange(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketStateChangeData eventData {};
    ParseRpcSocketStateChangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_state_change_format();
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
void ParseRpcSocketStateChangeEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcSocketStateChangeData eventData {};
    ParseRpcSocketStateChangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_state_change_format();
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_saddr(eventData.saddr);
    msg->set_daddr(eventData.daddr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_state_change, ParseRpcSocketStateChange);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_state_change, ParseRpcSocketStateChangeEncoder);

struct RpcTaskBeginData {
    uint32_t task_id {};
    uint32_t client_id {};
    uint64_t action {};
    uint64_t runstate {};
    int32_t status {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseRpcTaskBeginData(RpcTaskBeginData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.action = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.runstate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseRpcTaskBegin(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskBeginData eventData {};
    ParseRpcTaskBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_begin_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
void ParseRpcTaskBeginEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskBeginData eventData {};
    ParseRpcTaskBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_begin_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_begin, ParseRpcTaskBegin);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_begin, ParseRpcTaskBeginEncoder);

struct RpcTaskCompleteData {
    uint32_t task_id {};
    uint32_t client_id {};
    uint64_t action {};
    uint64_t runstate {};
    int32_t status {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseRpcTaskCompleteData(RpcTaskCompleteData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.action = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.runstate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseRpcTaskComplete(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskCompleteData eventData {};
    ParseRpcTaskCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_complete_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
void ParseRpcTaskCompleteEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskCompleteData eventData {};
    ParseRpcTaskCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_complete_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_complete, ParseRpcTaskComplete);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_complete, ParseRpcTaskCompleteEncoder);

struct RpcTaskRunActionData {
    uint32_t task_id {};
    uint32_t client_id {};
    uint64_t action {};
    uint64_t runstate {};
    int32_t status {};
    uint32_t flags {};
};
__attribute__((noinline)) void ParseRpcTaskRunActionData(RpcTaskRunActionData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.action = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.runstate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
}
void ParseRpcTaskRunAction(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskRunActionData eventData {};
    ParseRpcTaskRunActionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_run_action_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
void ParseRpcTaskRunActionEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskRunActionData eventData {};
    ParseRpcTaskRunActionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_run_action_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_run_action, ParseRpcTaskRunAction);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_run_action, ParseRpcTaskRunActionEncoder);

struct RpcTaskSleepData {
    uint32_t task_id {};
    uint32_t client_id {};
    uint64_t timeout {};
    uint64_t runstate {};
    int32_t status {};
    uint32_t flags {};
    std::string q_name {};
};
__attribute__((noinline)) void ParseRpcTaskSleepData(RpcTaskSleepData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timeout = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.runstate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.q_name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRpcTaskSleep(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskSleepData eventData {};
    ParseRpcTaskSleepData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_sleep_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_timeout(eventData.timeout);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
    msg->set_q_name(eventData.q_name);
}
void ParseRpcTaskSleepEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskSleepData eventData {};
    ParseRpcTaskSleepData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_sleep_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_timeout(eventData.timeout);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
    msg->set_q_name(eventData.q_name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_sleep, ParseRpcTaskSleep);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_sleep, ParseRpcTaskSleepEncoder);

struct RpcTaskWakeupData {
    uint32_t task_id {};
    uint32_t client_id {};
    uint64_t timeout {};
    uint64_t runstate {};
    int32_t status {};
    uint32_t flags {};
    std::string q_name {};
};
__attribute__((noinline)) void ParseRpcTaskWakeupData(RpcTaskWakeupData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.timeout = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.runstate = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.q_name = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseRpcTaskWakeup(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskWakeupData eventData {};
    ParseRpcTaskWakeupData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_wakeup_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_timeout(eventData.timeout);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
    msg->set_q_name(eventData.q_name);
}
void ParseRpcTaskWakeupEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    RpcTaskWakeupData eventData {};
    ParseRpcTaskWakeupData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_wakeup_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_timeout(eventData.timeout);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
    msg->set_q_name(eventData.q_name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_wakeup, ParseRpcTaskWakeup);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_wakeup, ParseRpcTaskWakeupEncoder);

struct SvcProcessData {
    uint32_t xid {};
    uint32_t vers {};
    uint32_t proc {};
    std::string service {};
    std::string procedure {};
    std::string addr {};
};
__attribute__((noinline)) void ParseSvcProcessData(SvcProcessData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.vers = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.proc = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.service = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.procedure = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseSvcProcess(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SvcProcessData eventData {};
    ParseSvcProcessData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_process_format();
    msg->set_xid(eventData.xid);
    msg->set_vers(eventData.vers);
    msg->set_proc(eventData.proc);
    msg->set_service(eventData.service);
    msg->set_procedure(eventData.procedure);
    msg->set_addr(eventData.addr);
}
void ParseSvcProcessEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SvcProcessData eventData {};
    ParseSvcProcessData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_process_format();
    msg->set_xid(eventData.xid);
    msg->set_vers(eventData.vers);
    msg->set_proc(eventData.proc);
    msg->set_service(eventData.service);
    msg->set_procedure(eventData.procedure);
    msg->set_addr(eventData.addr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_process, ParseSvcProcess);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_process, ParseSvcProcessEncoder);

struct SvcSendData {
    uint32_t server {};
    uint32_t client {};
    uint32_t netns_ino {};
    uint32_t xid {};
    int32_t status {};
    uint64_t flags {};
};
__attribute__((noinline)) void ParseSvcSendData(SvcSendData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.server = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.netns_ino = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseSvcSend(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SvcSendData eventData {};
    ParseSvcSendData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_send_format();
    msg->set_server(eventData.server);
    msg->set_client(eventData.client);
    msg->set_netns_ino(eventData.netns_ino);
    msg->set_xid(eventData.xid);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
void ParseSvcSendEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SvcSendData eventData {};
    ParseSvcSendData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_send_format();
    msg->set_server(eventData.server);
    msg->set_client(eventData.client);
    msg->set_netns_ino(eventData.netns_ino);
    msg->set_xid(eventData.xid);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_send, ParseSvcSend);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_send, ParseSvcSendEncoder);

struct SvcWakeUpData {
    int32_t pid {};
};
__attribute__((noinline)) void ParseSvcWakeUpData(SvcWakeUpData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseSvcWakeUp(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SvcWakeUpData eventData {};
    ParseSvcWakeUpData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_wake_up_format();
    msg->set_pid(eventData.pid);
}
void ParseSvcWakeUpEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SvcWakeUpData eventData {};
    ParseSvcWakeUpData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_wake_up_format();
    msg->set_pid(eventData.pid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_wake_up, ParseSvcWakeUp);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_wake_up, ParseSvcWakeUpEncoder);

struct SvcXprtDequeueData {
    uint32_t server {};
    uint32_t client {};
    uint64_t flags {};
    uint32_t netns_ino {};
    uint64_t wakeup {};
};
__attribute__((noinline)) void ParseSvcXprtDequeueData(SvcXprtDequeueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.server = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.netns_ino = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.wakeup = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
void ParseSvcXprtDequeue(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SvcXprtDequeueData eventData {};
    ParseSvcXprtDequeueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_xprt_dequeue_format();
    msg->set_server(eventData.server);
    msg->set_client(eventData.client);
    msg->set_flags(eventData.flags);
    msg->set_netns_ino(eventData.netns_ino);
    msg->set_wakeup(eventData.wakeup);
}
void ParseSvcXprtDequeueEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    SvcXprtDequeueData eventData {};
    ParseSvcXprtDequeueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_xprt_dequeue_format();
    msg->set_server(eventData.server);
    msg->set_client(eventData.client);
    msg->set_flags(eventData.flags);
    msg->set_netns_ino(eventData.netns_ino);
    msg->set_wakeup(eventData.wakeup);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_xprt_dequeue, ParseSvcXprtDequeue);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_xprt_dequeue, ParseSvcXprtDequeueEncoder);

struct XprtLookupRqstData {
    uint32_t xid {};
    int32_t status {};
    std::string addr {};
    std::string port {};
};
__attribute__((noinline)) void ParseXprtLookupRqstData(XprtLookupRqstData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.port = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
void ParseXprtLookupRqst(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    XprtLookupRqstData eventData {};
    ParseXprtLookupRqstData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xprt_lookup_rqst_format();
    msg->set_xid(eventData.xid);
    msg->set_status(eventData.status);
    msg->set_addr(eventData.addr);
    msg->set_port(eventData.port);
}
void ParseXprtLookupRqstEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    XprtLookupRqstData eventData {};
    ParseXprtLookupRqstData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xprt_lookup_rqst_format();
    msg->set_xid(eventData.xid);
    msg->set_status(eventData.status);
    msg->set_addr(eventData.addr);
    msg->set_port(eventData.port);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xprt_lookup_rqst, ParseXprtLookupRqst);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xprt_lookup_rqst, ParseXprtLookupRqstEncoder);

struct XprtTransmitData {
    uint32_t task_id {};
    uint32_t client_id {};
    uint32_t xid {};
    uint32_t seqno {};
    int32_t status {};
};
__attribute__((noinline)) void ParseXprtTransmitData(XprtTransmitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.seqno = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
void ParseXprtTransmit(FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    XprtTransmitData eventData {};
    ParseXprtTransmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xprt_transmit_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_xid(eventData.xid);
    msg->set_seqno(eventData.seqno);
    msg->set_status(eventData.status);
}
void ParseXprtTransmitEncoder(ProtoEncoder::FtraceEvent& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    XprtTransmitData eventData {};
    ParseXprtTransmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xprt_transmit_format();
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_xid(eventData.xid);
    msg->set_seqno(eventData.seqno);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xprt_transmit, ParseXprtTransmit);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xprt_transmit, ParseXprtTransmitEncoder);
}  // namespace
FTRACE_NS_END
