/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include "command_poller.h"

#include <gtest/gtest.h>

#include "plugin_manager.h"
#include "plugin_service.ipc.h"
#include "socket_context.h"
#include "parameters.h"

using namespace testing::ext;

namespace {
constexpr int DEFAULT_SLEEP_TIME = 1000;
class PluginManagerStub final : public ManagerInterface {
public:
    bool LoadPlugin(const std::string& pluginPath) override
    {
        if (pluginPath == "existplugin") {
            return true;
        } else if (pluginPath == "noexistplugin") {
            return false;
        }
        return true;
    }
    bool UnloadPlugin(const std::string& pluginPath) override
    {
        if (pluginPath == "existplugin") {
            return true;
        } else if (pluginPath == "noexistplugin") {
            return false;
        }
        return true;
    }
    bool UnloadPlugin(const uint32_t pluginId) override
    {
        if (pluginId == 0) {
            return false;
        }
        return true;
    }

    bool CreatePluginSession(const std::vector<ProfilerPluginConfig>& config) override
    {
        if (config[0].name() == "existplugin") {
            return true;
        } else if (config[0].name() == "noexistplugin") {
            return false;
        }
        return true;
    }
    bool DestroyPluginSession(const std::vector<uint32_t>& pluginIds) override
    {
        if (pluginIds[0] != 1) {
            return false;
        }
        return true;
    }
    bool StartPluginSession(const std::vector<uint32_t>& pluginIds, const std::vector<ProfilerPluginConfig>& config,
                            PluginResult& result) override
    {
        if (pluginIds[0] == 0) {
            return false;
        }

        if (config[0].name() == "existplugin") {
            return true;
        } else if (config[0].name() == "noexistplugin") {
            return false;
        }
        return true;
    }
    bool StopPluginSession(const std::vector<uint32_t>& pluginIds) override
    {
        if (pluginIds[0] == 0) {
            return false;
        }
        return true;
    }

    bool CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd,
                      bool isProtobufSerialize = true) override
    {
        if (bufferSize == 0) {
            return false;
        }
        return true;
    }
    bool ResetWriter(uint32_t pluginId) override
    {
        if (pluginId == 0) {
            return false;
        }
        return true;
    }
    void SetCommandPoller(const std::shared_ptr<CommandPoller>& p) override
    {
        this->commandPoller_ = p;
    }

    bool ReportPluginBasicData(const std::vector<uint32_t>& pluginIds) override
    {
        return true;
    }

private:
    CommandPollerPtr commandPoller_;
};

static void StartHiprofilerService()
{
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
#ifdef COVERAGE_TEST
    const int coverageSleepTime = DEFAULT_SLEEP_TIME * 5;  // sleep 5s
    std::this_thread::sleep_for(std::chrono::milliseconds(coverageSleepTime));
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
#endif
}

static void StopHiprofilerService()
{
    OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
}

class CommandPollerTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

HWTEST_F(CommandPollerTest, CreateCmdTest, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManagerStub>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    pluginManage->SetCommandPoller(commandPoller);

    CreateSessionCmd successCmd;
    CreateSessionCmd failed1Cmd;
    CreateSessionCmd failed2Cmd;
    CreateSessionCmd failed3Cmd;
    SocketContext ctx;

    successCmd.add_buffer_sizes(1024);
    successCmd.add_plugin_configs()->set_name("existplugin");

    failed1Cmd.add_buffer_sizes(0);
    failed1Cmd.add_plugin_configs()->set_name("existplugin");

    failed2Cmd.add_buffer_sizes(0);
    failed2Cmd.add_plugin_configs()->set_name("noexistplugin");

    failed3Cmd.add_buffer_sizes(1);
    failed3Cmd.add_plugin_configs()->set_name("noexistplugin");
    EXPECT_TRUE(commandPoller->OnCreateSessionCmd(successCmd, ctx));
    EXPECT_FALSE(commandPoller->OnCreateSessionCmd(failed1Cmd, ctx));
    EXPECT_FALSE(commandPoller->OnCreateSessionCmd(failed2Cmd, ctx));
    EXPECT_FALSE(commandPoller->OnCreateSessionCmd(failed3Cmd, ctx));
}

HWTEST_F(CommandPollerTest, StartCmdTest, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManagerStub>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    pluginManage->SetCommandPoller(commandPoller);

    StartSessionCmd successCmd;
    successCmd.add_plugin_ids(1);
    successCmd.add_plugin_configs()->set_name("existplugin");
    StartSessionCmd failed1Cmd;

    failed1Cmd.add_plugin_ids(0);
    failed1Cmd.add_plugin_configs()->set_name("existplugin");

    StartSessionCmd failed2Cmd;
    failed2Cmd.add_plugin_ids(1);
    failed2Cmd.add_plugin_configs()->set_name("noexistplugin");

    PluginResult result;
    EXPECT_TRUE(commandPoller->OnStartSessionCmd(successCmd, result));
    EXPECT_FALSE(commandPoller->OnStartSessionCmd(failed1Cmd, result));
    EXPECT_FALSE(commandPoller->OnStartSessionCmd(failed2Cmd, result));
}

