/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef COMMON_H
#define COMMON_H
#include <unordered_map>
#include <string>
namespace OHOS {
namespace SmartPerf {
enum class MessageType {
    GET_CPU_NUM,
    GET_CPU_FREQ,
    GET_CPU_LOAD,
    SET_PKG_NAME,
    SET_PROCESS_ID,
    GET_FPS_AND_JITTERS,
    GET_GPU_FREQ,
    GET_GPU_LOAD,
    GET_DDR_FREQ,
    GET_RAM_INFO,
    GET_TEMPERATURE,
    GET_POWER,
    GET_CAPTURE,
    CATCH_TRACE_START,
    CATCH_TRACE_FINISH,
    SET_DUBAI_DB
};

const std::unordered_map<MessageType, std::string> messageMap = {
    { MessageType::GET_CPU_NUM, std::string("get_cpu_num") },
    { MessageType::GET_CPU_FREQ, std::string("get_cpu_freq") },
    { MessageType::GET_CPU_LOAD, std::string("get_cpu_load") },
    { MessageType::SET_PKG_NAME, std::string("set_pkgName") },
    { MessageType::SET_PROCESS_ID, std::string("set_pid") },
    { MessageType::GET_FPS_AND_JITTERS, std::string("get_fps_and_jitters") },
    { MessageType::GET_GPU_FREQ, std::string("get_gpu_freq") },
    { MessageType::GET_GPU_LOAD, std::string("get_gpu_load") },
    { MessageType::GET_DDR_FREQ, std::string("get_ddr_freq") },
    { MessageType::GET_RAM_INFO, std::string("get_ram_info") },
    { MessageType::GET_TEMPERATURE, std::string("get_temperature") },
    { MessageType::GET_POWER, std::string("get_power") },
    { MessageType::GET_CAPTURE, std::string("get_capture") },
    { MessageType::CATCH_TRACE_START, std::string("catch_trace_start") },
    { MessageType::CATCH_TRACE_FINISH, std::string("catch_trace_end") },
    { MessageType::SET_DUBAI_DB, std::string("set_dubai_db") },
};

enum class CommandType {
    CT_N,
    CT_PKG,
    CT_PID,
    CT_OUT,
    CT_C,
    CT_G,
    CT_D,
    CT_F,
    CT_F1,
    CT_F2,
    CT_T,
    CT_P,
    CT_R,
    CT_TTRACE,
    CT_SNAPSHOT,
    CT_HW,
    CT_SESSIONID,
    CT_INTERVAL,
    CT_NET
};
enum class CommandHelp {
    HELP,
    VERSION
};

const std::unordered_map<std::string, CommandType> commandMap = {
    { std::string("-N"), CommandType::CT_N },          { std::string("-PKG"), CommandType::CT_PKG },
    { std::string("-PID"), CommandType::CT_PID },      { std::string("-OUT"), CommandType::CT_OUT },
    { std::string("-c"), CommandType::CT_C },          { std::string("-g"), CommandType::CT_G },
    { std::string("-f"), CommandType::CT_F },          { std::string("-f1"), CommandType::CT_F1 },
    { std::string("-f2"), CommandType::CT_F1 },        { std::string("-t"), CommandType::CT_T },
    { std::string("-p"), CommandType::CT_P },          { std::string("-r"), CommandType::CT_R },
    { std::string("-trace"), CommandType::CT_TTRACE }, { std::string("-snapshot"), CommandType::CT_SNAPSHOT },
    { std::string("-hw"), CommandType::CT_HW },        { std::string("-d"), CommandType::CT_D },
    { std::string("-INTERVAL"), CommandType::CT_INTERVAL },  { std::string("-SESSIONID"), CommandType::CT_SESSIONID },
    { std::string("-net"), CommandType::CT_NET },
};

const std::unordered_map<CommandType, std::string> COMMAND_MAP_REVERSE = {
    { CommandType::CT_N, std::string("-N") },          { CommandType::CT_PKG, std::string("-PKG") },
    { CommandType::CT_PID, std::string("-PID") },      { CommandType::CT_OUT, std::string("-OUT") },
    { CommandType::CT_C, std::string("-c") },          { CommandType::CT_G, std::string("-g") },
    { CommandType::CT_F, std::string("-f") },          { CommandType::CT_F1, std::string("-f1") },
    { CommandType::CT_F1, std::string("-f2") },        { CommandType::CT_T, std::string("-t") },
    { CommandType::CT_P, std::string("-p") },          { CommandType::CT_R, std::string("-r") },
    { CommandType::CT_TTRACE, std::string("-trace") }, { CommandType::CT_SNAPSHOT, std::string("-snapshot") },
    { CommandType::CT_HW, std::string("-hw") },        { CommandType::CT_D, std::string("-d") },
    { CommandType::CT_INTERVAL, std::string("-INTERVAL") },  { CommandType::CT_SESSIONID, std::string("-SESSIONID") },
    { CommandType::CT_NET, std::string("-net") },
};


const std::unordered_map<CommandHelp, std::string> commandHelpMap = {
    { CommandHelp::HELP, std::string("--help") },
    { CommandHelp::VERSION, std::string("--version") },
};

enum class TraceStatus {
    TRACE_START,
    TRACE_FINISH,
    TRACE_NO
};
}
}
#endif