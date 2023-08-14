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

#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <cinttypes>
#include <cstdio>
#include <ctime>
#include <unistd.h>

#include "hidump_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
#if defined(__LP64__)
const std::string DEFAULT_TEST_PATH("/system/lib64/");
#else
const std::string DEFAULT_TEST_PATH("/system/lib/");
#endif
const std::string DEFAULT_RECORD_FILE("/data/local/tmp/");
const int DEFAULT_WAIT = 10;
constexpr int BUF_MAX_LEN = 8;

class HidumpPluginUnittest : public ::testing::Test {
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

    return 0;
}

bool FlushFunc(WriterStruct* writer)
{
    if (writer == nullptr) {
        return false;
    }
    return true;
}

bool PluginStart(HidumpPlugin& plugin, HidumpConfig& config)
{
    // serialize
    int size = config.ByteSizeLong();
    std::vector<uint8_t> configData(size);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    CHECK_TRUE(ret > 0, false, "HidumpPluginUnittest: SerializeToArray fail!!!");
    HILOG_INFO(LOG_CORE, "HidumpPluginUnittest: SerializeToArray success");

    // start
    ret = plugin.Start(configData.data(), configData.size());
    CHECK_TRUE(ret == 0, false, "HidumpPluginUnittest: start plugin fail!!!");
    HILOG_INFO(LOG_CORE, "HidumpPluginUnittest: Start success");

    return true;
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test framework
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestFramework, TestSize.Level1)
{
    std::string path = DEFAULT_TEST_PATH + std::string("libhidumpplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "hidump-plugin");

    // set config
    HidumpConfig config;
    config.set_report_fps(true);
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // test framework process
    WriterStruct writer = {WriteFunc, FlushFunc};
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onRegisterWriterStruct(&writer), 0);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStart(configData.data(), configData.size()), 0);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStop(), 0);
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test if invalid cmd causes an exception
 *           expect："inaccessible or not found"
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestInvalidCmd1, TestSize.Level1)
{
    HidumpConfig config;
    HidumpPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    config.set_report_fps(true);
    plugin.SetConfig(config);

    const char *cmd = "";
    plugin.SetTestCmd(cmd);
    plugin.SetWriter(&writer);
    EXPECT_STREQ(plugin.GetTestCmd(), cmd);
    EXPECT_TRUE(PluginStart(plugin, config));
    EXPECT_EQ(plugin.Stop(), 0);
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test if invalid cmd causes an exception
 *           expect："HidumpPlugin: fps command not output error!"
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestInvalidCmd2, TestSize.Level1)
{
    HidumpConfig config;
    HidumpPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    config.set_report_fps(true);
    plugin.SetConfig(config);

    const char *cmd = "GP_daemon_fps 0";
    plugin.SetTestCmd(cmd);
    plugin.SetWriter(&writer);
    EXPECT_STREQ(plugin.GetTestCmd(), cmd);
    EXPECT_TRUE(PluginStart(plugin, config));
    EXPECT_EQ(plugin.Stop(), 0);
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test Default Cmd
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestDefaultCmd, TestSize.Level1)
{
    HidumpConfig config;
    HidumpPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    config.set_report_fps(true);
    plugin.SetConfig(config);

    plugin.SetWriter(&writer);
    EXPECT_TRUE(PluginStart(plugin, config));
    EXPECT_EQ(plugin.Stop(), 0);
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test Default Cmd and verify result
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestCmdAndVerifyResult, TestSize.Level1)
{
    HidumpConfig config;
    HidumpPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    config.set_report_fps(true);
    plugin.SetConfig(config);

    plugin.SetWriter(&writer);
    EXPECT_TRUE(PluginStart(plugin, config));
    sleep(DEFAULT_WAIT);
    EXPECT_EQ(plugin.Stop(), 0);
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test CustomPopen whether non-blocking
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestCustomPopenR, TestSize.Level1)
{
    FILE *fp = nullptr;
    HidumpPlugin plugin;

    fp = plugin.CustomPopen("", "r");
    EXPECT_NE(fp, nullptr);

    EXPECT_NE(plugin.CustomPclose(fp), -1);
}

bool RecordFileExist(std::string& file)
{
    if (access(DEFAULT_RECORD_FILE.c_str(), F_OK) != 0) {
        return false;
    }

    std::string cmd = "cat " + file;
    std::unique_ptr<FILE, int (*)(FILE*)> fp(popen(cmd.c_str(), "r"), pclose);
    if (!fp) {
        HILOG_ERROR(LOG_CORE, "%s:: popen error", __func__);
        return false;
    }

    char buff[BUF_MAX_LEN] = {0};
    char* pRet = fgets(buff, BUF_MAX_LEN - 1, fp.get());
    if (pRet == nullptr) {
        HILOG_ERROR(LOG_CORE, "%s: fgets Failed, errno(%d)", __func__, errno);
        return false;
    }
    buff[BUF_MAX_LEN - 1] = '\0';
    if (!strncmp(pRet, "123", strlen("123"))) {
        return true;
    }
    return false;
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test CustomPopen whether non-blocking
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestCustomPopenW, TestSize.Level1)
{
    FILE *fp = nullptr;
    HidumpPlugin plugin;
    std::string fileName = "/data/local/tmp/hidumpTest.txt";
    std::string cmd = "echo \"123\" > " + fileName;

    fp = plugin.CustomPopen(cmd.c_str(), "w");
    EXPECT_NE(fp, nullptr);
    fflush(fp);
    sleep(1);
    EXPECT_NE(plugin.CustomPclose(fp), -1);
    ASSERT_TRUE(RecordFileExist(fileName));

    std::string rmCmd = "rm -rf " + fileName;
    system(rmCmd.c_str());
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test CustomPopen whether nullptr cmd
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestCustomPopenCmdNull, TestSize.Level1)
{
    FILE *fp = nullptr;
    HidumpPlugin plugin;

    fp = plugin.CustomPopen(nullptr, "w");
    ASSERT_EQ(fp, nullptr);
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test CustomPopen whether nullptr type
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestCustomPopenTypeNull, TestSize.Level1)
{
    FILE *fp = nullptr;
    HidumpPlugin plugin;
    std::string fileName = "/data/local/tmp/hidumpTest.txt";
    std::string cmd = "echo \"123\" > " + fileName;

    fp = plugin.CustomPopen(cmd.c_str(), nullptr);
    ASSERT_EQ(fp, nullptr);
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: Test CustomPopen whether nullptr
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestCustomPopenNull, TestSize.Level1)
{
    FILE *fp = nullptr;
    HidumpPlugin plugin;

    fp = plugin.CustomPopen(nullptr, nullptr);
    ASSERT_EQ(fp, nullptr);
}

/**
 * @tc.name: hidump plugin
 * @tc.desc: start fail test
 * @tc.type: FUNC
 */
HWTEST_F(HidumpPluginUnittest, TestStartFail, TestSize.Level1)
{
    HidumpConfig config;
    HidumpPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};

    // set config
    config.set_report_fps(true);

    // test plugin process
    plugin.SetWriter(&writer);
    plugin.SetConfig(config);

    // serialize
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // start
    EXPECT_NE(plugin.Start(configData.data(), size - 1), 0);
}
} // namespace
