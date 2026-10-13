/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hook_service_exception_test.h"
#include "native_hook_config.pb.h"

#include <sys/socket.h>
#include <unistd.h>

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
void HookServiceDisconnectCallback(int socketFd);
extern HookService* g_hookServiceInstance;

namespace {
class TestSocketContext : public SocketContext {
public:
    void SetSocketHandle(int fd)
    {
        socketHandle_ = fd;
        fdsan_exchange_owner_tag(socketHandle_, 0, PROFILER_FD_TAG);
    }
};

void CloseFd(int fd)
{
    if (fd >= 0) {
        close(fd);
    }
}

std::shared_ptr<HookManager::HookManagerCtx> AddFileCacheContext(const std::shared_ptr<HookManager>& hookManager,
    int32_t pid, const std::string& processName)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_clock("monotonic");
    hookManager->SetHookConfig(config);
    hookManager->fileCacheDir_ = "/data/local/tmp/";
    auto ctx = std::make_shared<HookManager::HookManagerCtx>(pid);
    ctx->processName = processName;
    ctx->stackPreprocess = std::make_shared<StackPreprocess>(nullptr, hookManager->hookConfig_, CLOCK_MONOTONIC);
    hookManager->hookCtx_.push_back(ctx);
    return ctx;
}
}

void HookServiceExceptionTest::SetUpTestCase(void) {}
void HookServiceExceptionTest::TearDownTestCase(void) {}

void HookServiceExceptionTest::SetUp()
{
    hookManager_ = std::make_shared<HookManager>();
    clientConfig_ = ClientConfig();
}

void HookServiceExceptionTest::TearDown()
{
    hookService_ = nullptr;
    hookManager_ = nullptr;
}

