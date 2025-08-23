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

#include <gtest/gtest.h>
#include "command_poller.h"
#include "hook_manager.h"
#include "hook_service.h"
#include "hook_socket_client.h"
#include "parameters.h"
#include "socket_context.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;
namespace {
const std::string OUTPUT_PATH = "/data/local/tmp/hiprofiler_data.htrace";
const int SMB_PAGES = 16384;
class HookManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
#ifdef COVERAGE_TEST
        const int coverageSleepTime = 5; // sleep 5s
        sleep(coverageSleepTime);
#else
        sleep(1); // 睡眠1s确保hiprofilerd进程启动
#endif
    }
    static void TearDownTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
    }
};

std::string CreateCommand(const std::string& outputFile, const int32_t time, const std::string& processName)
{
    std::ostringstream cmdStream;
    cmdStream << "hiprofiler_cmd \\\n"
              << "-c - \\\n"
              << "-o " << outputFile << " \\\n"
              << "-t " << time << " \\\n"
              << "-s \\\n"
              << "-k \\\n"
              << "<<CONFIG\n"
              << "request_id: 1\n"
              << "session_config {\n"
              << "  buffers {\n"
              << "    pages: 14848" << "\n"
              << "  }\n"
              << "}\n"
              << "plugin_configs {\n"
              << "  plugin_name: \"nativehook\"\n"
              << "  config_data {\n"
              << "process_name: \"" << processName << "\"\n"
              << "smb_pages: " << SMB_PAGES << "\n"
              << "dump_nmd: true\n"
              << "  }\n"
              << "}\n"
              << "CONFIG\n";
    return cmdStream.str();
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

/*
 * @tc.name: RegisterPlugin
 * @tc.desc: test HookManager::RegisterAgentPlugin with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, RegisterPlugin, TestSize.Level1)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(hookManager);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_TRUE(commandPoller->OnConnect());
    hookManager->SetCommandPoller(commandPoller);
    ASSERT_TRUE(hookManager->RegisterAgentPlugin("nativehook"));
    ASSERT_TRUE(hookManager->UnregisterAgentPlugin("nativehook"));
}

/*
 * @tc.name: LoadPlugin
 * @tc.desc: test HookManager::LoadPlugin with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, LoadPlugin, TestSize.Level1)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(hookManager);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_TRUE(commandPoller->OnConnect());
    hookManager->SetCommandPoller(commandPoller);
    ASSERT_TRUE(hookManager->RegisterAgentPlugin("nativehook"));
    ASSERT_TRUE(hookManager->LoadPlugin("nativehook"));
    ASSERT_TRUE(hookManager->UnloadPlugin("nativehook"));
    ASSERT_TRUE(hookManager->UnregisterAgentPlugin("nativehook"));
}

/*
 * @tc.name: UnloadPlugin
 * @tc.desc: test HookManager::UnloadPlugin with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, UnloadPlugin, TestSize.Level1)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(hookManager);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_TRUE(commandPoller->OnConnect());
    hookManager->SetCommandPoller(commandPoller);
    ASSERT_TRUE(hookManager->RegisterAgentPlugin("nativehook"));
    ASSERT_TRUE(hookManager->LoadPlugin("nativehook"));
    ASSERT_TRUE(hookManager->UnloadPlugin(commandPoller->GetRequestId()));
    ASSERT_TRUE(hookManager->UnregisterAgentPlugin("nativehook"));
}

/*
 * @tc.name: PluginSession
 * @tc.desc: test HookManager process with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, PluginSession, TestSize.Level1)
{
    std::shared_ptr<HookManager> hookManager = std::make_shared<HookManager>();
    ASSERT_TRUE(hookManager != nullptr);
    std::shared_ptr<CommandPoller> commandPoller = std::make_shared<CommandPoller>(hookManager);
    ASSERT_TRUE(commandPoller != nullptr);
    EXPECT_TRUE(commandPoller->OnConnect());
    hookManager->SetCommandPoller(commandPoller);

    std::vector<uint32_t> pluginIds(1);
    ProfilerPluginConfig config;
    config.set_name("nativehook");
    config.set_plugin_sha256("");
    config.set_sample_interval(20);

    PluginResult result;
    std::vector<ProfilerPluginConfig> configVec;
    configVec.push_back(config);

    EXPECT_FALSE(hookManager->CreatePluginSession(configVec));
    EXPECT_FALSE(hookManager->StartPluginSession(pluginIds, configVec, result));
    EXPECT_TRUE(hookManager->CreateWriter("name", 0, 0, 0));
    EXPECT_TRUE(hookManager->ResetWriter(0));
    EXPECT_FALSE(hookManager->StopPluginSession(pluginIds));
    EXPECT_TRUE(hookManager->DestroyPluginSession(pluginIds));
}

/*
 * @tc.name: CheckProcess
 * @tc.desc: test CheckProcess with false case.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, CheckProcess, TestSize.Level1)
{
    HookManager hookManager;
    NativeHookConfig nativeConfig;
    nativeConfig.set_process_name("HookManagerTest");
    hookManager.SetHookConfig(nativeConfig);
    EXPECT_TRUE(hookManager.CheckProcess());

    nativeConfig.set_startup_mode(true);
    hookManager.SetHookConfig(nativeConfig);
    EXPECT_TRUE(hookManager.CheckProcess());
    hookManager.ResetStartupParam();

    // native_daemon_ut as a testing process
    nativeConfig.set_startup_mode(false);
    nativeConfig.set_process_name("native_daemon_ut");
    hookManager.SetHookConfig(nativeConfig);
    EXPECT_TRUE(hookManager.CheckProcess());
    EXPECT_TRUE(hookManager.CheckProcessName());
}

/*
 * @tc.name: CheckNmdInfo
 * @tc.desc: test CheckNmdInfoe when process is exit.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(HookManagerTest, CheckNmdInfo, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 1, "hiview");
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    EXPECT_TRUE(ret.find("FAIL") == std::string::npos);
    std::string filePath = "/data/local/tmp/nmd_hiview.txt";
    EXPECT_EQ(access(filePath.c_str(), F_OK), 0);

    std::ifstream infile;
    infile.open(filePath, std::ios::in);
    EXPECT_TRUE(infile.is_open());
    std::string buf;
    bool nmdResult = false;
    while (getline(infile, buf)) {
        if (buf.find("jemalloc statistics") != std::string::npos) {
            nmdResult = true;
            break;
        }
    }
    EXPECT_TRUE(nmdResult);
}
#endif

/*
 * @tc.name: CheckNmdInfoe002
 * @tc.desc: test CheckNmdInfoe002 when process is not exit.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerTest, CheckNmdInfo002, TestSize.Level1)
{
    std::string cmd = CreateCommand(OUTPUT_PATH, 1, "test_profiler");
    std::string ret;
    EXPECT_TRUE(RunCommand(cmd, ret));
    EXPECT_TRUE(ret.find("FAIL") == std::string::npos);
    std::string filePath = "/data/local/tmp/test_profiler.txt";
    EXPECT_EQ(access(filePath.c_str(), F_OK), -1);
}
} // namespace