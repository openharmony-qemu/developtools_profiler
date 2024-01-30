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
constexpr uint32_t PAGE_SIZE = 4 * 1024;
constexpr int LINE_SIZE = 1000;

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
    uint64_t purg_sum_kb;
    uint64_t purg_pin_kb;
};

TestElement g_pidtarget[] = {
    {1, "systemd", 226208, 9388, 2984, 6404, 0, 0, 0, 9616, -1, 3036, 4256, 288, 748, 0, 1388, 10232, 400},
    {2, "kthreadd", 0, 0, 0, 0, 0, 0, 0, 0, -100, 3036, 4260, 336, 760, 0, 4204, 0, 0},
    {11, "rcu_sched", 0, 0, 0, 0, 0, 0, 0, 0, 0, 3036, 4272, 400, 772, 0, 7160, 103, 0},
};

unsigned long g_meminfo[] = {
    16168696, 1168452, 12363564, 2726188, 7370484, 29260, 8450388, 4807668,
    2535372,  658832, 4148836, 10, 5678, 116790, 132, 0, 63999996, 62211580, 0
};

unsigned long g_vmeminfo[] = {
    112823, 0,      587,    1848,   101,   9074,  8426,   18314,
    0,     2416,  2348,  9073,   1876,  26863, 1,      0
};

struct AshMemInfo {
    std::string name;
    int32_t pid;
    int32_t fd;
    int32_t adj;
    std::string ashmem_name;
    uint64_t size;
    int32_t id;
    uint64_t time;
    uint64_t ref_count;
    uint64_t purged;
};

AshMemInfo g_ashMemInfo[] = {
    {"com.ohos.settin", 1474, 46, 1, "dev/ashmem/SharedBlock:/data/storage/el2/database/entry/rdb/settingsdata.db",
     2097152, 1, 1282999603, 1, 1},
    {"com.ohos.launch", 1515, 54, 1, "dev/ashmem/hooknativesmb", 67108864, 10, 1282945782, 1, 0},
    {"hiprofilerd", 6746, 27, 1, "dev/ashmem/memory-plugin", 67108864, 12, 1287845167, 0, 0},
    {"hiprofiler_plug", 6756, 7, 1, "dev/ashmem/memory-plugin", 67108864, 15, 1358999004, 0, 0},
};

struct ProcessGpuInfo {
    std::string addr;
    int32_t pid;
    int32_t tid;
    uint64_t used_gpu_size;
};

struct GpuMemInfo {
    std::string gpu_name;
    uint64_t all_gpu_size;
    ProcessGpuInfo gpuInfo[3];
};

GpuMemInfo g_gpuMemInfo = {
    "mali0",
    30217,
    {
        {"kctx-0xffffffc0108f5000", 1149, 1226, 30212},
        {"kctx-0xffffffc0108f6000", 1049, 1216, 1},
        {"kctx-0xffffffc0108f7000", 1206, 1206, 4}
    }
};

struct DmaMemInfo {
    std::string name;
    int32_t pid;
    int32_t fd;
    uint64_t size;
    int32_t ino;
    int32_t expPid;
    std::string exp_task_comm;
    std::string buf_name;
    std::string exp_name;
};

