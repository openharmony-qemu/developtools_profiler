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

#ifndef PROFILER_COMMAND_PARSER_H
#define PROFILER_COMMAND_PARSER_H

#include <string>

enum class CommandType {
    NORMAL,      // Normal command execution
    START,       // hiprofiler_cmd start <params>
    STOP         // hiprofiler_cmd stop
};

struct ProfilerCommandArgs {
    CommandType commandType = CommandType::NORMAL;
    bool isGetGrpcAddr = false;
    std::string traceKeepSecond;
    std::string configFile;
    std::string outputFile;
    bool isHelp = false;
    bool isShowPluginList = false;
    bool isStartProcess = false;
    bool isKillProcess = false;
    bool isNonBlock = false;
    std::string stdinConfig;
};

class ProfilerCommandParser {
public:
    static ProfilerCommandParser& GetInstance();
    ProfilerCommandParser(const ProfilerCommandParser&) = delete;
    ProfilerCommandParser& operator=(const ProfilerCommandParser&) = delete;
    bool ParseArguments(int argc, char* argv[], ProfilerCommandArgs& args);
    void PrintHelp() const;

private:
    ProfilerCommandParser() = default;
    ~ProfilerCommandParser() = default;

    bool ParseNormalArguments(int argc, char* argv[], ProfilerCommandArgs& args, bool isStart = false);
    bool HandleSpecialArgument(char option, const char* optarg, ProfilerCommandArgs& args);
};

#endif // PROFILER_COMMAND_PARSER_H

