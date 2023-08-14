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

#define final // disable final keyword
#define LOG_TAG "ProfilerServiceTest"

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <thread>

#include "logging.h"
#include "plugin_service.h"
#include "profiler_capability_manager.h"
#include "profiler_data_repeater.h"
#include "profiler_service.h"
#include "schedule_task_manager.h"
#include "common.h"

using namespace testing::ext;

using ProfilerServicePtr = STD_PTR(shared, ProfilerService);

namespace {
constexpr int ROUND_COUNT = 200;
constexpr int FETCH_DATA_DELAY_US = 100 * 1000;
constexpr int TEST_SESSION_TIMEOUT_MS = 1000;
constexpr int TIMEOUT_US = 3 * 1000 * 1000;

class Timer {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    Timer() : startTime_(Now()) {}

    ~Timer() {}

    long ElapsedUs()
    {
        auto currentTime = Now();
        return std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime_).count();
    }

    void Reset()
    {
        startTime_ = Now();
    }

protected:
    TimePoint Now()
    {
        return Clock::now();
    }

private:
    TimePoint startTime_;
};

class ProfilerServiceTest : public ::testing::Test {
protected:
    PluginServicePtr pluginService_;
    ProfilerServicePtr service_;

    PluginInfo pluginInfo;
    std::unique_ptr<grpc::ServerContext> context_;
    std::atomic<int> requestCounter = 0;

    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        pluginInfo.name = "test_plugin";
        pluginService_ = std::make_shared<PluginService>();
        if (pluginService_) {
            pluginService_->AddPluginInfo(pluginInfo);
        }
        service_ = std::make_shared<ProfilerService>(pluginService_);
        context_ = std::make_unique<grpc::ServerContext>();
    }

    void TearDown() override
    {
        context_.reset();
        service_.reset();
        if (pluginService_) {
            pluginService_->RemovePluginInfo(pluginInfo);
        }
        pluginService_.reset();
        ProfilerCapabilityManager::GetInstance().pluginCapabilities_.clear();
    }

    void AddPlugin(const std::string& pluginName)
    {
        PluginInfo pluginInfo;
        pluginInfo.name = pluginName;
        if (pluginService_) {
            pluginService_->AddPluginInfo(pluginInfo);
        }
    }

    grpc::Status StartSession(uint32_t sessionId)
    {
        StartSessionRequest request;
        StartSessionResponse response;

        request.set_session_id(sessionId);
        request.set_request_id(++requestCounter);
        return service_->StartSession(context_.get(), &request, &response);
    }

    grpc::Status StopSession(uint32_t sessionId)
    {
        StopSessionRequest request;
        StopSessionResponse response;

        request.set_session_id(sessionId);
        request.set_request_id(++requestCounter);
        return service_->StopSession(context_.get(), &request, &response);
    }

    grpc::Status DestroySession(uint32_t sessionId)
    {
        DestroySessionRequest request;
        DestroySessionResponse response;

        request.set_session_id(sessionId);
        request.set_request_id(++requestCounter);
        return service_->DestroySession(context_.get(), &request, &response);
    }

    uint32_t CreateOnlineSession()
    {
        CreateSessionRequest request;
        CreateSessionResponse response;

        auto sessionConfig = request.mutable_session_config();
        CHECK_NOTNULL(sessionConfig, 0, "request.mutable_session_config() return nullptr!");

        sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
        auto pluginConfig = request.add_plugin_configs();
        CHECK_NOTNULL(pluginConfig, 0, "request.add_plugin_configs() return nullptr!");

        pluginConfig->set_name(pluginInfo.name);
        request.set_request_id(++requestCounter);
        auto status = service_->CreateSession(context_.get(), &request, &response);
        CHECK_TRUE(status.error_code() == grpc::StatusCode::OK, 0, "create session FAILED!");

        return response.session_id();
    }

    uint32_t CreateOfflineSession()
    {
        CreateSessionRequest request;
        CreateSessionResponse response;

        auto sessionConfig = request.mutable_session_config();
        CHECK_NOTNULL(sessionConfig, 0, "request.mutable_session_config() return nullptr!");

        sessionConfig->set_session_mode(ProfilerSessionConfig::OFFLINE);
        sessionConfig->set_result_file("trace.bin");
        auto pluginConfig = request.add_plugin_configs();
        CHECK_NOTNULL(pluginConfig, 0, "request.add_plugin_configs() return nullptr!");

        pluginConfig->set_name(pluginInfo.name);
        request.set_request_id(++requestCounter);
        auto status = service_->CreateSession(context_.get(), &request, &response);
        CHECK_TRUE(status.error_code() == grpc::StatusCode::OK, 0, "create session FAILED!");

        return response.session_id();
    }
};

