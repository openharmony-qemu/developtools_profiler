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
#include <cinttypes>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <dlfcn.h>

#include "cpu_data_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
const std::string DEFAULT_TEST_PATH = "/data/local/tmp/resources";
#if defined(__LP64__)
const std::string SO_PATH = "/system/lib64/libcpudataplugin.z.so";
#else
const std::string SO_PATH = "/system/lib/libcpudataplugin.z.so";
#endif
const std::string DEFAULT_BIN_PATH("/data/local/tmp/cpudataplugintest");
constexpr uint32_t BUF_SIZE = 4 * 1024 * 1024;
constexpr int TEST_PID = 1;

std::string g_path;
std::string g_testPath;
std::vector<int> g_tidList = {1872, 1965, 1966, 1967, 1968, 1995, 1996};

constexpr int CORE_NUM = 6;
constexpr int THREAD_NUM = 7;
constexpr int FIRST_THREAD_NUM = 5;
constexpr int SECOND_THREAD_NUM = 6;
const int PERCENT = 100;

struct TestSystemStat {
    int32_t core;
    int64_t user;
    int64_t nice;
    int64_t system;
    int64_t idle;
    int64_t iowait;
    int64_t irq;
    int64_t softirq;
    int64_t steal;
};

struct TestCpuUsage {
    double userLoad;
    double sysLoad;
    double totalLoad;
};

struct TestStat {
    int64_t utime;
    int64_t stime;
    int64_t cutime;
    int64_t cstime;
};

struct TestTidStat {
    int32_t tid;
    std::string name;
    ThreadState state;
    TestStat stat;
};

struct TestFreq {
    int32_t curFreq;
    int32_t maxFreq;
    int32_t minFreq;
};

TestSystemStat g_systemStat[CORE_NUM + 1] = {
    {-1, 24875428, 3952448, 11859815, 1193297105, 8980661, 0, 2607250, 0},
    {0, 4165400, 662862, 1966195, 196987024, 3571925, 0, 817371, 0},
    {1, 3861506, 676578, 1702753, 199535158, 1752008, 0, 401639, 0},
    {2, 3549890, 676286, 1544630, 200640747, 1133743, 0, 205972, 0},
    {3, 3336646, 676939, 1458898, 201176432, 854578, 0, 124812, 0},
    {4, 4566158, 601107, 2305309, 197166395, 929594, 0, 1007959, 0},
    {5, 5395826, 658673, 2882028, 197791346, 738811, 0, 49496, 0},
};
TestSystemStat g_systemStat1[CORE_NUM + 1] = {
    {-1, 1234567, 2345678, 1111111, 1193291234, 3546789, 0, 2345678, 0},
    {0, 2345678, 662862, 1966195, 1111111, 3571925, 0, 817371, 0},
    {1, 3861506, 2345678, 1702753, 199535158, 1752008, 0, 401639, 0},
    {2, 3549890, 1111111, 2345678, 1234567, 1133743, 0, 205972, 0},
    {3, 3336646, 676939, 1458898, 2345678, 854578, 0, 2345678, 0},
    {4, 1111111, 601107, 2305309, 3546789, 929594, 0, 1007959, 0},
    {5, 3546789, 658673, 1234567, 197791346, 738811, 0, 49496, 0},
};
TestSystemStat g_systemStat2[CORE_NUM + 1] = {
    {-1, 3546789, 2345678, 1111111, 1193291234, 3546789, 0, 2345678, 0},
    {0, 3546789, 662862, 1966195, 2345678, 3571925, 0, 817371, 0},
    {1, 3861506, 1111111, 1702753, 199535158, 1752008, 0, 401639, 0},
    {2, 3549890, 2345678, 2345678, 3546789, 1111111, 0, 205972, 0},
    {3, 3336646, 676939, 1458898, 2345678, 854578, 0, 2345678, 0},
    {4, 2345678, 601107, 2305309, 3546789, 929594, 0, 1111111, 0},
    {5, 3546789, 658673, 3546789, 197791346, 738811, 0, 49496, 0},
};

TestStat g_pidStat = {60, 10, 20, 30};
TestStat g_pidStat1 = {50, 10, 40, 10};
TestStat g_pidStat2 = {70, 0, 10, 20};

TestTidStat g_tidStat[THREAD_NUM] = {
    {1872, "ibus-x11", THREAD_RUNNING, {17, 5, 10, 10}},
    {1965, "ibus-x1:disk$0", THREAD_SLEEPING, {8, 1, 5, 8}},
    {1966, "ibus-x1:disk$1", THREAD_UNSPECIFIED, {0, 0, 0, 0}},
    {1967, "ibus-x1:disk$2", THREAD_SLEEPING, {10, 1, 5, 8}},
    {1968, "ibus-x1:disk$3", THREAD_STOPPED, {7, 0, 0, 0}},
    {1995, "gmain", THREAD_SLEEPING, {15, 3, 0, 4}},
    {1996, "gdbus", THREAD_WAITING, {5, 0, 0, 0}},
};
TestTidStat g_tidStat1[THREAD_NUM] = {
    {1209, "skytone:service", THREAD_RUNNING, {22, 8, 0, 0}},
    {1646, "Jit thread pool", THREAD_UNSPECIFIED, {2, 1, 0, 0}},
    {1654, "Signal Catcher", THREAD_STOPPED, {1, 0, 0, 0}},
    {1655, "HeapTaskDaemon", THREAD_SLEEPING, {3, 0, 0, 0}},
    {1656, "ReferenceQueueD", THREAD_WAITING, {0, 0, 0, 0}},
};
TestTidStat g_tidStat2[THREAD_NUM] = {
    {1662, "FinalizerDaemon", THREAD_STOPPED, {0, 0, 0, 0}},
    {1663, "FinalizerWatchd", THREAD_RUNNING, {0, 0, 0, 0}},
    {1666, "Binder:1209_1", THREAD_RUNNING, {0, 0, 0, 0}},
    {1679, "Binder:1209_2", THREAD_UNSPECIFIED, {0, 0, 0, 0}},
    {1680, "Binder:1209_3", THREAD_WAITING, {54, 8, 0, 0}},
    {1681, "Binder:1209_4", THREAD_SLEEPING, {0, 0, 0, 0}},
};

