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
 */

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <thread>

#include "common.h"
#include "plugin_module.h"
#include "common_types.pb.h"

using namespace testing::ext;

namespace {
const std::string SUCCESS_PLUGIN_NAME = "libmemdataplugin.z.so";
constexpr size_t READ_BUFFER_SIZE = 4 * 1024 * 1024;
#if defined(__LP64__)
std::string g_testPluginDir("/system/lib64/");
#else
std::string g_testPluginDir("/system/lib/");
#endif

class PluginModuleTest : public ::testing::Test {
protected:
    static constexpr auto TEMP_DELAY = std::chrono::milliseconds(20);
    static void SetUpTestCase()
    {
#if defined(__i386__) || defined(__x86_64__)
        char pluginDir[PATH_MAX + 1] = {0};
        if (readlink("/proc/self/exe", pluginDir, PATH_MAX) > 0) {
            char* pos = strrchr(pluginDir, '/');
            if (pos != nullptr) {
                *(pos++) = '\0';
                g_testPluginDir = pluginDir;
            }
        }
#endif

        std::this_thread::sleep_for(TEMP_DELAY);
    }

    static void TearDownTestCase() {}
};

/**
 * @tc.name: plugin
 * @tc.desc: pluginmodule normal test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginModuleTest, PluginModuleNormal, TestSize.Level1)
{
    std::string pluginPath = g_testPluginDir + "/" + SUCCESS_PLUGIN_NAME;
    PluginModuleInfo info;
    auto plugin = std::make_shared<PluginModule>(pluginPath);
    EXPECT_TRUE(plugin->Load());
    EXPECT_TRUE(plugin->BindFunctions());
    EXPECT_TRUE(plugin->GetInfo(info));
    EXPECT_TRUE(plugin->IsLoaded());

    uint32_t size = 0;
    plugin->GetBufferSizeHint(size);
    EXPECT_EQ(size, READ_BUFFER_SIZE);
    std::string name;
    EXPECT_TRUE(plugin->GetPluginName(name));
    EXPECT_STREQ(name.c_str(), "memory-plugin");

    const uint8_t configData[] = {74, 1, 10};
    ProfilerPluginConfig config;
    config.set_name(g_testPluginDir + "/" + SUCCESS_PLUGIN_NAME);
    config.set_config_data((const void*)configData, 3);
    config.set_sample_interval(1000);
    plugin->SetConfigData(config.config_data());

    std::string cfgData = plugin->GetConfigData();
    EXPECT_EQ(cfgData.c_str()[0], 74);
    EXPECT_EQ(cfgData.c_str()[1], 1);
    EXPECT_EQ(cfgData.c_str()[2], 10);

    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(size);
    EXPECT_NE(buffer, nullptr);
    plugin->SetClockId(COMMON::GetClockId("realtime"));
    (void)plugin->GetClockId();
    EXPECT_FALSE(plugin->GetPath().empty());
    EXPECT_TRUE(plugin->StartSession(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()));
    EXPECT_FALSE(plugin->GetStandaloneFileData());
    EXPECT_TRUE(plugin->IsRunning());
    EXPECT_NE(plugin->ReportResult(buffer.get(), size), 0);
    EXPECT_TRUE(plugin->StopSession());

    EXPECT_TRUE(plugin->StartSession(nullptr, 0));
    EXPECT_EQ(plugin->ReportResult(buffer.get(), size), 0);
    EXPECT_TRUE(plugin->StopSession());

    EXPECT_TRUE(plugin->StartSession(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()));
    EXPECT_NE(plugin->ReportResult(nullptr, 0), 0);
    EXPECT_NE(plugin->ReportResult(nullptr, 0), -1);
    EXPECT_TRUE(plugin->StopSession());

    EXPECT_TRUE(plugin->StartSession(nullptr, 0));
    EXPECT_EQ(plugin->ReportResult(nullptr, 0), 0);
    EXPECT_TRUE(plugin->StopSession());

    EXPECT_TRUE(plugin->Unload());
    EXPECT_FALSE(plugin->IsLoaded());
}

/**
 * @tc.name: plugin
 * @tc.desc: pluginmodule abnormal test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginModuleTest, PluginModuleAbnormal, TestSize.Level1)
{
    std::string pluginPath = "invalid.z.so";
    PluginModuleInfo info;
    auto plugin = std::make_shared<PluginModule>(pluginPath);
    EXPECT_FALSE(plugin->Load());
    EXPECT_FALSE(plugin->BindFunctions());
    EXPECT_FALSE(plugin->GetInfo(info));
    EXPECT_FALSE(plugin->IsLoaded());

    uint32_t size = 0;
    plugin->GetBufferSizeHint(size);
    EXPECT_EQ(size, 0);

    std::string name;
    EXPECT_FALSE(plugin->GetPluginName(name));
    EXPECT_STREQ(name.c_str(), "");

    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(size);
    EXPECT_FALSE(plugin->StartSession(nullptr, 0));
    EXPECT_FALSE(plugin->GetStandaloneFileData());
    EXPECT_FALSE(plugin->IsRunning());
    EXPECT_EQ(plugin->ReportResult(buffer.get(), size), -1);
    EXPECT_FALSE(plugin->StopSession());
    EXPECT_FALSE(plugin->Unload());
}

/**
 * @tc.name: plugin
 * @tc.desc: pluginmodule  test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginModuleTest, PluginModuleTest, TestSize.Level1)
{
    std::string pluginPath = g_testPluginDir + "/" + SUCCESS_PLUGIN_NAME;
    PluginModuleInfo info;
    auto plugin = std::make_shared<PluginModule>(pluginPath);

    std::string outFileName;
    EXPECT_FALSE(plugin->GetOutFileName(outFileName));
    std::string pluginVersion;
    EXPECT_FALSE(plugin->GetPluginVersion(pluginVersion));
    EXPECT_EQ(plugin->GetWriter(), nullptr);

    EXPECT_TRUE(plugin->Load());
    EXPECT_TRUE(plugin->BindFunctions());
    EXPECT_TRUE(plugin->GetInfo(info));
    EXPECT_TRUE(plugin->IsLoaded());
    EXPECT_TRUE(plugin->GetOutFileName(outFileName));
    EXPECT_EQ(outFileName, "");
    EXPECT_TRUE(plugin->GetPluginVersion(pluginVersion));
    EXPECT_EQ(pluginVersion, "1.02");
    BufferWriterPtr writer;
    EXPECT_TRUE(plugin->RegisterWriter(writer));
}
} // namespace
