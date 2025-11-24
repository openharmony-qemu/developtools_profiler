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

#include "profiler_session_manager.h"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>
#include <grpcpp/grpcpp.h>
#include "common.h"
#include "parse_plugin_config.h"
#include "parameters.h"

namespace {
constexpr int MS_PER_S = 1000;
constexpr int DEFAULT_SESSION_TIME_S = 10;
constexpr int KEEP_SESSION_TIMEOUT_MS = 5 * 1000;
constexpr int KEEP_SESSION_SLEEP_SECOND = 3;
constexpr int ADDR_BUFFER_SIZE = 128;
constexpr uint32_t INT_MAX_LEN = 10;
const std::string DEFAULT_OUTPUT_FILE = "/data/local/tmp/hiprofiler_data.htrace";
const std::string DEFAULT_OUTPUT_ROOT = "/data/local/tmp/";

std::string GetLoopbackAddress()
{
    char addressBuffer[ADDR_BUFFER_SIZE] = "";
    struct ifaddrs* ifAddrStruct = nullptr;
    void* tmpAddrPtr = nullptr;

    if (getifaddrs(&ifAddrStruct) == -1) {
        printf("error: %s\n", COMMON::GetErrorMsg().c_str());
        return "";
    }
    
    while (ifAddrStruct != nullptr) {
        if (ifAddrStruct->ifa_addr == nullptr) {
            ifAddrStruct = ifAddrStruct->ifa_next;
            continue;
        }
        if (ifAddrStruct->ifa_addr->sa_family == AF_INET) {
            tmpAddrPtr = &((reinterpret_cast<struct sockaddr_in*>(ifAddrStruct->ifa_addr))->sin_addr);
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strcmp(addressBuffer, "127.0.0.1") == 0) {
                break;
            }
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }

    freeifaddrs(ifAddrStruct);
    return addressBuffer;
}

}  // namespace

ProfilerSessionManager& ProfilerSessionManager::GetInstance()
{
    static ProfilerSessionManager instance;
    return instance;
}

ProfilerSessionManager::ProfilerSessionManager()
    : exitRequested_(false), keepSessionRunning_(false)
{
}

ProfilerSessionManager::~ProfilerSessionManager()
{
    RequestExit();
    if (keepSessionThread_ && keepSessionThread_->joinable()) {
        keepSessionCv_.notify_one();
        keepSessionThread_->join();
    }
}

void ProfilerSessionManager::RequestExit()
{
    exitRequested_ = true;
    sessionCv_.notify_one();
}

std::unique_ptr<IProfilerService::Stub> ProfilerSessionManager::GetProfilerServiceStub()
{
    std::string serviceUri = GetLoopbackAddress() + ":" + std::to_string(COMMON::GetServicePort());
    auto grpcChannel = grpc::CreateChannel(serviceUri, grpc::InsecureChannelCredentials());
    if (grpcChannel == nullptr) {
        printf("Create gRPC channel failed!\n");
        return nullptr;
    }
    return IProfilerService::NewStub(grpcChannel);
}

