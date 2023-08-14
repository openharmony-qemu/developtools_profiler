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

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

#include <vector>
#include <map>
#include <memory>
#include "command_param.h"

#ifdef VT100_ENABLE
#define VT100_RED "\033[31m"
#define VT100_GREEN "\033[32m"
#define VT100_YELLOW "\033[33m"
#define VT100_BLUE "\033[34m"
#define VT100_PURPLE "\033[35m"
#define VT100_CYAN "\033[36m"
#define VT100_WHITE "\033[37m"

#define VT100_CLOSE "\033[0m"
#define VT100_BRIGHT "\033[1m"
#define VT100_UNDERLINE "\033[4m"
#define VT100_FLASH "\033[5m"
#else
#define VT100_RED ""
#define VT100_GREEN ""
#define VT100_YELLOW ""
#define VT100_BLUE ""
#define VT100_PURPLE ""
#define VT100_CYAN ""
#define VT100_WHITE ""

#define VT100_CLOSE ""
#define VT100_BRIGHT ""
#define VT100_UNDERLINE ""
#define VT100_FLASH ""
#endif

class CommandLine {
public:
    static CommandLine& GetInstance();

    int AnalyzeParam(std::vector<std::string> argv);

    void AddParamSwitch(const std::string& filter1, const std::string& filter2, bool &pbool, const std::string& desc);
    void AddParamText(const std::string& filter1,
                      const std::string& filter2,
                      std::string& pstr,
                      const std::string& desc);

    void PrintHelp();

private:
    CommandLine();
    ~CommandLine();

    int paramIdx_;
    std::map<uint32_t, std::shared_ptr<CommandParam>> commandParams_;

    int CheckParam(const std::string& s1, const std::string& s2);
};

#endif