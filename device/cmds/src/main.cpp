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

#include <arpa/inet.h>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <grpcpp/grpcpp.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <ostream>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "common.h"
#include "command_line.h"
#include "google/protobuf/text_format.h"
#include "parameters.h"
#include "parse_plugin_config.h"
#include "profiler_service.grpc.pb.h"
#include "trace_plugin_config.pb.h"

using google::protobuf::TextFormat;

namespace {
constexpr int ADDR_BUFFER_SIZE = 128;
constexpr int MS_PER_S = 1000;
constexpr int KEEP_SESSION_TIMEOUT_MS = 5 * 1000;
constexpr int KEEP_SESSION_SLEEP_SECOND = 3;
constexpr int DEFAULT_SESSION_TIME_S = 10;
const std::string DEFAULT_OUTPUT_FILE = "/data/local/tmp/hiprofiler_data.htrace";
const std::string HIPROFILERD_NAME("hiprofilerd");
const std::string HIPROFILER_PLUGINS_NAME("hiprofiler_plugins");
const std::string NATIVE_DAEMON_NAME("native_daemon");

uint32_t g_sampleDuration = 0;
int g_hiprofilerdPid = -1;
int g_hiprofilerPluginsPid = -1;
int g_nativeDaemonPid = -1;

std::string GetLoopbackAddress()
{
    char addressBuffer[ADDR_BUFFER_SIZE] = "";
    struct ifaddrs* ifAddrStruct = nullptr;
    void* tmpAddrPtr = nullptr;

    getifaddrs(&ifAddrStruct);
    while (ifAddrStruct != nullptr) {
        if (ifAddrStruct->ifa_addr == nullptr) {
            ifAddrStruct = ifAddrStruct->ifa_next;
            continue;
        }
        if (ifAddrStruct->ifa_addr->sa_family == AF_INET) {
            // is a valid IP4 Address
            tmpAddrPtr = &((reinterpret_cast<struct sockaddr_in*>(ifAddrStruct->ifa_addr))->sin_addr);
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if (strcmp(addressBuffer, "127.0.0.1") == 0) {
                break;
            }
        } else if (ifAddrStruct->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr = &((reinterpret_cast<struct sockaddr_in*>(ifAddrStruct->ifa_addr))->sin_addr);
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
        }
        ifAddrStruct = ifAddrStruct->ifa_next;
    }

    freeifaddrs(ifAddrStruct);
    return addressBuffer;
}

uint16_t GetServicePort()
{
    return COMMON::GetServicePort();
}

std::unique_ptr<CreateSessionRequest> MakeCreateRequest(const std::string& config,
                                                        const std::string& keepSecond,
                                                        const std::string& outputFile)
{
    auto request = std::make_unique<CreateSessionRequest>();
    if (!request) {
        return nullptr;
    }

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
    if (!keepSecond.empty()) {
        int ks = std::stoi(keepSecond);
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
    printf("keepSecond: %us, outputFileName: %s\n", sessionConfig->sample_duration() / MS_PER_S,
        sessionConfig->result_file().c_str());

    g_sampleDuration = sessionConfig->sample_duration();
    for (int i = 0; i < request->plugin_configs().size(); i++) {
        auto pluginConfig = request->mutable_plugin_configs(i);
        if (!ParsePluginConfig::GetInstance().SetSerializePluginsConfig(pluginConfig->name(), *pluginConfig)) {
            printf("set %s plugin config failed\n", pluginConfig->name().c_str());
            return nullptr;
        }
        if (sessionConfig->split_file() && pluginConfig->name() != "ftrace-plugin") {
            printf("only ftrace-plugin support split_file, please check session_config.\n");
            return nullptr;
        }
    }

    content.clear();
    if (!TextFormat::PrintToString(*request.get(), &content)) {
        printf("config message format FAILED!\n");
        return nullptr;
    }

    return request;
}

std::unique_ptr<IProfilerService::Stub> GetProfilerServiceStub()
{
    std::string serviceUri = GetLoopbackAddress() + ":" + std::to_string(GetServicePort());
    auto grpcChannel = grpc::CreateChannel(serviceUri, grpc::InsecureChannelCredentials());
    if (grpcChannel == nullptr) {
        printf("Create gRPC channel failed!\n");
        return nullptr;
    }
    return IProfilerService::NewStub(grpcChannel);
}

bool GetCapabilities(std::string& content, bool isCheck)
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

    if (!TextFormat::PrintToString(capResponse, &content)) {
        printf("capabilities message format FAILED!\n");
        return false;
    }

    if (!isCheck) {
        printf("support plugin list:\n%s\n", content.c_str());
    }
    return true;
}

uint32_t CreateSession(std::unique_ptr<IProfilerService::Stub>& profilerStub, const std::string& config,
    const std::string& keepSecond, const std::string& outputFile)
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

bool CheckStartSession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t& sessionId)
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

