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

#include "plugin_service.h"

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "common_types.pb.h"
#include "plugin_command_builder.h"
#include "plugin_service.ipc.h"
#include "plugin_service_impl.h"
#include "plugin_session_manager.h"
#include "profiler_data_repeater.h"
#include "profiler_service_types.pb.h"
#include "socket_context.h"

using namespace testing::ext;

namespace {
constexpr uint32_t BUFFER_SIZE = 4096;
constexpr size_t QUEUE_MAX_SIZE = 4096;
constexpr uint32_t SAMPLE_INTERVAL = 20;

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
        pluginService_ = std::make_shared<PluginService>();
        usleep(100000); // sleep for 100000 us.
        pluginClient_ = std::make_unique<PluginClientTest>();
        pluginServiceImp_ = std::make_unique<PluginServiceImpl>(*pluginService_);
        commandBuilder_ = std::make_unique<PluginCommandBuilder>();
        pluginSessionMgr_ = std::make_shared<PluginSessionManager>(pluginService_);
        pluginClient_->Connect(DEFAULT_UNIX_SOCKET_FULL_PATH);
        pluginService_->SetPluginSessionManager(pluginSessionMgr_);
    }

    void TearDown()
    {
        pluginService_ = nullptr;
        commandBuilder_ = nullptr;
        pluginSessionMgr_ = nullptr;
    }

public:
    std::shared_ptr<PluginService> pluginService_ = nullptr;
    std::unique_ptr<PluginClientTest> pluginClient_ = nullptr;
    std::unique_ptr<PluginServiceImpl> pluginServiceImp_ = nullptr;
    std::unique_ptr<PluginCommandBuilder> commandBuilder_ = nullptr;
    std::shared_ptr<PluginSessionManager> pluginSessionMgr_ = nullptr;
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
    ppc.set_name("abc1.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(SAMPLE_INTERVAL);
    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
    PluginInfo plugin;
    plugin.name = "abc1.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = (SocketContext*)pluginClient_->unixSocketClient_.get();
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
    ASSERT_TRUE(plugin.context != nullptr);
    EXPECT_TRUE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
    pluginService_->RemovePluginInfo(plugin);
}

