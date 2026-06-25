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
#include "hook_common.h"
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

void ExpectRemotePathRet(int32_t ret)
{
    EXPECT_TRUE(ret == RET_OK || ret == RET_ERR);
}
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

/**
 * @tc.name: StartMemProfilerInvalidArgTest
 * @tc.desc: StartMemProfiler should reject invalid duration or sample interval
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemProfilerInvalidArgTest, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    SimplifiedMemConfig config;
    config.sampleSize = 384;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartMemProfiler(0, config, nullptr),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG));

    config.sampleSize = 100;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartMemProfiler(10, config, nullptr),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG));
}

/**
 * @tc.name: StartProfilerByTypeInvalidArgTest
 * @tc.desc: StartProfilerByType should reject nullptr and invalid resource type
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartProfilerByTypeInvalidArgTest, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartProfilerByType(0, nullptr, nullptr),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG));

    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 10;
    inputConfig.filterSize = 8;
    inputConfig.maxStackDepth = 16;
    inputConfig.statisticsInterval = 120;
    inputConfig.sampleInterval = 512;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartProfilerByType(5, &inputConfig, nullptr),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_RESOURCE_TYPE_INVALID_ARG));
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartProfilerByTypeValidInputFailsWithoutRemote, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 10;
    inputConfig.filterSize = 8;
    inputConfig.maxStackDepth = 16;
    inputConfig.statisticsInterval = 120;
    inputConfig.sampleInterval = 512;

    int32_t ret = NativeMemoryProfilerSaClientManager::StartProfilerByType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_FD), &inputConfig, nullptr);
    EXPECT_NE(ret, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG));
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemProfilerValidInputFailsWithoutRemote, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    SimplifiedMemConfig config;
    config.sampleSize = 512;

    int32_t ret = NativeMemoryProfilerSaClientManager::StartMemProfiler(10, config, nullptr);
    EXPECT_NE(ret, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_INVALID_ARG));
}

/**
 * @tc.name: ParseSimplifiedConfigTargetSoNameTest
 * @tc.desc: ParseSimplifiedConfig should copy target so names from simplified config.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseSimplifiedConfigTargetSoNameTest, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);

    SimplifiedMemConfig inputConfig;
    inputConfig.sampleSize = 512;
    inputConfig.asyncStackEnable = true;
    inputConfig.asyncFlag = 7;
    inputConfig.targetSoName = "liba.so, libb.so";

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseSimplifiedConfig(config, inputConfig, 10), RET_OK);
    EXPECT_TRUE(config->asyncStackEnable_);
    EXPECT_EQ(config->asyncFlag_, inputConfig.asyncFlag);
    EXPECT_EQ(config->targetSoName_, inputConfig.targetSoName);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseSimplifiedConfigUsesResponseLibraryMode, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);

    SimplifiedMemConfig inputConfig;
    inputConfig.sampleSize = 512;
    inputConfig.responseLibraryMode = true;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseSimplifiedConfig(config, inputConfig, 10), RET_OK);
    EXPECT_TRUE(config->responseLibraryMode_);
    EXPECT_EQ(config->shareMemorySize_, 4096u);

    inputConfig.responseLibraryMode = false;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseSimplifiedConfig(config, inputConfig, 10), RET_OK);
    EXPECT_FALSE(config->responseLibraryMode_);
    EXPECT_EQ(config->shareMemorySize_, 16384u);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, CheckConfigHandlesDefaultsAndInvalidFields, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> nullConfig = nullptr;
    EXPECT_FALSE(NativeMemoryProfilerSaClientManager::CheckConfig(nullConfig));

    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    config->duration_ = 0;
    EXPECT_FALSE(NativeMemoryProfilerSaClientManager::CheckConfig(config));

    config->duration_ = 10;
    config->filePath_ = std::string(PATH_MAX, 'a');
    EXPECT_FALSE(NativeMemoryProfilerSaClientManager::CheckConfig(config));

    config->filePath_.clear();
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::CheckConfig(config));
    EXPECT_EQ(config->shareMemorySize_, 16384u);

    config->shareMemorySize_ = 0;
    config->responseLibraryMode_ = true;
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::CheckConfig(config));
    EXPECT_EQ(config->shareMemorySize_, 4096u);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseResTypeSetsExpectedMasks, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> nullConfig = nullptr;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseResType(0, nullConfig), RET_ERR);

    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseResType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_NATIVE), config), RET_OK);
    EXPECT_FALSE(config->mallocDisable_);
    EXPECT_FALSE(config->memtraceEnable_);

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseResType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_GPU), config), RET_OK);
    EXPECT_TRUE(config->mallocDisable_);
    EXPECT_EQ(config->traceMask_, RES_GPU_MASK);
    EXPECT_EQ(config->gpuRange_.size(), 5u);
    for (const auto& range : config->gpuRange_) {
        EXPECT_EQ(range.size(), 4u);
        EXPECT_EQ(range[0], 0u);
        EXPECT_EQ(range[1], 0xFFFFFFFFFFFFFFFFULL);
        EXPECT_EQ(range[2], 0u);
        EXPECT_EQ(range[3], 0xFFFFFFFFFFFFFFFFULL);
    }

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseResType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_FD), config), RET_OK);
    EXPECT_EQ(config->traceMask_, RES_FD_MASK);

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseResType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_THREAD), config), RET_OK);
    EXPECT_EQ(config->traceMask_, RES_THREAD_MASK);

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseResType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_GLOBAL_HANDLE), config), RET_OK);
    EXPECT_EQ(config->traceMask_, RES_ARK_GLOBAL_HANDLE);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseNormalConfigHandlesNullAndCopiesFields, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    MemAPIConfig inputConfig = {12, 34, 5, 67, 890};
    ASSERT_NE(config, nullptr);

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(nullptr, &inputConfig), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(config, nullptr), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(config, &inputConfig), RET_OK);
    EXPECT_EQ(config->duration_, inputConfig.maxDuration);
    EXPECT_EQ(config->filterSize_, static_cast<int32_t>(inputConfig.filterSize));
    EXPECT_EQ(config->maxStackDepth_, inputConfig.maxStackDepth);
    EXPECT_EQ(config->statisticsInterval_, inputConfig.statisticsInterval);
    EXPECT_EQ(config->sampleInterval_, inputConfig.sampleInterval);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, UtilityMethodsHandleBoundaries, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    EXPECT_FALSE(NativeMemoryProfilerSaClientManager::IsSeparateByMoreThan24Hours(-1, 0));
    EXPECT_FALSE(NativeMemoryProfilerSaClientManager::IsSeparateByMoreThan24Hours(100, 100 + 24 * 60 * 60));
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::IsSeparateByMoreThan24Hours(100 + 25 * 60 * 60, 100));

    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::GenerateFileName(-1, getpid()).empty());
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::GenerateFileName(5, getpid()).empty());
    std::string fileName = NativeMemoryProfilerSaClientManager::GenerateFileName(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_NATIVE), getpid());
    EXPECT_EQ(fileName.find("native-"), 0u);
    EXPECT_NE(fileName.find("-" + std::to_string(getpid()) + "-"), std::string::npos);
    EXPECT_NE(fileName.rfind(".htrace"), std::string::npos);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StopWithPathAndCallbackCleanup, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::string filePath;
    EXPECT_NE(NativeMemoryProfilerSaClientManager::Stop(filePath), RET_OK);
    EXPECT_TRUE(filePath.empty());
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, GetGwpAsanRejectsInvalidArgs, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    GwpAsanConfig config;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetGwpAsan(0, getpid(), config), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetGwpAsan(STDOUT_FILENO, 0, config), RET_ERR);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartOverloadsValidInputFailWithoutRemote, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    SimplifiedMemConfig simplifiedConfig;
    simplifiedConfig.sampleSize = 512;
    MemLeakConfig memLeakConfig;
    memLeakConfig.duration = 10;
    memLeakConfig.limit = 3;
    GwpAsanConfig gwpConfig;
    gwpConfig.holdTime = 1;

    int32_t memLeakRet = NativeMemoryProfilerSaClientManager::Start(STDOUT_FILENO, getpid(), memLeakConfig);
    ExpectRemotePathRet(memLeakRet);
    if (memLeakRet == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }
    int32_t simplifiedRet = NativeMemoryProfilerSaClientManager::Start(STDOUT_FILENO, getpid(), 10, simplifiedConfig);
    ExpectRemotePathRet(simplifiedRet);
    if (simplifiedRet == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }
    int32_t gwpRet = NativeMemoryProfilerSaClientManager::GetGwpAsan(STDOUT_FILENO, getpid(), gwpConfig);
    ExpectRemotePathRet(gwpRet);
    if (gwpRet == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemSaConfigCopiesGpuRangesBeforeRemoteFailure, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemSaConfig config;
    std::pair<uint64_t, uint64_t> lowRange = std::make_pair(1, 2);
    std::pair<uint64_t, uint64_t> highRange = std::make_pair(3, 4);
    config.mask = RES_GPU_MASK;
    config.asyncStackEnable = true;
    config.asyncFlag = 5;
    config.hookSizes[TAG_RES_GPU_VK] = std::make_pair(lowRange, highRange);
    config.hookSizes[TAG_RES_GPU_GLES_IMAGE] = std::make_pair(lowRange, highRange);
    config.hookSizes[TAG_RES_GPU_GLES_BUFFER] = std::make_pair(lowRange, highRange);
    config.hookSizes[TAG_RES_GPU_CL_IMAGE] = std::make_pair(lowRange, highRange);
    config.hookSizes[TAG_RES_GPU_CL_BUFFER] = std::make_pair(lowRange, highRange);

    int32_t ret = NativeMemoryProfilerSaClientManager::Start(STDOUT_FILENO, getpid(), 10, config);
    ExpectRemotePathRet(ret);
    if (ret == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseSimplifiedConfigCopiesSizeThresholds, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    SimplifiedMemConfig inputConfig;
    inputConfig.largestSize = 11;
    inputConfig.secondLargestSize = 22;
    inputConfig.maxGrowthSize = 33;
    inputConfig.sampleSize = 512;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseSimplifiedConfig(config, inputConfig, 44), RET_OK);
    EXPECT_EQ(config->duration_, 44u);
    EXPECT_EQ(config->largestSize_, 11u);
    EXPECT_EQ(config->secondLargestSize_, 22u);
    EXPECT_EQ(config->maxGrowthSize_, 33u);
    EXPECT_TRUE(config->jsStackReport_);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NmdValidInputsReachRemoteFailure, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::vector<SimplifiedMemStats> memStats;

    int32_t statsRet = NativeMemoryProfilerSaClientManager::GetMallocStats(STDOUT_FILENO, getpid(), 0);
    ExpectRemotePathRet(statsRet);
    int32_t nmdRet = NativeMemoryProfilerSaClientManager::GetMallocStats(STDOUT_FILENO, getpid(), NMD_ONLY_TYPE, true);
    ExpectRemotePathRet(nmdRet);
    if (nmdRet == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }
    int32_t simplifiedRet = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(getpid(), memStats);
    ExpectRemotePathRet(simplifiedRet);
}

HWTEST_F(NativeMemoryProfilerSaClientManagerTest, CountFileUpdateAndRollback, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    const std::string dir = "/data/storage/el2/base/files";
    const std::string filePath = dir + "/MemProfilerCount";
    ASSERT_EQ(system(("mkdir -p " + dir).c_str()), 0);
    unlink(filePath.c_str());

    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::UpdateMemProfilerUserCount());
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMemProfilerUseCountToday(), 1);
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::UpdateMemProfilerUserCount());
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMemProfilerUseCountToday(), 2);
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::RollbackMemProfilerUserCount());
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMemProfilerUseCountToday(), 1);

    unlink(filePath.c_str());
}

#endif // NATIVE_MEMORY_PROFILER_SA_CLINET_MANAGER_TEST_H
