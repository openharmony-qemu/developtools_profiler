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
#include <fstream>
#include <sstream>
#include "command_poller.h"
#include "hook_manager.h"
#include "hook_service.h"
#include "native_memory_profiler_sa_config.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;
namespace {
const std::string TEST_PROCESS_NAME = "test_process";
const int TEST_PID = 12345;
const int TEST_SMB_PAGES = 1024;
const int TEST_FILTER_SIZE = 512;
const int TEST_MAX_STACK_DEPTH = 20;
const int TEST_MAX_JS_STACK_DEPTH = 10;
const std::string TEST_NMD_FILE_PATH = "/data/local/tmp/nmd_test_process.txt";

class HookManagerExtTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
        sleep(1); 
    }
    
    static void TearDownTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
        int result = remove(TEST_NMD_FILE_PATH.c_str());
        if (result != 0) {
            std::cerr "Failed to remove file: " << TEST_NMD_FILE_PATH << std::endl;
        }
    }
    
    void SetUp() override
    {
        hookManager_ = std::make_shared<HookManager>();
        ASSERT_TRUE(hookManager_ != nullptr);
        
        commandPoller_ = std::make_shared<CommandPoller>(hookManager_);
        ASSERT_TRUE(commandPoller_ != nullptr);
        ASSERT_TRUE(commandPoller_->OnConnect());
        
        hookManager_->SetCommandPoller(commandPoller_);
    }
    
    void TearDown() override
    {
        hookManager_ = nullptr;
        commandPoller_ = nullptr;
    }
    
protected:
    std::shared_ptr<HookManager> hookManager_;
    std::shared_ptr<CommandPoller> commandPoller_;
};

