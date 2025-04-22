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
        SET_PKG_NAME,
        GET_TEMPERATURE,
        SET_DUBAI_DB,
        START_DUBAI_DB,
        SET_GAME_VIEW,
        GET_APP_TYPE,
        GET_DAEMON_VERSION,
        APP_START_COLLECT,
        APP_STOP_COLLECT,
        APP_RECEIVE_DATA_ON,
        APP_RECEIVE_DATA_OFF,
        APP_PAUSE_COLLECT,
        APP_RESUME_COLLECT,
        GET_INDEX_INFO
    };

const std::unordered_map<MessageType, std::string> MESSAGE_MAP = {
    { MessageType::SET_PKG_NAME, std::string("set_pkgName") },
    { MessageType::GET_TEMPERATURE, std::string("get_temperature") },
    { MessageType::SET_DUBAI_DB, std::string("set_dubai_db") },
    { MessageType::START_DUBAI_DB, std::string("start_dubai_db") },
    { MessageType::SET_GAME_VIEW, std::string("set_game_view") },
    { MessageType::GET_APP_TYPE, std::string("get_app_type") },
    { MessageType::APP_START_COLLECT, std::string("app_start_collect") },
    { MessageType::APP_STOP_COLLECT, std::string("app_stop_collect") },
    { MessageType::APP_RECEIVE_DATA_ON, std::string("app_receive_data_on") },
    { MessageType::APP_RECEIVE_DATA_OFF, std::string("app_receive_data_off") },
    { MessageType::APP_PAUSE_COLLECT, std::string("app_pause_collect") },
    { MessageType::APP_RESUME_COLLECT, std::string("app_resume_collect") },
    { MessageType::GET_INDEX_INFO, std::string("get_index_info") },
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
    CT_FDS,
    CT_T,
    CT_P,
    CT_PRINT,
    CT_R,
    CT_TTRACE,
    CT_THREADS,
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
    CT_O,
    CT_LF,
    CT_AS,
};
enum class CommandHelp {
    HELP,
    VERSION,
    SCREEN,
    CLEAR,
    CLEARALL,
    SERVER,
    EDITORSERVER,
    DEVICESERVER,
};

const std::unordered_map<std::string, CommandType> COMMAND_MAP = {
    { std::string("-N"), CommandType::CT_N },
    { std::string("-PKG"), CommandType::CT_PKG },
    { std::string("-PID"), CommandType::CT_PID },
    { std::string("-OUT"), CommandType::CT_OUT },
    { std::string("-c"), CommandType::CT_C },
    { std::string("-CPU"), CommandType::CT_C },
    { std::string("-g"), CommandType::CT_G },
    { std::string("-GPU"), CommandType::CT_G },
    { std::string("-f"), CommandType::CT_F },
    { std::string("-FPS"), CommandType::CT_F },
    { std::string("-LOW_POWER"), CommandType::CT_F },
    { std::string("-fds"), CommandType::CT_FDS },
    { std::string("-FDS"), CommandType::CT_FDS },
    { std::string("-t"), CommandType::CT_T },
    { std::string("-TEMP"), CommandType::CT_T },
    { std::string("-p"), CommandType::CT_P },
    { std::string("-POWER"), CommandType::CT_P },
    { std::string("-print"), CommandType::CT_PRINT },
    { std::string("-r"), CommandType::CT_R },
    { std::string("-RAM"), CommandType::CT_R },
    { std::string("-trace"), CommandType::CT_TTRACE },
    { std::string("-threads"), CommandType::CT_THREADS },
    { std::string("-snapshot"), CommandType::CT_SNAPSHOT },
    { std::string("-SCREEN"), CommandType::CT_SNAPSHOT },
    { std::string("-hw"), CommandType::CT_HW },
    { std::string("-d"), CommandType::CT_D },
    { std::string("-DDR"), CommandType::CT_D },
    { std::string("-INTERVAL"), CommandType::CT_INTERVAL },
    { std::string("-SESSIONID"), CommandType::CT_SESSIONID },
    { std::string("-net"), CommandType::CT_NET },
    { std::string("-NET"), CommandType::CT_NET },
    { std::string("-VIEW"), CommandType::CT_VIEW },
    { std::string("-fl"), CommandType::CT_FL },
    { std::string("-ftl"), CommandType::CT_FTL },
    { std::string("-gc"), CommandType::CT_GC },
    { std::string("-GPU_COUNTER"), CommandType::CT_GC },
    { std::string("-nav"), CommandType::CT_NAV },
    { std::string("-o"), CommandType::CT_O },
    { std::string("-lockfreq"), CommandType::CT_LF },
    { std::string("-aischedule"), CommandType::CT_AS },
};

const std::unordered_map<CommandType, std::string> COMMAND_MAP_REVERSE = {
    { CommandType::CT_N, std::string("-N") },
    { CommandType::CT_PKG, std::string("-PKG") },
    { CommandType::CT_PID, std::string("-PID") },
    { CommandType::CT_OUT, std::string("-OUT") },
    { CommandType::CT_C, std::string("-c") },
    { CommandType::CT_G, std::string("-g") },
    { CommandType::CT_F, std::string("-f") },
    { CommandType::CT_FDS, std::string("-fds") },
    { CommandType::CT_T, std::string("-t") },
    { CommandType::CT_P, std::string("-p") },
    { CommandType::CT_PRINT, std::string("-print") },
    { CommandType::CT_R, std::string("-r") },
    { CommandType::CT_TTRACE, std::string("-trace") },
    { CommandType::CT_THREADS, std::string("-threads") },
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
    { CommandType::CT_O, std::string("-o") },
    { CommandType::CT_LF, std::string("-lockfreq") },
    { CommandType::CT_AS, std::string("-aischedule") },
};


