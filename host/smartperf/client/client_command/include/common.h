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
    GET_CUR_FPS,
    GET_LOW_POWER_FPS,
    FPS_STOP,
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
    { MessageType::GET_LOW_POWER_FPS, std::string("get_low_power_fps") },
    { MessageType::GET_CUR_FPS, std::string("get_cur_fps") },
    { MessageType::FPS_STOP, std::string("fps_stop") },
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
    CT_GC,
    CT_NAV,
};
enum class CommandHelp {
    HELP,
    VERSION,
    SCREEN,
    CLEAR,
    SERVER,
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
    { std::string("-gc"), CommandType::CT_GC },
    { std::string("-nav"), CommandType::CT_NAV },
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
    { CommandType::CT_GC, std::string("-gc") },
    { CommandType::CT_NAV, std::string("-nav") },
};


const std::unordered_map<CommandHelp, std::string> COMMAND_HELP_MAP = {
    { CommandHelp::HELP, std::string("--help") },
    { CommandHelp::VERSION, std::string("--version") },
    { CommandHelp::SCREEN, std::string("-screen") },
    { CommandHelp::CLEAR, std::string("-clear") },
    { CommandHelp::SERVER, std::string("-server") },
};

enum class TraceStatus {
    TRACE_START,
    TRACE_FINISH,
    TRACE_NO
};

enum class CmdCommand {
    HITRACE_1024,
    HITRACE_2048,
    HITRACE_CMD,
    CAPTURE_FILE,
    SNAPSHOT,
    SERVER,
    OHTESTFPS,
    RM_FILE,
    UITEST_DUMPLAYOUT,
    UINPUT_POINT,
    DUBAI_CP,
    DUBAI_CHMOD,
    TASKSET,
    PROC_STAT,
    HIPROFILER,
    PERF,
    WRITE_PATH,
    HIPROFILER_CMD,
    HIPROFILER_PID,
    KILL_CMD,
    PIDOF_SP,
    SERVER_GREP,
    EDITOR_SERVER_GREP,
    UINPUT_BACK,
    TIMESTAMPS,
};

const std::unordered_map<CmdCommand, std::string> CMD_COMMAND_MAP = {
    { CmdCommand::HITRACE_1024, std::string(
        "hitrace --trace_clock mono -t 10 -b 102400 --overwrite idle ace app ohos ability graphic "
        "nweb sched freq sync workq multimodalinput > ") },
    { CmdCommand::HITRACE_2048, std::string(
        "hitrace --trace_clock mono -t 10 -b 204800 --overwrite idle ace app ohos ability graphic "
        "nweb sched freq sync workq multimodalinput > ") },
    { CmdCommand::HITRACE_CMD, std::string("ps -ef |grep hitrace |grep -v grep") },
    { CmdCommand::CAPTURE_FILE, std::string("mkdir -p /data/local/tmp/capture") },
    { CmdCommand::SNAPSHOT, std::string("snapshot_display -f ") },
    { CmdCommand::SERVER, std::string("SP_daemon -server") },
    { CmdCommand::OHTESTFPS, std::string("SP_daemon -ohtestfps 10") },
    { CmdCommand::RM_FILE, std::string("rm -rfv /data/local/tmp/") },
    { CmdCommand::UITEST_DUMPLAYOUT, std::string("uitest dumpLayout") },
    { CmdCommand::UINPUT_POINT, std::string("uinput -T -d ") },
    { CmdCommand::DUBAI_CP, std::string(
        "cp /data/service/el2/100/xpower/dubai.db /data/app/el2/100/database/com.ohos.smartperf/entry/rdb") },
    { CmdCommand::DUBAI_CHMOD, std::string(
        "chmod 777 /data/app/el2/100/database/com.ohos.smartperf/entry/rdb/dubai.db") },
    { CmdCommand::TASKSET, std::string("taskset -p f ") },
    { CmdCommand::PROC_STAT, std::string("chmod o+r /proc/stat") },
    { CmdCommand::HIPROFILER, std::string("rm -f /data/local/tmp/hiprofiler_[0-9]*.htrace") },
    { CmdCommand::PERF, std::string("rm -f /data/local/tmp/perf_[0-9]*.data") },
    { CmdCommand::WRITE_PATH, std::string("mkdir -p /data/local/tmp/smartperf/") },
    { CmdCommand::HIPROFILER_CMD, std::string("ps -ef |grep hiprofiler_cmd |grep -v grep") },
    { CmdCommand::HIPROFILER_PID, std::string("pidof hiprofiler_cmd") },
    { CmdCommand::KILL_CMD, std::string("kill ") },
    { CmdCommand::PIDOF_SP, std::string("pidof SP_daemon") },
    { CmdCommand::SERVER_GREP, std::string("ps -ef | grep -v grep | grep 'SP_daemon -server'") },
    { CmdCommand::EDITOR_SERVER_GREP, std::string("ps -ef | grep -v grep | grep 'SP_daemon -editorServer'") },
    { CmdCommand::UINPUT_BACK, std::string("uinput -T -m 600 2760 600 1300 200") },
    { CmdCommand::TIMESTAMPS, std::string("timestamps") },
};

