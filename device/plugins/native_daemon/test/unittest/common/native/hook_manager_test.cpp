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

#include <gtest/gtest.h>
#include "command_poller.h"
#include "hook_manager.h"
#include "hook_service.h"
#include "hook_socket_client.h"
#include "parameters.h"
#include "socket_context.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;
namespace {
const std::string OUTPUT_PATH = "/data/local/tmp/hiprofiler_data.htrace";
const int SMB_PAGES = 16384;
const int TEST_MAX_JS_STACK_DEPTH = 10;
class HookManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
#ifdef COVERAGE_TEST
        const int coverageSleepTime = 5; // sleep 5s
        sleep(coverageSleepTime);
#else
        sleep(1); // 睡眠1s确保hiprofilerd进程启动
#endif
    }
    static void TearDownTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
    }
};

std::string CreateCommand(const std::string& outputFile, const int32_t time, const std::string& processName)
{
    std::ostringstream cmdStream;
    cmdStream << "hiprofiler_cmd \\\n"
              << "-c - \\\n"
              << "-o " << outputFile << " \\\n"
              << "-t " << time << " \\\n"
              << "-s \\\n"
              << "-k \\\n"
              << "<<CONFIG\n"
              << "request_id: 1\n"
              << "session_config {\n"
              << "  buffers {\n"
              << "    pages: 14848" << "\n"
              << "  }\n"
              << "}\n"
              << "plugin_configs {\n"
              << "  plugin_name: \"nativehook\"\n"
              << "  config_data {\n"
              << "process_name: \"" << processName << "\"\n"
              << "smb_pages: " << SMB_PAGES << "\n"
              << "dump_nmd: true\n"
              << "  }\n"
              << "}\n"
              << "CONFIG\n";
    return cmdStream.str();
}

bool RunCommand(const std::string& cmd, std::string& content)
{
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    CHECK_TRUE(pipe, false, "RunCommand: create popen FAILED!");
    static constexpr int buffSize = 1024;
    std::array<char, buffSize> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        content += buffer.data();
    }
    return true;
}

