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
#ifndef PROFILER_SERVICE_H
#define PROFILER_SERVICE_H

#include <grpcpp/grpcpp.h>
#include <memory>
#include "logging.h"
#include "nocopyable.h"
#include "hiperf_plugin_config.pb.h"
#include "profiler_service.grpc.pb.h"

class PluginService;

class PluginSession;
class TraceFileWriter;
class ProfilerDataRepeater;
class PluginSessionManager;

using PluginSessionPtr = STD_PTR(shared, PluginSession);
using PluginServicePtr = STD_PTR(shared, PluginService);
using PluginSessionManagerPtr = STD_PTR(shared, PluginSessionManager);

class ProfilerService : public IProfilerService::Service {
public:
    ProfilerService(const PluginServicePtr& pluginService = nullptr);

    ~ProfilerService();

    // get all plugin infos and capabilities.
    ::grpc::Status GetCapabilities(::grpc::ServerContext* context,
                                   const ::GetCapabilitiesRequest* request,
                                   ::GetCapabilitiesResponse* response) override;

    // create tracing sesion and pass tracing config to plugins.
    ::grpc::Status CreateSession(::grpc::ServerContext* context,
                                 const ::CreateSessionRequest* request,
                                 ::CreateSessionResponse* response) override;

    // start tracing session, active server side tracing triggers.
    ::grpc::Status StartSession(::grpc::ServerContext* context,
                                const ::StartSessionRequest* request,
                                ::StartSessionResponse* response) override;

    // get server-side cached tracing data since current session started.
    ::grpc::Status FetchData(::grpc::ServerContext* context,
                             const ::FetchDataRequest* request,
                             ::grpc::ServerWriter<::FetchDataResponse>* writer) override;

    // stop tracing session, deactive server side tracing triggers.
    ::grpc::Status StopSession(::grpc::ServerContext* context,
                               const ::StopSessionRequest* request,
                               ::StopSessionResponse* response) override;

    // destroy tracing session.
    ::grpc::Status DestroySession(::grpc::ServerContext* context,
                                  const ::DestroySessionRequest* request,
                                  ::DestroySessionResponse* response) override;

    // keep tracing session alive.
    ::grpc::Status KeepSession(::grpc::ServerContext* context,
                               const ::KeepSessionRequest* request,
                               ::KeepSessionResponse* response) override;

    // only used in main, for service control.
    bool StartService(const std::string& listenUri);
    void WaitServiceDone();
    void StopService();

private:
    static constexpr size_t DEFAULT_REPEATER_BUFFER_SIZE = 2000;

    using BufferConfig = ProfilerSessionConfig::BufferConfig;

    struct SessionContext : public std::enable_shared_from_this<SessionContext> {
        uint32_t id = 0;
        std::string name;
        std::string offlineTask; // offline sample task name
        std::string timeoutTask; // session timeout task name
        ProfilerService* service = nullptr;
        std::mutex sessionMutex;
        ProfilerSessionConfig sessionConfig;
        std::vector<std::string> pluginNames;
        std::vector<BufferConfig> bufferConfigs;
        std::vector<ProfilerPluginConfig> pluginConfigs;
        std::shared_ptr<ProfilerDataRepeater> dataRepeater {nullptr};
        std::shared_ptr<TraceFileWriter> traceFileWriter {nullptr};

        SessionContext() = default;
        ~SessionContext();

        bool CreatePluginSessions();
        bool StartPluginSessions();
        bool StopPluginSessions();

        size_t UpdatePluginConfigs(const std::vector<ProfilerPluginConfig>& newPluginConfigs);

        void SetKeepAliveTime(uint32_t timeout);
        void StartSessionExpireTask();
        void StopSessionExpireTask();
        DISALLOW_COPY_AND_MOVE(SessionContext);
    };

    using SessionContextPtr = STD_PTR(shared, SessionContext);

    SessionContextPtr GetSessionContext(uint32_t sessionId) const;

    bool AddSessionContext(uint32_t sessionId, const SessionContextPtr& sessionCtx);

    bool RemoveSessionContext(uint32_t sessionId);

    void MergeStandaloneFile(const std::string& resultFile, const std::string& pluginName,
        const std::string& outputFile, const std::string& pluginVersion);

private:
    mutable std::mutex sessionContextMutex_ = {};
    PluginServicePtr pluginService_ = nullptr;
    PluginSessionManagerPtr pluginSessionManager_ = nullptr;
    std::atomic<uint32_t> sessionIdCounter_ = 0;
    std::atomic<uint32_t> responseIdCounter_ = 0;
    std::map<uint32_t, SessionContextPtr> sessionContext_ = {};
    std::unique_ptr<grpc::Server> server_ = nullptr;

    DISALLOW_COPY_AND_MOVE(ProfilerService);
};

#endif // PROFILER_SERVICE_H
