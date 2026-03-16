/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef NATIVE_MEMORY_PROFILER_SA_CLINET_MANAGER_TEST_H
#define NATIVE_MEMORY_PROFILER_SA_CLINET_MANAGER_TEST_H

#define private public

#include <gtest/gtest.h>
#include <unistd.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <chrono>
#include <thread>
#include "test_common.h"
#include "init_param.h"
#include "native_memory_profiler_sa_client_manager.h"
#include "utilities.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
using namespace OHOS::Security::AccessToken;
using namespace testing::ext;

namespace {
const std::string NATIVE_PARAM = "hiviewdfx.hiprofiler.native_memoryd.start";
const std::string TEST_PROC_NAME = "hiview";
const std::string TEST_NATIVE_NAME = "foundation";
const int NMD_ONLY_TYPE = 2;
static AccessTokenID g_selfTokenId;
static TEST_COMMON::MockNativeToken* g_mock = nullptr;
const std::string TEST_INVALID_FILE_PATH = "/invalid/path/test_count_file";
}

class NativeMemoryProfilerSaClientManagerTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        g_selfTokenId = GetSelfTokenID();
        TEST_COMMON::SetTestEvironment(g_selfTokenId);
        g_mock = new (std::nothrow) TEST_COMMON::MockNativeToken(TEST_PROC_NAME);
    }

    static void TearDownTestCase()
    {
        if (g_mock != nullptr) {
            delete g_mock;
            g_mock = nullptr;
        }
        SetSelfTokenID(g_selfTokenId);
        TEST_COMMON::ResetTestEvironment();
        SystemSetParameter(NATIVE_PARAM.c_str(), "0");
    }
    void SetUp() {}
    void TearDown() {}

    bool WriteFile(const std::string& filePath, const std::string& fileContent)
    {
        FILE* file = fopen(filePath.c_str(), "w");
        if (file == nullptr) {
            return false;
        }

        size_t len = fwrite(const_cast<char*>(fileContent.c_str()), 1, fileContent.length(), file);
        if (len != fileContent.length()) {
            (void)fclose(file);
            return false;
        }

        if (fflush(file) == EOF) {
            return false;
        }

        fsync(fileno(file));
        if (fclose(file) != 0) {
            return false;
        }
        // delay, avoid generating same update time.
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return true;
    }
};

