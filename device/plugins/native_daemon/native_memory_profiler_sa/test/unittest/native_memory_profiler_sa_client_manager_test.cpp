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
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <chrono>
#include <ctime>
#include <thread>
#include "test_common.h"
#include "init_param.h"
#include "native_memory_profiler_sa_client_manager.h"
#include "utilities.h"
#include "hook_common.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "i_semaphore.h"
#include "common.h"
using namespace OHOS::Security::AccessToken;
using namespace testing::ext;

namespace {
const std::string NATIVE_PARAM = "hiviewdfx.hiprofiler.native_memoryd.start";
const std::string TEST_PROC_NAME = "hiview";
const std::string TEST_NATIVE_NAME = "foundation";
const std::string TEST_APP_NAME = "com.ohos.sceneboard";
const int NMD_ONLY_TYPE = 2;
static AccessTokenID g_selfTokenId;
static TEST_COMMON::MockNativeToken* g_mock = nullptr;
const std::string TEST_INVALID_FILE_PATH = "/invalid/path/test_count_file";
const std::string TEST_HTRACE_FD_FILE_PATH = "/data/test/sa_test_data.htrace";
const std::string TEST_COMPOSITE_HEAP_FD_FILE_PATH = "/data/test/sa_test_data.zip";

void ExpectRemotePathRet(int32_t ret)
{
    EXPECT_TRUE(ret == RET_OK || ret == RET_ERR);
}

void MallocTest()
{
    int size = 10;
    void* arr[size];
    for (int i = 0; i < size; i++) {
        GTEST_LOG_(INFO) << "======i=" << i << "======";
        useconds_t sleepTime = 500000; // 500ms
        usleep(sleepTime);
        arr[i] = malloc(sizeof(int));
    }
    for (int i = 0; i < size; i++) {
        free(arr[i]);
    }
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
 * @tc.name: NativeMemoryProfilerSaClientManagerTest006
 * @tc.desc: Native memory profiler SA client manager test 006.
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

    invalidConfig.maxStackDepth = 0;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_MAX_STACK_DEPTH_INVALID_ARG);

    invalidConfig.maxStackDepth = 30;
    invalidConfig.statisticsInterval = 3601;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_STATISTICS_INTERVAL_INVALID_ARG);

    invalidConfig.statisticsInterval = 0;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_STATISTICS_INTERVAL_INVALID_ARG);

    invalidConfig.statisticsInterval = 100;
    invalidConfig.sampleInterval = 380;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_SAMPLE_INTERVAL_INVALID_ARG);

    invalidConfig.sampleInterval = 400;
    type = static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_MAX);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(type, &invalidConfig),
              ResProfilerErrorCode::RES_PROF_RESOURCE_TYPE_INVALID_ARG);
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
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartProfilerByType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_MAX), &inputConfig, nullptr),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_RESOURCE_TYPE_INVALID_ARG));
}

/**
 * @tc.name: StartProfilerByTypeValidInputFailsWithoutRemote
 * @tc.desc: Start profiler by type valid input fails without remote.
 * @tc.type: FUNC
 */
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

/**
 * @tc.name: StartMemProfilerValidInputFailsWithoutRemote
 * @tc.desc: Start Mem profiler valid input fails without remote.
 * @tc.type: FUNC
 */
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

/**
 * @tc.name: ParseSimplifiedConfigUsesResponseLibraryMode
 * @tc.desc: Parse simplified config uses response library mode.
 * @tc.type: FUNC
 */
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

/**
 * @tc.name: CheckConfigHandlesDefaultsAndInvalidFields
 * @tc.desc: Check config handles defaults and invalid fields.
 * @tc.type: FUNC
 */
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

/**
 * @tc.name: ParseResTypeSetsExpectedMasks
 * @tc.desc: Parse res type sets expected masks.
 * @tc.type: FUNC
 */
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

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseResType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_DMA), config), RET_OK);
    EXPECT_EQ(config->traceMask_, RES_FD_DMABUF | RES_DMABUF_MASK);

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseResType(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_ASHMEM), config), RET_OK);
    EXPECT_EQ(config->traceMask_, RES_FD_ASHMEM | RES_ASHMEM_MASK);
}