std::unique_ptr<CreateSessionRequest> ProfilerSessionManager::MakeCreateRequest(
    const std::string& config,
    const std::string& keepSecond,
    const std::string& outputFile)
{
    auto request = std::make_unique<CreateSessionRequest>();
    std::string content = config;
    
    if (content.empty()) {
        printf("config file empty!");
        return nullptr;
    }

    if (!ParsePluginConfig::GetInstance().GetParser().ParseFromString(content, request.get())) {
        printf("config [%s] parse FAILED!\n", content.c_str());
        return nullptr;
    }

    auto sessionConfig = request->mutable_session_config();
    if (!sessionConfig) {
        return nullptr;
    }

    request->set_request_id(1);
    
    if (!keepSecond.empty() && keepSecond.size() < INT_MAX_LEN) {
        int ks = COMMON::IsNumeric(keepSecond) ? std::stoi(keepSecond) : 0;
        if (ks > 0) {
            sessionConfig->set_sample_duration(ks * MS_PER_S);
        }
    } else if (sessionConfig->sample_duration() <= 0) {
        sessionConfig->set_sample_duration(DEFAULT_SESSION_TIME_S * MS_PER_S);
    }

    if (!outputFile.empty()) {
        sessionConfig->set_result_file(outputFile);
    } else if (sessionConfig->result_file() == "") {
        sessionConfig->set_result_file(DEFAULT_OUTPUT_FILE);
    }

    if (sessionConfig->result_file().substr(0, DEFAULT_OUTPUT_ROOT.size()) != DEFAULT_OUTPUT_ROOT) {
        printf("Filepath unable to write! Please use /data/local/tmp/ at the beginning.\n");
        return nullptr;
    }

    printf("keepSecond: %us\n", sessionConfig->sample_duration() / MS_PER_S);

    for (int i = 0; i < request->plugin_configs().size(); i++) {
        auto pluginConfig = request->mutable_plugin_configs(i);
        if (!ParsePluginConfig::GetInstance().SetSerializePluginsConfig(pluginConfig->name(), *pluginConfig)) {
            printf("set %s plugin config failed\n", pluginConfig->name().c_str());
            return nullptr;
        }
    }

    return request;
}

uint32_t ProfilerSessionManager::CreateSession(
    std::unique_ptr<IProfilerService::Stub>& profilerStub,
    const std::string& config,
    const std::string& keepSecond,
    const std::string& outputFile)
{
    auto request = MakeCreateRequest(config, keepSecond, outputFile);
    if (!request) {
        printf("MakeCreateRequest failed!\n");
        return 0;
    }

    CreateSessionResponse createResponse;
    grpc::ClientContext createSessionContext;
    grpc::Status status = profilerStub->CreateSession(&createSessionContext, *request, &createResponse);
    if (!status.ok()) {
        printf("CreateSession FAIL\n");
        return 0;
    }

    return createResponse.session_id();
}

bool ProfilerSessionManager::StartSession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t sessionId)
{
    StartSessionRequest startRequest;
    StartSessionResponse startResponse;
    startRequest.set_request_id(0);
    startRequest.set_session_id(sessionId);
    
    grpc::ClientContext startContext;
    grpc::Status status = profilerStub->StartSession(&startContext, startRequest, &startResponse);
    
    if (!status.ok()) {
        printf("StartSession FAIL\n");
        return false;
    }

    return true;
}

bool ProfilerSessionManager::StopSession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t sessionId)
{
    StopSessionRequest stopRequest;
    StopSessionResponse stopResponse;
    grpc::ClientContext stopContext;
    
    stopRequest.set_session_id(sessionId);
    grpc::Status status = profilerStub->StopSession(&stopContext, stopRequest, &stopResponse);
    
    if (!status.ok()) {
        return false;
    }

    printf("StopSession done!\n");
    return true;
}

bool ProfilerSessionManager::DestroySession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t sessionId)
{
    DestroySessionRequest destroyRequest;
    DestroySessionResponse destroyResponse;
    grpc::ClientContext destroyContext;
    
    destroyRequest.set_session_id(sessionId);
    grpc::Status status = profilerStub->DestroySession(&destroyContext, destroyRequest, &destroyResponse);
    
    if (!status.ok()) {
        return false;
    }

    printf("DestroySession done!\n");
    return true;
}

void ProfilerSessionManager::KeepSessionAlive(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t sessionId)
{
    while (keepSessionRunning_ && !exitRequested_) {
        KeepSessionRequest keepRequest;
        keepRequest.set_request_id(0);
        keepRequest.set_session_id(sessionId);
        keepRequest.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
        
        grpc::ClientContext keepContext;
        KeepSessionResponse keepResponse;
        profilerStub->KeepSession(&keepContext, keepRequest, &keepResponse);

        std::unique_lock<std::mutex> lck(keepSessionMutex_);
        keepSessionCv_.wait_for(lck, std::chrono::seconds(KEEP_SESSION_SLEEP_SECOND));
    }
}