DmaMemInfo g_dmaMemInfo[] = {
    {"ispserver", 433, 18, 12288, 3041, 433, "ispserver", "NULL", "videobuf2_vmalloc"},
    {"ispserver", 433, 19, 12288, 3042, 433, "ispserver", "ispserver1", "videobuf2_vmalloc"},
    {"ispserver", 433, 20, 12288, 3043, 433, "ispserver", "ispserver4", "videobuf2_vmalloc"},
    {"ispserver", 433, 21, 12288, 3044, 433, "ispserver", "NULL", "videobuf2_vmalloc"},
    {"render_service", 624, 9, 3686400, 28914, 624, "render_service", "NULL", "rockchipdrm"},
    {"render_service", 624, 30, 3686400, 30144, 539, "disp_gralloc_ho", "1", "rockchipdrm"},
    {"render_service", 624, 32, 3686400, 31026, 539, "disp_gralloc_ho", "5", "rockchipdrm"},
    {"render_service", 624, 34, 3686400, 29650, 539, "disp_gralloc_ho", "disp_gralloc_ho78", "rockchipdrm"},
    {"render_service", 624, 37, 3686400, 32896, 539, "disp_gralloc_ho", "0", "rockchipdrm"},
    {"render_service", 624, 38, 3686400, 30860, 539, "disp_gralloc_ho", "NULL", "rockchipdrm"},
    {"render_service", 624, 39, 3686400, 30860, 539, "disp_gralloc_ho", "NULL", "rockchipdrm"},
    {"render_service", 624, 58, 3686400, 30145, 539, "disp_gralloc_ho", "NULL", "rockchipdrm"},
    {"render_service", 624, 59, 208896, 32895, 539, "disp_gralloc_ho", "NULL", "rockchipdrm"},
    {"render_service", 624, 66, 3686400, 32896, 539, "disp_gralloc_ho", "NULL", "rockchipdrm"},
    {"com.ohos.system", 1298, 56, 3686400, 32896, 539, "disp_gralloc_ho", "NULL", "rockchipdrm"},
    {"com.ohos.system", 1298, 60, 3686400, 32899, 539, "disp_gralloc_ho", "NULL", "rockchipdrm"},
    {"com.ohos.system", 1298, 62, 3686400, 32901, 539, "disp_gralloc_ho", "NULL", "rockchipdrm"},
};

struct GpuSubTestInfo {
    std::string category_name;
    std::string size;
    std::string type;
    int32_t entryNum;
};

struct GpuDetailTestInfo {
    std::string module_name;
    GpuSubTestInfo gpu_sub_info[4];
};

struct GpuDumpTestInfo {
    std::string window_name;
    uint64_t id;
    GpuDetailTestInfo gpu_detail_info[5];
    std::string gpu_purgeable_size;
    std::string type;
};

GpuDumpTestInfo g_gpudumpAllInfo = {
    "",
    0,
    {
        {
            "skia/gr_text_blob_cache",
            {
                {"Other", "3.33", "KB", 1}
            }
        },
        {
            "SW Path Mask",
            {
                {"Texture", "352.34", "KB", 12}
            }
        },
        {
            "Other",
            {
                {"Buffer Object", "914.00", "bytes", 4},
                {"StencilAttachment", "4.50", "MB", 2}
            }
        },
        {
            "Image",
            {
                {"Texture", "7.98", "MB", 1}
            }
        },
        {
            "Scratch",
            {
                {"Texture", "1.00", "MB", 1},
                {"Buffer Object", "78.00", "KB", 2},
                {"RenderTarget", "29.42", "MB", 3},
                {"StencilAttachment", "14.21", "MB", 1}
            }
        }
    },
    "23.69",
    "MB",
};

GpuDumpTestInfo g_gpudumpInfo1 = {
    "SysUI_Volume",
    10007273799686,
    {
        {
            "SW Path Mask",
            {
                {"Texture", "352.34", "KB", 12}
            }
        },
        {
            "skia/gr_text_blob_cache",
            {
                {"Other", "3.33", "KB", 1}
            }
        }
    },
    "0.00",
    "bytes",
};

GpuDumpTestInfo g_gpudumpInfo2 = {
    "capture",
    0,
    {
        {
            "skia/gr_text_blob_cache",
            {
                {"Other", "3.33", "KB", 1}
            }
        }
    },
    "0.00",
    "bytes",
};

struct RSImageDumpInfo {
    uint64_t size; // bytes
    std::string type;
    int32_t pid;
    std::string name;
};

