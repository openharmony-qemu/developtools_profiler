/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
#include <thread>

#include "common.h"
#include "command_poller.h"
#include "hook_manager.h"
#include "logging.h"
#include "plugin_service_types.pb.h"
#include "writer_adapter.h"
#include "hook_standalone.h"
#include "hook_common.h"

using namespace OHOS::Developtools::NativeDaemon;

namespace {
const int SLEEP_ONE_SECOND = 1000;
const int BUF_MAX_LEN = 10;
const int VC_ARG_TWAIN = 2;
const int VC_ARG_STEP_SIZE = 2;
const int SMBSIZE_BASE = 4096;

bool ProcessExist(std::string pid)
{
    int ret = 0;
    std::string pid_path = std::string();
    struct stat stat_buf;
    if (pid.size() == 0) {
        return false;
    }
    pid_path = "/proc/" + pid + "/status";
    if (stat(pid_path.c_str(), &stat_buf) != 0) {
        return false;
    }
    return true;
}

bool ParseCommand(std::vector<std::string> args, HookData& hookData)
{
    size_t idx = 0;
    while (idx < args.size()) {
        if (args[idx] == "-o") {
            hookData.fileName = args[idx + 1].c_str();
        } else if (args[idx] == "-p") {
            hookData.pid = std::stoi(args[idx + 1], nullptr);
            if (std::to_string(hookData.pid) != args[idx + 1]) {
                return false;
            }
            if (!ProcessExist(args[idx + 1])) {
                printf("process does not exist\n");
                return false;
            }
        } else if (args[idx] == "-n") {
            hookData.processName = args[idx + 1];
        } else if (args[idx] == "-s") {
            hookData.smbSize = std::stoi(args[idx + 1], nullptr);
            if (std::to_string(hookData.smbSize) != args[idx + 1]) {
                return false;
            }
        } else if (args[idx] == "-f") {
            hookData.filterSize = std::stoi(args[idx + 1], nullptr);
            if (std::to_string(hookData.filterSize) != args[idx + 1]) {
                return false;
            }
            if (hookData.filterSize > MAX_UNWIND_DEPTH) {
                printf("set max depth = %d\n", MAX_UNWIND_DEPTH);
            }
        } else if (args[idx] == "-d") {
            hookData.maxStackDepth = std::stoi(args[idx + 1], nullptr);
            if (std::to_string(hookData.maxStackDepth) != args[idx + 1]) {
                return false;
            }
        } else if (args[idx] == "-L") {
            if (idx + 1 < args.size()) {
                hookData.duration = std::stoull(args[idx + 1]);
            }
        } else if (args[idx] == "-F") {
            if (idx + 1 < args.size()) {
                hookData.performance_filename = args[idx + 1];
            }
        } else if (args[idx] == "-u") {
            std::string unwind = args[idx + 1];
            if (unwind == "dwarf") {
                hookData.fpUnwind = false;
            } else if (unwind == "fp") {
                hookData.fpUnwind = true;
            } else {
                return false;
            }
            printf("set unwind mode:%s\n", unwind.c_str());
        } else if (args[idx] == "-S") {
            hookData.statisticsInterval = std::stoi(args[idx + 1], nullptr);
            if (std::to_string(hookData.statisticsInterval) != args[idx + 1]) {
                return false;
            }
        } else if (args[idx] == "-O") {
            std::string offline = args[idx + 1];
            if (offline == "false") {
                hookData.offlineSymbolization = false;
            } else if (offline == "true") {
                hookData.offlineSymbolization = true;
            } else {
                return false;
            }
            printf("set offlineSymbolization mode:%s\n", offline.c_str());
        } else if (args[idx] == "-C") {
            std::string callframeCompress = args[idx + 1];
            if (callframeCompress == "false") {
                hookData.callframeCompress = false;
            } else if (callframeCompress == "true") {
                hookData.callframeCompress = true;
            } else {
                return false;
            }
            printf("set callframeCompress mode:%s\n", callframeCompress.c_str());
        } else if (args[idx] == "-c") {
            std::string stringCompressed = args[idx + 1];
            if (stringCompressed == "false") {
                hookData.stringCompressed = false;
            } else if (stringCompressed == "true") {
                hookData.stringCompressed = true;
            } else {
                return false;
            }
            printf("set stringCompressed mode:%s\n", stringCompressed.c_str());
        } else if (args[idx] == "-r") {
            std::string rawString = args[idx + 1];
            if (rawString == "false") {
                hookData.rawString = false;
            } else if (rawString == "true") {
                hookData.rawString = true;
            } else {
                return false;
            }
            printf("set rawString mode:%s\n", rawString.c_str());
        } else {
            printf("args[%zu] = %s\n", idx, args[idx].c_str());
            return false;
        }
        idx += VC_ARG_STEP_SIZE;
    }
    return true;
}

bool VerifyCommand(std::vector<std::string> args, HookData& hookData)
{
    if ((args.size() % VC_ARG_TWAIN) != 0) {
        return false;
    }
    hookData.pid = 0;
    hookData.duration = 0;
    hookData.performance_filename = "./performance.txt";
    hookData.fileName = "";
    if (!ParseCommand(args, hookData)) {
        return false;
    }
    if ((hookData.smbSize % SMBSIZE_BASE) != 0) {
        printf("Please configure a multiple of 4096 for the shared memory size\n");
        return false;
    }
    if (!hookData.fileName.empty() && (!hookData.processName.empty() || hookData.pid > 0)) {
        return true;
    }
    return false;
}

void GetHookedProceInfo(HookData& hookData)
{
    printf("Record file = %s, apply sharememory size = %u\n", hookData.fileName.c_str(), hookData.smbSize);
    if (hookData.pid > 0) {
        printf("Please send signal 36 to target process %d\n", hookData.pid);
    } else if (!hookData.processName.empty()) {
        int pidValue = -1;
        const std::string processName = hookData.processName;
        bool isExist = COMMON::IsProcessExist(processName, pidValue);
        if (!isExist) {
            printf("Please start process %s\n", hookData.processName.c_str());
        } else {
            printf("If you want to hook current process, please send signal 36 to target process %d\n", pidValue);
            printf("If you want to hook new process, please\n");
            printf("1.param set libc.hook_mode startup:%s\n", hookData.processName.c_str());
            printf("2.restart process %s\n", hookData.processName.c_str());
        }
    }

    if (hookData.maxStackDepth > 0) {
        printf("depth greater than %u will not display\n", hookData.maxStackDepth);
    }
    if (hookData.filterSize > 0) {
        printf("malloc size smaller than %u will not record\n", hookData.filterSize);
    }

    if (!OHOS::Developtools::Profiler::Hook::StartHook(hookData)) {
        return;
    }
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_ONE_SECOND));
    }
}
} // namespace

