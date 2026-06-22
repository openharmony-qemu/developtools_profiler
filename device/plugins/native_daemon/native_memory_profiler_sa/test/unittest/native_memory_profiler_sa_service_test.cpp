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
#include "ipc_skeleton.h"
#include "native_memory_profiler_sa_callback_proxy.h"
#include "native_memory_profiler_sa_callback_stub.h"
#include "native_memory_profiler_sa_proxy.h"
#include "native_memory_profiler_sa_stub.h"
#define private public
#include "native_memory_profiler_sa_service.h"
#undef private
#include "hook_common.h"
#include "socket_context.h"
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
constexpr uint32_t START_TYPE_API = 1;
static AccessTokenID g_selfTokenId;
static TEST_COMMON::MockNativeToken* g_mock = nullptr;

std::shared_ptr<OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaConfig> MakeSaConfig()
{
    auto config = std::make_shared<OHOS::Developtools::NativeDaemon::NativeMemoryProfilerSaConfig>();
    if (config == nullptr) {
        return nullptr;
    }
    config->processName_ = "sa_coverage_test";
    config->filePath_ = "/data/local/tmp/sa_coverage_test.htrace";
    config->duration_ = TEST_DURATION;
    config->shareMemorySize_ = TEST_SHARE_MEMORY_SIZE;
    config->typeFileName_ = "test";
    return config;
}
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

namespace OHOS::Developtools::NativeDaemon {
class TestNativeMemoryProfilerSaStub : public NativeMemoryProfilerSaStub {
public:
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return RET_OK;
    }
    int32_t StartPidVoid(std::shared_ptr<NativeMemoryProfilerSaConfig>& config,
        const sptr<IRemoteObject>& callback) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        callbackIsNull_ = callback == nullptr;
        return RET_OK;
    }
    int32_t Stop(uint32_t pid) override
    {
        lastPid_ = static_cast<int32_t>(pid);
        return RET_OK;
    }
    int32_t Stop(const std::string& name) override
    {
        lastName_ = name;
        return RET_OK;
    }
    int32_t Stop() override
    {
        stoppedVoid_ = true;
        return RET_OK;
    }
    int32_t StopSync() override
    {
        stoppedSync_ = true;
        return RET_OK;
    }
    int32_t DumpData(uint32_t fd, std::shared_ptr<NativeMemoryProfilerSaConfig>& config) override
    {
        lastFd_ = fd;
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return RET_OK;
    }
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, std::string& replyStats) override
    {
        return RET_ERR;
    }
    int32_t Start(std::shared_ptr<NativeMemoryProfilerSaConfig>& config, MessageParcel &reply) override
    {
        lastPid_ = config == nullptr ? -1 : config->pid_;
        return RET_OK;
    }

    int32_t lastPid_ = -1;
    uint32_t lastFd_ = 0;
    std::string lastName_;
    bool callbackIsNull_ = false;
    bool stoppedVoid_ = false;
    bool stoppedSync_ = false;
};

class TestNativeMemoryProfilerSaCallbackStub : public NativeMemoryProfilerSaCallbackStub {
public:
    void OnProfilingResult(int32_t errorCode, const std::string& filePath) override
    {
        errorCode_ = errorCode;
        filePath_ = filePath;
    }

    int32_t errorCode_ = 0;
    std::string filePath_;
};
}

static bool WriteStubConfig(OHOS::MessageParcel& data, int32_t pid)
{
    auto config = MakeSaConfig();
    if (config == nullptr) {
        return false;
    }
    config->pid_ = pid;
    return config->Marshalling(data);
}

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
 * @tc.name: SaStubRejectsInvalidDescriptor
 * @tc.desc: OnRemoteRequest rejects parcels without interface token.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaStubRejectsInvalidDescriptor, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_PID),
        data, reply, option), OHOS::ERR_INVALID_STATE);
}

