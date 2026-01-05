/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <dlfcn.h>
#include <iostream>
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <memory>
#include <sys/syscall.h>

#include "command_poller.h"
#include "init_param.h"
#include "network_profiler_socker_client.h"
#include "network_profiler.h"
#include "network_profiler_manager_mock.h"
#include "common.h"
#include "grpc/impl/codegen/log.h"
#include "plugin_service.h"
#include "plugin_service.ipc.h"
#include "socket_context.h"
#include "network_profiler_config.pb.h"

using namespace testing::ext;
using namespace OHOS::Developtools::Profiler;

namespace {
const std::string OUTPUT_PATH = "/data/local/tmp/hiprofiler_data.htrace";
const std::string NETWORK_TEST_EXE = "/data/local/tmp/network_profiler_test_exe";
const std::string PARAM_KAY = "hiviewdfx.hiprofiler.networkprofiler.target";
constexpr uint32_t BUFFER_SIZE = 1024;
constexpr uint32_t SMB1_SIZE = 10 * 4096;
constexpr int FILE_SIZE = 2000;
constexpr int MOBILE_BIT = 32;
constexpr int SLEEP_TIME = 3;
const std::string WRITER_NAME = "NetworkProfilerWriterTest";
void* g_smbAddr = nullptr;
int g_smbFd1 = 0;

int InitShareMemory()
{
    int fd = syscall(SYS_memfd_create, WRITER_NAME.c_str(), 0);
    CHECK_TRUE(fd >= 0, -1, "CreateBlock FAIL SYS_memfd_create");

    int check = ftruncate(fd, SMB1_SIZE);
    if (check < 0) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock ftruncate ERR : %s", buf);
        return -1;
    }

    g_smbAddr = mmap(nullptr, SMB1_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_smbAddr == (reinterpret_cast<void *>(-1))) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock g_smbAddr mmap ERR : %s", buf);
        return -1;
    }

    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr);

    // initialize header infos
    header_->info.readOffset_ = 0;
    header_->info.writeOffset_ = 0;
    header_->info.memorySize_ = SMB1_SIZE - sizeof(ShareMemoryBlock::BlockHeader);
    header_->info.bytesCount_ = 0;
    header_->info.chunkCount_ = 0;

    return fd;
}

class NetworkProfilerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
    };

    static void TearDownTestCase()
    {
    }
    std::string CreateCommand(const std::string& outputFile, int32_t time, const std::string& model,
        const std::string& procedure) const
    {
        std::string cmdStr =
            "hiprofiler_cmd \\\n"
            "-c - \\\n";
        cmdStr += "-o " + outputFile + " \\\n";
        cmdStr += "-t " + std::to_string(time) + " \\\n";
        cmdStr += "-s \\\n";
        cmdStr += "-k \\\n"
            "<<CONFIG\n"
            "request_id: 1\n"
            "session_config {\n"
            "  buffers {\n"
            "    pages: 32768\n"
            "  }\n"
            "  result_file: \"/data/local/tmp/hiprofiler_data.htrace\"\n"
            "  sample_duration: 30000\n"
            "}\n"
            "plugin_configs {\n"
            "  plugin_name: \"ffrt-profiler\"\n"
            "  config_data {\n";
        cmdStr += model + ": " + procedure + '\n';
        cmdStr += "smb_pages: 16384\n"
                "flush_interval: 5\n"
                "block: true\n"
                "clock_id: BOOTTIME\n"
            "  }\n"
            "}\n"
            "CONFIG\n";
        return cmdStr;
    }
    void StartProcess(const std::string& name, const std::string& args)
    {
        if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
            return;
        }

        int processNum = fork();
        if (processNum == 0) {
            sleep(SLEEP_TIME);
            execl(name.c_str(), name.c_str(), args.c_str(), NULL);
            _exit(1);
        } else if (processNum < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "Failed to fork process");
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "sub process PID: %d", processNum);
            prfolerExePid = processNum;
        }
    }
    void SetUp()
    {
        g_smbFd1 = InitShareMemory();
    }
    void TearDown()
    {
        g_smbFd1 = 0;
    }
    int prfolerExePid{0};
};

void CallBackFunc()
{
    return;
}

