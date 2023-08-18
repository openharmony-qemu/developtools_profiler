/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#include <chrono>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <linux/perf_event.h>
#include <linux/bpf.h>
#include <sys/resource.h>

#include "bpf_controller.h"
#include "commandline_flags.h"
#include "hhlog.h"
#include "command_helper.h"
#include "ipc_unix_socket.h"

HIEBPF_DEFINE_string(help, "none", "specify what help message to print");
HIEBPF_DEFINE_string(output_file, "/data/local/tmp/hiebpf.data", "the file used to save hiebpf data");
HIEBPF_DEFINE_bool(exclude_tracer, true, "the flag indicates whether to trace the tracer itself or not");
HIEBPF_DEFINE_uint32(dump_events, 0, "maximum number of events to print instead of save into file");
HIEBPF_DEFINE_bool(unwind_stack, true, "whether enable unwinding or not");
HIEBPF_DEFINE_uint32(max_stack_depth, MAX_STACK_DEPTH, "max unwinding stack depth");
HIEBPF_DEFINE_uint32(duration, 0, "the tracer will run for <duration> seconds");
HIEBPF_DEFINE_array(events, string, "events to trace");
HIEBPF_DEFINE_array(pids, uint32_t, "pids of target process to trace");
HIEBPF_DEFINE_string(bpf_log_level, "NONE", "lowest level of BPF programs logs to print");
HIEBPF_DEFINE_string(bpf_log_file, "/data/local/tmp/bpf.log", "the file used to save BPF program logs");
HIEBPF_DEFINE_string(libbpf_log_level, "NONE", "lowest level of libbpf internal error messages to print");
HIEBPF_DEFINE_string(libbpf_log_file, "/data/local/tmp/libbpf.log",
                    "the file used to save libbpf internal error messages");
HIEBPF_DEFINE_string(hhlog_level, "NONE", "lowest level of HHLog to print");
HIEBPF_DEFINE_string(hhlog_file, "/data/local/tmp/hhlog.txt", "the file used to save HHLog");
HIEBPF_DEFINE_bool(start, false, "setup and start hiebpf server to trace");
HIEBPF_DEFINE_bool(stop, false, "stop hiebpf server to trace");

static BPFConfig GenBPFConfigFromFlags(const std::string &cmd);
static int GetHHLogLevelFromFlag();
static inline std::vector<pid_t> GetTargetPidsFromFlag();
static std::unique_ptr<BPFController> bpfController;
static OHOS::Developtools::Hiebpf::IpcUnixSocketServer g_ipcServer;
static bool g_interrupted = false;

const uint32_t IPC_RECV_TIMEOUT = 1000;

enum HiebpfIpcCommand:uint32_t {
    CMD_START = 0,
    CMD_STOP,
    CMD_HEART_BEAT,
    RET_OK = 0x10000,
};

static void HandleIpcMessage(const void *data, size_t size)
{
    if (size != sizeof(HiebpfIpcCommand)) {
        HHLOGE(true, "unknown command");
        return;
    }

    HiebpfIpcCommand cmd = *(const_cast<HiebpfIpcCommand*>(reinterpret_cast<const HiebpfIpcCommand*>(data)));
    if (cmd == HiebpfIpcCommand::CMD_STOP) {
        g_interrupted = true;
        if (bpfController != nullptr) {
            bpfController->Stop();
        }
        cmd = HiebpfIpcCommand::RET_OK;
        if (!g_ipcServer.SendMessage(&cmd, sizeof(HiebpfIpcCommand))) {
            HHLOGE(true, "send reply(0x%x) failed", cmd);
        }
    } else if (cmd == HiebpfIpcCommand::CMD_HEART_BEAT) {
        cmd = HiebpfIpcCommand::RET_OK;
        if (!g_ipcServer.SendMessage(&cmd, sizeof(HiebpfIpcCommand))) {
            HHLOGE(true, "send reply(0x%x) failed\n", cmd);
        }
    } else {
        HHLOGW(true, "the command(0x%x) don't support\n", cmd);
    }
}

static bool SendIpcCommand(OHOS::Developtools::Hiebpf::IpcUnixSocketClient &client,
                           HiebpfIpcCommand cmd, uint32_t timeout)
{
    CHECK_TRUE(client.SendMessage(&cmd, sizeof(HiebpfIpcCommand)), false, "send request(0x%x) failed\n", cmd);

    size_t size = OHOS::Developtools::Hiebpf::UNIX_SOCKET_BUFFER_SIZE;
    char buf[OHOS::Developtools::Hiebpf::UNIX_SOCKET_BUFFER_SIZE] = {0};
    CHECK_TRUE(client.RecvMessage(buf, size, timeout), false, "recv reply failed\n");
    if (size == 0) { // timeout
        HHLOGE(true, "recv reply timeout(%dms)\n", timeout);
        return false;
    } else if (size != sizeof(HiebpfIpcCommand)) {
        HHLOGE(true, "recv reply size error(%zu)\n", size);
        return false;
    }
    HiebpfIpcCommand reply = *(reinterpret_cast<HiebpfIpcCommand*>(buf));
    CHECK_TRUE(reply == HiebpfIpcCommand::RET_OK, false, "recv unknown reply: 0x%x\n", reply);
    return true;
}