RSImageDumpInfo g_rSImageDumpInfo[] = {
    {12441600, "pixelmap", 2230, "SCBScreenLock12"}, {331776, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {82944, "pixelmap", 2230, "NONE"}, {200704, "pixelmap", 2230, "NONE"},
    {1106000, "pixelmap", 2230, "SCBNegativeScreen3"}, {147456, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {173056, "pixelmap", 25177, "music0"},
    {3349548, "skimage", 2230, "NONE"}, {186624, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {518400, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {19600, "pixelmap", 3485, "ArkTSCardNode"}, {147456, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {254016, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {589824, "pixelmap", 3485, "ArkTSCardNode"},
    {147456, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {1788696, "skimage", 2230, "SCBNegativeScreen3"}, {147456, "pixelmap", 2230, "NONE"},
    {929600, "pixelmap", 3485, "ArkTSCardNode"}, {12441600, "pixelmap", 2230, "SCBWallpaper1"},
    {331776, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {160000, "pixelmap", 0, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {65536, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {331776, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {262144, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {147456, "pixelmap", 2230, "NONE"}, {3538944, "pixelmap", 25111, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"},
    {331776, "pixelmap", 2230, "NONE"}, {147456, "pixelmap", 2230, "NONE"}
};

struct WinMgrSvcInfo {
    std::string name;
    int32_t pid;
};

WinMgrSvcInfo g_winMgrSvcInfo[] = {
    {"SCBWallpaper1", 2230}, {"SCBDesktop2", 2230}, {"SCBStatusBar7", 2230}, {"SCBGestureBack10", 2230},
    {"SCBScreenLock12", 2230}, {"SCBNegativeScreen3", 2230}, {"SCBGlobalSearch4", 2230}, {"BlurComponent5", 2230},
    {"SCBBannerNotificatio", 2230}, {"SCBDropdownPanel8", 2230}, {"SCBVolumePanel9", 2230},
    {"SCBSysDialogDefault1", 2230}, {"SCBSysDialogUpper13", 2230}, {"imeWindow", 3109}, {"hmscore0", 24704},
    {"himovie0", 24901}, {"music0", 25177}
};

struct ProfileInfo {
    std::string channel;
    uint64_t size;
};

ProfileInfo g_profileInfo[] = {
    {"Unnamed", 630784}, {"Default Heap", 13281584}, {"Framepool", 0}, {"Frame Internal", 0}, {"GPU Program", 368640},
    {"EGL Color Plane", 0}, {"GLES Vertex Array Object", 0}, {"Image Descriptor", 3360}, {"Texture", 80546816},
    {"Buffer", 273814}, {"CRC Buffer", 538960}, {"CPOM Host Memory", 4404512}, {"CPOM Render State", 0},
    {"CPOM Compute Shader", 0}, {"CPOM Static Data", 0}, {"CFRAME Host Memory", 0}, {"CFRAME Sample Position", 0},
    {"CFRAME Discardable FBD", 0}, {"CFRAME Tessellation/Geometry", 0}, {"COBJ Host Memory", 782360},
    {"CMAR Host Memory", 52932608}, {"CMAR Profiling/Dumping", 0}, {"CBLEND Host Memory", 0},
    {"GLES Host Memory", 12828704}, {"GLES Query/XFB/Unroll", 0}, {"GLES Multiview", 0}, {"CDEPS Host Memory", 720896},
    {"CMEM Sub-allocators", 30341768}, {"CMEM Hoard", 2897728}, {"CMEM Registry", 0}, {"CL Command Payloads", 0},
    {"CL Workgroup/Thread", 0}, {"CL Host Memory", 0}, {"CL Shared Virtual Memory", 0}, {"CINSTR Memory", 0},
    {"GFX Device Memory CPU Uncached", 0}, {"GFX Device Memory CPU Cached", 0}, {"GFX Device Memory Transient", 0},
    {"GFX Device Memory Protected", 0}, {"GFX Device External Memory", 0}, {"GFX Device External Swapchain Memory", 0},
    {"GFX Device Internal Memory", 3678208}, {"GFX Device Internal Host Memory", 22879792},
    {"GFX Device Internal Protected Memory", 0}, {"GFX Descriptor Pool Memory", 0}, {"GFX Command Allocator Memory", 0},
    {"GFX Command Allocator Host Memory", 0}, {"GFX Command Allocator Protected Memory", 0},
    {"Vulkan Bound Buffer Memory", 0}, {"Vulkan Bound Image Memory", 0}, {"CMAR Signal Memory", 8192},
    {"CMAR Flush Chain Memory", 0}, {"CMAR Metadata List Memory", 8192}
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

int GetPid(const std::string processName)
{
    int pid = -1;
    std::string findpid = "pidof " + processName;
    PROFILER_LOG_INFO(LOG_CORE, "find pid command : %s", findpid.c_str());
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(findpid.c_str(), "r"), pclose);

    char line[LINE_SIZE];
    do {
        if (fgets(line, sizeof(line), pipe.get()) == nullptr) {
            PROFILER_LOG_INFO(LOG_CORE, "not find processName : %s", processName.c_str());
            return pid;
        } else if (strlen(line) > 0 && isdigit(static_cast<unsigned char>(line[0]))) {
            pid = atoi(line);
            PROFILER_LOG_INFO(LOG_CORE, "find processName : %s, pid: %d", processName.c_str(), pid);
            break;
        }
    } while (1);

    return pid;
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
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_ACTIVE_PURG);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_INACTIVE_PURG);
    protoConfig.add_sys_meminfo_counters(SysMeminfoType::PMEM_PINED_PURG);

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

    EXPECT_EQ(19, memoryData.meminfo().size());
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
        EXPECT_EQ(g_pidtarget[i].purg_sum_kb, it.purg_sum_kb());
        EXPECT_EQ(g_pidtarget[i].purg_pin_kb, it.purg_pin_kb());

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
    EXPECT_EQ(2, index); // 2: the size of processinfo

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
    EXPECT_EQ(singlepid.purg_sum_kb, it.purg_sum_kb());
    EXPECT_EQ(singlepid.purg_pin_kb, it.purg_pin_kb());

    EXPECT_EQ(singlepid.oom_score_adj, it.oom_score_adj());

    EXPECT_TRUE(it.has_memsummary());
    AppSummary app = it.memsummary();
    EXPECT_EQ(singlepid.java_heap, app.java_heap());
    EXPECT_EQ(singlepid.native_heap, app.native_heap());
    EXPECT_EQ(singlepid.code, app.code());
    EXPECT_EQ(singlepid.stack, app.stack());
    EXPECT_EQ(singlepid.graphics, app.graphics());

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
    EXPECT_EQ(6, index); // 3: the size of processinfo
    for (int i = 0; i < 3; ++i) {
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
        EXPECT_EQ(g_pidtarget[i].purg_sum_kb, it.purg_sum_kb());
        EXPECT_EQ(g_pidtarget[i].purg_pin_kb, it.purg_pin_kb());

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
        SmapsInfo* smapsInfo = nullptr;
        EXPECT_TRUE(smap.ParseMaps(expectPidList[i], processMemoryInfo, smapsInfo, true, false));
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
        PROFILER_LOG_ERROR(LOG_CORE, "MemoryDataPluginTest, %s:parseFromArray failed!", __func__);
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
    PROFILER_LOG_INFO(LOG_CORE, "clock time=%.3fs, timeofday=%.3fs", (double)(clockend - clockstart) / CLOCKS_PER_SEC,
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

    ASSERT_TRUE(plugin.ParseMemInfo(DUMP_FORMAT, memoryInfo));
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

/**
 * @tc.name: mem plugin
 * @tc.desc: test WriteAshmemInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestWriteAshmemInfo, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    plugin.SetPath(const_cast<char*>(g_path.c_str()));
    plugin.WriteAshmemInfo(memoryData);

    const int size = 4;
    ASSERT_EQ(memoryData.ashmeminfo().size(), size);
    for (int i = 0; i < size; i++) {
        auto ashmemInfo = memoryData.ashmeminfo(i);
        EXPECT_STREQ(ashmemInfo.name().c_str(), g_ashMemInfo[i].name.c_str());
        EXPECT_EQ(ashmemInfo.pid(), g_ashMemInfo[i].pid);
        EXPECT_EQ(ashmemInfo.adj(), g_ashMemInfo[i].adj);
        EXPECT_EQ(ashmemInfo.fd(), g_ashMemInfo[i].fd);
        EXPECT_STREQ(ashmemInfo.ashmem_name().c_str(), g_ashMemInfo[i].ashmem_name.c_str());
        EXPECT_EQ(ashmemInfo.size(), g_ashMemInfo[i].size);
        EXPECT_EQ(ashmemInfo.id(), g_ashMemInfo[i].id);
        EXPECT_EQ(ashmemInfo.time(), g_ashMemInfo[i].time);
        EXPECT_EQ(ashmemInfo.ref_count(), g_ashMemInfo[i].ref_count);
        EXPECT_EQ(ashmemInfo.purged(), g_ashMemInfo[i].purged);
    }
}

/**
 * @tc.name: mem plugin
 * @tc.desc: test WriteDmaInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestWriteDmaInfo, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    plugin.SetPath(const_cast<char*>(g_path.c_str()));
    plugin.WriteDmaInfo(memoryData);

    const int size = 17;
    ASSERT_EQ(memoryData.dmainfo().size(), size);
    for (int i = 0; i < size; i++) {
        auto dmaInfo = memoryData.dmainfo(i);
        EXPECT_STREQ(dmaInfo.name().c_str(), g_dmaMemInfo[i].name.c_str());
        EXPECT_EQ(dmaInfo.pid(), g_dmaMemInfo[i].pid);
        EXPECT_EQ(dmaInfo.fd(), g_dmaMemInfo[i].fd);
        EXPECT_EQ(dmaInfo.size(), g_dmaMemInfo[i].size);
        EXPECT_EQ(dmaInfo.ino(), g_dmaMemInfo[i].ino);
        EXPECT_EQ(dmaInfo.exp_pid(), g_dmaMemInfo[i].expPid);
        EXPECT_STREQ(dmaInfo.exp_task_comm().c_str(), g_dmaMemInfo[i].exp_task_comm.c_str());
        EXPECT_STREQ(dmaInfo.buf_name().c_str(), g_dmaMemInfo[i].buf_name.c_str());
        EXPECT_STREQ(dmaInfo.exp_name().c_str(), g_dmaMemInfo[i].exp_name.c_str());
    }
}

/**
 * @tc.name: mem plugin
 * @tc.desc: test WriteGpuMemInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestWriteGpuMemInfo, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    plugin.SetPath(const_cast<char*>(g_path.c_str()));
    plugin.WriteGpuMemInfo(memoryData);
    ASSERT_EQ(memoryData.gpumemoryinfo().size(), 1);

    auto gpuMemoryInfo = memoryData.gpumemoryinfo(0);
    EXPECT_STREQ(gpuMemoryInfo.gpu_name().c_str(), g_gpuMemInfo.gpu_name.c_str());
    EXPECT_EQ(gpuMemoryInfo.all_gpu_size(), g_gpuMemInfo.all_gpu_size * PAGE_SIZE);

    const int size = 3;
    ASSERT_EQ(gpuMemoryInfo.gpu_process_info().size(), size);
    for (int i = 0; i < size; i++) {
        auto gpuProcessInfo = gpuMemoryInfo.gpu_process_info(i);
        EXPECT_STREQ(gpuProcessInfo.addr().c_str(), g_gpuMemInfo.gpuInfo[i].addr.c_str());
        EXPECT_EQ(gpuProcessInfo.pid(), g_gpuMemInfo.gpuInfo[i].pid);
        EXPECT_EQ(gpuProcessInfo.tid(), g_gpuMemInfo.gpuInfo[i].tid);
        EXPECT_EQ(gpuProcessInfo.used_gpu_size(), g_gpuMemInfo.gpuInfo[i].used_gpu_size * PAGE_SIZE);
    }
}

/**
 * @tc.name: mem plugin
 * @tc.desc: test WriteGpuDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestWriteGpuDumpInfo, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    plugin.SetPath(const_cast<char*>(g_path.c_str()));
    plugin.WriteGpuDumpInfo(memoryData);

    const int size = 23;
    ASSERT_EQ(memoryData.gpudumpinfo().size(), size);

    auto gpuDumpAllInfo = memoryData.gpudumpinfo(0);
    EXPECT_STREQ(gpuDumpAllInfo.window_name().c_str(), g_gpudumpAllInfo.window_name.c_str());
    EXPECT_EQ(gpuDumpAllInfo.id(), g_gpudumpAllInfo.id);
    const int detailAllSize = 5;
    ASSERT_EQ(gpuDumpAllInfo.gpu_detail_info().size(), detailAllSize);
    for (int i = 0; i < detailAllSize; i++) {
        auto detailInfo = gpuDumpAllInfo.gpu_detail_info(i);
        auto detailInfoTest = g_gpudumpAllInfo.gpu_detail_info[i];
        EXPECT_STREQ(detailInfo.module_name().c_str(), detailInfoTest.module_name.c_str());
        if (i == 0 || i == 1 || i == 3) {
            ASSERT_EQ(detailInfo.gpu_sub_info().size(), 1);
            auto subInfo = detailInfo.gpu_sub_info(0);
            auto subInfoTest = detailInfoTest.gpu_sub_info[0];
            EXPECT_STREQ(subInfo.category_name().c_str(), subInfoTest.category_name.c_str());
            EXPECT_EQ(subInfo.size(), plugin.SizeToBytes(subInfoTest.size, subInfoTest.type));
            EXPECT_EQ(subInfo.entry_num(), subInfoTest.entryNum);
        } else if (i == 2) {
            const int subSize = 2;
            ASSERT_EQ(detailInfo.gpu_sub_info().size(), subSize);
            for (int j = 0; j < subSize; j++) {
                auto subInfo = detailInfo.gpu_sub_info(j);
                auto subInfoTest = detailInfoTest.gpu_sub_info[j];
                EXPECT_STREQ(subInfo.category_name().c_str(), subInfoTest.category_name.c_str());
                EXPECT_EQ(subInfo.size(), plugin.SizeToBytes(subInfoTest.size, subInfoTest.type));
                EXPECT_EQ(subInfo.entry_num(), subInfoTest.entryNum);
            }
        } else if (i == 4) {
            const int subSize = 4;
            ASSERT_EQ(detailInfo.gpu_sub_info().size(), subSize);
            for (int j = 0; j < subSize; j++) {
                auto subInfo = detailInfo.gpu_sub_info(j);
                auto subInfoTest = detailInfoTest.gpu_sub_info[j];
                EXPECT_STREQ(subInfo.category_name().c_str(), subInfoTest.category_name.c_str());
                EXPECT_EQ(subInfo.size(), plugin.SizeToBytes(subInfoTest.size, subInfoTest.type));
                EXPECT_EQ(subInfo.entry_num(), subInfoTest.entryNum);
            }
        }
    }
    EXPECT_EQ(gpuDumpAllInfo.gpu_purgeable_size(), plugin.SizeToBytes(g_gpudumpAllInfo.gpu_purgeable_size,
                                                                      g_gpudumpAllInfo.type));

    auto gpuDumpInfo1 = memoryData.gpudumpinfo(1);
    EXPECT_STREQ(gpuDumpInfo1.window_name().c_str(), g_gpudumpInfo1.window_name.c_str());
    EXPECT_EQ(gpuDumpInfo1.id(), g_gpudumpInfo1.id);
    const int detailSize = 2;
    ASSERT_EQ(gpuDumpInfo1.gpu_detail_info().size(), detailSize);
    for (int i = 0; i < detailSize; i++) {
        auto detailInfo = gpuDumpInfo1.gpu_detail_info(i);
        auto detailInfoTest = g_gpudumpInfo1.gpu_detail_info[i];
        EXPECT_STREQ(detailInfo.module_name().c_str(), detailInfoTest.module_name.c_str());
        ASSERT_EQ(detailInfo.gpu_sub_info().size(), 1);
        auto subInfo = detailInfo.gpu_sub_info(0);
        auto subInfoTest = detailInfoTest.gpu_sub_info[0];
        EXPECT_STREQ(subInfo.category_name().c_str(), subInfoTest.category_name.c_str());
        EXPECT_EQ(subInfo.size(), plugin.SizeToBytes(subInfoTest.size, subInfoTest.type));
        EXPECT_EQ(subInfo.entry_num(), subInfoTest.entryNum);
    }
    EXPECT_EQ(gpuDumpInfo1.gpu_purgeable_size(), plugin.SizeToBytes(g_gpudumpInfo1.gpu_purgeable_size,
                                                                    g_gpudumpInfo1.type));

    const int rSDumpSize = 66;
    ASSERT_EQ(memoryData.rsdumpinfo().size(), rSDumpSize);
    for (int i = 0; i < rSDumpSize; i++) {
        auto rsDumpInfo = memoryData.rsdumpinfo(i);
        EXPECT_EQ(rsDumpInfo.size(), g_rSImageDumpInfo[i].size);
        EXPECT_STREQ(rsDumpInfo.type().c_str(), g_rSImageDumpInfo[i].type.c_str());
        EXPECT_EQ(rsDumpInfo.pid(), g_rSImageDumpInfo[i].pid);
        EXPECT_STREQ(rsDumpInfo.surface_name().c_str(), g_rSImageDumpInfo[i].name.c_str());
    }

    auto gpuDumpInfo2 = memoryData.gpudumpinfo(size - 1);
    EXPECT_STREQ(gpuDumpInfo2.window_name().c_str(), g_gpudumpInfo2.window_name.c_str());
    EXPECT_EQ(gpuDumpInfo2.id(), g_gpudumpInfo2.id);
    ASSERT_EQ(gpuDumpInfo2.gpu_detail_info().size(), 1);
    auto detailInfo = gpuDumpInfo2.gpu_detail_info(0);
    auto detailInfoTest = g_gpudumpInfo2.gpu_detail_info[0];
    EXPECT_STREQ(detailInfo.module_name().c_str(), detailInfoTest.module_name.c_str());
    ASSERT_EQ(detailInfo.gpu_sub_info().size(), 1);
    auto subInfo = detailInfo.gpu_sub_info(0);
    auto subInfoTest = detailInfoTest.gpu_sub_info[0];
    EXPECT_STREQ(subInfo.category_name().c_str(), subInfoTest.category_name.c_str());
    EXPECT_EQ(subInfo.size(), plugin.SizeToBytes(subInfoTest.size, subInfoTest.type));
    EXPECT_EQ(subInfo.entry_num(), subInfoTest.entryNum);
    EXPECT_EQ(gpuDumpInfo2.gpu_purgeable_size(), plugin.SizeToBytes(g_gpudumpInfo2.gpu_purgeable_size,
                                                                    g_gpudumpInfo2.type));

    const uint64_t gpuLimitSize = 301989888;
    const uint64_t usedSize = 45064738;
    EXPECT_EQ(memoryData.gpu_limit_size(), gpuLimitSize);
    EXPECT_EQ(memoryData.gpu_used_size(), usedSize);
}

/**
 * @tc.name: mem plugin
 * @tc.desc: test WriteDumpProcessInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestWriteDumpProcessInfo, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;
    protoConfig.add_pid(1);
    protoConfig.set_report_process_mem_info(true);
    protoConfig.set_report_gpu_dump_info(true);
    EXPECT_TRUE(PluginStub(plugin, protoConfig, memoryData));
    plugin.Stop();
    const int size = 2;
    ASSERT_EQ(memoryData.processesinfo().size(), size);
    EXPECT_EQ(memoryData.processesinfo(1).pid(), 1);
    EXPECT_GE(memoryData.processesinfo(1).gl_pss_kb(), 0);
    EXPECT_GE(memoryData.processesinfo(1).graph_pss_kb(), 0);

    int pid = GetPid("render_service");
    if (pid > 0) {
        protoConfig.add_pid(pid);
        EXPECT_TRUE(PluginStub(plugin, protoConfig, memoryData));
        plugin.Stop();
        const int size = 4;
        ASSERT_EQ(memoryData.processesinfo().size(), size);
        EXPECT_EQ(memoryData.processesinfo(size - 1).pid(), pid);
        EXPECT_GE(memoryData.processesinfo(size - 1).gl_pss_kb(), 0);
        EXPECT_GE(memoryData.processesinfo(size - 1).graph_pss_kb(), 0);
    }
}

/**
 * @tc.name: mem plugin
 * @tc.desc: test WriteManagerServiceInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestWriteManagerServiceInfo, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;
    plugin.SetPath(const_cast<char*>(g_path.c_str()));
    plugin.WriteManagerServiceInfo(memoryData);

    const int winMgrSvcSize = 17;
    ASSERT_EQ(memoryData.windowinfo().size(), winMgrSvcSize);
    for (int i = 0; i < winMgrSvcSize; i++) {
        auto windowInfo = memoryData.windowinfo(i);
        EXPECT_STREQ(windowInfo.window_name().c_str(), g_winMgrSvcInfo[i].name.c_str());
        EXPECT_EQ(windowInfo.pid(), g_winMgrSvcInfo[i].pid);
    }
}

/**
 * @tc.name: mem plugin
 * @tc.desc: test WriteProfileMemInfo
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestWriteProfileMemInfo, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;
    protoConfig.add_pid(1);
    plugin.SetPath(const_cast<char*>(g_path.c_str()));
    plugin.SetProtoConfig(protoConfig);
    plugin.WriteProfileMemInfo(memoryData);

    const int profileMemSize = 53;
    ASSERT_EQ(memoryData.profilememinfo().size(), profileMemSize);
    for (int i = 0; i < profileMemSize; i++) {
        auto profileMemInfo = memoryData.profilememinfo(i);
        EXPECT_STREQ(profileMemInfo.channel().c_str(), g_profileInfo[i].channel.c_str());
        EXPECT_EQ(profileMemInfo.total_memory_size(), g_profileInfo[i].size);
    }
}

/**
 * @tc.name: mem plugin
 * @tc.desc: test GpuData
 * @tc.type: FUNC
 */
HWTEST_F(MemoryDataPluginTest, TestGpuData, TestSize.Level1)
{
    MemoryDataPlugin plugin;
    MemoryData memoryData;
    MemoryConfig protoConfig;
    protoConfig.add_pid(1);
    protoConfig.set_report_process_mem_info(true);
    protoConfig.set_report_purgeable_ashmem_info(true);
    protoConfig.set_report_dma_mem_info(true);
    protoConfig.set_report_gpu_mem_info(true);
    protoConfig.set_report_gpu_dump_info(true);
    plugin.SetPath(const_cast<char*>(g_path.c_str()));
    EXPECT_TRUE(PluginStub(plugin, protoConfig, memoryData));
    plugin.Stop();

    const int processesSize = 2;
    const int ashmemSize = 4;
    const int dmaSize = 17;
    const int dumpSize = 23;
    const uint64_t gpuLimitSize = 301989888;
    const uint64_t usedSize = 45064738;
    EXPECT_EQ(memoryData.processesinfo().size(), processesSize);
    EXPECT_EQ(memoryData.ashmeminfo().size(), ashmemSize);
    EXPECT_EQ(memoryData.dmainfo().size(), dmaSize);
    EXPECT_EQ(memoryData.gpumemoryinfo().size(), 1);
    EXPECT_EQ(memoryData.gpudumpinfo().size(), dumpSize);
    EXPECT_EQ(memoryData.gpu_limit_size(), gpuLimitSize);
    EXPECT_EQ(memoryData.gpu_used_size(), usedSize);
}
} // namespace
