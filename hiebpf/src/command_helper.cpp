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

#include <iostream>

#include "command_helper.h"

int CommandHelper::Start(int* argc, char*** argv, const std::string& help)
{
    if ((*argc) == 1) {
        // no argument specified, we speculate the user may do not know the command usage.
        // Hence print the usage help and continue
        std::cout << tipHelp_ << std::endl;
        return 0;
    }
    if (help.compare(SUPPORTED_ARGS[ARG_NONG]) == 0) {
        // user does not issue help
        bool printTip {false};
        for (int count = 1; count < (*argc); ++count) {
            if ((*argv)[count] and strlen((*argv)[count]) != 0) {
                // undefined commandline argument specified, we speculate the user may do
                // not know the command usage. Hence print the usage help and continue
                printTip = true;
                std::cout << "CommandHelper WARN: argument " << (*argv)[count]
                          << " specified but unused" << std::endl;
            }
        }
        if (printTip) {
            std::cout << tipHelp_ << std::endl;
        }
        return 0;
    }
    DoHelp(help);
    return -1;
}

void CommandHelper::DoHelp(const std::string& help)
{
    if (help.compare(SUPPORTED_ARGS[ARG_HELP]) == 0) {
        std::cout << helpHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_OUTPUT_FILE]) == 0) {
        std::cout << outputFileHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_EXCLUDE_TRACER]) == 0) {
        std::cout << excludeTracerHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_MAX_STACK_DEPTH]) == 0) {
        std::cout << maxStackDepthHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_DURATION]) == 0) {
        std::cout << durationHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_EVENTS]) == 0) {
        std::cout << eventsHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_PIDS]) == 0) {
        std::cout << pidsHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_DUMP_EVENTS]) == 0) {
        std::cout << dumpEventsHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_UNWIND_STACK]) == 0) {
        std::cout << unwindStackHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_BPF_LOG_LEVEL]) == 0) {
        std::cout << bpfLogLevelHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_BPF_LOG_FILE]) == 0) {
        std::cout << bpfLogFileHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_LIBBPF_LOG_LEVEL]) == 0) {
        std::cout << libbpfLogLevelHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_LIBBPF_LOG_FILE]) == 0) {
        std::cout << libbpfFileHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_SERVER_START]) == 0) {
        std::cout << serverStartHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_SERVER_STOP]) == 0) {
        std::cout << serverStopHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_HHLOG_LEVEL]) == 0) {
        std::cout << hhLogLevelHelp_ << std::endl;
    } else if (help.compare(SUPPORTED_ARGS[ARG_HHLOG_FILE]) == 0) {
        std::cout << hhLogFileHelp_ << std::endl;
    } else {
        std::cout << tipHelp_ << std::endl;
    }
    return;
}