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

#ifndef NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_H
#define NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_H

#define private public

#include <gtest/gtest.h>
#include <memory>
#include "test_common.h"
#include "init_param.h"
#define private public
#include "native_memory_profiler_sa_service.h"
#undef private
#include "utilities.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "singleton.h"
using namespace OHOS::Security::AccessToken;
using namespace testing::ext;

namespace {
const std::string TEST_PROC_NAME = "hiview";
const std::string TEST_NATIVE_NAME = "foundation";
const std::string NATIVE_PARAM = "hiviewdfx.hiprofiler.native_memoryd.start";
constexpr uint32_t TEST_DURATION = 10;
constexpr uint32_t TEST_SHARE_MEMORY_SIZE = 4096;
static AccessTokenID g_selfTokenId;
static TEST_COMMON::MockNativeToken* g_mock = nullptr;
}

class NativeMemoryProfilerSaServiceTest : public testing::Test {
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
 * @tc.name: NativeMemoryProfilerSaServiceTest001
 * @tc.desc: Test the service interface
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NativeMemoryProfilerSaServiceTest001, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    int32_t pid = GetProcessPid(TEST_NATIVE_NAME);
    EXPECT_NE(pid, 0);
    config->pid_ = pid;
    config->duration_ = TEST_DURATION;
    config->filePath_ = "/data/local/tmp/native_hook_test.htrace";
    config->offlineSymbolization_ = true;
    config->shareMemorySize_ = TEST_SHARE_MEMORY_SIZE;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    EXPECT_EQ(service->Start(config), RET_OK);
    sleep(3);
    EXPECT_EQ(service->Stop(config->pid_), RET_OK);

    uint32_t fd = open("/data/local/tmp/test_dump_data.htrace",
        O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    EXPECT_GT(fd, 0);
    EXPECT_EQ(service->DumpData(fd, config), RET_OK);
    sleep(2);
    EXPECT_EQ(service->Stop(config->pid_), RET_OK);
}

/**
 * @tc.name: NativeMemoryProfilerSaServiceTest
 * @tc.desc: Test the service interface with false case.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NativeMemoryProfilerSaServiceTest002, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    int32_t pid = GetProcessPid(TEST_NATIVE_NAME);
    EXPECT_NE(pid, 0);
    config->pid_ = pid;
    config->duration_ = TEST_DURATION;
    config->filePath_ = "/data/local/tmp/native_hook_test.htrace";
    config->offlineSymbolization_ = true;
    config->shareMemorySize_ = 0;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    EXPECT_EQ(service->Start(config), RET_ERR);

    config->pid_ = 0;
    config->processName_ = "";
    config->shareMemorySize_ = TEST_SHARE_MEMORY_SIZE;
    EXPECT_EQ(service->Start(config), RET_ERR);
}

/**
 * @tc.name: NativeMemoryProfilerSaServiceTest003
 * @tc.desc: Test the service interface with false case.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NativeMemoryProfilerSaServiceTest003, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    int32_t pid = GetProcessPid(TEST_NATIVE_NAME);
    EXPECT_NE(pid, 0);
    config->pid_ = pid;
    config->duration_ = TEST_DURATION;
    config->filePath_ = "/data/local/tmp/native_hook_test.htrace";
    config->offlineSymbolization_ = true;
    config->shareMemorySize_ = 0;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    OHOS::MessageParcel reply;
    EXPECT_EQ(service->Start(config, reply), RET_ERR);
}

/**
 * @tc.name: NativeMemoryProfilerSaServiceTest004
 * @tc.desc: ApplyNmdConfig short-circuit - Start with printNmd_/printNmdOnly_ returns RET_OK without hook
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NativeMemoryProfilerSaServiceTest004, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);
    int32_t pid = GetProcessPid(TEST_NATIVE_NAME);
    EXPECT_NE(pid, 0);
    config->pid_ = pid;
    config->printNmd_ = true;
    config->printNmdOnly_ = true;
    config->nmdPid_ = pid;
    config->nmdType_ = 0;
    config->duration_ = TEST_DURATION;
    config->offlineSymbolization_ = true;
    config->filePath_ = "/data/local/tmp/nmd_only.htrace";
    config->shareMemorySize_ = TEST_SHARE_MEMORY_SIZE;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    EXPECT_EQ(service->Start(config), RET_OK);
    sleep(2);
    EXPECT_EQ(service->Stop(config->pid_), RET_OK);
}

/**
 * @tc.name: GetTaskNumConfigNull
 * @tc.desc: GetTaskNum config is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumConfigNull, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = nullptr;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    int32_t result = service->GetTaskNum(config);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GetTaskNumConfigNull
 * @tc.desc: GetTaskNum startType is API,pid_ not found in g_pidCtx
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumApiTypePidNotFound, TestSize.Level3)
{
    int32_t startTypeApi = 1;
    int32_t notFoundPid = 999;
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->startType_ = startTypeApi;
    config->pid_ = notFoundPid;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    int32_t result = service->GetTaskNum(config);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GetTaskNumConfigNull
 * @tc.desc: GetTaskNum startType is HIVIEW Type, g_pidCtx is empty
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumHiviewTypeEmpty, TestSize.Level3)
{
    uint32_t startTypeHiview = 0;
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    config->startType_ = startTypeHiview;
    
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();

    int32_t result = service->GetTaskNum(config);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: IsNativeDeamonRunningTest
 * @tc.desc: have not process
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, IsNativeDeamonRunningTest, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsNativeDeamonRunning());
}

/**
 * @tc.name: CheckSystemResourceTest
 * @tc.desc: CheckSystemResourceTest
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckSystemResourceTest, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int32_t result = NativeMemoryProfilerSaService::CheckSystemResource();
    EXPECT_EQ(result, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_SUCCESS));
}

/**
 * @tc.name: GetSystemMemoryInfoTest001
 * @tc.desc: invalid params
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetSystemMemoryInfoTest001, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemoryInfo* memInfo = nullptr;
    bool result = NativeMemoryProfilerSaService::GetSystemMemoryInfo(memInfo);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetSystemMemoryInfoTest002
 * @tc.desc: normal case
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetSystemMemoryInfoTest002, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    MemoryInfo memInfo;
    bool result = NativeMemoryProfilerSaService::GetSystemMemoryInfo(&memInfo);
    if (result) {
        EXPECT_GT(memInfo.totalMem, 0);
        EXPECT_GT(memInfo.usedMem, 0);
        EXPECT_GT(memInfo.freeMem, 0);
        EXPECT_GT(memInfo.availablePercent, 0.0);
    }
}

/**
 * @tc.name: GetSystemCpuUsageInfoTest001
 * @tc.desc: invalid params
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetSystemCpuUsageInfoTest001, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    double* cpuUsage = nullptr;
    bool result = NativeMemoryProfilerSaService::GetSystemCpuUsageInfo(cpuUsage);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: GetSystemCpuUsageInfoTest002
 * @tc.desc: normal case
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetSystemCpuUsageInfoTest002, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    double cpuUsage = 0.0;
    bool result = NativeMemoryProfilerSaService::GetSystemCpuUsageInfo(&cpuUsage);
    if (result) {
        EXPECT_GE(cpuUsage, 0.0);
        EXPECT_LE(cpuUsage, 100.0);
    }
}
#endif // NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_H