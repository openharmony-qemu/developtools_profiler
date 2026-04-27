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

#include "memory_profiler_client_impl.h"

using namespace testing::ext;

namespace {
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
    std::vector<SimplifiedMemStats> memStats;
    EXPECT_EQ(client->Start(0, config), ResProfilerErrorCode::RES_PROF_ERROR);
    MemAPIConfig apiConfig = {
        .maxDuration = 0,
        .filterSize = 0,
        .maxStackDepth = 0,
        .statisticsInterval = 0,
        .sampleInterval = 0
    };
    int32_t resType = -1;
    EXPECT_EQ(client->Start(resType, apiConfig), ResProfilerErrorCode::RES_PROF_ERROR);
    resType = 5;
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
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest003 start.";
    using namespace OHOS::Developtools::NativeDaemon;
    MemoryProfilerClient* client = new MemoryProfilerClientImpl();
    std::vector<SimplifiedMemStats> memStats;
    std::thread t(MallocTest);
    EXPECT_EQ(client->StartPrintSimplifiedNmd(memStats), RET_OK);
    int left = 5; // 5 seconds
    int32_t end = time(nullptr) + left;
    while (left > 0) {
        left = end - time(nullptr);
    }
    EXPECT_FALSE(memStats.empty());
    delete client;
    if (t.joinable()) {
        t.join();
    }
    GTEST_LOG_(INFO) << "MemoryProfilerClientTest003 end.";
}