/**
 * @tc.name: SaStubDispatchesStopRequests
 * @tc.desc: OnRemoteRequest dispatches stop pid, name, and void requests.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaStubDispatchesStopRequests, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    OHOS::MessageParcel pidData;
    ASSERT_TRUE(pidData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(pidData.WriteUint32(123));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_PID),
        pidData, reply, option), RET_OK);
    EXPECT_EQ(stub->lastPid_, 123);

    OHOS::MessageParcel nameData;
    ASSERT_TRUE(nameData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(nameData.WriteString("native_test"));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_NAME),
        nameData, reply, option), RET_OK);
    EXPECT_EQ(stub->lastName_, "native_test");

    OHOS::MessageParcel voidData;
    ASSERT_TRUE(voidData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_PID_VOID),
        voidData, reply, option), RET_OK);
    EXPECT_TRUE(stub->stoppedVoid_);
}

/**
 * @tc.name: SaStubDispatchesStartAndDumpRequests
 * @tc.desc: OnRemoteRequest dispatches config based start and dump requests.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaStubDispatchesStartAndDumpRequests, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    OHOS::MessageParcel startData;
    ASSERT_TRUE(startData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(WriteStubConfig(startData, 0));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::START),
        startData, reply, option), RET_OK);
    EXPECT_EQ(stub->lastPid_, 0);

    OHOS::MessageParcel startVoidData;
    ASSERT_TRUE(startVoidData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(WriteStubConfig(startVoidData, 0));
    OHOS::sptr<TestNativeMemoryProfilerSaCallbackStub> callback =
        new (std::nothrow) TestNativeMemoryProfilerSaCallbackStub();
    ASSERT_NE(callback, nullptr);
    ASSERT_TRUE(startVoidData.WriteRemoteObject(callback->AsObject()));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::START_HOOK_PID_VOID),
        startVoidData, reply, option), RET_OK);
    EXPECT_FALSE(stub->callbackIsNull_);

    OHOS::MessageParcel dumpData;
    ASSERT_TRUE(dumpData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(WriteStubConfig(dumpData, 0));
    ASSERT_TRUE(dumpData.WriteFileDescriptor(STDOUT_FILENO));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_DATA),
        dumpData, reply, option), RET_OK);
    EXPECT_GT(stub->lastFd_, 0u);
}

/**
 * @tc.name: SaStubRejectsInvalidProcessForDump
 * @tc.desc: StubDumpFile rejects config with missing target pid.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaStubRejectsInvalidProcessForDump, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    ASSERT_TRUE(data.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(WriteStubConfig(data, 999999));
    ASSERT_TRUE(data.WriteFileDescriptor(STDOUT_FILENO));

    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_DATA),
        data, reply, option), RET_ERR);
}

/**
 * @tc.name: SaProxyRejectsNullRemote
 * @tc.desc: proxy methods return RET_ERR when remote object is null.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaProxyRejectsNullRemote, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::sptr<NativeMemoryProfilerSaProxy> proxy = new (std::nothrow) NativeMemoryProfilerSaProxy(nullptr);
    ASSERT_NE(proxy, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    std::string replyStats;

    EXPECT_EQ(proxy->Start(config), RET_ERR);
    EXPECT_EQ(proxy->StartPidVoid(config, nullptr), RET_ERR);
    EXPECT_EQ(proxy->DumpData(STDOUT_FILENO, config), RET_ERR);
    EXPECT_EQ(proxy->Start(config, replyStats), RET_ERR);
}

/**
 * @tc.name: SaProxyDispatchesStopRequests
 * @tc.desc: proxy stop methods dispatch pid, name, and void requests to remote stub.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaProxyDispatchesStopRequests, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);
    OHOS::sptr<NativeMemoryProfilerSaProxy> proxy =
        new (std::nothrow) NativeMemoryProfilerSaProxy(stub->AsObject());
    ASSERT_NE(proxy, nullptr);

    EXPECT_EQ(proxy->Stop(321), RET_OK);
    EXPECT_EQ(stub->lastPid_, 321);

    EXPECT_EQ(proxy->Stop("proxy_stop_name"), RET_OK);
    EXPECT_EQ(stub->lastName_, "proxy_stop_name");

    EXPECT_EQ(proxy->Stop(), RET_OK);
    EXPECT_TRUE(stub->stoppedVoid_);

    EXPECT_EQ(proxy->StopSync(), RET_OK);
    EXPECT_TRUE(stub->stoppedSync_);
}

/**
 * @tc.name: SaCallbackStubDispatchesResult
 * @tc.desc: callback stub validates descriptor and forwards profiling result.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaCallbackStubDispatchesResult, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::sptr<TestNativeMemoryProfilerSaCallbackStub> stub =
        new (std::nothrow) TestNativeMemoryProfilerSaCallbackStub();
    ASSERT_NE(stub, nullptr);
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    EXPECT_EQ(stub->OnRemoteRequest(0, data, reply, option), OHOS::ERR_INVALID_STATE);

    OHOS::MessageParcel validData;
    ASSERT_TRUE(validData.WriteInterfaceToken(INativeMemoryProfilerSaCallback::GetDescriptor()));
    ASSERT_TRUE(validData.WriteInt32(7));
    ASSERT_TRUE(validData.WriteString("/data/local/tmp/result.htrace"));
    EXPECT_EQ(stub->OnRemoteRequest(0, validData, reply, option), OHOS::ERR_NONE);
    EXPECT_EQ(stub->errorCode_, 7);
    EXPECT_EQ(stub->filePath_, "/data/local/tmp/result.htrace");
}

/**
 * @tc.name: SaCallbackProxyHandlesRemotePaths
 * @tc.desc: callback proxy handles null remote and dispatches to a local callback stub.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaCallbackProxyHandlesRemotePaths, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    OHOS::sptr<NativeMemoryProfilerSaCallbackProxy> nullProxy =
        new (std::nothrow) NativeMemoryProfilerSaCallbackProxy(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    nullProxy->OnProfilingResult(1, "/data/local/tmp/null.htrace");

    OHOS::sptr<TestNativeMemoryProfilerSaCallbackStub> callback =
        new (std::nothrow) TestNativeMemoryProfilerSaCallbackStub();
    ASSERT_NE(callback, nullptr);
    OHOS::sptr<NativeMemoryProfilerSaCallbackProxy> proxy =
        new (std::nothrow) NativeMemoryProfilerSaCallbackProxy(callback->AsObject());
    ASSERT_NE(proxy, nullptr);

    proxy->OnProfilingResult(9, "/data/local/tmp/proxy.htrace");

    EXPECT_EQ(callback->errorCode_, 9);
    EXPECT_EQ(callback->filePath_, "/data/local/tmp/proxy.htrace");
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
 * @tc.name: OnStopClearsStartedState
 * @tc.desc: OnStop clears started state from both stopped and started states.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, OnStopClearsStartedState, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    service->started_ = false;
    service->OnStop();
    EXPECT_FALSE(service->started_);
    service->started_ = true;
    service->OnStop();
    EXPECT_FALSE(service->started_);
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

/**
 * @tc.name: ParseCpuStatsValidContent
 * @tc.desc: ParseCpuStats parses aggregate CPU line.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ParseCpuStatsValidContent, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    unsigned long total = 0;
    unsigned long idle = 0;
    std::string content = "cpu  1 2 3 4 5 6 7 8 9 10\ncpu0 1 1 1 1 1 1 1 1 1 1\n";
    EXPECT_TRUE(NativeMemoryProfilerSaService::ParseCpuStats(content, total, idle));
    EXPECT_EQ(total, 36UL);
    EXPECT_EQ(idle, 9UL);
}

/**
 * @tc.name: ParseCpuStatsRejectsMissingCpuLine
 * @tc.desc: ParseCpuStats rejects content without aggregate CPU line.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ParseCpuStatsRejectsMissingCpuLine, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    unsigned long total = 0;
    unsigned long idle = 0;
    EXPECT_FALSE(NativeMemoryProfilerSaService::ParseCpuStats("intr 1 2 3\n", total, idle));
}

/**
 * @tc.name: ParseCpuStatsRejectsEmptyContent
 * @tc.desc: ParseCpuStats returns false when content has no first line.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ParseCpuStatsRejectsEmptyContent, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    unsigned long total = 0;
    unsigned long idle = 0;
    EXPECT_FALSE(NativeMemoryProfilerSaService::ParseCpuStats("", total, idle));
}

/**
 * @tc.name: GetCmdArgsIncludesConfiguredFields
 * @tc.desc: GetCmdArgs reports configured boolean and numeric fields.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetCmdArgsIncludesConfiguredFields, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filterSize_ = 32;
    config->mallocDisable_ = true;
    config->mmapDisable_ = true;
    config->asyncStackEnable_ = true;
    config->nmdPid_ = 1234;
    config->nmdType_ = 2;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    std::string args = service->GetCmdArgs(config);
    EXPECT_NE(args.find("filter_size: 32"), std::string::npos);
    EXPECT_NE(args.find("malloc_disable: true"), std::string::npos);
    EXPECT_NE(args.find("asyncStack_enable: true"), std::string::npos);
    EXPECT_NE(args.find("nmd_pid: 1234"), std::string::npos);
}

/**
 * @tc.name: CheckConfigRejectsMissingFilePath
 * @tc.desc: CheckConfig rejects config without output path when fd is empty.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigRejectsMissingFilePath, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filePath_.clear();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->CheckConfig(config), RET_ERR);
}

/**
 * @tc.name: CheckConfigAcceptsExistingFd
 * @tc.desc: CheckConfig accepts valid process config when caller provides fd.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigAcceptsExistingFd, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filePath_.clear();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->CheckConfig(config, STDOUT_FILENO), RET_OK);
}

/**
 * @tc.name: ApplyNmdConfigUsesPrintMode
 * @tc.desc: ApplyNmdConfig returns whether hook should be skipped by NMD mode.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ApplyNmdConfigUsesPrintMode, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_FALSE(service->ApplyNmdConfig(config));
    config->printNmd_ = true;
    config->printNmdOnly_ = false;
    config->nmdPid_ = 4321;
    EXPECT_TRUE(service->ApplyNmdConfig(config));
    config->printNmdOnly_ = true;
    EXPECT_FALSE(service->ApplyNmdConfig(config));
}

/**
 * @tc.name: ApplyMemtraceConfigUpdatesGpuRange
 * @tc.desc: ApplyMemtraceConfig copies GPU ranges used by client config.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ApplyMemtraceConfigUpdatesGpuRange, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->memtraceEnable_ = true;
    config->gpuRange_[GPU_VK_INDEX][0] = 11;
    config->gpuRange_[GPU_GLES_IMAGE_INDEX][1] = 22;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    service->ApplyMemtraceConfig(config);

    ClientConfig clientConfig;
    service->SetGpuConfig(clientConfig);
    EXPECT_EQ(clientConfig.gpuRange.gpuVk[0], 11u);
    EXPECT_EQ(clientConfig.gpuRange.gpuGlesImage[1], 22u);
}

/**
 * @tc.name: ApplyMemtraceConfigIgnoresDisabledConfig
 * @tc.desc: ApplyMemtraceConfig keeps previous GPU ranges when memtrace is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ApplyMemtraceConfigIgnoresDisabledConfig, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto enabledConfig = MakeSaConfig();
    auto disabledConfig = MakeSaConfig();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(enabledConfig, nullptr);
    ASSERT_NE(disabledConfig, nullptr);
    ASSERT_NE(service, nullptr);

    enabledConfig->memtraceEnable_ = true;
    enabledConfig->gpuRange_[GPU_CL_BUFFER_INDEX][0] = 31;
    enabledConfig->gpuRange_[GPU_GLES_IMAGE_INDEX][1] = 42;
    service->ApplyMemtraceConfig(enabledConfig);

    disabledConfig->memtraceEnable_ = false;
    disabledConfig->gpuRange_[GPU_CL_BUFFER_INDEX][0] = 99;
    disabledConfig->gpuRange_[GPU_GLES_IMAGE_INDEX][1] = 100;
    service->ApplyMemtraceConfig(disabledConfig);

    ClientConfig clientConfig;
    service->SetGpuConfig(clientConfig);
    EXPECT_EQ(clientConfig.gpuRange.gpuClBuffer[0], 31u);
    EXPECT_EQ(clientConfig.gpuRange.gpuGlesImage[1], 42u);
}

/**
 * @tc.name: RefreshFileCacheModeConfigEnablesFlag
 * @tc.desc: RefreshFileCacheModeConfig enables file cache mode.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, RefreshFileCacheModeConfigEnablesFlag, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_FALSE(config->enableFileCacheMode_);
    service->RefreshFileCacheModeConfig(config);
    EXPECT_TRUE(config->enableFileCacheMode_);
}

/**
 * @tc.name: StopHookWithoutStartedTaskReturnsNotStarted
 * @tc.desc: StopHook returns not-started error for unknown pid.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StopHookWithoutStartedTaskReturnsNotStarted, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int32_t result = NativeMemoryProfilerSaService::StopHook(987654, "");
    EXPECT_EQ(result, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED));
}

/**
 * @tc.name: StartAndDumpDataRejectNullConfig
 * @tc.desc: Start and DumpData reject nullptr config on all SA entry variants.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartAndDumpDataRejectNullConfig, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = nullptr;
    OHOS::MessageParcel reply;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->Start(config), RET_ERR);
    EXPECT_EQ(service->Start(config, reply), RET_ERR);
    EXPECT_EQ(service->DumpData(STDOUT_FILENO, config), RET_ERR);
}

/**
 * @tc.name: PrivateStartHookRejectsNullConfig
 * @tc.desc: private StartHook overloads reject nullptr config before creating hook session.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, PrivateStartHookRejectsNullConfig, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = nullptr;
    OHOS::MessageParcel reply;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->StartHook(config), RET_ERR);
    EXPECT_EQ(service->StartHook(config, STDOUT_FILENO, reply), RET_ERR);
}

/**
 * @tc.name: StopVoidReturnsNotStartedWhenNoCallingPidTask
 * @tc.desc: Stop without pid returns not-started when caller has no active API task.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StopVoidReturnsNotStartedWhenNoCallingPidTask, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->Stop(), static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED));
}

/**
 * @tc.name: StartPidVoidRejectsMismatchedCallingPid
 * @tc.desc: StartPidVoid rejects requests targeting a pid different from the caller.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartPidVoidRejectsMismatchedCallingPid, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = static_cast<int32_t>(OHOS::IPCSkeleton::GetCallingPid()) + 1;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->StartPidVoid(config, nullptr),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED));
}

HWTEST_F(NativeMemoryProfilerSaServiceTest, StartPidVoidNmdModeReturnsOkForCallerPid, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = static_cast<int32_t>(OHOS::IPCSkeleton::GetCallingPid());
    config->printNmd_ = true;
    config->printNmdOnly_ = false;
    config->nmdPid_ = static_cast<uint32_t>(config->pid_);

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->StartPidVoid(config, nullptr), RET_OK);
}

HWTEST_F(NativeMemoryProfilerSaServiceTest, StartAndDumpDataNmdModesReturnOk, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto startConfig = MakeSaConfig();
    auto dumpConfig = MakeSaConfig();
    ASSERT_NE(startConfig, nullptr);
    ASSERT_NE(dumpConfig, nullptr);
    startConfig->printNmd_ = true;
    startConfig->printNmdOnly_ = false;
    startConfig->nmdPid_ = static_cast<uint32_t>(getpid());
    dumpConfig->printNmd_ = true;
    dumpConfig->printNmdOnly_ = false;
    dumpConfig->nmdPid_ = static_cast<uint32_t>(getpid());

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->Start(startConfig), RET_OK);
    EXPECT_EQ(service->DumpData(STDOUT_FILENO, dumpConfig), RET_OK);
}

HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigAcceptsExtendedProcessAndFd, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->extendProcesses_ = true;
    config->filePath_.clear();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckConfig(config, STDOUT_FILENO), RET_OK);
}

HWTEST_F(NativeMemoryProfilerSaServiceTest, PidAndNameConflictAllowsProcessNameOnly, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "sa_name_only_coverage";

    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(config), RET_OK);
}

/**
 * @tc.name: PidAndNameConflictRejectsNullConfig
 * @tc.desc: PidAndNameConflict rejects nullptr config.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, PidAndNameConflictRejectsNullConfig, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = nullptr;
    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(config), RET_ERR);
}

/**
 * @tc.name: PidAndNameConflictRejectsMissingIdentifiers
 * @tc.desc: PidAndNameConflict rejects config without pid and process name.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, PidAndNameConflictRejectsMissingIdentifiers, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_.clear();

    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(config), RET_ERR);
}

/**
 * @tc.name: PidAndNameConflictClearsProcessNameForPidStart
 * @tc.desc: PidAndNameConflict clears processName when pid is provided and no task exists.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, PidAndNameConflictClearsProcessNameForPidStart, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = TEST_DURATION;
    config->processName_ = "pid_preferred_name";

    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(config), RET_OK);
    EXPECT_TRUE(config->processName_.empty());
}

/**
 * @tc.name: ReceiveCacheFileMsgRejectsNullBuffer
 * @tc.desc: ReceiveCacheFileMsg rejects nullptr buffer.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ReceiveCacheFileMsgRejectsNullBuffer, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    SocketContext context;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_FALSE(service->ReceiveCacheFileMsg(context, nullptr));
}

/**
 * @tc.name: ProtocolProcRejectsUnexpectedPacketSize
 * @tc.desc: ProtocolProc rejects non-int and non-cache-file packets.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcRejectsUnexpectedPacketSize, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int value = 0;
    SocketContext context;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_FALSE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&value), sizeof(uint16_t)));
}

/**
 * @tc.name: ProtocolProcRejectsCacheFileMsgWithoutContext
 * @tc.desc: ProtocolProc rejects cache file message when pid context is missing.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcRejectsCacheFileMsgWithoutContext, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    HookCacheFileMsg cacheFileMsg(7654321, 0, CACHE_FILE_STOP);
    SocketContext context;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_FALSE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&cacheFileMsg),
        sizeof(cacheFileMsg)));
}

/**
 * @tc.name: ProtocolProcHandlesMemLeakDoneWithoutContext
 * @tc.desc: ProtocolProc accepts MEM_LEAK_DONE_CMD even when socket has no registered pid.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcHandlesMemLeakDoneWithoutContext, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int peerConfig = static_cast<int>(MEM_LEAK_DONE_CMD);
    SocketContext context;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_TRUE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
}

/**
 * @tc.name: ProtocolProcRejectsNegativePeerConfig
 * @tc.desc: ProtocolProc rejects hook client pid -1.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcRejectsNegativePeerConfig, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    int peerConfig = -1;
    SocketContext context;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_FALSE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
}
/**
 * @tc.name: StartApiTypePidMismatchTest
 * @tc.desc: Start returns PERMISSION_DENIED when startType is API but pid does not match calling pid.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartApiTypePidMismatchTest, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = std::make_shared<NativeMemoryProfilerSaConfig>();
    ASSERT_NE(config, nullptr);
    config->startType_ = START_TYPE_API;
    config->pid_ = 99999;
    config->duration_ = TEST_DURATION;
    config->shareMemorySize_ = TEST_SHARE_MEMORY_SIZE;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    int32_t result = service->Start(config);
    EXPECT_EQ(result, static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED));
}

/**
 * @tc.name: CheckTaskLimitPassesWhenUnderLimit
 * @tc.desc: CheckTaskLimit returns RET_OK when task count is within limits.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckTaskLimitPassesWhenUnderLimit, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->extendProcesses_ = false;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckTaskLimit(config), RET_OK);
}

/**
 * @tc.name: CheckStartupModeConflictReturnsOkWhenNoStartupMode
 * @tc.desc: CheckStartupModeConflict returns RET_OK when no startup mode is active.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckStartupModeConflictReturnsOkWhenNoStartupMode, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->startupMode_ = false;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckStartupModeConflict(config), RET_OK);
}

/**
 * @tc.name: CheckPidNameConflictReturnsOkForUniquePid
 * @tc.desc: CheckPidNameConflict returns RET_OK when PID is not in use.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckPidNameConflictReturnsOkForUniquePid, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = getpid();

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckPidNameConflict(config), RET_OK);
}

/**
 * @tc.name: CheckFilePathSkipsWhenFdProvided
 * @tc.desc: CheckFilePath returns RET_OK when fd > 0 (file-cache mode).
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckFilePathSkipsWhenFdProvided, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filePath_.clear();

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckFilePath(STDOUT_FILENO, config), RET_OK);
}

/**
 * @tc.name: CheckFilePathRejectsEmptyPath
 * @tc.desc: CheckFilePath returns RET_ERR when filePath_ is empty and fd is 0.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckFilePathRejectsEmptyPath, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->filePath_.clear();

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckFilePath(0, config), RET_ERR);
}

/**
 * @tc.name: CheckTryBeginStopHook
 * @tc.desc: Check try stop hook fail when hook is stopping
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckTryBeginStopHook, TestSize.Level2)
{
    using namespace OHOS::Developtools::NativeDaemon;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    uint32_t pid = static_cast<uint32_t>(getpid());
    EXPECT_TRUE(TryBeginStopHook(pid));
    EXPECT_FALSE(TryBeginStopHook(pid));
    EndStopHook(pid);
    EXPECT_TRUE(TryBeginStopHook(pid));
    EndStopHook(pid);
}
#endif // NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_H