#ifndef ENABLE_SA_AUTO_START
/**
 * @tc.name: NativeMemoryProfilerSaClientManagerTest001
 * @tc.desc: The test service is not started
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NativeMemoryProfilerSaClientManagerTest001, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    config->duration_ = 100;
    std::string filePath = "XXXXXXXXXXXX";
    config->filePath_ = filePath;
    config->fpUnwind_ = false;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Start(config), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Start(
        NativeMemoryProfilerSaClientManager::NativeMemProfilerType::MEM_PROFILER_CALL_STACK, 0, 0, 0), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop("test"), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(1), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::DumpData(0, config), RET_ERR);
}

/**
 * @tc.name: NativeMemoryProfilerSaClientManagerTest002
 * @tc.desc: Test the normal situation
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NativeMemoryProfilerSaClientManagerTest002, TestSize.Level3)
{
    EXPECT_GE(SystemSetParameter(NATIVE_PARAM.c_str(), "2"), 0);
    sleep(1);

    using namespace OHOS::Developtools::NativeDaemon;
    int32_t pid = GetProcessPid(TEST_NATIVE_NAME);
    EXPECT_NE(pid, 0);
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    config->pid_ = pid;
    config->duration_ = 10;
    config->filePath_ = "/data/local/tmp/native_hook_test.htrace";
    config->offlineSymbolization_ = true;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Start(config), RET_OK);
    sleep(2);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(TEST_NATIVE_NAME), RET_OK);
    EXPECT_GE(SystemSetParameter(NATIVE_PARAM.c_str(), "0"), 0);
    sleep(1);
    EXPECT_GE(SystemSetParameter(NATIVE_PARAM.c_str(), "2"), 0);
    sleep(1);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Start(
        NativeMemoryProfilerSaClientManager::NativeMemProfilerType::MEM_PROFILER_CALL_STACK,
        pid, 10, 0), RET_OK);
    sleep(2);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(pid), RET_OK);
    EXPECT_GE(SystemSetParameter(NATIVE_PARAM.c_str(), "0"), 0);
    sleep(1);
    EXPECT_GE(SystemSetParameter(NATIVE_PARAM.c_str(), "2"), 0);
    sleep(1);
    uint32_t fd = open("/data/local/tmp/test_dump_data.htrace",
        O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    EXPECT_GT(fd, 0);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::DumpData(fd, config), RET_OK);
    sleep(2);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(pid), RET_OK);
    EXPECT_GE(SystemSetParameter(NATIVE_PARAM.c_str(), "0"), 0);
    sleep(1);
}
#endif

/**
 * @tc.name: NativeMemoryProfilerSaClientManagerTest003
 * @tc.desc: Test nmd-only mode
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NativeMemoryProfilerSaClientManagerTest003, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(0, 0, NMD_ONLY_TYPE, true), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(0, 0, 1), RET_ERR);
}

/**
 * @tc.name: NativeMemoryProfilerSaClientManagerTest004
 * @tc.desc: Test simplified nmd mode
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NativeMemoryProfilerSaClientManagerTest004, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::vector<SimplifiedMemStats> memStats;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(0, memStats), RET_ERR);
}

/**
 * @tc.name: NativeMemoryProfilerSaClientManagerTest005
 * @tc.desc: Test get simplified stack file
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NativeMemoryProfilerSaClientManagerTest005, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    SimplifiedMemConfig config;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Start(0, 0, 100, config), RET_ERR);
}

/**
 * @tc.name: NativeMemoryProfilerSaClientManagerTest004
 * @tc.desc: Test simplified nmd mode
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NativeMemoryProfilerSaClientManagerTest006, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemSaConfig config;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Start(0, 0, 100, config), RET_ERR);
}

/**
 * @tc.name: NativeMemoryProfilerSaClientManagerTest007
 * @tc.desc: GetMallocStats invalid param - fd=0, pid<=0, invalid type (feature 0212: NMD GetMallocStats)
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NativeMemoryProfilerSaClientManagerTest007, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int validFd = 1;
    int validPid = 1234;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(0, validPid, 0), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(validFd, 0, 0), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(validFd, validPid, -1), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(validFd, validPid, 3), RET_ERR);
}

/**
 * @tc.name: CheckInputConfigTest001
 * @tc.desc: type = 0 and config = nullptr;
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, CheckInputConfigTest001, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int type = 0;
    MemAPIConfig* config = nullptr;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, config),
              ResProfilerErrorCode::RES_PROF_INVALID_ARG);
}

/**
 * @tc.name: CheckInputConfigTest002
 * @tc.desc: type = 0 and config is valid;
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, CheckInputConfigTest002, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int type = 0;
    MemAPIConfig normalConfig;
    normalConfig.maxDuration = 30;
    normalConfig.filterSize = 100;
    normalConfig.maxStackDepth = 30;
    normalConfig.statisticsInterval = 100;
    normalConfig.sampleInterval = 400;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &normalConfig),
              ResProfilerErrorCode::RES_PROF_SUCCESS);
}

/**
 * @tc.name: CheckInputConfigTest003
 * @tc.desc: corner cases: invalid type or invalid config
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, CheckInputConfigTest003, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int type = 0;
    MemAPIConfig invalidConfig;
    invalidConfig.maxDuration = 0;
    invalidConfig.filterSize = 100;
    invalidConfig.maxStackDepth = 30;
    invalidConfig.statisticsInterval = 100;
    invalidConfig.sampleInterval = 400;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_MAX_DURATION_INVALID_ARG);

    invalidConfig.maxDuration = 30;
    invalidConfig.filterSize = 0;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_FILTER_SIZE_INVALID_ARG);

    invalidConfig.filterSize = 100;
    invalidConfig.maxStackDepth = 32;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_MAX_STACK_DEPTH_INVALID_ARG);

    invalidConfig.maxStackDepth = 30;
    invalidConfig.statisticsInterval = 3601;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_STATISTICS_INTERVAL_INVALID_ARG);

    invalidConfig.statisticsInterval = 100;
    invalidConfig.sampleInterval = 380;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_SAMPLE_INTERVAL_INVALID_ARG);

    invalidConfig.sampleInterval = 400;
    type = 5;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_RESOURCE_TYPE_INVALID_ARG);
}

/**
 * @tc.name: ReadCountFileTest001
 * @tc.desc: file path is invalid
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ReadCountFileTest001, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    long long timestamp = 0;
    int32_t count = 0;
    bool result = NativeMemoryProfilerSaClientManager::ReadCountFile(TEST_INVALID_FILE_PATH, timestamp, count);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ReadCountFileTest002
 * @tc.desc: normal case
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ReadCountFileTest002, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::string testFilePath = "/data/local/tmp/test_count_file";
    std::string testTimeStamp = "1234567890";
    std::string testCount = "1";
    std::string testContent = testTimeStamp + " " + testCount + '\n';
    EXPECT_TRUE(WriteFile(testFilePath, testContent));

    long long timestamp = 0;
    int32_t count = 0;
    bool result = NativeMemoryProfilerSaClientManager::ReadCountFile(testFilePath, timestamp, count);
    EXPECT_TRUE(result);
    EXPECT_EQ(timestamp, 1234567890);
    EXPECT_EQ(count, 1);
}

/**
 * @tc.name: ReadCountFileTest003
 * @tc.desc: empty file
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ReadCountFileTest003, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::string testFilePath = "/data/local/tmp/test_count_file";
    std::string testContent = "";
    EXPECT_TRUE(WriteFile(testFilePath, testContent));

    long long timestamp = 0;
    int32_t count = 0;
    bool result = NativeMemoryProfilerSaClientManager::ReadCountFile(testFilePath, timestamp, count);
    EXPECT_TRUE(result);
    EXPECT_EQ(timestamp, 0);
    EXPECT_EQ(count, 0);
}

/**
 * @tc.name: WriteCountFileAtomicTest
 * @tc.desc: normal case
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, WriteCountFileAtomicTest, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    long long timestamp = 1234567890;
    int32_t count = 5;
    bool result = NativeMemoryProfilerSaClientManager::WriteCountFileAtomic(TEST_INVALID_FILE_PATH, timestamp, count);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetMemProfilerUseCountTodayTest
 * @tc.desc: bundle path not exists
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, GetMemProfilerUseCountTodayTest, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMemProfilerUseCountToday(), 0);
}

#endif // NATIVE_MEMORY_PROFILER_SA_CLINET_MANAGER_TEST_H