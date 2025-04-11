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

#include <dlfcn.h>
#include <gtest/gtest.h>

#include "network_plugin.h"
#include "plugin_module_api.h"
#include "net_stats_client.h"

using namespace testing::ext;

namespace {
const std::string DEFAULT_TEST_PATH("/data/local/tmp/");
const std::string DEFAULT_NET_PATH("/proc/net/xt_qtaguid/stats");
constexpr uint32_t BUF_SIZE = 4 * 1024 * 1024;

std::string g_path;

struct NetDetails {
    uint64_t tx;
    uint64_t rx;
    std::string type;
};

struct TestElement {
    int32_t pid;
    int32_t uid;
    uint64_t tx;
    uint64_t rx;
    std::vector<NetDetails> details;
};

TestElement g_expectBegin = {9553, 10194, 0, 0};
TestElement g_expectEnd = {9553, 10194, 27085, 751549, {{27085, 751549, std::string("wlan0")}}};

std::string GetFullPath(std::string path);

class NetworkPluginTest : public ::testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase()
    {
        if (access(g_path.c_str(), F_OK) == 0) {
            std::string str = "rm -rf " + GetFullPath(DEFAULT_TEST_PATH) + "utresources";
            system(str.c_str());
        }
    }
    void SetUp() {}
    void TearDown() {}
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

bool SetConfig(std::vector<int> pidList, NetworkConfig& config)
{
    CHECK_TRUE(pidList.size() > 0, false, "ut: %s failed!\n", __func__);

    for (size_t i = 0; i < pidList.size(); i++) {
        config.add_pid(pidList.at(i));
    }
    return true;
}

bool PluginStub(NetworkPlugin& plugin, NetworkDatas& networkData, NetworkConfig& config)
{
    // serialize
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());

    // start
    ret = plugin.Start(configData.data(), configSize);
    CHECK_TRUE(ret == 0, false, "ut: start failed!\n");

    // report
    std::vector<uint8_t> bufferData(BUF_SIZE);
    ret = plugin.Report(bufferData.data(), bufferData.size());
    if (ret > 0) {
        networkData.ParseFromArray(bufferData.data(), ret);
        return true;
    }

    return false;
}

std::string GetFullPath(std::string path)
{
    if (path.size() > 0 && path[0] != '/') {
        return Getexepath() + path;
    }
    return path;
}

void NetworkPluginTest::SetUpTestCase()
{
    g_path = GetFullPath(DEFAULT_TEST_PATH);
    EXPECT_NE("", g_path);
    g_path += "utresources";
}

