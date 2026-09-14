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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "async_stack.h"
#include "memory_profiler_client_impl.h"

using namespace testing::ext;

namespace {
#if defined(__LP64__)
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
#endif
}

class MemoryProfilerClientTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: MemoryProfilerClientTest001
 * @tc.desc: The invalid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemoryProfilerClientTest001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest001 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    MemoryProfilerClient* client = new MemoryProfilerClientImpl();
    SimplifiedMemConfig config;
    EXPECT_EQ(client->Start(0, config), ResProfilerErrorCode::RES_PROF_ERROR);
    MemAPIConfig apiConfig;
    int32_t resType = -1;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    resType = 6;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    resType = 2;
    apiConfig.maxDuration = 4000;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.maxDuration = 10;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.filterSize = 8;
    apiConfig.maxStackDepth = 31;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.maxStackDepth = 16;
    apiConfig.statisticsInterval = 4000;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.statisticsInterval = 1;
    apiConfig.sampleInterval = 100;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.sampleInterval = 384;
    apiConfig.asyncTaskStackDepth = 0;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.asyncTaskStackDepth = 258;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.asyncTaskStackDepth = 16;
    apiConfig.asyncNestingDepth = 0;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.asyncNestingDepth = 17;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.asyncNestingDepth = 1;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.nestedAsyncStackEnable = false;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    apiConfig.asyncStackEnable = false;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    delete client;
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest001 end.";
}