const std::unordered_map<CommandHelp, std::string> COMMAND_HELP_MAP = {
    { CommandHelp::HELP, std::string("--help") },
    { CommandHelp::VERSION, std::string("--version") },
    { CommandHelp::SCREEN, std::string("-screen") },
    { CommandHelp::CLEAR, std::string("-clear") },
    { CommandHelp::CLEARALL, std::string("-clearAll") },
    { CommandHelp::SERVER, std::string("-server") },
    { CommandHelp::EDITORSERVER, std::string("-editorServer") },
    { CommandHelp::DEVICESERVER, std::string("-deviceServer") },
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
    CREAT_DIR,
    SNAPSHOT,
    SERVER,
    OHTESTFPS,
    RM_FILE,
    TASKSET,
    PROC_STAT,
    HIPROFILER,
    PERF,
    HIPROFILER_CMD,
    HIPROFILER_PID,
    KILL_CMD,
    PIDOF_SP,
    SERVER_GREP,
    EDITOR_SERVER_GREP,
    UINPUT_BACK,
    TIMESTAMPS,
    USER_PERMISSIONS,
    REMOVE,
    CP,
    TAR,
    GET_HILOG,
};

const std::unordered_map<CmdCommand, std::string> CMD_COMMAND_MAP = {
    { CmdCommand::HITRACE_1024, std::string(
        "hitrace --trace_clock mono -t 10 -b 102400 --overwrite idle ace app ohos ability graphic "
        "nweb sched freq sync workq multimodalinput > ") },
    { CmdCommand::HITRACE_2048, std::string(
        "hitrace --trace_clock mono -t 10 -b 204800 --overwrite idle ace app ohos ability graphic "
        "nweb sched freq sync workq multimodalinput > ") },
    { CmdCommand::HITRACE_CMD, std::string("ps -ef |grep hitrace |grep -v grep") },
    { CmdCommand::CREAT_DIR, std::string("mkdir -m 777 ") },
    { CmdCommand::SNAPSHOT, std::string("snapshot_display -f ") },
    { CmdCommand::SERVER, std::string("SP_daemon -server") },
    { CmdCommand::OHTESTFPS, std::string("SP_daemon -ohtestfps 10") },
    { CmdCommand::RM_FILE, std::string("rm -rfv /data/local/tmp/") },
    { CmdCommand::TASKSET, std::string("taskset -p f ") },
    { CmdCommand::PROC_STAT, std::string("chmod o+r /proc/stat") },
    { CmdCommand::HIPROFILER, std::string("rm -f /data/local/tmp/hiprofiler_[0-9]*.htrace") },
    { CmdCommand::PERF, std::string("rm -f /data/local/tmp/perf_[0-9]*.data") },
    { CmdCommand::HIPROFILER_CMD, std::string("ps -ef |grep hiprofiler_cmd |grep -v grep") },
    { CmdCommand::HIPROFILER_PID, std::string("pidof hiprofiler_cmd") },
    { CmdCommand::KILL_CMD, std::string("kill ") },
    { CmdCommand::PIDOF_SP, std::string("pidof SP_daemon") },
    { CmdCommand::SERVER_GREP, std::string("ps -ef | grep -v grep | grep 'SP_daemon -server'") },
    { CmdCommand::EDITOR_SERVER_GREP, std::string("ps -ef | grep -v grep | grep 'SP_daemon -editorServer'") },
    { CmdCommand::UINPUT_BACK, std::string("uinput -K -d 2076 -d 2020 -u 2076 -u 2020") },
    { CmdCommand::TIMESTAMPS, std::string("timestamps") },
    { CmdCommand::USER_PERMISSIONS, std::string("whoami") },
    { CmdCommand::REMOVE, std::string("rm -rf ") },
    { CmdCommand::CP, std::string("cp ") },
    { CmdCommand::TAR, std::string("tar -czf ") },
    { CmdCommand::GET_HILOG, std::string("timeout 1s hilog > ") },
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
    HISYS_SCROLL_ANIMATION,
};
const std::unordered_map<HisyseventCmd, std::string> HISYSEVENT_CMD_MAP = {
    { HisyseventCmd::HISYS_APP_START, std::string("hisysevent -r -o PERFORMANCE -n APP_START") },
    { HisyseventCmd::HISYS_JANK, std::string("hisysevent -r -o PERFORMANCE -n INTERACTION_JANK") },
    { HisyseventCmd::HISYS_SCROLL_ANIMATION, std::string("hisysevent -r -o PERFORMANCE -n SCROLL_ANIMATION") },
    { HisyseventCmd::HISYS_RESPONSE, std::string("hisysevent -r -n INTERACTION_RESPONSE_LATENCY") },
    { HisyseventCmd::HISYS_COMPLETED, std::string("hisysevent -r -n INTERACTION_COMPLETED_LATENCY") },
    { HisyseventCmd::HISYSEVENT, std::string("ps -ef |grep hisysevent") },
    { HisyseventCmd::HISYS_PID, std::string("pidof hisysevent") },
};
}
}
#endif