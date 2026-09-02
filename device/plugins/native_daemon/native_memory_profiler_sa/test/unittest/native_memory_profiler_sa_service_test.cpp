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

#include "native_memory_profiler_sa_service_test_common.h"
using namespace OHOS::Security::AccessToken;
using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace OHOS::Developtools::NativeDaemon {
static bool ResetRegisteredSocketFdForStop(NativeMemoryProfilerSaService& service, TestSocketContext& context,
    int peerConfig, const std::string& tag)
{
    auto config = MakeSaConfig();
    if (config == nullptr) {
        return false;
    }
    config->pid_ = 0;
    config->processName_ = tag + "_" + std::to_string(getpid());
    config->startupMode_ = true;
    config->filePath_ = MakeTracePath(tag);
    if (service.Start(config) != RET_OK) {
        return false;
    }
    context.SetSocketHandleForTest(0);
    bool socketFdReset = !service.ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig),
        sizeof(peerConfig));
    bool configStopped = service.Stop(config->processName_) == RET_OK;
    return socketFdReset && configStopped;
}

static void ExpectCacheFileReadyReplenishesFd(NativeMemoryProfilerSaService& service,
    TestSocketContext& serviceContext, TestSocketContext& peerContext, int peerConfig, int peerSock)
{
    constexpr uint16_t readyFileIndex = FILE_CACHE_FD_COUNT - 1;
    char tempPath[] = "/data/local/tmp/sa_cache_ready_XXXXXX";
    int cacheFd = mkstemp(tempPath);
    EXPECT_GE(cacheFd, 0);
    EXPECT_EQ(unlink(tempPath), 0);
    EXPECT_TRUE(peerContext.SendFileDescriptor(cacheFd));
    close(cacheFd);
    HookCacheFileMsg readyMsg(peerConfig, readyFileIndex, CACHE_FILE_READY);
    EXPECT_TRUE(service.ReceiveCacheFileMsg(serviceContext, reinterpret_cast<const int8_t*>(&readyMsg)));
    ProtocolHead head = {};
    EXPECT_EQ(read(peerSock, &head, sizeof(head)), static_cast<ssize_t>(sizeof(head)));
    HookCacheFileMsg createdMsg;
    EXPECT_EQ(read(peerSock, &createdMsg, sizeof(createdMsg)), static_cast<ssize_t>(sizeof(createdMsg)));
    int replenishFd = peerContext.ReceiveFileDiscriptor();
    EXPECT_EQ(createdMsg.msgType, CACHE_FILE_CREATED);
    EXPECT_EQ(createdMsg.fileIndex, readyFileIndex);
    EXPECT_GE(replenishFd, 0);
    close(replenishFd);
}
} // namespace OHOS::Developtools::NativeDaemon

