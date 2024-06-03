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
    GET_MEMORY_INFO,
    GET_TEMPERATURE,
    GET_POWER,
    GET_CAPTURE,
    CATCH_TRACE_CONFIG,
    CATCH_TRACE_CMD,
    SET_DUBAI_DB,
    GPU_COUNTER_HB_REQ,
    CATCH_GPU_COUNTER,      // 请求抓取GPU counter信息
    GET_GPU_COUNTER_RESULT, // 获取GPU counter信息
    CATCH_NETWORK_TRAFFIC,
    GET_NETWORK_TRAFFIC, // 获取网络流量信息
    BACK_TO_DESKTOP,
};

const std::unordered_map<MessageType, std::string> MESSAGE_MAP = {
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
    { MessageType::GET_MEMORY_INFO, std::string("get_memory") },
    { MessageType::CATCH_TRACE_CONFIG, std::string("catch_trace_config") },
    { MessageType::CATCH_TRACE_CMD, std::string("catch_trace_cmd") },
    { MessageType::SET_DUBAI_DB, std::string("set_dubai_db") },
    { MessageType::GPU_COUNTER_HB_REQ, std::string("gpu_counter_hb_req") },
    { MessageType::CATCH_GPU_COUNTER, std::string("catch_gpu_counter") },
    { MessageType::GET_GPU_COUNTER_RESULT, std::string("get_gpu_counter_result") },
    { MessageType::CATCH_NETWORK_TRAFFIC, std::string("catch_network_traffic") },
    { MessageType::GET_NETWORK_TRAFFIC, std::string("get_network_traffic") },
    { MessageType::BACK_TO_DESKTOP, std::string("back_to_desk") },
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
    CT_T,
    CT_P,
    CT_R,
    CT_TTRACE,
    CT_SNAPSHOT,
    CT_HW,
    CT_SESSIONID,
    CT_INTERVAL,
    CT_NET,
    CT_VIEW,
    CT_FL,      //帧率限制值
    CT_FTL,     //帧间隔限制值，单位ms
    CT_M,
    CT_GC,
};
enum class CommandHelp {
    HELP,
    VERSION,
    SCREEN,
};

const std::unordered_map<std::string, CommandType> COMMAND_MAP = {
    { std::string("-N"), CommandType::CT_N },
    { std::string("-PKG"), CommandType::CT_PKG },
    { std::string("-PID"), CommandType::CT_PID },
    { std::string("-OUT"), CommandType::CT_OUT },
    { std::string("-c"), CommandType::CT_C },
    { std::string("-g"), CommandType::CT_G },
    { std::string("-f"), CommandType::CT_F },
    { std::string("-t"), CommandType::CT_T },
    { std::string("-p"), CommandType::CT_P },
    { std::string("-r"), CommandType::CT_R },
    { std::string("-trace"), CommandType::CT_TTRACE },
    { std::string("-snapshot"), CommandType::CT_SNAPSHOT },
    { std::string("-hw"), CommandType::CT_HW },
    { std::string("-d"), CommandType::CT_D },
    { std::string("-INTERVAL"), CommandType::CT_INTERVAL },
    { std::string("-SESSIONID"), CommandType::CT_SESSIONID },
    { std::string("-net"), CommandType::CT_NET },
    { std::string("-VIEW"), CommandType::CT_VIEW },
    { std::string("-fl"), CommandType::CT_FL },
    { std::string("-ftl"), CommandType::CT_FTL },
    { std::string("-m"), CommandType::CT_M },
    { std::string("-gc"), CommandType::CT_GC },
};

const std::unordered_map<CommandType, std::string> COMMAND_MAP_REVERSE = {
    { CommandType::CT_N, std::string("-N") },
    { CommandType::CT_PKG, std::string("-PKG") },
    { CommandType::CT_PID, std::string("-PID") },
    { CommandType::CT_OUT, std::string("-OUT") },
    { CommandType::CT_C, std::string("-c") },
    { CommandType::CT_G, std::string("-g") },
    { CommandType::CT_F, std::string("-f") },
    { CommandType::CT_T, std::string("-t") },
    { CommandType::CT_P, std::string("-p") },
    { CommandType::CT_R, std::string("-r") },
    { CommandType::CT_TTRACE, std::string("-trace") },
    { CommandType::CT_SNAPSHOT, std::string("-snapshot") },
    { CommandType::CT_HW, std::string("-hw") },
    { CommandType::CT_D, std::string("-d") },
    { CommandType::CT_INTERVAL, std::string("-INTERVAL") },
    { CommandType::CT_SESSIONID, std::string("-SESSIONID") },
    { CommandType::CT_NET, std::string("-net") },
    { CommandType::CT_VIEW, std::string("-VIEW") },
    { CommandType::CT_FL, std::string("-fl") },
    { CommandType::CT_FTL, std::string("-ftl") },
    { CommandType::CT_M, std::string("-m") },
    { CommandType::CT_GC, std::string("-gc") },
};


const std::unordered_map<CommandHelp, std::string> COMMAND_HELP_MAP = {
    { CommandHelp::HELP, std::string("--help") },
    { CommandHelp::VERSION, std::string("--version") },
    { CommandHelp::SCREEN, std::string("-screen") },
};

enum class TraceStatus {
    TRACE_START,
    TRACE_FINISH,
    TRACE_NO
};
}
}
#endif