/**
 * @tc.name: Service
 * @tc.desc: Set session configuration.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, CreatePluginSession2, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("abc2.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(SAMPLE_INTERVAL);
    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);
    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
    PluginInfo plugin;
    plugin.name = "abc2.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = (SocketContext*)pluginClient_->unixSocketClient_.get();
    pluginService_->AddPluginInfo(plugin);
    ProfilerSessionConfig psc;
    psc.set_session_mode(ProfilerSessionConfig::OFFLINE);
    pluginService_->SetProfilerSessionConfig(psc);
    TraceFileWriterPtr traceWriter = std::make_shared<TraceFileWriter>("/data/local/tmp/tracewrite.trace");
    pluginService_->SetTraceWriter(traceWriter);
    EXPECT_TRUE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
    uint32_t pluginId = pluginService_->GetPluginIdByName("abc2.so");
    PluginContextPtr pluginCtx = pluginService_->GetPluginContextById(pluginId);
    pluginService_->ReadShareMemoryOffline(*pluginCtx);
    EXPECT_TRUE(pluginService_->DestroyPluginSession("abc2.so"));
    EXPECT_TRUE(pluginService_->RemovePluginSessionCtx("abc2.so"));
    pluginService_->RemovePluginInfo(plugin);
}

/**
 * @tc.name: Service
 * @tc.desc: Start plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StartPluginSession, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("abc3.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(SAMPLE_INTERVAL);
    ASSERT_FALSE(pluginService_->StartPluginSession(ppc));
    PluginInfo plugin;
    plugin.name = "abc3.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = (SocketContext*)pluginClient_->unixSocketClient_.get();
    pluginService_->AddPluginInfo(plugin);
    EXPECT_TRUE(pluginService_->StartPluginSession(ppc));
}

/**
 * @tc.name: Service
 * @tc.desc: Stop plugin session.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, StopPluginSession, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->StopPluginSession("abc.so"));
    PluginInfo plugin;
    SocketContext ctx;
    plugin.name = "abc4.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = &ctx;
    pluginService_->AddPluginInfo(plugin);
    EXPECT_FALSE(pluginService_->StopPluginSession("abc4.so"));
}

/**
 * @tc.name: Service
 * @tc.desc: Destroy receiving test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, DestroyPluginSession, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->DestroyPluginSession("abc.so"));
    PluginInfo plugin;
    plugin.name = "abc5.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = (SocketContext*)pluginClient_->unixSocketClient_.get();
    pluginService_->AddPluginInfo(plugin);
    EXPECT_TRUE(pluginService_->DestroyPluginSession("abc5.so"));
}
/**
 * @tc.name: Service
 * @tc.desc: RefreshPluginSession  test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, RefreshPluginSession, TestSize.Level1)
{
    ASSERT_FALSE(pluginService_->RefreshPluginSession("abc.so"));
    PluginInfo plugin;
    SocketContext ctx;
    plugin.name = "abc6.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = (SocketContext*)pluginClient_->unixSocketClient_.get();
    pluginService_->AddPluginInfo(plugin);
    EXPECT_TRUE(pluginService_->RefreshPluginSession("abc6.so"));
}
/**
 * @tc.name: Service
 * @tc.desc: GetPluginInfo  test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, GetPluginInfo, TestSize.Level1)
{
    PluginInfo plugin;
    EXPECT_FALSE(pluginService_->GetPluginInfo("edf.so", plugin));
    SocketContext ctx;
    plugin.name = "abc7.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = &ctx;
    pluginService_->AddPluginInfo(plugin);
    EXPECT_TRUE(pluginService_->GetPluginInfo("abc7.so", plugin));
}

/**
 * @tc.name: Service
 * @tc.desc: RemovePluginSessionCtx  test.
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, RemovePluginSessionCtx, TestSize.Level1)
{
    PluginInfo plugin;
    SocketContext ctx;
    plugin.name = "abc8.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = &ctx;
    pluginService_->AddPluginInfo(plugin);
    EXPECT_TRUE(pluginService_->RemovePluginSessionCtx("abc8.so"));
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
    pi.name = "abc9.so";
    pi.path = "abc9.so";
    pi.sha256 = "asdfasdf";
    ASSERT_FALSE(pluginService_->RemovePluginInfo(pi));
    pluginService_->AddPluginInfo(pi);
    pi.id = pluginService_->GetPluginIdByName("abc9.so");
    EXPECT_TRUE(pluginService_->RemovePluginInfo(pi));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

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
    plugin.bufferSizeHint = 10;
    plugin.isStandaloneFileData = true;
    plugin.outFileName = "test-file.bin";
    plugin.pluginVersion = "1.02";
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    ASSERT_FALSE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
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
    ppc.set_sample_interval(SAMPLE_INTERVAL);

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
    UnregisterPluginRequest unRequest;
    UnregisterPluginResponse unResponse;
    uint32_t plugId = pluginService_->GetPluginIdByName("sample2.so");
    unRequest.set_plugin_id(plugId);
    EXPECT_TRUE(pluginServiceImp_->UnregisterPlugin(ctx, unRequest, unResponse));
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

    GetCommandRequest gcRequest;
    GetCommandResponse gcResponse;
    EXPECT_FALSE(pluginServiceImp_->GetCommand(ctx, gcRequest, gcResponse));

    NotifyResultRequest nResRequest;
    NotifyResultResponse nResResponse;
    EXPECT_TRUE(pluginServiceImp_->NotifyResult(ctx, nResRequest, nResResponse));
}
/**
 * @tc.name: BuildCreateSessionCmd
 * @tc.desc: build create session command
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, pluginCommandBuilder, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("abc.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(SAMPLE_INTERVAL);
    const uint32_t pluginId = 1;
    EXPECT_TRUE(commandBuilder_->BuildCreateSessionCmd(ppc, BUFFER_SIZE) != nullptr);
    EXPECT_TRUE(commandBuilder_->BuildStartSessionCmd(ppc, pluginId) != nullptr);
    EXPECT_TRUE(commandBuilder_->BuildRefreshSessionCmd(pluginId) != nullptr);
    EXPECT_TRUE(commandBuilder_->BuildStopSessionCmd(pluginId) != nullptr);
    EXPECT_TRUE(commandBuilder_->BuildDestroySessionCmd(pluginId) != nullptr);
    EXPECT_FALSE(commandBuilder_->GetedCommandResponse(0));
    EXPECT_TRUE(commandBuilder_->GetedCommandResponse(1));
}

/**
 * @tc.name: PluginSessionManager
 * @tc.desc: test plugin session manager checkBufferConfig
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, PluginSessionManager_Check, TestSize.Level1)
{
    PluginInfo plugin;
    SocketContext ctx;
    plugin.name = "abc_check.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "asdfasdfasdfasfd";
    plugin.context = &ctx;
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);
    EXPECT_TRUE(pluginSessionMgr_->CheckBufferConfig(bc));
    ProfilerPluginConfig ppc;
    ppc.set_name("abc_check.so");
    ppc.set_plugin_sha256("asdfasdfasdfasfd");
    EXPECT_TRUE(pluginSessionMgr_->CheckPluginSha256(ppc));
}
/**
 * @tc.name: PluginSessionManager
 * @tc.desc: test plugin session manager by creating sesssion
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, PluginSessionManager_CreateSession, TestSize.Level1)
{
    PluginInfo plugin;
    plugin.name = "testsample.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = (SocketContext*)pluginClient_->unixSocketClient_.get();
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
    ProfilerPluginConfig ppc;
    ppc.set_name("testsample.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);
    EXPECT_TRUE(pluginSessionMgr_->CreatePluginSession(
                    ppc, bc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)) != nullptr);
}
/**
 * @tc.name: PluginSessionManager
 * @tc.desc: test plugin session manager by creating a lot of sesssions
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, PluginSessionManager_CreateSessions, TestSize.Level1)
{
    PluginInfo plugin;
    plugin.name = "testsample1.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = (SocketContext*)pluginClient_->unixSocketClient_.get();
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
    ProfilerPluginConfig ppc;
    ppc.set_name("testsample1.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(SAMPLE_INTERVAL);

    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);
    std::vector<ProfilerPluginConfig> vecConfig;
    std::vector<ProfilerSessionConfig::BufferConfig> vecBuf;
    vecConfig.push_back(ppc);
    vecBuf.push_back(bc);
    EXPECT_TRUE(pluginSessionMgr_->CreatePluginSessions(vecConfig, vecBuf,
                                                        std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
    std::vector<std::string> nameList{"testsample.so", "testsample1.so"};
    EXPECT_TRUE(pluginSessionMgr_->InvalidatePluginSessions(nameList));
    std::vector<PluginSession::State> vecRet = pluginSessionMgr_->GetStatus(nameList);
    EXPECT_TRUE(vecRet.size() != 0);
    EXPECT_TRUE(pluginSessionMgr_->RefreshPluginSession());
}

/**
 * @tc.name: plugin service
 * @tc.desc: test createpluginsession online
 * @tc.type: FUNC
 */
