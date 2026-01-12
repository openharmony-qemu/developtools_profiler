/*
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

#include "profiler_command_executor.h"
#include "profiler_config_manager.h"
#include "profiler_session_manager.h"
#include "profiler_process_manager.h"
#include "profiler_command_parser.h"
#include "common.h"
#include <unistd.h>
#include <sys/types.h>
#include <cstdio>
#include <cstring>
#include <future>
#include "logging.h"

namespace {
constexpr int PIPE_READ_END = 0;
constexpr int PIPE_WRITE_END = 1;
int g_pipefd[2];
}

int ProfilerCommandExecutor::Run(int argc, char* argv[])
{
    if (!COMMON::GetDeveloperMode()) {
        return -1;
    }

    ProfilerCommandArgs args;
    ProfilerCommandParser& parser = ProfilerCommandParser::GetInstance();
    
    if (!parser.ParseArguments(argc, argv, args) || args.isHelp) {
        parser.PrintHelp();
        return 0;
    }

    // Handle stop command
    if (args.commandType == CommandType::STOP) {
        HandleStopCommand();
        ProfilerProcessManager::GetInstance().KillDependentProcess();
        return 0;
    }

    // Normal command execution
    std::string config = "";
    if (!LoadConfigFromArgs(args, config)) {
        return -1;
    }

    // Handle start process command
    if (args.isStartProcess && !HandleStartProcessCommand(args)) {
        if (args.isKillProcess) {
            ProfilerProcessManager::GetInstance().KillDependentProcess();
        }
        return 0;
    }

    // Handle get grpc address command
    if (args.isGetGrpcAddr) {
        int ret = HandleGetGrpcAddrCommand(args);
        if (args.isKillProcess) {
            ProfilerProcessManager::GetInstance().KillDependentProcess();
        }
        return ret;
    }

    // Handle show plugin list command
    if (args.isShowPluginList) {
        int ret = HandleShowPluginListCommand(args);
        if (args.isKillProcess) {
            ProfilerProcessManager::GetInstance().KillDependentProcess();
        }
        return ret;
    }

    // Handle empty config case
    if (config.empty()) {
        return HandleEmptyConfig(args, config);
    }

    // Delete old output file
    remove(args.outputFile.c_str());

    // Handle start command
    if (args.commandType == CommandType::START) {
        std::string config = "";
        if (!LoadConfigFromArgs(args, config)) {
            return -1;
        }
        return HandleStartCommand(args, config);
    }
    // Execute profiler capture
    ExecuteProfilerCapture(args, config);
    
    if (args.isKillProcess && (!args.isNonBlock)) {
        ProfilerProcessManager::GetInstance().KillDependentProcess();
    }
    
    return 0;
}

bool ProfilerCommandExecutor::LoadConfigFromArgs(const ProfilerCommandArgs& args, std::string& config)
{
    if (!args.stdinConfig.empty()) {
        config = args.stdinConfig;
        return true;
    }
    
    if (!args.configFile.empty()) {
        return ProfilerConfigManager::GetInstance().ParseConfig(args.configFile, config);
    }
    
    return true;
}

bool ProfilerCommandExecutor::HandleStartProcessCommand(const ProfilerCommandArgs& args)
{
    if (!ProfilerProcessManager::GetInstance().StartDependentProcess()) {
        return false;
    }
    return true;
}

bool ProfilerCommandExecutor::HandleGetGrpcAddrCommand(const ProfilerCommandArgs& args)
{
    return ProfilerSessionManager::GetInstance().CheckServiceConnection();
}

int ProfilerCommandExecutor::HandleShowPluginListCommand(const ProfilerCommandArgs& args)
{
    std::string content = "";
    ProfilerSessionManager::GetInstance().GetCapabilities(content, true);
    return 0;
}

int ProfilerCommandExecutor::HandleEmptyConfig(const ProfilerCommandArgs& args, const std::string& config)
{
    if (args.isKillProcess) {
        ProfilerProcessManager::GetInstance().KillDependentProcess();
        return 1;
    }
    if (!args.isStartProcess) {
        printf("config file argument must sepcified!\n");
    }
    return 1;
}

void ProfilerCommandExecutor::ExecuteProfilerCapture(const ProfilerCommandArgs& args, const std::string& config)
{
    if (args.isNonBlock) {
        if (pipe(g_pipefd) == -1) {
            printf("pipe error\n");
            return;
        }
        
        pid_t childPid = fork();
        if (childPid == 0) {
            // Child process
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            if (setsid() < 0) {
                close(g_pipefd[PIPE_READ_END]);
                close(g_pipefd[PIPE_WRITE_END]);
                PROFILER_LOG_ERROR(LOG_CORE, "setsid error");
                return;
            }
            
            close(g_pipefd[PIPE_READ_END]);
            uint32_t sigFlag = 1;
            int retVal = TEMP_FAILURE_RETRY(write(g_pipefd[PIPE_WRITE_END], &sigFlag, sizeof(sigFlag)));
            if (retVal <= 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "write signal flag failed");
                close(g_pipefd[PIPE_WRITE_END]);
                return;
            }
            close(g_pipefd[PIPE_WRITE_END]);
            
            if (ProfilerSessionManager::GetInstance().Capture(config, args.traceKeepSecond, args.outputFile)) {
                printf("DONE\n");
            }
            if (args.isKillProcess && (!args.isNonBlock)) {
                ProfilerProcessManager::GetInstance().KillDependentProcess();
            }
        } else if (childPid > 0) {
            // Parent process
            close(g_pipefd[PIPE_WRITE_END]);
            uint32_t exitFlag = 0;
            ssize_t retVal = TEMP_FAILURE_RETRY(read(g_pipefd[PIPE_READ_END], &exitFlag, sizeof(exitFlag)));
            if (retVal == -1 && errno == EAGAIN) {
                close(g_pipefd[PIPE_READ_END]);
                return;
            }
            if (static_cast<size_t>(retVal) != sizeof(exitFlag)) {
                PROFILER_LOG_ERROR(LOG_CORE, "read exitFlag error");
            }
            if (exitFlag == 1) {
                printf("Running in nonblock mode: tracing %s s....\n", args.traceKeepSecond.c_str());
                PROFILER_LOG_INFO(LOG_CORE, "Running in nonblock mode: tracing %s s....\n",
                                  args.traceKeepSecond.c_str());
            }
            close(g_pipefd[PIPE_READ_END]);
        }
    } else {
        // Blocking mode
        if (ProfilerSessionManager::GetInstance().Capture(config, args.traceKeepSecond, args.outputFile)) {
            printf("DONE\n");
        }
    }
}

int ProfilerCommandExecutor::HandleStartCommand(const ProfilerCommandArgs& args, const std::string& config)
{
    // For start command, set maximum duration to 3600 seconds
    std::string duration = "3600";
    if (!args.traceKeepSecond.empty()) {
        if (COMMON::IsNumeric(args.traceKeepSecond)) {
            duration = args.traceKeepSecond;
        }
    }

    // Delete old output file
    if (!args.outputFile.empty()) {
        remove(args.outputFile.c_str());
    }

    // Start profiling in background (non-blocking)
    ProfilerSessionManager& sessionMgr = ProfilerSessionManager::GetInstance();
    
    // Use CaptureLongRunning for start command
    uint32_t sessionId = sessionMgr.CaptureLongRunning(config, duration, args.outputFile);
    if (sessionId == 0) {
        printf("Failed to start profiling session\n");
        return -1;
    }

    printf("Profiling started with session ID: %u\n", sessionId);
    printf("Maximum duration: %s seconds\n", duration.c_str());
    printf("Output file: %s\n", args.outputFile.empty() ? "default" : args.outputFile.c_str());
    printf("Use 'hiprofiler_cmd stop' to stop profiling\n");
    
    return 0;
}

void ProfilerCommandExecutor::HandleStopCommand()
{
    printf("Stopping profiling session 0...\n");
    
    ProfilerSessionManager& sessionMgr = ProfilerSessionManager::GetInstance();
    if (sessionMgr.StopAllSessions()) {
        printf("Profiling session stopped successfully\n");
        return;
    } else {
        printf("Failed to stop profiling session or session not found\n");
        return;
    }
}
