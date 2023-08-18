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
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <sys/types.h>

#include "memory_data_plugin.h"
#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
const std::string DEFAULT_TEST_PATH("/data/local/tmp/");
#if defined(__LP64__)
const std::string DEFAULT_SO_PATH("/system/lib64/");
#else
const std::string DEFAULT_SO_PATH("/system/lib/");
#endif
const std::string DEFAULT_BIN_PATH("/data/local/tmp/memorytest");
constexpr uint32_t BUF_SIZE = 4 * 1024 * 1024;
const int US_PER_S = 1000000;

std::string g_path;

struct TestElement {
    int32_t pid;
    std::string name;
    // data from /proc/$pid/stat
    uint64_t vm_size_kb;
    uint64_t vm_rss_kb;
    uint64_t rss_anon_kb;
    uint64_t rss_file_kb;
    uint64_t rss_shmem_kb;
    uint64_t vm_swap_kb;
    uint64_t vm_locked_kb;
    uint64_t vm_hwm_kb;
    int64_t oom_score_adj;

    uint64_t java_heap;
    uint64_t native_heap;
    uint64_t code;
    uint64_t stack;
    uint64_t graphics;
    uint64_t private_other;
};

TestElement g_pidtarget[] = {
    {1, "systemd", 226208, 9388, 2984, 6404, 0, 0, 0, 9616, -1, 3036, 4256, 288, 748, 0, 1388},
    {2, "kthreadd", 0, 0, 0, 0, 0, 0, 0, 0, -100, 3036, 4260, 336, 760, 0, 4204},
    {11, "rcu_sched", 0, 0, 0, 0, 0, 0, 0, 0, 0, 3036, 4272, 400, 772, 0, 7160},
};

unsigned long g_meminfo[] = {
    16168696, 1168452, 12363564, 2726188, 7370484, 29260,    8450388,  4807668,
    2535372,  658832,  4148836,  132,     0,       63999996, 62211580, 0
};

unsigned long g_vmeminfo[] = {
    112823, 0,      587,    1848,   101,   9074,  8426,   18314,
    0,     2416,  2348,  9073,   1876,  26863, 1,      0
};

std::string GetFullPath(std::string path);

class MemoryDataPluginTest : public ::testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase() {}
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

void SetPluginProcessConfig(std::vector<int> processList, MemoryConfig& protoConfig)
{
    if (processList.size() != 0) {
        // 具体进程
        protoConfig.set_report_process_mem_info(true);
        protoConfig.set_report_app_mem_info(true);
        for (size_t i = 0; i < processList.size(); i++) {
            protoConfig.add_pid(processList.at(i));
        }
    } else {
        // 进程树
        protoConfig.set_report_process_tree(true);
    }
}

void SetPluginSysMemConfig(MemoryConfig &protoConfig)
{
    protoConfig.set_report_sysmem_mem_info(true);

    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_MEM_TOTAL);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_MEM_FREE);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_MEM_AVAILABLE);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_BUFFERS);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_CACHED);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_SWAP_CACHED);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_ACTIVE);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_INACTIVE);

    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_ACTIVE_ANON);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_INACTIVE_ANON);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_INACTIVE_FILE);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_UNEVICTABLE);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_MLOCKED);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_SWAP_TOTAL);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_SWAP_FREE);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_DIRTY);

    protoConfig.set_report_sysmem_vmem_info(true);

    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_FREE_PAGES);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_INACTIVE_ANON);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_ACTIVE_ANON);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_INACTIVE_FILE);

    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_ACTIVE_FILE);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_UNEVICTABLE);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_MLOCK);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_ANON_PAGES);

    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_MAPPED);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_FILE_PAGES);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_DIRTY);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_SLAB_RECLAIMABLE);

    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_SLAB_UNRECLAIMABLE);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_PAGE_TABLE_PAGES);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_KERNEL_STACK);
    protoConfig.add_sys_vmeminfo_counters(SysVMeminfoType::VMEMINFO_NR_UNSTABLE);
}

void SetPluginMemoryServiceConfig(MemoryConfig& protoConfig)
{
    protoConfig.set_report_process_mem_info(true);
    protoConfig.set_report_app_mem_info(true);
    protoConfig.add_pid(1);
    protoConfig.set_report_app_mem_by_memory_service(true);
}