/**
 * @tc.name: network plugin
 * @tc.desc: Test whether the utresource path exists.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, Testpath, TestSize.Level1)
{
    EXPECT_NE(g_path, "");
}

/**
 * @tc.name: network plugin
 * @tc.desc: No network data for specific pid.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestNetworkDataNull, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkDatas networkData;
    plugin.setPathForTest(g_path + std::string("/begin"));

    std::vector<int> pidList = {g_expectBegin.pid};
    NetworkConfig config;
    EXPECT_TRUE(SetConfig(pidList, config));
    ASSERT_TRUE(PluginStub(plugin, networkData, config));

    for (uint32_t i = 0; i < pidList.size(); ++i) {
        EXPECT_EQ(g_expectBegin.uid, plugin.GetUid(pidList[i]));
        EXPECT_EQ(g_expectBegin.rx, networkData.mutable_networkinfo(i)->rx_bytes());
        EXPECT_EQ(g_expectBegin.tx, networkData.mutable_networkinfo(i)->tx_bytes());
        int index = networkData.mutable_networkinfo(i)->details_size();
        EXPECT_EQ(index, 0);
    }

    // stop
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: get network data for specific pid.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestGetNetworkData, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkDatas networkData;
    plugin.setPathForTest(g_path + std::string("/end"));

    std::vector<int> pidList = {g_expectEnd.pid};
    NetworkConfig config;
    EXPECT_TRUE(SetConfig(pidList, config));
    ASSERT_TRUE(PluginStub(plugin, networkData, config));

    for (uint32_t i = 0; i < pidList.size(); ++i) {
        EXPECT_EQ(g_expectEnd.uid, plugin.GetUid(pidList[i]));
        EXPECT_EQ(g_expectEnd.rx, networkData.mutable_networkinfo(i)->rx_bytes());
        EXPECT_EQ(g_expectEnd.tx, networkData.mutable_networkinfo(i)->tx_bytes());
        int index = networkData.mutable_networkinfo(i)->details_size();
        EXPECT_EQ(index, 1);
        for (int j = 0; j < index; ++j) {
            EXPECT_EQ(g_expectEnd.details[j].tx, networkData.mutable_networkinfo(i)->mutable_details(j)->tx_bytes());
            EXPECT_EQ(g_expectEnd.details[j].rx, networkData.mutable_networkinfo(i)->mutable_details(j)->rx_bytes());
            EXPECT_EQ(g_expectEnd.details[j].type, networkData.mutable_networkinfo(i)->mutable_details(j)->type());
        }
    }

    // stop
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: test the ParseFromArray fail of plugin start
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestParseFromArrayOfPluginStart, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    int32_t pid = 1;

    // set config
    config.add_pid(pid);

    // serialize
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // start
    EXPECT_NE(plugin.Start(configData.data(), size - 1), 0);
}

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
 * @tc.name: network plugin
 * @tc.desc: test register
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestRegister, TestSize.Level1)
{
    std::string path = std::string("libnetworkplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "network-plugin");

    // set config
    NetworkConfig config;
    int32_t pid = 1;
    config.add_pid(pid);
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // test framework process
    WriterStruct writer = {WriteFunc, FlushFunc};
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onRegisterWriterStruct(&writer), 0);
}

/**
 * @tc.name: network plugin
 * @tc.desc: Framework test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestFramework, TestSize.Level1)
{
    std::string path = std::string("libnetworkplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "network-plugin");

    // set config
    NetworkConfig config;
    int32_t pid = 1;
    config.add_pid(pid);
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // test framework process
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStart(configData.data(), configData.size()), 0);
    if (access(DEFAULT_NET_PATH.c_str(), F_OK) != 0) {
        EXPECT_LT(plugin->callbacks->onPluginReportResult(dataBuffer.data(), dataBuffer.size()), 0);
    } else {
        EXPECT_EQ(plugin->callbacks->onPluginReportResult(dataBuffer.data(), dataBuffer.size()), 0);
    }
    EXPECT_EQ(plugin->callbacks->onPluginSessionStop(), 0);
}

/**
 * @tc.name: network plugin
 * @tc.desc: systemdata test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestSystemData, TestSize.Level1)
{
    std::string path = std::string("libnetworkplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));

    // set config
    NetworkConfig config;
    std::string test_file = "/data/local/tmp/utresources/begin/proc/net/xt_qtaguid/stats";
    config.set_test_file(test_file);
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // systemdata test
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStart(configData.data(), configData.size()), 0);
    EXPECT_GT(plugin->callbacks->onPluginReportResult(dataBuffer.data(), dataBuffer.size()), 0);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStop(), 0);
}

/**
 * @tc.name: network plugin
 * @tc.desc: start test with single pid
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestStart1, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.set_single_pid(-1);
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ret = plugin.Start(configData.data(), configSize);
    EXPECT_EQ(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: start test with startup process name
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestStart2, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.set_startup_process_name("");
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ret = plugin.Start(configData.data(), configSize);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: start test with restart process name
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestStart3, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.set_restart_process_name("");
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ret = plugin.Start(configData.data(), configSize);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: start test with single pid
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestStart4, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.set_single_pid(9999);
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ret = plugin.Start(configData.data(), configSize);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: start test with startup process name
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestStart5, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.set_startup_process_name("process");
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ret = plugin.Start(configData.data(), configSize);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: start test with restart process name
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestStart6, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.set_restart_process_name("process");
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ret = plugin.Start(configData.data(), configSize);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: new version report test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestReportNewVersion, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.set_restart_process_name("process");
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    plugin.Start(configData.data(), configSize);
    ret = plugin.Report(nullptr, 0);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: new version report optimize test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestReportOptimizeNewVersion, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.set_restart_process_name("process");
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    plugin.Start(configData.data(), configSize);
    ret = plugin.ReportOptimize(nullptr);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: old version report test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestReportOldVersion, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.add_pid(9999);
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    plugin.Start(configData.data(), configSize);
    ret = plugin.ReportOptimize(nullptr);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: old version report optimize test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestReportOptimizeOldVersion, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkConfig config;
    config.add_pid(9999);
    int configSize = config.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = config.SerializeToArray(configData.data(), configData.size());
    plugin.Start(configData.data(), configSize);
    ret = plugin.Report(nullptr, 0);
    EXPECT_GE(ret, -1);
    plugin.Stop();
}

/**
 * @tc.name: network plugin
 * @tc.desc: GetBundleNameByPid test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestGetBundleNameByPid, TestSize.Level1)
{
    NetworkPlugin plugin;
    std::string processName = plugin.GetBundleNameByPid(1);
    EXPECT_EQ(processName, "/bin/init");
}

/**
 * @tc.name: network plugin
 * @tc.desc: GetUidByConfiguredBundleName test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestGetUidByConfiguredBundleName, TestSize.Level1)
{
    NetworkPlugin plugin;
    EXPECT_EQ(plugin.GetUidByConfiguredBundleName(""), -1);
}

/**
 * @tc.name: network plugin
 * @tc.desc: WriteNetFlowData test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestWriteNetFlowData, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkFlowData dataProto;
    ASSERT_FALSE(plugin.WriteNetFlowData(dataProto));
}

/**
 * @tc.name: network plugin
 * @tc.desc: Screen by uid test multiple iface
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestScreen1, TestSize.Level1)
{
    NetworkPlugin plugin;
    plugin.setSingleUid(1);
    NetFlowData data;
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos;
    plugin.ScreenNetworkStatByUid(infos, data);
    OHOS::NetManagerStandard::NetStatsInfo info1;
    info1.uid_ = 1;
    info1.iface_ = "wlan0";
    info1.rxBytes_ = 1;
    info1.txBytes_ = 1;
    info1.rxPackets_ = 1;
    info1.txPackets_ = 1;
    infos.push_back(info1);
    OHOS::NetManagerStandard::NetStatsInfo info2;
    info2.uid_ = 1;
    info2.iface_ = "rmnet0";
    info2.rxBytes_ = 2;
    info2.txBytes_ = 2;
    info2.rxPackets_ = 2;
    info2.txPackets_ = 2;
    infos.push_back(info2);
    plugin.ScreenNetworkStatByUid(infos, data);
    EXPECT_EQ(data.rxBytes, 3);
    EXPECT_EQ(data.txBytes, 3);
    EXPECT_EQ(data.rxPackets, 3);
    EXPECT_EQ(data.txPackets, 3);
    EXPECT_EQ(data.details.size(), 2);
}

/**
 * @tc.name: network plugin
 * @tc.desc: Screen by uid test with multiple uid
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestScreen2, TestSize.Level1)
{
    NetworkPlugin plugin;
    plugin.setSingleUid(1);
    NetFlowData data;
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos;
    plugin.ScreenNetworkStatByUid(infos, data);
    OHOS::NetManagerStandard::NetStatsInfo info1;
    info1.uid_ = 1;
    info1.iface_ = "wlan0";
    info1.rxBytes_ = 1;
    info1.txBytes_ = 1;
    info1.rxPackets_ = 1;
    info1.txPackets_ = 1;
    infos.push_back(info1);
    OHOS::NetManagerStandard::NetStatsInfo info2;
    info2.uid_ = 2;
    info2.iface_ = "rmnet0";
    info2.rxBytes_ = 2;
    info2.txBytes_ = 2;
    info2.rxPackets_ = 2;
    info2.txPackets_ = 2;
    infos.push_back(info2);
    plugin.ScreenNetworkStatByUid(infos, data);
    EXPECT_EQ(data.rxBytes, 1);
    EXPECT_EQ(data.txBytes, 1);
    EXPECT_EQ(data.rxPackets, 1);
    EXPECT_EQ(data.txPackets, 1);
    EXPECT_EQ(data.details.size(), 1);
}

/**
 * @tc.name: network plugin
 * @tc.desc: retain all test with multiple iface
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestScreen3, TestSize.Level1)
{
    NetworkPlugin plugin;
    plugin.setSingleUid(1);
    NetFlowData data;
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos;
    plugin.RetainAllNetworkStat(infos, data);
    OHOS::NetManagerStandard::NetStatsInfo info1;
    info1.uid_ = 1;
    info1.iface_ = "wlan0";
    info1.rxBytes_ = 1;
    info1.txBytes_ = 1;
    info1.rxPackets_ = 1;
    info1.txPackets_ = 1;
    infos.push_back(info1);
    OHOS::NetManagerStandard::NetStatsInfo info2;
    info2.uid_ = 1;
    info2.iface_ = "rmnet0";
    info2.rxBytes_ = 2;
    info2.txBytes_ = 2;
    info2.rxPackets_ = 2;
    info2.txPackets_ = 2;
    infos.push_back(info2);
    plugin.RetainAllNetworkStat(infos, data);
    EXPECT_EQ(data.rxBytes, 3);
    EXPECT_EQ(data.txBytes, 3);
    EXPECT_EQ(data.rxPackets, 3);
    EXPECT_EQ(data.txPackets, 3);
    EXPECT_EQ(data.details.size(), 2);
}

/**
 * @tc.name: network plugin
 * @tc.desc: retain all test with multiple uid
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestScreen4, TestSize.Level1)
{
    NetworkPlugin plugin;
    plugin.setSingleUid(1);
    NetFlowData data;
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos;
    plugin.RetainAllNetworkStat(infos, data);
    OHOS::NetManagerStandard::NetStatsInfo info1;
    info1.uid_ = 1;
    info1.iface_ = "wlan0";
    info1.rxBytes_ = 1;
    info1.txBytes_ = 1;
    info1.rxPackets_ = 1;
    info1.txPackets_ = 1;
    infos.push_back(info1);
    OHOS::NetManagerStandard::NetStatsInfo info2;
    info2.uid_ = 2;
    info2.iface_ = "rmnet0";
    info2.rxBytes_ = 2;
    info2.txBytes_ = 2;
    info2.rxPackets_ = 2;
    info2.txPackets_ = 2;
    infos.push_back(info2);
    plugin.RetainAllNetworkStat(infos, data);
    EXPECT_EQ(data.rxBytes, 3);
    EXPECT_EQ(data.txBytes, 3);
    EXPECT_EQ(data.rxPackets, 3);
    EXPECT_EQ(data.txPackets, 3);
    EXPECT_EQ(data.details.size(), 2);
}

/**
 * @tc.name: network plugin
 * @tc.desc: previous data test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestPreviousData, TestSize.Level1)
{
    NetworkPlugin plugin;
    plugin.setSingleUid(1);
    NetFlowData data;
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos1;
    OHOS::NetManagerStandard::NetStatsInfo info1;
    info1.uid_ = 1;
    info1.iface_ = "wlan0";
    info1.rxBytes_ = 1;
    info1.txBytes_ = 1;
    info1.rxPackets_ = 1;
    info1.txPackets_ = 1;
    infos1.push_back(info1);
    plugin.RetainAllNetworkStat(infos1, data);
    std::vector<OHOS::NetManagerStandard::NetStatsInfo> infos2;
    OHOS::NetManagerStandard::NetStatsInfo info2;
    info2.uid_ = 1;
    info2.iface_ = "wlan0";
    info2.rxBytes_ = 2;
    info2.txBytes_ = 2;
    info2.rxPackets_ = 2;
    info2.txPackets_ = 2;
    infos2.push_back(info2);
    plugin.RetainAllNetworkStat(infos2, data);
    EXPECT_EQ(data.rxBytes, 1);
    EXPECT_EQ(data.txBytes, 1);
    EXPECT_EQ(data.rxPackets, 1);
    EXPECT_EQ(data.txPackets, 1);
    EXPECT_EQ(data.details.size(), 1);
}

/**
 * @tc.name: network plugin
 * @tc.desc: WriteData test
 * @tc.type: FUNC
 */
HWTEST_F(NetworkPluginTest, TestWriteData, TestSize.Level1)
{
    NetworkPlugin plugin;
    NetworkFlowData dataProto;
    NetFlowData flow = {{0, 0}, 1, 1, 1, 1, std::vector<NetFlowDetail>()};
    plugin.WriteData(dataProto, flow);
    EXPECT_EQ(dataProto.rx_bytes(), 1);
    EXPECT_EQ(dataProto.rx_packets(), 1);
    EXPECT_EQ(dataProto.tx_bytes(), 1);
    EXPECT_EQ(dataProto.tx_packets(), 1);
    EXPECT_EQ(dataProto.details().size(), 0);
}
} // namespace
