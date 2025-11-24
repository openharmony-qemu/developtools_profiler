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
#include <cstdio>
#include <cstring>
#include <getopt.h>
#include <iostream>
#include <istream>
#include <iterator>
#include "parse_plugin_config.h"

ProfilerCommandParser& ProfilerCommandParser::GetInstance()
{
    static ProfilerCommandParser instance;
    return instance;
}

bool ProfilerCommandParser::HandleSpecialArgument(char option, const char* optarg, ProfilerCommandArgs& args)
{
    if (option == 'c' && optarg != nullptr && strcmp(optarg, "-") == 0) {
        std::string content;
        std::istreambuf_iterator<char> begin(std::cin);
        std::istreambuf_iterator<char> end = {};
        content.assign(begin, end);
        
        args.stdinConfig = ParsePluginConfig::GetInstance().GetPluginsConfig(content);
        if (args.stdinConfig.empty()) {
            printf("Please check the configuration!\n");
            return false;
        }
    }
    return true;
}

bool ProfilerCommandParser::ParseArguments(int argc, char* argv[], ProfilerCommandArgs& args)
{
    while (true) {
        struct option long_options[] = {
            {"getport", no_argument, nullptr, 'q'},
            {"time", required_argument, nullptr, 't'},
            {"out", required_argument, nullptr, 'o'},
            {"help", no_argument, nullptr, 'h'},
            {"list", no_argument, nullptr, 'l'},
            {"start", no_argument,  nullptr, 's'},
            {"kill", no_argument,  nullptr, 'k'},
            {"config", required_argument, nullptr, 'c'},
            {nullptr, 0, nullptr, 0}
        };
        int option = getopt_long(argc, argv, "c:t:o:qhlsk", long_options, nullptr);
        if (option == -1) {
            break;
        }
        
        switch (option) {
            case 'q':
                args.isGetGrpcAddr = true;
                break;
            case 't':
                args.traceKeepSecond = optarg ? optarg : "";
                break;
            case 'o':
                args.outputFile = optarg ? optarg : "";
                break;
            case 'h':
                args.isHelp = true;
                break;
            case 'l':
                args.isShowPluginList = true;
                break;
            case 's':
                args.isStartProcess = true;
                break;
            case 'k':
                args.isKillProcess = true;
                break;
            case 'c':
                if (optarg != nullptr) {
                    if (strcmp(optarg, "-") == 0) {
                        if (!HandleSpecialArgument(option, optarg, args)) {
                            return false;
                        }
                    } else {
                        args.configFile = optarg;
                    }
                }
                break;
            default:
                printf("invalid param: %c\n", option);
                return false;
        }
    }
    
    return true;
}

void ProfilerCommandParser::RegisterAllArguments()
{
}

void ProfilerCommandParser::PrintHelp() const
{
    printf("Usage: hiprofiler [OPTIONS]\n");
    printf("\n");
    printf("Options:\n");
    printf("  --getport, -q      Get gRPC address and port\n");
    printf("  --time, -t N       Trace time in seconds\n");
    printf("  --out, -o FILE     Output file name\n");
    printf("  --help, -h         Show this help message\n");
    printf("  --list, -l         Show plugin list\n");
    printf("  --start, -s        Start dependent processes\n");
    printf("  --kill, -k         Kill dependent processes\n");
    printf("  --config, -c FILE  Start trace by config file (use '-' for stdin)\n");
}