/*
 * @tc.name: UpdateAsyncFlagConfigDisabled
 * @tc.desc: test UpdateAsyncFlagConfig when async_stack_enable is false.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateAsyncFlagConfigDisabled, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(false);
    hookManager_->SetHookConfig(config);
    
    hookManager_->UpdateAsyncFlagConfig();
    
    // asyncFlagMask should remain 0 when async_stack_enable is false
    EXPECT_EQ(hookManager_->asyncFlagMask_, 0ULL);
}

/*
 * @tc.name: UpdateAsyncFlagConfigWithTypes
 * @tc.desc: test UpdateAsyncFlagConfig with async types.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateAsyncFlagConfigWithTypes, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    config.add_async_type(AsyncFlagType::LIBUV_SEND);
    config.add_async_type(AsyncFlagType::FFRT_POOL);
    hookManager_->SetHookConfig(config);
    
    hookManager_->UpdateAsyncFlagConfig();
    
    // asyncFlagMask should include the specified types
    EXPECT_TRUE(hookManager_->asyncFlagMask_ & ASYNC_TYPE_LIBUV_SEND);
    EXPECT_TRUE(hookManager_->asyncFlagMask_ & ASYNC_TYPE_FFRT_POOL);
}

/*
 * @tc.name: UpdateAsyncFlagConfigAllTypes
 * @tc.desc: test UpdateAsyncFlagConfig with ALL_ASYNC_TYPE.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateAsyncFlagConfigAllTypes, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    config.add_async_type(AsyncFlagType::ALL_ASYNC_TYPE);
    hookManager_->SetHookConfig(config);
    
    hookManager_->UpdateAsyncFlagConfig();
    
    // asyncFlagMask should include all async types
    EXPECT_TRUE(hookManager_->asyncFlagMask_ != 0ULL);
}

/*
 * @tc.name: GetClientConfigBasic
 * @tc.desc: test GetClientConfig with basic configuration.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetClientConfigBasic, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_smb_pages(TEST_SMB_PAGES);
    config.set_filter_size(TEST_FILTER_SIZE);
    config.set_max_stack_depth(TEST_MAX_STACK_DEPTH);
    config.set_max_js_stack_depth(TEST_MAX_JS_STACK_DEPTH);
    config.set_malloc_disable(false);
    config.set_mmap_disable(false);
    config.set_free_stack_report(true);
    config.set_munmap_stack_report(true);
    config.set_fp_unwind(true);
    config.set_blocked(false);
    config.set_memtrace_enable(true);
    config.set_async_stack_enable(true);
    config.set_offline_symbolization(false);
    config.set_response_library_mode(false);
    config.set_js_stack_report(1);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    
    hookManager_->UpdateAsyncFlagConfig();
    
    ClientConfig clientConfig;
    hookManager_->GetClientConfig(clientConfig);
    
    EXPECT_EQ(clientConfig.filterSize, TEST_FILTER_SIZE);
    EXPECT_EQ(clientConfig.maxStackDepth, TEST_MAX_STACK_DEPTH);
    EXPECT_EQ(clientConfig.arktsConfig.maxJsStackDepth, TEST_MAX_JS_STACK_DEPTH);
    EXPECT_FALSE(clientConfig.mallocDisable);
    EXPECT_FALSE(clientConfig.mmapDisable);
    EXPECT_TRUE(clientConfig.freeStackData);
    EXPECT_TRUE(clientConfig.munmapStackData);
    EXPECT_TRUE(clientConfig.fpunwind);
    EXPECT_TRUE(clientConfig.arktsConfig.jsFpunwind);
    EXPECT_FALSE(clientConfig.isBlocked);
    EXPECT_TRUE(clientConfig.memtraceEnable);
    EXPECT_TRUE(clientConfig.asyncStackEnable);
    EXPECT_FALSE(clientConfig.offlineSymbolization);
    EXPECT_FALSE(clientConfig.responseLibraryMode);
    EXPECT_TRUE(clientConfig.arktsConfig.jsStackReport);
}

/*
 * @tc.name: SetHookConfigFromSaConfig
 * @tc.desc: test SetHookConfig with NativeMemoryProfilerSaConfig.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetHookConfigFromSaConfig, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->processName_ = TEST_PROCESS_NAME;
    saConfig->filterSize_ = TEST_FILTER_SIZE;
    saConfig->shareMemorySize_ = TEST_SMB_PAGES;
    saConfig->maxStackDepth_ = TEST_MAX_STACK_DEPTH;
    saConfig->mallocDisable_ = false;
    saConfig->mmapDisable_ = false;
    saConfig->freeStackData_ = true;
    saConfig->munmapStackData_ = true;
    saConfig->mallocFreeMatchingInterval_ = 1000;
    saConfig->mallocFreeMatchingCnt_ = 5;
    saConfig->stringCompressed_ = true;
    saConfig->fpUnwind_ = true;
    saConfig->blocked_ = false;
    saConfig->recordAccurately_ = true;
    saConfig->startupMode_ = false;
    saConfig->memtraceEnable_ = true;
    saConfig->asyncStackEnable_ = true;
    saConfig->traceMask_ = 0x12345678;
    saConfig->offlineSymbolization_ = false;
    saConfig->callframeCompress_ = true;
    saConfig->statisticsInterval_ = 30;
    saConfig->clockId_ = CLOCK_MONOTONIC;
    saConfig->sampleInterval_ = 100;
    saConfig->responseLibraryMode_ = false;
    saConfig->jsStackReport_ = 1;
    saConfig->maxJsStackDepth_ = TEST_MAX_JS_STACK_DEPTH;
    saConfig->filterNapiName_ = "test_napi";
    saConfig->targetSoName_ = "libtest.so";
    saConfig->printNmd_ = true;
    saConfig->hookstandalone_ = false;
    saConfig->saveFile_ = false;
    saConfig->fileName_ = "test.htrace";
    saConfig->largestSize_ = 1024;
    saConfig->secondLargestSize_ = 512;
    saConfig->maxGrowthSize_ = 2048;
    
    hookManager_->SetHookConfig(saConfig);
    
    const auto& config = hookManager_->GetHookConfig();
    EXPECT_EQ(config.pid(), TEST_PID);
    EXPECT_EQ(config.process_name(), TEST_PROCESS_NAME);
    EXPECT_EQ(config.filter_size(), TEST_FILTER_SIZE);
    EXPECT_EQ(config.smb_pages(), TEST_SMB_PAGES);
    EXPECT_EQ(config.max_stack_depth(), TEST_MAX_STACK_DEPTH);
    EXPECT_FALSE(config.malloc_disable());
    EXPECT_FALSE(config.mmap_disable());
    EXPECT_TRUE(config.free_stack_report());
    EXPECT_TRUE(config.munmap_stack_report());
    EXPECT_EQ(config.malloc_free_matching_interval(), 1000);
    EXPECT_EQ(config.malloc_free_matching_cnt(), 5);
    EXPECT_TRUE(config.string_compressed());
    EXPECT_TRUE(config.fp_unwind());
    EXPECT_FALSE(config.blocked());
    EXPECT_TRUE(config.record_accurately());
    EXPECT_FALSE(config.startup_mode());
    EXPECT_TRUE(config.memtrace_enable());
    EXPECT_TRUE(config.async_stack_enable());
    EXPECT_FALSE(config.offline_symbolization());
    EXPECT_TRUE(config.callframe_compress());
    EXPECT_EQ(config.statistics_interval(), 30);
    EXPECT_EQ(config.sample_interval(), 100);
    EXPECT_FALSE(config.response_library_mode());
    EXPECT_EQ(config.js_stack_report(), 1);
    EXPECT_EQ(config.max_js_stack_depth(), TEST_MAX_JS_STACK_DEPTH);
    EXPECT_EQ(config.filter_napi_name(), "test_napi");
    EXPECT_EQ(config.target_so_name(), "libtest.so");
}

/*
 * @tc.name: CreateWriterAndReset
 * @tc.desc: test CreateWriter and ResetWriter methods.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CreateWriterAndReset, TestSize.Level0)
{
    const std::string pluginName = "test_plugin";
    const uint32_t bufferSize = 1024 * 1024;
    const int smbFd = 1;
    const int eventFd = 2;
    
    EXPECT_TRUE(hookManager_->CreateWriter(pluginName, bufferSize, smbFd, eventFd, true));
    EXPECT_TRUE(hookManager_->ResetWriter(0));
}

/*
 * @tc.name: RegisterFdForSa
 * @tc.desc: test RegisterFdForSa method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, RegisterFdForSa, TestSize.Level0)
{
    const uint32_t saFd = 10;
    hookManager_->RegisterFdForSa(saFd);
    
    // Check if saFd_ was set correctly
    EXPECT_EQ(hookManager_->saFd_, saFd);
}

/*
 * @tc.name: SetSaMode
 * @tc.desc: test SetSaMode method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetSaMode, TestSize.Level0)
{
    hookManager_->SetSaMode(true);
    EXPECT_TRUE(hookManager_->saMode_);
    
    hookManager_->SetSaMode(false);
    EXPECT_FALSE(hookManager_->saMode_);
}

/*
 * @tc.name: SetHookStandalone
 * @tc.desc: test SethookStandalone method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetHookStandalone, TestSize.Level0)
{
    hookManager_->SethookStandalone(true);
    EXPECT_TRUE(hookManager_->isHookStandalone_);
    
    hookManager_->SethookStandalone(false);
    EXPECT_FALSE(hookManager_->isHookStandalone_);
}

/*
 * @tc.name: SetNmdInfo
 * @tc.desc: test SetNmdInfo method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetNmdInfo, TestSize.Level0)
{
    const uint32_t fd = 5;
    const uint32_t type = 1;
    hookManager_->SetNmdInfo(std::make_pair(fd, type));
    
    // Check if nmdParamInfo_ was set correctly
    EXPECT_EQ(hookManager_->nmdParamInfo_.fd, fd);
    EXPECT_EQ(hookManager_->nmdParamInfo_.type, type);
}

/*
 * @tc.name: DumpNmdInfo
 * @tc.desc: test DumpNmdInfo method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, DumpNmdInfo, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    
    const char* nmdResult = "jemalloc statistics:\nAllocated: 1024 bytes\n";
    hookManager_->DumpNmdInfo(nmdResult);
    
    // Check if the file was created and contains the expected content
    std::ifstream file(TEST_NMD_FILE_PATH);
    EXPECT_TRUE(file.is_open());
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    
    EXPECT_NE(content.find("jemalloc statistics:"), std::string::npos);
    EXPECT_NE(content.find("Allocated: 1024 bytes"), std::string::npos);
    
    // Clean up
    remove(TEST_NMD_FILE_PATH.c_str());
}

/*
 * @tc.name: CheckProcessWithPid
 * @tc.desc: test CheckProcess with valid pid.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CheckProcessWithPid, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_pid(TEST_PID);
    config.add_expand_pids(TEST_PID);
    hookManager_->SetHookConfig(config);
    
    // This should return true even if the process doesn't exist
    // because CheckProcess only checks if the pid is > 0 and adds it to hookCtx_
    EXPECT_FALSE(hookManager_->CheckProcess());
}

/*
 * @tc.name: CheckProcessWithProcessName
 * @tc.desc: test CheckProcess with process name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CheckProcessWithProcessName, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_startup_mode(true);
    hookManager_->SetHookConfig(config);
    
    // This should return true even if the process doesn't exist
    // because startup_mode is true
    EXPECT_TRUE(hookManager_->CheckProcess());
    
    // Reset startup param
    hookManager_->ResetStartupParam();
}

/*
 * @tc.name: MergeAsyncFlagMaskFromEnum
 * @tc.desc: test MergeAsyncFlagMaskFromEnum method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, MergeAsyncFlagMaskFromEnum, TestSize.Level0)
{
    uint64_t baseMask = 0;
    
    NativeHookConfig config;
    config.add_async_type(AsyncFlagType::LIBUV_SEND);
    config.add_async_type(AsyncFlagType::PROMISE);
    hookManager_->SetHookConfig(config);
    
    uint64_t result = hookManager_->MergeAsyncFlagMaskFromEnum(baseMask);
    EXPECT_TRUE(result & ASYNC_TYPE_LIBUV_SEND);
    EXPECT_TRUE(result & ASYNC_TYPE_PROMISE);
}

/*
 * @tc.name: CreatePluginSessionEmptyConfig
 * @tc.desc: test CreatePluginSession with empty config.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CreatePluginSessionEmptyConfig, TestSize.Level0)
{
    std::vector<ProfilerPluginConfig> configVec;
    
    // This should return false because no config is provided
    EXPECT_FALSE(hookManager_->CreatePluginSession(configVec));
}

/*
 * @tc.name: StartPluginSessionEmptyContext
 * @tc.desc: test StartPluginSession with empty hook context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, StartPluginSessionEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {0};
    std::vector<ProfilerPluginConfig> configVec;
    PluginResult result;
    
    // This should return false because hookCtx_ is empty
    EXPECT_FALSE(hookManager_->StartPluginSession(pluginIds, configVec, result));
}

/*
 * @tc.name: StopPluginSessionEmptyContext
 * @tc.desc: test StopPluginSession with empty hook context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, StopPluginSessionEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {0};
    
    // This should return false because hookCtx_ is empty
    EXPECT_FALSE(hookManager_->StopPluginSession(pluginIds));
}

/*
 * @tc.name: DestroyPluginSessionEmptyContext
 * @tc.desc: test DestroyPluginSession with empty hook context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, DestroyPluginSessionEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {0};
    
    // This should return true even with empty hook context
    EXPECT_TRUE(hookManager_->DestroyPluginSession(pluginIds));
}

/*
 * @tc.name: ReportPluginBasicData
 * @tc.desc: test ReportPluginBasicData method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, ReportPluginBasicData, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {0};
    
    // This should return true
    EXPECT_TRUE(hookManager_->ReportPluginBasicData(pluginIds));
}

/*
 * @tc.name: ResetStartupParam
 * @tc.desc: test ResetStartupParam method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, ResetStartupParam, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_startup_mode(true);
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    
    // This should not crash
    hookManager_->ResetStartupParam();
}

/*
 * @tc.name: GetCmdArgsWithExpandPids
 * @tc.desc: test GetCmdArgs with expand_pids.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetCmdArgsWithExpandPids, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_pid(TEST_PID);
    config.set_process_name(TEST_PROCESS_NAME);
    config.add_expand_pids(TEST_PID);
    config.add_expand_pids(TEST_PID + 1);
    config.add_expand_pids(TEST_PID + 2);
    
    std::string result = hookManager_->GetCmdArgs(config);
    
    EXPECT_NE(result, "");
    // process does not exist.
    EXPECT_EQ(result.find("pid: " + std::to_string(TEST_PID)), std::string::npos);
    EXPECT_NE(result.find("process_name: " + TEST_PROCESS_NAME), std::string::npos);
    EXPECT_NE(result.find("expand_pids: " + std::to_string(TEST_PID)), std::string::npos);
    EXPECT_NE(result.find("expand_pids: " + std::to_string(TEST_PID + 1)), std::string::npos);
    EXPECT_NE(result.find("expand_pids: " + std::to_string(TEST_PID + 2)), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsWithMallocFreeMatching
 * @tc.desc: test GetCmdArgs with malloc_free_matching parameters.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetCmdArgsWithMallocFreeMatching, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_malloc_free_matching_interval(500);
    config.set_malloc_free_matching_cnt(10);
    
    std::string result = hookManager_->GetCmdArgs(config);
    
    EXPECT_NE(result, "");
    EXPECT_NE(result.find("malloc_free_matching_interval: 500"), std::string::npos);
    EXPECT_NE(result.find("malloc_free_matching_cnt: 10"), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsWithClockAndSample
 * @tc.desc: test GetCmdArgs with clock and sample parameters.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetCmdArgsWithClockAndSample, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_clock("realtime");
    config.set_sample_interval(50);
    
    std::string result = hookManager_->GetCmdArgs(config);
    
    EXPECT_NE(result, "");
    EXPECT_NE(result.find("clock: realtime"), std::string::npos);
    EXPECT_NE(result.find("sample_interval: 50"), std::string::npos);
}
} // namespace
