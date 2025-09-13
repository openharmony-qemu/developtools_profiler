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

#include <gtest/gtest.h>
#include <unistd.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
};

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
 * @tc.desc: Test get stack info with MemSaConfig
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaClientManagerTest, NativeMemoryProfilerSaClientManagerTest006, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemSaConfig config;
    EXPECT_EQ(NativeMemoryProfilerSaClientManager::Start(0, 0, 100, config), RET_ERR);
}
#endif // NATIVE_MEMORY_PROFILER_SA_CLINET_MANAGER_TEST_H