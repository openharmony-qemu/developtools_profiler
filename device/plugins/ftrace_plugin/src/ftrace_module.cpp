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
 *
 * Description: ftrace plugin module implements
 */
#include "ftrace_module.h"
#include <memory>
#include <mutex>

#include "flow_controller.h"
#include "logging.h"

namespace {
constexpr uint32_t MAX_BUFFER_SIZE = 4 * 1024 * 1024;

std::mutex g_mutex;
std::unique_ptr<FTRACE_NS::FlowController> g_mainController;
} // namespace

int TracePluginRegisterWriter(const WriterStruct* writer)
{
    std::unique_lock<std::mutex> lock(g_mutex);
    g_mainController = std::make_unique<FTRACE_NS::FlowController>();

    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);
    int result = g_mainController->SetWriter(const_cast<WriterStructPtr>(writer));
    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);
    return result;
}

int TracePluginStartSession(const uint8_t configData[], const uint32_t configSize)
{
    std::unique_lock<std::mutex> lock(g_mutex);
    CHECK_NOTNULL(g_mainController, -1, "no FlowController created!");

    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);
    int retval = g_mainController->LoadConfig(configData, configSize);
    CHECK_TRUE(retval == 0, retval, "LoadConfig failed!");

    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);
    int result = g_mainController->StartCapture();
    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);
    return result;
}

static int TracePluginReportBasicData()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    CHECK_NOTNULL(g_mainController, -1, "no FlowController created!");

    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);
    int result = g_mainController->ParseBasicData();
    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);
    return result;
}

int TracePluginStopSession()
{
    std::unique_lock<std::mutex> lock(g_mutex);
    CHECK_NOTNULL(g_mainController, -1, "no FlowController created!");

    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);
    int result = g_mainController->StopCapture();
    HILOG_INFO(LOG_CORE, "%s: %d", __func__, __LINE__);

    g_mainController.reset();
    return result;
}

static PluginModuleCallbacks moduleCallbacks = {
    .onPluginSessionStart = TracePluginStartSession,
    .onPluginReportResult = 0,
    .onPluginSessionStop = TracePluginStopSession,
    .onRegisterWriterStruct = TracePluginRegisterWriter,
    .onReportBasicDataCallback = TracePluginReportBasicData, // report ftrace_plugin basic data
};

EXPORT_API PluginModuleStruct g_pluginModule = {
    .callbacks = &moduleCallbacks,
    .name = "ftrace-plugin",
    .version = "1.02",
    .resultBufferSizeHint = MAX_BUFFER_SIZE,
};
