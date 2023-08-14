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

#include "command_line.h"
#include <cstdint>
#include <cstdio>
#include <list>
#include <map>
#include <ostream>
#include "command_param.h"
#include "command_param_switch.h"
#include "command_param_text.h"

CommandLine& CommandLine::GetInstance()
{
    static CommandLine commandLine;
    return commandLine;
}

CommandLine::CommandLine()
{
    paramIdx_ = 0;
}
CommandLine::~CommandLine() {}

void CommandLine::AddParamText(const std::string& filter1,
                               const std::string& filter2,
                               std::string& pstr,
                               const std::string& desc)
{
    auto pret = std::make_shared<CommandParamText>(pstr);
    if (!filter1.empty()) {
        pret->AddFilter(filter1);
    }
    if (!filter2.empty()) {
        pret->AddFilter(filter2);
    }
    pret->paramType_ = PARAM_TYPE_TEXT;
    pret->sDescriptor_ = desc;
    commandParams_[paramIdx_++] = pret;
}

void CommandLine::AddParamSwitch(const std::string& filter1,
                                 const std::string& filter2,
                                 bool &pbool,
                                 const std::string& desc)
{
    auto pret = std::make_shared<CommandParamSwitch>(pbool);
    if (!filter1.empty()) {
        pret->AddFilter(filter1);
    }
    if (!filter2.empty()) {
        pret->AddFilter(filter2);
    }
    pret->paramType_ = PARAM_TYPE_SWITCH;
    pret->sDescriptor_ = desc;
    commandParams_[paramIdx_++] = pret;
}

namespace {
const int USED_PARAM_ONE = 1;
const int USED_PARAM_TWO = 2;
const int USED_PARAM_ERR = 99999;

const int MAX_TAB_COUNT = 2;
} // namespace

int CommandLine::CheckParam(const std::string& s1, const std::string& s2)
{
    for (auto it = commandParams_.begin(); it != commandParams_.end(); ++it) {
        auto p = it->second;
        if (!p->IsInFilter(s1)) {
            continue;
        }
        switch (p->paramType_) {
            case PARAM_TYPE_SWITCH: {
                CommandParamSwitch* pswitch = (CommandParamSwitch*)p.get();
                pswitch->SetValue(true);
                return USED_PARAM_ONE;
            }
                break;
            case PARAM_TYPE_TEXT: {
                CommandParamText* ptext = (CommandParamText*)p.get();
                if (s2 != "") {
                    ptext->SetValue(s2);
                    return USED_PARAM_TWO;
                }
                printf("%s lost content\n", s1.c_str());
                return USED_PARAM_ERR;
            }
                break;
            default:
                printf("unknown type\n");
                break;
        }
    }
    printf("unknown param '%s'\n", s1.c_str());
    return USED_PARAM_ERR;
}
int CommandLine::AnalyzeParam(std::vector<std::string> argv)
{
    size_t i = 1;
    while (i < argv.size()) {
        i += CheckParam(argv[i], (i + 1 < argv.size()) ? argv[i + 1] : "");
    }
    if (i >= USED_PARAM_ERR) {
        return -1;
    }
    return 1;
}

void CommandLine::PrintHelp()
{
    printf("%shelp :\n", VT100_RED);
    for (auto it = commandParams_.begin(); it != commandParams_.end(); ++it) {
        auto p = it->second;

        int i = 0;
        for (std::string temp : p->paramFilter_) {
            i++;
            if (i == 1) {
                printf("  %-16s", temp.c_str());
            } else {
                printf(" %-2s", temp.c_str());
                break;
            }
        }
        for (; i < MAX_TAB_COUNT; i++) {
            printf("   ");
        }
        printf("     : %s\n", p->sDescriptor_.c_str());
    }
    printf("%s", VT100_CLOSE);
}