static void StopHiebpfServer()
{
    OHOS::Developtools::Hiebpf::IpcUnixSocketClient ipcClient;
    if (!ipcClient.Connect()) {
        printf("ipc client connect failed\n");
        return;
    }

    if (SendIpcCommand(ipcClient, HiebpfIpcCommand::CMD_STOP, IPC_RECV_TIMEOUT)) {
        printf("send STOP command to hiebpf server sucess");
        fflush(stdout);
        while (SendIpcCommand(ipcClient, HiebpfIpcCommand::CMD_HEART_BEAT, IPC_RECV_TIMEOUT)) {
            sleep(1);
            printf(".");
            fflush(stdout);
        }
        printf("\n");
    } else {
        printf("send STOP command to hiebpf server failed\n");
    }

    ipcClient.Disconnect();
}

static bool CheckHiebpfServer()
{
    sleep(1); // wait hiebpf IPC server started
    OHOS::Developtools::Hiebpf::IpcUnixSocketClient ipcClient;
    if (!ipcClient.Connect()) {
        printf("ipc client connect failed\n");
        return false;
    }

    bool ret = false;
    if (SendIpcCommand(ipcClient, HiebpfIpcCommand::CMD_HEART_BEAT, IPC_RECV_TIMEOUT)) {
        ret = true;
    }

    ipcClient.Disconnect();
    return ret;
}

int main(int argc, char* argv[])
{
    std::string cmd;
    for (int32_t i = 0; i < argc; ++i) {
        cmd += std::string(argv[i]) + " ";
    }
    // parse all defined command line arguments
    if (CommandLineFlags::ParseFlags(&argc, &argv, true) != 0) {
        std::cout << "[ERROR] " << __FILE__ << ":"
            << __FUNCTION__ << ":"
            << __LINE__ << ": "
            << "failed to parse command line flags" << std::endl;
        return -1;
    }

    // check if user issued help, none-zero return value means user does issue
    // help, hence we exit right after printing the required help message.
    if (CommandHelper::GetInstance().Start(&argc, &argv, FLAGS_help) != 0) {
        // Note this is the workflow when user asking for command help, it's not
        // an error. Besides, help workflow never fails.
        return 0;
    }

    // start HHLogger
    if (HHLogger::GetInstance().Start(GetHHLogLevelFromFlag(), FLAGS_hhlog_file) != 0) {
        std::cout << "[ERROR] " << __FILE__ << ":"
            << __FUNCTION__ << ":"
            << __LINE__ << ": "
            << "failed to start HHLogger" << std::endl;
        return -1;
    }

    if (FLAGS_stop) {
        StopHiebpfServer();
        return 0;
    }

    // fork() must be precede BPFController::MakeUnique(),
    // otherwise ring_buffer__poll() will crash. why?
    if (FLAGS_start) {
        pid_t pid = fork();
        if (pid == -1) {
            printf("fork failed. %d:%s\n", errno, strerror(errno));
            return -1;
        } else if (pid == 0) { // child process
            int32_t nullFd = open("/dev/null", O_WRONLY);
            (void)dup2(nullFd, STDOUT_FILENO); // redirect stdout to /dev/null
            g_ipcServer.SetHandleCallback(HandleIpcMessage);
            if (!g_ipcServer.Start()) {
                HHLOGE(true, "ipc server start failed");
                return 0;
            }
        } else { // parent process
            if (CheckHiebpfServer()) {
                printf("hiebpf server has started.\n");
            } else {
                printf("hiebpf server start failed\n");
            }
            return 0;
        }
    }
    HHLogger::GetInstance().LogSaver();
    // start BPFController
    bpfController = BPFController::MakeUnique(GenBPFConfigFromFlags(cmd));
    if (bpfController == nullptr) {
        std::cout << "[ERROR] " << __FILE__ << ":"
            << __FUNCTION__ << ":"
            << __LINE__ << ": "
            << "failed to init BPFController" << std::endl;
        return -1;
    }

    if (g_interrupted) {
        return 0;
    }

    if (bpfController->Start() != 0) {
        std::cout << "[ERROR] " << __FILE__ << ":"
            << __FUNCTION__ << ":"
            << __LINE__ << ": "
            << "failed to start BPFEvent loop" << std::endl;
        return -1;
    }
    bpfController = nullptr;

    if (FLAGS_start) {
        g_ipcServer.Stop();
    }

    return 0;
}

static inline std::vector<pid_t> GetTargetPidsFromFlag()
{
    std::vector<pid_t> pids {};
    for (size_t k = 0; k < FLAGS_pids.size(); ++ k) {
        pids.push_back(static_cast<pid_t>(FLAGS_pids[k]));
    }
    return pids;
}

