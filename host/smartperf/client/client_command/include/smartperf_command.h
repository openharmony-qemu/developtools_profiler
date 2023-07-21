/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef SMARTPERF_COMMAND_H
#define SMARTPERF_COMMAND_H

#include <iostream>
#include <vector>
#include "common.h"

namespace OHOS {
namespace SmartPerf {
class SmartPerfCommand {
public:
    const std::string smartPerfExeName = "SP_daemon";
    const std::string smartPerfVersion = "1.0.2\n";
    const std::string smartPerfMsgErr = "error input!\n use command '--help' get more information\n";
    const std::string smartPerfMsg = "usage: SP_daemon <options> <arguments> \n"
        "--------------------------------------------------------------------\n"
        "These are common commands list:\n"
        " -N             set num of profiler <must be non-null>\n"
        " -PKG           set pkgname of profiler \n"
        " -PID           set process id of profiler \n"
        " -OUT           set output path of CSV\n"
        " -c             get cpuFreq and cpuLoad  \n"
        " -g             get gpuFreq and gpuLoad  \n"
        " -f             get fps and fps jitters \n"
        " -t             get soc-temp gpu-temp .. \n"
        " -p             get current_now and voltage_now \n"
        " -r             get ram(pss) \n"
        " -snapshot      get screen capture\n"
        "--------------------------------------------------------------------\n"
        "Example: SP_daemon -N 20 -PKG ohos.samples.ecg -c -g -t -p -f \n"
        "--------------------------------------------------------------------\n";
    const int oneParam = 1;
    const int twoParam = 2;
    const int threeParamMore = 3;
    SmartPerfCommand(int argc, char *argv[]);
    ~SmartPerfCommand() {};
    static void InitSomething();
    std::string ExecCommand();
    void HelpCommand(CommandHelp type) const;
    void HandleCommand(std::string argStr, std::string argStr1);
    // 采集次数
    int num = 0;
    // 包名
    std::string pkgName = "";
    // 是否开启trace 抓取
    int trace = 0;
    // csv输出路径
    std::string outPath = "/data/local/tmp/data.csv";
    std::string outPathParam = "";
    // 指定进程pid
    std::string pid = "";
    // 采集配置项
    std::vector<std::string> configs;
};
}
}
#endif // SMARTPERF_COMMAND_H