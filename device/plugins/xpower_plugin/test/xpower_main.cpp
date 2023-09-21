/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <sys/file.h>
#include <unistd.h>
#include <functional>
#include <sstream>
#include <string>

#include "common_types.pb.h"
#include "trace_file_reader.h"
#include "xpower_format.h"
#include "xpower_plugin_result.pb.h"

namespace {
using UsageHandle = std::function<void(void)>;
static std::map<std::string, std::string> params;

int ParseArgs(int argc, char** argv, UsageHandle usage)
{
    params.clear();
    for (int i = 1; i < argc;) {
        std::string key = argv[i];
        i++;
        if (i >= argc) {
            if (usage) usage();
            break;
        }
        std::string val = argv[i];
        i++;
        params.insert(std::make_pair(key, val));
    }
    return params.size();
}

int GetStringArg(const char* name, std::string& val, const char* defaultVal)
{
    val = params[name];
    if (val.empty()) {
        val = defaultVal;
    }
    return val.size();
}

void DisplayRealBattery(OptimizeReport& dataProto)
{
    PrintOutput(
        "power report: real_battery:capacity ={0},charge={1},current={2},gas_gauge={3},"
        "level={4},screen={5}",
        dataProto.real_battery().capacity(), dataProto.real_battery().charge(), dataProto.real_battery().current(),
        dataProto.real_battery().gas_gauge(), dataProto.real_battery().level(), dataProto.real_battery().screen());
}

void DisplayAppStatistic(OptimizeReport& dataProto)
{
    PrintOutput("power report: app_statistic:audio enegy={0},time={1}", dataProto.app_statistic().audio().energy(),
                dataProto.app_statistic().audio().time());

    PrintOutput("power report: app_statistic:bluetooth enegy={0},time={1}",
                dataProto.app_statistic().bluetooth().energy(), dataProto.app_statistic().bluetooth().time());

    PrintOutput("power report: app_statistic:camera enegy={0},time={1}", dataProto.app_statistic().camera().energy(),
                dataProto.app_statistic().camera().time());

    PrintOutput("power report: app_statistic:cpu enegy={0},time={1}", dataProto.app_statistic().cpu().energy(),
                dataProto.app_statistic().cpu().time());

    PrintOutput("power report: app_statistic:display enegy={0},time={1}", dataProto.app_statistic().display().energy(),
                dataProto.app_statistic().display().time());
    PrintOutput("power report: app_statistic:flashlight enegy={0},time={1}",
                dataProto.app_statistic().flashlight().energy(), dataProto.app_statistic().flashlight().time());
    PrintOutput("power report: app_statistic:gpu enegy={0},time={1}", dataProto.app_statistic().gpu().energy(),
                dataProto.app_statistic().gpu().time());

    PrintOutput("power report: app_statistic:location enegy={0},time={1}",
                dataProto.app_statistic().location().energy(), dataProto.app_statistic().location().time());
    PrintOutput("power report: app_statistic:wifiscan enegy={0},time={1}",
                dataProto.app_statistic().wifiscan().energy(), dataProto.app_statistic().wifiscan().time());
}

void DisplayAppDetail(OptimizeReport& dataProto)
{
    PrintOutput("power report: app_detail:cpu thread_count={0}", dataProto.app_detail().cpu().thread_count());
    std::string loadStr;
    for (int i = 0; i < dataProto.app_detail().cpu().thread_load().size(); i++) {
        loadStr += std::to_string(static_cast<long long>(dataProto.app_detail().cpu().thread_load(i))) + ":";
    }
    loadStr.pop_back();
    PrintOutput("power report: app_detail:cpu tread_load={0}", loadStr);
    std::string threadName;
    for (int i = 0; i < dataProto.app_detail().cpu().thread_name().size(); i++) {
        threadName += dataProto.app_detail().cpu().thread_name(i) + ":";
    }
    threadName.pop_back();
    PrintOutput("power report: app_detail:cpu thread_name={0}", threadName);
    PrintOutput("power report: app_detail:gpu frequency_count={0}", dataProto.app_detail().gpu().frequency_count());
    std::string freqStr;
    for (int i = 0; i < dataProto.app_detail().gpu().frequency().size(); i++) {
        freqStr += std::to_string(dataProto.app_detail().gpu().frequency(i)) + ":";
    }
    freqStr.pop_back();
    PrintOutput("power report: app_detail:gpu frequency={0}", freqStr);
    std::string idleStr;
    for (int i = 0; i < dataProto.app_detail().gpu().idle_time().size(); i++) {
        idleStr += std::to_string(dataProto.app_detail().gpu().idle_time(i)) + ":";
    }
    idleStr.pop_back();
    PrintOutput("power report: app_detail:gpu idle_time={0}", idleStr);
    std::string runStr;
    for (int i = 0; i < dataProto.app_detail().gpu().run_time().size(); i++) {
        runStr += std::to_string(dataProto.app_detail().gpu().run_time(i)) + ":";
    }
    runStr.pop_back();
    PrintOutput("power report: app_detail:gpu run_time={0}", runStr);
}

} // namespace

void Usage()
{
    printf("usage: xpowertool.exe [-f filepath] \n");
}
int main(int argc, char* argv[])
{
    std::string filePath;
    int ret = ParseArgs(argc, argv, Usage);
    if (ret == 0) {
        std::cout << "parse parameters error!" << std::endl;
        return 0;
    }
    GetStringArg("-f", filePath, "/data/local/tmp/hiprofiler_data.htrace");
    auto reader = std::make_shared<TraceFileReader>();
    if (!reader->Open(filePath)) {
        std::cout << "open file :" << filePath << "failed!" << std::endl;
        return 0;
    }
    long bytes = 0;
    do {
        ProfilerPluginData data{};
        bytes = reader->Read(data);
        if (data.data().size() <= 0) {
            continue;
        }
        PrintOutput("name = {0},status={1},tv_sec={2},tv_nsec={3},version={4}", data.name(), data.status(),
                    data.tv_sec(), data.tv_nsec(), data.version());
        OptimizeReport dataProto;
        if (!dataProto.ParseFromArray(data.data().data(), data.data().size())) {
            std::cout << "parse profiler plugin data failed!" << std::endl;
            continue;
        }
        PrintOutput("power report :recive optimize start_time ={0},end_time={1},bundle_name={2}",
                    dataProto.start_time(), dataProto.end_time(), dataProto.bundle_name());
        DisplayRealBattery(dataProto);
        DisplayAppDetail(dataProto);
        DisplayAppStatistic(dataProto);
    } while (bytes > 0);
    return 0;
}