static inline std::set<HiebpfEventGroup> GetTargetEventsFromFlag()
{
    const static std::unordered_map<std::string, HiebpfEventGroup> eventGroupMap {
        {"fs", FS_GROUP_ALL},
        {"fs:open", FS_GROUP_OPEN},
        {"fs:read", FS_GROUP_READ},
        {"fs:write", FS_GROUP_WRITE},
        {"fs:close", FS_GROUP_CLOSE},
        {"ptrace", MEM_GROUP_ALL},
        {"bio", BIO_GROUP_ALL},
    };

    std::set<HiebpfEventGroup> result {};
    for (size_t k = 0; k < FLAGS_events.size(); ++k) {
        const auto& it = eventGroupMap.find(FLAGS_events[k]);
        if (it == eventGroupMap.end()) {
            // cmdline input error, how notify cmdline?
        } else {
            result.insert(it->second);
        }
    }
    return result;
}

static inline __u32 GetBPFLogLevelFromFlag()
{
    if (FLAGS_bpf_log_level.compare("DEBUG") == 0 or
        FLAGS_bpf_log_level.compare("debug") == 0) {
        return BPF_LOG_DEBUG;
    }
    if (FLAGS_bpf_log_level.compare("INFO") == 0 or
        FLAGS_bpf_log_level.compare("info") == 0) {
        return BPF_LOG_INFO;
    }
    if (FLAGS_bpf_log_level.compare("WARN") == 0 or
        FLAGS_bpf_log_level.compare("warn") == 0) {
        return BPF_LOG_WARN;
    }
    if (FLAGS_bpf_log_level.compare("ERROR") == 0 or
        FLAGS_bpf_log_level.compare("error") == 0) {
        return BPF_LOG_ERROR;
    }
    if (FLAGS_bpf_log_level.compare("FATAL") == 0 or
        FLAGS_bpf_log_level.compare("fatal") == 0) {
        return BPF_LOG_FATAL;
    }
    return BPF_LOG_NONE;
}

static inline int GetLIBBPFLogLevelFromFlag()
{
    if (FLAGS_libbpf_log_level.compare("DEBUG") == 0 or
        FLAGS_libbpf_log_level.compare("debug") == 0) {
        return LIBBPF_DEBUG;
    }
    if (FLAGS_libbpf_log_level.compare("INFO") == 0 or
        FLAGS_libbpf_log_level.compare("info") == 0) {
        return LIBBPF_INFO;
    }
    if (FLAGS_libbpf_log_level.compare("WARN") == 0 or
        FLAGS_libbpf_log_level.compare("warn") == 0) {
        return LIBBPF_WARN;
    }
    if (FLAGS_libbpf_log_level.compare("ERROR") == 0 or
        FLAGS_libbpf_log_level.compare("error") == 0) {
        return LIBBPF_ERROR;
    }
    if (FLAGS_libbpf_log_level.compare("FATAL") == 0 or
        FLAGS_libbpf_log_level.compare("fatal") == 0) {
        return LIBBPF_FATAL;
    }
    return LIBBPF_NONE;
}

static int GetHHLogLevelFromFlag()
{
    if (FLAGS_hhlog_level.compare("DEBUG") == 0 or
        FLAGS_hhlog_level.compare("debug") == 0) {
        return HHLOG_DEBUG;
    }
    if (FLAGS_hhlog_level.compare("INFO") == 0 or
        FLAGS_hhlog_level.compare("info") == 0) {
        return HHLOG_INFO;
    }
    if (FLAGS_hhlog_level.compare("WARN") == 0 or
        FLAGS_hhlog_level.compare("warn") == 0) {
        return HHLOG_WARN;
    }
    if (FLAGS_hhlog_level.compare("ERROR") == 0 or
        FLAGS_hhlog_level.compare("error") == 0) {
        return HHLOG_ERROR;
    }
    if (FLAGS_hhlog_level.compare("FATAL") == 0 or
        FLAGS_hhlog_level.compare("fatal") == 0) {
        return HHLOG_FATAL;
    }
    if (FLAGS_hhlog_level.compare("NONE") == 0 or
        FLAGS_hhlog_level.compare("none") == 0) {
        return HHLOG_NONE;
    }
    return HHLOG_NONE;
}

static inline BPFConfig GenBPFConfigFromFlags(const std::string &cmd)
{
    BPFConfig config {};
    config.cmd_ = cmd;
    config.excludeTracer_ = FLAGS_exclude_tracer;
    config.unwindStack_ = FLAGS_max_stack_depth == 0 ? false : FLAGS_unwind_stack;
    config.dumpEvents_ = FLAGS_dump_events;
    config.traceDuration_ = FLAGS_duration;
    config.maxStackDepth_ = FLAGS_max_stack_depth == 0 ? 1 : FLAGS_max_stack_depth;
    config.targetPids_ = GetTargetPidsFromFlag();
    config.selectEventGroups_ = GetTargetEventsFromFlag();
    config.BPFLogLevel_ = GetBPFLogLevelFromFlag();
    config.LIBBPFLogLevel_ = GetLIBBPFLogLevelFromFlag();
    config.BPFLogFile_ = FLAGS_bpf_log_file;
    config.LIBBPFLogFile_ = FLAGS_libbpf_log_file;
    config.outputFile_ = FLAGS_output_file;
    return config;
}