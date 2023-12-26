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
#include "bytrace_module.h"

#include <array>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "bytrace_plugin_config.pb.h"
#include "logging.h"
#include "securec.h"

namespace {
constexpr uint32_t MAX_BUFFER_SIZE = 4 * 1024 * 1024;
constexpr uint32_t READ_BUFFER_SIZE = 4096;
const std::string DEFAULT_FILE = "/local/data/tmp/bytrace.txt";

std::mutex g_taskMutex;

struct BytraceInfo {
    bool isRoot;
    uint32_t buffSize;
    uint32_t time;
    std::string clockType;
    std::string outFile;
    std::vector<std::string> categoryVec;
};
std::unique_ptr<BytraceInfo> g_bytraceInfo = nullptr;

void ParseConfig(const BytracePluginConfig& config)
{
    if (config.buffe_size() != 0) {
        g_bytraceInfo->buffSize = config.buffe_size();
    }
    g_bytraceInfo->time =  config.time();
    g_bytraceInfo->isRoot = config.is_root();
    if (!config.clock().empty()) {
        g_bytraceInfo->clockType = config.clock();
    }
    if (!config.outfile_name().empty()) {
        g_bytraceInfo->outFile = config.outfile_name();
    }
    if (!config.categories().empty()) {
        for (std::string category : config.categories()) {
            g_bytraceInfo->categoryVec.push_back(category);
        }
    }
}

bool RunCommand(const std::string& cmd)
{
    HILOG_INFO(LOG_CORE, "%s:start running commond: %s", __func__, cmd.c_str());
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.data(), "r"), pclose);
    CHECK_TRUE(pipe, false, "BytraceCall::RunCommand: create popen FAILED!");

    std::array<char, READ_BUFFER_SIZE> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    HILOG_INFO(LOG_CORE, "%s:runCommand result: %s", __func__, result.data());
    return true;
}

bool BeginTrace()
{
    // two case: real-time and offline type.
    std::string beginCmd;
    if (g_bytraceInfo->isRoot) {
        beginCmd = "su root ";
    }
    beginCmd += "bytrace ";
    if (g_bytraceInfo->buffSize != 0) {
        beginCmd += " -b " + std::to_string(g_bytraceInfo->buffSize);
    }
    if (!g_bytraceInfo->clockType.empty()) {
        beginCmd += " --trace_clock " + g_bytraceInfo->clockType;
    }
    // real-time: time is set 0.
    if (g_bytraceInfo->time == 0) {
        // if time is not set 1s(must >= 1), bytrace tool will use 5s by default.
        beginCmd += " -t 1 ";
        beginCmd += " --trace_begin ";
    } else {
        beginCmd += " -t " + std::to_string(g_bytraceInfo->time);
        beginCmd += " -o ";
        beginCmd += g_bytraceInfo->outFile;
        beginCmd += " ";
    }
    for (const std::string& category : g_bytraceInfo->categoryVec) {
        beginCmd += category;
        beginCmd += " ";
    }
    return RunCommand(beginCmd);
}

bool StopTrace()
{
    std::string finishCmd;
    if (g_bytraceInfo->isRoot) {
        finishCmd = "su root ";
    }
    finishCmd += "bytrace --trace_finish --trace_dump";
    if (g_bytraceInfo->outFile.empty()) {
        g_bytraceInfo->outFile = DEFAULT_FILE;
    }
    finishCmd += " -o ";
    finishCmd += g_bytraceInfo->outFile;
    return RunCommand(finishCmd);
}
} // namespace

int BytracePluginSessionStart(const uint8_t* configData, const uint32_t configSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    BytracePluginConfig config;
    int res = config.ParseFromArray(configData, configSize);
    CHECK_TRUE(res, 0, "BytracePluginSessionStart, parse config FAILED! configSize: %u", configSize);
    g_bytraceInfo = std::make_unique<BytraceInfo>();
    ParseConfig(config);
    res = BeginTrace();
    CHECK_TRUE(res, 0, "BytracePluginSessionStart, bytrace begin FAILED!");
    return 0;
}

int BytracePluginSessionStop()
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    // real-time type need finish trace.
    if (g_bytraceInfo->time == 0) {
        int res = StopTrace();
        CHECK_TRUE(res, 0, "BytracePluginSessionStop, bytrace finish FAILED!");
    }
    g_bytraceInfo = nullptr;
    return 0;
}

int BytraceRegisterWriterStruct(const WriterStruct* writer)
{
    HILOG_INFO(LOG_CORE, "%s:writer %p", __func__, writer);
    return 0;
}

static PluginModuleCallbacks g_callbacks = {
    .onPluginSessionStart = BytracePluginSessionStart,
    .onPluginReportResult = nullptr,
    .onPluginSessionStop = BytracePluginSessionStop,
    .onRegisterWriterStruct = BytraceRegisterWriterStruct,
};

EXPORT_API PluginModuleStruct g_pluginModule = {
    .callbacks = &g_callbacks,
    .name = "bytrace_plugin",
    .version = "1.02",
    .resultBufferSizeHint = MAX_BUFFER_SIZE,
};