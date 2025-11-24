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
#include <cstdio>
#include "profiler_config_manager.h"
#include "profiler_process_manager.h"
#include "profiler_session_manager.h"

bool ProfilerCommandExecutor::LoadConfigFromArgs(const ProfilerCommandArgs& args, std::string& config)
{
    config = args.stdinConfig;
    if (config.empty() && !args.configFile.empty()) {
        return ProfilerConfigManager::GetInstance().ParseConfig(args.configFile, config);
    }
    return true;
}

int ProfilerCommandExecutor::HandleStartProcessCommand(const ProfilerCommandArgs& args)
{
    if (!args.isStartProcess) {
        return -1;
    }
    
    if (!ProfilerProcessManager::GetInstance().StartDependentProcess()) {
        if (args.isKillProcess) {
            ProfilerProcessManager::GetInstance().KillDependentProcess();
        }
        return 0;
    }
    return -1;
}

int ProfilerCommandExecutor::HandleGetGrpcAddrCommand(const ProfilerCommandArgs& args)
{
    if (!args.isGetGrpcAddr) {
        return -1;
    }
    
    int ret = ProfilerSessionManager::GetInstance().CheckServiceConnection();
    if (args.isKillProcess) {
        ProfilerProcessManager::GetInstance().KillDependentProcess();
    }
    return ret;
}

int ProfilerCommandExecutor::HandleShowPluginListCommand(const ProfilerCommandArgs& args)
{
    if (!args.isShowPluginList) {
        return -1;
    }
    
    std::string content;
    ProfilerSessionManager::GetInstance().GetCapabilities(content, true);
    if (args.isKillProcess) {
        ProfilerProcessManager::GetInstance().KillDependentProcess();
    }
    return 0;
}

int ProfilerCommandExecutor::HandleEmptyConfig(const ProfilerCommandArgs& args, const std::string& config)
{
    if (!config.empty()) {
        return -1;
    }
    
    if (!args.isStartProcess) {
        printf("config file argument must specified!\n");
    }
    if (args.isKillProcess) {
        ProfilerProcessManager::GetInstance().KillDependentProcess();
    }
    return 1;
}

void ProfilerCommandExecutor::ExecuteProfilerCapture(const ProfilerCommandArgs& args, const std::string& config)
{
    if (ProfilerSessionManager::GetInstance().Capture(config, args.traceKeepSecond, args.outputFile)) {
        printf("DONE\n");
    }
    
    if (args.isKillProcess) {
        ProfilerProcessManager::GetInstance().KillDependentProcess();
    }
}

int ProfilerCommandExecutor::Run(int argc, char* argv[])
{
    ProfilerCommandArgs args;
    if (!ProfilerCommandParser::GetInstance().ParseArguments(argc, argv, args)) {
        return -1;
    }
    
    if (args.isHelp) {
        ProfilerCommandParser::GetInstance().PrintHelp();
        return 0;
    }

    remove(args.outputFile.c_str());

    std::string config;
    if (!LoadConfigFromArgs(args, config)) {
        return -1;
    }

    int ret = HandleStartProcessCommand(args);
    if (ret != -1) {
        return ret;
    }

    ret = HandleGetGrpcAddrCommand(args);
    if (ret != -1) {
        return ret;
    }

    ret = HandleShowPluginListCommand(args);
    if (ret != -1) {
        return ret;
    }

    ret = HandleEmptyConfig(args, config);
    if (ret != -1) {
        return ret;
    }
    
    ExecuteProfilerCapture(args, config);
    return 0;
}

