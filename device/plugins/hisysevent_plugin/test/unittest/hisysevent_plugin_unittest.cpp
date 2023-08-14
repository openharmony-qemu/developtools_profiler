/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <cinttypes>
#include <cstdio>
#include <ctime>
#include <unistd.h>

#include "hisysevent_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
#if defined(__LP64__)
const std::string DEFAULT_TEST_PATH("/system/lib64/");
#else
const std::string DEFAULT_TEST_PATH("/system/lib/");
#endif
const int US_PER_S = 1000000;
const int DEFAULT_WAIT = 5;

std::atomic<uint64_t> g_testId(1);

std::vector<HisyseventInfo> g_proto;

class HisyseventPluginTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}
};

long WriteFunc(WriterStruct* writer, const void* data, size_t size)
{
    if (writer == nullptr || data == nullptr || size <= 0) {
        return -1;
    }

    HisyseventInfo info;
    if (info.ParseFromArray(data, size) <= 0) {
        return -1;
    }
    g_proto.push_back(info);
    return 0;
}

bool FlushFunc(WriterStruct* writer)
{
    if (writer == nullptr) {
        return false;
    }
    return true;
}

bool PluginStart(HisyseventPlugin& plugin, HisyseventConfig& config)
{
    // serialize
    int size = config.ByteSizeLong();
    std::vector<uint8_t> configData(size);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    CHECK_TRUE(ret > 0, false, "HisyseventPluginTest: SerializeToArray fail!!!");
    HILOG_INFO(LOG_CORE, "HisyseventPluginTest: SerializeToArray success");

    // start
    ret = plugin.Start(configData.data(), configData.size());
    CHECK_TRUE(ret == 0, false, "HisyseventPluginTest: start plugin fail!!!");
    HILOG_INFO(LOG_CORE, "HisyseventPluginTest: Start success");

    return true;
}

/**
 * @tc.name: hisysevent plugin
 * @tc.desc: Test default cmd
 * @tc.type: FUNC
 * @tc.require: issueI5UGTK
 */
HWTEST_F(HisyseventPluginTest, TestDefaultCmd, TestSize.Level1)
{
    HisyseventConfig config;
    HisyseventPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    g_proto.erase(g_proto.begin(), g_proto.end());
    // set config
    config.set_msg("Hisysevent Config.");

    // test plugin process
    plugin.SetWriter(&writer);
    EXPECT_TRUE(PluginStart(plugin, config));
    usleep(US_PER_S * DEFAULT_WAIT); // 5s
    EXPECT_EQ(plugin.Stop(), 0);

    // test proto data
    int protoSize = g_proto.size();
    EXPECT_TRUE(protoSize >= 0);
    g_testId = 1;
    for (int i = 0; i < protoSize; i++) {
        HisyseventInfo info = g_proto[i];
        for (int j = 0; j < info.info_size(); j++) {
            EXPECT_EQ(info.info(j).id(), g_testId);
            g_testId++;
        }
    }
}

/**
 * @tc.name: hisysevent plugin
 * @tc.desc: Framework test
 * @tc.type: FUNC
 * @tc.require: issueI5UGTK
 */
HWTEST_F(HisyseventPluginTest, TestFramework, TestSize.Level1)
{
    std::string path = DEFAULT_TEST_PATH + std::string("libhisyseventplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "hisysevent-plugin");

    g_testId = 1;
    g_proto.erase(g_proto.begin(), g_proto.end());

    // set config
    HisyseventConfig config;
    config.set_msg("Hisysevent Config.");
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // test framework process
    WriterStruct writer = {WriteFunc, FlushFunc};
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onRegisterWriterStruct(&writer), 0);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStart(configData.data(), configData.size()), 0);
    usleep(US_PER_S * DEFAULT_WAIT); // 5s
    EXPECT_EQ(plugin->callbacks->onPluginSessionStop(), 0);

    // test proto data
    int protoSize = g_proto.size();
    EXPECT_TRUE(protoSize >= 0);
    g_testId = 1;
    for (int i = 0; i < protoSize; i++) {
        HisyseventInfo info = g_proto[i];
        for (int j = 0; j < info.info_size(); j++) {
            EXPECT_EQ(info.info(j).id(), g_testId);
            g_testId++;
        }
    }
}

/**
 * @tc.name: hisysevent plugin
 * @tc.desc: start fail test
 * @tc.type: FUNC
 * @tc.require: issueI5UGTK
 */
HWTEST_F(HisyseventPluginTest, TestStartFail, TestSize.Level1)
{
    HisyseventConfig config;
    HisyseventPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    g_testId = 1;
    g_proto.erase(g_proto.begin(), g_proto.end());
    // set config
    config.set_msg("H");

    // test plugin process
    plugin.SetWriter(&writer);

    // serialize
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // start
    EXPECT_EQ(plugin.Start(configData.data(), 0), 0);
    usleep(US_PER_S * DEFAULT_WAIT); // 5s
    plugin.Stop();

    EXPECT_NE(plugin.Start(nullptr, configData.size()), 0);
    EXPECT_EQ(plugin.Start(configData.data(), configData.size()), 0);
    usleep(US_PER_S * DEFAULT_WAIT); // 5s
    plugin.Stop();
}

/**
 * @tc.name: hisysevent plugin
 * @tc.desc: customer popen test
 * @tc.type: FUNC
 */
HWTEST_F(HisyseventPluginTest, TestCustomPopenClose, TestSize.Level1)
{
    HisyseventConfig config;
    HisyseventPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};
    plugin.SetWriter(&writer);
    // set config
    config.set_msg("H");
    int size = config.ByteSizeLong();
    std::vector<uint8_t> configData(size);
    config.SerializeToArray(configData.data(), configData.size());
    EXPECT_EQ(plugin.Start(configData.data(), configData.size()), 0);
    EXPECT_EQ(plugin.GetFullCmd(), "/bin/hisysevent hisysevent -rd");
    std::vector<std::string> fullCmdTest;
    fullCmdTest.push_back("/bin/hisysevent");
    fullCmdTest.push_back("hisysevent");
    fullCmdTest.push_back("-rd");
    volatile pid_t childPid = -1;
    int pipeFds[2] = {-1, -1};
    FILE* fpr = COMMON::CustomPopen(fullCmdTest, nullptr, pipeFds, childPid, true);
    EXPECT_EQ(fpr, nullptr);
    EXPECT_EQ(COMMON::CustomPclose(fpr, pipeFds, childPid, true), -1);

    childPid = -1;
    pipeFds[0] = -1;
    pipeFds[1] = -1;
    FILE* fpw = COMMON::CustomPopen(fullCmdTest, "w", pipeFds, childPid);
    EXPECT_NE(fpw, nullptr);
    EXPECT_EQ(COMMON::CustomPclose(fpw, pipeFds, childPid, true), -1);
    usleep(US_PER_S * DEFAULT_WAIT); // 5s
    plugin.Stop();
}
}