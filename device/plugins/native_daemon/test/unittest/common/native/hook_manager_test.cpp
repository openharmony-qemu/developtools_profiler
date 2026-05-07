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
#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include "command_poller.h"
#include "hook_manager.h"
#include "hook_service.h"
#include "hook_socket_client.h"
#include "parameters.h"
#include "socket_context.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;
namespace {
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
 * @tc.name: ConvertTagToMaskSingleTagLocalHandle
 * @tc.desc: test ConvertTagToMask with single tag local handle.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskSingleTagLocalHandle, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Add single tag
    config.add_restrace_tag("RES_ARK_LOCAL_HANDLE");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_ARK_LOCAL_HANDLE);
}

/*
 * @tc.name: ConvertTagToMaskSingleTagVMA
 * @tc.desc: test ConvertTagToMask with single tag local handle.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskSingleTagVMA, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    // Add single tag
    config.add_restrace_tag("RES_VMA_ARKWEB");
    hookManager.SetHookConfig(config);
    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_VMA_ARKWEB);

    config.clear_restrace_tag();
    config.add_restrace_tag("RES_KOTLIN_HEAP_MASK");
    hookManager.SetHookConfig(config);
    result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_KOTLIN_HEAP_MASK);
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
    config.add_restrace_tag("RES_ARKTS_STATIC_HEAP");
    config.add_restrace_tag("RES_JS_HEAP_MASK");
    config.add_restrace_tag("RES_KOTLIN_HEAP_MASK");
    config.add_restrace_tag("RES_DMABUF_MASK");
    config.add_restrace_tag("RES_DART_HEAP");
    config.add_restrace_tag("RES_RN_HERMES_HEAP");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_GPU_VK | RES_GPU_GLES_IMAGE | RES_GPU_GLES_BUFFER | RES_GPU_CL_IMAGE | RES_GPU_CL_BUFFER |
              RES_FD_OPEN | RES_FD_EPOLL | RES_FD_EVENTFD | RES_FD_SOCKET | RES_FD_PIPE | RES_FD_DUP |
              RES_FD_MASK | RES_THREAD_PTHREAD | RES_THREAD_MASK | RES_ARKTS_HEAP_MASK | RES_JS_HEAP_MASK |
              RES_KOTLIN_HEAP_MASK | RES_DMABUF_MASK | RES_DART_HEAP | RES_RN_HERMES_HEAP | RES_ARKTS_STATIC_HEAP);
}

/*
 * @tc.name: ConvertTagToMaskArkTSStaticHeapConversion
 * @tc.desc: test ConvertTagToMask with ARKTS_STATIC_HEAP tag.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskArkTSStaticHeapConversion, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    config.add_restrace_tag("RES_ARKTS_STATIC_HEAP");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_ARKTS_STATIC_HEAP);
}

/*
 * @tc.name: ConvertTagToMaskDartHeapConversion
 * @tc.desc: test ConvertTagToMask with DART_HEAP tag.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskDartHeapConversion, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    config.add_restrace_tag("RES_DART_HEAP");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_DART_HEAP);
}

/*
 * @tc.name: ConvertTagToMaskRnHermesHeapConversion
 * @tc.desc: test ConvertTagToMask with RN_HERMES_HEAP tag.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, ConvertTagToMaskRnHermesHeapConversion, TestSize.Level0)
{
    HookManager hookManager;
    NativeHookConfig config;

    config.add_restrace_tag("RES_RN_HERMES_HEAP");
    hookManager.SetHookConfig(config);

    unsigned long long result = hookManager.ConvertTagToMask();
    EXPECT_EQ(result, RES_RN_HERMES_HEAP);
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
    config.add_filter_lib_list("libcustom_filter.so");
    std::string result = hookManager.GetCmdArgs(config);
    EXPECT_NE(result.find("clock: monotonic"), std::string::npos);
    EXPECT_NE(result.find("filter_napi_name: test"), std::string::npos);
    EXPECT_NE(result.find("target_so_name: test.so"), std::string::npos);
    EXPECT_NE(result.find("filter_lib_list: libcustom_filter.so"), std::string::npos);
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

namespace {
constexpr size_t STACK_DEPTH = 4;
constexpr size_t ALLOC_DATA_SIZE = sizeof(GwpAllocationData) + (STACK_DEPTH - 1) * sizeof(uintptr_t);
constexpr size_t DEFAULT_ALLOC_SIZE = 256;
constexpr uint64_t DEFAULT_ALIVE_TIME = 1000;

void BuildGwpAsanTestData(std::vector<uint8_t>& data, size_t count, size_t sampledRate)
{
    data.resize(sizeof(size_t) + sizeof(GwpHeader) + ALLOC_DATA_SIZE * count, 0);
    
    size_t* sizePtr = reinterpret_cast<size_t*>(data.data());
    *sizePtr = count;
    
    GwpHeader* header = reinterpret_cast<GwpHeader*>(data.data() + sizeof(size_t));
    header->max_slots = 10;
    header->sampled_rate = sampledRate;
    
    for (size_t i = 0; i < count; ++i) {
        GwpAllocationData* alloc = reinterpret_cast<GwpAllocationData*>(
            data.data() + sizeof(size_t) + sizeof(GwpHeader) + i * ALLOC_DATA_SIZE);
        alloc->addr = 0x1000 + i * 0x1000;
        alloc->size = DEFAULT_ALLOC_SIZE * (i + 1);
        alloc->alive_time = DEFAULT_ALIVE_TIME * (i + 1);
        alloc->stack[0] = 0x100 + i * 0x100;
        alloc->stack[1] = 0x200 + i * 0x100;
        alloc->stack[2] = 0;
    }
}

void BuildFrameMap(std::unordered_map<uintptr_t, DfxFrame>& frameMap, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        DfxFrame frame;
        frame.mapName = "test" + std::to_string(i) + ".so";
        frame.mapOffset = 0x100 + i * 0x100;
        frame.funcName = "test_func" + std::to_string(i);
        frame.funcOffset = 0x10 + i * 0x10;
        frame.buildId = "build" + std::to_string(i);
        frameMap[0x100 + i * 0x100] = frame;
    }
}

bool VerifyOutputContains(const std::string& output, const std::vector<std::string>& keywords)
{
    for (const auto& keyword : keywords) {
        if (output.find(keyword) == std::string::npos) {
            return false;
        }
    }
    return true;
}
}

/*
 * @tc.name: DumpGwpAsanInfoNullPointer
 * @tc.desc: test DumpGwpAsanInfo with null pointer.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, DumpGwpAsanInfoNullPointer, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    hookManager.DumpGwpAsanInfo(nullptr);
    EXPECT_EQ(hookManager.gwpAsanMaxStackDepth_, STACK_DEPTH);
}

/*
 * @tc.name: DumpGwpAsanInfoInvalidSampledRate
 * @tc.desc: test DumpGwpAsanInfo with invalid sampled_rate.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, DumpGwpAsanInfoInvalidSampledRate, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    
    std::vector<uint8_t> data;
    BuildGwpAsanTestData(data, 1, 0);
    
    hookManager.DumpGwpAsanInfo(data.data());
    EXPECT_EQ(hookManager.gwpAsanMaxStackDepth_, STACK_DEPTH);
}

/*
 * @tc.name: DumpGwpAsanInfoNormalCase
 * @tc.desc: test DumpGwpAsanInfo with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, DumpGwpAsanInfoNormalCase, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    hookManager.hookConfig_.set_pid(getpid());
    
    std::vector<uint8_t> data;
    BuildGwpAsanTestData(data, 1, 100);
    
    std::string testFile = "/data/local/tmp/test_gwp_asan.txt";
    int fd = open(testFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd > 0) {
        hookManager.SetGwpFd(fd);
        hookManager.DumpGwpAsanInfo(data.data());
        close(fd);
        
        std::ifstream infile(testFile);
        std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        infile.close();
        
        EXPECT_TRUE(VerifyOutputContains(content, {
            "GWP_ASAN_CONFIG", "SLOT:10", "SAMPLE:100",
            "GWP_ASAN_MEMORY_SAMPLE", "address:0x1000",
            "size:256 bytes", "alloc_time:1000 ms"
        }));
        
        unlink(testFile.c_str());
    }
}

/*
 * @tc.name: DumpGwpAsanInfoMultipleSamples
 * @tc.desc: test DumpGwpAsanInfo with multiple samples.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, DumpGwpAsanInfoMultipleSamples, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    hookManager.hookConfig_.set_pid(getpid());
    
    std::vector<uint8_t> data;
    BuildGwpAsanTestData(data, 3, 100);
    
    std::string testFile = "/data/local/tmp/test_gwp_asan_multi.txt";
    int fd = open(testFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd > 0) {
        hookManager.SetGwpFd(fd);
        hookManager.DumpGwpAsanInfo(data.data());
        close(fd);
        
        std::ifstream infile(testFile);
        std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        infile.close();
        
        EXPECT_TRUE(VerifyOutputContains(content, {
            "GWP_ASAN_CONFIG", "SLOT:10", "SAMPLE:100",
            "address:0x1000", "address:0x2000", "address:0x3000",
            "size:256 bytes", "size:512 bytes", "size:768 bytes"
        }));
        
        unlink(testFile.c_str());
    }
}

/*
 * @tc.name: CollectGwpAsanFramesEmptyData
 * @tc.desc: test CollectGwpAsanFrames with empty data.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, CollectGwpAsanFramesEmptyData, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    hookManager.hookConfig_.set_pid(getpid());
    
    std::unordered_map<uintptr_t, DfxFrame> frameMap;
    hookManager.CollectGwpAsanFrames(frameMap, nullptr, 0, 0);
    EXPECT_EQ(frameMap.size(), 0);
}

/*
 * @tc.name: CollectGwpAsanFramesNormalCase
 * @tc.desc: test CollectGwpAsanFrames with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, CollectGwpAsanFramesNormalCase, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    hookManager.hookConfig_.set_pid(getpid());
    
    std::vector<uint8_t> allocData(ALLOC_DATA_SIZE, 0);
    GwpAllocationData* alloc = reinterpret_cast<GwpAllocationData*>(allocData.data());
    alloc->stack[0] = 0x100;
    alloc->stack[1] = 0x200;
    alloc->stack[2] = 0x300;
    alloc->stack[3] = 0;
    
    std::unordered_map<uintptr_t, DfxFrame> frameMap;
    hookManager.CollectGwpAsanFrames(frameMap, allocData.data(), 1, ALLOC_DATA_SIZE);
    EXPECT_EQ(hookManager.gwpAsanMaxStackDepth_, STACK_DEPTH);
}

/*
 * @tc.name: CollectGwpAsanFramesDuplicatePcs
 * @tc.desc: test CollectGwpAsanFrames with duplicate PCs.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, CollectGwpAsanFramesDuplicatePcs, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    hookManager.hookConfig_.set_pid(getpid());
    
    std::vector<uint8_t> allocData(ALLOC_DATA_SIZE * 2, 0);
    for (size_t i = 0; i < 2; ++i) {
        GwpAllocationData* alloc = reinterpret_cast<GwpAllocationData*>(allocData.data() + i * ALLOC_DATA_SIZE);
        alloc->stack[0] = 0x100;
        alloc->stack[1] = 0x200;
        alloc->stack[2] = 0;
    }
    
    std::unordered_map<uintptr_t, DfxFrame> frameMap;
    hookManager.CollectGwpAsanFrames(frameMap, allocData.data(), 2, ALLOC_DATA_SIZE);
    EXPECT_EQ(hookManager.gwpAsanMaxStackDepth_, STACK_DEPTH);
}

/*
 * @tc.name: FormatGwpAsanOutputNormalCase
 * @tc.desc: test FormatGwpAsanOutput with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, FormatGwpAsanOutputNormalCase, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    
    GwpHeader header;
    header.max_slots = 10;
    header.sampled_rate = 100;
    
    std::vector<uint8_t> allocData(ALLOC_DATA_SIZE, 0);
    GwpAllocationData* alloc = reinterpret_cast<GwpAllocationData*>(allocData.data());
    alloc->addr = 0x1000;
    alloc->size = DEFAULT_ALLOC_SIZE;
    alloc->alive_time = DEFAULT_ALIVE_TIME;
    alloc->stack[0] = 0x100;
    alloc->stack[1] = 0;
    
    std::unordered_map<uintptr_t, DfxFrame> frameMap;
    BuildFrameMap(frameMap, 1);
    
    std::stringstream ss;
    hookManager.FormatGwpAsanOutput(ss, &header, allocData.data(), 1, ALLOC_DATA_SIZE, frameMap);
    
    std::string result = ss.str();
    EXPECT_TRUE(VerifyOutputContains(result, {
        "GWP_ASAN_CONFIG", "SLOT:10", "SAMPLE:100",
        "GWP_ASAN_MEMORY_SAMPLE", "address:0x1000",
        "size:256 bytes", "alloc_time:1000 ms",
        "test0.so", "test_func0"
    }));
}

/*
 * @tc.name: FormatGwpAsanOutputMultipleSamples
 * @tc.desc: test FormatGwpAsanOutput with multiple samples.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, FormatGwpAsanOutputMultipleSamples, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    
    GwpHeader header;
    header.max_slots = 10;
    header.sampled_rate = 100;
    
    std::vector<uint8_t> allocData(ALLOC_DATA_SIZE * 3, 0);
    for (size_t i = 0; i < 3; ++i) {
        GwpAllocationData* alloc = reinterpret_cast<GwpAllocationData*>(allocData.data() + i * ALLOC_DATA_SIZE);
        alloc->addr = 0x1000 + i * 0x1000;
        alloc->size = DEFAULT_ALLOC_SIZE * (i + 1);
        alloc->alive_time = DEFAULT_ALIVE_TIME * (i + 1);
        alloc->stack[0] = 0x100 + i * 0x100;
        alloc->stack[1] = 0;
    }
    
    std::unordered_map<uintptr_t, DfxFrame> frameMap;
    BuildFrameMap(frameMap, 3);
    
    std::stringstream ss;
    hookManager.FormatGwpAsanOutput(ss, &header, allocData.data(), 3, ALLOC_DATA_SIZE, frameMap);
    
    std::string result = ss.str();
    EXPECT_TRUE(VerifyOutputContains(result, {
        "GWP_ASAN_CONFIG", "SLOT:10", "SAMPLE:100",
        "address:0x1000", "address:0x2000", "address:0x3000",
        "size:256 bytes", "size:512 bytes", "size:768 bytes",
        "test0.so", "test1.so", "test2.so"
    }));
}

/*
 * @tc.name: FormatGwpAsanOutputEmptyFrameMap
 * @tc.desc: test FormatGwpAsanOutput with empty frame map.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, FormatGwpAsanOutputEmptyFrameMap, TestSize.Level0)
{
    HookManager hookManager;
    hookManager.gwpAsanMaxStackDepth_ = STACK_DEPTH;
    
    GwpHeader header;
    header.max_slots = 10;
    header.sampled_rate = 100;
    
    std::vector<uint8_t> allocData(ALLOC_DATA_SIZE, 0);
    GwpAllocationData* alloc = reinterpret_cast<GwpAllocationData*>(allocData.data());
    alloc->addr = 0x1000;
    alloc->size = DEFAULT_ALLOC_SIZE;
    alloc->alive_time = DEFAULT_ALIVE_TIME;
    alloc->stack[0] = 0x100;
    alloc->stack[1] = 0;
    
    std::unordered_map<uintptr_t, DfxFrame> frameMap;
    
    std::stringstream ss;
    hookManager.FormatGwpAsanOutput(ss, &header, allocData.data(), 1, ALLOC_DATA_SIZE, frameMap);
    
    std::string result = ss.str();
    EXPECT_TRUE(VerifyOutputContains(result, {
        "GWP_ASAN_CONFIG", "SLOT:10", "SAMPLE:100",
        "GWP_ASAN_MEMORY_SAMPLE", "address:0x1000",
        "size:256 bytes", "alloc_time:1000 ms"
    }));
}
} // namespace