void RandData(uint8_t* data, int size)
{
    time_t tv = time(nullptr);
    if (tv == -1) {
        tv = 1;
    }
    unsigned int seed = static_cast<unsigned int>(tv);
    while (--size) {
        data[size] = rand_r(&seed) / static_cast<uint8_t>(-1);
    }
}

/**
 * @tc.name: NetworkProfilerTest001
 * @tc.desc: Test NetworkProfilerSocketClient IsProfilerEnable without SystemSetParameter
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerTest001, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    auto networkProfilerService = std::make_unique<NetworkProfilerSocketService>(networkProfilerMgr);
    ASSERT_TRUE(networkProfilerService != nullptr);
    networkProfilerMgr->Init();
    auto networkProfiler = NetworkProfiler::GetInstance();
    auto ret = networkProfiler->IsProfilerEnable();
    ASSERT_FALSE(ret);
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    client.SendNetworkProfilerData(nullptr, 0, nullptr, 0);
}

/**
 * @tc.name: NetworkProfilerTest002
 * @tc.desc: Test NetworkProfilerSocketClient send empty data to server
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerTest002, TestSize.Level1)
{
    SystemSetParameter("hiviewdfx.hiprofiler.plugins.start", "1");
    auto networkProfiler = NetworkProfiler::GetInstance();
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    sleep(2);
    auto ret = client.SendNetworkProfilerData(nullptr, 0, nullptr, 0);
    ASSERT_FALSE(ret);
    NetworkConfig config;
    SocketContext ctx;
    ret = client.ProtocolProc(ctx, 0, reinterpret_cast<int8_t*>(&config), sizeof(config));
    ASSERT_TRUE(ret);
    SystemSetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
}

/**
 * @tc.name: NetworkProfilerTest003
 * @tc.desc: Test NetworkProfilerSocketClient send empty data to server
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerTest003, TestSize.Level1)
{
    SystemSetParameter("hiviewdfx.hiprofiler.plugins.start", "1");
    auto networkProfiler = NetworkProfiler::GetInstance();
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    sleep(2);
    auto ret = client.SendNetworkProfilerData(nullptr, 0, nullptr, 0);
    ASSERT_FALSE(ret);
    networkProfiler->Enable();
    networkProfiler->ServiceCloseCallback();
    networkProfiler->Disable();
    SystemSetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
}

/**
 * @tc.name: NetworkProfilerTest004
 * @tc.desc: Test NetworkProfilerSocketClient IsProfilerEnable without SystemSetParameter
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerTest004, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    auto networkProfilerService = std::make_unique<NetworkProfilerSocketService>(networkProfilerMgr);
    ASSERT_TRUE(networkProfilerService != nullptr);
    networkProfilerMgr->Init();
    auto networkProfiler = NetworkProfiler::GetInstance();
    auto ret = networkProfiler->IsProfilerEnable();
    ASSERT_FALSE(ret);
    networkProfiler->Enable();
    networkProfiler->SetEnableFlag(true);
    networkProfiler->Enable();
    networkProfiler->~NetworkProfiler();
}

/**
 * @tc.name: NetworkProfilerWriterTest001
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerWriterTest001, TestSize.Level1)
{
    auto write = std::make_shared<NetworkProfilerWriter>(WRITER_NAME, SMB1_SIZE, g_smbFd1, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    uint8_t data[] = {0x55, 0xAA, 0x55, 0xAA};
    auto myCallback = []() -> bool {
        return true;
    };
    EXPECT_TRUE(write->WriteTimeout(static_cast<void*>(data), sizeof(data), myCallback));
    EXPECT_FALSE(write->WriteTimeout(static_cast<void*>(data), 0, myCallback));
    EXPECT_FALSE(write->WriteTimeout(nullptr, 0, myCallback));

    uint8_t payload[] = {0x11, 0x22, 0x33, 0x44};
    EXPECT_TRUE(write->WriteWithPayloadTimeout(static_cast<void*>(data), sizeof(data),
        static_cast<void*>(payload), sizeof(payload), myCallback));
    EXPECT_FALSE(write->WriteWithPayloadTimeout(static_cast<void*>(data), 0, nullptr, 0, myCallback));
    EXPECT_FALSE(write->WriteWithPayloadTimeout(nullptr, 0, nullptr, 0, myCallback));
    write->block_ = true;
    EXPECT_TRUE(write->WriteWithPayloadTimeout(static_cast<void*>(data), sizeof(data),
        static_cast<void*>(payload), sizeof(payload), myCallback));
}

/**
 * @tc.name: NetworkProfilerWriterTest002
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerWriterTest002, TestSize.Level1)
{
    uint8_t data[BUFFER_SIZE];
    RandData(data, BUFFER_SIZE);
    auto write = std::make_shared<NetworkProfilerWriter>(WRITER_NAME, SMB1_SIZE, g_smbFd1, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);
    long bytes = BUFFER_SIZE;

    EXPECT_TRUE(write->Flush());
    write->DoStats(bytes);
    write->Report();
}

/**
 * @tc.name: NetworkProfilerWriterTest003
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerWriterTest003, TestSize.Level1)
{
    auto write = std::make_shared<NetworkProfilerWriter>(WRITER_NAME, SMB1_SIZE, g_smbFd1, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t buffer2[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t buffer3[] = {0xAA, 0xBB, 0xCC, 0xDD};

    EXPECT_TRUE(write->Write((const void*)buffer1, sizeof(buffer1)));
    EXPECT_TRUE(write->Write((const void*)buffer2, sizeof(buffer2)));
    EXPECT_TRUE(write->Write((const void*)buffer3, sizeof(buffer3)));

    EXPECT_FALSE(write->Write((const void*)buffer3, 0));
    EXPECT_FALSE(write->Write(nullptr, 0));
}

/**
 * @tc.name: NetworkProfilerSockerClientTest004
 * @tc.desc: test NetworkProfilerSockerClient::NetworkProfilerSockerClientTest004 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerSockerClientTest004, TestSize.Level1)
{
    auto write = std::make_shared<NetworkProfilerWriter>(WRITER_NAME, SMB1_SIZE, g_smbFd1, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    auto networkProfiler = NetworkProfiler::GetInstance();
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    NetworkConfig config;
    SocketContext ctx;
    auto ret = client.ProtocolProc(ctx, 0, reinterpret_cast<int8_t*>(&config), sizeof(config));
    ASSERT_TRUE(ret);
    client.unixSocketClient_ = std::make_shared<UnixSocketClient>();
    sleep(2);
    uint8_t data[] = {0x55, 0xAA, 0x55, 0xAA};
    uint8_t payload[] = {0x11, 0x22, 0x33, 0x44};
    ASSERT_FALSE(client.SendNetworkProfilerData(static_cast<void*>(data), sizeof(data),
        static_cast<void*>(payload), sizeof(payload)));
}

/**
 * @tc.name: NetworkProfilerSockerClientTest005
 * @tc.desc: test NetworkProfilerSockerClient::NetworkProfilerSockerClientTest005 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerSockerClientTest005, TestSize.Level1)
{
    auto write = std::make_shared<NetworkProfilerWriter>(WRITER_NAME, SMB1_SIZE, g_smbFd1, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    auto networkProfiler = NetworkProfiler::GetInstance();
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    ASSERT_FALSE(client.PeerIsConnected());
    client.unixSocketClient_ = std::make_shared<UnixSocketClient>();
    ASSERT_TRUE(client.PeerIsConnected());
    client.Flush();

    NetworkConfig config;
    SocketContext ctx;
    auto ret = client.ProtocolProc(ctx, 0, reinterpret_cast<int8_t*>(&config), sizeof(config));
    ASSERT_TRUE(ret);
    client.Flush();
}

/**
 * @tc.name: NetworkProfilerSockerClientTest006
 * @tc.desc: test NetworkProfilerSockerClient::NetworkProfilerSockerClientTest006 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerSockerClientTest006, TestSize.Level1)
{
    auto write = std::make_shared<NetworkProfilerWriter>(WRITER_NAME, SMB1_SIZE, g_smbFd1, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);

    auto networkProfiler = NetworkProfiler::GetInstance();
    NetworkProfilerSocketClient client(1, networkProfiler, CallBackFunc);
    client.unixSocketClient_ = std::make_shared<UnixSocketClient>();
    ASSERT_FALSE(client.Connect(NETWORK_PROFILER_UNIX_SOCKET_FULL_PATH, nullptr));
}

/**
 * @tc.name: NetworkProfilerServiceTest001
 * @tc.desc: test NetworkProfilerService::NetworkProfilerServiceTest001 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerServiceTest001, TestSize.Level1)
{
    StartProcess(NETWORK_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(prfolerExePid));
    string str = "network_plugin_test_string";
    string seq = "_";
    std::vector<string> ret;
    SplitParamValue(str, seq, ret);
    EXPECT_EQ(ret.size(), 4);
}

/**
 * @tc.name: NetworkProfilerServiceTest002
 * @tc.desc: test NetworkProfilerService::NetworkProfilerServiceTest002 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerServiceTest002, TestSize.Level1)
{
    StartProcess(NETWORK_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(prfolerExePid));
    string str = "";
    string seq = "_";
    std::vector<string> ret;
    SplitParamValue(str, seq, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.name: NetworkProfilerServiceTest003
 * @tc.desc: test NetworkProfilerService::NetworkProfilerServiceTest003 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerServiceTest003, TestSize.Level1)
{
    StartProcess(NETWORK_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(prfolerExePid));
    auto res = GetProcessNameByPid(prfolerExePid);
    EXPECT_EQ(res, "networkprofiler_ut");
}

/**
 * @tc.name: NetworkProfilerManagerTest001
 * @tc.desc: test NetworkProfilerManager::NetworkProfilerManagerTest001 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerManagerTest001, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    ASSERT_TRUE(networkProfilerMgr != nullptr);
    networkProfilerMgr->Init();

    std::vector<uint32_t> pluginIds(1);
    ProfilerPluginConfig config;
    config.set_name("network");
    config.set_plugin_sha256("");
    config.set_sample_interval(20);

    PluginResult result;
    std::vector<ProfilerPluginConfig> configVec;
    configVec.push_back(config);

    EXPECT_TRUE(networkProfilerMgr->CreatePluginSession(configVec));
    EXPECT_FALSE(networkProfilerMgr->StartPluginSession(pluginIds, configVec, result));
    EXPECT_TRUE(networkProfilerMgr->CreateWriter("name", 0, 0, 0));
    EXPECT_TRUE(networkProfilerMgr->ResetWriter(0));
    EXPECT_TRUE(networkProfilerMgr->StopPluginSession(pluginIds));
    EXPECT_TRUE(networkProfilerMgr->DestroyPluginSession(pluginIds));
}

/**
 * @tc.name: NetworkProfilerManagerTest002
 * @tc.desc: test NetworkProfilerManager::NetworkProfilerManagerTest002 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerManagerTest002, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    ASSERT_TRUE(networkProfilerMgr != nullptr);
    auto commandPoller = std::make_shared<CommandPoller>(networkProfilerMgr);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_FALSE(commandPoller->OnConnect());
    networkProfilerMgr->SetCommandPoller(commandPoller);

    std::vector<uint32_t> pluginIds = {1, 2, 3};
    EXPECT_FALSE(networkProfilerMgr->RegisterAgentPlugin("network"));
    EXPECT_TRUE(networkProfilerMgr->ReportPluginBasicData(pluginIds));
}

/**
 * @tc.name: NetworkProfilerManagerTest003
 * @tc.desc: test NetworkProfilerManager::NetworkProfilerManagerTest003 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerManagerTest003, TestSize.Level1)
{
    SystemSetParameter("hiviewdfx.hiprofiler.plugins.start", "1");
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    ASSERT_TRUE(networkProfilerMgr != nullptr);

    auto commandPoller = std::make_shared<CommandPoller>(networkProfilerMgr);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_FALSE(commandPoller->OnConnect());
    networkProfilerMgr->SetCommandPoller(commandPoller);

    EXPECT_FALSE(networkProfilerMgr->RegisterAgentPlugin("network"));
    ASSERT_TRUE(networkProfilerMgr->LoadPlugin("network"));
    ASSERT_TRUE(networkProfilerMgr->UnloadPlugin("network"));
    SystemSetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
}

/**
 * @tc.name: NetworkProfilerManagerTest004
 * @tc.desc: test NetworkProfilerManager::NetworkProfilerManagerTest004 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerManagerTest004, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    ASSERT_TRUE(networkProfilerMgr != nullptr);
    auto commandPoller = std::make_shared<CommandPoller>(networkProfilerMgr);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_FALSE(commandPoller->OnConnect());
    networkProfilerMgr->SetCommandPoller(commandPoller);

    EXPECT_FALSE(networkProfilerMgr->RegisterAgentPlugin("network"));
    ASSERT_TRUE(networkProfilerMgr->LoadPlugin("network"));
    ASSERT_TRUE(networkProfilerMgr->UnloadPlugin(commandPoller->GetRequestId()));
}

/**
 * @tc.name: NetworkProfilerHandleTest001
 * @tc.desc: test NetworkProfilerHandle::NetworkProfilerHandleTest001 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerHandleTest001, TestSize.Level1)
{
    std::shared_ptr<NetworkProfilerManager> networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    NetworkProfilerConfig config;
    int32_t pid = -1;
    config.add_pid(pid);
    config.add_pid(prfolerExePid);
    config.add_startup_process_name("");
    config.add_startup_process_name("test_name002");
    config.add_restart_process_name("");
    config.set_clock_id(NetworkProfilerConfig::MONOTONIC);
    networkProfilerMgr->SetConfig(config);
    EXPECT_TRUE(networkProfilerMgr->CheckConfig());

    auto handle = std::make_shared<NetworkProfilerHandle>(BUFFER_SIZE, true);
    const int8_t data[] = {1, 2, 3};
    handle->SerializeData(data, FILE_SIZE);
    handle->StopHandle();
}

/**
 * @tc.name: NetworkProfilerHandleTest002
 * @tc.desc: test NetworkProfilerHandle::NetworkProfilerHandleTest002 with normal case.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(NetworkProfilerTest, NetworkProfilerHandleTest002, TestSize.Level1)
{
    auto write = std::make_shared<NetworkProfilerWriter>(WRITER_NAME, SMB1_SIZE, g_smbFd1, -1, false);
    EXPECT_NE(write->shareMemoryBlock_, nullptr);
    uint8_t buffer1[] = {0x55, 0xAA, 0x55, 0xAA};
    std::shared_ptr<NetworkProfilerManager> networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    NetworkProfilerConfig config;
    int32_t pid = 1;
    config.add_pid(pid);
    config.add_pid(prfolerExePid);
    config.add_startup_process_name("");
    config.add_startup_process_name("test_name003");
    config.add_restart_process_name("");
    config.set_clock_id(NetworkProfilerConfig::MONOTONIC);
    networkProfilerMgr->SetConfig(config);
    EXPECT_TRUE(networkProfilerMgr->CheckConfig());
    EXPECT_FALSE(networkProfilerMgr->StartNetworkProfiler());
    EXPECT_TRUE(networkProfilerMgr->CreateWriter("name", 0, 0, 0, false));
    auto handle = std::make_shared<NetworkProfilerHandle>(BUFFER_SIZE, true);
    const int8_t data[] = {1, 2, 3};
    handle->SerializeData(data, FILE_SIZE);

    EXPECT_TRUE(write->Write((const void*)buffer1, sizeof(buffer1)));
    handle->SetWriter(write);
    handle->StopHandle();
}
#endif

/**
 * @tc.name: NetworkProfilerHandleTest003
 * @tc.desc: test NetworkProfilerHandle::NetworkProfilerHandleTest003 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerHandleTest003, TestSize.Level1)
{
    std::shared_ptr<NetworkProfilerManager> networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    NetworkProfilerConfig config;
    int32_t pid = 1;
    config.add_pid(pid);
    config.add_pid(prfolerExePid);
    config.add_startup_process_name("network");
    config.set_clock_id(NetworkProfilerConfig::MONOTONIC);
    networkProfilerMgr->SetConfig(config);
    EXPECT_TRUE(networkProfilerMgr->CheckConfig());
    EXPECT_FALSE(networkProfilerMgr->StartNetworkProfiler());
    EXPECT_TRUE(networkProfilerMgr->CreateWriter("name", 0, 0, 0, true));

    auto handle = std::make_shared<NetworkProfilerHandle>(BUFFER_SIZE, true);
    const int8_t data[] = {};
    handle->SerializeData(data, 0);

    handle->StopHandle();
    ASSERT_TRUE(networkProfilerMgr->UnloadPlugin("network"));
}

/**
 * @tc.name: NetworkProfilerSocketServiceTest004
 * @tc.desc: test NetworkProfilerSocketService::NetworkProfilerSocketServiceTest004 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerSocketServiceTest004, TestSize.Level1)
{
    uint64_t config = FILE_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB1_SIZE;
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    auto socketService_ = std::make_shared<NetworkProfilerSocketService>(networkProfilerMgr);

    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(uint64_t);
    ASSERT_FALSE(socketService_->ProtocolProc(socketContext, 0, ptr, size));
}

/**
 * @tc.name: NetworkProfilerSocketServiceTest005
 * @tc.desc: test NetworkProfilerSocketService::NetworkProfilerSocketServiceTest005 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerSocketServiceTest005, TestSize.Level1)
{
    uint64_t config = FILE_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB1_SIZE;
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    auto socketService = std::make_shared<NetworkProfilerSocketService>(networkProfilerMgr);

    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(int);
    socketService->SetConfig(sizeof(uint64_t), sizeof(uint64_t), true, 0);
    ASSERT_FALSE(socketService->ProtocolProc(socketContext, 0, ptr, size));
}

/**
 * @tc.name: NetworkProfilerSocketServiceTest006
 * @tc.desc: test NetworkProfilerSocketService::NetworkProfilerSocketServiceTest006 with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, NetworkProfilerSocketServiceTest006, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    auto socketService = std::make_shared<NetworkProfilerSocketService>(networkProfilerMgr);

    SocketContext socketContext;
    int pid = -1;
    const int8_t* ptr = reinterpret_cast<const int8_t*>(&pid);
    socketService->SetConfig(sizeof(uint64_t), sizeof(uint64_t), true, 0);
    ASSERT_FALSE(socketService->ProtocolProc(socketContext, 0, ptr, sizeof(pid)));
}

/**
 * @tc.name: StartNetworkProfilerTest
 * @tc.desc: test NetworkProfilerManager::StartNetworkProfilerTest with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, StartNetworkProfilerTest, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManagerMock>();
    testing::Mock::AllowLeak(networkProfilerMgr.get());
    EXPECT_CALL(*networkProfilerMgr, CheckConfig()).WillRepeatedly(::testing::Return(true));
    EXPECT_CALL(*networkProfilerMgr, HandleNetworkProfilerContext(::testing::_))
        .WillRepeatedly(::testing::Return(true));
    ASSERT_TRUE(networkProfilerMgr != nullptr);
    networkProfilerMgr->Init();

    std::vector<uint32_t> pluginIds(1);
    ProfilerPluginConfig config;
    config.set_name("network");
    config.set_plugin_sha256("");
    config.set_sample_interval(20);

    PluginResult result;
    std::vector<ProfilerPluginConfig> configVec;
    configVec.push_back(config);

    EXPECT_FALSE(networkProfilerMgr->StartNetworkProfiler());
    networkProfilerMgr->paramValue_ = "test";
    EXPECT_FALSE(networkProfilerMgr->StartNetworkProfiler());

    int32_t pid = 1;
    networkProfilerMgr->networkCtx_.clear();
    networkProfilerMgr->networkCtx_.emplace_back(std::make_shared<NetworkProfilerManager::NetworkProfilerCtx>(pid));
    networkProfilerMgr->config_.add_pid(pid);
    networkProfilerMgr->config_.add_startup_process_name("test_name_startup");
    networkProfilerMgr->config_.add_restart_process_name("test_name_restart");
    EXPECT_TRUE(networkProfilerMgr->StartNetworkProfiler());
    auto commandPollerMock = std::make_shared<CommandPollerMock>(networkProfilerMgr);
    EXPECT_CALL(*commandPollerMock, PushResult(::testing::_, ::testing::_)).WillRepeatedly(::testing::DoDefault());
    testing::Mock::AllowLeak(commandPollerMock.get());
    ASSERT_TRUE(commandPollerMock != nullptr);
    networkProfilerMgr->SetCommandPoller(commandPollerMock);
    EXPECT_TRUE(networkProfilerMgr->StartPluginSession(pluginIds, configVec, result));
}

/**
 * @tc.name: HandleNetworkProfilerContextTest
 * @tc.desc: test NetworkProfilerManager::HandleNetworkProfilerContextTest with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, HandleNetworkProfilerContextTest, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    EXPECT_FALSE(networkProfilerMgr->HandleNetworkProfilerContext(nullptr));

    int32_t pid = -1;
    auto ctx1 = std::make_shared<NetworkProfilerManager::NetworkProfilerCtx>(pid);
    EXPECT_FALSE(networkProfilerMgr->HandleNetworkProfilerContext(ctx1));

    pid = 1;
    auto ctx2 = std::make_shared<NetworkProfilerManager::NetworkProfilerCtx>(pid);
    uint32_t smbPages = 100;
    networkProfilerMgr->config_.set_smb_pages(smbPages);
    EXPECT_TRUE(networkProfilerMgr->HandleNetworkProfilerContext(ctx2));
    EXPECT_EQ(ctx2->smbName, "network_profiler_smb_1");

    string processName = "test";
    EXPECT_TRUE(networkProfilerMgr->CreateWriter("name", 0, 0, 0, false));
    auto ctx3 = std::make_shared<NetworkProfilerManager::NetworkProfilerCtx>(processName);
    EXPECT_TRUE(networkProfilerMgr->HandleNetworkProfilerContext(ctx3));
    EXPECT_EQ(ctx3->smbName, "network_profiler_smb_test");
    ctx3->eventNotifier = EventNotifier::Create(0, EventNotifier::NONBLOCK);
    networkProfilerMgr->ReadShareMemory(ctx3);
}

/**
 * @tc.name: GetClockIdTest
 * @tc.desc: test NetworkProfilerManager::GetClockIdTest with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, GetClockIdTest, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    EXPECT_EQ(networkProfilerMgr->GetClockId(NetworkProfilerConfig::BOOTTIME), CLOCK_BOOTTIME);
    EXPECT_EQ(networkProfilerMgr->GetClockId(NetworkProfilerConfig::REALTIME), CLOCK_REALTIME);
    EXPECT_EQ(networkProfilerMgr->GetClockId(NetworkProfilerConfig::REALTIME_COARSE), CLOCK_REALTIME_COARSE);
    EXPECT_EQ(networkProfilerMgr->GetClockId(NetworkProfilerConfig::MONOTONIC), CLOCK_MONOTONIC);
    EXPECT_EQ(networkProfilerMgr->GetClockId(NetworkProfilerConfig::MONOTONIC_COARSE), CLOCK_MONOTONIC_COARSE);
    EXPECT_EQ(networkProfilerMgr->GetClockId(NetworkProfilerConfig::MONOTONIC_RAW), CLOCK_MONOTONIC_RAW);
    EXPECT_EQ(networkProfilerMgr->GetClockId(NetworkProfilerConfig::UNKNOW), CLOCK_BOOTTIME);
}

/**
 * @tc.name: CheckRestartProcessNameTestNotStart
 * @tc.desc: test NetworkProfilerManager::CheckRestartProcessNameTestNotStart with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, CheckRestartProcessNameTestNotStart, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    networkProfilerMgr->config_.add_restart_process_name("test_name_restart");
    std::set<int32_t> pidCache;
    EXPECT_FALSE(networkProfilerMgr->CheckRestartProcessName(pidCache));
}

/**
 * @tc.name: CheckRestartProcessNameTestStarted
 * @tc.desc: test NetworkProfilerManager::CheckRestartProcessNameTestStarted with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(NetworkProfilerTest, CheckRestartProcessNameTestStarted, TestSize.Level1)
{
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    const std::string processName = "foundation";
    networkProfilerMgr->config_.add_restart_process_name(processName);
    std::set<int32_t> pidCache;
    EXPECT_TRUE(networkProfilerMgr->CheckRestartProcessName(pidCache));
    int32_t pidValue = -1;
    COMMON::IsProcessExist(processName, pidValue);
    EXPECT_EQ(networkProfilerMgr->networkCtx_.size(), static_cast<size_t>(1));
    EXPECT_EQ(networkProfilerMgr->GetNetworkProfilerCtx(pidValue, processName), std::make_pair(0, 0));
}

} // namespace
