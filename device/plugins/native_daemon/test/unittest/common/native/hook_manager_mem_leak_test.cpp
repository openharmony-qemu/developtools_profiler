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
#include <cstdio>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <functional>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
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
const std::string TEST_MEM_LEAK_FILE_PATH = "/data/local/tmp/mem_leak_test.txt";
const std::string FILE_CACHE_DIR = "/data/log/hiprofiler/";
const std::string MEM_LEAK_FILE_PATH = "/data/local/tmp/";

bool RunInChildProcess(const std::function<void()>& task)
{
    pid_t pid = fork();
    if (pid < 0) {
        return false;
    }
    if (pid == 0) {
        task();
        bool ok = !::testing::Test::HasFailure();
        fflush(nullptr);
        _exit(ok ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) != pid) {
        return false;
    }
    return WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS;
}

class HookManagerMemLeakTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
        int result = remove(TEST_MEM_LEAK_FILE_PATH.c_str());
        if (result != 0 && errno != ENOENT) {
            std::cout << "Failed to remove file: " << TEST_MEM_LEAK_FILE_PATH << std::endl;
        }
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
    }

    void SetUp() override
    {
        hookManager_ = std::make_shared<HookManager>();
        ASSERT_TRUE(hookManager_ != nullptr);
    }

    void TearDown() override
    {
        hookManager_->CloseResource();
        hookManager_ = nullptr;
    }

protected:
    std::shared_ptr<HookManager> hookManager_;
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
    int testFd = dup(STDIN_FILENO);
    ASSERT_GE(testFd, 0);
    hookManager_->SetMemLeakFd(testFd);

    // Verify through subsequent operations that fd was set
    NativeHookConfig config;
    config.set_native_orphansnapshots_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);

    EXPECT_EQ(hookManager_->memLeakFd_, testFd);
}

