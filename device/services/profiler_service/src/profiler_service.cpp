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
#define LOG_TAG "ProfilerService"
#include "profiler_service.h"
#include <algorithm>
#include <unistd.h>
#include "common.h"
#include "logging.h"
#include "native_hook_config.pb.h"
#include "plugin_service.h"
#include "plugin_session.h"
#include "plugin_session_manager.h"
#include "profiler_capability_manager.h"
#include "profiler_data_repeater.h"
#include "schedule_task_manager.h"
#include "trace_file_writer.h"

using namespace ::grpc;
using PluginContextPtr = std::shared_ptr<PluginContext>;

#define CHECK_REQUEST_RESPONSE(context, request, response)         \
    do {                                                          \
        CHECK_POINTER_NOTNULL(context, "context ptr invalid!");   \
        CHECK_POINTER_NOTNULL(request, "request ptr invalid!");    \
        CHECK_POINTER_NOTNULL(response, "response ptr invalid!"); \
    } while (0)

#define CHECK_POINTER_NOTNULL(ptr, errorMessage)                              \
    do {                                                                      \
        if ((ptr) == nullptr) {                                                 \
            HILOG_ERROR(LOG_CORE, "%s: FAILED, %s is null!", __func__, #ptr); \
            return {StatusCode::INTERNAL, errorMessage};                      \
        }                                                                     \
    } while (0)

#define CHECK_EXPRESSION_TRUE(expr, errorMessage)                            \
    do {                                                                     \
        if (!(expr)) {                                                       \
            HILOG_ERROR(LOG_CORE, "%s: FAILED, %s", __func__, errorMessage); \
            return {StatusCode::INTERNAL, (errorMessage)};                   \
        }                                                                    \
    } while (0)

namespace {
constexpr int MIN_SESSION_TIMEOUT_MS = 1000;
constexpr int MAX_SESSION_TIMEOUT_MS = 1000 * 3600;
constexpr int DEFAULT_KEEP_ALIVE_DELTA = 3000;
} // namespace

ProfilerService::ProfilerService(const PluginServicePtr& pluginService)
    : pluginService_(pluginService), pluginSessionManager_(std::make_shared<PluginSessionManager>(pluginService))
{
    pluginService_->SetPluginSessionManager(pluginSessionManager_);
}

ProfilerService::~ProfilerService() {}

ProfilerService::SessionContext::~SessionContext()
{
    HILOG_INFO(LOG_CORE, "~SessionContext id = %d", id);
    if (offlineTask.size() > 0) {
        ScheduleTaskManager::GetInstance().UnscheduleTask(offlineTask);
    }
    StopSessionExpireTask();
    service->pluginSessionManager_->RemovePluginSessions(pluginNames);
}

Status ProfilerService::GetCapabilities(ServerContext* context,
                                        const ::GetCapabilitiesRequest* request,
                                        ::GetCapabilitiesResponse* response)
{
    CHECK_REQUEST_RESPONSE(context, request, response);
    HILOG_INFO(LOG_CORE, "GetCapabilities %d start", request->request_id());
    std::vector<ProfilerPluginCapability> capabilities = ProfilerCapabilityManager::GetInstance().GetCapabilities();

    response->set_status(StatusCode::OK);
    for (size_t i = 0; i < capabilities.size(); i++) {
        *response->add_capabilities() = capabilities[i];
    }
    HILOG_INFO(LOG_CORE, "GetCapabilities %d done!", request->request_id());
    return Status::OK;
}

size_t ProfilerService::SessionContext::UpdatePluginConfigs(const std::vector<ProfilerPluginConfig>& newPluginConfigs)
{
    std::unordered_map<std::string, size_t> nameIndex;
    for (size_t i = 0; i < pluginConfigs.size(); i++) {
        nameIndex[pluginConfigs[i].name()] = i;
    }

    size_t updateCount = 0;
    for (auto& cfg : newPluginConfigs) {
        auto it = nameIndex.find(cfg.name());
        if (it != nameIndex.end()) {
            int index = it->second;
            pluginConfigs[index] = cfg;
            updateCount++;
        }
    }
    return updateCount;
}

bool ProfilerService::SessionContext::CreatePluginSessions()
{
    if (bufferConfigs.size() > 0) { // with buffer configs
        CHECK_TRUE(service->pluginSessionManager_->CreatePluginSessions(pluginConfigs, bufferConfigs, dataRepeater),
                   false, "create plugin sessions with buffer configs failed!");
    } else { // without buffer configs
        CHECK_TRUE(service->pluginSessionManager_->CreatePluginSessions(pluginConfigs, dataRepeater), false,
                   "create plugin sessions without buffer configs failed!");
    }
    return true;
}

bool ProfilerService::SessionContext::StartPluginSessions()
{
    std::unique_lock<std::mutex> lock(sessionMutex);

    // if dataRepeater exists, reset it to usable state.
    if (dataRepeater) {
        dataRepeater->Reset();
    }

    if (sessionConfig.session_mode() == ProfilerSessionConfig::OFFLINE) {
        uint32_t sampleDuration = sessionConfig.sample_duration();
        if (sampleDuration > 0) {
            offlineTask = "stop-session-" + std::to_string(id);
            std::weak_ptr<SessionContext> weakCtx(shared_from_this());
            // start offline trace timeout task
            ScheduleTaskManager::GetInstance().ScheduleTask(
                offlineTask,
                [weakCtx]() {
                    if (auto ctx = weakCtx.lock(); ctx != nullptr) {
                        ctx->StopPluginSessions();
                    }
                },
                std::chrono::milliseconds(0), // do not repeat
                std::chrono::milliseconds(sampleDuration));

            // keep_alive_time not set by client, but the sample_duration setted
            if (sessionConfig.keep_alive_time() == 0) {
                // use sample_duration add a little time to set keep_alive_time
                SetKeepAliveTime(sampleDuration + DEFAULT_KEEP_ALIVE_DELTA);
                StartSessionExpireTask();
            }
        }
    }

    // start each plugin sessions
    service->pluginSessionManager_->StartPluginSessions(pluginNames);
    return true;
}

bool ProfilerService::SessionContext::StopPluginSessions()
{
    std::unique_lock<std::mutex> lock(sessionMutex);
    // stop each plugin sessions
    service->pluginSessionManager_->StopPluginSessions(pluginNames);

    if (sessionConfig.session_mode() == ProfilerSessionConfig::OFFLINE) {
        if (offlineTask.size() > 0) {
            ScheduleTaskManager::GetInstance().UnscheduleTask(offlineTask);
        }
        traceFileWriter->Finish();
    }

    // make sure FetchData thread exit
    if (dataRepeater) {
        dataRepeater->Close();
    }
    return true;
}

namespace {
bool IsValidKeepAliveTime(uint32_t timeout)
{
    if (timeout < MIN_SESSION_TIMEOUT_MS) {
        return false;
    }
    if (timeout > MAX_SESSION_TIMEOUT_MS) {
        return false;
    }
    return true;
}
}  // namespace

void ProfilerService::SessionContext::SetKeepAliveTime(uint32_t timeout)
{
    if (timeout > 0) {
        timeoutTask = "timeout-session-" + std::to_string(id);
        sessionConfig.set_keep_alive_time(timeout);
    }
}

void ProfilerService::SessionContext::StartSessionExpireTask()
{
    if (timeoutTask.size() > 0) {
        ScheduleTaskManager::GetInstance().ScheduleTask(timeoutTask,
                                                        std::bind(&ProfilerService::RemoveSessionContext, service, id),
                                                        std::chrono::milliseconds(0), // do not repeat
                                                        std::chrono::milliseconds(sessionConfig.keep_alive_time()));
    }
}

void ProfilerService::SessionContext::StopSessionExpireTask()
{
    if (timeoutTask.size() > 0) {
        ScheduleTaskManager::GetInstance().UnscheduleTask(timeoutTask);
    }
}

Status ProfilerService::CreateSession(ServerContext* context,
                                      const ::CreateSessionRequest* request,
                                      ::CreateSessionResponse* response)
{
    CHECK_REQUEST_RESPONSE(context, request, response);
    CHECK_POINTER_NOTNULL(pluginService_, "plugin service not ready!");
    // check plugin configs
    HILOG_INFO(LOG_CORE, "CreateSession %d start", request->request_id());
    const int nConfigs = request->plugin_configs_size();
    CHECK_EXPRESSION_TRUE(nConfigs > 0, "no plugin configs!");

    // check buffer configs
    ProfilerSessionConfig sessionConfig = request->session_config();
    pluginService_->SetProfilerSessionConfig(sessionConfig);
    const int nBuffers = sessionConfig.buffers_size();
    CHECK_EXPRESSION_TRUE(nBuffers == 0 || nBuffers == 1 || nBuffers == nConfigs, "buffers config invalid!");
    // copy plugin configs from request
    std::vector<ProfilerPluginConfig> pluginConfigs;
    pluginConfigs.reserve(nConfigs);

    for (int i = 0; i < nConfigs; i++) {
        if (request->plugin_configs(i).name() == "nativehook" && getuid() != 0) {
            NativeHookConfig hookConfig;
            std::string cfgData = request->plugin_configs(i).config_data();
            if (hookConfig.ParseFromArray(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()) <= 0) {
                HILOG_ERROR(LOG_CORE, "%s: ParseFromArray failed", __func__);
                continue;
            }
            if (!COMMON::CheckApplicationPermission(hookConfig.pid(), hookConfig.process_name())) {
                HILOG_ERROR(LOG_CORE, "Application debug permisson denied!");
                continue;
            }
        }
        pluginConfigs.push_back(request->plugin_configs(i));
    }

    if (pluginConfigs.empty()) {
        HILOG_ERROR(LOG_CORE, "No plugins are loaded!");
        return Status(StatusCode::PERMISSION_DENIED, "");
    }
    // copy buffer configs
    std::vector<BufferConfig> bufferConfigs;
    if (nBuffers == 1) {
        // if only one buffer config provided, all plugin use the same buffer config
        bufferConfigs.resize(pluginConfigs.size(), sessionConfig.buffers(0));
    } else if (nBuffers > 0) {
        // if more than one buffer config provided, the number of buffer configs must equals number of plugin configs
        bufferConfigs.assign(sessionConfig.buffers().begin(), sessionConfig.buffers().end());
    }
    HILOG_INFO(LOG_CORE, "bufferConfigs: %zu", bufferConfigs.size());
    std::vector<std::string> pluginNames;
    std::transform(pluginConfigs.begin(), pluginConfigs.end(), std::back_inserter(pluginNames),
                   [](ProfilerPluginConfig& config) { return config.name(); });
    std::sort(pluginNames.begin(), pluginNames.end());

    // create TraceFileWriter for offline mode
    TraceFileWriterPtr traceWriter;
    std::shared_ptr<ProfilerDataRepeater> dataRepeater = nullptr;
    if (sessionConfig.session_mode() == ProfilerSessionConfig::OFFLINE) {
        auto resultFile = sessionConfig.result_file();
        CHECK_EXPRESSION_TRUE(resultFile.size() > 0, "result_file empty!");
        traceWriter = std::make_shared<TraceFileWriter>(resultFile, sessionConfig.split_file(),
            sessionConfig.split_file_max_size_mb(), sessionConfig.split_file_max_num());
        CHECK_POINTER_NOTNULL(traceWriter, "alloc TraceFileWriter failed!");
        pluginService_->SetTraceWriter(traceWriter);
        for (std::vector<ProfilerPluginConfig>::size_type i = 0; i < pluginConfigs.size(); i++) {
            ProfilerPluginData pluginData;
            pluginData.set_name(pluginConfigs[i].name() + "_config");
            pluginData.set_data(pluginConfigs[i].config_data());
            std::vector<char> msgData(pluginData.ByteSizeLong());
            if (pluginData.SerializeToArray(msgData.data(), msgData.size()) <= 0) {
                HILOG_WARN(LOG_CORE, "PluginConfig SerializeToArray failed!");
            }
            traceWriter->SetPluginConfig(msgData.data(), msgData.size());
        }
        traceWriter->Flush();
    } else {
        dataRepeater = std::make_shared<ProfilerDataRepeater>(DEFAULT_REPEATER_BUFFER_SIZE);
        CHECK_POINTER_NOTNULL(dataRepeater, "alloc ProfilerDataRepeater failed!");
    }

    // create session context
    auto ctx = std::make_shared<SessionContext>();
    CHECK_POINTER_NOTNULL(ctx, "alloc SessionContext failed!");

    // fill fields of SessionContext
    ctx->service = this;
    if (dataRepeater != nullptr) {
        ctx->dataRepeater = dataRepeater;
    }
    if (traceWriter != nullptr) {
        ctx->traceFileWriter = traceWriter;
    }
    ctx->sessionConfig = sessionConfig;
    ctx->pluginNames = std::move(pluginNames);
    ctx->pluginConfigs = std::move(pluginConfigs);
    ctx->bufferConfigs = std::move(bufferConfigs);

    // create plugin sessions
    CHECK_EXPRESSION_TRUE(ctx->CreatePluginSessions(), "create plugin sessions failed!");
    // alloc new session id
    uint32_t sessionId = ++sessionIdCounter_;
    ctx->id = sessionId;
    ctx->name = "session-" + std::to_string(sessionId);

    // add {sessionId, ctx} to map
    CHECK_EXPRESSION_TRUE(AddSessionContext(sessionId, ctx), "sessionId conflict!");

    // create session expire schedule task
    auto keepAliveTime = sessionConfig.keep_alive_time();
    if (keepAliveTime) {
        CHECK_EXPRESSION_TRUE(IsValidKeepAliveTime(keepAliveTime), "keep_alive_time invalid!");
        // create schedule task for session timeout feature
        ctx->SetKeepAliveTime(keepAliveTime);
        ctx->StartSessionExpireTask();
    }

    // prepare response data fields
    response->set_status(0);
    response->set_session_id(sessionId);

    HILOG_INFO(LOG_CORE, "CreateSession %d %u done!", request->request_id(), sessionId);
    return Status::OK;
}

bool ProfilerService::AddSessionContext(uint32_t sessionId, const SessionContextPtr& sessionCtx)
{
    std::unique_lock<std::mutex> lock(sessionContextMutex_);
    CHECK_TRUE(sessionContext_.count(sessionId) == 0, false, "sessionId already exists!");
    sessionContext_[sessionId] = sessionCtx;
    return true;
}

ProfilerService::SessionContextPtr ProfilerService::GetSessionContext(uint32_t sessionId) const
{
    std::unique_lock<std::mutex> lock(sessionContextMutex_);
    auto it = sessionContext_.find(sessionId);
    if (it != sessionContext_.end()) {
        auto ptr = it->second;
        return ptr;
    }
    return nullptr;
}

bool ProfilerService::RemoveSessionContext(uint32_t sessionId)
{
    std::unique_lock<std::mutex> lock(sessionContextMutex_);
    auto it = sessionContext_.find(sessionId);
    if (it != sessionContext_.end()) {
        auto ptr = it->second;
        HILOG_INFO(LOG_CORE, "DelCtx use_count: %ld", ptr.use_count());
        sessionContext_.erase(it);
        return true;
    }
    return false;
}

void ProfilerService::MergeStandaloneFile(const std::string& resultFile, const std::string& pluginName,
    const std::string& outputFile, const std::string& pluginVersion)
{
    if (pluginName.empty() || outputFile.empty()) {
        HILOG_ERROR(LOG_CORE, "pluginName(%s) didn't set output file(%s)", pluginName.c_str(), outputFile.c_str());
        return;
    }

    std::ifstream fsFile {}; // read from output file
    fsFile.open(outputFile, std::ios_base::in | std::ios_base::binary);
    if (!fsFile.good()) {
        HILOG_ERROR(LOG_CORE, "open file(%s) failed: %d", outputFile.c_str(), fsFile.rdstate());
        return;
    }

    std::ofstream fsTarget {}; // write to profiler ouput file
    fsTarget.open(resultFile, std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    if (!fsTarget.good()) {
        HILOG_ERROR(LOG_CORE, "open file(%s) failed: %d", resultFile.c_str(), fsTarget.rdstate());
        return;
    }
    fsTarget.seekp(0, std::ios_base::end);
    int posFile = fsTarget.tellp(); // for update sha256

    TraceFileHeader header {};
    if (pluginName == "hiperf-plugin") {
        header.data_.dataType = DataType::HIPERF_DATA;
    } else {
        header.data_.dataType = DataType::STANDALONE_DATA;
    }
    fsFile.seekg(0, std::ios_base::end);
    uint64_t fileSize = (uint64_t)(fsFile.tellg());
    header.data_.length += fileSize;
    size_t pluginSize = sizeof(header.data_.standalonePluginName);
    int ret = strncpy_s(header.data_.standalonePluginName, pluginSize, pluginName.c_str(), pluginSize - 1);
    if (ret != EOK) {
        HILOG_ERROR(LOG_CORE, "strncpy_s error! pluginName is %s", pluginName.c_str());
        return;
    }
    pluginSize = sizeof(header.data_.pluginVersion);
    ret = strncpy_s(header.data_.pluginVersion, pluginSize, pluginVersion.c_str(), pluginSize - 1);
    if (ret != EOK) {
        HILOG_ERROR(LOG_CORE, "strncpy_s error! pluginVersion is %s", pluginVersion.c_str());
        return;
    }
    fsTarget.write(reinterpret_cast<char*>(&header), sizeof(header));
    if (!fsTarget.good()) {
        HILOG_ERROR(LOG_CORE, "write file(%s) header failed: %d\n", resultFile.c_str(), fsTarget.rdstate());
        return;
    }

    SHA256_CTX sha256Ctx;
    SHA256_Init(&sha256Ctx);
    constexpr uint64_t bufSize = 4 * 1024 * 1024;
    std::vector<char> buf(bufSize);
    uint64_t readSize = 0;
    fsFile.seekg(0);
    while ((readSize = std::min(bufSize, fileSize)) > 0) {
        fsFile.read(buf.data(), readSize);
        fsTarget.write(buf.data(), readSize);
        if (!fsTarget.good()) {
            HILOG_ERROR(LOG_CORE, "write file(%s) failed: %d\n", resultFile.c_str(), fsTarget.rdstate());
            return;
        }
        fileSize -= readSize;

        SHA256_Update(&sha256Ctx, buf.data(), readSize);
    }
    SHA256_Final(header.data_.sha256, &sha256Ctx);
    fsTarget.seekp(posFile, std::ios_base::beg);
    fsTarget.write(reinterpret_cast<char*>(&header), sizeof(header));

    fsFile.close();
    fsTarget.close();

    HILOG_INFO(LOG_CORE, "write standalone(%s) to result(%s) done", outputFile.c_str(), resultFile.c_str());
}

Status ProfilerService::StartSession(ServerContext* context,
                                     const ::StartSessionRequest* request,
                                     ::StartSessionResponse* response)
{
    CHECK_REQUEST_RESPONSE(context, request, response);

    uint32_t sessionId = request->session_id();
    HILOG_INFO(LOG_CORE, "StartSession %d %u start", request->request_id(), sessionId);

    // copy plugin configs from request
    std::vector<ProfilerPluginConfig> newPluginConfigs;
    newPluginConfigs.reserve(request->update_configs_size());
    for (int i = 0; i < request->update_configs_size(); i++) {
        HILOG_INFO(LOG_CORE, "update_configs %d, name = %s", i, request->update_configs(i).name().c_str());
        newPluginConfigs.push_back(request->update_configs(i));
    }

    // get plugin names in request
    std::vector<std::string> requestNames;
    std::transform(newPluginConfigs.begin(), newPluginConfigs.end(), std::back_inserter(requestNames),
                   [](auto& config) { return config.name(); });
    std::sort(requestNames.begin(), requestNames.end());

    auto ctx = GetSessionContext(sessionId);
    CHECK_POINTER_NOTNULL(ctx, "session_id invalid!");

    // get intersection set of requestNames and pluginNames
    std::vector<std::string> updateNames;
    std::set_intersection(requestNames.begin(), requestNames.end(), ctx->pluginNames.begin(), ctx->pluginNames.end(),
                          std::back_inserter(updateNames));

    if (updateNames.size() > 0) {
        // remove old plugin sessions
        pluginSessionManager_->RemovePluginSessions(updateNames);

        // update plugin configs
        size_t updates = ctx->UpdatePluginConfigs(newPluginConfigs);

        // re-create plugin sessions
        CHECK_EXPRESSION_TRUE(ctx->CreatePluginSessions(), "refresh sessions failed!");
        HILOG_INFO(LOG_CORE, "StartSession %zu plugin config updated!", updates);
    }

    // start plugin sessions with configs
    CHECK_EXPRESSION_TRUE(ctx->StartPluginSessions(), "start plugin sessions failed!");
    HILOG_INFO(LOG_CORE, "StartSession %d %u done!", request->request_id(), sessionId);
    return Status::OK;
}

Status ProfilerService::FetchData(ServerContext* context,
                                  const ::FetchDataRequest* request,
                                  ServerWriter<::FetchDataResponse>* writer)
{
    CHECK_POINTER_NOTNULL(context, "context ptr invalid!");
    CHECK_POINTER_NOTNULL(request, "request ptr invalid!");
    CHECK_POINTER_NOTNULL(writer, "writer ptr invalid!");

    CHECK_POINTER_NOTNULL(request, "request invalid!");
    CHECK_POINTER_NOTNULL(writer, "writer invalid!");

    uint32_t sessionId = request->session_id();
    HILOG_INFO(LOG_CORE, "FetchData %d %u start", request->request_id(), sessionId);

    auto ctx = GetSessionContext(sessionId);
    CHECK_POINTER_NOTNULL(ctx, "session_id invalid!");

    // check each plugin session states
    CHECK_EXPRESSION_TRUE(pluginSessionManager_->CheckStatus(ctx->pluginNames, PluginSession::STARTED),
                          "session status invalid!");

    if (ctx->sessionConfig.session_mode() == ProfilerSessionConfig::ONLINE) {
        auto dataRepeater = ctx->dataRepeater;
        CHECK_POINTER_NOTNULL(dataRepeater, "repeater invalid!");

        while (1) {
            ctx = GetSessionContext(sessionId);
            CHECK_POINTER_NOTNULL(ctx, "session_id invalid!");

            FetchDataResponse response;
            response.set_status(StatusCode::OK);
            response.set_response_id(++responseIdCounter_);

            std::vector<ProfilerPluginDataPtr> pluginDataVec;
            int count = dataRepeater->TakePluginData(pluginDataVec);
            if (count > 0) {
                response.set_has_more(true);
                for (int i = 0; i < count; i++) {
                    auto data = response.add_plugin_data();
                    CHECK_POINTER_NOTNULL(data, "new plugin data invalid");
                    CHECK_POINTER_NOTNULL(pluginDataVec[i], "plugin data invalid");
                    *data = *pluginDataVec[i];
                }
            } else {
                response.set_has_more(false);
                HILOG_INFO(LOG_CORE, "no more data need to fill to response!");
            }

            bool sendSuccess = writer->Write(response);
            if (count <= 0 || !sendSuccess) {
                HILOG_INFO(LOG_CORE, "count = %d, sendSuccess = %d", count, sendSuccess);
                break;
            }
        }
    }

    HILOG_INFO(LOG_CORE, "FetchData %d %u done!", request->request_id(), sessionId);
    return Status::OK;
}

Status ProfilerService::StopSession(ServerContext* context,
                                    const ::StopSessionRequest* request,
                                    ::StopSessionResponse* response)
{
    CHECK_REQUEST_RESPONSE(context, request, response);
    uint32_t sessionId = request->session_id();
    HILOG_INFO(LOG_CORE, "StopSession %d %u start", request->request_id(), sessionId);

    auto ctx = GetSessionContext(sessionId);
    CHECK_POINTER_NOTNULL(ctx, "session_id invalid!");
    CHECK_EXPRESSION_TRUE(ctx->StopPluginSessions(), "stop plugin sessions failed!");
    HILOG_INFO(LOG_CORE, "StopSession %d %u done!", request->request_id(), sessionId);
    return Status::OK;
}

Status ProfilerService::DestroySession(ServerContext* context,
                                       const ::DestroySessionRequest* request,
                                       ::DestroySessionResponse* response)
{
    CHECK_REQUEST_RESPONSE(context, request, response);

    uint32_t sessionId = request->session_id();
    HILOG_INFO(LOG_CORE, "DestroySession %d %u start", request->request_id(), sessionId);

    auto ctx = GetSessionContext(sessionId);
    CHECK_POINTER_NOTNULL(ctx, "session_id invalid!");

    CHECK_EXPRESSION_TRUE(RemoveSessionContext(sessionId), "remove session FAILED!");
    CHECK_EXPRESSION_TRUE(pluginSessionManager_->RemovePluginSessions(ctx->pluginNames),
                          "remove plugin session FAILED!");
    HILOG_INFO(LOG_CORE, "DestroySession %d %u done!", request->request_id(), sessionId);

    if (ctx->sessionConfig.session_mode() == ProfilerSessionConfig::OFFLINE) {
        uint32_t pluginId = 0;
        PluginContextPtr pluginCtx = nullptr;
        for (size_t i = 0; i < ctx->pluginNames.size(); i++) {
            auto pluginName = ctx->pluginNames[i];
            std::tie(pluginId, pluginCtx) = pluginService_->GetPluginContext(pluginName);
            if (pluginCtx->isStandaloneFileData == true) {
                std::string file = ctx->sessionConfig.result_file();
                if (ctx->sessionConfig.split_file()) {
                    file = ctx->traceFileWriter.get()->Path();
                }
                MergeStandaloneFile(file, pluginName, pluginCtx->outFileName, pluginCtx->pluginVersion);
            }
        }
    }

    return Status::OK;
}

::grpc::Status ProfilerService::KeepSession(::grpc::ServerContext* context,
                                            const ::KeepSessionRequest* request,
                                            ::KeepSessionResponse* response)
{
    CHECK_REQUEST_RESPONSE(context, request, response);
    uint32_t sessionId = request->session_id();
    HILOG_INFO(LOG_CORE, "KeepSession %d %u start", request->request_id(), sessionId);

    auto ctx = GetSessionContext(sessionId);
    CHECK_POINTER_NOTNULL(ctx, "session_id invalid!");

    // update keep alive time if keep_alive_time parameter provided
    auto keepAliveTime = request->keep_alive_time();
    if (keepAliveTime) {
        CHECK_EXPRESSION_TRUE(IsValidKeepAliveTime(keepAliveTime), "keep_alive_time invalid!");
        ctx->SetKeepAliveTime(keepAliveTime);
    }

    // reschedule session timeout task
    if (ctx->timeoutTask.size() > 0) {
        ctx->StopSessionExpireTask();
        ctx->StartSessionExpireTask();
    }
    HILOG_INFO(LOG_CORE, "KeepSession %d %u done!", request->request_id(), sessionId);
    return Status::OK;
}

struct LoggingInterceptor : public grpc::experimental::Interceptor {
public:
    explicit LoggingInterceptor(grpc::experimental::ServerRpcInfo* info) : info_(info) {}

    void Intercept(experimental::InterceptorBatchMethods* methods) override
    {
        const char* method = info_->method();
        if (methods->QueryInterceptionHookPoint(experimental::InterceptionHookPoints::POST_SEND_MESSAGE)) {
            HILOG_DEBUG(LOG_CORE, "POST_SEND_MESSAGE method: %s", method);
        } else if (methods->QueryInterceptionHookPoint(experimental::InterceptionHookPoints::POST_RECV_MESSAGE)) {
            HILOG_DEBUG(LOG_CORE, "POST_RECV_MESSAGE method: %s", method);
        }
        methods->Proceed();
    }

private:
    grpc::experimental::ServerRpcInfo* info_ = nullptr;
};

struct InterceptorFactory : public grpc::experimental::ServerInterceptorFactoryInterface {
protected:
    grpc::experimental::Interceptor* CreateServerInterceptor(grpc::experimental::ServerRpcInfo* info) override
    {
        return new LoggingInterceptor(info);
    }
};

bool ProfilerService::StartService(const std::string& listenUri)
{
    CHECK_TRUE(!listenUri.empty(), false, "listenUri empty!");

    ServerBuilder builder;
    builder.AddListeningPort(listenUri, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    auto server = builder.BuildAndStart();
    CHECK_NOTNULL(server, false, "start service failed!");
    HILOG_INFO(LOG_CORE, "Service started successfully.");
    server_ = std::move(server);
    return true;
}

void ProfilerService::WaitServiceDone()
{
    if (server_) {
        HILOG_INFO(LOG_CORE, "waiting Server...");
        server_->Wait();
        HILOG_INFO(LOG_CORE, "Server done!");
    }
}

void ProfilerService::StopService()
{
    if (server_) {
        server_->Shutdown();
        HILOG_INFO(LOG_CORE, "Server stop done!");
    }
}