bool PluginStub(MemoryDataPlugin& memoryPlugin, MemoryConfig& protoConfig, MemoryData& memoryData)
{
    // serialize
    int configSize = protoConfig.ByteSizeLong();
    std::vector<uint8_t> configData(configSize);
    int ret = protoConfig.SerializeToArray(configData.data(), configData.size());
    CHECK_TRUE(ret > 0, false, "PluginStub::SerializeToArray fail!!!");

    // start
    ret = memoryPlugin.Start(configData.data(), configData.size());
    CHECK_TRUE(ret == 0, false, "PluginStub::start plugin fail!!!");

    // report
    std::vector<uint8_t> bufferData(BUF_SIZE);
    ret = memoryPlugin.Report(bufferData.data(), bufferData.size());
    if (ret >= 0) {
        memoryData.ParseFromArray(bufferData.data(), ret);
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

void MemoryDataPluginTest::SetUpTestCase()
{
    g_path = GetFullPath(DEFAULT_TEST_PATH);
    EXPECT_NE("", g_path);
    g_path += "utresources/proc";
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test whether the path exists.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestUtpath, TestSize.Level1)
{
    EXPECT_NE(g_path, "");
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Pid list test in a specific directory.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, Testpidlist, TestSize.Level1)
{
    MemoryDataPlugin* memoryPlugin = new MemoryDataPlugin();
    const std::vector<int> expectPidList = {1, 2, 11};

    DIR* dir = memoryPlugin->OpenDestDir(g_path.c_str());
    EXPECT_NE(nullptr, dir);

    std::vector<int> cmpPidList;
    while (int32_t pid = memoryPlugin->GetValidPid(dir)) {
        cmpPidList.push_back(pid);
    }
    sort(cmpPidList.begin(), cmpPidList.end());
    closedir(dir);
    EXPECT_EQ(cmpPidList, expectPidList);
    delete memoryPlugin;
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Mem information test for specific pid.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, Testpluginformeminfo, TestSize.Level1)
{
    MemoryDataPlugin memoryPlugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;

    memoryPlugin.SetPath(const_cast<char*>(g_path.c_str()));
    SetPluginSysMemConfig(protoConfig);
    EXPECT_TRUE(PluginStub(memoryPlugin, protoConfig, memoryData));

    EXPECT_EQ(16, memoryData.meminfo().size());
    int index = memoryData.meminfo_size();
    for (int i = 0; i < index; ++i) {
        EXPECT_EQ(g_meminfo[i], memoryData.meminfo(i).value());
    }

    EXPECT_EQ(16, memoryData.vmeminfo().size());
    index = memoryData.vmeminfo_size();
    for (int i = 0; i < index; ++i) {
        EXPECT_EQ(g_vmeminfo[i], memoryData.vmeminfo(i).value());
    }
    memoryPlugin.Stop();
}

/**
 * @tc.name: memory plugin
 * @tc.desc: pid list information test for process tree.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, Testpluginforlist, TestSize.Level1)
{
    MemoryDataPlugin memoryPlugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;

    std::vector<int> cmpPidList;
    EXPECT_EQ((size_t)0, cmpPidList.size());

    memoryPlugin.SetPath(const_cast<char*>(g_path.c_str()));

    SetPluginProcessConfig(cmpPidList, protoConfig);
    EXPECT_TRUE(PluginStub(memoryPlugin, protoConfig, memoryData));

    int index = memoryData.processesinfo_size();
    EXPECT_EQ(3, index);
    for (int i = 0; i < index; ++i) {
        ProcessMemoryInfo it = memoryData.processesinfo(i);
        EXPECT_EQ(g_pidtarget[i].pid, it.pid());
        EXPECT_EQ(g_pidtarget[i].name, it.name());
        EXPECT_EQ(g_pidtarget[i].vm_size_kb, it.vm_size_kb());
        EXPECT_EQ(g_pidtarget[i].vm_rss_kb, it.vm_rss_kb());
        EXPECT_EQ(g_pidtarget[i].rss_anon_kb, it.rss_anon_kb());
        EXPECT_EQ(g_pidtarget[i].rss_file_kb, it.rss_file_kb());
        EXPECT_EQ(g_pidtarget[i].rss_shmem_kb, it.rss_shmem_kb());
        EXPECT_EQ(g_pidtarget[i].vm_locked_kb, it.vm_locked_kb());
        EXPECT_EQ(g_pidtarget[i].vm_hwm_kb, it.vm_hwm_kb());

        EXPECT_EQ(g_pidtarget[i].oom_score_adj, it.oom_score_adj());

        EXPECT_FALSE(it.has_memsummary());
    }

    memoryPlugin.Stop();
}

/**
 * @tc.name: memory plugin
 * @tc.desc: pid list information test for specific pid.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, Testpluginforsinglepid, TestSize.Level1)
{
    MemoryDataPlugin memoryPlugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;

    std::vector<int> pid = {5};
    TestElement singlepid = {};

    memoryPlugin.SetPath(const_cast<char*>(g_path.c_str()));

    SetPluginProcessConfig(pid, protoConfig);
    EXPECT_TRUE(PluginStub(memoryPlugin, protoConfig, memoryData));

    int index = memoryData.processesinfo_size();
    EXPECT_EQ(1, index);

    ProcessMemoryInfo it = memoryData.processesinfo(0);
    EXPECT_EQ(singlepid.pid, it.pid());
    EXPECT_EQ(singlepid.name, it.name());
    EXPECT_EQ(singlepid.vm_size_kb, it.vm_size_kb());
    EXPECT_EQ(singlepid.vm_rss_kb, it.vm_rss_kb());
    EXPECT_EQ(singlepid.rss_anon_kb, it.rss_anon_kb());
    EXPECT_EQ(singlepid.rss_file_kb, it.rss_file_kb());
    EXPECT_EQ(singlepid.rss_shmem_kb, it.rss_shmem_kb());
    EXPECT_EQ(singlepid.vm_locked_kb, it.vm_locked_kb());
    EXPECT_EQ(singlepid.vm_hwm_kb, it.vm_hwm_kb());

    EXPECT_EQ(singlepid.oom_score_adj, it.oom_score_adj());

    EXPECT_TRUE(it.has_memsummary());
    AppSummary app = it.memsummary();
    EXPECT_EQ(singlepid.java_heap, app.java_heap());
    EXPECT_EQ(singlepid.native_heap, app.native_heap());
    EXPECT_EQ(singlepid.code, app.code());
    EXPECT_EQ(singlepid.stack, app.stack());
    EXPECT_EQ(singlepid.graphics, app.graphics());
    EXPECT_EQ(singlepid.private_other, app.private_other());

    memoryPlugin.Stop();
}

/**
 * @tc.name: memory plugin
 * @tc.desc: pid list information test for specific pids.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, Testpluginforpids, TestSize.Level1)
{
    MemoryDataPlugin memoryPlugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;

    std::vector<int> cmpPidList = {1, 2, 11};
    EXPECT_NE((size_t)0, cmpPidList.size());

    memoryPlugin.SetPath(const_cast<char*>(g_path.c_str()));

    SetPluginProcessConfig(cmpPidList, protoConfig);
    EXPECT_TRUE(PluginStub(memoryPlugin, protoConfig, memoryData));

    int index = memoryData.processesinfo_size();
    EXPECT_EQ(3, index);
    for (int i = 0; i < index; ++i) {
        ProcessMemoryInfo it = memoryData.processesinfo(i);
        EXPECT_EQ(g_pidtarget[i].pid, it.pid());
        EXPECT_EQ(g_pidtarget[i].name, it.name());
        EXPECT_EQ(g_pidtarget[i].vm_size_kb, it.vm_size_kb());
        EXPECT_EQ(g_pidtarget[i].vm_rss_kb, it.vm_rss_kb());
        EXPECT_EQ(g_pidtarget[i].rss_anon_kb, it.rss_anon_kb());
        EXPECT_EQ(g_pidtarget[i].rss_file_kb, it.rss_file_kb());
        EXPECT_EQ(g_pidtarget[i].rss_shmem_kb, it.rss_shmem_kb());
        EXPECT_EQ(g_pidtarget[i].vm_locked_kb, it.vm_locked_kb());
        EXPECT_EQ(g_pidtarget[i].vm_hwm_kb, it.vm_hwm_kb());

        EXPECT_EQ(g_pidtarget[i].oom_score_adj, it.oom_score_adj());

        if (i == 0) {
            EXPECT_TRUE(it.has_memsummary());
        }
    }

    memoryPlugin.Stop();
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Smaps stats info test for specific pids.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestSmapsStatsInfo, TestSize.Level1)
{
    const std::vector<int> expectPidList = {1, 2, 11};

    SmapsStats smap(std::string(g_path + "/"));
    for (size_t i = 0; i < expectPidList.size(); i++) {
        ProcessMemoryInfo processMemoryInfo;
        EXPECT_TRUE(smap.ParseMaps(expectPidList[i], processMemoryInfo, true, false));
        EXPECT_EQ(g_pidtarget[i].java_heap, (uint64_t)(smap.GetProcessJavaHeap()));
        EXPECT_EQ(g_pidtarget[i].native_heap, (uint64_t)(smap.GetProcessNativeHeap()));
        EXPECT_EQ(g_pidtarget[i].code, (uint64_t)(smap.GetProcessCode()));
        EXPECT_EQ(g_pidtarget[i].stack, (uint64_t)(smap.GetProcessStack()));
        EXPECT_EQ(g_pidtarget[i].graphics, (uint64_t)(smap.GetProcessGraphics()));
        EXPECT_EQ(g_pidtarget[i].private_other, (uint64_t)(smap.GetProcessPrivateOther()));
    }
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Vmstat info test for specific pids.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestpluginWriteVmstat, TestSize.Level1)
{
    MemoryDataPlugin memoryPlugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;

    protoConfig.set_report_sysmem_vmem_info(true);
    EXPECT_TRUE(PluginStub(memoryPlugin, protoConfig, memoryData));

    memoryPlugin.Stop();
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Get information through MemoryService.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestpluginMemoryService, TestSize.Level1)
{
    MemoryDataPlugin memoryPlugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;

    SetPluginMemoryServiceConfig(protoConfig);
    EXPECT_TRUE(PluginStub(memoryPlugin, protoConfig, memoryData));
    std::string line = "01234567890";
    memoryPlugin.ParseNumber(line);

    ProcessMemoryInfo it = memoryData.processesinfo(0);
    EXPECT_FALSE(it.has_memsummary());
    AppSummary app = it.memsummary();
    EXPECT_EQ((uint64_t)0, app.java_heap());
    EXPECT_EQ((uint64_t)0, app.native_heap());
    EXPECT_EQ((uint64_t)0, app.code());
    EXPECT_EQ((uint64_t)0, app.stack());
    EXPECT_EQ((uint64_t)0, app.graphics());
    EXPECT_EQ((uint64_t)0, app.private_other());

    memoryPlugin.Stop();
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
 * @tc.name: mem plugin
 * @tc.desc: test register
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestRegister, TestSize.Level1)
{
    std::string path = DEFAULT_SO_PATH + std::string("libmemdataplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "memory-plugin");

    // set config
    MemoryConfig config;
    config.set_report_process_mem_info(true);
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
 * @tc.name: mem plugin
 * @tc.desc: start fail test
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestStartFail, TestSize.Level1)
{
    MemoryConfig config;
    MemoryDataPlugin plugin;

    // set config
    config.set_report_process_mem_info(true);

    // serialize
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // start
    EXPECT_NE(plugin.Start(configData.data(), size - 1), 0);
}

/**
 * @tc.name: mem plugin
 * @tc.desc: Framework test
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestFramework, TestSize.Level1)
{
    std::string path = DEFAULT_SO_PATH + std::string("libmemdataplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "memory-plugin");

    // set config
    MemoryConfig config;
    config.set_report_process_mem_info(true);
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // test framework process
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStart(configData.data(), configData.size()), 0);
    EXPECT_EQ(plugin->callbacks->onPluginReportResult(dataBuffer.data(), dataBuffer.size()), 0);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStop(), 0);
}

void OutputData(uint8_t* data, uint32_t size)
{
    MemoryData memoryData;
    int ret = memoryData.ParseFromArray(data, size);
    if (ret <= 0) {
        HILOG_ERROR(LOG_CORE, "MemoryDataPluginTest, %s:parseFromArray failed!", __func__);
        return;
    }

    return;
}

/**
 * @tc.name: mem plugin
 * @tc.desc: ProcessTree test
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestProcessTreeRunTime, TestSize.Level1)
{
    std::string path = DEFAULT_SO_PATH + std::string("libmemdataplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "memory-plugin");

    // set config
    MemoryConfig config;
    config.set_report_process_tree(true);
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // test framework process
    int testCount = 10;
    struct timeval start, end;
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStart(configData.data(), configData.size()), 0);
    clock_t clockstart = clock();
    gettimeofday(&start, nullptr);
    while (testCount--) {
        int ret = plugin->callbacks->onPluginReportResult(dataBuffer.data(), dataBuffer.size());
        ASSERT_GT(ret, 0);
        OutputData(dataBuffer.data(), (uint32_t)ret);
    }
    gettimeofday(&end, nullptr);
    clock_t clockend = clock();
    int timeuse = US_PER_S * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    HILOG_INFO(LOG_CORE, "clock time=%.3fs, timeofday=%.3fs", (double)(clockend - clockstart) / CLOCKS_PER_SEC,
        (double)timeuse / US_PER_S);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStop(), 0);
}

namespace {
const char* DUMP_FORMAT = R"(Applications Memory Usage (in Kilobytes):
Uptime: 559174 Realtime: 559174
App Summary
Pss(KB)
------
Java Heap:  0
Native Heap:    2932
Code:   640
Stack:  60
Graphics:   0
Private Other:  1056
System: 1092
TOTAL:  5780      TOTAL SWAP (KB):        0)";
}

/**
 * @tc.name: mem plugin
 * @tc.desc: test ParseMemInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestParseMemInfo, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    ProcessMemoryInfo memoryInfo;
    uint64_t javaHeap = 0;
    uint64_t nativeHeap = 2932;
    uint64_t code = 640;
    uint64_t stack = 60;
    uint64_t graphics = 0;
    uint64_t other = 1056;
    uint64_t system = 1092;

    ASSERT_TRUE(plugin.ParseMemInfo(DUMP_FORMAT, &memoryInfo));
    // test result
    EXPECT_EQ(memoryInfo.mutable_memsummary()->java_heap(), javaHeap);
    EXPECT_EQ(memoryInfo.mutable_memsummary()->native_heap(), nativeHeap);
    EXPECT_EQ(memoryInfo.mutable_memsummary()->code(), code);
    EXPECT_EQ(memoryInfo.mutable_memsummary()->stack(), stack);
    EXPECT_EQ(memoryInfo.mutable_memsummary()->graphics(), graphics);
    EXPECT_EQ(memoryInfo.mutable_memsummary()->private_other(), other);
    EXPECT_EQ(memoryInfo.mutable_memsummary()->system(), system);
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
 * @tc.name: mem plugin
 * @tc.desc: test ParseMemInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestPid, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    MemoryConfig config;

    std::string cmd = "chmod 777 " + DEFAULT_BIN_PATH;
    system(cmd.c_str());
    pid_t pid1 = fork();
    if (pid1 == 0) {
        std::vector<std::string> argv = {"childpidtest1", "10"};
        ASSERT_TRUE(ExecuteBin(DEFAULT_BIN_PATH, argv));
    }
    pid_t pid2 = fork();
    if (pid2 == 0) {
        std::vector<std::string> argv = {"childpidtest2", "1"};
        ASSERT_TRUE(ExecuteBin(DEFAULT_BIN_PATH, argv));
    }
    sleep(1);

    // set config
    config.set_report_process_mem_info(true);
    config.set_report_app_mem_info(true);
    config.add_pid(pid1);
    config.add_pid(pid2);
    // check result
    EXPECT_TRUE(PluginStub(plugin, config, memoryData));
    EXPECT_GT(memoryData.processesinfo(0).vm_size_kb(), memoryData.processesinfo(1).vm_size_kb());

    while (waitpid(-1, nullptr, WNOHANG) == 0) {
        kill(pid1, SIGKILL);
        kill(pid2, SIGKILL);
    }
    plugin.Stop();
}
} // namespace