/*
 * @tc.name: SetMemLeakFd_ValidFd
 * @tc.desc: test SetMemLeakFd with a valid fd.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_ValidFd, TestSize.Level0)
{
    int testFd = dup(STDIN_FILENO);
    ASSERT_GE(testFd, 0);
    hookManager_->SetMemLeakFd(testFd);

    EXPECT_EQ(hookManager_->memLeakFd_, testFd);
}

/*
 * @tc.name: SetMemLeakFd_InvalidValue
 * @tc.desc: test SetMemLeakFd with invalid value.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetMemLeakFd_InvalidValue, TestSize.Level0)
{
    int testFd = -1;
    hookManager_->SetMemLeakFd(testFd);

    EXPECT_EQ(hookManager_->memLeakFd_, -1);
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
    EXPECT_EQ(hookManager_->memLeakFd_, 10);

    hookManager_->SetMemLeakFd(20);
    EXPECT_EQ(hookManager_->memLeakFd_, 20);

    hookManager_->SetMemLeakFd(-1);
    EXPECT_EQ(hookManager_->memLeakFd_, -1);
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

/*
 * @tc.name: HandleMemLeakDone_CleansTempFile
 * @tc.desc: test socket mem leak done handles fd content before cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, HandleMemLeakDone_CleansTempFile, TestSize.Level0)
{
    EXPECT_TRUE(RunInChildProcess([this]() {
        NativeHookConfig config;
        config.set_native_orphansnapshots_enable(true);
        config.set_pid(TEST_PID);
        hookManager_->SetHookConfig(config);
        hookManager_->SetMemLeakFormat(static_cast<uint32_t>(MemLeakFormat::TYPE_SNAPSHOT));
        const std::string tempDir = "/data/local/tmp/memleak_test_cache/";
        if (mkdir(tempDir.c_str(), 0755) != 0) {
            ASSERT_EQ(errno, EEXIST) << strerror(errno);
        }
        hookManager_->fileCacheDir_ = tempDir;
        hookManager_->memLeakFileName_ = "memleak-native-orphansnapshot-test.txt";

        int fd = hookManager_->CreateMemLeakFile();
        ASSERT_GE(fd, 0);
        EXPECT_FALSE(hookManager_->memLeakDumpDone_.load());
        int clientFd = dup(fd);
        ASSERT_GE(clientFd, 0);
        const char* content = "mem-leak-content";
        ssize_t writeSize = write(clientFd, content, strlen(content));
        ASSERT_EQ(writeSize, static_cast<ssize_t>(strlen(content)));
        ASSERT_EQ(close(clientFd), 0);

        EXPECT_TRUE(hookManager_->HandleMemLeakDone());
        EXPECT_TRUE(hookManager_->memLeakDumpDone_.load());
        EXPECT_EQ(hookManager_->tempMemLeakFd_, -1);
        std::ifstream result("/data/local/tmp/memleak-native-orphansnapshot-test.txt");
        std::stringstream buffer;
        buffer << result.rdbuf();
        EXPECT_EQ(buffer.str(), content);
        remove("/data/local/tmp/memleak-native-orphansnapshot-test.txt");
        rmdir(tempDir.c_str());
    }));
}

/*
 * @tc.name: HandleMemLeakDone_SaModeCopiesTempFdToClientFd
 * @tc.desc: test SA mem leak done copies temporary fd content into client fd.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, HandleMemLeakDone_SaModeCopiesTempFdToClientFd, TestSize.Level0)
{
    EXPECT_TRUE(RunInChildProcess([this]() {
        NativeHookConfig config;
        config.set_native_orphansnapshots_enable(true);
        hookManager_->SetHookConfig(config);
        hookManager_->SetSaMode(true);
        hookManager_->fileCacheDir_ = "/data/local/tmp/";
        hookManager_->memLeakFileName_ = "memleak-sa-copy.txt";
        int srcFd = hookManager_->CreateMemLeakFile();
        ASSERT_GE(srcFd, 0);

        const char* content = "sa-mem-leak-content";
        ASSERT_EQ(write(srcFd, content, strlen(content)), static_cast<ssize_t>(strlen(content)));
        char destPath[] = "/data/local/tmp/memleak_dest_XXXXXX";
        int destFd = mkstemp(destPath);
        ASSERT_GE(destFd, 0);
        ASSERT_EQ(unlink(destPath), 0);
        int verifyFd = dup(destFd);
        ASSERT_GE(verifyFd, 0);
        hookManager_->SetMemLeakFd(destFd);

        EXPECT_TRUE(hookManager_->HandleMemLeakDone());
        EXPECT_EQ(lseek(verifyFd, 0, SEEK_SET), 0);
        char buffer[64] = {0};
        EXPECT_EQ(read(verifyFd, buffer, sizeof(buffer)), static_cast<ssize_t>(strlen(content)));
        EXPECT_STREQ(buffer, content);
        EXPECT_EQ(close(verifyFd), 0);
    }));
}

/*
 * @tc.name: HandleMemLeakDone_SaModeHandlesUnseekableTempFd
 * @tc.desc: test SA mem leak done finishes even when temporary fd cannot be copied.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, HandleMemLeakDone_SaModeHandlesUnseekableTempFd, TestSize.Level0)
{
    EXPECT_TRUE(RunInChildProcess([this]() {
        int pipeFds[2] = {-1, -1};
        ASSERT_EQ(pipe(pipeFds), 0);
        char destPath[] = "/data/local/tmp/memleak_bad_dest_XXXXXX";
        int destFd = mkstemp(destPath);
        ASSERT_GE(destFd, 0);
        ASSERT_EQ(unlink(destPath), 0);
        NativeHookConfig config;
        config.set_native_orphansnapshots_enable(true);
        hookManager_->SetHookConfig(config);
        hookManager_->SetSaMode(true);
        hookManager_->tempMemLeakFd_ = pipeFds[0];
        hookManager_->SetMemLeakFd(destFd);
        hookManager_->memLeakDumpDone_ = false;

        EXPECT_TRUE(hookManager_->HandleMemLeakDone());
        EXPECT_TRUE(hookManager_->memLeakDumpDone_.load());
        EXPECT_EQ(hookManager_->tempMemLeakFd_, -1);
        EXPECT_EQ(close(pipeFds[1]), 0);
    }));
}

/*
 * @tc.name: CreateMemLeakFile_InvalidDirectory
 * @tc.desc: test CreateMemLeakFile fails when file cache directory is invalid.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, CreateMemLeakFile_InvalidDirectory, TestSize.Level0)
{
    hookManager_->fileCacheDir_ = "/data/local/tmp/not_exists_memleak_dir/";
    hookManager_->memLeakFileName_ = "memleak-invalid.txt";
    EXPECT_LT(hookManager_->CreateMemLeakFile(), 0);
}

/*
 * @tc.name: GetMemLeakFileName_StartupModeUsesZeroPid
 * @tc.desc: test mem leak filename uses pid 0 for startup mode tasks.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetMemLeakFileName_StartupModeUsesZeroPid, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_pid(TEST_PID);
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_startup_mode(true);
    hookManager_->SetHookConfig(config);

    EXPECT_EQ(hookManager_->GetMemLeakFileName(MemLeakFormat::TYPE_SNAPSHOT),
        "memleak-native-orphansnapshot-0.txt");
}

/*
 * @tc.name: HandleMemLeakDone_AlreadyFinished
 * @tc.desc: test HandleMemLeakDone returns true immediately after dump state is complete.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, HandleMemLeakDone_AlreadyFinished, TestSize.Level0)
{
    hookManager_->memLeakDumpDone_ = true;
    EXPECT_TRUE(hookManager_->HandleMemLeakDone());
}

/*
 * @tc.name: SetHookConfigExt_discardDestroyedTraces
 * @tc.desc: test SetHookConfigExt propagates discardDestroyedTraces_ to hookConfig_.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, SetHookConfigExt_discardDestroyedTraces, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->processName_ = TEST_PROCESS_NAME;
    saConfig->discardDestroyedTraces_ = true;
    hookManager_->SetHookConfig(saConfig);
    EXPECT_TRUE(hookManager_->hookConfig_.discard_destroyed_traces());

    saConfig->discardDestroyedTraces_ = false;
    hookManager_->SetHookConfig(saConfig);
    EXPECT_FALSE(hookManager_->hookConfig_.discard_destroyed_traces());
}

/*
 * @tc.name: GetClientConfig_discardDestroyedTraces
 * @tc.desc: test GetClientConfig propagates discard_destroyed_traces to clientConfig.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfig_discardDestroyedTraces, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->processName_ = TEST_PROCESS_NAME;
    saConfig->discardDestroyedTraces_ = true;
    hookManager_->SetHookConfig(saConfig);

    ClientConfig clientConfig;
    hookManager_->GetClientConfig(clientConfig);
    EXPECT_TRUE(clientConfig.discardDestroyedTraces);
}

/*
 * @tc.name: GetClientConfig_mmapSpecificAddrOnly
 * @tc.desc: test GetClientConfig propagates mmap_specific_addr_only to clientConfig.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, GetClientConfig_mmapSpecificAddrOnly, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_pid(TEST_PID);
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_mmap_specific_addr_only(true);
    hookManager_->SetHookConfig(config);

    ClientConfig clientConfig;
    hookManager_->GetClientConfig(clientConfig);
    EXPECT_TRUE(clientConfig.mmapSpecificAddrOnly);

    config.set_mmap_specific_addr_only(false);
    hookManager_->SetHookConfig(config);
    hookManager_->GetClientConfig(clientConfig);
    EXPECT_FALSE(clientConfig.mmapSpecificAddrOnly);
}

/*
 * @tc.name: RefreshValidLiveDataFromFile_DiscardMallocFreed
 * @tc.desc: test RefreshValidLiveDataFromFile builds validLiveData_ where malloc matched by free is removed.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, RefreshValidLiveDataFromFile_DiscardMallocFreed, TestSize.Level0)
{
    char path[] = "/data/local/tmp/test_malloc_dedup_XXXXXX";
    int fd = mkstemp(path);
    ASSERT_GE(fd, 0);
    unlink(path);

    BaseStackRawData mallocRec{};
    mallocRec.type = MALLOC_MSG;
    mallocRec.addr = reinterpret_cast<void*>(0x1234);
    mallocRec.ts = {1, 0};
    BaseStackRawData freeRec{};
    freeRec.type = FREE_MSG;
    freeRec.addr = reinterpret_cast<void*>(0x1234);
    freeRec.ts = {2, 0};

    uint32_t dataSize = 2 * (sizeof(uint32_t) + sizeof(BaseStackRawData));
    ASSERT_TRUE(write(fd, &dataSize, DATA_SIZE_HEADER_LEN) == static_cast<ssize_t>(DATA_SIZE_HEADER_LEN));
    uint32_t mallocSize = sizeof(BaseStackRawData);
    ASSERT_TRUE(write(fd, &mallocSize, sizeof(uint32_t)) == sizeof(uint32_t));
    ASSERT_TRUE(write(fd, &mallocRec, sizeof(BaseStackRawData)) == sizeof(BaseStackRawData));
    uint32_t freeSize = sizeof(BaseStackRawData);
    ASSERT_TRUE(write(fd, &freeSize, sizeof(uint32_t)) == sizeof(uint32_t));
    ASSERT_TRUE(write(fd, &freeRec, sizeof(BaseStackRawData)) == sizeof(BaseStackRawData));

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    hookManager_->shouldSkipGlobalFreed_ = true;
    hookManager_->validLiveData_.clear();
    hookManager_->RefreshValidLiveDataFromFile(ctx, fd);
    close(fd);

    EXPECT_EQ(hookManager_->validLiveData_.count(reinterpret_cast<void*>(0x1234)), 0u);
    EXPECT_TRUE(ShouldSkipRecord(&mallocRec, hookManager_->validLiveData_));
}

/*
 * @tc.name: RefreshValidLiveDataFromFile_CrashFileStopsAtZeroRegion
 * @tc.desc: test RefreshValidLiveDataFromFile scans crash file (size=0) until zero region.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, RefreshValidLiveDataFromFile_CrashFileStopsAtZeroRegion, TestSize.Level0)
{
    char path[] = "/data/local/tmp/test_malloc_crash_XXXXXX";
    int fd = mkstemp(path);
    ASSERT_GE(fd, 0);
    unlink(path);

    constexpr uintptr_t crashRecordAddr = 0x5678;
    BaseStackRawData mallocRec{};
    mallocRec.type = MALLOC_MSG;
    mallocRec.addr = reinterpret_cast<void*>(crashRecordAddr);
    mallocRec.ts = {1, 0};
    uint32_t mallocSize = sizeof(BaseStackRawData);
    uint32_t dataSize = 0;
    ASSERT_TRUE(write(fd, &dataSize, DATA_SIZE_HEADER_LEN) == static_cast<ssize_t>(DATA_SIZE_HEADER_LEN));
    ASSERT_TRUE(write(fd, &mallocSize, sizeof(uint32_t)) == sizeof(uint32_t));
    ASSERT_TRUE(write(fd, &mallocRec, sizeof(BaseStackRawData)) == sizeof(BaseStackRawData));
    constexpr size_t zeroTailSize = 16;
    uint8_t zeros[CRASH_ZERO_THRESHOLD + zeroTailSize] = {0};
    ASSERT_TRUE(write(fd, zeros, sizeof(zeros)) == sizeof(zeros));

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    hookManager_->shouldSkipGlobalFreed_ = true;
    hookManager_->validLiveData_.clear();
    hookManager_->RefreshValidLiveDataFromFile(ctx, fd);
    close(fd);

    EXPECT_EQ(hookManager_->validLiveData_.count(reinterpret_cast<void*>(crashRecordAddr)), 1u);
    EXPECT_FALSE(ShouldSkipRecord(&mallocRec, hookManager_->validLiveData_));
}

/*
 * @tc.name: RefreshValidLiveDataFromFile_CrashHeaderWithQueuedFdsDiscards
 * @tc.desc: test RefreshValidLiveDataFromFile skips zero-header fd when other fds are still queued.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerMemLeakTest, RefreshValidLiveDataFromFile_CrashHeaderWithQueuedFdsDiscards, TestSize.Level0)
{
    char path[] = "/data/local/tmp/test_malloc_crash_queue_XXXXXX";
    int fd = mkstemp(path);
    ASSERT_GE(fd, 0);
    unlink(path);

    constexpr uintptr_t crashRecordAddr = 0x5679;
    BaseStackRawData mallocRec{};
    mallocRec.type = MALLOC_MSG;
    mallocRec.addr = reinterpret_cast<void*>(crashRecordAddr);
    mallocRec.ts = {1, 0};
    uint32_t mallocSize = sizeof(BaseStackRawData);
    uint32_t dataSize = 0;
    ASSERT_TRUE(write(fd, &dataSize, DATA_SIZE_HEADER_LEN) == static_cast<ssize_t>(DATA_SIZE_HEADER_LEN));
    ASSERT_TRUE(write(fd, &mallocSize, sizeof(uint32_t)) == sizeof(uint32_t));
    ASSERT_TRUE(write(fd, &mallocRec, sizeof(BaseStackRawData)) == sizeof(BaseStackRawData));

    constexpr int dummyQueuedFd = -2; // negative fd only inflates queue size, never used for I/O
    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    ctx->PushBackFdQueue(fd);
    ctx->PushBackFdQueue(dummyQueuedFd);
    hookManager_->shouldSkipGlobalFreed_ = true;
    hookManager_->validLiveData_.clear();
    hookManager_->RefreshValidLiveDataFromFile(ctx, fd);
    close(fd);

    EXPECT_EQ(hookManager_->validLiveData_.count(reinterpret_cast<void*>(crashRecordAddr)), 0u);
}
} // namespace