/**
 * @tc.name: server
 * @tc.desc: Abnormal test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, CtorDtor, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
}

/**
 * @tc.name: server
 * @tc.desc: get plugin capabilities normal.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, GetCapabilities, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    GetCapabilitiesRequest request;
    auto response = std::make_unique<GetCapabilitiesResponse>();
    ASSERT_NE(response, nullptr);

    ProfilerPluginCapability cap;
    cap.set_name("cap1");

    request.set_request_id(++requestCounter);
    auto status = service_->GetCapabilities(context_.get(), &request, response.get());
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(response->capabilities_size(), ProfilerCapabilityManager::GetInstance().GetCapabilities().size());

    int capSize = response->capabilities_size();
    EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(cap));

    request.set_request_id(++requestCounter);
    response = std::make_unique<GetCapabilitiesResponse>();
    status = service_->GetCapabilities(context_.get(), &request, response.get());
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(response->capabilities_size(), ProfilerCapabilityManager::GetInstance().GetCapabilities().size());
    EXPECT_EQ(response->capabilities_size(), capSize + 1);
}

/**
 * @tc.name: server
 * @tc.desc: get plugin capabilities normal.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, GetCapabilitiesAfterRemove, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    GetCapabilitiesRequest request;
    auto response = std::make_unique<GetCapabilitiesResponse>();
    ASSERT_NE(response, nullptr);

    ProfilerPluginCapability cap;
    cap.set_name("cap1");

    request.set_request_id(++requestCounter);
    auto status = service_->GetCapabilities(context_.get(), &request, response.get());
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(response->capabilities_size(), ProfilerCapabilityManager::GetInstance().GetCapabilities().size());

    int capSize = response->capabilities_size();
    EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(cap));

    request.set_request_id(++requestCounter);
    response = std::make_unique<GetCapabilitiesResponse>();
    status = service_->GetCapabilities(context_.get(), &request, response.get());
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(response->capabilities_size(), capSize + 1);

    EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().RemoveCapability(cap.name()));
    response = std::make_unique<GetCapabilitiesResponse>();
    status = service_->GetCapabilities(context_.get(), &request, response.get());
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(response->capabilities_size(), ProfilerCapabilityManager::GetInstance().GetCapabilities().size());
    EXPECT_EQ(response->capabilities_size(), capSize);
}

/**
 * @tc.name: server
 * @tc.desc: get plugin capabilities batch test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, GetCapabilitiesBatchTest, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    GetCapabilitiesRequest request;
    auto response = std::make_unique<GetCapabilitiesResponse>();
    ASSERT_NE(response, nullptr);

    request.set_request_id(++requestCounter);
    auto status = service_->GetCapabilities(context_.get(), &request, response.get());
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(response->capabilities_size(), ProfilerCapabilityManager::GetInstance().GetCapabilities().size());

    for (int i = 0; i < ROUND_COUNT; i++) {
        ProfilerPluginCapability cap;
        cap.set_name("cap_" + std::to_string(i));
        EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(cap));
    }

    Timer timer = {};
    for (int i = 0; i < ROUND_COUNT; i++) {
        request.set_request_id(++requestCounter);
        response = std::make_unique<GetCapabilitiesResponse>();
        status = service_->GetCapabilities(context_.get(), &request, response.get());
        EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    }
    auto timeCost = timer.ElapsedUs();
    printf("GetCapabilities %d time cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
}

/**
 * @tc.name: server
 * @tc.desc: get plugin capabilities with invalid context.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, GetCapabilitiesWithInvalidContext, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    GetCapabilitiesRequest request;
    auto response = std::make_unique<GetCapabilitiesResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->GetCapabilities(nullptr, &request, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: get plugin capabilities with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, GetCapabilitiesWithInvalidArguments, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    GetCapabilitiesRequest request;
    auto response = std::make_unique<GetCapabilitiesResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->GetCapabilities(context_.get(), nullptr, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->GetCapabilities(context_.get(), &request, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->GetCapabilities(context_.get(), nullptr, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: create session without plugin config.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, CreateSessionNoPluginConfig, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    CreateSessionRequest request;
    CreateSessionResponse response;

    auto status = service_->CreateSession(context_.get(), &request, &response);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: create session offline test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, CreateSessionOffline, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    CreateSessionRequest request;
    CreateSessionResponse response;

    auto sessionConfig = request.mutable_session_config();
    ASSERT_NE(sessionConfig, nullptr);
    sessionConfig->set_session_mode(ProfilerSessionConfig::OFFLINE);
    sessionConfig->set_result_file("trace.bin");

    auto pluginConfig = request.add_plugin_configs();
    ASSERT_NE(pluginConfig, nullptr);
    pluginConfig->set_name(pluginInfo.name);

    request.set_request_id(++requestCounter);
    auto status = service_->CreateSession(context_.get(), &request, &response);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: create session online test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, CreateSessionOnline, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    CreateSessionRequest request;
    CreateSessionResponse response;

    auto sessionConfig = request.mutable_session_config();
    ASSERT_NE(sessionConfig, nullptr);
    sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
    sessionConfig->clear_result_file();

    auto pluginConfig = request.add_plugin_configs();
    ASSERT_NE(pluginConfig, nullptr);
    pluginConfig->set_name(pluginInfo.name);

    request.set_request_id(++requestCounter);
    auto status = service_->CreateSession(context_.get(), &request, &response);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: create session batch test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, CreateSessionBatchTest, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    Timer timer = {};
    for (int i = 0; i < ROUND_COUNT; i++) {
        CreateSessionRequest request;
        CreateSessionResponse response;

        auto sessionConfig = request.mutable_session_config();
        ASSERT_NE(sessionConfig, nullptr);
        sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);
        sessionConfig->clear_result_file();

        auto pluginConfig = request.add_plugin_configs();
        ASSERT_NE(pluginConfig, nullptr);
        std::string pluginName = "create_session_batch_test_" + std::to_string(i);
        AddPlugin(pluginName);
        pluginConfig->set_name(pluginName);

        request.set_request_id(++requestCounter);
        auto status = service_->CreateSession(context_.get(), &request, &response);
        EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    }
    auto timeCost = timer.ElapsedUs();
    printf("CreateSession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
}

/**
 * @tc.name: server
 * @tc.desc: create session with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, CreateSessionInvalidArguments, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    CreateSessionRequest request;
    auto response = std::make_unique<CreateSessionResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->CreateSession(context_.get(), nullptr, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->CreateSession(context_.get(), &request, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->CreateSession(context_.get(), nullptr, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: create session with invalid context.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, CreateSessionWithInvalidContext, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    CreateSessionRequest request;
    auto response = std::make_unique<CreateSessionResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->CreateSession(nullptr, &request, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: destroy session with invalid session id.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, DestroySessionInvalidSessionId, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);

    EXPECT_NE(DestroySession(0).error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: destroy session with offline session.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, DestroySessionOffline, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);

    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    EXPECT_EQ(DestroySession(sessionId).error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: destroy session with online session.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, DestroySessionOnline, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);

    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);

    EXPECT_EQ(DestroySession(sessionId).error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: destroy session batch test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, DestroySessionBatchTest, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    std::vector<uint32_t> sessionIds;
    Timer timer = {};
    for (int i = 0; i < ROUND_COUNT; i++) {
        uint32_t sessionId = 0;
        {
            CreateSessionRequest request;
            CreateSessionResponse response;

            auto sessionConfig = request.mutable_session_config();
            ASSERT_NE(sessionConfig, nullptr);
            sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);

            auto pluginConfig = request.add_plugin_configs();
            ASSERT_NE(pluginConfig, nullptr);

            std::string pluginName = "create_session_batch_test_" + std::to_string(i);
            AddPlugin(pluginName);
            pluginConfig->set_name(pluginName);

            request.set_request_id(++requestCounter);
            auto status = service_->CreateSession(context_.get(), &request, &response);
            EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
            sessionId = response.session_id();
        }
        if (sessionId) {
            sessionIds.push_back(sessionId);
        }
    }
    auto timeCost = timer.ElapsedUs();
    printf("CreateSession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    timer.Reset();
    for (auto sessionId : sessionIds) {
        EXPECT_EQ(DestroySession(sessionId).error_code(), grpc::StatusCode::OK);
    }
    timeCost = timer.ElapsedUs();
    printf("DestroySession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
}

/**
 * @tc.name: server
 * @tc.desc: destroy session with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, DestroySessionInvalidArguments, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);

    DestroySessionRequest request;
    auto response = std::make_unique<DestroySessionResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->DestroySession(context_.get(), nullptr, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->DestroySession(context_.get(), &request, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->DestroySession(context_.get(), nullptr, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: destroy session with invalid context.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, DestroySessionWithInvalidContext, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);

    DestroySessionRequest request;
    auto response = std::make_unique<DestroySessionResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->DestroySession(nullptr, &request, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: start session with invalid session id.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StartSessionInvalidSessionId, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    auto status = StartSession(0);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: start session with valid offline session.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StartSessionOffline, TestSize.Level1)
{
    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    auto status = StartSession(sessionId);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: start session with valid online session.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StartSessionOnline, TestSize.Level1)
{
    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);

    auto status = StartSession(sessionId);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: start session and update configs.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StartSessionUpdateConfigs, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    StartSessionRequest request;
    StartSessionResponse response;

    request.set_session_id(sessionId);
    request.set_request_id(++requestCounter);
    auto pluginConfig = request.add_update_configs();
    ASSERT_NE(pluginConfig, nullptr);

    pluginConfig->set_name(pluginInfo.name);
    pluginConfig->set_sample_interval(1000);
    auto status = service_->StartSession(context_.get(), &request, &response);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: start session batch test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StartSessionBatchTest, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    std::vector<uint32_t> sessionIds;
    Timer timer = {};
    for (int i = 0; i < ROUND_COUNT; i++) {
        uint32_t sessionId = 0;
        {
            CreateSessionRequest request;
            CreateSessionResponse response;

            auto sessionConfig = request.mutable_session_config();
            ASSERT_NE(sessionConfig, nullptr);
            sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);

            auto pluginConfig = request.add_plugin_configs();
            ASSERT_NE(pluginConfig, nullptr);

            std::string pluginName = "create_session_batch_test_" + std::to_string(i);
            AddPlugin(pluginName);
            pluginConfig->set_name(pluginName);

            request.set_request_id(++requestCounter);
            auto status = service_->CreateSession(context_.get(), &request, &response);
            EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
            sessionId = response.session_id();
        }
        if (sessionId) {
            sessionIds.push_back(sessionId);
        }
    }
    auto timeCost = timer.ElapsedUs();
    printf("CreateSession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    timer.Reset();
    for (auto sessionId : sessionIds) {
        EXPECT_EQ(StartSession(sessionId).error_code(), grpc::StatusCode::OK);
    }
    timeCost = timer.ElapsedUs();
    printf("StartSession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
}

/**
 * @tc.name: server
 * @tc.desc: start session with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StartSessionInvalidArguments, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);

    StartSessionRequest request;
    auto response = std::make_unique<StartSessionResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->StartSession(context_.get(), nullptr, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->StartSession(context_.get(), &request, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->StartSession(context_.get(), nullptr, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: start session with invalid context.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StartSessionWithInvalidContext, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);

    StartSessionRequest request;
    auto response = std::make_unique<StartSessionResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->StartSession(nullptr, &request, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: stop session with invalid session id.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StopSessionInvalidSessionId, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    auto status = StopSession(0);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: stop session with offline session.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StopSessionOffline, TestSize.Level1)
{
    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    auto status = StopSession(sessionId);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: stop session with online session.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StopSessionOnline, TestSize.Level1)
{
    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);
    {
        auto status = StartSession(sessionId);
        EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    }

    auto status = StopSession(sessionId);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: stop session batch test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StopSessionBatchTest, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    std::vector<uint32_t> sessionIds;
    Timer timer = {};
    for (int i = 0; i < ROUND_COUNT; i++) {
        uint32_t sessionId = 0;
        {
            CreateSessionRequest request;
            CreateSessionResponse response;

            auto sessionConfig = request.mutable_session_config();
            ASSERT_NE(sessionConfig, nullptr);
            sessionConfig->set_session_mode(ProfilerSessionConfig::ONLINE);

            auto pluginConfig = request.add_plugin_configs();
            ASSERT_NE(pluginConfig, nullptr);

            std::string pluginName = "create_session_batch_test_" + std::to_string(i);
            AddPlugin(pluginName);
            pluginConfig->set_name(pluginName);

            request.set_request_id(++requestCounter);
            auto status = service_->CreateSession(context_.get(), &request, &response);
            EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
            sessionId = response.session_id();
        }
        if (sessionId) {
            sessionIds.push_back(sessionId);
        }
    }
    auto timeCost = timer.ElapsedUs();
    printf("CreateSession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    timer.Reset();
    for (auto sessionId : sessionIds) {
        EXPECT_EQ(StartSession(sessionId).error_code(), grpc::StatusCode::OK);
    }
    timeCost = timer.ElapsedUs();
    printf("StartSession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    timer.Reset();
    for (auto sessionId : sessionIds) {
        EXPECT_EQ(StopSession(sessionId).error_code(), grpc::StatusCode::OK);
    }
    timeCost = timer.ElapsedUs();
    printf("StopSession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);
}

/**
 * @tc.name: server
 * @tc.desc: stop session with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StopSessionInvalidArguments, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);
    EXPECT_EQ(StartSession(sessionId).error_code(), grpc::StatusCode::OK);

    StopSessionRequest request;
    auto response = std::make_unique<StopSessionResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->StopSession(context_.get(), nullptr, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->StopSession(context_.get(), &request, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->StopSession(context_.get(), nullptr, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: stop session with invalid context.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StopSessionWithInvalidContext, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOnlineSession();
    EXPECT_NE(sessionId, 0);
    EXPECT_EQ(StartSession(sessionId).error_code(), grpc::StatusCode::OK);

    StopSessionRequest request;
    auto response = std::make_unique<StopSessionResponse>();
    ASSERT_NE(response, nullptr);

    auto status = service_->StopSession(nullptr, &request, response.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

class FakeServerWriter : public ::grpc::ServerWriter<::FetchDataResponse> {
public:
    FakeServerWriter(::grpc::internal::Call* call, grpc::ServerContext* ctx)
        : ::grpc::ServerWriter<::FetchDataResponse>(call, ctx)
    {
    }
    ~FakeServerWriter() = default;

    uint32_t GetDataCount() const
    {
        return dataCount_;
    }

    using grpc::internal::WriterInterface<::FetchDataResponse>::Write;
    bool Write(const ::FetchDataResponse& msg, ::grpc::WriteOptions options) override
    {
        if (msg.plugin_data_size() > 0) {
            dataCount_ += msg.plugin_data_size();
        }
        return true;
    }

private:
    std::atomic<uint32_t> dataCount_ = 0;
};

/**
 * @tc.name: server
 * @tc.desc: fetch data with invalid session id.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, FetchDataInvalidSessionId, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    FetchDataRequest request;
    auto writer = std::make_unique<FakeServerWriter>(nullptr, context_.get());

    request.set_session_id(0); // invalid session id
    request.set_request_id(++requestCounter);
    auto status = service_->FetchData(context_.get(), &request, writer.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: fetch data with offline.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, FetchDataOffline, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);
    {
        auto status = StartSession(sessionId);
        EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    }

    FetchDataRequest request;
    request.set_session_id(sessionId);
    request.set_request_id(++requestCounter);
    auto writer = std::make_unique<FakeServerWriter>(nullptr, context_.get());
    auto status = service_->FetchData(context_.get(), &request, writer.get());
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);

    EXPECT_EQ(StopSession(sessionId).error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(DestroySession(sessionId).error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: fetch data online.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, FetchDataOnline, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOnlineSession();
    StartSession(sessionId);

    auto sessionCtx = service_->GetSessionContext(sessionId);
    ASSERT_NE(sessionCtx->dataRepeater, nullptr);
    {
        auto data = std::make_shared<ProfilerPluginData>();
        ASSERT_NE(data, nullptr);
        data->set_name(pluginInfo.name);
        sessionCtx->dataRepeater->PutPluginData(data);
    }

    FetchDataRequest request;
    auto writer = std::make_unique<FakeServerWriter>(nullptr, context_.get());
    request.set_session_id(sessionId);
    request.set_request_id(++requestCounter);
    std::thread bgThread([&] {
        auto status = service_->FetchData(context_.get(), &request, writer.get());
        EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    });

    usleep(FETCH_DATA_DELAY_US);
    sessionCtx->dataRepeater->Close();
    bgThread.join();

    EXPECT_EQ(StopSession(sessionId).error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(DestroySession(sessionId).error_code(), grpc::StatusCode::OK);

    EXPECT_EQ(writer->GetDataCount(), 1);
}

/**
 * @tc.name: server
 * @tc.desc: fetch data online batch test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, FetchDataBatchTest, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    auto writer = std::make_unique<FakeServerWriter>(nullptr, context_.get());

    uint32_t sessionId = CreateOnlineSession();
    StartSession(sessionId);

    auto sessionCtx = service_->GetSessionContext(sessionId);
    ASSERT_NE(sessionCtx->dataRepeater, nullptr);

    // 只能存入小于maxsize的数据才能确保dataProducer线程正常退出
    int size = ProfilerService::DEFAULT_REPEATER_BUFFER_SIZE - 1;
    std::thread dataProducer([&]() {
        for (int i = 0; i < size; i++) {
            auto data = std::make_shared<ProfilerPluginData>();
            ASSERT_NE(data, nullptr);
            data->set_name(pluginInfo.name);
            data->set_status(i);
            sessionCtx->dataRepeater->PutPluginData(data);
        }
    });
    dataProducer.join();

    std::thread closeDataRepeater([&]() {
        while (1) {
            if (sessionCtx->dataRepeater->Size() == 0) {
                sessionCtx->dataRepeater->Close();
                break;
            }
        }
    });

    Timer timer = {};
    std::thread dataConsumer([&]() {
        FetchDataRequest request;
        request.set_session_id(sessionId);
        request.set_request_id(++requestCounter);
        auto status = service_->FetchData(context_.get(), &request, writer.get());
        EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    });

    dataConsumer.join();
    auto timeCost = timer.ElapsedUs();
    printf("FetchData %d time, cost %ldus.\n", size, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    EXPECT_EQ(StopSession(sessionId).error_code(), grpc::StatusCode::OK);
    EXPECT_EQ(DestroySession(sessionId).error_code(), grpc::StatusCode::OK);

    EXPECT_EQ(writer->GetDataCount(), size);
    closeDataRepeater.join();
}

/**
 * @tc.name: server
 * @tc.desc: fetch data with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, FetchDataInvalidArguments, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    FetchDataRequest request;
    auto writer = std::make_unique<FakeServerWriter>(nullptr, context_.get());
    request.set_request_id(++requestCounter);
    auto status = service_->FetchData(context_.get(), &request, writer.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    uint32_t sessionId = CreateOnlineSession();

    auto sessionCtx = service_->GetSessionContext(sessionId);
    ASSERT_NE(sessionCtx->dataRepeater, nullptr);
    {
        auto data = std::make_shared<ProfilerPluginData>();
        ASSERT_NE(data, nullptr);
        data->set_name(pluginInfo.name);
        sessionCtx->dataRepeater->PutPluginData(data);
    }

    status = service_->FetchData(context_.get(), nullptr, writer.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->FetchData(context_.get(), &request, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->FetchData(context_.get(), nullptr, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: fetch data with invalid context.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, FetchDataInvalidContext, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    FetchDataRequest request;
    auto writer = std::make_unique<FakeServerWriter>(nullptr, context_.get());
    request.set_request_id(++requestCounter);
    auto status = service_->FetchData(context_.get(), &request, writer.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    uint32_t sessionId = CreateOnlineSession();

    auto sessionCtx = service_->GetSessionContext(sessionId);
    ASSERT_NE(sessionCtx->dataRepeater, nullptr);
    {
        auto data = std::make_shared<ProfilerPluginData>();
        ASSERT_NE(data, nullptr);
        data->set_name(pluginInfo.name);
        sessionCtx->dataRepeater->PutPluginData(data);
    }

    status = service_->FetchData(nullptr, &request, writer.get());
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: keep session with invalid session id.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, KeepSessionInvalidSessionId, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    KeepSessionRequest request;
    KeepSessionResponse response;

    request.set_session_id(0);
    request.set_request_id(++requestCounter);
    auto status = service_->KeepSession(context_.get(), &request, &response);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: keep session with offline session id.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, KeepSessionWithoutAliveTime, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    KeepSessionRequest request;
    KeepSessionResponse response;

    request.set_session_id(sessionId);
    request.set_request_id(++requestCounter);
    auto status = service_->KeepSession(context_.get(), &request, &response);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: keep session with offline session id.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, KeepSessionOffline, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    KeepSessionRequest request;
    KeepSessionResponse response;

    request.set_session_id(sessionId);
    request.set_request_id(++requestCounter);
    request.set_keep_alive_time(TEST_SESSION_TIMEOUT_MS);
    auto status = service_->KeepSession(context_.get(), &request, &response);
    EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);

    // make sure session expired by backgroud task
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SESSION_TIMEOUT_MS));
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SESSION_TIMEOUT_MS));

    status = DestroySession(sessionId);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: keep session with batch test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, KeepSessionBatchTest, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    Timer timer = {};
    for (int i = 0; i < ROUND_COUNT; i++) {
        KeepSessionRequest request;
        KeepSessionResponse response;

        request.set_session_id(sessionId);
        request.set_request_id(++requestCounter);
        request.set_keep_alive_time(TEST_SESSION_TIMEOUT_MS);
        auto status = service_->KeepSession(context_.get(), &request, &response);
        EXPECT_EQ(status.error_code(), grpc::StatusCode::OK);
    }
    auto timeCost = timer.ElapsedUs();
    printf("KeepSession %d time, cost %ldus.\n", ROUND_COUNT, timeCost);
    EXPECT_LE(timeCost, TIMEOUT_US);

    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SESSION_TIMEOUT_MS));
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SESSION_TIMEOUT_MS));
    auto status = DestroySession(sessionId);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: keep session with invalid arguments.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, KeepSessionInvalidArgs, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    KeepSessionRequest request;
    KeepSessionResponse response;

    request.set_session_id(sessionId);
    request.set_request_id(++requestCounter);
    request.set_keep_alive_time(TEST_SESSION_TIMEOUT_MS);
    auto status = service_->KeepSession(context_.get(), nullptr, &response);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);

    status = service_->KeepSession(context_.get(), &request, nullptr);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: keep session with invalid context.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, KeepSessionInvalidContext, TestSize.Level1)
{
    ASSERT_NE(service_, nullptr);
    ASSERT_NE(context_, nullptr);

    uint32_t sessionId = CreateOfflineSession();
    EXPECT_NE(sessionId, 0);

    KeepSessionRequest request;
    KeepSessionResponse response;

    request.set_session_id(sessionId);
    request.set_request_id(++requestCounter);
    request.set_keep_alive_time(TEST_SESSION_TIMEOUT_MS);
    auto status = service_->KeepSession(nullptr, &request, &response);
    EXPECT_NE(status.error_code(), grpc::StatusCode::OK);
}

/**
 * @tc.name: server
 * @tc.desc: start service.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, StartService, TestSize.Level1)
{
    auto service = service_;
    EXPECT_NE(service, nullptr);
    EXPECT_FALSE(service->StartService(""));
    std::string listenUri = std::string("0.0.0.0:") + std::to_string(COMMON::GetServicePort());
    EXPECT_TRUE(service->StartService(listenUri));
    std::thread waiterThread(&ProfilerService::WaitServiceDone, service.get());
    sleep(1);
    service->StopService();
    waiterThread.join();
}

/**
 * @tc.name: server
 * @tc.desc: MergeStandaloneFile test.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerServiceTest, MergeStandaloneFile, TestSize.Level1)
{
    std::string testPath = "testFile.bin";
    std::string targetPath = "target.trace.bin";
    auto writer = std::make_shared<TraceFileWriter>(testPath, false, 0, 0);
    auto writerTarget = std::make_shared<TraceFileWriter>(targetPath, false, 0, 0);
    EXPECT_NE(writer, nullptr);
    EXPECT_NE(writerTarget, nullptr);
    std::string testData = "this is a test case!this is a test case!";
    std::string traceData = "test";
    EXPECT_EQ(writer->Write(testData.data(), testData.size()), sizeof(uint32_t) + testData.size());
    EXPECT_EQ(writerTarget->Write(traceData.data(), traceData.size()), sizeof(uint32_t) + traceData.size());
    std::string pluginName = "destroy_session_standalone_test";
    service_->MergeStandaloneFile(targetPath, pluginName, testPath, "1.02");
    service_->MergeStandaloneFile(targetPath, "", testPath, "1.02");
    service_->MergeStandaloneFile(targetPath, pluginName, "", "1.02");
    service_->MergeStandaloneFile("", pluginName, testPath, "1.02");
    service_->MergeStandaloneFile(targetPath, "hiperf-plugin", testPath, "1.02");
}

} // namespace
