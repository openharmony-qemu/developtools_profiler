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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
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
const std::string FILE_CACHE_DIR = "/data/log/hiprofiler/";
const std::string MEM_LEAK_FILE_PATH = "/data/local/tmp/";

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
        hookManager_->CloseResource();
        hookManager_ = nullptr;
        commandPoller_ = nullptr;
    }

protected:
    std::shared_ptr<HookManager> hookManager_;
    std::shared_ptr<CommandPoller> commandPoller_;
};

/*
 * @tc.name: SetMemLeakFormat_Basic
 * @tc.desc: test SetMemLeakFormat method with TYPE_SNAPSHOT.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFormat_Basic, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    hookManager_->SetSaMode(true);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_SNAPSHOT);
}

/*
 * @tc.name: SetMemLeakFormat_TxtFormat
 * @tc.desc: test SetMemLeakFormat with TYPE_TXT format.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFormat_TxtFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_TXT));
    hookManager_->SetSaMode(true);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_TXT);
}

/*
 * @tc.name: SetMemLeakFormat_VectorFormat
 * @tc.desc: test SetMemLeakFormat with TYPE_VECTOR format.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFormat_VectorFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_VECTOR));
    hookManager_->SetSaMode(true);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_VECTOR);
}

/*
 * @tc.name: SetMemLeakFormat_DisableFormat
 * @tc.desc: test SetMemLeakFormat with TYPE_DISABLE format.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFormat_DisableFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_DISABLE));
    hookManager_->SetSaMode(true);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_DISABLE);
}

/*
 * @tc.name: GetClientConfigExt_SaModeFalse
 * @tc.desc: test GetClientConfigExt with saMode=false uses native_orphansnapshots_enable.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_SaModeFalse, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(false);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_SNAPSHOT);
}

/*
 * @tc.name: GetClientConfigExt_SaModeFalseDisable
 * @tc.desc: test GetClientConfigExt with saMode=false and native_orphansnapshots_enable=false.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_SaModeFalseDisable, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_native_orphansnapshots_enable(false);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(false);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_DISABLE);
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
 * @tc.name: CreateMemLeakFile_Disabled
 * @tc.desc: test CreateMemLeakFile when native_orphansnapshots_enable is false.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, CreateMemLeakFile_Disabled, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_native_orphansnapshots_enable(false);
    hookManager_->SetHookConfig(config);
    hookManager_->SetPid(TEST_PID);

    // memLeakFileName_ is not set when disabled, CreateMemLeakFile should return -1
    int fd = hookManager_->CreateMemLeakFile();
    EXPECT_LT(fd, 0);
}

/*
 * @tc.name: CloseResource_NoMemLeakFd
 * @tc.desc: test CloseResource when no mem leak fd is set.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, CloseResource_NoMemLeakFd, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);

    hookManager_->CloseResource();
    // Should not crash or throw
}

/*
 * @tc.name: MemLeakFormat_EnumValues
 * @tc.desc: verify MemLeakFormat enum values.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, MemLeakFormat_EnumValues, TestSize.Level0)
{
    EXPECT_EQ(static_cast<uint32_t>(MemLeakFormat::TYPE_TXT), 0u);
    EXPECT_EQ(static_cast<uint32_t>(MemLeakFormat::TYPE_VECTOR), 1u);
    EXPECT_EQ(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT), 2u);
    EXPECT_EQ(static_cast<uint32_t>(MemLeakFormat::TYPE_DISABLE), 0xFFFFFFFFu);
}

/*
 * @tc.name: SetMemLeakFd_Basic
 * @tc.desc: test SetMemLeakFd method sets memLeakFd_ correctly.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_Basic, TestSize.Level0)
{
    uint32_t testFd = 100;
    hookManager_->SetMemLeakFd(testFd);

    // Verify through subsequent operations that fd was set
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);

    // The memLeakFd_ should be set to testFd
    EXPECT_EQ(hookManager_->memLeakFd_, testFd);
}

/*
 * @tc.name: SetMemLeakFd_ZeroValue
 * @tc.desc: test SetMemLeakFd with zero value.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_ZeroValue, TestSize.Level0)
{
    uint32_t testFd = 0;
    hookManager_->SetMemLeakFd(testFd);

    EXPECT_EQ(hookManager_->memLeakFd_, 0u);
}

/*
 * @tc.name: SetMemLeakFd_InvalidValue
 * @tc.desc: test SetMemLeakFd with invalid value (max uint32_t).
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_InvalidValue, TestSize.Level0)
{
    uint32_t testFd = 0xFFFFFFFF;
    hookManager_->SetMemLeakFd(testFd);

    EXPECT_EQ(hookManager_->memLeakFd_, 0xFFFFFFFFu);
}

/*
 * @tc.name: GetClientConfigExt_SaModeTrueWithMemLeakFormat
 * @tc.desc: test GetClientConfigExt with saMode=true and specific memLeakFormat.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_SaModeTrueWithMemLeakFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    config.set_pid(TEST_PID);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_TXT));
    hookManager_->SetSaMode(true);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    // In SA mode, memLeakFormat is set via SetMemLeakFormat
    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_TXT);
}

/*
 * @tc.name: SetSaMode_True
 * @tc.desc: test SetSaMode with true value.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetSaMode_True, TestSize.Level0)
{
    hookManager_->SetSaMode(true);
    EXPECT_EQ(hookManager_->saMode_, true);
}

/*
 * @tc.name: SetSaMode_False
 * @tc.desc: test SetSaMode with false value.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetSaMode_False, TestSize.Level0)
{
    hookManager_->SetSaMode(false);
    EXPECT_EQ(hookManager_->saMode_, false);
}

/*
 * @tc.name: SetPid_Basic
 * @tc.desc: test SetPid method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetPid_Basic, TestSize.Level0)
{
    hookManager_->SetPid(TEST_PID);
    EXPECT_EQ(hookManager_->GetPid(), TEST_PID);
}

/*
 * @tc.name: SetPid_ZeroValue
 * @tc.desc: test SetPid with zero value.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetPid_ZeroValue, TestSize.Level0)
{
    hookManager_->SetPid(0);
    EXPECT_EQ(hookManager_->GetPid(), 0);
}

/*
 * @tc.name: SetPid_NegativeValue
 * @tc.desc: test SetPid with negative value.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetPid_NegativeValue, TestSize.Level0)
{
    hookManager_->SetPid(-1);
    EXPECT_EQ(hookManager_->GetPid(), -1);
}

/*
 * @tc.name: GetHookConfig_Basic
 * @tc.desc: test GetHookConfig returns configured values.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetHookConfig_Basic, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_pid(TEST_PID);
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);

    NativeHookConfig retrievedConfig = hookManager_->GetHookConfig();
    EXPECT_EQ(retrievedConfig.process_name(), TEST_PROCESS_NAME);
    EXPECT_EQ(retrievedConfig.pid(), TEST_PID);
    EXPECT_TRUE(retrievedConfig.native_orphansnapshots_enable());
}

/*
 * @tc.name: SetHookConfig_NativeOrphanSnapshots
 * @tc.desc: test SetHookConfig with native_orphansnapshots_enable flag.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetHookConfig_NativeOrphanSnapshots, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_pid(TEST_PID);
    hookManager_->SetHookConfig(config);

    // Verify the config was set
    NativeHookConfig retrievedConfig = hookManager_->GetHookConfig();
    EXPECT_TRUE(retrievedConfig.native_orphansnapshots_enable());
}

/*
 * @tc.name: SetHookConfigExt_WithMemLeakFormat
 * @tc.desc: test SetHookConfigExt with memLeakFormat setting.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetHookConfigExt_WithMemLeakFormat, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->processName_ = TEST_PROCESS_NAME;
    saConfig->nativeOrphanSnapshotsEnable_ = true;
    saConfig->memLeakFormat_ = static_cast<uint32_t>(MemLeakFormat::TYPE_TXT);

    hookManager_->SetHookConfig(saConfig);
    hookManager_->SetSaMode(true);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_TXT);
}

/*
 * @tc.name: GetClientConfigExt_MemLeakFormatSnapshot
 * @tc.desc: test GetClientConfigExt with TYPE_SNAPSHOT format in SA mode.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_MemLeakFormatSnapshot, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    hookManager_->SetSaMode(true);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_SNAPSHOT);
}

/*
 * @tc.name: GetClientConfigExt_MemLeakFormatVector
 * @tc.desc: test GetClientConfigExt with TYPE_VECTOR format in SA mode.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_MemLeakFormatVector, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_VECTOR));
    hookManager_->SetSaMode(true);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_VECTOR);
}

/*
 * @tc.name: CloseResource_WithMemLeakFd
 * @tc.desc: test CloseResource cleans up memLeakFd_ properly.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, CloseResource_WithMemLeakFd, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_native_orphansnapshots_enable(true);
    config.set_pid(TEST_PID);
    hookManager_->SetHookConfig(config);
    hookManager_->SetMemLeakFd(100);

    hookManager_->CloseResource();
    // After CloseResource, internal state should be cleaned
}

/*
 * @tc.name: SetHookConfigExt_MemLeakLimit
 * @tc.desc: test SetHookConfigExt with memLeakLimit setting.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetHookConfigExt_MemLeakLimit, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->processName_ = TEST_PROCESS_NAME;
    saConfig->memLeakLimit_ = 1024 * 1024; // 1MB limit

    hookManager_->SetHookConfig(saConfig);

    EXPECT_EQ(hookManager_->memLeakLimit_, 1024 * 1024);
}

/*
 * @tc.name: GetClientConfigExt_WithoutOrphanSnapshots
 * @tc.desc: test GetClientConfigExt when native_orphansnapshots_enable is false in SA mode.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_WithoutOrphanSnapshots, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(false);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);
    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_DISABLE));

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_DISABLE);
}

/*
 * @tc.name: SetMemLeakFormat_MultipleCalls
 * @tc.desc: test SetMemLeakFormat called multiple times.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFormat_MultipleCalls, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);

    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_TXT));
    ClientConfig clientConfig1;
    hookManager_->GetClientConfigExt(clientConfig1);
    EXPECT_EQ(clientConfig1.memLeakFormat, MemLeakFormat::TYPE_TXT);

    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_VECTOR));
    ClientConfig clientConfig2;
    hookManager_->GetClientConfigExt(clientConfig2);
    EXPECT_EQ(clientConfig2.memLeakFormat, MemLeakFormat::TYPE_VECTOR);

    hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
    ClientConfig clientConfig3;
    hookManager_->GetClientConfigExt(clientConfig3);
    EXPECT_EQ(clientConfig3.memLeakFormat, MemLeakFormat::TYPE_SNAPSHOT);
}

/*
 * @tc.name: SetMemLeakFd_MultipleCalls
 * @tc.desc: test SetMemLeakFd called multiple times.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_MultipleCalls, TestSize.Level0)
{
    hookManager_->SetMemLeakFd(10);
    EXPECT_EQ(hookManager_->memLeakFd_, 10u);

    hookManager_->SetMemLeakFd(20);
    EXPECT_EQ(hookManager_->memLeakFd_, 20u);

    hookManager_->SetMemLeakFd(0);
    EXPECT_EQ(hookManager_->memLeakFd_, 0u);
}

/*
 * @tc.name: GetClientConfigExt_DefaultFormat
 * @tc.desc: test GetClientConfigExt returns TYPE_DISABLE by default.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfigExt_DefaultFormat, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(false);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(false);

    ClientConfig clientConfig;
    hookManager_->GetClientConfigExt(clientConfig);

    // Default should be TYPE_DISABLE when not enabled
    EXPECT_EQ(clientConfig.memLeakFormat, MemLeakFormat::TYPE_DISABLE);
}
} // namespace