bool CheckStopSession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t& sessionId)
{
    StopSessionRequest stopRequest;
    StopSessionResponse stopResponse;
    grpc::ClientContext stopContext;
    stopRequest.set_session_id(sessionId);
    grpc::Status status = profilerStub->StopSession(&stopContext, stopRequest, &stopResponse);
    if (!status.ok()) {
        return false;
    }

    return true;
}

bool CheckDestroySession(std::unique_ptr<IProfilerService::Stub>& profilerStub, uint32_t& sessionId)
{
    DestroySessionRequest destroyRequest;
    DestroySessionResponse destroyResponse;
    grpc::ClientContext destroyContext;
    destroyRequest.set_session_id(sessionId);
    grpc::Status status = profilerStub->DestroySession(&destroyContext, destroyRequest, &destroyResponse);
    if (!status.ok()) {
        return false;
    }

    return true;
}

bool DoCapture(const std::string& config, const std::string& keepSecond, const std::string& outputFile)
{
    auto profilerStub = GetProfilerServiceStub();
    if (profilerStub == nullptr) {
        printf("Get profiler service stub failed!\n");
        return 0;
    }

    uint32_t sessionId = CreateSession(profilerStub, config, keepSecond, outputFile);
    if (sessionId == 0) {
        printf("Create session returns Id 0\n");
        return false;
    }

    // 开启心跳线程确保会话正常，睡眠3s下发一次5s超时心跳
    bool sendHeart = true;
    std::thread keepSessionThread([&]() {
        while (sendHeart) {
            KeepSessionRequest keepRequest;
            keepRequest.set_request_id(0);
            keepRequest.set_session_id(sessionId);
            keepRequest.set_keep_alive_time(KEEP_SESSION_TIMEOUT_MS);
            grpc::ClientContext keepContext;
            KeepSessionResponse keepResponse;
            profilerStub->KeepSession(&keepContext, keepRequest, &keepResponse);
            std::this_thread::sleep_for(std::chrono::seconds(KEEP_SESSION_SLEEP_SECOND));
        }
    });

    if (!CheckStartSession(profilerStub, sessionId)) {
        return false;
    }

    printf("tracing %u ms....\n", g_sampleDuration);
    std::this_thread::sleep_for(std::chrono::milliseconds(g_sampleDuration));

    if (!CheckStopSession(profilerStub, sessionId)) {
        sendHeart = false;
        keepSessionThread.join();
        return false;
    }
    printf("StopSession done!\n");

    if (!CheckDestroySession(profilerStub, sessionId)) {
        sendHeart = false;
        keepSessionThread.join();
        return false;
    }
    printf("DestroySession done!\n");

    sendHeart = false;
    keepSessionThread.join();
    return true;
}

struct DataContext {
    bool isGetGrpcAddr = false;
    std::string traceKeepSecond;
    std::string configFile;
    std::string outputFile;
    bool isHelp = false;
    bool isShowPluginList = false;
    bool isStartProcess = false;
    bool isKillProcess = false;
};