/**
 * @tc.name: ParseNormalConfigHandlesNullAndCopiesFields
 * @tc.desc: Parse normal config handles null and copies fields.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseNormalConfigHandlesNullAndCopiesFields, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    MemAPIConfig inputConfig = {12, 34, 5, 67, 890, {}};
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

/**
 * @tc.name: GenerateFileNameHandleBoundaries
 * @tc.desc: Generate file name handle boundaries.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, GenerateFileNameHandleBoundaries, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::GenerateFileName(-1, getpid()).empty());
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::GenerateFileName(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_MAX), getpid()).empty());
    std::string fileName = NativeMemoryProfilerSaClientManager::GenerateFileName(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_NATIVE), getpid());
    EXPECT_EQ(fileName.find("native-"), 0u);
    EXPECT_NE(fileName.find("-" + std::to_string(getpid()) + "-"), std::string::npos);
    EXPECT_NE(fileName.rfind(".htrace"), std::string::npos);
}

/**
 * @tc.name: GenerateFileNamePrefixHandleBoundaries
 * @tc.desc: Generate file name prefix handle boundaries.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, GenerateFileNamePrefixHandleBoundaries, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::GenerateFileNamePrefix(-1, getpid()).empty());
    EXPECT_TRUE(NativeMemoryProfilerSaClientManager::GenerateFileNamePrefix(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_MAX), getpid()).empty());

    std::string procName = COMMON::GetProcessNameByPid(getpid());
    EXPECT_FALSE(procName.empty());

    std::string prefix = NativeMemoryProfilerSaClientManager::GenerateFileNamePrefix(
        static_cast<int32_t>(InternalResourceType::OH_RES_TYPE_NATIVE), getpid());
    EXPECT_EQ(prefix, std::string("native-") + procName);
}

/**
 * @tc.name: GenerateFileNamePrefixAllValidTypes
 * @tc.desc: Generate file name prefix maps each valid resource type to its type tag.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, GenerateFileNamePrefixAllValidTypes, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::string procName = COMMON::GetProcessNameByPid(getpid());
    ASSERT_FALSE(procName.empty());

    const std::pair<InternalResourceType, std::string> typeTags[] = {
        {InternalResourceType::OH_RES_TYPE_FD, "fd"},
        {InternalResourceType::OH_RES_TYPE_THREAD, "thread"},
        {InternalResourceType::OH_RES_TYPE_NATIVE, "native"},
        {InternalResourceType::OH_RES_TYPE_GPU, "gpu"},
        {InternalResourceType::OH_RES_TYPE_GLOBAL_HANDLE, "global_handle"},
        {InternalResourceType::OH_RES_TYPE_DMA, "dma"},
        {InternalResourceType::OH_RES_TYPE_ASHMEM, "ashmem"},
        {InternalResourceType::OH_RES_TYPE_COMPOSITE_HEAP, "composite_heap"},
    };
    for (const auto& tag : typeTags) {
        std::string prefix = NativeMemoryProfilerSaClientManager::GenerateFileNamePrefix(
            static_cast<int32_t>(tag.first), getpid());
        EXPECT_EQ(prefix, tag.second + "-" + procName);
    }
}

/**
 * @tc.name: GetGwpAsanRejectsInvalidArgs
 * @tc.desc: Get GWPAsan asan rejects invalid args.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, GetGwpAsanRejectsInvalidArgs, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    GwpAsanConfig config;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetGwpAsan(0, getpid(), config), RET_ERR);
    int fd1 = open("/data/test/sa_test_gwp.htrace", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd1, 0);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetGwpAsan(fd1, 0, config), RET_ERR);
    close(fd1);
}

/**
 * @tc.name: StartOverloadsValidInputFailWithoutRemote
 * @tc.desc: Start overloads valid input fail without remote.
 * @tc.type: FUNC
 */
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

    int fd = open("/data/test/sa_test_output.htrace", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    ExpectRemotePathRet(NativeMemoryProfilerSaClientManager::Start(fd, getpid(), memLeakConfig));
    ExpectRemotePathRet(NativeMemoryProfilerSaClientManager::Start(fd, getpid(), 10, simplifiedConfig));
    ExpectRemotePathRet(NativeMemoryProfilerSaClientManager::GetGwpAsan(fd, getpid(), gwpConfig));
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StopWithService(nullptr, getpid()), RET_ERR);
    close(fd);
}

/**
 * @tc.name: StartMemSaConfigCopiesGpuRangesBeforeRemoteFailure
 * @tc.desc: Start Mem SA config copies GPU ranges before remote failure.
 * @tc.type: FUNC
 */
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

    int fd2 = open("/data/test/sa_test_gpu.htrace", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd2, 0);
    int32_t ret = NativeMemoryProfilerSaClientManager::Start(fd2, getpid(), 10, config);
    ExpectRemotePathRet(ret);
    if (ret == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }
    close(fd2);
}