/*
 * @tc.name: ProtocolProcWithInvalidDataSize
 * @tc.desc: test ProtocolProc with invalid data size.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithInvalidDataSize, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    int pid = 1;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, 0));
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, 1));
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, 100));
}

/*
 * @tc.name: ProtocolProcWithNullHookManager
 * @tc.desc: test ProtocolProc when hookMgr_ is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithNullHookManager, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, nullptr);
    SocketContext socketContext;
    int pid = 1;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}

/*
 * @tc.name: DisconnectCallbackSkipsWhenServiceIsMissing
 * @tc.desc: test disconnect callback returns when global service pointer is null.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, DisconnectCallbackSkipsWhenServiceIsMissing, TestSize.Level0)
{
    HookService* savedService = g_hookServiceInstance;
    g_hookServiceInstance = nullptr;
    HookServiceDisconnectCallback(-1);
    EXPECT_EQ(g_hookServiceInstance, nullptr);
    g_hookServiceInstance = savedService;
}

/*
 * @tc.name: StartServiceRejectsInvalidSocketPath
 * @tc.desc: test StartService failure path with invalid unix socket path.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, StartServiceRejectsInvalidSocketPath, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    std::string invalidPath(200, 'a');
    EXPECT_FALSE(hookService_->StartService(invalidPath));
    EXPECT_EQ(g_hookServiceInstance, nullptr);
}

/*
 * @tc.name: ProtocolProcWithInvalidPidNegative
 * @tc.desc: test ProtocolProc with negative PID (-1).
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithInvalidPidNegative, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    int pid = -1;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}

/*
 * @tc.name: RemovePidInfoWithInvalidPid
 * @tc.desc: test RemovePidInfo with invalid PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, RemovePidInfoWithInvalidPid, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    hookService_->RemovePidInfo(999999);
    hookService_->RemovePidInfo(-1);
    hookService_->RemovePidInfo(0);
    EXPECT_TRUE(hookService_->pidInfo_.empty());
}

/*
 * @tc.name: AddAndRemovePidInfo
 * @tc.desc: test AddPidInfo and RemovePidInfo.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, AddAndRemovePidInfo, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    pid_t testPid = 12345;
    uid_t testUid = 1000;
    gid_t testGid = 1000;
    hookService_->AddPidInfo(testPid, testUid, testGid);
    EXPECT_EQ(hookService_->pidInfo_.size(), 1);
    EXPECT_EQ(hookService_->pidInfo_[testPid].first, testUid);
    EXPECT_EQ(hookService_->pidInfo_[testPid].second, testGid);
    hookService_->RemovePidInfo(testPid);
    EXPECT_TRUE(hookService_->pidInfo_.empty());
}

/*
 * @tc.name: ProtocolProcWithZeroPid
 * @tc.desc: test ProtocolProc with zero PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithZeroPid, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    int pid = 0;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}

/*
 * @tc.name: ProtocolProcWithMaxPid
 * @tc.desc: test ProtocolProc with INT_MAX PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcWithMaxPid, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    int pid = INT_MAX;
    const int8_t* pidPtr = reinterpret_cast<const int8_t*>(&pid);
    EXPECT_FALSE(hookService_->ProtocolProc(socketContext, 0, pidPtr, sizeof(pid)));
}

/*
 * @tc.name: AddDuplicatePidInfo
 * @tc.desc: test AddPidInfo with duplicate PID.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, AddDuplicatePidInfo, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    pid_t testPid = 12345;
    hookService_->AddPidInfo(testPid, 1000, 1000);
    hookService_->AddPidInfo(testPid, 2000, 2000);
    EXPECT_EQ(hookService_->pidInfo_.size(), 1);
    EXPECT_EQ(hookService_->pidInfo_[testPid].first, 2000);
}

/*
 * @tc.name: SocketFdStateHandlesMissingAndInvalidFd
 * @tc.desc: test socket fd lookup, invalid fd cleanup and send command guard.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, SocketFdStateHandlesMissingAndInvalidFd, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    constexpr pid_t testPid = 12345;
    EXPECT_FALSE(hookService_->HasSocketFd(testPid));

    hookService_->pidFds_[testPid] = -1;
    EXPECT_FALSE(hookService_->HasSocketFd(testPid));
    hookService_->SendCommand(testPid, STOP_HOOK_CMD);
    hookService_->CloseSocketFd(testPid);
    EXPECT_TRUE(hookService_->pidFds_.empty());

    hookService_->CloseSocketFd(testPid);
    hookService_->SendCommand(testPid, STOP_HOOK_CMD);
}

/*
 * @tc.name: ClientDisconnectCallbackHandlesSocketMatch
 * @tc.desc: test disconnect callback with unmatched, matched and null manager paths.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ClientDisconnectCallbackHandlesSocketMatch, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    constexpr pid_t testPid = 23456;
    constexpr int socketFd = 123;
    hookService_->pidFds_[testPid] = socketFd;

    hookService_->ClientDisconnectCallback(socketFd + 1);
    EXPECT_TRUE(hookService_->HasSocketFd(testPid));
    hookService_->ClientDisconnectCallback(socketFd);
    EXPECT_TRUE(hookService_->HasSocketFd(testPid));

    auto nullMgrService = std::make_shared<HookService>(clientConfig_, nullptr);
    nullMgrService->pidFds_[testPid] = socketFd;
    nullMgrService->ClientDisconnectCallback(socketFd);
    EXPECT_TRUE(nullMgrService->HasSocketFd(testPid));
}

/*
 * @tc.name: SendCommandWritesCommandWhenSocketIsAvailable
 * @tc.desc: test SendCommand writes protocol header and command to socket.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, SendCommandWritesCommandWhenSocketIsAvailable, TestSize.Level0)
{
    int sockets[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    constexpr pid_t testPid = 34567;
    hookService_->pidFds_[testPid] = sockets[0];

    hookService_->SendCommand(testPid, STOP_HOOK_CMD);
    ProtocolHead head;
    uint32_t cmd = 0;
    EXPECT_EQ(read(sockets[1], &head, sizeof(head)), static_cast<ssize_t>(sizeof(head)));
    EXPECT_EQ(read(sockets[1], &cmd, sizeof(cmd)), static_cast<ssize_t>(sizeof(cmd)));
    EXPECT_EQ(head.protoType, PROTOCOL_TYPE_PROTOBUF);
    EXPECT_EQ(head.protoSize, sizeof(cmd) + sizeof(ProtocolHead));
    EXPECT_EQ(cmd, STOP_HOOK_CMD);

    close(sockets[1]);
    hookService_->pidFds_.erase(testPid);
    close(sockets[0]);
}

/*
 * @tc.name: SendCommandHandlesInvalidPositiveSocket
 * @tc.desc: test SendCommand keeps registered pid when send fails on invalid fd.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, SendCommandHandlesInvalidPositiveSocket, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    constexpr pid_t testPid = 34568;
    constexpr int invalidFd = 123456;
    hookService_->pidFds_[testPid] = invalidFd;

    hookService_->SendCommand(testPid, STOP_HOOK_CMD);
    EXPECT_TRUE(hookService_->HasSocketFd(testPid));
    hookService_->pidFds_.erase(testPid);
}

/*
 * @tc.name: SaveSocketFdAndCloseHandleBoundaryStates
 * @tc.desc: test invalid socket save and valid socket cleanup without service entry.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, SaveSocketFdAndCloseHandleBoundaryStates, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    constexpr pid_t testPid = 45678;
    TestSocketContext invalidContext;
    hookService_->SaveSocketFd(invalidContext, testPid);
    EXPECT_FALSE(hookService_->HasSocketFd(testPid));

    int sockets[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    TestSocketContext socketContext;
    socketContext.SetSocketHandle(sockets[0]);
    hookService_->SaveSocketFd(socketContext, testPid);
    EXPECT_TRUE(hookService_->HasSocketFd(testPid));

    hookService_->serviceEntry_ = nullptr;
    hookService_->CloseSocketFd(testPid);
    EXPECT_FALSE(hookService_->HasSocketFd(testPid));
    close(sockets[1]);
}

/*
 * @tc.name: FileCacheFdAndSharedMemoryFdRejectMissingResources
 * @tc.desc: test fd sending helpers reject null manager and missing hook context.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, FileCacheFdAndSharedMemoryFdRejectMissingResources, TestSize.Level0)
{
    SocketContext socketContext;
    auto nullMgrService = std::make_shared<HookService>(clientConfig_, nullptr);
    EXPECT_FALSE(nullMgrService->SendFileCacheFd(socketContext, getpid(), 0));

    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    EXPECT_FALSE(hookService_->SendFileCacheFd(socketContext, getpid(), 0));
    EXPECT_FALSE(hookService_->SendSharedMemoryFds(socketContext, getpid(), "native_daemon_ut"));
    EXPECT_FALSE(hookService_->SendPeerFds(socketContext, getpid(), "native_daemon_ut"));
}

/*
 * @tc.name: HandlePeerMessageRejectsInvalidPreloadData
 * @tc.desc: test non-cache peer messages with invalid preload payload.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, HandlePeerMessageRejectsInvalidPreloadData, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    const std::string invalidMsg = "invalid_preload_message";
    EXPECT_FALSE(hookService_->HandlePeerMessage(socketContext,
        reinterpret_cast<const int8_t*>(invalidMsg.data()), invalidMsg.size()));
}

/*
 * @tc.name: ProtocolProcRoutesCacheAndControlBranches
 * @tc.desc: test ProtocolProc routes cache messages and callback forwards to active service.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ProtocolProcRoutesCacheAndControlBranches, TestSize.Level0)
{
    clientConfig_.enableFileCacheMode = true;
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    HookCacheFileMsg stopMsg(45678, 0, CACHE_FILE_STOP);
    EXPECT_TRUE(hookService_->ProtocolProc(socketContext, 0,
        reinterpret_cast<const int8_t*>(&stopMsg), sizeof(stopMsg)));

    HookServiceDisconnectCallback(-1);
    int cmd = static_cast<int>(MEM_LEAK_DONE_CMD);
    EXPECT_TRUE(hookService_->ProtocolProc(socketContext, 0,
        reinterpret_cast<const int8_t*>(&cmd), sizeof(cmd)));
}

/*
 * @tc.name: ResolveProcessNameHandlesCurrentAndMissingProcess
 * @tc.desc: test ResolveProcessName success for current process and failure for missing pid.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ResolveProcessNameHandlesCurrentAndMissingProcess, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    std::string processName;

    ASSERT_TRUE(hookService_->ResolveProcessName(getpid(), processName));
    EXPECT_FALSE(processName.empty());
    EXPECT_EQ(hookManager_->GetPid(), getpid());
    EXPECT_FALSE(hookService_->ResolveProcessName(INT_MAX, processName));
}

/*
 * @tc.name: ControlAndCacheMessagesHandleFailurePaths
 * @tc.desc: test control command and cache file stop failure paths.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, ControlAndCacheMessagesHandleFailurePaths, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext socketContext;
    EXPECT_FALSE(hookService_->HandleControlCommand(socketContext, STOP_HOOK_CMD));
    EXPECT_TRUE(hookService_->HandleControlCommand(socketContext, MEM_LEAK_DONE_CMD));

    hookService_->ReceiveCacheFileMsg(socketContext, nullptr);
    HookCacheFileMsg stopMsg(45678, 0, CACHE_FILE_STOP);
    hookService_->ReceiveCacheFileMsg(socketContext, reinterpret_cast<const int8_t*>(&stopMsg));

    HookCacheFileMsg unknownMsg(45678, 0, CACHE_FILE_CREATED);
    hookService_->ReceiveCacheFileMsg(socketContext, reinterpret_cast<const int8_t*>(&unknownMsg));
}

/*
 * @tc.name: HandleControlCommandWithValidContext
 * @tc.desc: test HandleControlCommand succeeds with valid socket and registered hook context.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, HandleControlCommandWithValidContext, TestSize.Level0)
{
    pid_t testPid = getpid();
    auto ctx = AddFileCacheContext(hookManager_, testPid, "native_daemon_ut");
    ASSERT_NE(ctx, nullptr);

    int sockets[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    TestSocketContext serviceContext;
    serviceContext.SetSocketHandle(sockets[0]);

    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    hookService_->AddPidInfo(testPid, getuid(), getgid());
    hookService_->pidFds_[testPid] = sockets[0];

    EXPECT_FALSE(hookService_->HandleControlCommand(serviceContext, STOP_HOOK_CMD));
    EXPECT_TRUE(hookService_->HandleControlCommand(serviceContext, MEM_LEAK_DONE_CMD));

    close(sockets[1]);
}

/*
 * @tc.name: CheckConnectionPermissionHandlesStateAndCredentialBranches
 * @tc.desc: test connection permission state, startup mode and peer credential checks.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, CheckConnectionPermissionHandlesStateAndCredentialBranches, TestSize.Level0)
{
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    SocketContext invalidContext;
    EXPECT_FALSE(hookService_->CheckConnectionPermission(invalidContext, getpid()));
    EXPECT_FALSE(hookService_->CheckConnectionPermission(invalidContext, getpid()));

    NativeHookConfig hookConfig;
    hookConfig.set_startup_mode(true);
    hookManager_->SetHookConfig(hookConfig);
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    EXPECT_FALSE(hookService_->CheckConnectionPermission(invalidContext, getpid()));

    int sockets[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    hookConfig.set_startup_mode(true);
    hookManager_->SetHookConfig(hookConfig);
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    TestSocketContext socketContext;
    socketContext.SetSocketHandle(sockets[0]);
    EXPECT_TRUE(hookService_->CheckConnectionPermission(socketContext, getpid()));

    hookConfig.set_startup_mode(false);
    hookManager_->SetHookConfig(hookConfig);
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    EXPECT_FALSE(hookService_->CheckConnectionPermission(socketContext, getpid()));
    close(sockets[1]);
}

/*
 * @tc.name: CheckConnectionPermissionAcceptsMatchingPeerCredential
 * @tc.desc: test credential whitelist succeeds for connected peer.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, CheckConnectionPermissionAcceptsMatchingPeerCredential, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_startup_mode(false);
    hookManager_->SetHookConfig(hookConfig);
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    hookService_->AddPidInfo(getpid(), getuid(), getgid());

    int sockets[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    TestSocketContext socketContext;
    socketContext.SetSocketHandle(sockets[0]);
    EXPECT_TRUE(hookService_->CheckConnectionPermission(socketContext, getpid()));
    close(sockets[1]);
}

/*
 * @tc.name: SendOptionalFdsHandleDisabledAndMissingResources
 * @tc.desc: test async stack and mem leak fd disabled or missing-resource paths.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, SendOptionalFdsHandleDisabledAndMissingResources, TestSize.Level0)
{
    SocketContext socketContext;
    clientConfig_.asyncStackEnable = false;
    clientConfig_.memLeakFormat = MemLeakFormat::TYPE_DISABLE;
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    EXPECT_TRUE(hookService_->SendAsyncStackFd(socketContext));
    EXPECT_TRUE(hookService_->SendMemLeakFd(socketContext));

    clientConfig_.asyncStackEnable = true;
    clientConfig_.memLeakFormat = MemLeakFormat::TYPE_SNAPSHOT;
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    EXPECT_FALSE(hookService_->SendAsyncStackFd(socketContext));
    EXPECT_FALSE(hookService_->SendMemLeakFd(socketContext));
}

/*
 * @tc.name: SendPeerFdsRejectsMissingFileCacheContext
 * @tc.desc: test file cache mode fails when no cache context can be prepared.
 * @tc.type: FUNC
 */
