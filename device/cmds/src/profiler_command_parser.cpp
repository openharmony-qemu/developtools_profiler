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

#include "profiler_command_parser.h"
#include "command_line.h"
#include <getopt.h>
#include <cstring>
#include <iostream>
#include "logging.h"
#include <unistd.h>  // for optind
#include "profiler_config_manager.h"

namespace {
    constexpr int DOUBLE = 2;
}
ProfilerCommandParser& ProfilerCommandParser::GetInstance()
{
    static ProfilerCommandParser instance;
    return instance;
}

bool ProfilerCommandParser::ParseArguments(int argc, char* argv[], ProfilerCommandArgs& args)
{
    // Check for subcommands (start/stop)
    if (argc >= DOUBLE) {
        std::string firstArg = argv[1];
        if (firstArg == "start") {
            args.commandType = CommandType::START;
            // Parse remaining arguments as if they were normal command arguments
            // Shift argv to skip "start"
            int newArgc = argc - 1;
            char** newArgv = new char*[newArgc + 1];
            newArgv[0] = argv[0];  // program name
            for (int i = 1; i < newArgc; i++) {
                newArgv[i] = argv[i + 1];  // Skip "start"
            }
            newArgv[newArgc] = nullptr;
            
            bool result = ParseNormalArguments(newArgc, newArgv, args, true);
            delete[] newArgv;
            return result;
        } else if (firstArg == "stop") {
            args.commandType = CommandType::STOP;
            return true;  // stop command doesn't need additional arguments
        }
    }
    
    // Normal command parsing
    args.commandType = CommandType::NORMAL;
    return ParseNormalArguments(argc, argv, args);
}

bool ProfilerCommandParser::ParseNormalArguments(int argc, char* argv[], ProfilerCommandArgs& args,
                                                 bool isStart)
{
    // Handle stdin config case and nonblock flag
    int optionIndex = 0;
    optind = 1;  // Reset getopt
    while (true) {
        struct option long_options[] = {
            {"getport", no_argument, nullptr, 'q'},
            {"time", required_argument, nullptr, 't'},
            {"out", required_argument, nullptr, 'o'},
            {"help", no_argument, nullptr, 'h'},
            {"list", no_argument, nullptr, 'l'},
            {"start", no_argument, nullptr, 's'},
            {"kill", no_argument, nullptr, 'k'},
            {"nonblock", no_argument, nullptr, 0},
            {"config", required_argument, nullptr, 'c'},
            {nullptr, 0, nullptr, 0}
        };
        
        int option = getopt_long(argc, argv, "c:t:o:qhlsk", long_options, &optionIndex);
        if (option == -1) {
            break;
        }
        
        if (option == 0) {
            // Handle long option without short equivalent (nonblock)
            if (long_options[optionIndex].name && strcmp(long_options[optionIndex].name, "nonblock") == 0) {
                args.isNonBlock = true;
            }
            continue;
        }
        
        std::string optionStr("qtohlskc");
        if (optionIndex == 0 && optionStr.find(option) == std::string::npos) {
            printf("invalid param\n");
            return false;
        }
        
        if (option == 'c' && optarg && strcmp(optarg, "-") == 0) {
            if (!HandleSpecialArgument(option, optarg, args)) {
                return false;
            }
        }
    }
    
    // Build argv vector excluding stdin config case
    std::vector<std::string> argvVector;
    bool startCommandExit = false;
    for (int i = 0; i < argc; i++) {
        if (isStart && ((strcmp(argv[i], "-t") == 0) || strcmp(argv[i], "--time") == 0)) {
            printf("hiprofiler_cmd start command shouldn't use time parameter\n");
            startCommandExit = true;
        }
        if (((i + 1) < argc) && (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) &&
            (strcmp(argv[i + 1], "-") == 0)) {
            i++;
        } else {
            argvVector.push_back(argv[i]);
        }
    }

    // Use CommandLine to parse arguments
    CommandLine& cmdLine = CommandLine::GetInstance();
    
    // Register all parameters with CommandLine
    cmdLine.AddParamSwitch("--getport", "-q", args.isGetGrpcAddr, "get grpc address");
    cmdLine.AddParamText("--time", "-t", args.traceKeepSecond, "trace time");
    cmdLine.AddParamText("--out", "-o", args.outputFile, "output file name");
    cmdLine.AddParamSwitch("--help", "-h", args.isHelp, "make some help");
    cmdLine.AddParamSwitch("--list", "-l", args.isShowPluginList, "plugin list");
    cmdLine.AddParamSwitch("--start", "-s", args.isStartProcess, "start dependent process");
    cmdLine.AddParamSwitch("--kill", "-k", args.isKillProcess, "kill dependent process");
    cmdLine.AddParamText("--config", "-c", args.configFile, "start trace by config file");
    cmdLine.AddParamSwitch("--nonblock", "", args.isNonBlock, "start capture without block");
    
    if (argc < 1 || cmdLine.AnalyzeParam(argvVector) < 0 || startCommandExit) {
        return false;
    }
    
    return true;
}

bool ProfilerCommandParser::HandleSpecialArgument(char option, const char* optarg, ProfilerCommandArgs& args)
{
    if (option == 'c' && strcmp(optarg, "-") == 0) {
        ProfilerConfigManager::GetInstance().ReadConfigFromStdin(args.stdinConfig);
        return true;
    }
    return false;
}

void ProfilerCommandParser::PrintHelp() const
{
    CommandLine::GetInstance().PrintHelp();
}