/**
 * @tc.name: ParseSimplifiedConfigCopiesSizeThresholds
 * @tc.desc: Parse simplified config copies size thresholds.
 * @tc.type: FUNC
 */
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

/**
 * @tc.name: NmdValidInputsReachRemoteFailure
 * @tc.desc: NMD valid inputs reach remote failure.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NmdValidInputsReachRemoteFailure, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::vector<SimplifiedMemStats> memStats;

    int fd3 = open("/data/test/sa_test_nmd.htrace", O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd3, 0);
    int32_t statsRet = NativeMemoryProfilerSaClientManager::GetMallocStats(fd3, getpid(), 0);
    ExpectRemotePathRet(statsRet);
    int32_t nmdRet = NativeMemoryProfilerSaClientManager::GetMallocStats(fd3, getpid(), NMD_ONLY_TYPE, true);
    ExpectRemotePathRet(nmdRet);
    if (nmdRet == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }
    int32_t simplifiedRet = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(getpid(), memStats);
    ExpectRemotePathRet(simplifiedRet);
    close(fd3);
}

/**
 * @tc.name: CheckInputConfigRejectsUpperAndLowerBounds
 * @tc.desc: Check input config rejects upper and lower bounds.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, CheckInputConfigRejectsUpperAndLowerBounds, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 3601;
    inputConfig.filterSize = 8;
    inputConfig.maxStackDepth = 16;
    inputConfig.statisticsInterval = 120;
    inputConfig.sampleInterval = 512;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(0, &inputConfig),
        ResProfilerErrorCode::RES_PROF_MAX_DURATION_INVALID_ARG);

    inputConfig.maxDuration = 10;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::CheckInputConfig(-1, &inputConfig),
        ResProfilerErrorCode::RES_PROF_RESOURCE_TYPE_INVALID_ARG);
}

/**
 * @tc.name: StartProfilerTypeOverloadsReachConfigBranches
 * @tc.desc: Start profiler type overloads reach config branches.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartProfilerTypeOverloadsReachConfigBranches, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto libraryType = NativeMemoryProfilerSaClientManager::NativeMemProfilerType::MEM_PROFILER_LIBRARY;
    auto stackType = NativeMemoryProfilerSaClientManager::NativeMemProfilerType::MEM_PROFILER_CALL_STACK;

    int32_t libraryRet = NativeMemoryProfilerSaClientManager::Start(libraryType, getpid(), 10, 512);
    ExpectRemotePathRet(libraryRet);
    if (libraryRet == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }

    int32_t stackRet = NativeMemoryProfilerSaClientManager::Start(stackType, getpid(), 10, 512);
    ExpectRemotePathRet(stackRet);
    if (stackRet == RET_OK) {
        EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(getpid()), RET_OK);
    }
}

#endif // NATIVE_MEMORY_PROFILER_SA_CLINET_MANAGER_TEST_H

/**
 * @tc.name: ParseNormalConfigSetsAsyncFlagWhenNestedEnabled
 * @tc.desc: Parse normal config sets async flag when nested enabled.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseNormalConfigSetsAsyncFlagWhenNestedEnabled, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);

    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 11;
    inputConfig.filterSize = 22;
    inputConfig.maxStackDepth = 3;
    inputConfig.statisticsInterval = 44;
    inputConfig.sampleInterval = 512;
    inputConfig.asyncStackEnable = true;
    inputConfig.nestedAsyncStackEnable = true;
    constexpr uint64_t expectedFlag = ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL | ASYNC_TYPE_FFRT_QUEUE |
        ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROFILER | ASYNC_TYPE_CUSTOMIZE | ASYNC_TYPE_ARKTS_WORKER |
        ASYNC_TYPE_ARKTS_TASKPOOL;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(config, &inputConfig), RET_OK);
    EXPECT_EQ(config->asyncFlag_, expectedFlag);
}

/**
 * @tc.name: ParseNormalConfigSetsAsyncFlagWhenOnlyAsyncEnabled
 * @tc.desc: Parse normal config sets async flag when only async enabled.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseNormalConfigSetsAsyncFlagWhenOnlyAsyncEnabled, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);

    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 15;
    inputConfig.filterSize = 26;
    inputConfig.maxStackDepth = 4;
    inputConfig.statisticsInterval = 55;
    inputConfig.sampleInterval = 768;
    inputConfig.asyncStackEnable = true;
    inputConfig.nestedAsyncStackEnable = false;
    constexpr uint64_t expectedFlag = ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL | ASYNC_TYPE_FFRT_QUEUE |
        ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROMISE | ASYNC_TYPE_CUSTOMIZE | ASYNC_TYPE_ARKTS_WORKER |
        ASYNC_TYPE_ARKTS_TASKPOOL | ASYNC_TYPE_ARKTS_STA_LAUNCH | ASYNC_TYPE_ARKWEB | ASYNC_TYPE_JSVM;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(config, &inputConfig), RET_OK);
    EXPECT_EQ(config->asyncFlag_, expectedFlag);
}

/**
 * @tc.name: ParseNormalConfigSetsAsyncFlagWhenAsyncDisabled
 * @tc.desc: Parse normal config sets async flag when async disabled.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseNormalConfigSetsAsyncFlagWhenAsyncDisabled, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    constexpr uint64_t preservedFlag = 0x123456789ULL;
    config->asyncFlag_ = preservedFlag;

    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 21;
    inputConfig.filterSize = 31;
    inputConfig.maxStackDepth = 5;
    inputConfig.statisticsInterval = 61;
    inputConfig.sampleInterval = 1024;
    inputConfig.asyncStackEnable = false;
    inputConfig.nestedAsyncStackEnable = true;

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(config, &inputConfig), RET_OK);
    EXPECT_EQ(config->asyncFlag_, 0x0);
}

/**
 * @tc.name: ParseNormalConfigCopiesAsyncAndGpuHookSizes
 * @tc.desc: Parse normal config copies async and GPU hook sizes.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseNormalConfigCopiesAsyncAndGpuHookSizes, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    config->traceMask_ = RES_GPU_MASK;
    config->gpuRange_ = {{99, 99, 99, 99}};

    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 12;
    inputConfig.filterSize = 34;
    inputConfig.maxStackDepth = 5;
    inputConfig.statisticsInterval = 67;
    inputConfig.sampleInterval = 890;
    inputConfig.asyncStackEnable = true;
    inputConfig.nestedAsyncStackEnable = true;
    inputConfig.asyncNestingDepth = 8;
    inputConfig.asyncTaskStackDepth = 9;

    inputConfig.hookSizes[TAG_RES_GPU_VK] = std::make_pair(std::make_pair(1u, 2u), std::make_pair(3u, 4u));
    inputConfig.hookSizes[TAG_RES_GPU_GLES_IMAGE] = std::make_pair(std::make_pair(5u, 6u), std::make_pair(7u, 8u));
    inputConfig.hookSizes[TAG_RES_GPU_GLES_BUFFER] =
        std::make_pair(std::make_pair(9u, 10u), std::make_pair(11u, 12u));
    inputConfig.hookSizes[TAG_RES_GPU_CL_IMAGE] = std::make_pair(std::make_pair(13u, 14u), std::make_pair(15u, 16u));
    inputConfig.hookSizes[TAG_RES_GPU_CL_BUFFER] =
        std::make_pair(std::make_pair(17u, 18u), std::make_pair(19u, 20u));

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(config, &inputConfig), RET_OK);
    EXPECT_TRUE(config->asyncStackEnable_);
    EXPECT_TRUE(config->nestedAsyncStackEnable_);
    EXPECT_EQ(config->asyncNestingDepth_, inputConfig.asyncNestingDepth);
    EXPECT_EQ(config->asyncTaskStackDepth_, inputConfig.asyncTaskStackDepth);

    ASSERT_EQ(config->gpuRange_.size(), 5u);
    const std::vector<uint64_t> vkExpected = {1, 2, 3, 4};
    const std::vector<uint64_t> glesImageExpected = {5, 6, 7, 8};
    const std::vector<uint64_t> glesBufferExpected = {9, 10, 11, 12};
    const std::vector<uint64_t> clImageExpected = {13, 14, 15, 16};
    const std::vector<uint64_t> clBufferExpected = {17, 18, 19, 20};
    EXPECT_EQ(config->gpuRange_[0], vkExpected);
    EXPECT_EQ(config->gpuRange_[1], glesImageExpected);
    EXPECT_EQ(config->gpuRange_[2], glesBufferExpected);
    EXPECT_EQ(config->gpuRange_[3], clImageExpected);
    EXPECT_EQ(config->gpuRange_[4], clBufferExpected);
}

/**
 * @tc.name: ParseNormalConfigGpuWithoutHookSizesKeepsGpuRange
 * @tc.desc: Parse normal config GPU without hook sizes keeps GPU range.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseNormalConfigGpuWithoutHookSizesKeepsGpuRange, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    config->traceMask_ = RES_GPU_MASK;
    const std::vector<uint64_t> firstRange = {21, 22, 23, 24};
    const std::vector<uint64_t> secondRange = {31, 32, 33, 34};
    config->gpuRange_ = {firstRange, secondRange};

    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 20;
    inputConfig.filterSize = 40;
    inputConfig.maxStackDepth = 6;
    inputConfig.statisticsInterval = 80;
    inputConfig.sampleInterval = 900;
    inputConfig.asyncStackEnable = false;
    inputConfig.nestedAsyncStackEnable = false;
    inputConfig.asyncNestingDepth = 2;
    inputConfig.asyncTaskStackDepth = 3;

    EXPECT_TRUE(inputConfig.hookSizes.empty());
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(config, &inputConfig), RET_OK);
    ASSERT_EQ(config->gpuRange_.size(), 2u);
    EXPECT_EQ(config->gpuRange_[0], firstRange);
    EXPECT_EQ(config->gpuRange_[1], secondRange);
}

/**
 * @tc.name: ParseNormalConfigNonGpuMaskIgnoresHookSizes
 * @tc.desc: Parse normal config non GPU mask ignores hook sizes.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, ParseNormalConfigNonGpuMaskIgnoresHookSizes, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    config->traceMask_ = RES_FD_MASK;
    const std::vector<uint64_t> preservedRange = {41, 42, 43, 44};
    config->gpuRange_ = {preservedRange};

    MemAPIConfig inputConfig;
    inputConfig.maxDuration = 30;
    inputConfig.filterSize = 50;
    inputConfig.maxStackDepth = 7;
    inputConfig.statisticsInterval = 90;
    inputConfig.sampleInterval = 1000;
    inputConfig.hookSizes[TAG_RES_GPU_VK] = std::make_pair(std::make_pair(1u, 2u), std::make_pair(3u, 4u));
    inputConfig.hookSizes[TAG_RES_GPU_GLES_IMAGE] = std::make_pair(std::make_pair(5u, 6u), std::make_pair(7u, 8u));
    inputConfig.hookSizes[TAG_RES_GPU_GLES_BUFFER] =
        std::make_pair(std::make_pair(9u, 10u), std::make_pair(11u, 12u));
    inputConfig.hookSizes[TAG_RES_GPU_CL_IMAGE] = std::make_pair(std::make_pair(13u, 14u), std::make_pair(15u, 16u));
    inputConfig.hookSizes[TAG_RES_GPU_CL_BUFFER] =
        std::make_pair(std::make_pair(17u, 18u), std::make_pair(19u, 20u));

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::ParseNormalConfig(config, &inputConfig), RET_OK);
    ASSERT_EQ(config->gpuRange_.size(), 1u);
    EXPECT_EQ(config->gpuRange_[0], preservedRange);
}

/**
 * @tc.name: StopWithPathAndCallbackCleanup
 * @tc.desc: Stop with path and callback cleanup.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StopWithPathAndCallbackCleanup, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::string filePath;
    EXPECT_NE(NativeMemoryProfilerSaClientManager::Stop(filePath), RET_OK);
    EXPECT_TRUE(filePath.empty());
}

/**
 * @tc.name: DumpDataRejectsInvalidArgs
 * @tc.desc: Dump data rejects invalid args.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, DumpDataRejectsInvalidArgs, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    config->duration_ = 10;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::DumpData(0, config), RET_ERR);
    auto badConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    badConfig->duration_ = 0;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::DumpData(1, badConfig), RET_ERR);
}

/**
 * @tc.name: GetMallocStatsRejectsInvalidArgs
 * @tc.desc: Get malloc stats rejects invalid args.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, GetMallocStatsRejectsInvalidArgs, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(0, getpid(), 0, false), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(1, 0, 0, false), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::GetMallocStats(1, getpid(), 99, false), RET_ERR);
}

/**
 * @tc.name: EntryCheckRejectsInvalidPid
 * @tc.desc: Entry check rejects invalid PID.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, EntryCheckRejectsInvalidPid, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::vector<SimplifiedMemStats> memStats;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(0, memStats), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(0u), RET_ERR);
}

/**
 * @tc.name: EntryCheckRejectsInvalidPid
 * @tc.desc: Entry check rejects invalid param.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemSaConfigAsyncRejectsInvalidParam, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemSaConfig config;
    config.mask = RES_COMPOSITE_HEAP;
    auto callback = [](int32_t errCode) {};

    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(0, 0, 10, config, nullptr), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(1, 0, 10, config, nullptr), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(1, 1, 10, config, nullptr), RET_ERR);
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(1, 1, 10, config, callback), RET_ERR);
}

/**
 * @tc.name: StartMemSaConfigAsync
 * @tc.desc: StartHook Async And Stoped By Duration.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemSaConfigAsync, TestSize.Level2)
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

    SemaphorePtr sem = GetSemaphoreFactory().Create(0);
    ASSERT_NE(sem, nullptr);
    auto callback = [&sem](int32_t errCode) { sem->Post(); };
    int fd = open(TEST_HTRACE_FD_FILE_PATH.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    int pid = 0;
    if (!COMMON::IsProcessExist(TEST_APP_NAME, pid)) {
        COMMON::IsProcessExist(TEST_NATIVE_NAME, pid);
    }
    ASSERT_TRUE(pid > 0);

    std::string cmd = "setenforce 0";
    system(cmd.c_str());

    ASSERT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(fd, pid, 5, config, callback), RET_OK);
    EXPECT_TRUE(sem->TimedWait(60));
    close(fd);
    unlink(TEST_HTRACE_FD_FILE_PATH.c_str());
    cmd = "setenforce 1";
    system(cmd.c_str());
}

/**
 * @tc.name: StartMemSaConfigAsyncAndStop
 * @tc.desc: StartHook Async And Stoped.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemSaConfigAsyncAndStop, TestSize.Level2)
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

    SemaphorePtr sem = GetSemaphoreFactory().Create(0);
    ASSERT_NE(sem, nullptr);
    auto callback = [&sem](int32_t errCode) { sem->Post(); };
    int fd = open(TEST_HTRACE_FD_FILE_PATH.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    int pid = 0;
    if (!COMMON::IsProcessExist(TEST_APP_NAME, pid)) {
        COMMON::IsProcessExist(TEST_NATIVE_NAME, pid);
    }
    ASSERT_TRUE(pid > 0);

    std::string cmd = "setenforce 0";
    system(cmd.c_str());

    ASSERT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(fd, pid, 10, config, callback), RET_OK);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ASSERT_EQ(NativeMemoryProfilerSaClientManager::Stop(static_cast<uint32_t>(pid)), RET_OK);
    EXPECT_TRUE(sem->TimedWait(60));
    close(fd);
    unlink(TEST_HTRACE_FD_FILE_PATH.c_str());
    cmd = "setenforce 1";
    system(cmd.c_str());
}

/**
 * @tc.name: StartMemSaConfigCompositeHeapAsync
 * @tc.desc: StartHook composite heap Async And Stoped By Duration.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemSaConfigCompositeHeapAsync, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemSaConfig config;
    config.asyncStackEnable = true;
    config.asyncFlag = 5;
    config.mask = RES_COMPOSITE_HEAP | RES_ARK_GLOBAL_HANDLE;

    SemaphorePtr sem = GetSemaphoreFactory().Create(0);
    ASSERT_NE(sem, nullptr);
    auto callback = [&sem](int32_t errCode) { sem->Post(); };
    int fd = open(TEST_COMPOSITE_HEAP_FD_FILE_PATH.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    int pid = 0;
    if (!COMMON::IsProcessExist(TEST_APP_NAME, pid)) {
        COMMON::IsProcessExist(TEST_NATIVE_NAME, pid);
    }
    ASSERT_TRUE(pid > 0);

    std::string cmd = "setenforce 0";
    system(cmd.c_str());

    ASSERT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(fd, pid, 5, config, callback), RET_OK);
    EXPECT_TRUE(sem->TimedWait(60));
    close(fd);
    unlink(TEST_COMPOSITE_HEAP_FD_FILE_PATH.c_str());
    cmd = "setenforce 1";
    system(cmd.c_str());
}

/**
 * @tc.name: StartMemSaConfigCompositeHeapAsyncAndStop
 * @tc.desc: StartHook composite heap Async And Stoped.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemSaConfigCompositeHeapAsyncAndStop, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemSaConfig config;
    config.asyncStackEnable = true;
    config.asyncFlag = 5;
    config.mask = RES_COMPOSITE_HEAP | RES_ARK_GLOBAL_HANDLE;

    SemaphorePtr sem = GetSemaphoreFactory().Create(0);
    ASSERT_NE(sem, nullptr);
    auto callback = [&sem](int32_t errCode) { sem->Post(); };
    int fd = open(TEST_COMPOSITE_HEAP_FD_FILE_PATH.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    int pid = 0;
    if (!COMMON::IsProcessExist(TEST_APP_NAME, pid)) {
        COMMON::IsProcessExist(TEST_NATIVE_NAME, pid);
    }
    ASSERT_TRUE(pid > 0);

    std::string cmd = "setenforce 0";
    system(cmd.c_str());

    ASSERT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(fd, pid, 10, config, callback), RET_OK);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    ASSERT_EQ(NativeMemoryProfilerSaClientManager::Stop(static_cast<uint32_t>(pid)), RET_OK);
    EXPECT_TRUE(sem->TimedWait(60));
    close(fd);
    unlink(TEST_COMPOSITE_HEAP_FD_FILE_PATH.c_str());
    cmd = "setenforce 1";
    system(cmd.c_str());
}

/**
 * @tc.name: StartPrintSimplifiedNmdInHook
 * @tc.desc: Print simplefiled nmd check
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartPrintSimplifiedNmdInHook, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::vector<SimplifiedMemStats> memStats;
    int pid = getpid();
    SemaphorePtr sem = GetSemaphoreFactory().Create(0);
    SimplifiedMemConfig config;
    config.sampleSize = 384;
    config.largestSize = 8;
    config.secondLargestSize = 4;
    config.maxGrowthSize = 8;
    NativeMemoryProfilerSaClientManager::ProfilingDumpCallback profilingCallback = [&sem](int32_t errCode) {
        GTEST_LOG_(INFO) << "Post nmd result:" << errCode;
        sem->Post();
    };
    int fd = open(TEST_COMPOSITE_HEAP_FD_FILE_PATH.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    std::thread t(MallocTest);
    int32_t result = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid, memStats);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd 1 returned:" << result << ", memStats size: " << memStats.size();

    result = NativeMemoryProfilerSaClientManager::StartAsync(fd, pid, 60, config, profilingCallback);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartAsync returned:" << result;
    std::this_thread::sleep_for(std::chrono::seconds(3));   // wait StartAsync done

    memStats.clear();
    result = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid, memStats, true);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd 2 returned:" << result << ", memStats size: " << memStats.size();

    memStats.clear();
    result = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid, memStats, true);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd 3 returned:" << result << ", memStats size: " << memStats.size();

    std::string cmd = "setenforce 0";
    system(cmd.c_str());
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(static_cast<uint32_t>(pid)), RET_OK);
    EXPECT_TRUE(sem->TimedWait(80));
    close(fd);
    if (t.joinable()) {
        t.join();
    }
    cmd = "setenforce 1";
    system(cmd.c_str());
}

/**
 * @tc.name: StartPrintOtherSimplifiedNmdInHook
 * @tc.desc: Print simplefiled nmd of other pid check
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartPrintOtherSimplifiedNmdInHook, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::vector<SimplifiedMemStats> memStats;
    int pid = 0;
    if (!COMMON::IsProcessExist(TEST_APP_NAME, pid)) {
        COMMON::IsProcessExist(TEST_NATIVE_NAME, pid);
    }
    ASSERT_TRUE(pid > 0);
    int fd = open(TEST_COMPOSITE_HEAP_FD_FILE_PATH.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    SemaphorePtr sem = GetSemaphoreFactory().Create(0);
    SimplifiedMemConfig config;
    config.sampleSize = 384;
    config.largestSize = 8;
    config.secondLargestSize = 4;
    config.maxGrowthSize = 8;
    NativeMemoryProfilerSaClientManager::ProfilingDumpCallback profilingCallback = [&sem](int32_t errCode) {
        GTEST_LOG_(INFO) << "StartPrintOtherSimplifiedNmdInHook Post nmd result:" << errCode;
        sem->Post();
    };

    int32_t result = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid, memStats);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd 1 returned:" << result << ", memStats size: " << memStats.size();

    result = NativeMemoryProfilerSaClientManager::StartAsync(fd, pid, 60, config, profilingCallback);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartAsync returned:" << result;
    std::this_thread::sleep_for(std::chrono::seconds(3));   // wait StartAsync done

    memStats.clear();
    result = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid, memStats, true);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd 2 returned:" << result << ", memStats size: " << memStats.size();

    memStats.clear();
    result = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid, memStats, true);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd 3 returned:" << result << ", memStats size: " << memStats.size();

    std::string cmd = "setenforce 0";
    system(cmd.c_str());
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(static_cast<uint32_t>(pid)), RET_OK);
    EXPECT_TRUE(sem->TimedWait(80));
    close(fd);
    cmd = "setenforce 1";
    system(cmd.c_str());
}

/**
 * @tc.name: StartPrintOtherSimplifiedNmdInHookAndUpdateConfig
 * @tc.desc: Print simplefiled nmd of other pid and update config
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartPrintOtherSimplifiedNmdInHookAndUpdateConfig, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::vector<SimplifiedMemStats> memStats;
    int pid = 0;
    if (!COMMON::IsProcessExist(TEST_APP_NAME, pid)) {
        COMMON::IsProcessExist(TEST_NATIVE_NAME, pid);
    }
    ASSERT_TRUE(pid > 0);
    int fd = open(TEST_COMPOSITE_HEAP_FD_FILE_PATH.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    SemaphorePtr sem = GetSemaphoreFactory().Create(0);
    SimplifiedMemConfig config;
    config.sampleSize = 384;
    config.largestSize = 8;
    config.secondLargestSize = 4;
    config.maxGrowthSize = 0;
    NativeMemoryProfilerSaClientManager::ProfilingDumpCallback profilingCallback = [&sem](int32_t errCode) {
        GTEST_LOG_(INFO) << "StartPrintOtherSimplifiedNmdInHook Post nmd result:" << errCode;
        sem->Post();
    };

    int32_t result = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid, memStats);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd 1 returned:" << result << ", memStats size: " << memStats.size();

    result = NativeMemoryProfilerSaClientManager::StartAsync(fd, pid, 60, config, profilingCallback);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartAsync returned:" << result;
    std::this_thread::sleep_for(std::chrono::seconds(3));   // wait StartAsync done

    memStats.clear();
    result = NativeMemoryProfilerSaClientManager::StartPrintSimplifiedNmd(pid, memStats, true);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd 2 returned:" << result << ", memStats size: " << memStats.size();

    config.maxGrowthSize = 8;
    result = NativeMemoryProfilerSaClientManager::UpdateSimplifiedMemConfig(pid, config);
    EXPECT_EQ(result, RET_OK);
    GTEST_LOG_(INFO) << "UpdateSimplifiedMemConfig returned:" << result;

    std::string cmd = "setenforce 0";
    system(cmd.c_str());
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Stop(static_cast<uint32_t>(pid)), RET_OK);
    EXPECT_TRUE(sem->TimedWait(80));
    close(fd);
    cmd = "setenforce 1";
    system(cmd.c_str());
}

/**
 * @tc.name: StartMemSaConfigAsyncMmapAnonymousOnly
 * @tc.desc: StartAsync with mmapAnonymousOnly=true and empty hookSizes (covers BuildMemSaConfig false branches).
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, StartMemSaConfigAsyncMmapAnonymousOnly, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemSaConfig config;
    config.mask = RES_GPU_MASK;
    config.asyncStackEnable = true;
    config.asyncFlag = 5;
    config.mmapAnonymousOnly = true;

    SemaphorePtr sem = GetSemaphoreFactory().Create(0);
    ASSERT_NE(sem, nullptr);
    auto callback = [&sem](int32_t errCode) { sem->Post(); };
    int fd = open(TEST_HTRACE_FD_FILE_PATH.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    int pid = 0;
    if (!COMMON::IsProcessExist(TEST_APP_NAME, pid)) {
        COMMON::IsProcessExist(TEST_NATIVE_NAME, pid);
    }
    ASSERT_TRUE(pid > 0);

    std::string cmd = "setenforce 0";
    system(cmd.c_str());

    ASSERT_EQ(NativeMemoryProfilerSaClientManager::StartAsync(fd, pid, 5, config, callback), RET_OK);
    EXPECT_TRUE(sem->TimedWait(60));
    close(fd);
    unlink(TEST_HTRACE_FD_FILE_PATH.c_str());
    cmd = "setenforce 1";
    system(cmd.c_str());
}
