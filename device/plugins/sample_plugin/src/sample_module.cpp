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

#include "plugin_module_api.h"
#include "sample_plugin.h"

namespace {
constexpr uint32_t MAX_BUFFER_SIZE = 4 * 1024 * 1024;
std::unique_ptr<SamplePlugin> g_plugin = nullptr;
std::mutex g_taskMutex;
} // namespace

static int SamplePluginSessionStart(const uint8_t* configData, uint32_t configSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    g_plugin = std::make_unique<SamplePlugin>();
    return g_plugin->Start(configData, configSize);
}

static int SamplePluginReportResult(uint8_t* bufferData, uint32_t bufferSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    return g_plugin->Report(bufferData, bufferSize);
}

static int SamplePluginSessionStop()
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    g_plugin->Stop();
    g_plugin = nullptr;
    return 0;
}

static int SampleRegisterWriterStruct(const WriterStruct* writer)
{
    return 0;
}

static PluginModuleCallbacks g_callbacks = {
    .onPluginSessionStart = SamplePluginSessionStart,
    .onPluginReportResult = SamplePluginReportResult,
    .onPluginSessionStop = SamplePluginSessionStop,
    .onRegisterWriterStruct = SampleRegisterWriterStruct,
};

EXPORT_API PluginModuleStruct g_pluginModule = {
    .callbacks = &g_callbacks,
    .name = "sample-plugin",
    .version = "1.02",
    .resultBufferSizeHint = MAX_BUFFER_SIZE,
};