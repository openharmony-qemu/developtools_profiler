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

#ifndef PROFILER_COMMAND_EXECUTOR_H
#define PROFILER_COMMAND_EXECUTOR_H

#include <string>
#include "profiler_command_parser.h"

class ProfilerCommandExecutor {
public:
    ProfilerCommandExecutor() = default;
    ~ProfilerCommandExecutor() = default;

    ProfilerCommandExecutor(const ProfilerCommandExecutor&) = delete;
    ProfilerCommandExecutor& operator=(const ProfilerCommandExecutor&) = delete;

    int Run(int argc, char* argv[]);

private:
    bool LoadConfigFromArgs(const ProfilerCommandArgs& args, std::string& config);
    bool HandleStartProcessCommand(const ProfilerCommandArgs& args);
    bool HandleGetGrpcAddrCommand(const ProfilerCommandArgs& args);
    int HandleShowPluginListCommand(const ProfilerCommandArgs& args);
    int HandleEmptyConfig(const ProfilerCommandArgs& args, const std::string& config);
    void ExecuteProfilerCapture(const ProfilerCommandArgs& args, const std::string& config);
    int HandleStartCommand(const ProfilerCommandArgs& args, const std::string& config);
    void HandleStopCommand();
};

#endif // PROFILER_COMMAND_EXECUTOR_H

