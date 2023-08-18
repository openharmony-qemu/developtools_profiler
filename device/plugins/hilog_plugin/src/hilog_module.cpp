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

#include <mutex>
#include <sys/types.h>

#include "hilog_plugin.h"
#include "plugin_module_api.h"

namespace {
constexpr uint32_t MAX_BUFFER_SIZE = 4 * 1024 * 1024;
std::unique_ptr<HilogPlugin> g_plugin = nullptr;
std::mutex g_taskMutex;
} // namespace

static int HilogPluginSessionStart(const uint8_t* configData, uint32_t configSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    CHECK_NOTNULL(g_plugin, -1, "start hilog session FAILED!");
    return g_plugin->Start(configData, configSize);
}

static int HilogPluginSessionStop()
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    g_plugin->Stop();
    g_plugin.reset();
    return 0;
}

static int HilogPluginRegisterWriterStruct(WriterStruct* writer)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    g_plugin = std::make_unique<HilogPlugin>();
    g_plugin->SetWriter(writer);
    return 0;
}

static PluginModuleCallbacks g_callbacks = {
    .onPluginSessionStart = HilogPluginSessionStart,
    .onPluginReportResult = 0,
    .onPluginSessionStop = HilogPluginSessionStop,
    .onRegisterWriterStruct = (RegisterWriterStructCallback)HilogPluginRegisterWriterStruct,
};

EXPORT_API PluginModuleStruct g_pluginModule = {
    .callbacks = &g_callbacks,
    .name = "hilog-plugin",
    .version = "1.02",
    .resultBufferSizeHint = MAX_BUFFER_SIZE,
};