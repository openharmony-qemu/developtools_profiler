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

#include "hiperf_module.h"
#include "hiperf_plugin_config.pb.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
#if defined(__arm__)
const std::string DEFAULT_LIB_PATH("/system/lib/libhiperfplugin.z.so");
#elif defined(__aarch64__)
const std::string DEFAULT_LIB_PATH("/system/lib64/libhiperfplugin.z.so");
#endif
const std::string DEFAULT_OUT_PATH("/data/local/tmp/perf.data");
const int DEFAULT_WAIT = 2;

class HiperfPluginUnittest : public ::testing::Test {
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

/**
 * @tc.name: hiperf plugin
 * @tc.desc: Test framework
 * @tc.type: FUNC
 */
HWTEST_F(HiperfPluginUnittest, TestFramework, TestSize.Level1)
{
    void* handle = dlopen(DEFAULT_LIB_PATH.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "hiperf-plugin");

    // set config
    HiperfPluginConfig config;
    config.set_outfile_name(DEFAULT_OUT_PATH);
    config.set_record_args("-a -c 0 -d 2");
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

    dlclose(handle);
}

HWTEST_F(HiperfPluginUnittest, TestNoRecordTarget, TestSize.Level1)
{
    HiperfPluginConfig config;
    config.set_outfile_name(DEFAULT_OUT_PATH);
    std::vector<uint8_t> buffer(config.ByteSizeLong());
    ASSERT_TRUE(config.SerializeToArray(buffer.data(), buffer.size())) << "proto serialize FAILED!";

    ASSERT_NE(HiperfPluginSessionStart(buffer.data(), buffer.size()), 0) << "hiperf should return failed";
}

HWTEST_F(HiperfPluginUnittest, TestRepeatOutput, TestSize.Level1)
{
    HiperfPluginConfig config;
    config.set_log_level(HiperfPluginConfig_LogLevel_MUCH);
    config.set_outfile_name(DEFAULT_OUT_PATH); // 1: set output file
    // 2: set output file by "-o"
    config.set_record_args("-a -c 0 -d 2 -o /data/local/tmp/perf.data2");
    std::vector<uint8_t> buffer(config.ByteSizeLong());
    ASSERT_TRUE(config.SerializeToArray(buffer.data(), buffer.size())) << "proto serialize FAILED!";

    ASSERT_NE(HiperfPluginSessionStart(buffer.data(), buffer.size()), 0) << "hiperf should return failed";
}

HWTEST_F(HiperfPluginUnittest, TestRepeatStart, TestSize.Level1)
{
    HiperfPluginConfig config;
    config.set_log_level(HiperfPluginConfig_LogLevel_VERBOSE);
    config.set_outfile_name(DEFAULT_OUT_PATH);
    config.set_record_args("-a -c 0 -d 2");
    std::vector<uint8_t> buffer(config.ByteSizeLong());
    ASSERT_TRUE(config.SerializeToArray(buffer.data(), buffer.size())) << "proto serialize FAILED!";

    ASSERT_EQ(HiperfPluginSessionStart(buffer.data(), buffer.size()), 0) << "hiperf should return ok";
    // repeate to start
    ASSERT_NE(HiperfPluginSessionStart(buffer.data(), buffer.size()), 0) << "hiperf should return failed";
    ASSERT_EQ(HiperfPluginSessionStop(), 0) << "hiperf should return ok";
}

HWTEST_F(HiperfPluginUnittest, TestStopBeforeHiperfExit, TestSize.Level1)
{
    HiperfPluginConfig config;
    config.set_log_level(HiperfPluginConfig_LogLevel_DEBUG);
    config.set_outfile_name(DEFAULT_OUT_PATH);
    config.set_record_args("-a -c 0 -d 20"); // hiperf will exit after 20s
    std::vector<uint8_t> buffer(config.ByteSizeLong());
    ASSERT_TRUE(config.SerializeToArray(buffer.data(), buffer.size())) << "proto serialize FAILED!";

    ASSERT_EQ(HiperfPluginSessionStart(buffer.data(), buffer.size()), 0) << "hiperf should return ok";

    sleep(DEFAULT_WAIT);
    ASSERT_EQ(HiperfPluginSessionStop(), 0) << "hiperf should return ok";
}

HWTEST_F(HiperfPluginUnittest, TestStopAfterHiperfExit, TestSize.Level1)
{
    HiperfPluginConfig config;
    config.set_outfile_name(DEFAULT_OUT_PATH);
    config.set_record_args("-a -c 0 -d 1"); // hiperf will exit after 1s
    std::vector<uint8_t> buffer(config.ByteSizeLong());
    ASSERT_TRUE(config.SerializeToArray(buffer.data(), buffer.size())) << "proto serialize FAILED!";

    ASSERT_EQ(HiperfPluginSessionStart(buffer.data(), buffer.size()), 0) << "hiperf should return ok";
    sleep(DEFAULT_WAIT);
    ASSERT_EQ(HiperfPluginSessionStop(), 0) << "hiperf should return ok";
}

void CheckRecordFile(const std::string& fileName)
{
    ASSERT_EQ(access(fileName.c_str(), F_OK), 0) << "perf.data should be exist";

    std::unique_ptr<FILE, int (*)(FILE*)> fp(fopen(fileName.c_str(), "rb"), fclose);
    ASSERT_NE(fp, nullptr);

    constexpr long perfDataHeaderLen = 13 * 8;
    fseek(fp.get(), 0, SEEK_END);
    ASSERT_GE(ftell(fp.get()), perfDataHeaderLen) << "perf.data should have data";
    fseek(fp.get(), 0, SEEK_SET);

    char buff[perfDataHeaderLen] = {0};
    long rLen = fread(buff, 1, perfDataHeaderLen, fp.get());
    ASSERT_EQ(rLen, perfDataHeaderLen);
    ASSERT_EQ(strncmp(buff, "PERFILE2", strlen("PERFILE2")), 0) << "perf.data magic error";
}

HWTEST_F(HiperfPluginUnittest, TestCheckOutputFile, TestSize.Level1)
{
    HiperfPluginConfig config;
    config.set_outfile_name(DEFAULT_OUT_PATH);
    config.set_record_args("-a -c 0 -d 2");
    std::vector<uint8_t> buffer(config.ByteSizeLong());
    ASSERT_TRUE(config.SerializeToArray(buffer.data(), buffer.size())) << "proto serialize FAILED!";

    ASSERT_EQ(HiperfPluginSessionStart(buffer.data(), buffer.size()), 0) << "hiperf should return ok";
    sleep(DEFAULT_WAIT);
    ASSERT_EQ(HiperfPluginSessionStop(), 0) << "hiperf should return ok";

    CheckRecordFile(DEFAULT_OUT_PATH);
}
} // namespace