HWTEST_F(CommandPollerTest, StopCmdTest, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManagerStub>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    pluginManage->SetCommandPoller(commandPoller);

    StopSessionCmd successCmd;
    successCmd.add_plugin_ids(1);
    StopSessionCmd failedCmd;
    failedCmd.add_plugin_ids(0);
    EXPECT_TRUE(commandPoller->OnStopSessionCmd(successCmd));
    EXPECT_FALSE(commandPoller->OnStopSessionCmd(failedCmd));
}

HWTEST_F(CommandPollerTest, DestoryCmdTest, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManagerStub>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    pluginManage->SetCommandPoller(commandPoller);
    DestroySessionCmd successCmd;
    DestroySessionCmd failed1Cmd;
    DestroySessionCmd failed2Cmd;
    DestroySessionCmd failed3Cmd;
    successCmd.add_plugin_ids(1);
    failed1Cmd.add_plugin_ids(0);
    failed2Cmd.add_plugin_ids(2);
    failed3Cmd.add_plugin_ids(3);
    EXPECT_TRUE(commandPoller->OnDestroySessionCmd(successCmd));
    EXPECT_FALSE(commandPoller->OnDestroySessionCmd(failed1Cmd));
    EXPECT_FALSE(commandPoller->OnDestroySessionCmd(failed2Cmd));
    EXPECT_FALSE(commandPoller->OnDestroySessionCmd(failed3Cmd));
}

HWTEST_F(CommandPollerTest, RefreshSessionCmd, TestSize.Level1)
{
    StartHiprofilerService();
    auto pluginManage = std::make_shared<PluginManagerStub>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    EXPECT_TRUE(commandPoller->OnConnect());
    pluginManage->SetCommandPoller(commandPoller);
    ProfilerPluginState state;
    commandPoller->PushResult(state, 1);
    RefreshSessionCmd refreshCmd;
    refreshCmd.add_plugin_ids(1);
    EXPECT_TRUE(commandPoller->OnReportBasicDataCmd(refreshCmd));
    StopHiprofilerService();
}

HWTEST_F(CommandPollerTest, GetCommandResponse, TestSize.Level1)
{
    StartHiprofilerService();
    auto pluginManage = std::make_shared<PluginManagerStub>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    EXPECT_TRUE(commandPoller->OnConnect());
    pluginManage->SetCommandPoller(commandPoller);
    SocketContext ctx;
    GetCommandResponse cmdResponse;
    cmdResponse.set_command_id(1);
    EXPECT_FALSE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));
    CreateSessionCmd* csc = cmdResponse.mutable_create_session_cmd();
    csc->add_buffer_sizes(1024);  // 1024: buffer size
    csc->add_plugin_configs()->set_name("existplugin");
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));
    cmdResponse.clear_create_session_cmd();
    csc = cmdResponse.mutable_create_session_cmd();
    csc->add_buffer_sizes(0);
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));

    DestroySessionCmd* dsc = cmdResponse.mutable_destroy_session_cmd();
    dsc->add_plugin_ids(1);
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));
    cmdResponse.clear_destroy_session_cmd();
    dsc = cmdResponse.mutable_destroy_session_cmd();
    dsc->add_plugin_ids(0);
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));

    StartSessionCmd* ssc = cmdResponse.mutable_start_session_cmd();
    ssc->add_plugin_ids(1);
    ssc->add_plugin_configs()->set_name("existplugin");
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));
    cmdResponse.clear_start_session_cmd();
    ssc = cmdResponse.mutable_start_session_cmd();
    ssc->add_plugin_ids(0);
    ssc->add_plugin_configs()->set_name("notexistplugin");
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));
    StopHiprofilerService();
}

HWTEST_F(CommandPollerTest, GetCommandResponse2, TestSize.Level1)
{
    StartHiprofilerService();
    auto pluginManage = std::make_shared<PluginManagerStub>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    EXPECT_TRUE(commandPoller->OnConnect());
    pluginManage->SetCommandPoller(commandPoller);
    SocketContext ctx;
    GetCommandResponse cmdResponse;
    StopSessionCmd* stopSessionCmd = cmdResponse.mutable_stop_session_cmd();
    stopSessionCmd->add_plugin_ids(1);
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));

    cmdResponse.clear_stop_session_cmd();
    stopSessionCmd = cmdResponse.mutable_stop_session_cmd();
    stopSessionCmd->add_plugin_ids(0);
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));

    RefreshSessionCmd* refreshSessionCmd = cmdResponse.mutable_refresh_session_cmd();
    refreshSessionCmd->add_plugin_ids(1);
    EXPECT_TRUE(commandPoller->OnGetCommandResponse(ctx, cmdResponse));
    StopHiprofilerService();
}

} // namespace