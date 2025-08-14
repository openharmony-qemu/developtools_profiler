/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include <cinttypes>
#include <dlfcn.h>
#include <gtest/gtest.h>
#include <csignal>
#include <filesystem>

#include "command_poller.h"
#include "ffrt_profiler_common.h"
#include "ffrt_profiler_manager.h"
#include "socket_context.h"

namespace fs = std::filesystem;
using namespace testing::ext;
using namespace OHOS::Developtools::Profiler;

namespace {
const std::string OUTPUT_PATH = "/data/local/tmp/hiprofiler_data.htrace";
const std::string FFRT_TEST_EXE = "/data/local/tmp/ffrt_profiler_test_exe";
constexpr uint32_t BUFFER_SIZE = (1UL << 23);
constexpr int FILE_SIZE = 2000;
constexpr int MOBILE_BIT = 32;
constexpr int32_t SMB_SIZE = 409600;

class FfrtPofilerTest : public ::testing::Test {
public:
    FfrtPofilerTest() {}
    ~FfrtPofilerTest() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

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
            execl(name.c_str(), name.c_str(), args.c_str(), NULL);
            _exit(1);
        } else if (processNum < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "Failed to fork process");
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "sub process PID: %d", processNum);
            ffrtPrfolerExePid_ = processNum;
        }
    }

    bool RunCommand(const std::string& cmd, std::string& content)
    {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        CHECK_TRUE(pipe, false, "RunCommand: create popen FAILED!");
        static constexpr int buffSize = 1024;
        std::array<char, buffSize> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            content += buffer.data();
        }
        return true;
    }

    bool CheckFileSize(const std::string& filePath)
    {
        if (!fs::exists(filePath)) {
            return false;
        }
        if (fs::file_size(filePath) < FILE_SIZE) {
            return false;
        }
        return true;
    }

    int ffrtPrfolerExePid_{0};
};