HWTEST_F(HookServiceExceptionTest, SendPeerFdsRejectsMissingFileCacheContext, TestSize.Level0)
{
    SocketContext socketContext;
    clientConfig_.enableFileCacheMode = true;
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    EXPECT_FALSE(hookService_->SendInitialFileCacheFds(socketContext, getpid(), "native_daemon_ut"));
    EXPECT_FALSE(hookService_->SendPeerFds(socketContext, getpid(), "native_daemon_ut"));
}

HWTEST_F(HookServiceExceptionTest, SendInitialFileCacheFdsSendsAllDescriptors, TestSize.Level0)
{
    int sockets[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    TestSocketContext serviceContext;
    TestSocketContext peerContext;
    serviceContext.SetSocketHandle(sockets[0]);
    peerContext.SetSocketHandle(sockets[1]);
    constexpr pid_t testPid = 45679;
    auto ctx = AddFileCacheContext(hookManager_, testPid, "native_daemon_ut");
    ASSERT_NE(ctx, nullptr);
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);

    EXPECT_TRUE(hookService_->SendInitialFileCacheFds(serviceContext, testPid, "native_daemon_ut"));
    for (uint16_t i = 0; i < FILE_CACHE_FD_COUNT; ++i) {
        int fd = peerContext.ReceiveFileDiscriptor();
        EXPECT_GE(fd, 0);
        CloseFd(fd);
    }
    EXPECT_FALSE(ctx->cacheFileStopRequested);
}

