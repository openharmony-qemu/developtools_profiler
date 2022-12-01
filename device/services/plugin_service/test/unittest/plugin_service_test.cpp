/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "common_types.pb.h"
#include "plugin_service.h"
#include "plugin_service.ipc.h"
#include "profiler_data_repeater.h"
#include "profiler_service_types.pb.h"
#include "socket_context.h"
#include "plugin_service_impl.h"

using namespace testing::ext;

namespace {
class PluginClientTest final : public IPluginServiceClient {
public:
    bool OnGetCommandResponse(SocketContext& context, ::GetCommandResponse& response) override
    {
        return true;
    }
};

class UnitTestPluginService : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp()
    {
        pluginService_ = std::make_unique<PluginService>();
        usleep(100000); // sleep for 100000 us.
        pluginClient_ = std::make_unique<PluginClientTest>();
        pluginServiceImp_ = std::make_unique<PluginServiceImpl>(*pluginService_);
    }

    void TearDown()
    {
        pluginClient_ = nullptr;
        pluginService_ = nullptr;
    }

public:
    std::unique_ptr<PluginService> pluginService_ = nullptr;
    std::unique_ptr<PluginClientTest> pluginClient_ = nullptr;
    std::unique_ptr<PluginServiceImpl> pluginServiceImp_ = nullptr;
    uint32_t pluginId_;
};

