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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <dlfcn.h>

#include "diskio_data_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
const std::string DEFAULT_TEST_PATH = "/data/local/tmp/resources";
#if defined(__LP64__)
const std::string SO_PATH = "/system/lib64/libdiskiodataplugin.z.so";
#else
const std::string SO_PATH = "/system/lib/libdiskiodataplugin.z.so";
#endif
const std::string DEFAULT_BIN_PATH("/data/local/tmp/diskiodataplugintest");
constexpr uint32_t BUF_SIZE = 4 * 1024 * 1024;
constexpr int WRITE_KB = 1000;

std::string g_path;
std::string g_testPath;

struct TestVmstat {
    int64_t pgpgin;
    int64_t pgpgout;
};

TestVmstat g_vmStat = {2072559, 27543064};

class DiskioDataPluginTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase()
    {
        if (access(g_testPath.c_str(), F_OK) == 0) {
            std::string str = "rm -rf " + g_testPath;
            system(str.c_str());
        }
    }
};

string Getexepath()
{
    char buf[PATH_MAX] = "";
    std::string path = "/proc/self/exe";
    size_t rslt = readlink(path.c_str(), buf, sizeof(buf));
    if (rslt < 0 || (rslt >= sizeof(buf))) {
        return "";
    }
    buf[rslt] = '\0';
    for (int i = rslt; i >= 0; i--) {
        if (buf[i] == '/') {
            buf[i + 1] = '\0';
            break;
        }
    }
    return buf;
}

std::string GetFullPath(std::string path)
{
    if (path.size() > 0 && path[0] != '/') {
        return Getexepath() + path;
    }
    return path;
}

bool PluginDiskioInfoStub(DiskioDataPlugin& diskioPlugin, DiskioData& diskioData, bool unusualBuff)
{
    DiskioConfig protoConfig;
    std::vector<uint8_t> configData(protoConfig.ByteSizeLong());
    int ret = protoConfig.SerializeToArray(configData.data(), configData.size());
    if (ret < 0) {
        return false;
    }

    // start
    ret = diskioPlugin.Start(configData.data(), configData.size());
    if (ret < 0) {
        return false;
    }

    // report
    std::vector<uint8_t> bufferData(BUF_SIZE);
    if (unusualBuff) { // buffer异常，调整缓冲区长度为1，测试异常情况
        bufferData.resize(1, 0);
    }

    ret = diskioPlugin.Report(bufferData.data(), bufferData.size());
    if (ret > 0) {
        diskioData.ParseFromArray(bufferData.data(), ret);
        return true;
    }
    return false;
}

/**
 * @tc.name: diskio plugin
 * @tc.desc: Test whether the path exists.
 * @tc.type: FUNC
 */
HWTEST_F(DiskioDataPluginTest, TestPath, TestSize.Level1)
{
    g_path = GetFullPath(DEFAULT_TEST_PATH);
    g_testPath = g_path;
    EXPECT_NE("", g_path);
    g_path += "/proc/vmstat";
}

/**
 * @tc.name: diskio plugin
 * @tc.desc: diskio information test for specific path.
 * @tc.type: FUNC
 */
HWTEST_F(DiskioDataPluginTest, TestPlugin, TestSize.Level1)
{
    DiskioDataPlugin diskioPlugin;
    DiskioData diskioData;
    diskioPlugin.SetPath(g_path);

    EXPECT_TRUE(PluginDiskioInfoStub(diskioPlugin, diskioData, false));
    EXPECT_EQ(diskioData.prev_rd_sectors_kb(), 0);
    EXPECT_EQ(diskioData.prev_wr_sectors_kb(), 0);
    EXPECT_EQ(diskioData.rd_sectors_kb(), g_vmStat.pgpgin);
    EXPECT_EQ(diskioData.wr_sectors_kb(), g_vmStat.pgpgout);

    EXPECT_EQ(diskioPlugin.Stop(), 0);

    // 缓冲区异常
    EXPECT_FALSE(PluginDiskioInfoStub(diskioPlugin, diskioData, true));
    EXPECT_EQ(diskioPlugin.Stop(), 0);
}

/**
 * @tc.name: diskio plugin
 * @tc.desc: diskio information test for unusual path.
 * @tc.type: FUNC
 */
HWTEST_F(DiskioDataPluginTest, TestPluginBoundary, TestSize.Level1)
{
    DiskioDataPlugin diskioPlugin;
    DiskioData diskioData;
    diskioPlugin.SetPath("123");
    EXPECT_FALSE(PluginDiskioInfoStub(diskioPlugin, diskioData, false));
    diskioPlugin.Stop();
}

/**
 * @tc.name: diskio plugin
 * @tc.desc: diskio plugin registration test.
 * @tc.type: FUNC
 */