bool ProfilerSessionManager::Capture(const std::string& config, 
                                     const std::string& durationSeconds,
                                     const std::string& outputFile)
{
    auto profilerStub = GetProfilerServiceStub();
    if (profilerStub == nullptr) {
        printf("Get profiler service stub failed!\n");
        return false;
    }

    if (exitRequested_) {
        return false;
    }
    
    if (!COMMON::IsNumeric(durationSeconds)) {
        printf("please input a valid time value");
        return false;
    }

    uint32_t sessionId = CreateSession(profilerStub, config, durationSeconds, outputFile);
    if (sessionId == 0) {
        printf("Create session returns Id 0\n");
        return false;
    }

    if (exitRequested_) {
        DestroySession(profilerStub, sessionId);
        return false;
    }

    keepSessionRunning_ = true;
    keepSessionThread_ = std::make_unique<std::thread>(
        &ProfilerSessionManager::KeepSessionAlive, this, std::ref(profilerStub), sessionId);

    if (exitRequested_) {
        keepSessionRunning_ = false;
        keepSessionCv_.notify_one();
        keepSessionThread_->join();
        return DestroySession(profilerStub, sessionId);
    }

    if (!StartSession(profilerStub, sessionId)) {
        keepSessionRunning_ = false;
        keepSessionCv_.notify_one();
        keepSessionThread_->join();
        return false;
    }

    if (!exitRequested_) {
        printf("tracing %s ms....\n", durationSeconds.c_str());
        std::cout.flush();
        
        auto request = MakeCreateRequest(config, durationSeconds, outputFile);
        uint32_t duration = request ? request->session_config().sample_duration() : DEFAULT_SESSION_TIME_S * MS_PER_S;
        
        std::unique_lock<std::mutex> lck(sessionMutex_);
        sessionCv_.wait_for(lck, std::chrono::milliseconds(duration));
    }

    bool ret = StopSession(profilerStub, sessionId) && DestroySession(profilerStub, sessionId);

    keepSessionRunning_ = false;
    keepSessionCv_.notify_one();
    if (keepSessionThread_ && keepSessionThread_->joinable()) {
        keepSessionThread_->join();
    }

    return ret;
}

bool ProfilerSessionManager::GetCapabilities(std::string& capabilities, bool printResult)
{
    auto profilerStub = GetProfilerServiceStub();
    if (profilerStub == nullptr) {
        printf("Get profiler service stub failed!\n");
        return false;
    }

    GetCapabilitiesRequest capRequest;
    GetCapabilitiesResponse capResponse;
    capRequest.set_request_id(0);
    
    grpc::ClientContext capContext;
    grpc::Status status = profilerStub->GetCapabilities(&capContext, capRequest, &capResponse);
    
    if (!status.ok()) {
        printf("Service not started\n");
        return false;
    }

    if (!google::protobuf::TextFormat::PrintToString(capResponse, &capabilities)) {
        printf("capabilities message format FAILED!\n");
        return false;
    }

    if (!printResult) {
        printf("support plugin list:\n%s\n", capabilities.c_str());
    }
    
    return true;
}

int ProfilerSessionManager::CheckServiceConnection()
{
    auto profilerStub = GetProfilerServiceStub();
    if (profilerStub == nullptr) {
        printf("Get profiler service stub failed!\n");
        return -1;
    }

    GetCapabilitiesRequest request;
    GetCapabilitiesResponse response;
    request.set_request_id(0);

    grpc::ClientContext context;
    grpc::Status status = profilerStub->GetCapabilities(&context, request, &response);
    if (!status.ok()) {
        printf("Service not started\n");
        return -1;
    }

    printf("OK\n");
    printf("ip:%s\n", GetLoopbackAddress().c_str());
    printf("port:%u\n", COMMON::GetServicePort());
    return 0;
}

