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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "plugin_service.ipc.h"
#include "socket_context.h"

using namespace testing::ext;
namespace {
uint32_t g_pluginId;

class PluginClient final : public IPluginServiceClient {
public:
    bool OnRegisterPluginResponse(SocketContext& context, ::RegisterPluginResponse& response) override
    {
        g_pluginId = response.plugin_id();
        return true;
    }
    bool OnUnregisterPluginResponse(SocketContext& context, ::UnregisterPluginResponse& response) override
    {
        return true;
    }
    bool OnGetCommandResponse(SocketContext& context, ::GetCommandResponse& response) override
    {
        return true;
    }
    bool OnNotifyResultResponse(SocketContext& context, ::NotifyResultResponse& response) override
    {
        return true;
    }
};

std::unique_ptr<PluginClient> g_pluginClient;

class ModuleTestPluginService : public testing::Test {
public:
    static void SetUpTestCase()
    {
        g_pluginClient = std::make_unique<PluginClient>();
        ASSERT_FALSE(g_pluginClient->Connect("test"));
    }

    static void TearDownTestCase()
    {
        usleep(1000000); // sleep 1000000 us.
        g_pluginClient = nullptr;
    }
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(ModuleTestPluginService, RegisterPlugin, TestSize.Level1)
{
    RegisterPluginRequest request;
    RegisterPluginResponse response;

    request.set_request_id(1);
    request.set_path("abc.so");
    request.set_sha256("ADSFAFASFASFASF");
    request.set_name("abc.so");
    ASSERT_TRUE(response.status() != ResponseStatus::OK);
    g_pluginId = response.plugin_id();
}
} // namespace