HWTEST_F(DiskioDataPluginTest, TestPluginRegister, TestSize.Level1)
{
    void* handle = dlopen(SO_PATH.c_str(), RTLD_LAZY);
    ASSERT_NE(handle, nullptr);
    PluginModuleStruct* diskioPlugin = (PluginModuleStruct*)dlsym(handle, "g_pluginModule");
    ASSERT_NE(diskioPlugin, nullptr);
    EXPECT_STREQ(diskioPlugin->name, "diskio-plugin");
    EXPECT_EQ(diskioPlugin->resultBufferSizeHint, BUF_SIZE);

    // Serialize config
    DiskioConfig protoConfig;
    int configLength = protoConfig.ByteSizeLong();
    ASSERT_EQ(configLength, 0);
    std::vector<uint8_t> configBuffer(configLength);
    EXPECT_TRUE(protoConfig.SerializeToArray(configBuffer.data(), configLength));

    // run plugin
    std::vector<uint8_t> dataBuffer(diskioPlugin->resultBufferSizeHint);
    EXPECT_EQ(diskioPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength), RET_SUCC);
    ASSERT_GT(diskioPlugin->callbacks->onPluginReportResult(dataBuffer.data(), diskioPlugin->resultBufferSizeHint), 0);
    EXPECT_EQ(diskioPlugin->callbacks->onPluginSessionStop(), RET_SUCC);

    // 反序列化失败导致的start失败
    configLength++;
    std::vector<uint8_t> configBuffer2(configLength);
    EXPECT_TRUE(protoConfig.SerializeToArray(configBuffer2.data(), configLength));
    EXPECT_EQ(diskioPlugin->callbacks->onPluginSessionStart(configBuffer2.data(), configLength+1), RET_FAIL);
}

bool ExecuteBin(const std::string& bin, const std::vector<std::string>& args)
{
    std::vector<char*> argv;
    for (size_t i = 0; i < args.size(); i++) {
        argv.push_back(const_cast<char*>(args[i].c_str()));
    }
    argv.push_back(nullptr); // last item in argv must be NULL

    int retval = execvp(bin.c_str(), argv.data());
    CHECK_TRUE(retval != -1, false, "execv %s failed, %d!", bin.c_str(), errno);
    _exit(EXIT_FAILURE);
    abort(); // never should be here.
    return true;
}

/**
 * @tc.name: diskio plugin
 * @tc.desc: test ParseDiskioInfo for system file
 * @tc.type: FUNC
 */
HWTEST_F(DiskioDataPluginTest, TestSystemFile, TestSize.Level1)
{
    DiskioDataPlugin plugin1, plugin2;
    DiskioData diskioData1, diskioData2;

    std::string cmd = "chmod 777 " + DEFAULT_BIN_PATH;
    system(cmd.c_str());

    EXPECT_TRUE(PluginDiskioInfoStub(plugin1, diskioData1, false));
    pid_t pid = fork();
    if (pid == 0) {
        // set 1, write data to disk
        std::vector<std::string> argv = {"childpidtest", "1"};
        ASSERT_TRUE(ExecuteBin(DEFAULT_BIN_PATH, argv));
    }
    sleep(1); // 睡眠1s，确保已写入磁盘1000kb的数据
    EXPECT_TRUE(PluginDiskioInfoStub(plugin2, diskioData2, false));
    EXPECT_LE(diskioData1.wr_sectors_kb() + WRITE_KB, diskioData2.wr_sectors_kb());

    while (waitpid(-1, nullptr, WNOHANG) == 0) {
        kill(pid, SIGKILL);
    }

    plugin1.Stop();
    plugin2.Stop();
}

/**
 * @tc.name: diskio plugin
 * @tc.desc: test io_stats.cpp
 * @tc.type: FUNC
 */
HWTEST_F(DiskioDataPluginTest, TestIOStats, TestSize.Level1)
{
    IoStats ioStats;
    std::unique_ptr<StatsData> statsData = std::make_unique<StatsData>();
    EXPECT_TRUE(ioStats.GetIoData());
    EXPECT_TRUE(ioStats.PutPluginStatsData(statsData.get()));

    IoStats ioStatsIoReport(DiskioConfig::IO_REPORT);
    EXPECT_TRUE(ioStatsIoReport.GetIoData());
    EXPECT_TRUE(ioStatsIoReport.PutPluginStatsData(statsData.get()));

    IoStats ioStatsIoReportEx(DiskioConfig::IO_REPORT_EX);
    EXPECT_TRUE(ioStatsIoReportEx.GetIoData());
    EXPECT_TRUE(ioStatsIoReportEx.PutPluginStatsData(statsData.get()));
}
} // namespace