/**
 * @tc.name: MemoryProfilerClientTest002
 * @tc.desc: Test stop without start.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemoryProfilerClientTest002, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest002 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    MemoryProfilerClient* client = new MemoryProfilerClientImpl();
    EXPECT_EQ(client->Stop(), RET_ERR);
    delete client;
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest002 end.";
}

/**
 * @tc.name: MemoryProfilerClientTest003
 * @tc.desc: Test simplified nmd mode.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemoryProfilerClientTest003, TestSize.Level3)
{
#if defined(__LP64__)
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest003 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    MemoryProfilerClient* client = new MemoryProfilerClientImpl();
    std::vector<SimplifiedMemStats> memStats;
    std::thread t(MallocTest);
    int32_t ret = client->StartPrintSimplifiedNmd(memStats);
    EXPECT_EQ(ret, RET_OK);
    if (ret == RET_OK) {
        GTEST_LOG_(INFO) << "StartPrintSimplifiedNmd returned ok, memStats size: " << memStats.size();
    }
    delete client;
    if (t.joinable()) {
        t.join();
    }
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest003 end.";
#endif
}

/**
 * @tc.name: MemConfigTest001
 * @tc.desc: Test simplified mem config.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemConfigTest001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "MemConfigTest001 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    uint64_t expectedAsyncFlag = ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL | ASYNC_TYPE_FFRT_QUEUE |
                                 ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROFILER | ASYNC_TYPE_CUSTOMIZE |
                                 ASYNC_TYPE_ARKTS_WORKER | ASYNC_TYPE_ARKTS_TASKPOOL;
    SimplifiedMemConfig config;
    EXPECT_EQ(config.largestSize, 0);
    EXPECT_EQ(config.secondLargestSize, 0);
    EXPECT_EQ(config.maxGrowthSize, 0);
    EXPECT_EQ(config.sampleSize, 0);
    EXPECT_EQ(config.asyncStackEnable, true);
    EXPECT_EQ(config.asyncFlag, expectedAsyncFlag);
    EXPECT_EQ(config.targetSoName, "");
    EXPECT_EQ(config.responseLibraryMode, false);
    EXPECT_EQ(config.nestedAsyncStackEnable, true);
    EXPECT_EQ(config.asyncNestingDepth, 1);
    EXPECT_EQ(config.asyncTaskStackDepth, 16);
    GTEST_LOG_(INFO) << "MemConfigTest001 end.";
}

/**
 * @tc.name: MemConfigTest002
 * @tc.desc: Test SimplifiedMemConfig with partial parameters.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemConfigTest002, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "MemConfigTest002 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    uint64_t expectedAsyncFlag = ASYNC_TYPE_LIBUV_SEND | ASYNC_TYPE_FFRT_POOL | ASYNC_TYPE_FFRT_QUEUE |
                                 ASYNC_TYPE_EVENTHANDLER | ASYNC_TYPE_PROFILER | ASYNC_TYPE_CUSTOMIZE |
                                 ASYNC_TYPE_ARKTS_WORKER | ASYNC_TYPE_ARKTS_TASKPOOL;
    SimplifiedMemConfig config(20, 10, 10, 384);
    EXPECT_EQ(config.largestSize, 20);
    EXPECT_EQ(config.secondLargestSize, 10);
    EXPECT_EQ(config.maxGrowthSize, 10);
    EXPECT_EQ(config.sampleSize, 384);
    EXPECT_EQ(config.asyncStackEnable, true);
    EXPECT_EQ(config.asyncFlag, expectedAsyncFlag);
    EXPECT_EQ(config.targetSoName, "");
    EXPECT_EQ(config.responseLibraryMode, false);
    EXPECT_EQ(config.nestedAsyncStackEnable, true);
    EXPECT_EQ(config.asyncNestingDepth, 1);
    EXPECT_EQ(config.asyncTaskStackDepth, 16);
    GTEST_LOG_(INFO) << "MemConfigTest002 end.";
}

/**
 * @tc.name: MemConfigTest003
 * @tc.desc: Test SimplifiedMemConfig with all parameters.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemConfigTest003, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "MemConfigTest003 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    SimplifiedMemConfig config(20, 10, 10, 384, true, 0x123456789ULL, "libtest.so", true, false, 2, 8);
    EXPECT_EQ(config.largestSize, 20);
    EXPECT_EQ(config.secondLargestSize, 10);
    EXPECT_EQ(config.maxGrowthSize, 10);
    EXPECT_EQ(config.sampleSize, 384);
    EXPECT_EQ(config.asyncStackEnable, true);
    EXPECT_EQ(config.asyncFlag, 0x123456789ULL);
    EXPECT_EQ(config.targetSoName, "libtest.so");
    EXPECT_EQ(config.responseLibraryMode, true);
    EXPECT_EQ(config.nestedAsyncStackEnable, false);
    EXPECT_EQ(config.asyncNestingDepth, 2);
    EXPECT_EQ(config.asyncTaskStackDepth, 8);
    GTEST_LOG_(INFO) << "MemConfigTest003 end.";
}

/**
 * @tc.name: MemConfigTest004
 * @tc.desc: Test MemApiConfig.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemConfigTest004, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "MemConfigTest004 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    MemAPIConfig config;
    EXPECT_EQ(config.maxDuration, 0);
    EXPECT_EQ(config.filterSize, 0);
    EXPECT_EQ(config.maxStackDepth, 0);
    EXPECT_EQ(config.statisticsInterval, 0);
    EXPECT_EQ(config.sampleInterval, 0);
    EXPECT_EQ(config.hookSizes.size(), 0);
    EXPECT_EQ(config.asyncStackEnable, true);
    EXPECT_EQ(config.nestedAsyncStackEnable, true);
    EXPECT_EQ(config.asyncNestingDepth, 1);
    EXPECT_EQ(config.asyncTaskStackDepth, 16);
    GTEST_LOG_(INFO) << "MemConfigTest004 end.";
}

/**
 * @tc.name: MemoryProfilerClientTest004
 * @tc.desc: Test simplified nmd mode in hook mode.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemoryProfilerClientTest004, TestSize.Level3)
{
#if defined(__LP64__)
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest004 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    MemoryProfilerClient* client = new MemoryProfilerClientImpl();
    std::vector<SimplifiedMemStats> memStats;
    EXPECT_TRUE(client->StartPrintSimplifiedNmd(memStats, true) != RET_OK);
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest004 end.";
#endif
}

/**
 * @tc.name: MemoryProfilerClientTest005
 * @tc.desc: Test update mem config err.
 * @tc.type: FUNC
 */
HWTEST_F(MemoryProfilerClientTest, MemoryProfilerClientTest005, TestSize.Level3)
{
#if defined(__LP64__)
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest005 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    MemoryProfilerClient* client = new MemoryProfilerClientImpl();
    SimplifiedMemConfig config;
    config.sampleSize = 384;
    config.largestSize = 8;
    config.secondLargestSize = 4;
    config.maxGrowthSize = 8;
    EXPECT_TRUE(client->UpdateSimplifiedMemConfig(config) != RET_OK);
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest005 end.";
#endif
}