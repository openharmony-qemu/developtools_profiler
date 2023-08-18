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

#include "process_data_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
const std::string DEFAULT_TEST_PATH = "/data/local/tmp/resources";
#if defined(__arm__)
const std::string SO_PATH = "/system/lib/libprocessplugin.z.so";
#elif defined(__aarch64__)
const std::string SO_PATH = "/system/lib64/libprocessplugin.z.so";
#endif
constexpr uint32_t BUF_SIZE = 4 * 1024 * 1024;
const int PERCENT = 100;
constexpr int PROCESS_NUM = 2;
constexpr int THREAD_NUM = 3;
std::string g_path = "";
std::string g_testPath = "";
std::shared_ptr<ProcessDataPlugin> processPlugin = nullptr;

struct PssData {
    int32_t pssInfo;
};

struct DiskioData {
    int64_t rchar;
    int64_t wchar;
    int64_t syscr;
    int64_t syscw;
    int64_t rbytes;
    int64_t wbytes;
    int64_t cancelledWbytes;
};

struct CpuData {
    double cpuUsage;
    int32_t threadSum;
    int64_t cpuTimeMs;
};

struct ProcessStatus {
    int32_t pid;
    std::string name;
    int32_t ppid;
    int32_t uid;
};

struct ProcessCpuData {
    int64_t utime;
    int64_t stime;
    int64_t cutime;
    int64_t cstime;
};

struct BootData {
    int64_t user;
    int64_t nice;
    int64_t system;
    int64_t idle;
    int64_t iowait;
    int64_t irq;
    int64_t softirq;
    int64_t steal;
};

PssData g_pssData[] = { {1499}, {230} };
DiskioData g_diskioData[] = { {479, 577, 1973, 8092, 2574, 50, 91}, {7201784, 3168, 54150, 35, 22499328, 0, 0} };
ProcessStatus g_processStatus[] = { {11, "test", 2, 0}, {1872, "ibus-x11", 0, 1} };
ProcessCpuData g_processCpuData[] = { {60, 10, 20, 30}, {70, 10, 50, 30} };
BootData g_bootData = {24875428, 3952448, 11859815, 1193297105, 8980661, 0, 2607250, 0};