void ParseCmdline(CommandLine& cmdLine, DataContext& data)
{
    cmdLine.AddParamSwitch("--getport", "-q", data.isGetGrpcAddr, "get grpc address");
    cmdLine.AddParamText("--time", "-t", data.traceKeepSecond, "trace time");
    cmdLine.AddParamText("--out", "-o", data.outputFile, "output file name");
    cmdLine.AddParamSwitch("--help", "-h", data.isHelp, "make some help");
    cmdLine.AddParamSwitch("--list", "-l", data.isShowPluginList, "plugin list");
    cmdLine.AddParamSwitch("--start", "-s", data.isStartProcess, "start dependent process");
    cmdLine.AddParamSwitch("--kill", "-k", data.isKillProcess, "kill dependent process");
    cmdLine.AddParamText("--config", "-c", data.configFile, "start trace by config file");
}

int CheckGrpcMsgSend()
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
    printf("port:%u\n", GetServicePort());
    return 0;
}

bool StartDependentProcess()
{
    constexpr int waitProcMills = 300;
    if (getuid() == 0) {
        if (!COMMON::IsProcessExist(HIPROFILERD_NAME, g_hiprofilerdPid)) {
            // need start hiprofilerd
            std::vector<char*> argvVec;
            argvVec.push_back(const_cast<char*>(HIPROFILERD_NAME.c_str()));
            g_hiprofilerdPid = COMMON::StartProcess(HIPROFILERD_NAME, argvVec);
            // Wait for the hiprofilerd to start
            std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
        }

        if (!COMMON::IsProcessExist(HIPROFILER_PLUGINS_NAME, g_hiprofilerPluginsPid)) {
            // need start hiprofiler_plugins
            std::vector<char*> argvVec;
            argvVec.push_back(const_cast<char*>(HIPROFILER_PLUGINS_NAME.c_str()));
            g_hiprofilerPluginsPid = COMMON::StartProcess(HIPROFILER_PLUGINS_NAME, argvVec);
            // Wait for the hiprofiler_plugins add preset plugin
            std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
        }

        if (!COMMON::IsProcessExist(NATIVE_DAEMON_NAME, g_nativeDaemonPid)) {
            // need start native_daemon
            std::vector<char*> argvVec;
            argvVec.push_back(const_cast<char*>(NATIVE_DAEMON_NAME.c_str()));
            g_nativeDaemonPid = COMMON::StartProcess(NATIVE_DAEMON_NAME, argvVec);
            // Wait for the native_daemon to start
            std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
        }
    } else {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.native_memoryd.start", "0");

        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
        std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.plugins.start", "1");
        std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.native_memoryd.start", "1");
        std::this_thread::sleep_for(std::chrono::milliseconds(waitProcMills));
    }

    std::string content = "";
    GetCapabilities(content, true);
    if (content == "") {
        printf("Please confirm whether the plugin exists\n");
        return false;
    }

    return true;
}

void KillDependentProcess()
{
    if (getuid() == 0) {
        // if pid is equal to -1, need to get pid first.
        if (g_nativeDaemonPid == -1) {
            COMMON::IsProcessExist(NATIVE_DAEMON_NAME, g_nativeDaemonPid);
        }
        if (g_hiprofilerPluginsPid == -1) {
            COMMON::IsProcessExist(HIPROFILER_PLUGINS_NAME, g_hiprofilerPluginsPid);
        }
        if (g_hiprofilerdPid == -1) {
            COMMON::IsProcessExist(HIPROFILERD_NAME, g_hiprofilerdPid);
        }

        COMMON::KillProcess(g_nativeDaemonPid);
        COMMON::KillProcess(g_hiprofilerPluginsPid);
        COMMON::KillProcess(g_hiprofilerdPid);
    } else {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.plugins.start", "0");
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.native_memoryd.start", "0");
    }
}

