/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <csignal>
#include <mutex>
#include <array>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "common.h"
#include "hiebpf_plugin_config.pb.h"
#include "logging.h"
#include "plugin_module_api.h"

namespace {
constexpr uint32_t MAX_BUFFER_SIZE = 4 * 1024 * 1024;
std::mutex g_taskMutex;
constexpr int32_t RET_OK = 0;
constexpr int32_t RET_ERR = -1;
bool g_releaseResources = false;

void RunCmd(std::string& cmd)
{
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (pipe == nullptr) {
        HILOG_ERROR(LOG_CORE, "HiebpfPlugin::RunCmd: create popen FAILED!");
        return;
    }
    constexpr uint32_t readBufferSize = 4096;
    std::array<char, readBufferSize> buffer;
    std::string result;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    HILOG_INFO(LOG_CORE, "HiebpfPlugin::run command result: %s", result.c_str());
}
} // namespace

static int32_t HiebpfSessionStart(const uint8_t* configData, uint32_t configSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    CHECK_TRUE(!g_releaseResources, 0, "%s: hiebpf released resources, return", __func__);
    HILOG_DEBUG(LOG_CORE, "enter");
    if (configData == nullptr || configSize < 0) {
        HILOG_ERROR(LOG_CORE, "Parameter error");
        return RET_ERR;
    }
    HiebpfConfig config;
    CHECK_TRUE(config.ParseFromArray(configData, configSize) > 0, RET_ERR, "Parameter parsing failed");

    size_t defaultSize = sizeof(g_pluginModule.outFileName);
    CHECK_TRUE(sizeof(config.outfile_name().c_str()) <= defaultSize - 1, RET_ERR,
               "The out file path more than %zu bytes", defaultSize);
    int32_t ret = strncpy_s(g_pluginModule.outFileName, defaultSize, config.outfile_name().c_str(), defaultSize - 1);
    CHECK_TRUE(ret == EOK, RET_ERR, "strncpy_s error! outfile is %s", config.outfile_name().c_str());
    std::string cmd = config.cmd_line();
    cmd += " --start true --output_file " + config.outfile_name();
    RunCmd(cmd);
    HILOG_DEBUG(LOG_CORE, "leave");
    return RET_OK;
}

static int32_t HiebpfSessionStop()
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    CHECK_TRUE(!g_releaseResources, 0, "%s: hiebpf released resources, return", __func__);
    HILOG_DEBUG(LOG_CORE, "enter");
    std::string stop = "hiebpf --stop true";
    RunCmd(stop);
    HILOG_DEBUG(LOG_CORE, "leave");
    return RET_OK;
}

static PluginModuleCallbacks g_callbacks = {
    .onPluginSessionStart = HiebpfSessionStart,
    .onPluginReportResult = nullptr,
    .onPluginSessionStop = HiebpfSessionStop,
    .onRegisterWriterStruct = nullptr,
};

EXPORT_API PluginModuleStruct g_pluginModule = {
    .callbacks = &g_callbacks,
    .name = "hiebpf-plugin",
    .resultBufferSizeHint = MAX_BUFFER_SIZE,
    .isStandaloneFileData = true,
    .outFileName = "/data/local/tmp/hiebpf.data",
};