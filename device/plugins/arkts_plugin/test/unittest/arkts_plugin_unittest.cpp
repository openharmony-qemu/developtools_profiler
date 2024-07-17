/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#include <cinttypes>
#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <fcntl.h>
#include <string>
#include <regex>
#include <unistd.h>

#include "arkts_plugin.h"
#include "common.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
constexpr int SLEEP_TIME = 5;
class ArkTSPluginTest : public ::testing::Test {
public:
    ArkTSPluginTest()
    {
        sleep(SLEEP_TIME); // Wait for the application to start successfully.
        const std::string processName = "cn.openharmony.rebound_project";
        COMMON::IsProcessExist(processName, pid_);
        HILOG_INFO(LOG_CORE, "ArkTSPluginTest pid: %d", pid_);
    }
    ~ArkTSPluginTest() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    int32_t pid_{0};
};

std::vector<uint8_t> SetArkTSConfig(
    ArkTSConfig &protoConfig, int32_t pid, ArkTSConfig::HeapType type,
    uint32_t interval, bool capture_numeric_value, bool track_allocations,
    bool enable_cpu_profiler, uint32_t cpu_profiler_interval = 1000)
{
    protoConfig.set_pid(pid);
    protoConfig.set_type(type);
    protoConfig.set_interval(interval);
    protoConfig.set_capture_numeric_value(capture_numeric_value);
    protoConfig.set_track_allocations(track_allocations);
    protoConfig.set_enable_cpu_profiler(enable_cpu_profiler);
    protoConfig.set_cpu_profiler_interval(cpu_profiler_interval);

    std::vector<uint8_t> configData(protoConfig.ByteSizeLong());
    protoConfig.SerializeToArray(configData.data(), configData.size());
    return configData;
}

long WriteFunc(WriterStruct* writer, const void* data, size_t size)
{
    if (writer == nullptr || data == nullptr || size == 0) {
        return -1;
    }
    return 0;
}

bool FlushFunc(WriterStruct* writer)
{
    if (writer == nullptr) {
        return false;
    }
    return true;
}

/**
 * @tc.name: arkts plugin
 * @tc.desc: arkts plugin test boundary values.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTSPluginTest, TestStartFunction, TestSize.Level1)
{
    ArkTSPlugin arkTSPlugin;
    ArkTSConfig protoConfig;

    std::vector<uint8_t> configData = SetArkTSConfig(protoConfig, -1, ArkTSConfig::INVALID, 0, false, false, false);
    arkTSPlugin.Start(configData.data(), configData.size());

    configData.clear();
    configData = SetArkTSConfig(protoConfig, 1, ArkTSConfig::INVALID, 0, false, false, false);
    arkTSPlugin.Start(configData.data(), configData.size());

    configData.clear();
    configData = SetArkTSConfig(protoConfig, pid_, ArkTSConfig::INVALID, 0, false, false, false);
    arkTSPlugin.Start(configData.data(), configData.size());
}

/**
 * @tc.name: arkts plugin
 * @tc.desc: arkts plugin test memory timeline.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTSPluginTest, TestTimeline, TestSize.Level1)
{
    ArkTSPlugin arkTSPlugin;
    ArkTSConfig protoConfig;
    WriterStruct writer = {WriteFunc, FlushFunc};

    std::vector<uint8_t> configData = SetArkTSConfig(protoConfig, pid_, ArkTSConfig::TIMELINE, 0, false, true, false);
    arkTSPlugin.SetWriter(&writer);
}

/**
 * @tc.name: arkts plugin
 * @tc.desc: arkts plugin test cpu profiler.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTSPluginTest, TestCpuProfiler, TestSize.Level1)
{
    ArkTSPlugin arkTSPlugin;
    ArkTSConfig protoConfig;
    WriterStruct writer = {WriteFunc, FlushFunc};
    std::vector<uint8_t> configData = SetArkTSConfig(protoConfig, pid_, ArkTSConfig::INVALID, 0, false, false, true);
    arkTSPlugin.SetWriter(&writer);
}
} // namespace