HWTEST_F(UnitTestPluginService, PluginService_CreatePluginSession_Online, TestSize.Level1)
{
    ProfilerPluginConfig ppc;
    ppc.set_name("abconline.so");
    ppc.set_plugin_sha256("ASDFAADSF");
    ppc.set_sample_interval(SAMPLE_INTERVAL);
    ProfilerSessionConfig::BufferConfig bc;
    bc.set_pages(1);
    bc.set_policy(ProfilerSessionConfig_BufferConfig_Policy_RECYCLE);

    PluginInfo plugin;
    plugin.name = "abconline.so";
    plugin.bufferSizeHint = 0;
    plugin.sha256 = "ASDFAADSF";
    plugin.context = (SocketContext*)pluginClient_->unixSocketClient_.get();
    EXPECT_TRUE(pluginService_->AddPluginInfo(plugin));
    ProfilerSessionConfig psc;
    psc.set_session_mode(ProfilerSessionConfig::ONLINE);
    pluginService_->SetProfilerSessionConfig(psc);
    EXPECT_TRUE(pluginService_->CreatePluginSession(ppc, bc, std::make_shared<ProfilerDataRepeater>(QUEUE_MAX_SIZE)));
    uint32_t pluginId = pluginService_->GetPluginIdByName("abconline.so");
    PluginContextPtr pluginCtx = pluginService_->GetPluginContextById(pluginId);
    pluginCtx->profilerPluginState.set_state(ProfilerPluginState::LOADED);
    pluginService_->ReadShareMemoryOnline(*pluginCtx);
    NotifyResultRequest nrr;
    nrr.set_request_id(5);
    nrr.set_command_id(5);
    PluginResult* result = nrr.add_result();
    result->set_plugin_id(pluginId);
    result->set_out_file_name("/data/local/tmp/testonline.trace");
    result->set_data("test plugin result!");
    EXPECT_TRUE(pluginService_->AppendResult(nrr));
    plugin.id = pluginId;
    EXPECT_TRUE(pluginService_->RemovePluginInfo(plugin));
}
} // namespace