enum class DeviceCmd {
    SN,
    DEVICET_NAME,
    BRAND,
    VERSION,
    ABILIST,
    NAME,
    MODEL,
    FULL_NAME,
};
const std::unordered_map<DeviceCmd, std::string> DEVICE_CMD_MAP = {
    { DeviceCmd::SN, std::string("param get ohos.boot.sn") },
    { DeviceCmd::DEVICET_NAME, std::string("param get ohos.boot.hardware") },
    { DeviceCmd::BRAND, std::string("param get const.product.brand") },
    { DeviceCmd::VERSION, std::string("param get const.product.software.version") },
    { DeviceCmd::ABILIST, std::string("param get const.product.cpu.abilist") },
    { DeviceCmd::NAME, std::string("param get const.product.name") },
    { DeviceCmd::MODEL, std::string("param get const.product.model") },
    { DeviceCmd::FULL_NAME, std::string("param get const.ohos.fullname") },
};

enum class HidumperCmd {
    DUMPER_DUBAI_B,
    DUMPER_DUBAI_F,
    DUMPER_SURFACE,
    DUMPER_HEAD,
    DUMPER_SCREEN,
    DUMPER_A_A,
    DUMPER_NAV,
    DUMPER_MEM,
};
const std::unordered_map<HidumperCmd, std::string> HIDUMPER_CMD_MAP = {
    { HidumperCmd::DUMPER_DUBAI_B, std::string("hidumper -s 1213 -a '-b'") },
    { HidumperCmd::DUMPER_DUBAI_F, std::string("hidumper -s 1213 -a '-f'") },
    { HidumperCmd::DUMPER_SURFACE, std::string("hidumper -s 10 -a surface | grep surface") },
    { HidumperCmd::DUMPER_HEAD, std::string(
        "hidumper -s AbilityManagerService -a '-a' | grep 'bundle name' | head -n 1") },
    { HidumperCmd::DUMPER_SCREEN, std::string("hidumper -s 10 -a screen") },
    { HidumperCmd::DUMPER_A_A, std::string("hidumper -s WindowManagerService -a '-a'") },
    { HidumperCmd::DUMPER_NAV, std::string("hidumper -s WindowManagerService -a '-w ") },
    { HidumperCmd::DUMPER_MEM, std::string("hidumper --mem ") },
};

enum class HisyseventCmd {
    HISYS_APP_START,
    HISYS_JANK,
    HISYS_RESPONSE,
    HISYS_COMPLETED,
    HISYSEVENT,
    HISYS_PID,
};
const std::unordered_map<HisyseventCmd, std::string> HISYSEVENT_CMD_MAP = {
    { HisyseventCmd::HISYS_APP_START, std::string("hisysevent -r -o PERFORMANCE -n APP_START") },
    { HisyseventCmd::HISYS_JANK, std::string("hisysevent -r -o PERFORMANCE -n INTERACTION_JANK") },
    { HisyseventCmd::HISYS_RESPONSE, std::string("hisysevent -r -n INTERACTION_RESPONSE_LATENCY") },
    { HisyseventCmd::HISYS_COMPLETED, std::string("hisysevent -r -n INTERACTION_COMPLETED_LATENCY") },
    { HisyseventCmd::HISYSEVENT, std::string("ps -ef |grep hisysevent") },
    { HisyseventCmd::HISYS_PID, std::string("pidof hisysevent") },
};
}
}
#endif