TestFreq g_freq[CORE_NUM + 1] = {
    {1018000, 3844000, 509000}, {1023000, 2844000, 509000},
    {1011000, 3844000, 509000}, {1518000, 3844000, 1018000},
    {1245000, 1844000, 1018000}, {1767000, 3044000, 1018000},
};

class CpuDataPluginTest : public ::testing::Test {
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

bool CheckTid(std::vector<int>& tidListTmp)
{
    sort(g_tidList.begin(), g_tidList.end());
    for (size_t i = 0; i < g_tidList.size(); i++) {
        if (tidListTmp.at(i) != g_tidList.at(i)) {
            return false;
        }
    }
    return true;
}

bool PluginCpuinfoStub(CpuDataPlugin& cpuPlugin, CpuData& cpuData, int pid, bool reportProcess, bool unusualBuff)
{
    CpuConfig protoConfig;
    protoConfig.set_pid(pid);
    if (reportProcess) {
        protoConfig.set_report_process_info(true);
    }

    // serialize
    std::vector<uint8_t> configData(protoConfig.ByteSizeLong());
    int ret = protoConfig.SerializeToArray(configData.data(), configData.size());

    // start
    ret = cpuPlugin.Start(configData.data(), configData.size());
    if (ret < 0) {
        return false;
    }

    // report
    std::vector<uint8_t> bufferData(BUF_SIZE);
    if (unusualBuff) { // buffer异常，调整缓冲区长度为1，测试异常情况
        bufferData.resize(1, 0);
    }

    ret = cpuPlugin.Report(bufferData.data(), bufferData.size());
    if (ret > 0) {
        cpuData.ParseFromArray(bufferData.data(), ret);
        return true;
    }

    return false;
}

void GetSystemCpuTime(TestSystemStat& stat, int64_t Hz, int64_t& usageTime, int64_t& time)
{
    usageTime = (stat.user + stat.nice + stat.system + stat.irq + stat.softirq + stat.steal) * Hz;
    time = usageTime + (stat.idle + stat.iowait) * Hz;
}

void GetCpuUsage(TestCpuUsage& cpuUsage, int64_t Hz)
{
    TestSystemStat stat = g_systemStat[0];
    int64_t userTime = stat.user * Hz;
    int64_t sysTime = stat.system * Hz;
    int64_t totalTime = (stat.user + stat.nice + stat.system + stat.irq + stat.softirq + stat.steal) * Hz;
    int64_t bootTime = totalTime + (stat.idle + stat.iowait) * Hz;
    cpuUsage.userLoad = (static_cast<double>(userTime) / bootTime) * PERCENT;
    cpuUsage.sysLoad = (static_cast<double>(sysTime) / bootTime) * PERCENT;
    cpuUsage.totalLoad = (static_cast<double>(totalTime) / bootTime) * PERCENT;
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: Test whether the path exists.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPath, TestSize.Level1)
{
    g_path = GetFullPath(DEFAULT_TEST_PATH);
    g_testPath = g_path;
    EXPECT_NE("", g_path);
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: Tid list test in a specific directory.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestTidlist, TestSize.Level1)
{
    CpuDataPlugin cpuPlugin;
    std::string path = g_path + "/proc/1872/task/";
    DIR* dir = cpuPlugin.OpenDestDir(path);
    EXPECT_NE(nullptr, dir);

    std::vector<int> tidListTmp;
    while (int32_t pid = cpuPlugin.GetValidTid(dir)) {
        tidListTmp.push_back(pid);
        sort(tidListTmp.begin(), tidListTmp.end());
    }
    EXPECT_TRUE(CheckTid(tidListTmp));
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: a part of cpu information test for specific pid.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginInfo, TestSize.Level1)
{
    CpuDataPlugin cpuPlugin;
    CpuData cpuData;
    cpuPlugin.SetFreqPath(g_path);
    g_path = g_testPath + "/proc/";
    cpuPlugin.SetPath(g_path);
    EXPECT_TRUE(PluginCpuinfoStub(cpuPlugin, cpuData, 1872, true, false));

    TestCpuUsage cpuUsage;
    int64_t systemCpuTime = 0;
    int64_t systemBootTime = 0;
    int64_t Hz = cpuPlugin.GetUserHz();
    int64_t processCpuTime = (g_pidStat.utime + g_pidStat.stime) * Hz;
    GetSystemCpuTime(g_systemStat[0], Hz, systemCpuTime, systemBootTime);
    GetCpuUsage(cpuUsage, Hz);

    CpuUsageInfo cpuUsageInfo = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo.prev_process_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo.prev_system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo.prev_system_boot_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo.process_cpu_time_ms(), processCpuTime);
    EXPECT_EQ(cpuUsageInfo.system_cpu_time_ms(), systemCpuTime);
    EXPECT_EQ(cpuUsageInfo.system_boot_time_ms(), systemBootTime);
    EXPECT_EQ(cpuData.process_num(), 1);
    EXPECT_FLOAT_EQ(cpuData.user_load(), cpuUsage.userLoad);
    EXPECT_FLOAT_EQ(cpuData.sys_load(), cpuUsage.sysLoad);
    EXPECT_FLOAT_EQ(cpuData.total_load(), cpuUsage.totalLoad);