HWTEST_F(HookServiceExceptionTest, ReceiveCacheFileReadyQueuesAndReplenishesFd, TestSize.Level0)
{
    int sockets[2] = {-1, -1};
    ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
    TestSocketContext serviceContext;
    TestSocketContext peerContext;
    serviceContext.SetSocketHandle(sockets[0]);
    peerContext.SetSocketHandle(sockets[1]);
    pid_t testPid = getpid();
    auto ctx = AddFileCacheContext(hookManager_, testPid, "native_daemon_ut");
    ASSERT_NE(ctx, nullptr);
    hookService_ = std::make_shared<HookService>(clientConfig_, hookManager_);
    char tempPath[] = "/data/local/tmp/hook_cache_ready_XXXXXX";
    int cacheFd = mkstemp(tempPath);
    ASSERT_GE(cacheFd, 0);
    ASSERT_EQ(unlink(tempPath), 0);
    ASSERT_TRUE(peerContext.SendFileDescriptor(cacheFd));
    CloseFd(cacheFd);

    HookCacheFileMsg readyMsg(testPid, 1, CACHE_FILE_READY);
    hookService_->ReceiveCacheFileMsg(serviceContext, reinterpret_cast<const int8_t*>(&readyMsg));

    ProtocolHead head = {};
    HookCacheFileMsg createdMsg;
    EXPECT_EQ(read(sockets[1], &head, sizeof(head)), static_cast<ssize_t>(sizeof(head)));
    EXPECT_EQ(read(sockets[1], &createdMsg, sizeof(createdMsg)), static_cast<ssize_t>(sizeof(createdMsg)));
    int replenishFd = peerContext.ReceiveFileDiscriptor();
    EXPECT_EQ(createdMsg.msgType, CACHE_FILE_CREATED);
    EXPECT_EQ(createdMsg.fileIndex, 1);
    EXPECT_EQ(ctx->GetFdQueueSize(), 1u);
    CloseFd(replenishFd);
}
} // namespace OHOS::Developtools::NativeDaemon