/**
 * @tc.name: Service
 * @tc.desc: Set plugin registration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AddPluginInfo, TestSize.Level1)
{
    RegisterPluginRequest request;
    RegisterPluginResponse response;
    PluginInfo plugin;

    request.set_request_id(1);
    request.set_path("abc.so");
    request.set_sha256("asdfasdfasdfasfd");
    request.set_name("abc.so");
    ASSERT_TRUE(response.status() != ResponseStatus::OK);
    pluginId_ = response.plugin_id();

    plugin.name = "abc.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "";
    ASSERT_TRUE(pluginService_->AddPluginInfo(plugin));
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin configuration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession1, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("abc.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Set session configuration.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession2, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("abc.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Start plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StartPluginSession, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("abc.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->StartPluginSession(ppc));
}

/**
 * @tc.name: Service
 * @tc.desc: Stop plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StopPluginSession, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->StopPluginSession("abc.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Destroy receiving test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, DestroyPluginSession, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->DestroyPluginSession("abc.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Remove the specified plugin.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, RemovePluginInfo, TestSize.Level1)
{
    UnregisterPluginRequest request;
    PluginInfo pi;
    pi.id = 0;
    pi.name = "abc.so";
    pi.path = "abc.so";
    pi.sha256 = "asdfasdf";
    ASSERT_FALSE(pluginService_->RemovePluginInfo(pi));
}

/**
 * @tc.name: Service
 * @tc.desc: Setting report results.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AppendResult, TestSize.Level1)
{
    NotifyResultRequest nrr;
    nrr.set_request_id(1);
    nrr.set_command_id(1);
    ASSERT_TRUE(pluginService_->AppendResult(nrr));
}

/**
 * @tc.name: Service
 * @tc.desc: Get plugin status.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginStatus, TestSize.Level1)
{
    auto status = pluginService_->GetPluginStatus();
    ASSERT_TRUE(status.size() == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the plugin with the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginIdByName, TestSize.Level1)
{
    ASSERT_TRUE(pluginService_->GetPluginIdByName("abc.so") == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin registration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AddPluginInfo2, TestSize.Level1)
{
    RegisterPluginRequest request;
    RegisterPluginResponse response;
    PluginInfo plugin;

    request.set_request_id(2);
    request.set_path("mem.so");
    request.set_sha256("asdfasdfasdfasfd");
    request.set_name("mem.so");
    ASSERT_TRUE(response.status() != ResponseStatus::OK);
    pluginId_ = response.plugin_id();

    plugin.name = "mem.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "";
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin configuration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession12, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("mem.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Set session configuration.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession22, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("mem.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Start plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StartPluginSession2, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("mem.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->StartPluginSession(ppc));
}

/**
 * @tc.name: Service
 * @tc.desc: Stop plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StopPluginSession2, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->StopPluginSession("mem.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Destroy receiving test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, DestroyPluginSession2, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->DestroyPluginSession("mem.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Remove the specified plugin.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, RemovePluginInfo2, TestSize.Level1)
{
    UnregisterPluginRequest request;
    PluginInfo pi;
    pi.id = 0;
    pi.name = "mem.so";
    pi.path = "mem.so";
    pi.sha256 = "asdfasdf";
    ASSERT_FALSE(pluginService_->RemovePluginInfo(pi));
}

/**
 * @tc.name: Service
 * @tc.desc: Setting report results.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AppendResult2, TestSize.Level1)
{
    NotifyResultRequest nrr;
    nrr.set_request_id(2);
    nrr.set_command_id(2);
    ASSERT_TRUE(pluginService_->AppendResult(nrr));
}

/**
 * @tc.name: Service
 * @tc.desc: Get plugin status.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginStatus2, TestSize.Level1)
{
    auto status = pluginService_->GetPluginStatus();
    ASSERT_TRUE(status.size() == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the plugin with the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginIdByName2, TestSize.Level1)
{
    ASSERT_TRUE(pluginService_->GetPluginIdByName("mem.so") == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin registration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AddPluginInfo3, TestSize.Level1)
{
    RegisterPluginRequest request;
    RegisterPluginResponse response;
    PluginInfo plugin;

    request.set_request_id(3);
    request.set_path("cpu.so");
    request.set_sha256("asdfasdfasdfasfd");
    request.set_name("cpu.so");
    ASSERT_TRUE(response.status() != ResponseStatus::OK);
    pluginId_ = response.plugin_id();

    plugin.name = "cpu.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "";
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin configuration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession13, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("cpu.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Set session configuration.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession23, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("cpu.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Start plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StartPluginSession3, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("cpu.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->StartPluginSession(ppc));
}

/**
 * @tc.name: Service
 * @tc.desc: Stop plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StopPluginSession3, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->StopPluginSession("cpu.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Destroy receiving test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, DestroyPluginSession3, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->DestroyPluginSession("cpu.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Remove the specified plugin.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, RemovePluginInfo3, TestSize.Level1)
{
    UnregisterPluginRequest request;
    PluginInfo pi;
    pi.id = 0;
    pi.name = "cpu.so";
    pi.path = "cpu.so";
    pi.sha256 = "asdfasdf";
    ASSERT_FALSE(pluginService_->RemovePluginInfo(pi));
}

/**
 * @tc.name: Service
 * @tc.desc: Setting report results.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AppendResult3, TestSize.Level1)
{
    NotifyResultRequest nrr;
    nrr.set_request_id(3);
    nrr.set_command_id(3);
    ASSERT_TRUE(pluginService_->AppendResult(nrr));
}

/**
 * @tc.name: Service
 * @tc.desc: Get plugin status.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginStatus3, TestSize.Level1)
{
    auto status = pluginService_->GetPluginStatus();
    ASSERT_TRUE(status.size() == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the plugin with the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginIdByName3, TestSize.Level1)
{
    ASSERT_TRUE(pluginService_->GetPluginIdByName("cpu.so") == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin registration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AddPluginInfo4, TestSize.Level1)
{
    RegisterPluginRequest request;
    RegisterPluginResponse response;
    PluginInfo plugin;

    request.set_request_id(4);
    request.set_path("stream.so");
    request.set_sha256("asdfasdfasdfasfd");
    request.set_name("stream.so");
    ASSERT_TRUE(response.status() != ResponseStatus::OK);
    pluginId_ = response.plugin_id();

    plugin.name = "stream.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "";
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin configuration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession14, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("stream.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Set session configuration.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession24, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("stream.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Start plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StartPluginSession4, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("stream.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->StartPluginSession(ppc));
}

/**
 * @tc.name: Service
 * @tc.desc: Stop plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StopPluginSession4, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->StopPluginSession("stream.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Destroy receiving test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, DestroyPluginSession4, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->DestroyPluginSession("stream.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Remove the specified plugin.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, RemovePluginInfo4, TestSize.Level1)
{
    UnregisterPluginRequest request;
    PluginInfo pi;
    pi.id = 0;
    pi.name = "stream.so";
    pi.path = "stream.so";
    pi.sha256 = "asdfasdf";
    ASSERT_FALSE(pluginService_->RemovePluginInfo(pi));
}

/**
 * @tc.name: Service
 * @tc.desc: Setting report results.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AppendResult4, TestSize.Level1)
{
    NotifyResultRequest nrr;
    nrr.set_request_id(4);
    nrr.set_command_id(4);
    ASSERT_TRUE(pluginService_->AppendResult(nrr));
}

/**
 * @tc.name: Service
 * @tc.desc: Get plugin status.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginStatus4, TestSize.Level1)
{
    auto status = pluginService_->GetPluginStatus();
    ASSERT_TRUE(status.size() == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the plugin with the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginIdByName4, TestSize.Level1)
{
    ASSERT_TRUE(pluginService_->GetPluginIdByName("stream.so") == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin registration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AddPluginInfo5, TestSize.Level1)
{
    RegisterPluginRequest request;
    RegisterPluginResponse response;
    PluginInfo plugin;

    request.set_request_id(5);
    request.set_path("sample.so");
    request.set_sha256("asdfasdfasdfasfd");
    request.set_name("sample.so");
    ASSERT_TRUE(response.status() != ResponseStatus::OK);
    pluginId_ = response.plugin_id();

    plugin.name = "sample.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "";
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
}

/**
 * @tc.name: Service
 * @tc.desc: Set plugin configuration information.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession15, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("sample.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Set session configuration.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession25, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("sample.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(4096)));
}

/**
 * @tc.name: Service
 * @tc.desc: Start plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StartPluginSession5, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("sample.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(20);

    ASSERT_FALSE(pluginService_->StartPluginSession(ppc));
}

/**
 * @tc.name: Service
 * @tc.desc: Stop plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StopPluginSession5, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->StopPluginSession("sample.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Destroy receiving test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, DestroyPluginSession5, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->DestroyPluginSession("sample.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Remove the specified plugin.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, RemovePluginInfo5, TestSize.Level1)
{
    UnregisterPluginRequest request;
    PluginInfo pi;
    pi.id = 0;
    pi.name = "sample.so";
    pi.path = "sample.so";
    pi.sha256 = "asdfasdf";
    ASSERT_FALSE(pluginService_->RemovePluginInfo(pi));
}

/**
 * @tc.name: Service
 * @tc.desc: Setting report results.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, AppendResult5, TestSize.Level1)
{
    NotifyResultRequest nrr;
    nrr.set_request_id(5);
    nrr.set_command_id(5);
    ASSERT_TRUE(pluginService_->AppendResult(nrr));
}

/**
 * @tc.name: Service
 * @tc.desc: Get plugin status.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginStatus5, TestSize.Level1)
{
    auto status = pluginService_->GetPluginStatus();
    ASSERT_TRUE(status.size() == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the plugin with the specified name.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginIdByName5, TestSize.Level1)
{
    ASSERT_TRUE(pluginService_->GetPluginIdByName("sample.so") == 0);
}

/**
 * @tc.name: Service
 * @tc.desc: test function RegisterPlugin.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, PluginServiceImpl_RegisterPlugin, TestSize.Level1)
{
    RegisterPluginRequest request;
    RegisterPluginResponse response;
    SocketContext ctx;

    request.set_request_id(5);
    request.set_path("sample2.so");
    request.set_sha256("");
    request.set_name("sample2.so");
    request.set_buffer_size_hint(0);
    EXPECT_TRUE(pluginServiceImp_->RegisterPlugin(ctx, request, response));
    request.set_sha256("abcdsfdsfad");
    EXPECT_FALSE(pluginServiceImp_->RegisterPlugin(ctx, request, response));
}

/**
 * @tc.name: Service
 * @tc.desc: test function UnregisterPlugin
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, PluginServiceImpl_UnregisterPlugin, TestSize.Level1)
{
    UnregisterPluginRequest request;
    UnregisterPluginResponse response;
    SocketContext ctx;
    request.set_plugin_id(1);
    EXPECT_FALSE(pluginServiceImp_->UnregisterPlugin(ctx, request, response));
}
} // namespace