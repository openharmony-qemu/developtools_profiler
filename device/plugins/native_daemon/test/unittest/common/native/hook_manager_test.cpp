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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "hook_manager.h"
#include "hook_service.h"
#include "hook_socket_client.h"
#include "socket_context.h"
#include "command_poller.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
const std::string DEFAULT_HIPROFILERD_PATH("/system/bin/hiprofilerd");
int g_hiprofilerdProcessNum = -1;

class HookManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        StartServerStub(DEFAULT_HIPROFILERD_PATH);
        sleep(1); // 睡眠1s确保hiprofilerd进程启动
    }
    static void TearDownTestCase()
    {
        StopServerStub(g_hiprofilerdProcessNum);
    }

    static void StartServerStub(std::string name)
    {
        int processNum = fork();
        HILOG_INFO(LOG_CORE, "processNum : %d", processNum);
        if (processNum == 0) {
            if (DEFAULT_HIPROFILERD_PATH == name) {
                // start running hiprofilerd
                execl(name.c_str(), nullptr, nullptr);
            }
            _exit(1);
        } else if (DEFAULT_HIPROFILERD_PATH == name) {
            g_hiprofilerdProcessNum = processNum;
        }
    }

    static void StopServerStub(int processNum)
    {
        std::string stopCmd = "kill " + std::to_string(processNum);
        HILOG_INFO(LOG_CORE, "stop command : %s", stopCmd.c_str());
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(stopCmd.c_str(), "r"), pclose);
    }
};

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
} // namespace