class ProcessDataPluginTest : public ::testing::Test {
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

bool PluginCpuinfoStub(ProcessData& processData, ProcessConfig& protoConfig, bool unusualBuff)
{
    CHECK_NOTNULL(processPlugin, false, "PluginCpuinfoStub fail");
    // serialize
    std::vector<uint8_t> configData(protoConfig.ByteSizeLong());
    int ret = protoConfig.SerializeToArray(configData.data(), configData.size());

    // start
    ret = processPlugin->Start(configData.data(), configData.size());
    if (ret < 0) {
        return false;
    }

    // report
    std::vector<uint8_t> bufferData(BUF_SIZE);
    if (unusualBuff) { // buffer异常，调整缓冲区长度为1，测试异常情况
        bufferData.resize(1, 0);
    }

    ret = processPlugin->Report(bufferData.data(), bufferData.size());
    if (ret > 0) {
        processData.ParseFromArray(bufferData.data(), ret);
        return true;
    }

    return false;
}

void GetCpuData(std::vector<CpuData>& cpuDataVec, int64_t Hz)
{
    int64_t bootTime = (g_bootData.user + g_bootData.nice + g_bootData.system + g_bootData.idle + g_bootData.iowait
                        + g_bootData.irq + g_bootData.softirq + g_bootData.steal) * Hz;
    for (int i = 0; i < PROCESS_NUM; i++) {
        int cpuTimeMs = (g_processCpuData[i].utime + g_processCpuData[i].stime + g_processCpuData[i].cutime
                            + g_processCpuData[i].cstime) * Hz;
        double cpuUsage = static_cast<double>(cpuTimeMs) / bootTime * PERCENT;
        cpuDataVec.push_back({cpuUsage, 1, cpuTimeMs});
    }
    cpuDataVec[1].threadSum = THREAD_NUM;
}

/**
 * @tc.name: process plugin
 * @tc.desc: Test whether the path exists.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPath, TestSize.Level1)
{
    g_path = GetFullPath(DEFAULT_TEST_PATH);
    g_testPath = g_path;
    EXPECT_NE("", g_path);
}

/**
 * @tc.name: process plugin
 * @tc.desc: process plugin test for report process tree.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginReportProcessTree, TestSize.Level1)
{
    g_path = g_testPath + "/proc/";
    processPlugin = std::make_shared<ProcessDataPlugin>();
    processPlugin->SetPath(g_path);

    ProcessData processData;
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    EXPECT_TRUE(PluginCpuinfoStub(processData, processConfig, false));

    for (int i = 0; i < PROCESS_NUM && i < processData.processesinfo().size(); i++) {
        ProcessInfo processesinfo = processData.processesinfo()[i];
        EXPECT_EQ(processesinfo.pid(), g_processStatus[i].pid);
        EXPECT_STREQ(processesinfo.name().c_str(), g_processStatus[i].name.c_str());
        EXPECT_EQ(processesinfo.ppid(), g_processStatus[i].ppid);
        EXPECT_EQ(processesinfo.uid(), g_processStatus[i].uid);
    }

    EXPECT_EQ(processPlugin->Stop(), 0);
}

/**
 * @tc.name: process plugin
 * @tc.desc: process plugin test for report cpu.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginReportCpu, TestSize.Level1)
{
    g_path = g_testPath + "/proc/";
    processPlugin = std::make_shared<ProcessDataPlugin>();
    processPlugin->SetPath(g_path);

    ProcessData processData;
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    processConfig.set_report_cpu(true);
    EXPECT_TRUE(PluginCpuinfoStub(processData, processConfig, false));

    std::vector<CpuData> cpuDataVec;
    int64_t Hz = processPlugin->GetUserHz();
    GetCpuData(cpuDataVec, Hz);
    for (int i = 0; i < PROCESS_NUM && i < processData.processesinfo().size(); i++) {
        CpuInfo cpuInfo = processData.processesinfo()[i].cpuinfo();
        EXPECT_FLOAT_EQ(cpuInfo.cpu_usage(), cpuDataVec[i].cpuUsage);
        EXPECT_EQ(cpuInfo.thread_sum(), cpuDataVec[i].threadSum);
        EXPECT_EQ(cpuInfo.cpu_time_ms(), cpuDataVec[i].cpuTimeMs);
    }

    EXPECT_EQ(processPlugin->Stop(), 0);
}

/**
 * @tc.name: process plugin
 * @tc.desc: process plugin test for report diskio.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginReportDiskio, TestSize.Level1)
{
    g_path = g_testPath + "/proc/";
    processPlugin = std::make_shared<ProcessDataPlugin>();
    processPlugin->SetPath(g_path);

    ProcessData processData;
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    processConfig.set_report_diskio(true);
    EXPECT_TRUE(PluginCpuinfoStub(processData, processConfig, false));

    for (int i = 0; i < PROCESS_NUM && i < processData.processesinfo().size(); i++) {
        DiskioInfo diskinfo = processData.processesinfo()[i].diskinfo();
        EXPECT_EQ(diskinfo.rchar(), g_diskioData[i].rchar);
        EXPECT_EQ(diskinfo.wchar(), g_diskioData[i].wchar);
        EXPECT_EQ(diskinfo.syscr(), g_diskioData[i].syscr);
        EXPECT_EQ(diskinfo.syscw(), g_diskioData[i].syscw);
        EXPECT_EQ(diskinfo.rbytes(), g_diskioData[i].rbytes);
        EXPECT_EQ(diskinfo.wbytes(), g_diskioData[i].wbytes);
        EXPECT_EQ(diskinfo.cancelled_wbytes(), g_diskioData[i].cancelledWbytes);
    }

    EXPECT_EQ(processPlugin->Stop(), 0);
}

/**
 * @tc.name: process plugin
 * @tc.desc: process plugin test for report pss.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginReportPss, TestSize.Level1)
{
    g_path = g_testPath + "/proc/";
    processPlugin = std::make_shared<ProcessDataPlugin>();
    processPlugin->SetPath(g_path);

    ProcessData processData;
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    processConfig.set_report_pss(true);
    EXPECT_TRUE(PluginCpuinfoStub(processData, processConfig, false));

    for (int i = 0; i < PROCESS_NUM && i < processData.processesinfo().size(); i++) {
        PssInfo pssinfo = processData.processesinfo()[i].pssinfo();
        EXPECT_EQ(pssinfo.pss_info(), g_pssData[i].pssInfo);
    }

    EXPECT_EQ(processPlugin->Stop(), 0);
}

/**
 * @tc.name: process plugin
 * @tc.desc: process plugin test.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginReportAll, TestSize.Level1)
{
    g_path = g_testPath + "/proc/";
    processPlugin = std::make_shared<ProcessDataPlugin>();
    processPlugin->SetPath(g_path);

    ProcessData processData;
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    processConfig.set_report_cpu(true);
    processConfig.set_report_pss(true);
    processConfig.set_report_diskio(true);
    EXPECT_TRUE(PluginCpuinfoStub(processData, processConfig, false));

    std::vector<CpuData> cpuDataVec;
    int64_t Hz = processPlugin->GetUserHz();
    GetCpuData(cpuDataVec, Hz);
    for (int i = 0; i < PROCESS_NUM && i < processData.processesinfo().size(); i++) {
        ProcessInfo processesinfo = processData.processesinfo()[i];
        CpuInfo cpuInfo = processData.processesinfo()[i].cpuinfo();
        DiskioInfo diskinfo = processData.processesinfo()[i].diskinfo();
        PssInfo pssinfo = processData.processesinfo()[i].pssinfo();
        EXPECT_EQ(processesinfo.pid(), g_processStatus[i].pid);
        EXPECT_STREQ(processesinfo.name().c_str(), g_processStatus[i].name.c_str());
        EXPECT_EQ(processesinfo.ppid(), g_processStatus[i].ppid);
        EXPECT_EQ(processesinfo.uid(), g_processStatus[i].uid);
        EXPECT_FLOAT_EQ(cpuInfo.cpu_usage(), cpuDataVec[i].cpuUsage);
        EXPECT_EQ(cpuInfo.thread_sum(), cpuDataVec[i].threadSum);
        EXPECT_EQ(cpuInfo.cpu_time_ms(), cpuDataVec[i].cpuTimeMs);
        EXPECT_EQ(diskinfo.rchar(), g_diskioData[i].rchar);
        EXPECT_EQ(diskinfo.wchar(), g_diskioData[i].wchar);
        EXPECT_EQ(diskinfo.syscr(), g_diskioData[i].syscr);
        EXPECT_EQ(diskinfo.syscw(), g_diskioData[i].syscw);
        EXPECT_EQ(diskinfo.rbytes(), g_diskioData[i].rbytes);
        EXPECT_EQ(diskinfo.wbytes(), g_diskioData[i].wbytes);
        EXPECT_EQ(diskinfo.cancelled_wbytes(), g_diskioData[i].cancelledWbytes);
        EXPECT_EQ(pssinfo.pss_info(), g_pssData[i].pssInfo);
    }

    EXPECT_EQ(processPlugin->Stop(), 0);
}

/**
 * @tc.name: process plugin
 * @tc.desc: process plugin test for unusual path.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginUnusualPath, TestSize.Level1)
{
    processPlugin = std::make_shared<ProcessDataPlugin>();
    processPlugin->SetPath("123");

    ProcessData processData;
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    EXPECT_FALSE(PluginCpuinfoStub(processData, processConfig, false));
}


/**
 * @tc.name: process plugin
 * @tc.desc: process plugin test for buffer exception.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginBufferException, TestSize.Level1)
{
    g_path = g_testPath + "/proc/";
    processPlugin = std::make_shared<ProcessDataPlugin>();
    processPlugin->SetPath(g_path);

    // 缓冲区异常
    ProcessData processData;
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    EXPECT_FALSE(PluginCpuinfoStub(processData, processConfig, true));
}

/**
 * @tc.name: process plugin
 * @tc.desc: process plugin registration test.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginRegister, TestSize.Level1)
{
    void* handle = dlopen(SO_PATH.c_str(), RTLD_LAZY);
    ASSERT_NE(handle, nullptr);
    PluginModuleStruct* processPlugin = (PluginModuleStruct*)dlsym(handle, "g_pluginModule");
    ASSERT_NE(processPlugin, nullptr);
    EXPECT_STREQ(processPlugin->name, "process-plugin");
    EXPECT_EQ(processPlugin->resultBufferSizeHint, BUF_SIZE);

    // Serialize config
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    int configLength = processConfig.ByteSizeLong();
    ASSERT_GT(configLength, 0);
    std::vector<uint8_t> configBuffer(configLength);
    EXPECT_TRUE(processConfig.SerializeToArray(configBuffer.data(), configLength));

    // run plugin
    std::vector<uint8_t> dataBuffer(processPlugin->resultBufferSizeHint);
    EXPECT_EQ(processPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength), RET_SUCC);
    int len = processPlugin->callbacks->onPluginReportResult(dataBuffer.data(), processPlugin->resultBufferSizeHint);
    ASSERT_GT(len, 0);
    EXPECT_EQ(processPlugin->callbacks->onPluginSessionStop(), RET_SUCC);

    // 反序列化失败导致的start失败
    EXPECT_EQ(processPlugin->callbacks->onPluginSessionStart(configBuffer.data(), configLength+1), RET_FAIL);
}

/**
 * @tc.name: process plugin
 * @tc.desc: process plugin anomaly branch test.
 * @tc.type: FUNC
 */
HWTEST_F(ProcessDataPluginTest, TestPluginReportAnomaly, TestSize.Level1)
{
    g_path = g_testPath + "/other/";
    processPlugin = std::make_shared<ProcessDataPlugin>();
    processPlugin->SetPath(g_path);

    ProcessData processData;
    ProcessConfig processConfig;
    processConfig.set_report_process_tree(true);
    EXPECT_TRUE(PluginCpuinfoStub(processData, processConfig, false));

    EXPECT_EQ(processPlugin->Stop(), 0);
}
} // namespace