    ASSERT_EQ(cpuUsageInfo.cores_size(), 6);
    for (int i = 1; i <= CORE_NUM; i++) {
        CpuCoreUsageInfo cpuCoreUsageInfo = cpuUsageInfo.cores()[i - 1];
        GetSystemCpuTime(g_systemStat[i], Hz, systemCpuTime, systemBootTime);
        EXPECT_EQ(cpuCoreUsageInfo.cpu_core(), g_systemStat[i].core);
        EXPECT_EQ(cpuCoreUsageInfo.prev_system_cpu_time_ms(), 0);
        EXPECT_EQ(cpuCoreUsageInfo.prev_system_boot_time_ms(), 0);
        EXPECT_EQ(cpuCoreUsageInfo.system_cpu_time_ms(), systemCpuTime);
        EXPECT_EQ(cpuCoreUsageInfo.system_boot_time_ms(), systemBootTime);

        EXPECT_EQ(cpuCoreUsageInfo.frequency().min_frequency_khz(), g_freq[i - 1].minFreq);
        EXPECT_EQ(cpuCoreUsageInfo.frequency().max_frequency_khz(), g_freq[i - 1].maxFreq);
        EXPECT_EQ(cpuCoreUsageInfo.frequency().cur_frequency_khz(), g_freq[i - 1].curFreq);
        if (i == 1) { // cpu0为大核
            EXPECT_EQ(cpuCoreUsageInfo.is_little_core(), false);
        } else {
            EXPECT_EQ(cpuCoreUsageInfo.is_little_core(), true);
        }
    }
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: cpu information test for specific pid.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPlugin, TestSize.Level1)
{
    CpuDataPlugin cpuPlugin;
    CpuData cpuData;
    g_path = g_testPath;
    cpuPlugin.SetFreqPath(g_path);
    g_path = g_testPath + "/proc/";
    cpuPlugin.SetPath(g_path);
    EXPECT_TRUE(PluginCpuinfoStub(cpuPlugin, cpuData, 1872, false, false));

    int64_t Hz = cpuPlugin.GetUserHz();
    int64_t threadCpuTime;
    ASSERT_EQ(cpuData.thread_info_size(), 7);
    for (int i = 0; i < THREAD_NUM && i < cpuData.thread_info().size(); i++) {
        threadCpuTime = (g_tidStat[i].stat.utime + g_tidStat[i].stat.stime) * Hz;
        ThreadInfo threadInfo = cpuData.thread_info()[i];
        EXPECT_EQ(threadInfo.tid(), g_tidStat[i].tid);
        EXPECT_STREQ(threadInfo.thread_name().c_str(), g_tidStat[i].name.c_str());
        EXPECT_EQ(threadInfo.thread_state(), g_tidStat[i].state);
        EXPECT_EQ(threadInfo.thread_cpu_time_ms(), threadCpuTime);
        EXPECT_EQ(threadInfo.prev_thread_cpu_time_ms(), 0);
    }

    EXPECT_EQ(cpuPlugin.Stop(), 0);

    // 缓冲区异常
    EXPECT_FALSE(PluginCpuinfoStub(cpuPlugin, cpuData, 1872, false, true));
    EXPECT_EQ(cpuPlugin.Stop(), 0);
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: cpu information test for unusual path.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginBoundary, TestSize.Level1)
{
    CpuDataPlugin cpuPlugin;
    CpuData cpuData;
    g_path = g_testPath + "/proc/";
    cpuPlugin.SetPath(g_path);
    EXPECT_FALSE(PluginCpuinfoStub(cpuPlugin, cpuData, -1, false, false));
    EXPECT_FALSE(PluginCpuinfoStub(cpuPlugin, cpuData, 12345, false, false));

    CpuDataPlugin cpuPlugin2;
    cpuPlugin2.SetPath("123");
    EXPECT_FALSE(PluginCpuinfoStub(cpuPlugin2, cpuData, 1872, false, false));
    EXPECT_FALSE(PluginCpuinfoStub(cpuPlugin2, cpuData, -1, false, false));
    EXPECT_FALSE(PluginCpuinfoStub(cpuPlugin2, cpuData, 12345, false, false));
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: cpu plugin registration test.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginRegister, TestSize.Level1)
{
    void* handle = dlopen(SO_PATH.c_str(), RTLD_LAZY);
    ASSERT_NE(handle, nullptr);
    PluginModuleStruct* cpuPlugin = (PluginModuleStruct*)dlsym(handle, "g_pluginModule");
    ASSERT_NE(cpuPlugin, nullptr);
    EXPECT_STREQ(cpuPlugin->name, "cpu-plugin");
    EXPECT_EQ(cpuPlugin->resultBufferSizeHint, BUF_SIZE);

    // Serialize config
    CpuConfig protoConfig;
    protoConfig.set_pid(TEST_PID);
    int configLength = protoConfig.ByteSizeLong();
    ASSERT_GT(configLength, 0);
    std::vector<uint8_t> configBuffer(configLength);
    EXPECT_TRUE(protoConfig.SerializeToArray(configBuffer.data(), configLength));

    // run plugin
    std::vector<uint8_t> dataBuffer(cpuPlugin->resultBufferSizeHint);
    EXPECT_EQ(cpuPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength), RET_SUCC);
    ASSERT_GT(cpuPlugin->callbacks->onPluginReportResult(dataBuffer.data(), cpuPlugin->resultBufferSizeHint), 0);
    EXPECT_EQ(cpuPlugin->callbacks->onPluginSessionStop(), RET_SUCC);

    // 反序列化失败导致的start失败
    EXPECT_EQ(cpuPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength+1), RET_FAIL);
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: a part of cpu process information test.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginProcessInfo, TestSize.Level1)
{
    CpuData cpuData;
    CpuDataPlugin cpuPlugin;
    int64_t Hz = cpuPlugin.GetUserHz();
    std::string proStr1 = "2000 (ibus-x11) S 1 1865 1780 1025 1780 4194304 3233 0 457 0 50 10 40 10 20 0 7";
    std::string proStr2 = "2000 (ibus-x12) R 1 1865 1780 1025 1780 4194304 3233 0 457 0 70 0 10 20 20 0 7";
    int64_t processCpuTime1 = (g_pidStat1.utime + g_pidStat1.stime) * Hz;
    int64_t processCpuTime2 = (g_pidStat2.utime + g_pidStat2.stime) * Hz;

    // 存入proStr1
    auto* cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr1.c_str(), proStr1.length());
    CpuUsageInfo cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_process_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.process_cpu_time_ms(), processCpuTime1);

    // 存入proStr2
    cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr2.c_str(), proStr2.length());
    CpuUsageInfo cpuUsageInfo2 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo2.prev_process_cpu_time_ms(), processCpuTime1);
    EXPECT_EQ(cpuUsageInfo2.process_cpu_time_ms(), processCpuTime2);

    // 重复存入proStr2
    cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr2.c_str(), proStr2.length());
    CpuUsageInfo cpuUsageInfo3 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo3.prev_process_cpu_time_ms(), processCpuTime2);
    EXPECT_EQ(cpuUsageInfo3.process_cpu_time_ms(), processCpuTime2);
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: cpu process information test for abnormal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginProcessBoundary, TestSize.Level1)
{
    CpuData cpuData;
    CpuDataPlugin cpuPlugin;
    std::string proStr;
    int64_t Hz = cpuPlugin.GetUserHz();
    int64_t processCpuTime = (g_pidStat1.utime + g_pidStat1.stime) * Hz;

    // 空字符串
    proStr = "";
    auto* cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr.c_str(), proStr.length());
    CpuUsageInfo cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_process_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.process_cpu_time_ms(), 0);

    // 空格字符串
    proStr = " ";
    cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr.c_str(), proStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_process_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.process_cpu_time_ms(), 0);

    // 数据错误
    proStr = "2000";
    cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr.c_str(), proStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_process_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.process_cpu_time_ms(), 0);

    // 数据不够（"2000 (ibus-x11) S 1 1865 1780 1025 1780 4194304 3233 0 457 0 50 10 40 10 20 0 7"需要的字符串是50 10 40 10）
    proStr = "2000 (ibus-x11) S 1 1865 1780 1025 1780 4194304 3233 0 457 0 50 10";
    cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr.c_str(), proStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_process_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.process_cpu_time_ms(), 0);

    // 多个空格，可以取出正确数据
    proStr = "2000 (ibus-x11) S 1 1865 1780 1025 1780 4194304 3233 0 457    0 50   10 40 10 20     ";
    cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr.c_str(), proStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_process_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.process_cpu_time_ms(), processCpuTime);

    // 最后一个数据之后没有空格，可以取出正确数据
    proStr = "2000 (ibus-x11) S 1 1865 1780 1025 1780 4194304 3233 0 457 0 50 10 40 10 20";
    cpuUsageInfo = cpuData.mutable_cpu_usage_info();
    cpuPlugin.WriteProcessCpuUsage(*cpuUsageInfo, proStr.c_str(), proStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_process_cpu_time_ms(), processCpuTime);
    EXPECT_EQ(cpuUsageInfo1.process_cpu_time_ms(), processCpuTime);
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: a part of cpu system information test.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginSystemInfo, TestSize.Level1)
{
    std::string sysStr1 = "cpu  1234567 2345678 1111111 1193291234 3546789 0 2345678 0 0\n"
                          "cpu0 2345678 662862 1966195 1111111 3571925 0 817371 0 0\n"
                          "cpu1 3861506 2345678 1702753 199535158 1752008 0 401639 0 0\n"
                          "cpu2 3549890 1111111 2345678 1234567 1133743 0 205972 0 0\n"
                          "cpu3 3336646 676939 1458898 2345678 854578 0 2345678 0 0\n"
                          "cpu4 1111111 601107 2305309 3546789 929594 0 1007959 0 0\n"
                          "cpu5 3546789 658673 1234567 197791346 738811 0 49496 0 0\n";
    std::string sysStr2 = "cpu  3546789 2345678 1111111 1193291234 3546789 0 2345678 0 0\n"
                          "cpu0 3546789 662862 1966195 2345678 3571925 0 817371 0 0\n"
                          "cpu1 3861506 1111111 1702753 199535158 1752008 0 401639 0 0\n"
                          "cpu2 3549890 2345678 2345678 3546789 1111111 0 205972 0 0\n"
                          "cpu3 3336646 676939 1458898 2345678 854578 0 2345678 0 0\n"
                          "cpu4 2345678 601107 2305309 3546789 929594 0 1111111 0 0\n"
                          "cpu5 3546789 658673 3546789 197791346 738811 0 49496 0 0\n";
    CpuData cpuData;
    CpuDataPlugin cpuPlugin;
    int64_t Hz = cpuPlugin.GetUserHz();

    // 存入sysStr1
    int64_t systemCpuTime1 = 0;
    int64_t systemBootTime1 = 0;
    GetSystemCpuTime(g_systemStat1[0], Hz, systemCpuTime1, systemBootTime1);
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr1.c_str(), sysStr1.length());
    CpuUsageInfo cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.prev_system_boot_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_cpu_time_ms(), systemCpuTime1);
    EXPECT_EQ(cpuUsageInfo1.system_boot_time_ms(), systemBootTime1);
    ASSERT_EQ(cpuUsageInfo1.cores_size(), CORE_NUM);
    std::vector<int64_t> systemCpuTimeVec1(CORE_NUM);
    std::vector<int64_t> systemBootTimeVec1(CORE_NUM);
    for (int i = 1; i <= CORE_NUM; i++) {
        CpuCoreUsageInfo cpuCoreUsageInfo1 = cpuUsageInfo1.cores()[i - 1];
        GetSystemCpuTime(g_systemStat1[i], Hz, systemCpuTimeVec1[i-1], systemBootTimeVec1[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo1.cpu_core(), i-1);
        EXPECT_EQ(cpuCoreUsageInfo1.prev_system_cpu_time_ms(), 0);
        EXPECT_EQ(cpuCoreUsageInfo1.prev_system_boot_time_ms(), 0);
        EXPECT_EQ(cpuCoreUsageInfo1.system_cpu_time_ms(), systemCpuTimeVec1[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo1.system_boot_time_ms(), systemBootTimeVec1[i-1]);
    }

    // 存入sysStr2
    int64_t systemCpuTime2 = 0;
    int64_t systemBootTime2 = 0;
    GetSystemCpuTime(g_systemStat2[0], Hz, systemCpuTime2, systemBootTime2);
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr2.c_str(), sysStr2.length());
    CpuUsageInfo cpuUsageInfo2 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo2.prev_system_cpu_time_ms(), systemCpuTime1);
    EXPECT_EQ(cpuUsageInfo2.prev_system_boot_time_ms(), systemBootTime1);
    EXPECT_EQ(cpuUsageInfo2.system_cpu_time_ms(), systemCpuTime2);
    EXPECT_EQ(cpuUsageInfo2.system_boot_time_ms(), systemBootTime2);
    ASSERT_EQ(cpuUsageInfo2.cores_size(), CORE_NUM*2);
    std::vector<int64_t> systemCpuTimeVec2(CORE_NUM);
    std::vector<int64_t> systemBootTimeVec2(CORE_NUM);
    for (int i = 1; i <= CORE_NUM; i++) {
        CpuCoreUsageInfo cpuCoreUsageInfo2 = cpuUsageInfo2.cores()[CORE_NUM+i-1];
        GetSystemCpuTime(g_systemStat2[i], Hz, systemCpuTimeVec2[i-1], systemBootTimeVec2[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo2.cpu_core(), i-1);
        EXPECT_EQ(cpuCoreUsageInfo2.prev_system_cpu_time_ms(), systemCpuTimeVec1[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo2.prev_system_boot_time_ms(), systemBootTimeVec1[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo2.system_cpu_time_ms(), systemCpuTimeVec2[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo2.system_boot_time_ms(), systemBootTimeVec2[i-1]);
    }

    // 重复存入sysStr2
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr2.c_str(), sysStr2.length());
    CpuUsageInfo cpuUsageInfo3 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo3.prev_system_cpu_time_ms(), systemCpuTime2);
    EXPECT_EQ(cpuUsageInfo3.prev_system_boot_time_ms(), systemBootTime2);
    EXPECT_EQ(cpuUsageInfo3.system_cpu_time_ms(), systemCpuTime2);
    EXPECT_EQ(cpuUsageInfo3.system_boot_time_ms(), systemBootTime2);
    ASSERT_EQ(cpuUsageInfo3.cores_size(), CORE_NUM*3);
    std::vector<int64_t> systemCpuTimeVec3(CORE_NUM);
    std::vector<int64_t> systemBootTimeVec3(CORE_NUM);
    for (int i = 1; i <= CORE_NUM; i++) {
        CpuCoreUsageInfo cpuCoreUsageInfo3 = cpuUsageInfo3.cores()[CORE_NUM*2+i-1];
        GetSystemCpuTime(g_systemStat2[i], Hz, systemCpuTimeVec3[i-1], systemBootTimeVec3[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo3.cpu_core(), i-1);
        EXPECT_EQ(cpuCoreUsageInfo3.prev_system_cpu_time_ms(), systemCpuTimeVec2[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo3.prev_system_boot_time_ms(), systemBootTimeVec2[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo3.system_cpu_time_ms(), systemCpuTimeVec3[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo3.system_boot_time_ms(), systemBootTimeVec3[i-1]);
    }
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: cpu system information test for abnormal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginSystemBoundary, TestSize.Level1)
{
    CpuData cpuData;
    CpuDataPlugin cpuPlugin;
    std::string sysStr;
    int64_t Hz = cpuPlugin.GetUserHz();
    int64_t systemCpuTime = 0;
    int64_t systemBootTime = 0;
    GetSystemCpuTime(g_systemStat1[0], Hz, systemCpuTime, systemBootTime);
    // 空字符串
    sysStr = "";
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr.c_str(), sysStr.length());
    CpuUsageInfo cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.prev_system_boot_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_boot_time_ms(), 0);
    ASSERT_EQ(cpuUsageInfo1.cores_size(), 0);

    // 空格字符串
    sysStr = " ";
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr.c_str(), sysStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.prev_system_boot_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_boot_time_ms(), 0);
    ASSERT_EQ(cpuUsageInfo1.cores_size(), 0);

    // 数据错误
    sysStr = "1000";
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr.c_str(), sysStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.prev_system_boot_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_boot_time_ms(), 0);
    ASSERT_EQ(cpuUsageInfo1.cores_size(), 0);

    // 数据不够
    sysStr = "cpu  1234567 2345678 1111111 1193291234 3546789\n"
             "cpu0 2345678 662862 1966195 1111111 3571925\n"
             "cpu1 3861506 2345678 1702753 199535158 1752008\n"
             "cpu2 3549890 1111111 2345678 1234567 1133743\n"
             "cpu3 3336646 676939 1458898 2345678 854578\n"
             "cpu4 1111111 601107 2305309 3546789 929594\n"
             "cpu5 3546789 658673 1234567 197791346 738811\n";
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr.c_str(), sysStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.prev_system_boot_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_boot_time_ms(), 0);
    ASSERT_EQ(cpuUsageInfo1.cores_size(), 0);

    // 多个空格，可以取出正确数据
    sysStr = "cpu  1234567 2345678 1111111    1193291234 3546789 0 2345678 0 0\n"
             "cpu0 2345678    662862 1966195 1111111 3571925 0 817371 0 0\n"
             "cpu1 3861506 2345678 1702753   199535158     1752008 0 401639 0 0\n"
             "cpu2 3549890    1111111  2345678 1234567 1133743 0 205972     0 0\n"
             "cpu3 3336646 676939     1458898 2345678 854578 0 2345678 0 0\n"
             "cpu4 1111111   601107 2305309 3546789 929594     0 1007959 0 0\n"
             "cpu5 3546789 658673 1234567     197791346 738811 0 49496 0 0\n";
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr.c_str(), sysStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_system_cpu_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.prev_system_boot_time_ms(), 0);
    EXPECT_EQ(cpuUsageInfo1.system_cpu_time_ms(), systemCpuTime);
    EXPECT_EQ(cpuUsageInfo1.system_boot_time_ms(), systemBootTime);
    ASSERT_EQ(cpuUsageInfo1.cores_size(), CORE_NUM);
    std::vector<int64_t> systemCpuTimeVec1(CORE_NUM);
    std::vector<int64_t> systemBootTimeVec1(CORE_NUM);
    for (int i = 1; i <= CORE_NUM; i++) {
        CpuCoreUsageInfo cpuCoreUsageInfo = cpuUsageInfo1.cores()[i - 1];
        GetSystemCpuTime(g_systemStat1[i], Hz, systemCpuTimeVec1[i-1], systemBootTimeVec1[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo.cpu_core(), i-1);
        EXPECT_EQ(cpuCoreUsageInfo.prev_system_cpu_time_ms(), 0);
        EXPECT_EQ(cpuCoreUsageInfo.prev_system_boot_time_ms(), 0);
        EXPECT_EQ(cpuCoreUsageInfo.system_cpu_time_ms(), systemCpuTimeVec1[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo.system_boot_time_ms(), systemBootTimeVec1[i-1]);
    }

    // 最后一个数据之后没有空格，可以取出正确数据
    sysStr = "cpu  1234567 2345678 1111111 1193291234 3546789 0 2345678 0 0\n"
             "cpu0 2345678 662862 1966195 1111111 3571925 0 817371 0 0\n"
             "cpu1 3861506 2345678 1702753 199535158 1752008 0 401639 0 0\n"
             "cpu2 3549890 1111111 2345678 1234567 1133743 0 205972 0 0\n"
             "cpu3 3336646 676939 1458898 2345678 854578 0 2345678 0 0\n"
             "cpu4 1111111 601107 2305309 3546789 929594 0 1007959 0 0\n"
             "cpu5 3546789 658673 1234567 197791346 738811 0 49496 0 0\n";
    cpuPlugin.WriteSystemCpuUsage(cpuData, sysStr.c_str(), sysStr.length());
    cpuUsageInfo1 = cpuData.cpu_usage_info();
    EXPECT_EQ(cpuUsageInfo1.prev_system_cpu_time_ms(), systemCpuTime);
    EXPECT_EQ(cpuUsageInfo1.prev_system_boot_time_ms(), systemBootTime);
    EXPECT_EQ(cpuUsageInfo1.system_cpu_time_ms(), systemCpuTime);
    EXPECT_EQ(cpuUsageInfo1.system_boot_time_ms(), systemBootTime);
    ASSERT_EQ(cpuUsageInfo1.cores_size(), CORE_NUM*2);
    std::vector<int64_t> systemCpuTimeVec2(CORE_NUM);
    std::vector<int64_t> systemBootTimeVec2(CORE_NUM);
    for (int i = 1; i <= CORE_NUM; i++) {
        CpuCoreUsageInfo cpuCoreUsageInfo = cpuUsageInfo1.cores()[CORE_NUM+i-1];
        GetSystemCpuTime(g_systemStat1[i], Hz, systemCpuTimeVec2[i-1], systemBootTimeVec2[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo.cpu_core(), i-1);
        EXPECT_EQ(cpuCoreUsageInfo.prev_system_cpu_time_ms(), systemCpuTimeVec1[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo.prev_system_boot_time_ms(), systemBootTimeVec1[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo.system_cpu_time_ms(), systemCpuTimeVec2[i-1]);
        EXPECT_EQ(cpuCoreUsageInfo.system_boot_time_ms(), systemBootTimeVec2[i-1]);
    }
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: a part of cpu thread information test.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginThreadInfo, TestSize.Level1)
{
    std::vector<std::string> threadVec1;
    threadVec1.push_back("1209 (skytone:service) R 1746 1746 0 0 -1 1077936448 10399 0 1 0 22 8 0 0 20 0 19 0");
    threadVec1.push_back("1646 (Jit thread pool) L 1746 1746 0 0 -1 1077936192 1831 0 0 0 2 1 0 0 20 0 19 0");
    threadVec1.push_back("1654 (Signal Catcher) T 1746 1746 0 0 -1 1077936192 588 0 0 0 1 0 0 0 20 0 19 0");
    threadVec1.push_back("1655 (HeapTaskDaemon) S 1746 1746 0 0 -1 1077936192 1515 0 0 0 3 0 0 0 24 4 19 0");
    threadVec1.push_back("1656 (ReferenceQueueD) D 1746 1746 0 0 -1 1077936192 24 0 0 0 0 0 0 0 24 4 19 0");

    std::vector<std::string> threadVec2;
    threadVec2.push_back("1662 (FinalizerDaemon) T 1746 1746 0 0 -1 1077936192 18 0 0 0 0 0 0 0 24 4 19 0");
    threadVec2.push_back("1663 (FinalizerWatchd) R 1746 1746 0 0 -1 1077936192 5 0 0 0 0 0 0 0 24 4 19 0");
    threadVec2.push_back("1666 (Binder:1209_1) R 1746 1746 0 0 -1 1077936192 412 0 0 0 0 0 0 0 20 0 19 0");
    threadVec2.push_back("1679 (Binder:1209_2) O 1746 1746 0 0 -1 1077936192 14 0 0 0 0 0 0 0 20 0 19 0");
    threadVec2.push_back("1680 (Binder:1209_3) D 1746 1746 0 0 -1 1077936192 11513 0 4 0 54 8 0 0 20 0 49 0");
    threadVec2.push_back("1681 (Binder:1209_4) S 1751 1751 0 0 -1 1077936192 17 0 0 0 0 0 0 0 20 0 24 0");
    CpuData cpuData;
    CpuDataPlugin cpuPlugin;
    int64_t Hz = cpuPlugin.GetUserHz();

    // 存入threadVec0-4
    int32_t tid;
    for (int i = 0; i < FIRST_THREAD_NUM; i++) {
        auto* threadInfo = cpuData.add_thread_info();
        tid = atoi(threadVec1[i].substr(0, threadVec1[i].find(" ")).c_str());
        cpuPlugin.WriteThread(*threadInfo, threadVec1[i].c_str(), threadVec1[i].length(), tid);
    }
    ASSERT_EQ(cpuData.thread_info_size(), FIRST_THREAD_NUM);
    int64_t threadCpuTime1;
    for (int i = 0; i < FIRST_THREAD_NUM; i++) {
        threadCpuTime1 = (g_tidStat1[i].stat.utime + g_tidStat1[i].stat.stime) * Hz;
        ThreadInfo threadInfo1 = cpuData.thread_info()[i];
        EXPECT_EQ(threadInfo1.tid(), g_tidStat1[i].tid);
        EXPECT_STREQ(threadInfo1.thread_name().c_str(), g_tidStat1[i].name.c_str());
        EXPECT_EQ(threadInfo1.thread_state(), g_tidStat1[i].state);
        EXPECT_EQ(threadInfo1.prev_thread_cpu_time_ms(), 0);
        EXPECT_EQ(threadInfo1.thread_cpu_time_ms(), threadCpuTime1);
    }

    // 存入threadVec5-10
    for (int i = 0; i < SECOND_THREAD_NUM; i++) {
        auto* threadInfo = cpuData.add_thread_info();
        tid = atoi(threadVec2[i].substr(0, threadVec2[i].find(" ")).c_str());
        cpuPlugin.WriteThread(*threadInfo, threadVec2[i].c_str(), threadVec2[i].length(), tid);
    }
    ASSERT_EQ(cpuData.thread_info_size(), FIRST_THREAD_NUM+SECOND_THREAD_NUM);
    std::vector<int64_t> threadCpuTime2;
    for (int i = 0; i < SECOND_THREAD_NUM; i++) {
        threadCpuTime2.push_back((g_tidStat2[i].stat.utime + g_tidStat2[i].stat.stime) * Hz);
        ThreadInfo threadInfo2 = cpuData.thread_info()[i+FIRST_THREAD_NUM];
        EXPECT_EQ(threadInfo2.tid(), g_tidStat2[i].tid);
        EXPECT_STREQ(threadInfo2.thread_name().c_str(), g_tidStat2[i].name.c_str());
        EXPECT_EQ(threadInfo2.thread_state(), g_tidStat2[i].state);
        EXPECT_EQ(threadInfo2.prev_thread_cpu_time_ms(), 0);
        EXPECT_EQ(threadInfo2.thread_cpu_time_ms(), threadCpuTime2[i]);
    }

    // 重复存入threadVec5-10
    for (int i = 0; i < SECOND_THREAD_NUM; i++) {
        auto* threadInfo = cpuData.add_thread_info();
        tid = atoi(threadVec2[i].substr(0, threadVec2[i].find(" ")).c_str());
        cpuPlugin.WriteThread(*threadInfo, threadVec2[i].c_str(), threadVec2[i].length(), tid);
    }

    ASSERT_EQ(cpuData.thread_info_size(), FIRST_THREAD_NUM+SECOND_THREAD_NUM*2);
    int64_t threadCpuTime3;
    for (int i = 0; i < SECOND_THREAD_NUM; i++) {
        threadCpuTime3 = (g_tidStat2[i].stat.utime + g_tidStat2[i].stat.stime) * Hz;
        ThreadInfo threadInfo3 = cpuData.thread_info()[i+FIRST_THREAD_NUM+SECOND_THREAD_NUM];
        EXPECT_EQ(threadInfo3.tid(), g_tidStat2[i].tid);
        EXPECT_STREQ(threadInfo3.thread_name().c_str(), g_tidStat2[i].name.c_str());
        EXPECT_EQ(threadInfo3.thread_state(), g_tidStat2[i].state);
        EXPECT_EQ(threadInfo3.prev_thread_cpu_time_ms(), threadCpuTime2[i]);
        EXPECT_EQ(threadInfo3.thread_cpu_time_ms(), threadCpuTime3);
    }
}

/**
 * @tc.name: cpu plugin
 * @tc.desc: cpu thread information test for abnormal parameters.
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPluginThreadBoundary, TestSize.Level1)
{
    CpuData cpuData;
    CpuDataPlugin cpuPlugin;
    std::string threadStr;
    int thread_info_count = 0;
    int64_t Hz = cpuPlugin.GetUserHz();
    int64_t threadCpuTime = (g_tidStat1[0].stat.utime + g_tidStat1[0].stat.stime) * Hz;

    // 空字符串
    threadStr = "";
    auto* threadInfo = cpuData.add_thread_info();
    thread_info_count++;
    cpuPlugin.WriteThread(*threadInfo, threadStr.c_str(), threadStr.length(), 0);
    ASSERT_EQ(cpuData.thread_info_size(), thread_info_count);

    // 空格字符串
    threadStr = " ";
    threadInfo = cpuData.add_thread_info();
    thread_info_count++;
    cpuPlugin.WriteThread(*threadInfo, threadStr.c_str(), threadStr.length(), 0);
    ASSERT_EQ(cpuData.thread_info_size(), thread_info_count);

    // 数据错误
    threadStr = "1000";
    threadInfo = cpuData.add_thread_info();
    thread_info_count++;
    cpuPlugin.WriteThread(*threadInfo, threadStr.c_str(), threadStr.length(), 0);
    ASSERT_EQ(cpuData.thread_info_size(), thread_info_count);

    // 数据不够
    threadStr = "1209 (skytone:service) R 1746 1746 0 0 -1 1077936448 10399 0 1 0 22 8";
    threadInfo = cpuData.add_thread_info();
    thread_info_count++;
    cpuPlugin.WriteThread(*threadInfo, threadStr.c_str(), threadStr.length(), 0);
    ASSERT_EQ(cpuData.thread_info_size(), thread_info_count);

    // 线程名缺失左括号
    threadStr = "1209 skytone:service) R 1746 1746 0 0 -1 1077936448 10399 0 1 0 22 8 0 0 20 0 19 0";
    int32_t tid = atoi(threadStr.substr(0, threadStr.find(" ")).c_str());
    threadInfo = cpuData.add_thread_info();
    thread_info_count++;
    cpuPlugin.WriteThread(*threadInfo, threadStr.c_str(), threadStr.length(), tid);
    ASSERT_EQ(cpuData.thread_info_size(), thread_info_count);
    ThreadInfo threadInfo1 = cpuData.thread_info()[thread_info_count-1];
    EXPECT_EQ(threadInfo1.tid(), g_tidStat1[0].tid);
    EXPECT_STRNE(threadInfo1.thread_name().c_str(), g_tidStat1[0].name.c_str());
    EXPECT_EQ(threadInfo1.thread_state(), g_tidStat1[0].state);
    EXPECT_EQ(threadInfo1.prev_thread_cpu_time_ms(), 0);
    EXPECT_EQ(threadInfo1.thread_cpu_time_ms(), threadCpuTime);

    // 多个空格，可以取出正确数据
    threadStr = "1209 (skytone:service)    R 1746 1746 0 0  -1 1077936448   10399 0 1 0 22 8 0 0    20 0 19   0";
    tid = atoi(threadStr.substr(0, threadStr.find(" ")).c_str());
    threadInfo = cpuData.add_thread_info();
    thread_info_count++;
    cpuPlugin.WriteThread(*threadInfo, threadStr.c_str(), threadStr.length(), tid);
    ASSERT_EQ(cpuData.thread_info_size(), thread_info_count);
    threadInfo1 = cpuData.thread_info()[thread_info_count-1];
    EXPECT_EQ(threadInfo1.tid(), g_tidStat1[0].tid);
    EXPECT_STREQ(threadInfo1.thread_name().c_str(), g_tidStat1[0].name.c_str());
    EXPECT_EQ(threadInfo1.thread_state(), g_tidStat1[0].state);
    EXPECT_EQ(threadInfo1.prev_thread_cpu_time_ms(), threadCpuTime);
    EXPECT_EQ(threadInfo1.thread_cpu_time_ms(), threadCpuTime);

    // 最后一个数据之后没有空格，可以取出正确数据
    threadStr = "1209 (skytone:service) R 1746 1746 0 0 -1 1077936448 10399 0 1 0 22 8 0 0 20";
    tid = atoi(threadStr.substr(0, threadStr.find(" ")).c_str());
    threadInfo = cpuData.add_thread_info();
    thread_info_count++;
    cpuPlugin.WriteThread(*threadInfo, threadStr.c_str(), threadStr.length(), tid);
    ASSERT_EQ(cpuData.thread_info_size(), thread_info_count);
    threadInfo1 = cpuData.thread_info()[thread_info_count-1];
    EXPECT_EQ(threadInfo1.tid(), g_tidStat1[0].tid);
    EXPECT_STREQ(threadInfo1.thread_name().c_str(), g_tidStat1[0].name.c_str());
    EXPECT_EQ(threadInfo1.thread_state(), g_tidStat1[0].state);
    EXPECT_EQ(threadInfo1.prev_thread_cpu_time_ms(), threadCpuTime);
    EXPECT_EQ(threadInfo1.thread_cpu_time_ms(), threadCpuTime);
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
 * @tc.name: cpu plugin
 * @tc.desc: test ParseCpuInfo for pid
 * @tc.type: FUNC
 */
HWTEST_F(CpuDataPluginTest, TestPid, TestSize.Level1)
{
    CpuDataPlugin plugin1, plugin2;
    CpuData cpuData1, cpuData2;

    std::string cmd = "chmod 777 " + DEFAULT_BIN_PATH;
    system(cmd.c_str());

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // set 0, not to do
        std::vector<std::string> argv = {"childpidtest1", "0"};
        ASSERT_TRUE(ExecuteBin(DEFAULT_BIN_PATH, argv));
    }
    pid_t pid2 = fork();
    if (pid2 == 0) {
        // set 1, consume cpu
        std::vector<std::string> argv = {"childpidtest2", "1"};
        ASSERT_TRUE(ExecuteBin(DEFAULT_BIN_PATH, argv));
    }

    sleep(1); // 睡眠1s，确保pid1进入睡眠状态，pid2进入while循环
    EXPECT_TRUE(PluginCpuinfoStub(plugin1, cpuData1, static_cast<int>(pid1), false, false));
    EXPECT_TRUE(PluginCpuinfoStub(plugin2, cpuData2, static_cast<int>(pid2), false, false));
    EXPECT_LT(cpuData1.cpu_usage_info().process_cpu_time_ms(), cpuData2.cpu_usage_info().process_cpu_time_ms());

    while (waitpid(-1, nullptr, WNOHANG) == 0) {
        kill(pid1, SIGKILL);
        kill(pid2, SIGKILL);
    }

    plugin1.Stop();
    plugin2.Stop();
}
} // namespace
