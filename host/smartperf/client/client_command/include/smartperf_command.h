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
        "-------------------------------------------------------------------------------------\n"
        "These are common commands list:\n"
        " -N             set the collection times, for example: -N 10 \n"
        " -PKG           set package name, must add, for example: -PKG ohos.samples.ecg \n"
        " -c             get device cpuFreq and cpuUsage, process cpuUsage and cpuLoad .. \n"
        " -g             get device gpuFreq and gpuLoad  \n"
        " -f             get app refresh fps and fps jitters \n"
        " -profilerfps    get refresh fps and timestamp \n"
        " -t             get soc-temp battery-temp .. \n"
        " -p             get current_now and voltage_now \n"
        " -r             get process memory and total memory .. \n"
        " -snapshot      get screen capture\n"
        " -net           get networkUp and networkDown\n"
        " -start          collection start command \n"
        " -stop          collection stop command \n"
        " -VIEW          set layler, for example: -VIEW DisplayNode \n"
        " -screen        get screen resolution \n"
        "-------------------------------------------------------------------------------------\n"
        "Example 1: SP_daemon -N 20 -c -g -t -p -r -net -snapshot \n"
        "-------------------------------------------------------------------------------------\n"
        "-------------------------------------------------------------------------------------\n"
        "Example 2: SP_daemon -N 20 -PKG ohos.samples.ecg -c -g -t -p -f -r -net -snapshot \n"
        "-------------------------------------------------------------------------------------\n"
        "-------------------------------------------------------------------------------------\n"
        "Example 3: SP_daemon -start -c \n"
        "-------------------------------------------------------------------------------------\n"
         "-------------------------------------------------------------------------------------\n"
        "Example 4: SP_daemon -stop \n"
        "-------------------------------------------------------------------------------------\n"
             "-------------------------------------------------------------------------------------\n"
        "Example 5: SP_daemon -screen \n"
        "-------------------------------------------------------------------------------------\n";
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
    // 图层名
    std::string layerName = "";
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