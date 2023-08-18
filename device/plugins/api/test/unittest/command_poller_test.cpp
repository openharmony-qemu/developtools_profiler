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

#include <gmock/gmock.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "command_poller.h"

#include "plugin_manager.h"
#include "plugin_service.ipc.h"
#include "socket_context.h"

using namespace testing::ext;

namespace {
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
    virtual bool UnloadPlugin(const std::string& pluginPath)
    {
        if (pluginPath == "existplugin") {
            return true;
        } else if (pluginPath == "noexistplugin") {
            return false;
        }
        return true;
    }
    virtual bool UnloadPlugin(const uint32_t pluginId)
    {
        if (pluginId == 0) {
            return false;
        }
        return true;
    }

    virtual bool CreatePluginSession(const std::vector<ProfilerPluginConfig>& config)
    {
        if (config[0].name() == "existplugin") {
            return true;
        } else if (config[0].name() == "noexistplugin") {
            return false;
        }
        return true;
    }
    virtual bool DestroyPluginSession(const std::vector<uint32_t>& pluginIds)
    {
        if (pluginIds[0] != 1) {
            return false;
        }
        return true;
    }
    virtual bool StartPluginSession(const std::vector<uint32_t>& pluginIds,
                                        const std::vector<ProfilerPluginConfig>& config, PluginResult& result)
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
    virtual bool StopPluginSession(const std::vector<uint32_t>& pluginIds)
    {
        if (pluginIds[0] == 0) {
            return false;
        }
        return true;
    }

    virtual bool CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd)
    {
        if (bufferSize == 0) {
            return false;
        }
        return true;
    }
    virtual bool ResetWriter(uint32_t pluginId)
    {
        if (pluginId == 0) {
            return false;
        }
        return true;
    }
    virtual void SetCommandPoller(const std::shared_ptr<CommandPoller>& p)
    {
        this->commandPoller_ = p;
    }

    virtual bool ReportPluginBasicData(const std::vector<uint32_t>& pluginIds)
    {
        return true;
    }
private:
    CommandPollerPtr commandPoller_;
};


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
}