/**
 * @tc.name: NativeMemoryProfilerSaServiceTest001
 * @tc.desc: Test the service interface
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NativeMemoryProfilerSaServiceTest001, TestSize.Level3)
{
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
    EXPECT_EQ(service->Stop(config->pid_), RET_OK);

    uint32_t fd = open("/data/local/tmp/test_dump_data.htrace",
        O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    EXPECT_GT(fd, 0);
    EXPECT_EQ(service->DumpData(fd, config), RET_OK);
    EXPECT_EQ(service->Stop(config->pid_), RET_OK);
}

/**
 * @tc.name: NativeMemoryProfilerSaServiceTest002
 * @tc.desc: Native memory profiler SA service test 002.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NativeMemoryProfilerSaServiceTest002, TestSize.Level3)
{
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
 * @tc.name: SaStubRejectsMalformedPayloads
 * @tc.desc: OnRemoteRequest rejects requests with missing payload fields.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaStubRejectsMalformedPayloads, TestSize.Level2)
{
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);

    auto callWithTokenOnly = [&stub](NativeMemoryProfilerSaInterfaceCode code) {
        OHOS::MessageParcel data;
        OHOS::MessageParcel reply;
        OHOS::MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
        return stub->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
    };

    EXPECT_EQ(callWithTokenOnly(NativeMemoryProfilerSaInterfaceCode::START), RET_ERR);
    EXPECT_EQ(callWithTokenOnly(NativeMemoryProfilerSaInterfaceCode::START_HOOK_PID_VOID), RET_ERR);
    EXPECT_EQ(callWithTokenOnly(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_PID), RET_ERR);
    EXPECT_EQ(callWithTokenOnly(NativeMemoryProfilerSaInterfaceCode::STOP_HOOK_NAME), RET_ERR);
    EXPECT_EQ(callWithTokenOnly(NativeMemoryProfilerSaInterfaceCode::DUMP_DATA), RET_ERR);
    EXPECT_EQ(callWithTokenOnly(NativeMemoryProfilerSaInterfaceCode::DUMP_SIMP_DATA), RET_ERR);
}

/**
 * @tc.name: SaStubDispatchesDumpSimpRequests
 * @tc.desc: OnRemoteRequest dispatches simplified dump request and rejects missing process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaStubDispatchesDumpSimpRequests, TestSize.Level2)
{
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    OHOS::MessageParcel validData;
    ASSERT_TRUE(validData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(WriteStubConfig(validData, 0));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_SIMP_DATA),
        validData, reply, option), RET_OK);
    EXPECT_EQ(stub->lastPid_, 0);

    OHOS::MessageParcel missingPidData;
    ASSERT_TRUE(missingPidData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(WriteStubConfig(missingPidData, 999999));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_SIMP_DATA),
        missingPidData, reply, option), RET_ERR);
}

/**
 * @tc.name: SaStubHandlesUnknownAndExistingPidDumpRequests
 * @tc.desc: OnRemoteRequest handles default code and dump requests for an existing process.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaStubHandlesUnknownAndExistingPidDumpRequests, TestSize.Level2)
{
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;

    OHOS::MessageParcel unknownData;
    ASSERT_TRUE(unknownData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    EXPECT_NE(stub->OnRemoteRequest(UINT32_MAX, unknownData, reply, option), RET_OK);

    OHOS::MessageParcel dumpData;
    ASSERT_TRUE(dumpData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(WriteStubConfig(dumpData, getpid()));
    ASSERT_TRUE(dumpData.WriteFileDescriptor(STDOUT_FILENO));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_DATA),
        dumpData, reply, option), RET_OK);
    EXPECT_EQ(stub->lastPid_, getpid());

    OHOS::MessageParcel simpData;
    ASSERT_TRUE(simpData.WriteInterfaceToken(INativeMemoryProfilerSa::GetDescriptor()));
    ASSERT_TRUE(WriteStubConfig(simpData, getpid()));
    EXPECT_EQ(stub->OnRemoteRequest(static_cast<uint32_t>(NativeMemoryProfilerSaInterfaceCode::DUMP_SIMP_DATA),
        simpData, reply, option), RET_OK);
}

/**
 * @tc.name: SaStubRejectsInvalidProcessForDump
 * @tc.desc: StubDumpFile rejects config with missing target pid.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaStubRejectsInvalidProcessForDump, TestSize.Level2)
{
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
}

/**
 * @tc.name: SaProxyDispatchesStartDumpAndReplyStats
 * @tc.desc: proxy start and dump methods dispatch to remote stub and validate reply stats.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaProxyDispatchesStartDumpAndReplyStats, TestSize.Level2)
{
    OHOS::sptr<TestNativeMemoryProfilerSaStub> stub = new (std::nothrow) TestNativeMemoryProfilerSaStub();
    ASSERT_NE(stub, nullptr);
    OHOS::sptr<NativeMemoryProfilerSaProxy> proxy =
        new (std::nothrow) NativeMemoryProfilerSaProxy(stub->AsObject());
    ASSERT_NE(proxy, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;

    EXPECT_EQ(proxy->Start(config), RET_OK);
    EXPECT_EQ(stub->lastPid_, 0);

    OHOS::sptr<TestNativeMemoryProfilerSaCallbackStub> callback =
        new (std::nothrow) TestNativeMemoryProfilerSaCallbackStub();
    ASSERT_NE(callback, nullptr);
    EXPECT_EQ(proxy->StartPidVoid(config, callback->AsObject()), RET_OK);
    EXPECT_FALSE(stub->callbackIsNull_);

    EXPECT_EQ(proxy->DumpData(STDOUT_FILENO, config), RET_OK);
    EXPECT_GT(stub->lastFd_, 0u);
    std::string replyStats = "unchanged";
    EXPECT_EQ(proxy->Start(config, replyStats), RET_ERR);
    EXPECT_TRUE(replyStats.empty());
}

/**
 * @tc.name: SaProxyHandlesRemoteErrorsAndReplyStatsSuccess
 * @tc.desc: SA proxy handles remote errors and reply stats success.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaProxyHandlesRemoteErrorsAndReplyStatsSuccess, TestSize.Level2)
{
    OHOS::sptr<FailingNativeMemoryProfilerSaStub> failingStub =
        new (std::nothrow) FailingNativeMemoryProfilerSaStub();
    ASSERT_NE(failingStub, nullptr);
    OHOS::sptr<NativeMemoryProfilerSaProxy> failingProxy =
        new (std::nothrow) NativeMemoryProfilerSaProxy(failingStub->AsObject());
    ASSERT_NE(failingProxy, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    OHOS::sptr<TestNativeMemoryProfilerSaCallbackStub> callback =
        new (std::nothrow) TestNativeMemoryProfilerSaCallbackStub();
    ASSERT_NE(callback, nullptr);

    EXPECT_EQ(failingProxy->Start(config), RET_ERR);
    EXPECT_EQ(failingProxy->StartPidVoid(config, callback->AsObject()), RET_ERR);
    EXPECT_EQ(failingProxy->DumpData(STDOUT_FILENO, config), RET_ERR);
    EXPECT_EQ(failingProxy->Stop(123), RET_ERR);
    EXPECT_EQ(failingProxy->Stop("remote_error"), RET_ERR);
    EXPECT_EQ(failingProxy->Stop(), RET_ERR);
    EXPECT_EQ(failingStub->lastPid_, 123);
    EXPECT_EQ(failingStub->lastName_, "remote_error");
    EXPECT_TRUE(failingStub->stoppedVoid_);

    OHOS::sptr<ReplyNativeMemoryProfilerSaStub> replyStub =
        new (std::nothrow) ReplyNativeMemoryProfilerSaStub();
    ASSERT_NE(replyStub, nullptr);
    OHOS::sptr<NativeMemoryProfilerSaProxy> replyProxy =
        new (std::nothrow) NativeMemoryProfilerSaProxy(replyStub->AsObject());
    ASSERT_NE(replyProxy, nullptr);
    std::string replyStats;
    EXPECT_EQ(replyProxy->Start(config, replyStats), RET_OK);
    EXPECT_EQ(replyStats, "reply_stats_ok");
}

/**
 * @tc.name: SaProxyHandleTimeoutClearsStartParameter
 * @tc.desc: HandleTimeout clears memprofiler start parameter when collector is missing.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaProxyHandleTimeoutClearsStartParameter, TestSize.Level2)
{
    NativeMemoryProfilerSaProxy::HandleTimeout();
    EXPECT_EQ(OHOS::system::GetParameter("hiviewdfx.hiprofiler.memprofiler.start", ""), "0");
}

/**
 * @tc.name: SaCallbackStubDispatchesResult
 * @tc.desc: callback stub validates descriptor and forwards profiling result.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, SaCallbackStubDispatchesResult, TestSize.Level2)
{
    OHOS::sptr<TestNativeMemoryProfilerSaCallbackStub> stub =
        new (std::nothrow) TestNativeMemoryProfilerSaCallbackStub();
    ASSERT_NE(stub, nullptr);
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    OHOS::MessageOption option;
    EXPECT_EQ(stub->OnRemoteRequest(0, data, reply, option), OHOS::ERR_INVALID_STATE);

    OHOS::MessageParcel validData;
    ASSERT_TRUE(validData.WriteInterfaceToken(INativeMemoryProfilerSaCallback::GetDescriptor()));
    ASSERT_TRUE(validData.WriteInt32(100));
    ASSERT_TRUE(validData.WriteInt32(7));
    ASSERT_TRUE(validData.WriteString("/data/local/tmp/result.htrace"));
    EXPECT_EQ(stub->OnRemoteRequest(0, validData, reply, option), OHOS::ERR_NONE);
    EXPECT_EQ(stub->pid_, 100);
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
    OHOS::sptr<NativeMemoryProfilerSaCallbackProxy> nullProxy =
        new (std::nothrow) NativeMemoryProfilerSaCallbackProxy(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    nullProxy->OnProfilingResult(0, 1, "/data/local/tmp/null.htrace");

    OHOS::sptr<TestNativeMemoryProfilerSaCallbackStub> callback =
        new (std::nothrow) TestNativeMemoryProfilerSaCallbackStub();
    ASSERT_NE(callback, nullptr);
    OHOS::sptr<NativeMemoryProfilerSaCallbackProxy> proxy =
        new (std::nothrow) NativeMemoryProfilerSaCallbackProxy(callback->AsObject());
    ASSERT_NE(proxy, nullptr);

    proxy->OnProfilingResult(0, 9, "/data/local/tmp/proxy.htrace");

    EXPECT_EQ(callback->pid_, 0);
    EXPECT_EQ(callback->errorCode_, 9);
    EXPECT_EQ(callback->filePath_, "/data/local/tmp/proxy.htrace");
}

/**
 * @tc.name: IsNativeDeamonRunningTest
 * @tc.desc: have not process
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, IsNativeDeamonRunningTest, TestSize.Level2)
{
    EXPECT_FALSE(NativeMemoryProfilerSaService::IsNativeDeamonRunning());
}

/**
 * @tc.name: OnStopClearsStartedState
 * @tc.desc: OnStop clears started state from both stopped and started states.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, OnStopClearsStartedState, TestSize.Level2)
{
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
 * @tc.name: OnStartSkipsWhenAlreadyStarted
 * @tc.desc: On start skips when already started.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, OnStartSkipsWhenAlreadyStarted, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    service->started_ = true;
    service->OnStart();
    EXPECT_TRUE(service->started_);
    service->started_ = false;
}

/**
 * @tc.name: DelayedShutdownCancelHandlesMissingTimer
 * @tc.desc: Delayed shutdown cancel handles missing timer.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, DelayedShutdownCancelHandlesMissingTimer, TestSize.Level2)
{
    EXPECT_NO_FATAL_FAILURE(NativeMemoryProfilerSaService::DelayedShutdown(true));
}

/**
 * @tc.name: StopHookWithoutStartedTaskReturnsNotStarted
 * @tc.desc: StopHook returns not-started error for unknown pid.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StopHookWithoutStartedTaskReturnsNotStarted, TestSize.Level2)
{
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
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = static_cast<int32_t>(OHOS::IPCSkeleton::GetCallingPid()) + 1;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->StartPidVoid(config, nullptr),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PERMISSION_DENIED));
}

/**
 * @tc.name: StartPidVoidNmdModeReturnsOkForCallerPid
 * @tc.desc: Start PID void NMD mode returns ok for caller PID.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartPidVoidNmdModeReturnsOkForCallerPid, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = static_cast<int32_t>(OHOS::IPCSkeleton::GetCallingPid());
    config->printNmd_ = true;
    config->printNmdOnly_ = false;
    config->nmdPid_ = static_cast<uint32_t>(config->pid_);
    config->typeFileName_ = "start_pid_void_nmd";

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->StartPidVoid(config, nullptr), RET_OK);
}

/**
 * @tc.name: StartPidVoidRejectsUnsafeTypeFileName
 * @tc.desc: Start PID void rejects unsafe type file name.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartPidVoidRejectsUnsafeTypeFileName, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    const std::vector<std::string> invalidNames = {"", ".", "../bad", "bad/name"};
    for (const auto& name : invalidNames) {
        auto config = MakeSaConfig();
        ASSERT_NE(config, nullptr);
        config->pid_ = static_cast<int32_t>(OHOS::IPCSkeleton::GetCallingPid());
        config->typeFileName_ = name;
        EXPECT_EQ(service->StartPidVoid(config, nullptr), RET_ERR);
    }
}

/**
 * @tc.name: StartAndDumpDataNmdModesReturnOk
 * @tc.desc: Start and DumpData accept printNmd mode and return RET_OK.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartAndDumpDataNmdModesReturnOk, TestSize.Level2)
{
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
    EXPECT_EQ(service->Stop(startConfig->processName_), RET_OK);
}

/**
 * @tc.name: StartWithReplyNmdModeReturnsOk
 * @tc.desc: Start with reply accepts printNmd mode and returns RET_OK.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartWithReplyNmdModeReturnsOk, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->printNmd_ = true;
    config->printNmdOnly_ = false;
    config->nmdPid_ = static_cast<uint32_t>(getpid());
    OHOS::MessageParcel reply;

    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->Start(config, reply), RET_OK);
    EXPECT_EQ(service->Stop(config->processName_), RET_OK);
}

/**
 * @tc.name: CheckConfigAcceptsExtendedProcessAndFd
 * @tc.desc: Check config accepts extended process and FD.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigAcceptsExtendedProcessAndFd, TestSize.Level2)
{
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->extendProcesses_ = true;
    config->filePath_.clear();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_EQ(service->CheckConfig(config, STDOUT_FILENO), RET_OK);
}

/**
 * @tc.name: PidAndNameConflictAllowsProcessNameOnly
 * @tc.desc: PID and name conflict allows process name only.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, PidAndNameConflictAllowsProcessNameOnly, TestSize.Level2)
{
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
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = TEST_DURATION;
    config->processName_ = "pid_preferred_name";

    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(config), RET_OK);
    EXPECT_TRUE(config->processName_.empty());
}

/**
 * @tc.name: CheckConfigReturnsOverlimitForApiAndHiview
 * @tc.desc: CheckConfig returns overlimit for API and hiview tasks when taskMaxNum is exhausted.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigReturnsOverlimitForApiAndHiview, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    service->taskMaxNum_ = 0;

    auto apiConfig = MakeSaConfig();
    ASSERT_NE(apiConfig, nullptr);
    apiConfig->pid_ = 1234;
    apiConfig->startType_ = TEST_START_TYPE_API;
    EXPECT_EQ(service->CheckConfig(apiConfig, STDOUT_FILENO),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_PROCESS_OVERLIMIT));

    auto hiviewConfig = MakeSaConfig();
    ASSERT_NE(hiviewConfig, nullptr);
    hiviewConfig->pid_ = 1234;
    hiviewConfig->startType_ = TEST_START_TYPE_HIVIEW;
    EXPECT_EQ(service->CheckConfig(hiviewConfig, STDOUT_FILENO), RET_ERR);
    service->taskMaxNum_ = 4;
}

/**
 * @tc.name: CheckConfigRejectsStartupModeConflict
 * @tc.desc: CheckConfig rejects a second startup-mode task when one already exists.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigRejectsStartupModeConflict, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    auto startConfig = MakeSaConfig();
    ASSERT_NE(startConfig, nullptr);
    startConfig->pid_ = 0;
    startConfig->processName_ = "startup_mode_conflict";
    startConfig->startupMode_ = true;
    startConfig->startType_ = TEST_START_TYPE_HIVIEW;
    startConfig->filePath_ = MakeTracePath("startup_mode_conflict");
    ASSERT_EQ(service->Start(startConfig), RET_OK);
    sleep(1);

    auto conflictConfig = MakeSaConfig();
    ASSERT_NE(conflictConfig, nullptr);
    conflictConfig->pid_ = 0;
    conflictConfig->processName_ = "startup_mode_conflict_2";
    conflictConfig->startupMode_ = true;
    conflictConfig->filePath_ = MakeTracePath("startup_mode_conflict_2");
    EXPECT_EQ(service->CheckConfig(conflictConfig, STDOUT_FILENO), RET_ERR);
    EXPECT_EQ(service->Stop(startConfig->processName_), RET_OK);
}

/**
 * @tc.name: ReceiveCacheFileMsgRejectsNullBuffer
 * @tc.desc: ReceiveCacheFileMsg rejects nullptr buffer.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ReceiveCacheFileMsgRejectsNullBuffer, TestSize.Level2)
{
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
 * @tc.name: ProtocolProcRejectsMissingCmdline
 * @tc.desc: Protocol proc rejects missing cmdline.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcRejectsMissingCmdline, TestSize.Level2)
{
    int peerConfig = 987654321;
    SocketContext context;
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    EXPECT_FALSE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
}

/**
 * @tc.name: ProtocolProcSkipsMemLeakWhenDisabled
 * @tc.desc: Protocol proc skips mem leak when disabled.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcSkipsMemLeakWhenDisabled, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);

    int peerConfig = 0;
    SocketContext context;
    EXPECT_FALSE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
}

/**
 * @tc.name: ProtocolProcRejectsRegisteredPidWithInvalidSocket
 * @tc.desc: Protocol proc rejects registered PID with invalid socket.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcRejectsRegisteredPidWithInvalidSocket, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = GetProtocolProcessName();
    ASSERT_FALSE(config->processName_.empty());
    config->filePath_ = MakeTracePath("protocol_invalid_socket");
    ASSERT_EQ(service->Start(config), RET_OK);

    int peerConfig = static_cast<int>(getpid());
    SocketContext context;
    EXPECT_FALSE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(peerConfig)), RET_OK);
}

/**
 * @tc.name: ProtocolProcStartupModeReachesSendConfigFailure
 * @tc.desc: Protocol proc startup mode reaches send config failure.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcStartupModeReachesSendConfigFailure, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = GetProtocolProcessName();
    ASSERT_FALSE(config->processName_.empty());
    config->startupMode_ = true;
    config->filePath_ = MakeTracePath("protocol_startup_socket");
    service->Stop(config->processName_);
    ASSERT_EQ(service->Start(config), RET_OK);

    int peerConfig = static_cast<int>(getpid());
    TestSocketContext context;
    context.SetSocketHandleForTest(0);
    EXPECT_FALSE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
    int memLeakDone = static_cast<int>(MEM_LEAK_DONE_CMD);
    EXPECT_TRUE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&memLeakDone), sizeof(memLeakDone)));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(peerConfig)), RET_OK);
    EXPECT_TRUE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&memLeakDone), sizeof(memLeakDone)));
}

/**
 * @tc.name: ProtocolProcStartupModeSendsSharedMemoryFds
 * @tc.desc: Protocol proc startup mode sends shared memory fds.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcStartupModeSendsSharedMemoryFds, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = GetProtocolProcessName();
    ASSERT_FALSE(config->processName_.empty());
    config->startupMode_ = true;
    config->offlineSymbolization_ = true;
    config->filePath_ = MakeTracePath("protocol_startup_shared_memory");
    service->Stop(config->processName_);
    ASSERT_EQ(service->Start(config), RET_OK);

    ScopedSocketPair socketPair;
    ASSERT_TRUE(socketPair.IsValid());
    int peerConfig = static_cast<int>(getpid());
    TestSocketContext context;
    context.SetSocketHandleForTest(socketPair.First());

    EXPECT_TRUE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
    ASSERT_TRUE(ResetRegisteredSocketFdForStop(*service, context, peerConfig, "protocol_reset_shared_memory"));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(peerConfig)), RET_OK);
}

/**
 * @tc.name: ProtocolProcNonStartupModeSendsConfigForCallerCredentials
 * @tc.desc: Protocol proc non startup mode sends config for caller credentials.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcNonStartupModeSendsConfigForCallerCredentials, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = static_cast<uint32_t>(getpid());
    config->processName_.clear();
    config->startupMode_ = false;
    config->filePath_ = MakeTracePath("protocol_non_startup_socket");
    service->Stop(GetProtocolProcessName());
    ASSERT_EQ(service->Start(config), RET_OK);

    ScopedSocketPair socketPair;
    ASSERT_TRUE(socketPair.IsValid());
    int peerConfig = static_cast<int>(getpid());
    TestSocketContext context;
    context.SetSocketHandleForTest(socketPair.First());

    EXPECT_TRUE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
    ASSERT_TRUE(ResetRegisteredSocketFdForStop(*service, context, peerConfig, "protocol_reset_non_startup"));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(peerConfig)), RET_OK);
}

/**
 * @tc.name: ProtocolProcStartupModeSendsAsyncUniqueStackFd
 * @tc.desc: Protocol proc startup mode sends async unique stack FD.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcStartupModeSendsAsyncUniqueStackFd, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = GetProtocolProcessName();
    ASSERT_FALSE(config->processName_.empty());
    config->startupMode_ = true;
    config->asyncStackEnable_ = true;
    config->filePath_ = MakeTracePath("protocol_startup_async_stack");
    service->Stop(config->processName_);
    ASSERT_EQ(service->Start(config), RET_OK);

    ScopedSocketPair socketPair;
    ASSERT_TRUE(socketPair.IsValid());
    int peerConfig = static_cast<int>(getpid());
    TestSocketContext context;
    context.SetSocketHandleForTest(socketPair.First());

    EXPECT_TRUE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
    ASSERT_TRUE(ResetRegisteredSocketFdForStop(*service, context, peerConfig, "protocol_reset_async_stack"));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(peerConfig)), RET_OK);
}

/**
 * @tc.name: ProtocolProcStartupModeSendsInitialFileCacheFds
 * @tc.desc: Protocol proc startup mode sends initial file cache fds.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ProtocolProcStartupModeSendsInitialFileCacheFds, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = GetProtocolProcessName();
    ASSERT_FALSE(config->processName_.empty());
    config->startupMode_ = true;
    config->enableFileCacheMode_ = true;
    config->filePath_ = MakeTracePath("protocol_startup_file_cache");
    service->Stop(config->processName_);
    ASSERT_EQ(service->Start(config), RET_OK);

    ScopedSocketPair socketPair;
    ASSERT_TRUE(socketPair.IsValid());
    int peerConfig = static_cast<int>(getpid());
    TestSocketContext context;
    context.SetSocketHandleForTest(socketPair.First());

    EXPECT_TRUE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
    HookCacheFileMsg cacheStopMsg(peerConfig, 0, CACHE_FILE_STOP);
    EXPECT_TRUE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&cacheStopMsg),
        sizeof(cacheStopMsg)));
    ASSERT_TRUE(ResetRegisteredSocketFdForStop(*service, context, peerConfig, "protocol_reset_file_cache"));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(peerConfig)), RET_OK);
}

/**
 * @tc.name: ClientDisconnectCallbackStopsSocketMappedTask
 * @tc.desc: ClientDisconnectCallback handles disconnect for socket-mapped task.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ClientDisconnectCallbackStopsSocketMappedTask, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = GetProtocolProcessName();
    ASSERT_FALSE(config->processName_.empty());
    config->startupMode_ = true;
    config->filePath_ = MakeTracePath("client_disconnect_socket");
    service->Stop(config->processName_);
    ASSERT_EQ(service->Start(config), RET_OK);

    int peerConfig = static_cast<int>(getpid());
    TestSocketContext context;
    context.SetSocketHandleForTest(0);
    EXPECT_FALSE(service->ProtocolProc(context, 0, reinterpret_cast<const int8_t*>(&peerConfig), sizeof(peerConfig)));
    EXPECT_NO_FATAL_FAILURE(NativeMemoryProfilerSaService::ClientDisconnectCallback(0));
    EXPECT_EQ(service->Stop(config->processName_), RET_OK);
}

/**
 * @tc.name: ReceiveCacheFileMsgRejectsDisabledFileCacheMode
 * @tc.desc: Receive cache file msg rejects disabled file cache mode.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ReceiveCacheFileMsgRejectsDisabledFileCacheMode, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "cache_disabled_task_" + std::to_string(getpid());
    config->filePath_ = MakeTracePath("cache_disabled_task");
    ASSERT_EQ(service->Start(config), RET_OK);

    constexpr int32_t promotedPid = 24683;
    service->FillTaskConfigContext(promotedPid, config->processName_);
    HookCacheFileMsg cacheFileMsg(promotedPid, 0, CACHE_FILE_READY);
    SocketContext context;
    EXPECT_FALSE(service->ReceiveCacheFileMsg(context, reinterpret_cast<const int8_t*>(&cacheFileMsg)));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(promotedPid)), RET_OK);
}

/**
 * @tc.name: ReceiveCacheFileMsgRejectsReadyWithoutRegisteredContext
 * @tc.desc: Receive cache file msg rejects ready without registered context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ReceiveCacheFileMsgRejectsReadyWithoutRegisteredContext, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    TestSocketContext context;
    context.SetSocketHandleForTest(-1);
    HookCacheFileMsg readyMsg(24684, 0, CACHE_FILE_READY);
    EXPECT_FALSE(service->ReceiveCacheFileMsg(context, reinterpret_cast<const int8_t*>(&readyMsg)));
}

/**
 * @tc.name: ReceiveCacheFileMsgReadyQueuesAndReplenishesFd
 * @tc.desc: Receive cache file msg ready queues and replenishes FD.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ReceiveCacheFileMsgReadyQueuesAndReplenishesFd, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = GetProtocolProcessName();
    ASSERT_FALSE(config->processName_.empty());
    config->startupMode_ = true;
    config->enableFileCacheMode_ = true;
    config->filePath_ = MakeTracePath("cache_ready_replenish");
    service->Stop(config->processName_);
    ASSERT_EQ(service->Start(config), RET_OK);

    ScopedSocketPair socketPair;
    ASSERT_TRUE(socketPair.IsValid());
    TestSocketContext serviceContext;
    TestSocketContext peerContext;
    serviceContext.SetSocketHandleForTest(socketPair.First());
    peerContext.SetSocketHandleForTest(socketPair.Second());
    int peerConfig = static_cast<int>(getpid());
    ASSERT_TRUE(service->ProtocolProc(serviceContext, 0, reinterpret_cast<const int8_t*>(&peerConfig),
        sizeof(peerConfig)));
    ProtocolHead clientHead = {};
    ASSERT_EQ(read(socketPair.Second(), &clientHead, sizeof(clientHead)), static_cast<ssize_t>(sizeof(clientHead)));
    std::vector<uint8_t> clientConfig(clientHead.protoSize - sizeof(clientHead));
    ASSERT_EQ(read(socketPair.Second(), clientConfig.data(), clientConfig.size()),
        static_cast<ssize_t>(clientConfig.size()));
    for (uint16_t i = 0; i < FILE_CACHE_FD_COUNT; ++i) {
        int fd = peerContext.ReceiveFileDiscriptor();
        ASSERT_GE(fd, 0);
        close(fd);
    }

    ExpectCacheFileReadyReplenishesFd(*service, serviceContext, peerContext, peerConfig, socketPair.Second());
    ASSERT_TRUE(ResetRegisteredSocketFdForStop(*service, serviceContext, peerConfig, "protocol_reset_ready"));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(peerConfig)), RET_OK);
}

/**
 * @tc.name: StartHookReplySimpNmdTimesOutAndStops
 * @tc.desc: Start hook reply simp NMD times out and stops.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartHookReplySimpNmdTimesOutAndStops, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "simp_nmd_timeout_" + std::to_string(getpid());
    config->filePath_ = MakeTracePath("simp_nmd_timeout");
    config->duration_ = TEST_DURATION;
    config->nmdType_ = 3;
    OHOS::MessageParcel reply;

    EXPECT_EQ(service->StartHook(config, 0, reply), RET_ERR);
    EXPECT_EQ(service->Stop(config->processName_), RET_OK);
}

/**
 * @tc.name: NotifyProfilingResultUsesCallbackWhenPresent
 * @tc.desc: Notify profiling result uses callback when present.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NotifyProfilingResultUsesCallbackWhenPresent, TestSize.Level2)
{
    NativeMemoryProfilerSaService::NotifyProfilingResult(nullptr, RET_ERR, "");
    auto task = std::make_shared<TaskConfig>(nullptr, 0, "notify_task", -1, "", -1, false);
    ASSERT_NE(task, nullptr);
    NativeMemoryProfilerSaService::NotifyProfilingResult(task, RET_ERR, "");

    OHOS::sptr<TestNativeMemoryProfilerSaCallbackStub> callback =
        new (std::nothrow) TestNativeMemoryProfilerSaCallbackStub();
    ASSERT_NE(callback, nullptr);
    task->callback = callback->AsObject();
    NativeMemoryProfilerSaService::NotifyProfilingResult(task, RET_OK, "/data/local/tmp/notify.htrace");

    EXPECT_EQ(callback->errorCode_, RET_OK);
    EXPECT_EQ(callback->filePath_, "/data/local/tmp/notify.htrace");
}

/**
 * @tc.name: CopyApiCollectorFileReturnsOkWhenInfoMissing
 * @tc.desc: Copy API collector file returns ok when info missing.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CopyApiCollectorFileReturnsOkWhenInfoMissing, TestSize.Level2)
{
    auto task = std::make_shared<TaskConfig>(nullptr, 12345, "copy_missing", -1, "", -1, false);
    ASSERT_NE(task, nullptr);
    std::string resultPath = "unchanged";

    EXPECT_EQ(NativeMemoryProfilerSaService::CopyApiCollectorFile(12345, task, resultPath), RET_OK);
    EXPECT_EQ(resultPath, "unchanged");
}

/**
 * @tc.name: CopyApiCollectorFileRejectsMissingPidInfo
 * @tc.desc: Copy API collector file returns err when pid is not registered.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CopyApiCollectorFileRejectsMissingPidInfo, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    constexpr uint32_t pid = 24688;
    auto task = std::make_shared<TaskConfig>(nullptr, static_cast<int32_t>(pid), "copy_no_pid", -1, "", -1, false);
    ASSERT_NE(task, nullptr);
    task->typeFileName = "no_pid.htrace";
    std::string resultPath = "unchanged";

    EXPECT_EQ(NativeMemoryProfilerSaService::CopyApiCollectorFile(pid, task, resultPath), RET_ERR);
    EXPECT_EQ(resultPath, "unchanged");
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: CopyApiCollectorFileRejectsUnsafeTypeFileName
 * @tc.desc: Copy API collector file returns err when typeFileName has traversal or slash.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CopyApiCollectorFileRejectsUnsafeTypeFileName, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    constexpr uint32_t pid = 24689;
    NativeMemoryProfilerSaService::InjectPidInfoForTest(static_cast<pid_t>(pid), 200000, 0);
    std::string resultPath = "unchanged";

    auto dotTask = std::make_shared<TaskConfig>(nullptr, static_cast<int32_t>(pid), "copy_unsafe", -1, "", -1, false);
    ASSERT_NE(dotTask, nullptr);
    dotTask->typeFileName = "../evil.htrace";
    EXPECT_EQ(NativeMemoryProfilerSaService::CopyApiCollectorFile(pid, dotTask, resultPath), RET_ERR);
    EXPECT_EQ(resultPath, "unchanged");

    auto slashTask = std::make_shared<TaskConfig>(nullptr, static_cast<int32_t>(pid), "copy_unsafe", -1, "", -1, false);
    ASSERT_NE(slashTask, nullptr);
    slashTask->typeFileName = "sub/dir.htrace";
    EXPECT_EQ(NativeMemoryProfilerSaService::CopyApiCollectorFile(pid, slashTask, resultPath), RET_ERR);
    EXPECT_EQ(resultPath, "unchanged");

    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: CopyApiCollectorFileRejectsCopyFailure
 * @tc.desc: Copy API collector file returns err when dest path is not writable.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CopyApiCollectorFileRejectsCopyFailure, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    constexpr uint32_t pid = 24690;
    NativeMemoryProfilerSaService::InjectPidInfoForTest(static_cast<pid_t>(pid), 200000, 0);
    auto task = std::make_shared<TaskConfig>(nullptr, static_cast<int32_t>(pid),
        "copy_dest_missing", -1, "", -1, false, STDOUT_FILENO);
    ASSERT_NE(task, nullptr);
    task->typeFileName = "copy_fail.htrace";
    std::string resultPath = "unchanged";

    EXPECT_EQ(NativeMemoryProfilerSaService::CopyApiCollectorFile(pid, task, resultPath), RET_ERR);
    EXPECT_EQ(resultPath, "unchanged");
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: CopyApiCollectorFileRejectsCompositeHeapCollectFailure
 * @tc.desc: Copy API collector file returns err when CollectApm fails for composite heap.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CopyApiCollectorFileRejectsCompositeHeapCollectFailure, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    constexpr uint32_t pid = 24691;
    NativeMemoryProfilerSaService::InjectPidInfoForTest(static_cast<pid_t>(pid), 200000, 0);
    auto task = std::make_shared<TaskConfig>(nullptr, static_cast<int32_t>(pid),
        "copy_composite", -1, "", -1, false);
    ASSERT_NE(task, nullptr);
    task->typeFileName = "composite.htrace";
    task->compositeHeapEnable = true;
    std::string resultPath = "unchanged";

    EXPECT_EQ(NativeMemoryProfilerSaService::CopyApiCollectorFile(pid, task, resultPath), RET_ERR);
    EXPECT_EQ(resultPath, "unchanged");
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
}

/**
 * @tc.name: CopyApiCollectorFileSucceedsForApiTask
 * @tc.desc: Copy API collector file returns ok and sets sandbox path on success.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CopyApiCollectorFileSucceedsForApiTask, TestSize.Level2)
{
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    constexpr uint32_t pid = 24692;
    NativeMemoryProfilerSaService::InjectPidInfoForTest(static_cast<pid_t>(pid), 200000, 0);
    std::string processName = "api_copy_success";
    std::string typeFileName = "api_success.htrace";
    std::string destDir = "/data/app/el2/1/base/" + processName + "/files";
    system(("mkdir -p " + destDir).c_str());
    std::string probePath = destDir + "/.probe";
    int probeFd = open(probePath.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (probeFd < 0) {
        GTEST_SKIP() << "app sandbox dir not writable";
    }
    close(probeFd);
    unlink(probePath.c_str());

    std::string srcPath = "/data/local/tmp/api_copy_success_src.htrace";
    int srcFd = open(srcPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(srcFd, 0);
    std::string srcData = "API_SOURCE";
    ASSERT_EQ(write(srcFd, srcData.data(), srcData.size()), static_cast<ssize_t>(srcData.size()));
    (void)fsync(srcFd);

    auto task = std::make_shared<TaskConfig>(nullptr, static_cast<int32_t>(pid),
        processName, -1, "", -1, false, static_cast<uint32_t>(srcFd), TEST_START_TYPE_API);
    ASSERT_NE(task, nullptr);
    task->typeFileName = typeFileName;
    std::string resultPath = "unchanged";

    EXPECT_EQ(NativeMemoryProfilerSaService::CopyApiCollectorFile(pid, task, resultPath), RET_OK);
    EXPECT_EQ(resultPath, std::string("/data/storage/el2/base/files/") + typeFileName);

    close(srcFd);
    unlink(srcPath.c_str());
    unlink((destDir + "/" + typeFileName).c_str());
    NativeMemoryProfilerSaService::ResetTaskContextsForTest();
    system(("rm -rf /data/app/el2/1/base/" + processName).c_str());
}

/**
 * @tc.name: StopHookByTypeRoutesMissingTask
 * @tc.desc: Stop hook by type routes missing task.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StopHookByTypeRoutesMissingTask, TestSize.Level2)
{
    std::string name = "missing_stop_task";

    EXPECT_EQ(NativeMemoryProfilerSaService::StopHookByType(0, name, RET_OK, false),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED));
    EXPECT_EQ(NativeMemoryProfilerSaService::StopHookByType(987654, name, RET_OK, true),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED));
}

/**
 * @tc.name: CheckConfigRejectsFilePathAlreadyUsed
 * @tc.desc: Check config rejects file path already used.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigRejectsFilePathAlreadyUsed, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    std::string tracePath = MakeTracePath("file_path_conflict");
    auto startedConfig = MakeSaConfig();
    ASSERT_NE(startedConfig, nullptr);
    startedConfig->pid_ = 0;
    startedConfig->filePath_ = tracePath;
    startedConfig->processName_ = tracePath;
    ASSERT_EQ(service->Start(startedConfig), RET_OK);

    auto conflictConfig = MakeSaConfig();
    ASSERT_NE(conflictConfig, nullptr);
    conflictConfig->pid_ = 0;
    conflictConfig->processName_ = "file_path_conflict_new";
    conflictConfig->filePath_ = tracePath;
    EXPECT_EQ(service->CheckConfig(conflictConfig), RET_ERR);
    EXPECT_EQ(service->Stop(startedConfig->processName_), RET_OK);
}

/**
 * @tc.name: FillTaskConfigContextPromotesNameTaskToPid
 * @tc.desc: Fill task config context promotes name task to PID.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, FillTaskConfigContextPromotesNameTaskToPid, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "fill_context_name_task";
    config->filePath_ = MakeTracePath("fill_context_name_task");
    ASSERT_EQ(service->Start(config), RET_OK);

    constexpr int32_t promotedPid = 24680;
    service->FillTaskConfigContext(promotedPid, config->processName_);

    EXPECT_EQ(service->Stop(static_cast<uint32_t>(promotedPid)), RET_OK);
}

/**
 * @tc.name: PidAndNameConflictReportsStartedPidByStartType
 * @tc.desc: PID and name conflict reports started PID by start type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, PidAndNameConflictReportsStartedPidByStartType, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "pid_conflict_name_task";
    config->filePath_ = MakeTracePath("pid_conflict_name_task");
    config->startType_ = TEST_START_TYPE_HIVIEW;
    ASSERT_EQ(service->Start(config), RET_OK);
    constexpr int32_t promotedPid = 24681;
    service->FillTaskConfigContext(promotedPid, config->processName_);

    auto sameTypeConfig = MakeSaConfig();
    ASSERT_NE(sameTypeConfig, nullptr);
    sameTypeConfig->pid_ = promotedPid;
    sameTypeConfig->processName_ = "ignored_name";
    sameTypeConfig->startType_ = TEST_START_TYPE_HIVIEW;
    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(sameTypeConfig),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_ALREADY_STARTED));
    EXPECT_TRUE(sameTypeConfig->processName_.empty());

    auto conflictConfig = MakeSaConfig();
    ASSERT_NE(conflictConfig, nullptr);
    conflictConfig->pid_ = promotedPid;
    conflictConfig->startType_ = TEST_START_TYPE_API;
    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(conflictConfig),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT));
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(promotedPid)), RET_OK);
}

/**
 * @tc.name: CheckConfigMapsPidConflictForHiview
 * @tc.desc: Check config maps PID conflict for hiview.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigMapsPidConflictForHiview, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "check_config_pid_conflict";
    config->filePath_ = MakeTracePath("check_config_pid_conflict");
    config->startType_ = TEST_START_TYPE_HIVIEW;
    ASSERT_EQ(service->Start(config), RET_OK);
    constexpr int32_t promotedPid = 24682;
    service->FillTaskConfigContext(promotedPid, config->processName_);

    auto apiConfig = MakeSaConfig();
    ASSERT_NE(apiConfig, nullptr);
    apiConfig->pid_ = promotedPid;
    apiConfig->startType_ = TEST_START_TYPE_API;
    EXPECT_EQ(service->CheckConfig(apiConfig, STDOUT_FILENO),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_CONFLICT));

    auto hiviewConfig = MakeSaConfig();
    ASSERT_NE(hiviewConfig, nullptr);
    hiviewConfig->pid_ = promotedPid;
    hiviewConfig->startType_ = TEST_START_TYPE_HIVIEW;
    EXPECT_EQ(service->CheckConfig(hiviewConfig, STDOUT_FILENO), RET_ERR);
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(promotedPid)), RET_OK);
}

/**
 * @tc.name: DisconnectAndCloseSkipMissingContext
 * @tc.desc: Disconnect and close skip missing context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, DisconnectAndCloseSkipMissingContext, TestSize.Level2)
{
    EXPECT_NO_FATAL_FAILURE(NativeMemoryProfilerSaService::ClientDisconnectCallback(-1));
    EXPECT_NO_FATAL_FAILURE(NativeMemoryProfilerSaService::CloseSocketFd(987654));
}

/**
 * @tc.name: ReleaseForStopHookHandlesAsyncApiFileCache
 * @tc.desc: Release for stop hook handles async API file cache.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ReleaseForStopHookHandlesAsyncApiFileCache, TestSize.Level2)
{
    auto hook = std::make_shared<HookManager>();
    ASSERT_NE(hook, nullptr);
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    hook->SetHookConfig(hookConfig);
    int fd = open(MakeTracePath("release_async_api").c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    constexpr uint32_t pid = 24684;
    auto task = std::make_shared<TaskConfig>(hook, pid, "release_async_api", -1, "", -1, true, fd, TEST_START_TYPE_API);
    ASSERT_NE(task, nullptr);
    std::string resultPath = "unchanged";

    EXPECT_EQ(NativeMemoryProfilerSaService::ReleaseForStopHook(pid, task, resultPath), RET_OK);
    EXPECT_EQ(resultPath, "unchanged");
    EXPECT_EQ(fcntl(fd, F_GETFD), -1);
}

/**
 * @tc.name: ReleaseForStopHookHandlesSyncApiBranch
 * @tc.desc: Release for stop hook handles sync API branch.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, ReleaseForStopHookHandlesSyncApiBranch, TestSize.Level2)
{
    auto hook = std::make_shared<HookManager>();
    ASSERT_NE(hook, nullptr);
    NativeHookConfig hookConfig;
    hook->SetHookConfig(hookConfig);
    constexpr uint32_t pid = 24685;
    auto task = std::make_shared<TaskConfig>(hook, pid, "release_sync_api", -1, "", -1, false, 0, TEST_START_TYPE_API);
    ASSERT_NE(task, nullptr);
    std::string resultPath = "unchanged";

    EXPECT_EQ(NativeMemoryProfilerSaService::ReleaseForStopHook(pid, task, resultPath), RET_OK);
    EXPECT_EQ(resultPath, "unchanged");
}

/**
 * @tc.name: GetTaskNumCountsPromotedHiviewTask
 * @tc.desc: Get task num counts promoted hiview task.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, GetTaskNumCountsPromotedHiviewTask, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "get_task_num_hiview_task";
    config->filePath_ = MakeTracePath("get_task_num_hiview_task");
    config->startType_ = TEST_START_TYPE_HIVIEW;
    ASSERT_EQ(service->Start(config), RET_OK);
    constexpr int32_t promotedPid = 24686;
    service->FillTaskConfigContext(promotedPid, config->processName_);

    auto queryConfig = MakeSaConfig();
    ASSERT_NE(queryConfig, nullptr);
    queryConfig->startType_ = TEST_START_TYPE_HIVIEW;
    EXPECT_GE(NativeMemoryProfilerSaService::GetTaskNum(queryConfig), 1);
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(promotedPid)), RET_OK);
}

/**
 * @tc.name: FillTaskConfigContextHandlesStartupPidContext
 * @tc.desc: Fill task config context handles startup PID context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, FillTaskConfigContextHandlesStartupPidContext, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "startup_pid_context";
    config->filePath_ = MakeTracePath("startup_pid_context");
    config->startupMode_ = true;
    config->startType_ = TEST_START_TYPE_HIVIEW;
    ASSERT_EQ(service->Start(config), RET_OK);
    constexpr int32_t promotedPid = 24687;
    service->FillTaskConfigContext(promotedPid, config->processName_);
    service->FillTaskConfigContext(promotedPid, config->processName_);

    auto nextStartup = MakeSaConfig();
    ASSERT_NE(nextStartup, nullptr);
    nextStartup->pid_ = 0;
    nextStartup->processName_ = "startup_pid_context_next";
    nextStartup->startupMode_ = true;
    EXPECT_EQ(service->CheckConfig(nextStartup, STDOUT_FILENO), RET_OK);
    EXPECT_EQ(service->Stop(static_cast<uint32_t>(promotedPid)), RET_OK);
}

/**
 * @tc.name: PidAndNameConflictRejectsActiveProcessName
 * @tc.desc: PID and name conflict rejects active process name.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, PidAndNameConflictRejectsActiveProcessName, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "active_name_conflict_task";
    config->filePath_ = MakeTracePath("active_name_conflict_task");
    ASSERT_EQ(service->Start(config), RET_OK);

    auto conflictConfig = MakeSaConfig();
    ASSERT_NE(conflictConfig, nullptr);
    conflictConfig->pid_ = 0;
    conflictConfig->processName_ = config->processName_;

    EXPECT_EQ(NativeMemoryProfilerSaService::PidAndNameConflict(conflictConfig), RET_ERR);
    EXPECT_EQ(service->Stop(config->processName_), RET_OK);
}

/**
 * @tc.name: CheckConfigAllowsExtendedTaskWhenLimitZero
 * @tc.desc: Check config allows extended task when limit zero.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigAllowsExtendedTaskWhenLimitZero, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    service->taskMaxNum_ = 0;
    config->extendProcesses_ = true;
    config->pid_ = 0;
    config->processName_ = "extended_limit_zero_task";

    EXPECT_EQ(service->CheckConfig(config, STDOUT_FILENO), RET_OK);
    service->taskMaxNum_ = 4;
}

/**
 * @tc.name: CheckConfigMapsNameConflictForHiview
 * @tc.desc: Check config maps name conflict for hiview.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CheckConfigMapsNameConflictForHiview, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto startedConfig = MakeSaConfig();
    ASSERT_NE(startedConfig, nullptr);
    startedConfig->pid_ = 0;
    startedConfig->processName_ = "hiview_name_conflict_task";
    startedConfig->filePath_ = MakeTracePath("hiview_name_conflict_task");
    ASSERT_EQ(service->Start(startedConfig), RET_OK);

    auto conflictConfig = MakeSaConfig();
    ASSERT_NE(conflictConfig, nullptr);
    conflictConfig->pid_ = 0;
    conflictConfig->processName_ = startedConfig->processName_;
    conflictConfig->startType_ = TEST_START_TYPE_HIVIEW;

    EXPECT_EQ(service->CheckConfig(conflictConfig, STDOUT_FILENO), RET_ERR);
    EXPECT_EQ(service->Stop(startedConfig->processName_), RET_OK);
}

/**
 * @tc.name: FillTaskConfigContextIgnoresMissingContext
 * @tc.desc: Fill task config context ignores missing context.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, FillTaskConfigContextIgnoresMissingContext, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    constexpr int32_t missingPid = 28670;
    std::string missingName = "fill_context_missing_task";

    service->FillTaskConfigContext(missingPid, missingName);

    EXPECT_EQ(NativeMemoryProfilerSaService::StopHook(missingPid, ""),
        static_cast<int32_t>(ResProfilerErrorCode::RES_PROF_NOT_STARTED));
}

/**
 * @tc.name: StartHookCreatesDefaultFilePathWhenMissing
 * @tc.desc: Start hook creates default file path when missing.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartHookCreatesDefaultFilePathWhenMissing, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "default_file_path_task_" + std::to_string(getpid());
    config->filePath_.clear();

    ASSERT_EQ(service->Start(config), RET_OK);
    EXPECT_NE(config->filePath_.find("/data/local/tmp/native_memory_" + config->processName_), std::string::npos);
    EXPECT_EQ(service->Stop(config->processName_), RET_OK);
}

/**
 * @tc.name: StartHookLockHandlesPrintNmdOnlyWithInputFd
 * @tc.desc: Start hook lock handles print NMD only with input FD.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, StartHookLockHandlesPrintNmdOnlyWithInputFd, TestSize.Level2)
{
    auto service = OHOS::DelayedSingleton<NativeMemoryProfilerSaService>::GetInstance();
    ASSERT_NE(service, nullptr);
    auto config = MakeSaConfig();
    ASSERT_NE(config, nullptr);
    config->pid_ = 0;
    config->processName_ = "print_nmd_only_task_" + std::to_string(getpid());
    config->filePath_.clear();
    config->printNmdOnly_ = true;
    uint32_t fd = static_cast<uint32_t>(open(MakeTracePath("print_nmd_only_fd").c_str(),
        O_CREAT | O_RDWR, S_IRUSR | S_IWUSR));
    ASSERT_GT(fd, 0u);
    auto hook = std::make_shared<HookManager>();
    ASSERT_NE(hook, nullptr);
    std::string args = service->GetCmdArgs(config);

    int32_t ret = service->StartHookLock(config, fd, hook, args);

    if (ret == RET_OK) {
        EXPECT_EQ(service->Stop(config->processName_), RET_OK);
    } else {
        close(static_cast<int>(fd));
    }
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: CollectApmReturnsEmptyForNonAppPid
 * @tc.desc: Collect APM returns empty for non app pid.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CollectApmReturnsEmptyForNonAppPid, TestSize.Level2)
{
    auto task = std::make_shared<TaskConfig>(nullptr, static_cast<int32_t>(getpid()),
        "collect_apm_pid", -1, "", -1, false);
    ASSERT_NE(task, nullptr);

    EXPECT_TRUE(NativeMemoryProfilerSaService::CollectApm(task).empty());

    std::string compositePath = "/data/local/tmp/composite_heap_" + std::to_string(getpid()) + ".zip";
    EXPECT_EQ(access(compositePath.c_str(), F_OK), -1);
}

/**
 * @tc.name: CollectApmReturnsEmptyForZeroPid
 * @tc.desc: Collect APM returns empty for zero pid using process name.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, CollectApmReturnsEmptyForZeroPid, TestSize.Level2)
{
    std::string processName = "collect_apm_name";
    auto task = std::make_shared<TaskConfig>(nullptr, 0, processName, -1, "", -1, false);
    ASSERT_NE(task, nullptr);

    EXPECT_TRUE(NativeMemoryProfilerSaService::CollectApm(task).empty());

    std::string compositePath = "/data/local/tmp/composite_heap_" + processName + ".zip";
    EXPECT_EQ(access(compositePath.c_str(), F_OK), -1);
}