/*
 * @tc.name: RegisterPlugin
 * @tc.desc: test HookManager::RegisterAgentPlugin with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, RegisterPlugin, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(hookManager);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_TRUE(commandPoller->OnConnect());
    hookManager->SetCommandPoller(commandPoller);
    ASSERT_TRUE(hookManager->RegisterAgentPlugin("nativehook"));
    ASSERT_TRUE(hookManager->UnregisterAgentPlugin("nativehook"));
}

/*
 * @tc.name: LoadPlugin
 * @tc.desc: test HookManager::LoadPlugin with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, LoadPlugin, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(hookManager);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_TRUE(commandPoller->OnConnect());
    hookManager->SetCommandPoller(commandPoller);
    ASSERT_TRUE(hookManager->RegisterAgentPlugin("nativehook"));
    ASSERT_TRUE(hookManager->LoadPlugin("nativehook"));
    ASSERT_TRUE(hookManager->UnloadPlugin("nativehook"));
    ASSERT_TRUE(hookManager->UnregisterAgentPlugin("nativehook"));
}

/*
 * @tc.name: UnloadPlugin
 * @tc.desc: test HookManager::UnloadPlugin with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, UnloadPlugin, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(hookManager);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_TRUE(commandPoller->OnConnect());
    hookManager->SetCommandPoller(commandPoller);
    ASSERT_TRUE(hookManager->RegisterAgentPlugin("nativehook"));
    ASSERT_TRUE(hookManager->LoadPlugin("nativehook"));
    ASSERT_TRUE(hookManager->UnloadPlugin(commandPoller->GetRequestId()));
    ASSERT_TRUE(hookManager->UnregisterAgentPlugin("nativehook"));
}

/*
 * @tc.name: PluginSession
 * @tc.desc: test HookManager process with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, PluginSession, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(hookManager);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_TRUE(commandPoller->OnConnect());
    hookManager->SetCommandPoller(commandPoller);

    std::vector<uint32_t> pluginIds(1);
    ProfilerPluginConfig config;
    config.set_name("nativehook");
    config.set_plugin_sha256("");
    config.set_sample_interval(20);

    PluginResult result;
    std::vector<ProfilerPluginConfig> configVec;
    configVec.push_back(config);

    EXPECT_FALSE(hookManager->CreatePluginSession(configVec));
    EXPECT_FALSE(hookManager->StartPluginSession(pluginIds, configVec, result));
    EXPECT_TRUE(hookManager->CreateWriter("name", 0, 0, 0));
    EXPECT_TRUE(hookManager->ResetWriter(0));
    EXPECT_FALSE(hookManager->StopPluginSession(pluginIds));
    EXPECT_TRUE(hookManager->DestroyPluginSession(pluginIds));
}

/*
 * @tc.name: CheckProcess
 * @tc.desc: test CheckProcess with false case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, CheckProcess, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig nativeConfig;
    nativeConfig.set_process_name("HookManagerTest");
    hookManager.SetHookConfig(nativeConfig);
    EXPECT_TRUE(hookManager.CheckProcess());

    nativeConfig.set_startup_mode(true);
    hookManager.SetHookConfig(nativeConfig);
    EXPECT_TRUE(hookManager.CheckProcess());
    hookManager.ResetStartupParam();

    // native_daemon_ut as a testing process
    nativeConfig.set_startup_mode(false);
    nativeConfig.set_process_name("native_daemon_ut");
    hookManager.SetHookConfig(nativeConfig);
    EXPECT_TRUE(hookManager.CheckProcess());
    EXPECT_TRUE(hookManager.CheckProcessName());
}

/*
 * @tc.name: CheckNmdInfo
 * @tc.desc: test CheckNmdInfoe when process is exit.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(HookManagerTest, CheckNmdInfo, TestSize.Level0)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 1, "hiview");
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    EXPECT_TRUE(ret.find("FAIL") == std::string::npos);
    std::string filePath = "/data/local/tmp/nmd_hiview.txt";
    EXPECT_EQ(access(filePath.c_str(), F_OK), 0);

    std::ifstream infile;
    infile.open(filePath, std::ios::in);
    EXPECT_TRUE(infile.is_open());
    std::string buf;
    bool nmdResult = false;
    while (getline(infile, buf)) {
        if (buf.find("jemalloc statistics") != std::string::npos) {
            nmdResult = true;
            break;
        }
    }
    EXPECT_TRUE(nmdResult);
}
#endif

/*
 * @tc.name: CheckNmdInfoe002
 * @tc.desc: test CheckNmdInfoe002 when process is not exit.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, CheckNmdInfo002, TestSize.Level0)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 1, "test_profiler");
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    EXPECT_TRUE(ret.find("FAIL") == std::string::npos);
    std::string filePath = "/data/local/tmp/test_profiler.txt";
    EXPECT_EQ(access(filePath.c_str(), F_OK), -1);
}

/*
 * @tc.name: CheckHapEncryped
 * @tc.desc: test CheckHapEncryped when saMode_ and fp_unwind are both true.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, CheckHapEncryped, TestSize.Level0)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    hookManager->saMode_ = true;
    hookManager->hookConfig_.set_fp_unwind(true);
    hookManager->hookConfig_.set_js_stack_report(1);
    hookManager->hookConfig_.set_max_js_stack_depth(TEST_MAX_JS_STACK_DEPTH);
    hookManager->CheckHapEncryped();
    EXPECT_EQ(hookManager->hookConfig_.js_stack_report(), 1);
    EXPECT_EQ(hookManager->hookConfig_.max_js_stack_depth(), TEST_MAX_JS_STACK_DEPTH);
}

/*
 * @tc.name: ConvertTagToMaskEmptyTagList
 * @tc.desc: test ConvertTagToMask with empty tag list.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskEmptyTagList, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;
    // Empty restrace_tag list
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, 0ULL);
}

/*
 * @tc.name: ConvertTagToMaskSingleTagConversion
 * @tc.desc: test ConvertTagToMask with single tag conversion.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskSingleTagConversion, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Add single tag
    config.add_restrace_tag("RES_GPU_VK");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_GPU_VK);
}

/*
 * @tc.name: ConvertTagToMaskMultipleTagConversion
 * @tc.desc: test ConvertTagToMask with multiple tags conversion.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskMultipleTagConversion, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Add multiple tags
    config.add_restrace_tag("RES_GPU_VK");
    config.add_restrace_tag("RES_FD_OPEN");
    config.add_restrace_tag("RES_THREAD_PTHREAD");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_GPU_VK | RES_FD_OPEN | RES_THREAD_PTHREAD);
}

/*
 * @tc.name: ConvertTagToMaskAllTagsConversion

 * @tc.desc: test ConvertTagToMask with all tags conversion.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskAllTagsConversion, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Add all tags
    config.add_restrace_tag("RES_GPU_VK");
    config.add_restrace_tag("RES_GPU_GLES_IMAGE");
    config.add_restrace_tag("RES_GPU_GLES_BUFFER");
    config.add_restrace_tag("RES_GPU_CL_IMAGE");
    config.add_restrace_tag("RES_GPU_CL_BUFFER");
    config.add_restrace_tag("RES_FD_OPEN");
    config.add_restrace_tag("RES_FD_EPOLL");
    config.add_restrace_tag("RES_FD_EVENTFD");
    config.add_restrace_tag("RES_FD_SOCKET");
    config.add_restrace_tag("RES_FD_PIPE");
    config.add_restrace_tag("RES_FD_DUP");
    config.add_restrace_tag("RES_FD_ALL");
    config.add_restrace_tag("RES_THREAD_PTHREAD");
    config.add_restrace_tag("RES_THREAD_ALL");
    config.add_restrace_tag("RES_ARKTS_HEAP_MASK");
    config.add_restrace_tag("RES_JS_HEAP_MASK");
    config.add_restrace_tag("RES_KMP_HEAP_MASK");
    config.add_restrace_tag("RES_RN_HEAP_MASK");
    config.add_restrace_tag("RES_DMABUF_MASK");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_GPU_VK | RES_GPU_GLES_IMAGE | RES_GPU_GLES_BUFFER | RES_GPU_CL_IMAGE | RES_GPU_CL_BUFFER |
              RES_FD_OPEN | RES_FD_EPOLL | RES_FD_EVENTFD | RES_FD_SOCKET | RES_FD_PIPE | RES_FD_DUP |
              RES_FD_MASK | RES_THREAD_PTHREAD | RES_THREAD_MASK | RES_ARKTS_HEAP_MASK | RES_JS_HEAP_MASK |
              RES_KMP_HEAP_MASK | RES_RN_HEAP_MASK | RES_DMABUF_MASK);
}

/*
 * @tc.name: ConvertTagToMaskInvalidTagHandling
 * @tc.desc: test ConvertTagToMask with invalid tag handling.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskInvalidTagHandling, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Add invalid tag
    config.add_restrace_tag("RES_GPU_VK");
    config.add_restrace_tag("INVALID_TAG");
    config.add_restrace_tag("RES_FD_OPEN");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    // Invalid tags should be ignored, only process valid tags
    EXPECT_EQ(result, RES_GPU_VK | RES_FD_OPEN);
}

/*
 * @tc.name: ConvertTagToMaskDuplicateTagHandling
 * @tc.desc: test ConvertTagToMask with duplicate tag handling.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskDuplicateTagHandling, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Add duplicate tags
    config.add_restrace_tag("RES_GPU_VK");
    config.add_restrace_tag("RES_GPU_VK");
    config.add_restrace_tag("RES_FD_OPEN");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    // Duplicate tags should be ignored, only process unique tags
    EXPECT_EQ(result, RES_GPU_VK | RES_FD_OPEN);
}

/*
 * @tc.name: ConvertTagToMaskSpecificTagCombinations
 * @tc.desc: test ConvertTagToMask with specific tag combinations.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskSpecificTagCombinations, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Test FD related tags
    config.add_restrace_tag("RES_FD_ALL");
    config.add_restrace_tag("RES_FD_OPEN");
    config.add_restrace_tag("RES_FD_SOCKET");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    // RES_FD_ALL should include all FD related flags
    EXPECT_TRUE(result & RES_FD_MASK);
    EXPECT_TRUE(result & RES_FD_OPEN);
    EXPECT_TRUE(result & RES_FD_SOCKET);
}

/*
 * @tc.name: ConvertTagToMaskBoundaryConditions
 * @tc.desc: test ConvertTagToMask with boundary conditions.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskBoundaryConditions, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Test boundary conditions
    config.add_restrace_tag("RES_DMABUF_MASK");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_DMABUF_MASK);
}

/*
 * @tc.name: GetCmdArgsEmptyConfig
 * @tc.desc: test GetCmdArgs with empty config.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, GetCmdArgsEmptyConfig, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;
    
    std::string result = hookManager.GetCmdArgs(config);

    EXPECT_NE(result, "");
    EXPECT_NE(result.find("pid: "), std::string::npos);
    EXPECT_NE(result.find("save_file: false"), std::string::npos);
    EXPECT_NE(result.find("filter_size: 0"), std::string::npos);
    EXPECT_NE(result.find("smb_pages: 0"), std::string::npos);
    EXPECT_NE(result.find("max_stack_depth: 0"), std::string::npos);
    EXPECT_NE(result.find("process_name: "), std::string::npos);
    EXPECT_NE(result.find("malloc_disable: false"), std::string::npos);
    EXPECT_NE(result.find("mmap_disable: false"), std::string::npos);
    EXPECT_NE(result.find("free_stack_report: false"), std::string::npos);
    EXPECT_NE(result.find("munmap_stack_report: false"), std::string::npos);
    EXPECT_NE(result.find("malloc_free_matching_interval: 0"), std::string::npos);
    EXPECT_NE(result.find("malloc_free_matching_cnt: 0"), std::string::npos);
    EXPECT_NE(result.find("string_compressed: false"), std::string::npos);
    EXPECT_NE(result.find("fp_unwind: false"), std::string::npos);
    EXPECT_NE(result.find("blocked: false"), std::string::npos);
    EXPECT_NE(result.find("record_accurately: false"), std::string::npos);
    EXPECT_NE(result.find("startup_mode: false"), std::string::npos);
    EXPECT_NE(result.find("memtrace_enable: false"), std::string::npos);
    EXPECT_NE(result.find("offline_symbolization: false"), std::string::npos);
    EXPECT_NE(result.find("callframe_compress: false"), std::string::npos);
    EXPECT_NE(result.find("statistics_interval: 0"), std::string::npos);
    EXPECT_NE(result.find("clock: "), std::string::npos);
    EXPECT_NE(result.find("sample_interval: 0"), std::string::npos);
    EXPECT_NE(result.find("response_library_mode: false"), std::string::npos);
    EXPECT_NE(result.find("js_stack_report: 0"), std::string::npos);
    EXPECT_NE(result.find("max_js_stack_depth: 0"), std::string::npos);
    EXPECT_NE(result.find("filter_napi_name: "), std::string::npos);
    EXPECT_NE(result.find("target_so_name: "), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsBasicConfig
 * @tc.desc: test GetCmdArgs with basic config.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, GetCmdArgsBasicConfig, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;
    config.set_pid(1);
    config.set_save_file(true);
    config.set_filter_size(1024);
    config.set_smb_pages(64);
    config.set_max_stack_depth(32);
    config.set_process_name("test_process");

    std::string result = hookManager.GetCmdArgs(config);

    EXPECT_NE(result, "");
    // Notice that "pid: test_process" cannot be found.
    EXPECT_NE(result.find("save_file: true"), std::string::npos);
    EXPECT_NE(result.find("filter_size: 1024"), std::string::npos);
    EXPECT_NE(result.find("smb_pages: 64"), std::string::npos);
    EXPECT_NE(result.find("max_stack_depth: 32"), std::string::npos);
    EXPECT_NE(result.find("process_name: test_process"), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsBooleanFlags
 * @tc.desc: test GetCmdArgs with boolean flags.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, GetCmdArgsBooleanFlags, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;
    config.set_malloc_disable(true);
    config.set_mmap_disable(true);
    config.set_free_stack_report(true);
    config.set_munmap_stack_report(true);
    config.set_string_compressed(true);
    config.set_fp_unwind(true);
    config.set_blocked(true);
    config.set_record_accurately(true);
    config.set_startup_mode(true);
    config.set_memtrace_enable(true);
    config.set_offline_symbolization(true);
    config.set_callframe_compress(true);
    config.set_response_library_mode(true);

    std::string result = hookManager.GetCmdArgs(config);
    EXPECT_NE(result.find("malloc_disable: true"), std::string::npos);
    EXPECT_NE(result.find("mmap_disable: true"), std::string::npos);
    EXPECT_NE(result.find("free_stack_report: true"), std::string::npos);
    EXPECT_NE(result.find("munmap_stack_report: true"), std::string::npos);
    EXPECT_NE(result.find("string_compressed: true"), std::string::npos);
    EXPECT_NE(result.find("fp_unwind: true"), std::string::npos);
    EXPECT_NE(result.find("blocked: true"), std::string::npos);
    EXPECT_NE(result.find("record_accurately: true"), std::string::npos);
    EXPECT_NE(result.find("startup_mode: true"), std::string::npos);
    EXPECT_NE(result.find("memtrace_enable: true"), std::string::npos);
    EXPECT_NE(result.find("offline_symbolization: true"), std::string::npos);
    EXPECT_NE(result.find("callframe_compress: true"), std::string::npos);
    EXPECT_NE(result.find("response_library_mode: true"), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsNumericValues
 * @tc.desc: test GetCmdArgs with numeric values.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, GetCmdArgsNumericValues, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;
    config.set_malloc_free_matching_interval(1000);
    config.set_malloc_free_matching_cnt(5);
    config.set_statistics_interval(30);
    config.set_sample_interval(100);
    config.set_js_stack_report(1);
    config.set_max_js_stack_depth(10);
    std::string result = hookManager.GetCmdArgs(config);
    EXPECT_NE(result.find("malloc_free_matching_interval: 1000"), std::string::npos);
    EXPECT_NE(result.find("malloc_free_matching_cnt: 5"), std::string::npos);
    EXPECT_NE(result.find("statistics_interval: 30"), std::string::npos);
    EXPECT_NE(result.find("sample_interval: 100"), std::string::npos);
    EXPECT_NE(result.find("js_stack_report: 1"), std::string::npos);
    EXPECT_NE(result.find("max_js_stack_depth: 10"), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsStringValues
 * @tc.desc: test GetCmdArgs with string values.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, GetCmdArgsStringValues, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;
    config.set_clock("clock: monotonic");
    config.set_filter_napi_name("test");
    config.set_target_so_name("test.so");
    std::string result = hookManager.GetCmdArgs(config);
    EXPECT_NE(result.find("clock: monotonic"), std::string::npos);
    EXPECT_NE(result.find("filter_napi_name: test"), std::string::npos);
    EXPECT_NE(result.find("target_so_name: test.so"), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsExpandPids
 * @tc.desc: test GetCmdArgs with expand_pids.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, GetCmdArgsExpandPids, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;
    config.add_expand_pids(2001);
    config.add_expand_pids(2002);
    std::string result = hookManager.GetCmdArgs(config);
    EXPECT_NE(result.find("expand_pids: 2001"), std::string::npos);
    EXPECT_NE(result.find("expand_pids: 2002"), std::string::npos);
}
} // namespace