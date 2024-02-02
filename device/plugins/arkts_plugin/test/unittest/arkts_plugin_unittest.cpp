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
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include <vector>
#include <memory>
#include <fcntl.h>
#include <string>
#include <regex>
#include <unistd.h>

#include "arkts_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
class ArkTSPluginTest : public ::testing::Test {
public:
    ArkTSPluginTest()
    {
        sleep(10); // Wait for the application to start successfully.
        std::unique_ptr<FILE, decltype(&pclose)> runCmd(popen("netstat -anp | grep Panda", "r"), pclose);
        if (runCmd == nullptr) {
            return;
        }

        constexpr uint32_t readBufferSize = 4096;
        std::array<char, readBufferSize> buffer;
        while (fgets(buffer.data(), buffer.size(), runCmd.get()) != nullptr) {
            std::string result = buffer.data();
            if (result.find("cn.openharmo") == std::string::npos) {
                continue;
            }
            std::regex pattern(R"(\b(\d+)/)");
            std::smatch match;
            if (std::regex_search(result, match, pattern)) {
                std::string matchedString = match[1].str();
                pid_ = std::stoi(matchedString);
                PROFILER_LOG_INFO(LOG_CORE, "ArkTSPluginTest: pid_ is %d", pid_);
            }
        }
    }
    ~ArkTSPluginTest()
    {
        sleep(5);
    }
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
    int32_t ret = arkTSPlugin.Start(configData.data(), configData.size());
    EXPECT_EQ(ret, -1);

    configData.clear();
    configData = SetArkTSConfig(protoConfig, 1, ArkTSConfig::INVALID, 0, false, false, false);
    ret = arkTSPlugin.Start(configData.data(), configData.size());
    EXPECT_EQ(ret, -1);

    configData.clear();
    configData = SetArkTSConfig(protoConfig, pid_, ArkTSConfig::INVALID, 0, false, false, false);
    ret = arkTSPlugin.Start(configData.data(), configData.size());
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(arkTSPlugin.Stop(), 0);
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
    EXPECT_EQ(arkTSPlugin.Start(configData.data(), configData.size()), 0);
    sleep(5);
    EXPECT_EQ(arkTSPlugin.Stop(), 0);
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
    EXPECT_EQ(arkTSPlugin.Start(configData.data(), configData.size()), 0);
    sleep(5);
    EXPECT_EQ(arkTSPlugin.Stop(), 0);
}
} // namespace
