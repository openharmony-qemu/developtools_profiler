/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#include "hook_socket_client.h"
#include "parameters.h"
#include "socket_context.h"
#include "async_stack.h"
#include "command_poller.h"
#include "hook_manager.h"
#include "hook_service.h"
#include "native_memory_profiler_sa_config.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;
namespace {
const std::string TEST_PROCESS_NAME = "test_process";
const int TEST_PID = 12345;
const std::string TEST_NMD_FILE_PATH = "/data/local/tmp/nmd_test_process.txt";
const std::string TEST_MEM_LEAK_FILE_PATH = "/data/local/tmp/mem_leak_test.txt";

class HookManagerMemLeakTest : public ::testing::Test {
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
            std::cout << "Failed to remove file: " << TEST_NMD_FILE_PATH << std::endl;
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
 * @tc.name: SetMemLeakFd_Basic
 * @tc.desc: test SetMemLeakFd method with valid fd.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_Basic, TestSize.Level0)
{
    const uint32_t memLeakFd = 15;
    hookManager_->SetMemLeakFd(memLeakFd);
    
    EXPECT_EQ(hookManager_->memLeakFd_, memLeakFd);
}

/*
 * @tc.name: SetMemLeakFd_Zero
 * @tc.desc: test SetMemLeakFd with zero fd.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_Zero, TestSize.Level0)
{
    hookManager_->SetMemLeakFd(0);
    EXPECT_EQ(hookManager_->memLeakFd_, 0);
}

/*
 * @tc.name: SetMemLeakFd_UpdateValue
 * @tc.desc: test SetMemLeakFd updates value correctly.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_UpdateValue, TestSize.Level0)
{
    hookManager_->SetMemLeakFd(10);
    EXPECT_EQ(hookManager_->memLeakFd_, 10);
    
    hookManager_->SetMemLeakFd(20);
    EXPECT_EQ(hookManager_->memLeakFd_, 20);
}

/*
 * @tc.name: GetClientConfigExt_Basic
 * @tc.desc: test GetClientConfigExt method with memLeakFormat set.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_Basic, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);
    
    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);
    
    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_SNAPSHOT);
    EXPECT_TRUE(clientConfig.onlyMemLeak);
}

/*
 * @tc.name: GetClientConfigExt_DisableFormat
 * @tc.desc: test GetClientConfigExt with TYPE_DISABLE format.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_DisableFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_DISABLE));
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(false);
    
    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);
    
    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_DISABLE);
    EXPECT_FALSE(clientConfig.onlyMemLeak);
}

/*
 * @tc.name: GetClientConfigExt_TxtFormat
 * @tc.desc: test GetClientConfigExt with TYPE_TXT format.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_TxtFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_TXT));
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);
    
    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);
    
    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_TXT);
    EXPECT_TRUE(clientConfig.onlyMemLeak);
}

/*
 * @tc.name: GetClientConfigExt_VectorFormat
 * @tc.desc: test GetClientConfigExt with TYPE_VECTOR format.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_VectorFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_VECTOR));
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);
    
    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);
    
    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_VECTOR);
    EXPECT_TRUE(clientConfig.onlyMemLeak);
}

/*
 * @tc.name: SetHookConfigExt_Basic
 * @tc.desc: test SetHookConfigExt method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetHookConfigExt_Basic, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->processName_ = TEST_PROCESS_NAME;
    saConfig->largestSize_ = 1024;
    saConfig->secondLargestSize_ = 512;
    saConfig->maxGrowthSize_ = 2048;
    saConfig->gwpAsanHoldTime_ = 100;
    saConfig->gwpAsanMaxEntries_ = 50;
    saConfig->gwpAsanMaxStackDepth_ = 10;
    
    hookManager_->SetHookConfig(saConfig);
    
    EXPECT_EQ(hookManager_->largestSize_, 1024);
    EXPECT_EQ(hookManager_->secondLargestSize_, 512);
    EXPECT_EQ(hookManager_->maxGrowthSize_, 2048);
    EXPECT_EQ(hookManager_->gwpAsanHoldTime_, 100);
    EXPECT_EQ(hookManager_->gwpAsanMaxEntries_, 50);
    EXPECT_EQ(hookManager_->gwpAsanMaxStackDepth_, 10);
}

/*
 * @tc.name: SetHookConfigExt_ZeroValues
 * @tc.desc: test SetHookConfigExt with zero values.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetHookConfigExt_ZeroValues, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->largestSize_ = 0;
    saConfig->secondLargestSize_ = 0;
    saConfig->maxGrowthSize_ = 0;
    saConfig->gwpAsanHoldTime_ = 0;
    saConfig->gwpAsanMaxEntries_ = 0;
    saConfig->gwpAsanMaxStackDepth_ = 0;
    
    hookManager_->SetHookConfig(saConfig);
    
    EXPECT_EQ(hookManager_->largestSize_, 0);
    EXPECT_EQ(hookManager_->secondLargestSize_, 0);
    EXPECT_EQ(hookManager_->maxGrowthSize_, 0);
    EXPECT_EQ(hookManager_->gwpAsanHoldTime_, 0);
    EXPECT_EQ(hookManager_->gwpAsanMaxEntries_, 0);
    EXPECT_EQ(hookManager_->gwpAsanMaxStackDepth_, 0);
}

/*
 * @tc.name: GetMemLeakFileName_SnapshotFormat
 * @tc.desc: test GetMemLeakFileName with TYPE_SNAPSHOT format.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetMemLeakFileName_SnapshotFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    
    std::string fileName = hookManager_->GetMemLeakFileName(MemLeakFormat::TYPE_SNAPSHOT);
    
    EXPECT_NE(fileName.find(".heapsnapshot"), std::string::npos);
    EXPECT_NE(fileName.find("/data/local/tmp/"), std::string::npos);
    EXPECT_NE(fileName.find("mem_leak_"), std::string::npos);
}

/*
 * @tc.name: GetMemLeakFileName_TxtFormat
 * @tc.desc: test GetMemLeakFileName with TYPE_TXT format (default suffix).
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetMemLeakFileName_TxtFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    
    std::string fileName = hookManager_->GetMemLeakFileName(MemLeakFormat::TYPE_TXT);
    
    EXPECT_NE(fileName.find(".txt"), std::string::npos);
    EXPECT_NE(fileName.find("/data/local/tmp/"), std::string::npos);
    EXPECT_NE(fileName.find("mem_leak_"), std::string::npos);
}

/*
 * @tc.name: GetMemLeakFileName_VectorFormat
 * @tc.desc: test GetMemLeakFileName with TYPE_VECTOR format (default suffix).
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetMemLeakFileName_VectorFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    
    std::string fileName = hookManager_->GetMemLeakFileName(MemLeakFormat::TYPE_VECTOR);
    
    EXPECT_NE(fileName.find(".txt"), std::string::npos);
}

/*
 * @tc.name: GetMemLeakFileName_DisableFormat
 * @tc.desc: test GetMemLeakFileName with TYPE_DISABLE format (default suffix).
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetMemLeakFileName_DisableFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    
    std::string fileName = hookManager_->GetMemLeakFileName(MemLeakFormat::TYPE_DISABLE);
    
    EXPECT_NE(fileName.find(".txt"), std::string::npos);
}

/*
 * @tc.name: CreateMemLeakFile_DisableFormat
 * @tc.desc: test CreateMemLeakFile when mem_leak_format is TYPE_DISABLE.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, CreateMemLeakFile_DisableFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_DISABLE));
    hookManager_->SetHookConfig(config);
    
    hookManager_->CreateMemLeakFile();
    
    EXPECT_EQ(hookManager_->memLeakFd_, 0);
}

/*
 * @tc.name: CreateMemLeakFile_AlreadyHasFd
 * @tc.desc: test CreateMemLeakFile when memLeakFd_ is already set (SA mode).
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, CreateMemLeakFile_AlreadyHasFd, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFd(100);
    
    hookManager_->CreateMemLeakFile();
    
    EXPECT_EQ(hookManager_->memLeakFd_, 100);
}

/*
 * @tc.name: DumpMemLeakInfo_NullData
 * @tc.desc: test DumpMemLeakInfo with nullptr data.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, DumpMemLeakInfo_NullData, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    hookManager_->SetHookConfig(config);
    
    hookManager_->DumpMemLeakInfo(nullptr);
}

/*
 * @tc.name: DumpMemLeakInfo_ValidData
 * @tc.desc: test DumpMemLeakInfo with valid data when fd is 0.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, DumpMemLeakInfo_ValidData, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFd(0);
    
    uint8_t testData[] = "test memory leak data";
    hookManager_->DumpMemLeakInfo(testData);
}

/*
 * @tc.name: CloseResource_WithMemLeakFd
 * @tc.desc: test CloseResource closes memLeakFd correctly.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, CloseResource_WithMemLeakFd, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFd(50);
    
    hookManager_->CloseResource();
    
    EXPECT_EQ(hookManager_->memLeakFd_, 0);
}

/*
 * @tc.name: CloseResource_WithoutMemLeakFd
 * @tc.desc: test CloseResource when memLeakFd_ is 0.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, CloseResource_WithoutMemLeakFd, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFd(0);
    
    hookManager_->CloseResource();
    
    EXPECT_EQ(hookManager_->memLeakFd_, 0);
}

/*
 * @tc.name: DestroyPluginSession_ClosesMemLeakFd
 * @tc.desc: test DestroyPluginSession calls CloseResource.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, DestroyPluginSession_ClosesMemLeakFd, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFd(75);
    
    std::vector<uint32_t> pluginIds = {0};
    hookManager_->DestroyPluginSession(pluginIds);
    
    EXPECT_EQ(hookManager_->memLeakFd_, 0);
}

/*
 * @tc.name: MemLeakFormat_EnumValues
 * @tc.desc: verify MemLeakFormat enum values.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, MemLeakFormat_EnumValues, TestSize.Level0)
{
    EXPECT_EQ(static_cast<int>(MemLeakFormat::TYPE_DISABLE), 0);
    EXPECT_EQ(static_cast<int>(MemLeakFormat::TYPE_TXT), 1);
    EXPECT_EQ(static_cast<int>(MemLeakFormat::TYPE_VECTOR), 2);
    EXPECT_EQ(static_cast<int>(MemLeakFormat::TYPE_SNAPSHOT), 3);
}

/*
 * @tc.name: SetHookConfig_WithMemLeakFormat
 * @tc.desc: test SetHookConfig (from NativeMemoryProfilerSaConfig) sets mem_leak_format.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetHookConfig_WithMemLeakFormat, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->processName_ = TEST_PROCESS_NAME;
    saConfig->memLeakFormat_ = static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT);
    
    hookManager_->SetHookConfig(saConfig);
    
    const auto& config = hookManager_->GetHookConfig();
    EXPECT_EQ(config.mem_leak_format(), static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
}

/*
 * @tc.name: GetCmdArgs_IncludesMemLeakFormat
 * @tc.desc: test GetCmdArgs includes mem_leak_format parameter.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetCmdArgs_IncludesMemLeakFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_pid(TEST_PID);
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mem_leak_format(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    
    std::string result = hookManager_->GetCmdArgs(config);
    
    EXPECT_NE(result.find("mem_leak_format:"), std::string::npos);
}
} // namespace