HWTEST_F(FfrtPofilerTest, TestFfrtProfilerRuntime, TestSize.Level1)
{
    StartProcess(FFRT_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    fs::remove(OUTPUT_PATH);
    EXPECT_TRUE(RunCommand(cmd, ret));
    EXPECT_TRUE(ret.find("FAIL") == std::string::npos);
}

HWTEST_F(FfrtPofilerTest, TestFfrtProfilerError, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    fs::remove(OUTPUT_PATH);
    EXPECT_TRUE(RunCommand(cmd, ret));
    EXPECT_TRUE(ret.find("FAIL") == std::string::npos);
    EXPECT_FALSE(CheckFileSize(OUTPUT_PATH));
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc: FfrtPofiler CheckConfig Function return false
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction001, TestSize.Level1)
{
    using namespace OHOS::Developtools::Profiler;
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    ffrtProfilerMgr->Init();
    EXPECT_FALSE(ffrtProfilerMgr->CheckConfig());
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc: FfrtPofiler CheckConfig Function startup_process is exit return false
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction002, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::string pidService;
    RunCommand("pidof render_service", pidService);

    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = 1;
    config.add_pid(pid);
    config.add_pid(stoi(pidService));
    config.add_pid(ffrtPrfolerExePid_);
    config.add_startup_process_name("test_name001");
    config.add_startup_process_name("render_service");
    config.add_startup_process_name("test_name003");
    config.set_clock_id(FfrtProfilerConfig::REALTIME_COARSE);
    ffrtProfilerMgr->SetConfig(config);
    EXPECT_FALSE(ffrtProfilerMgr->CheckConfig());
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc: FfrtPofiler CheckConfig Function startup_process is not exit return true
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction003, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));

    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = 1;
    config.add_pid(pid);
    config.add_pid(ffrtPrfolerExePid_);
    config.add_startup_process_name("test_name001");
    config.add_startup_process_name("test_name002");
    config.set_clock_id(FfrtProfilerConfig::REALTIME_COARSE);
    ffrtProfilerMgr->SetConfig(config);
    EXPECT_TRUE(ffrtProfilerMgr->CheckConfig());
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc: FfrtPofiler CheckConfig Function startup_process_name is not exit return flase
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction004, TestSize.Level1)
{
    StartProcess(FFRT_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = 1;

    std::string cont;
    RunCommand("pidof render_service", cont);
    auto pidService = stoi(cont);

    config.add_pid(pid);
    config.add_pid(pidService);
    config.add_startup_process_name("test_name");
    config.add_startup_process_name("test_name001");
    config.add_restart_process_name("test_name1");
    config.add_restart_process_name("render_service2");
    config.set_clock_id(FfrtProfilerConfig::MONOTONIC_COARSE);
    ffrtProfilerMgr->SetConfig(config);
    EXPECT_FALSE(ffrtProfilerMgr->CheckConfig());
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc: FfrtPofiler CheckConfig Function startup_process_name is exit return flase
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction005, TestSize.Level1)
{
    StartProcess(FFRT_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = 1;

    std::string cont;
    RunCommand("pidof render_service", cont);
    auto pidService = stoi(cont);

    config.add_pid(pid);
    config.add_pid(pidService);
    config.add_startup_process_name("test_name");
    config.add_startup_process_name("test_name001");
    config.add_restart_process_name("render_service");
    config.add_restart_process_name("test_name2");
    config.set_clock_id(FfrtProfilerConfig::MONOTONIC_COARSE);
    ffrtProfilerMgr->SetConfig(config);
    EXPECT_FALSE(ffrtProfilerMgr->CheckConfig());
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc: FfrtPofiler StartFfrtProfiler Function Test return false
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction006, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();

    FfrtProfilerConfig config;
    config.add_startup_process_name("test_name001");
    config.add_startup_process_name("test_name002");
    config.set_clock_id(FfrtProfilerConfig::REALTIME_COARSE);
    ffrtProfilerMgr->Init();
    EXPECT_FALSE(ffrtProfilerMgr->StartFfrtProfiler());
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc:  FfrtPofiler StartFfrtProfiler Function set smb_pages Test return true
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction007, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = 1;
    config.add_pid(pid);
    config.add_pid(ffrtPrfolerExePid_);
    config.add_startup_process_name("test_name001");
    config.add_startup_process_name("test_name002");
    config.set_clock_id(FfrtProfilerConfig::MONOTONIC_COARSE);
    config.set_smb_pages(4096);
    ffrtProfilerMgr->SetConfig(config);
    ffrtProfilerMgr->Init();
    EXPECT_TRUE(ffrtProfilerMgr->StartFfrtProfiler());
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc:  FfrtPofiler GetFfrtProfilerCtx Function Test
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction008, TestSize.Level1)
{
    StartProcess(FFRT_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = 1;
    std::string pidStr;
    RunCommand("pidof render_service", pidStr);
    auto pidService = stoi(pidStr);
    config.add_pid(pidService);
    config.add_startup_process_name("render_service");
    config.set_clock_id(FfrtProfilerConfig::REALTIME_COARSE);
    ffrtProfilerMgr->SetConfig(config);
    ffrtProfilerMgr->Init();
    ffrtProfilerMgr->CheckConfig();
    ffrtProfilerMgr->GetFfrtProfilerCtx(pid, "test_name005");
    EXPECT_TRUE(RunCommand(cmd, ret));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler RegisterAgentPlugin Function Test return false
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction011, TestSize.Level1)
{
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    ASSERT_TRUE(ffrtProfilerMgr != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(ffrtProfilerMgr);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_FALSE(commandPoller->OnConnect());
    ffrtProfilerMgr->SetCommandPoller(commandPoller);
    EXPECT_FALSE(ffrtProfilerMgr->RegisterAgentPlugin("ffrt-profiler"));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc: FfrtPofiler LoadPlugin Function Test return true
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction012, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::string pluginPath = std::string("libffrt_profiler.z.so");
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    EXPECT_TRUE(ffrtProfilerMgr->LoadPlugin(pluginPath));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc: FfrtPofiler UnloadPlugin Function Test return true
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction013, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::string pluginPath = std::string("libffrt_profiler.z.so");
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    EXPECT_TRUE(ffrtProfilerMgr->UnloadPlugin(pluginPath));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc: FfrtPofiler UnloadPlugin Function Test return true
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction014, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    EXPECT_TRUE(ffrtProfilerMgr->UnloadPlugin(ffrtPrfolerExePid_));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc: FfrtPofiler UnloadPlugin Function Test return true
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction015, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    EXPECT_TRUE(ffrtProfilerMgr->UnloadPlugin(ffrtPrfolerExePid_));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc: systemdata test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestSystemData, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = 1;
    config.add_pid(pid);
    config.add_pid(ffrtPrfolerExePid_);
    int size = config.ByteSizeLong();
    EXPECT_GT(size, 0);
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler CheckConfig Function Test return true
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction016, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = -1;
    config.add_pid(pid);
    config.add_pid(ffrtPrfolerExePid_);
    config.add_startup_process_name("");
    config.add_startup_process_name("test_name002");
    config.add_restart_process_name("");
    config.set_clock_id(FfrtProfilerConfig::MONOTONIC);
    ffrtProfilerMgr->SetConfig(config);
    EXPECT_TRUE(ffrtProfilerMgr->CheckConfig());
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler StopFfrtProfiler Function Test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction017, TestSize.Level1)
{
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    FfrtProfilerConfig config;
    int32_t pid = -1;
    config.add_pid(pid);
    config.add_pid(ffrtPrfolerExePid_);
    config.add_startup_process_name("");
    config.add_startup_process_name("test_name002");
    config.set_clock_id(FfrtProfilerConfig::MONOTONIC);
    ffrtProfilerMgr->SetConfig(config);
    ffrtProfilerMgr->StopFfrtProfiler();
    EXPECT_EQ(ffrtProfilerMgr->ffrtCtx_.size(), 0);
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler ReportPluginBasicData Function Test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction019, TestSize.Level1)
{
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::vector<uint32_t> pluginIds = {1, 2, 3};
    EXPECT_TRUE(ffrtProfilerMgr->ReportPluginBasicData(pluginIds));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler StopPluginSession Function Test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction020, TestSize.Level1)
{
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::vector<uint32_t> pluginIds = {1, 2, 3};
    EXPECT_TRUE(ffrtProfilerMgr->StopPluginSession(pluginIds));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler DestroyPluginSession Function Test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction021, TestSize.Level1)
{
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::vector<uint32_t> pluginIds = {1, 2, 3};
    EXPECT_TRUE(ffrtProfilerMgr->DestroyPluginSession(pluginIds));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler CreatePluginSession Function Test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction022, TestSize.Level1)
{
    const uint8_t configData[] = {0x30, 0x01, 0x38, 0x01, 0x42, 0x01, 0x01};
    ProfilerPluginConfig  ppc;
    std::vector<ProfilerPluginConfig> config;

    std::string pluginName = "ffrt-plugin";
    const std::vector<uint32_t> pluginIdsVector = {2};
    ppc.set_name(pluginName);
    ppc.set_config_data((const void*)configData, 7);
    config.push_back(ppc);
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    EXPECT_TRUE(ffrtProfilerMgr->CreatePluginSession(config));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler GetProcessName Function Test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction023, TestSize.Level1)
{
    std::string pid_str;
    RunCommand("pidof render_service", pid_str);
    auto pid_service = stoi(pid_str);
    auto res = GetProcessName(pid_service);
    EXPECT_EQ(res, "render_service");
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler SplitString Function Test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction024, TestSize.Level1)
{
    StartProcess(FFRT_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    string str = "ffrt_plugin_test_string";
    string seq = "_";
    std::vector<string> ret;
    SplitString(str, seq, ret);
    EXPECT_EQ(ret.size(), 4);
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler SplitString Function test string is empty
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction025, TestSize.Level1)
{
    StartProcess(FFRT_TEST_EXE, "100");
    std::string cmd = CreateCommand(OUTPUT_PATH, 10, "pid", std::to_string(ffrtPrfolerExePid_));
    string str = "";
    string seq = "_";
    std::vector<string> ret;
    SplitString(str, seq, ret);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc:  FfrtPofiler ProtocolProc Function Test
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction026, TestSize.Level1)
{
    uint64_t config = FILE_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB_SIZE;
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::shared_ptr<FfrtProfilerSocketService> socketService_ =
        std::make_shared<FfrtProfilerSocketService>(ffrtProfilerMgr);

    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(uint64_t);
    ASSERT_FALSE(socketService_->ProtocolProc(socketContext, 0, ptr, size));
}

// /**
//  * @tc.name: ffrt plugin
//  * @tc.desc:  FfrtPofiler ProtocolProc Function Test
//  * @tc.type: FUNC
//  */
HWTEST_F(FfrtPofilerTest, TestFunction027, TestSize.Level1)
{
    uint64_t config = FILE_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB_SIZE;
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::shared_ptr<FfrtProfilerSocketService> socketService_ =
        std::make_shared<FfrtProfilerSocketService>(ffrtProfilerMgr);

    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(int);
    socketService_->SetConfig(sizeof(uint64_t), sizeof(uint64_t), true, 0);
    ASSERT_FALSE(socketService_->ProtocolProc(socketContext, 0, ptr, size));
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc:  FfrtPofiler ProtocolProc Function Test
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction028, TestSize.Level1)
{
    uint64_t config = FILE_SIZE;
    config <<= MOBILE_BIT;
    config |= SMB_SIZE;
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::shared_ptr<FfrtProfilerSocketService> socketService_ =
        std::make_shared<FfrtProfilerSocketService>(ffrtProfilerMgr);

    SocketContext socketContext;
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    auto size = sizeof(int);
    ASSERT_FALSE(socketService_->ProtocolProc(socketContext, 0, ptr, size));
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc:  FfrtPofiler StartService Function Test
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction029, TestSize.Level1)
{
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::shared_ptr<FfrtProfilerSocketService> socketService_ =
        std::make_shared<FfrtProfilerSocketService>(ffrtProfilerMgr);
    ASSERT_FALSE(socketService_->StartService("ffrt_profiler_unix_socket"));
}

/**
 * @tc.name: ffrt plugin
 * @tc.desc:  FfrtPofiler FfrtProfilerManager SerializeData Function Test
 * @tc.type: FUNC
 */
HWTEST_F(FfrtPofilerTest, TestFunction030, TestSize.Level1)
{
    std::shared_ptr<FfrtProfilerManager> ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    std::shared_ptr<FfrtProfilerHandle> handle = std::make_shared<FfrtProfilerHandle>(BUFFER_SIZE, true);
    const int8_t data[] = {-1, 0, 1, 2, 3, 4};
    handle->SerializeData(data, MOBILE_BIT);
    FfrtProfilerConfig config;
    int32_t pid = -1;
    config.add_pid(pid);
    config.add_startup_process_name("test_name002");
    config.set_clock_id(FfrtProfilerConfig::MONOTONIC);
    ffrtProfilerMgr->SetConfig(config);
    EXPECT_TRUE(ffrtProfilerMgr->CheckConfig());
}
}