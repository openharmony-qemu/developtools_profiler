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
#include "sp_utils.h"

namespace OHOS {
namespace SmartPerf {
class SmartPerfCommand {
public:
    const std::string smartPerfExeName = "SP_daemon";
    const std::string smartPerfVersion = "1.0.2\n";
    const std::string smartPerfMsgErr = "error input!\n use command '--help' get more information\n";
    const std::string smartPerfMsg = "OpenHarmony performance testing tool SmartPerf command-line version\n"
        "Usage: SP_daemon [options] [arguments]\n\n"
        "options:\n"
        " -N             set the collection times(default value is 0) range[1,2147483647], for example: -N 10 \n"
        " -PKG           set package name, must add, for example: -PKG ohos.samples.ecg \n"
        " -c             get device CPU frequency and CPU usage, process CPU usage and CPU load .. \n"
        " -g             get device GPU frequency and GPU load  \n"
        " -f             get app refresh fps(frames per second) and fps jitters and refreshrate \n"
        " -profilerfps   get refresh fps and timestamp \n"
        " -sections      set collection time period(using with profilerfps)\n"
        " -t             get remaining battery power and temperature.. \n"
        " -p             get battery power consumption and voltage \n"
        " -r             get process memory and total memory \n"
        " -snapshot      get screen capture\n"
        " -net           get uplink and downlink traffic\n"
        " -start         collection start command \n"
        " -stop          collection stop command \n"
        " -VIEW          set layler, for example: -VIEW DisplayNode \n"
        " -screen        get screen resolution \n"
        " -OUT           set csv output path.\n"
        " -d             get device DDR information \n"
        " -nav           get page navigation info \n"
        "example:\n"
        "SP_daemon -N 20 -c -g -t -p -r -net -snapshot -d \n"
        "SP_daemon -N 20 -PKG ohos.samples.ecg -c -g -t -p -f -r -net -snapshot -d -nav \n"
        "SP_daemon -start -c \n"
        "SP_daemon -stop \n"
        "SP_daemon -screen \n";

    const size_t oneParam = 1;
    const size_t twoParam = 2;
    const size_t threeParamMore = 3;
    explicit SmartPerfCommand(std::vector<std::string> argv);
    ~SmartPerfCommand() {};
    static void InitSomething();
    std::string ExecCommand();
    void HelpCommand(CommandHelp type) const;
    void HandleCommand(std::string argStr, const std::string &argStr1);
    int GetItemInfo(std::multimap<std::string, std::string, decltype(SPUtils::Cmp) *> &spMap);
    void PrintfExecCommand(const std::map<std::string, std::string> data) const;
    void PrintMap(std::multimap<std::string, std::string, decltype(SPUtils::Cmp) *> &spMap, int index) const;
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