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
struct RpcBindStatusData {
    uint32_t task_id {};
    uint32_t client_id {};
    int32_t status {};
};
__attribute__((noinline)) void ParseRpcBindStatusData(RpcBindStatusData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.task_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.client_id = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRpcBindStatus(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcBindStatusData eventData {};
    ParseRpcBindStatusData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_bind_status_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_bind_status, ParseRpcBindStatus<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_bind_status, ParseRpcBindStatus<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseRpcCallStatus(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcCallStatusData eventData {};
    ParseRpcCallStatusData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_call_status_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_call_status, ParseRpcCallStatus<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_call_status, ParseRpcCallStatus<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseRpcConnectStatus(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcConnectStatusData eventData {};
    ParseRpcConnectStatusData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_connect_status_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_status(eventData.status);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_connect_status, ParseRpcConnectStatus<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_connect_status, ParseRpcConnectStatus<ProtoEncoder::FtraceEvent>);

struct RpcSocketCloseData {
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string dstaddr {};
    std::string dstport {};
};
__attribute__((noinline)) void ParseRpcSocketCloseData(RpcSocketCloseData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dstaddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.dstport = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRpcSocketClose(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcSocketCloseData eventData {};
    ParseRpcSocketCloseData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_close_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_dstaddr(eventData.dstaddr);
    msg->set_dstport(eventData.dstport);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_close, ParseRpcSocketClose<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_close, ParseRpcSocketClose<ProtoEncoder::FtraceEvent>);

struct RpcSocketConnectData {
    int32_t error {};
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string dstaddr {};
    std::string dstport {};
};
__attribute__((noinline)) void ParseRpcSocketConnectData(RpcSocketConnectData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dstaddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.dstport = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRpcSocketConnect(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcSocketConnectData eventData {};
    ParseRpcSocketConnectData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_connect_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_dstaddr(eventData.dstaddr);
    msg->set_dstport(eventData.dstport);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_connect, ParseRpcSocketConnect<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_connect, ParseRpcSocketConnect<ProtoEncoder::FtraceEvent>);

struct RpcSocketErrorData {
    int32_t error {};
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string dstaddr {};
    std::string dstport {};
};
__attribute__((noinline)) void ParseRpcSocketErrorData(RpcSocketErrorData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dstaddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.dstport = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRpcSocketError(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcSocketErrorData eventData {};
    ParseRpcSocketErrorData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_error_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_dstaddr(eventData.dstaddr);
    msg->set_dstport(eventData.dstport);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_error, ParseRpcSocketError<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_error, ParseRpcSocketError<ProtoEncoder::FtraceEvent>);

struct RpcSocketResetConnectionData {
    int32_t error {};
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string dstaddr {};
    std::string dstport {};
};
__attribute__((noinline)) void ParseRpcSocketResetConnectionData(RpcSocketResetConnectionData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.error = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dstaddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.dstport = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRpcSocketResetConnection(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcSocketResetConnectionData eventData {};
    ParseRpcSocketResetConnectionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_reset_connection_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_error(eventData.error);
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_dstaddr(eventData.dstaddr);
    msg->set_dstport(eventData.dstport);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_reset_connection, ParseRpcSocketResetConnection<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_reset_connection, ParseRpcSocketResetConnection<ProtoEncoder::FtraceEvent>);

struct RpcSocketShutdownData {
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string dstaddr {};
    std::string dstport {};
};
__attribute__((noinline)) void ParseRpcSocketShutdownData(RpcSocketShutdownData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dstaddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.dstport = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRpcSocketShutdown(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcSocketShutdownData eventData {};
    ParseRpcSocketShutdownData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_shutdown_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_dstaddr(eventData.dstaddr);
    msg->set_dstport(eventData.dstport);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_shutdown, ParseRpcSocketShutdown<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_shutdown, ParseRpcSocketShutdown<ProtoEncoder::FtraceEvent>);

struct RpcSocketStateChangeData {
    uint32_t socket_state {};
    uint32_t sock_state {};
    uint64_t ino {};
    std::string dstaddr {};
    std::string dstport {};
};
__attribute__((noinline)) void ParseRpcSocketStateChangeData(RpcSocketStateChangeData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.socket_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.sock_state = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.ino = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.dstaddr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.dstport = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseRpcSocketStateChange(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcSocketStateChangeData eventData {};
    ParseRpcSocketStateChangeData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_socket_state_change_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_socket_state(eventData.socket_state);
    msg->set_sock_state(eventData.sock_state);
    msg->set_ino(eventData.ino);
    msg->set_dstaddr(eventData.dstaddr);
    msg->set_dstport(eventData.dstport);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_socket_state_change, ParseRpcSocketStateChange<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_socket_state_change, ParseRpcSocketStateChange<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseRpcTaskBegin(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcTaskBeginData eventData {};
    ParseRpcTaskBeginData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_begin_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_begin, ParseRpcTaskBegin<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_begin, ParseRpcTaskBegin<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseRpcTaskComplete(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcTaskCompleteData eventData {};
    ParseRpcTaskCompleteData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_complete_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_complete, ParseRpcTaskComplete<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_complete, ParseRpcTaskComplete<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseRpcTaskRunAction(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcTaskRunActionData eventData {};
    ParseRpcTaskRunActionData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_run_action_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_action(eventData.action);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_run_action, ParseRpcTaskRunAction<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_run_action, ParseRpcTaskRunAction<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseRpcTaskSleep(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcTaskSleepData eventData {};
    ParseRpcTaskSleepData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_sleep_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_timeout(eventData.timeout);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
    msg->set_q_name(eventData.q_name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_sleep, ParseRpcTaskSleep<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_sleep, ParseRpcTaskSleep<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseRpcTaskWakeup(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    RpcTaskWakeupData eventData {};
    ParseRpcTaskWakeupData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_rpc_task_wakeup_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_task_id(eventData.task_id);
    msg->set_client_id(eventData.client_id);
    msg->set_timeout(eventData.timeout);
    msg->set_runstate(eventData.runstate);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
    msg->set_q_name(eventData.q_name);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(rpc_task_wakeup, ParseRpcTaskWakeup<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(rpc_task_wakeup, ParseRpcTaskWakeup<ProtoEncoder::FtraceEvent>);

struct SvcHandleXprtData {
    uint64_t xprt {};
    int32_t len {};
    uint64_t flags {};
    std::string addr {};
};
__attribute__((noinline)) void ParseSvcHandleXprtData(SvcHandleXprtData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xprt = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSvcHandleXprt(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SvcHandleXprtData eventData {};
    ParseSvcHandleXprtData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_handle_xprt_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xprt(eventData.xprt);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
    msg->set_addr(eventData.addr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_handle_xprt, ParseSvcHandleXprt<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_handle_xprt, ParseSvcHandleXprt<ProtoEncoder::FtraceEvent>);

struct SvcProcessData {
    uint32_t xid {};
    uint32_t vers {};
    uint32_t proc {};
    std::string service {};
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
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSvcProcess(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SvcProcessData eventData {};
    ParseSvcProcessData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_process_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xid(eventData.xid);
    msg->set_vers(eventData.vers);
    msg->set_proc(eventData.proc);
    msg->set_service(eventData.service);
    msg->set_addr(eventData.addr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_process, ParseSvcProcess<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_process, ParseSvcProcess<ProtoEncoder::FtraceEvent>);

struct SvcRecvData {
    uint32_t xid {};
    int32_t len {};
    uint64_t flags {};
    std::string addr {};
};
__attribute__((noinline)) void ParseSvcRecvData(SvcRecvData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.len = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSvcRecv(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SvcRecvData eventData {};
    ParseSvcRecvData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_recv_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xid(eventData.xid);
    msg->set_len(eventData.len);
    msg->set_flags(eventData.flags);
    msg->set_addr(eventData.addr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_recv, ParseSvcRecv<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_recv, ParseSvcRecv<ProtoEncoder::FtraceEvent>);

struct SvcSendData {
    uint32_t xid {};
    int32_t status {};
    uint64_t flags {};
    std::string addr {};
};
__attribute__((noinline)) void ParseSvcSendData(SvcSendData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSvcSend(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SvcSendData eventData {};
    ParseSvcSendData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_send_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xid(eventData.xid);
    msg->set_status(eventData.status);
    msg->set_flags(eventData.flags);
    msg->set_addr(eventData.addr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_send, ParseSvcSend<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_send, ParseSvcSend<ProtoEncoder::FtraceEvent>);

struct SvcWakeUpData {
    int32_t pid {};
};
__attribute__((noinline)) void ParseSvcWakeUpData(SvcWakeUpData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSvcWakeUp(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SvcWakeUpData eventData {};
    ParseSvcWakeUpData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_wake_up_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_pid(eventData.pid);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_wake_up, ParseSvcWakeUp<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_wake_up, ParseSvcWakeUp<ProtoEncoder::FtraceEvent>);

struct SvcXprtDequeueData {
    uint64_t xprt {};
    uint64_t flags {};
    uint64_t wakeup {};
    std::string addr {};
};
__attribute__((noinline)) void ParseSvcXprtDequeueData(SvcXprtDequeueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xprt = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.wakeup = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSvcXprtDequeue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SvcXprtDequeueData eventData {};
    ParseSvcXprtDequeueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_xprt_dequeue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xprt(eventData.xprt);
    msg->set_flags(eventData.flags);
    msg->set_wakeup(eventData.wakeup);
    msg->set_addr(eventData.addr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_xprt_dequeue, ParseSvcXprtDequeue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_xprt_dequeue, ParseSvcXprtDequeue<ProtoEncoder::FtraceEvent>);

struct SvcXprtDoEnqueueData {
    uint64_t xprt {};
    int32_t pid {};
    uint64_t flags {};
    std::string addr {};
};
__attribute__((noinline)) void ParseSvcXprtDoEnqueueData(SvcXprtDoEnqueueData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xprt = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.pid = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseSvcXprtDoEnqueue(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    SvcXprtDoEnqueueData eventData {};
    ParseSvcXprtDoEnqueueData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_svc_xprt_do_enqueue_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xprt(eventData.xprt);
    msg->set_pid(eventData.pid);
    msg->set_flags(eventData.flags);
    msg->set_addr(eventData.addr);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(svc_xprt_do_enqueue, ParseSvcXprtDoEnqueue<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(svc_xprt_do_enqueue, ParseSvcXprtDoEnqueue<ProtoEncoder::FtraceEvent>);

struct XprtCompleteRqstData {
    uint32_t xid {};
    int32_t status {};
    std::string addr {};
    std::string port {};
};
__attribute__((noinline)) void ParseXprtCompleteRqstData(XprtCompleteRqstData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.port = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseXprtCompleteRqst(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    XprtCompleteRqstData eventData {};
    ParseXprtCompleteRqstData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xprt_complete_rqst_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xid(eventData.xid);
    msg->set_status(eventData.status);
    msg->set_addr(eventData.addr);
    msg->set_port(eventData.port);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xprt_complete_rqst, ParseXprtCompleteRqst<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xprt_complete_rqst, ParseXprtCompleteRqst<ProtoEncoder::FtraceEvent>);

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
template <typename FtraceEventType>
void ParseXprtLookupRqst(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    XprtLookupRqstData eventData {};
    ParseXprtLookupRqstData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xprt_lookup_rqst_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xid(eventData.xid);
    msg->set_status(eventData.status);
    msg->set_addr(eventData.addr);
    msg->set_port(eventData.port);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xprt_lookup_rqst, ParseXprtLookupRqst<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xprt_lookup_rqst, ParseXprtLookupRqst<ProtoEncoder::FtraceEvent>);

struct XprtTransmitData {
    uint32_t xid {};
    int32_t status {};
    std::string addr {};
    std::string port {};
};
__attribute__((noinline)) void ParseXprtTransmitData(XprtTransmitData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.status = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.port = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseXprtTransmit(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    XprtTransmitData eventData {};
    ParseXprtTransmitData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xprt_transmit_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_xid(eventData.xid);
    msg->set_status(eventData.status);
    msg->set_addr(eventData.addr);
    msg->set_port(eventData.port);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xprt_transmit, ParseXprtTransmit<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xprt_transmit, ParseXprtTransmit<ProtoEncoder::FtraceEvent>);

struct XsTcpDataReadyData {
    int32_t err {};
    uint32_t total {};
    std::string addr {};
    std::string port {};
};
__attribute__((noinline)) void ParseXsTcpDataReadyData(XsTcpDataReadyData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.err = FtraceFieldParser::ParseIntField<int32_t>(format.fields, i++, data, size);
    eventData.total = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.port = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseXsTcpDataReady(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    XsTcpDataReadyData eventData {};
    ParseXsTcpDataReadyData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xs_tcp_data_ready_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_err(eventData.err);
    msg->set_total(eventData.total);
    msg->set_addr(eventData.addr);
    msg->set_port(eventData.port);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xs_tcp_data_ready, ParseXsTcpDataReady<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xs_tcp_data_ready, ParseXsTcpDataReady<ProtoEncoder::FtraceEvent>);

struct XsTcpDataRecvData {
    std::string addr {};
    std::string port {};
    uint32_t xid {};
    uint64_t flags {};
    uint64_t copied {};
    uint32_t reclen {};
    uint64_t offset {};
};
__attribute__((noinline)) void ParseXsTcpDataRecvData(XsTcpDataRecvData& eventData, uint8_t data[], size_t size,
                                             const EventFormat& format)
{
    int i = 0;
    eventData.addr = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.port = FtraceFieldParser::ParseStrField(format.fields, i++, data, size);
    eventData.xid = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.flags = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.copied = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
    eventData.reclen = FtraceFieldParser::ParseIntField<uint32_t>(format.fields, i++, data, size);
    eventData.offset = FtraceFieldParser::ParseIntField<uint64_t>(format.fields, i++, data, size);
}
template <typename FtraceEventType>
void ParseXsTcpDataRecv(FtraceEventType& ftraceEvent, uint8_t data[], size_t size,
                               const EventFormat& format)
{
    if (data == nullptr || size == 0) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: invalid input data or size", __func__);
        return;
    }
    XsTcpDataRecvData eventData {};
    ParseXsTcpDataRecvData(eventData, data, size, format);
    auto msg = ftraceEvent.mutable_xs_tcp_data_recv_format();
    if (msg == nullptr) {
        return;
    }
    msg->set_addr(eventData.addr);
    msg->set_port(eventData.port);
    msg->set_xid(eventData.xid);
    msg->set_flags(eventData.flags);
    msg->set_copied(eventData.copied);
    msg->set_reclen(eventData.reclen);
    msg->set_offset(eventData.offset);
}
REGISTER_FTRACE_EVENT_PARSE_FUNCTION(xs_tcp_data_recv, ParseXsTcpDataRecv<FtraceEvent>);
REGISTER_FTRACE_EVENT_PARSE_ENCODER_FUNCTION(xs_tcp_data_recv, ParseXsTcpDataRecv<ProtoEncoder::FtraceEvent>);
} // namespace
FTRACE_NS_END