int main(int argc, char* argv[])
{
    int lockFileFd = -1;
    if (COMMON::IsProcessRunning(lockFileFd)) { // process is running
        return 0;
    }

    if (argc > 1) {
        std::vector<std::string> args;
        for (int i = 1; i < argc; i++) {
            args.push_back(argv[i]);
        }
        HookData hookData = {0, 0, 0, 0, MAX_UNWIND_DEPTH, 0, "", "", "",
                             false, false, false, false, false,
                             false, false, false, false};
        if (VerifyCommand(args, hookData)) {
            GetHookedProceInfo(hookData);
        } else {
            std::string help = R"(Usage: native_daemon
            [-o file]
            [-s smb_size]
            <-n process_name>
            <-f filter_size>
            <-d max_stack_depth>
            <-u fp|dwarf>
            <-S statistics_interval>
            <-O offline_symbolization true|false>
            <-C callframe_compress true|false>
            <-c string_compressed true|false>
            <-r raw_string true|false>
            )";
            printf("%s\n", help.c_str());
            return 0;
        }
    } else {
        auto hookManager = std::make_shared<HookManager>();
        CHECK_NOTNULL(hookManager, 1, "create PluginManager FAILED!");

        auto commandPoller = std::make_shared<CommandPoller>(hookManager);
        CHECK_NOTNULL(commandPoller, 1, "create CommandPoller FAILED!");
        CHECK_TRUE(commandPoller->OnConnect(), 1, "connect FAILED");
        hookManager->SetCommandPoller(commandPoller);
        hookManager->RegisterAgentPlugin("nativehook");
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_ONE_SECOND));
        }
    }
    close(lockFileFd);
    return 0;
}