bool ParseConfig(const std::string& configFile, std::string& config)
{
    std::string configFileWithPath = configFile;
    if (configFile.find('/') == std::string::npos) {
        std::string path("/data/local/tmp/");
        configFileWithPath = path + configFile; // add default path
    }

    printf("Read config from %s\n", configFileWithPath.c_str());
    std::vector<std::string> validPaths = { "/data/local/tmp/" };
    if (!COMMON::ReadFile(configFileWithPath, validPaths, config)) {
        printf("Read %s fail, please place it under \'/data/local/tmp/\'.\n", configFile.c_str());
        return false;
    }
    config = ParsePluginConfig::GetInstance().GetPluginsConfig(config);
    if (config.empty()) {
        printf("Error config file: %s\n", configFileWithPath.c_str()); // no config in command or config file
        return false;
    }
    return true;
}
} // namespace

int main(int argc, char* argv[])
{
    std::string config = "";
    while (true) {
        struct option long_options[] = {
            {"getport", no_argument, nullptr, 'q'},
            {"time", required_argument, nullptr, 't'},
            {"out", required_argument, nullptr, 'o'},
            {"help", no_argument, nullptr, 'h'},
            {"list", no_argument, nullptr, 'l'},
            {"start", no_argument,  nullptr, 's'},
            {"kill", no_argument,  nullptr, 'k'},
            {"config", required_argument, nullptr, 'c'},
            {nullptr, 0, nullptr, 0}
        };
        int option = getopt_long(argc, argv, "c:t:o:qhlsk", long_options, nullptr);
        if (option == -1) {
            break;  // CONFIG.
        }

        if (option == 'c' && strcmp(optarg, "-") == 0) {
            std::string content;
            std::istreambuf_iterator<char> begin(std::cin);
            std::istreambuf_iterator<char> end = {};
            content.assign(begin, end);
            config = ParsePluginConfig::GetInstance().GetPluginsConfig(content);
            if (config.empty()) {
                printf("Please check the configuration!\n");
                return -1;
            }
        }
    }

    DataContext data;
    CommandLine& cmdLine = CommandLine::GetInstance();
    ParseCmdline(cmdLine, data);

    std::vector<std::string> argvVector;
    for (int i = 0; i < argc; i++) {
        if (((i + 1) < argc) && (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--config") == 0) &&
            (strcmp(argv[i + 1], "-") == 0)) {
            i++;
        } else {
            argvVector.push_back(argv[i]);
        }
    }
    if (argc < 1 || cmdLine.AnalyzeParam(argvVector) < 0 || data.isHelp) {
        cmdLine.PrintHelp();
        exit(0);
    }

    if (config.empty() && !data.configFile.empty()) {
        if (!ParseConfig(data.configFile, config)) {
            return -1;
        }
    }

    if (data.isStartProcess) {
        if (!StartDependentProcess()) {
            if (data.isKillProcess) {
                KillDependentProcess();
            }
            return 0;
        }
    }

    if (data.isGetGrpcAddr) { // handle get port
        int ret = CheckGrpcMsgSend();
        if (data.isKillProcess) {
            KillDependentProcess();
        }
        return ret;
    }

    if (data.isShowPluginList) { // handle show plugin list
        std::string content = "";
        GetCapabilities(content, false);
        if (data.isKillProcess) {
            KillDependentProcess();
        }
        return 0;
    }

    if (config.empty()) { // normal case
        if (data.isKillProcess) {
            KillDependentProcess();
            return 1;
        }
        if (!data.isStartProcess) {
            printf("config file argument must sepcified!\n");
        }
        return 1;
    }
    // do capture work
    if (DoCapture(config, data.traceKeepSecond, data.outputFile)) {
        printf("DONE\n");
    }

    if (data.isKillProcess) {
        KillDependentProcess();
    }
    return 0;
}
