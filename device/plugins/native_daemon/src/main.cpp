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
#include <sys/file.h>
#include "common.h"
#include "command_poller.h"
#include "hook_manager.h"
#include "logging.h"
#include "plugin_service_types.pb.h"
#include "writer_adapter.h"
#include "hook_standalone.h"
#include "hook_common.h"
#include "native_memory_profiler_sa_service.h"

using namespace OHOS::Developtools::NativeDaemon;

namespace {
const int SLEEP_ONE_SECOND = 1000;
const int VC_ARG_TWAIN = 2;
const int VC_ARG_STEP_SIZE = 2;
const int SMBSIZE_BASE = 4096;

bool ProcessExist(const std::string pid)
{
    std::string pid_path = "";
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

bool ParseCommand(const std::vector<std::string>& args, HookData& hookData)
{
    size_t idx = 0;
    while (idx < args.size()) {
        if (args[idx] == "-o") {
            hookData.fileName = args[idx + 1].c_str();
        } else if (args[idx] == "-p") {
            std::vector<std::string> pids = StringSplit(args[idx + 1], ",");
            hookData.pids.insert(pids.begin(), pids.end());
            for (auto iter = hookData.pids.begin(); iter != hookData.pids.end();) {
                if (!ProcessExist(*iter)) {
                    iter = hookData.pids.erase(iter);
                    printf("process does not exist %s\n", iter->c_str());
                } else {
                    ++iter;
                }
            }
            if (hookData.pids.empty()) {
                printf("all process does not exist\n");
                return false;
            }
        } else if (args[idx] == "-n") {
            hookData.processName = args[idx + 1];
        } else if (args[idx] == "-s") {
            hookData.smbSize = static_cast<uint32_t>(IsDigits(args[idx + 1]) ? std::stoi(args[idx + 1]) : 0);
            if (std::to_string(hookData.smbSize) != args[idx + 1]) {
                return false;
            }
        } else if (args[idx] == "-f") {
            hookData.filterSize = static_cast<uint32_t>(IsDigits(args[idx + 1]) ? std::stoi(args[idx + 1]) : 0);
            if (std::to_string(hookData.filterSize) != args[idx + 1]) {
                return false;
            }
            if (hookData.filterSize > MAX_UNWIND_DEPTH) {
                printf("set max depth = %d\n", MAX_UNWIND_DEPTH);
            }
        } else if (args[idx] == "-d") {
            hookData.maxStackDepth = static_cast<uint32_t>(IsDigits(args[idx + 1]) ? std::stoi(args[idx + 1]) : 0);
            if (std::to_string(hookData.maxStackDepth) != args[idx + 1]) {
                return false;
            }
        } else if (args[idx] == "-L") {
            if (idx + 1 < args.size()) {
                hookData.duration = std::stoull(args[idx + 1]);
            }
        } else if (args[idx] == "-F") {
            if (idx + 1 < args.size()) {
                hookData.performanceFilename = args[idx + 1];
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
            hookData.statisticsInterval = static_cast<uint32_t>(IsDigits(args[idx + 1]) ?
                                                                std::stoi(args[idx + 1]) : 0);
            if (std::to_string(hookData.statisticsInterval) != args[idx + 1]) {
                return false;
            }
        } else if (args[idx] == "-i") {
            hookData.sampleInterval = static_cast<uint32_t>(IsDigits(args[idx + 1]) ? std::stoi(args[idx + 1]) : 0);
            if (std::to_string(hookData.sampleInterval) != args[idx + 1]) {
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
        } else if (args[idx] == "-so") {
            std::string rawString = args[idx + 1];
            if (rawString == "false") {
                hookData.responseLibraryMode = false;
            } else if (rawString == "true") {
                hookData.responseLibraryMode = true;
            } else {
                return false;
            }
            printf("set responseLibraryMode mode:%s\n", rawString.c_str());
        } else if (args[idx] == "-js") {
            hookData.jsStackReport = IsDigits(args[idx + 1]) ? std::stoi(args[idx + 1]) : 0;
            if (std::to_string(hookData.jsStackReport) != args[idx + 1]) {
                return false;
            }
        } else if (args[idx] == "-jsd") {
            hookData.maxJsStackdepth = static_cast<uint32_t>(IsDigits(args[idx + 1]) ? std::stoi(args[idx + 1]) : 0);
            if (std::to_string(hookData.maxJsStackdepth) != args[idx + 1]) {
                return false;
            }
        } else if (args[idx] == "-jn") {
            hookData.filterNapiName = args[idx + 1];
        } else if (args[idx] == "-mfm") {
            hookData.mallocFreeMatchingInterval = static_cast<uint32_t>(IsDigits(args[idx + 1]) ?
                                                                        std::stoi(args[idx + 1]) : 0);
            if (std::to_string(hookData.mallocFreeMatchingInterval) != args[idx + 1]) {
                return false;
            }
        } else {
            printf("args[%zu] = %s\n", idx, args[idx].c_str());
            return false;
        }
        idx += VC_ARG_STEP_SIZE;
    }
    return true;
}

bool VerifyCommand(const std::vector<std::string>& args, HookData& hookData)
{
    if ((args.size() % VC_ARG_TWAIN) != 0) {
        return false;
    }
    if (!ParseCommand(args, hookData)) {
        return false;
    }
    if ((hookData.smbSize % SMBSIZE_BASE) != 0) {
        printf("Please configure a multiple of 4096 for the shared memory size\n");
        return false;
    }
    if (!hookData.fileName.empty() && (!hookData.processName.empty() || hookData.pids.size() > 0)) {
        return true;
    }
    return false;
}

volatile sig_atomic_t g_isRunning = true;
void SignalSigintHandler(int sig)
{
    g_isRunning = false;
}

void GetHookedProceInfo(HookData& hookData)
{
    printf("Record file = %s, apply sharememory size = %u\n", hookData.fileName.c_str(), hookData.smbSize);
    if (hookData.pids.size() > 0) {
        for (const auto& pid : hookData.pids) {
            printf("hook target process %s start\n", pid.c_str());
        }
    } else if (!hookData.processName.empty()) {
        int pidValue = -1;
        const std::string processName = hookData.processName;
        bool isExist = COMMON::IsProcessExist(processName, pidValue);
        if (!isExist) {
            hookData.startupMode = true;
            printf("startup mode ,Please start process %s\n", hookData.processName.c_str());
        } else {
            hookData.pids.emplace(std::to_string(pidValue));
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
    while (g_isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_ONE_SECOND));
    }
    OHOS::Developtools::Profiler::Hook::EndHook();
}
} // namespace

int main(int argc, char* argv[])
{
    int lockFileFd = -1;
    if (COMMON::IsProcessRunning(lockFileFd)) { // process is running
        return 0;
    }

    if (argc > 1) {
        if (argc == 2 && strcmp(argv[1], "sa") == 0) { // 2: argc size
            if (!OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaService::StartServiceAbility()) {
                if (lockFileFd > 0) {
                    flock(lockFileFd, LOCK_UN);
                    close(lockFileFd);
                }
                return 0;
            }
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_ONE_SECOND));
            }
        } else {
            if (!COMMON::GetDeveloperMode()) {
                return 0;
            }
            if (!COMMON::IsBetaVersion()) {
                printf("memory profiler only support in beta version\n");
                if (lockFileFd > 0) {
                    flock(lockFileFd, LOCK_UN);
                    close(lockFileFd);
                }
                return 0;
            }
            std::vector<std::string> args;
            for (int i = 1; i < argc; i++) {
                args.push_back(argv[i]);
            }
            HookData hookData;
            if (VerifyCommand(args, hookData)) {
                signal(SIGINT, SignalSigintHandler);
                GetHookedProceInfo(hookData);
            } else {
                std::string help = R"(Usage: native_daemon
                [-o file]
                [-s smb_size]
                <-n process_name>
                <-p pids>
                <-f filter_size>
                <-d max_stack_depth>
                <-i sample_interval>
                <-u fp|dwarf>
                <-S statistics_interval>
                <-O offline_symbolization true|false>
                <-C callframe_compress true|false>
                <-c string_compressed true|false>
                <-r raw_string true|false>
                <-so responseLibraryMode true|false>
                <-js jsStackReport>
                <-jsd maxJsStackDepth>
                <-jn filterNapiName>
                <-mfm mallocFreeMatchingInterval_>
                )";
                printf("%s\n", help.c_str());
                if (lockFileFd > 0) {
                    flock(lockFileFd, LOCK_UN);
                    close(lockFileFd);
                }
                return 0;
            }
        }
    } else {
        if (!COMMON::GetDeveloperMode()) {
            return 0;
        }
        auto hookManager = std::make_shared<HookManager>();
        if (hookManager == nullptr) {
            if (lockFileFd > 0) {
                flock(lockFileFd, LOCK_UN);
                close(lockFileFd);
                PROFILER_LOG_INFO(LOG_CORE, "create PluginManager FAILED!");
                return 1;
            }
            return 0;
        }
        auto commandPoller = std::make_shared<CommandPoller>(hookManager);
        if (commandPoller == nullptr) {
            if (lockFileFd > 0) {
                flock(lockFileFd, LOCK_UN);
                close(lockFileFd);
                PROFILER_LOG_INFO(LOG_CORE, "create CommandPoller FAILED!");
                return 1;
            }
            return 0;
        }
        if (!commandPoller->OnConnect()) {
            if (lockFileFd > 0) {
                flock(lockFileFd, LOCK_UN);
                close(lockFileFd);
                PROFILER_LOG_INFO(LOG_CORE, "connect FAILED");
                return 1;
            }
            return 0;
        }
        hookManager->SetCommandPoller(commandPoller);
        hookManager->RegisterAgentPlugin("nativehook");

        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_ONE_SECOND));
        }
    }
    if (lockFileFd > 0) {
        flock(lockFileFd, LOCK_UN);
        close(lockFileFd);
    }
    return 0;
}