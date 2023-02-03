/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "command_poller.h"
#include "buffer_writer.h"
#include "plugin_manager.h"
#include "socket_context.h"

#include <fcntl.h>
#include <unistd.h>

namespace {
constexpr int SLEEP_TIME = 10;
}

CommandPoller::CommandPoller(const ManagerInterfacePtr& p) : requestIdAutoIncrease_(1), pluginManager_(p) {}

CommandPoller::~CommandPoller() {}

uint32_t CommandPoller::GetRequestId()
{
    return requestIdAutoIncrease_++;
}

bool CommandPoller::OnConnect()
{
    return Connect(DEFAULT_UNIX_SOCKET_FULL_PATH);
}

bool CommandPoller::OnCreateSessionCmd(const CreateSessionCmd& cmd, SocketContext& context) const
{
    HILOG_DEBUG(LOG_CORE, "%s:proc", __func__);
    if (cmd.buffer_sizes().size() == 0 || cmd.plugin_configs().size() == 0) {
        HILOG_ERROR(LOG_CORE, "%s:cmd invalid!", __func__);
        return false;
    }
    uint32_t bufferSize = cmd.buffer_sizes(0);
    ProfilerPluginConfig config = cmd.plugin_configs(0);
    std::vector<ProfilerPluginConfig> configVec;
    configVec.push_back(config);

    auto pluginManager = pluginManager_.lock(); // promote to shared_ptr
    CHECK_NOTNULL(pluginManager, false, "promote FAILED!");

    if (!pluginManager->LoadPlugin(config.name())) {
        HILOG_DEBUG(LOG_CORE, "%s:fail 1", __func__);
        return false;
    }
    int smbFd = -1;
    int eventFd = -1;
    if (bufferSize != 0) {
        HILOG_DEBUG(LOG_CORE, "%s:bufferSize = %d", __func__, bufferSize);
        smbFd = context.ReceiveFileDiscriptor();
        eventFd = context.ReceiveFileDiscriptor();
        int flags = fcntl(eventFd, F_GETFL);
        HILOG_DEBUG(LOG_CORE, "%s:smbFd = %d, eventFd = %d", __func__, smbFd, eventFd);
        HILOG_DEBUG(LOG_CORE, "%s:eventFd flags = %X", __func__, flags);
    }
    if (!pluginManager->CreateWriter(config.name(), bufferSize, smbFd, eventFd)) {
        HILOG_DEBUG(LOG_CORE, "%s:createWriter failed!", __func__);
        return false;
    }
    if (!pluginManager->CreatePluginSession(configVec)) {
        HILOG_DEBUG(LOG_CORE, "%s:createPluginSession failed!", __func__);
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "%s:ok", __func__);
    return true;
}

bool CommandPoller::OnDestroySessionCmd(const DestroySessionCmd& cmd) const
{
    HILOG_DEBUG(LOG_CORE, "%s:proc", __func__);
    if (cmd.plugin_ids().size() == 0) {
        HILOG_ERROR(LOG_CORE, "%s:cmd invalid!", __func__);
        return false;
    }
    uint32_t pluginId = cmd.plugin_ids(0);
    std::vector<uint32_t> pluginIdVec;
    pluginIdVec.push_back(pluginId);

    auto pluginManager = pluginManager_.lock(); // promote to shared_ptr
    CHECK_NOTNULL(pluginManager, false, "promote FAILED!");

    if (!pluginManager->DestroyPluginSession(pluginIdVec)) {
        HILOG_DEBUG(LOG_CORE, "%s:destroyPluginSession failed!", __func__);
        return false;
    }
    if (!pluginManager->ResetWriter(pluginId)) {
        HILOG_DEBUG(LOG_CORE, "%s:resetWriter failed!", __func__);
        return false;
    }
    if (!pluginManager->UnloadPlugin(pluginId)) {
        HILOG_DEBUG(LOG_CORE, "%s:unloadPlugin failed!", __func__);
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "%s:ok", __func__);
    return true;
}

bool CommandPoller::OnStartSessionCmd(const StartSessionCmd& cmd, PluginResult& result) const
{
    HILOG_DEBUG(LOG_CORE, "%s:proc", __func__);
    if (cmd.plugin_ids().size() == 0 || cmd.plugin_configs().size() == 0) {
        HILOG_ERROR(LOG_CORE, "%s:cmd invalid!", __func__);
        return false;
    }
    std::vector<uint32_t> pluginIds;
    pluginIds.push_back(cmd.plugin_ids(0));
    std::vector<ProfilerPluginConfig> configVec;
    configVec.push_back(cmd.plugin_configs(0));

    auto pluginManager = pluginManager_.lock(); // promote to shared_ptr
    CHECK_NOTNULL(pluginManager, false, "promote FAILED!");

    if (!pluginManager->StartPluginSession(pluginIds, configVec, result)) {
        HILOG_DEBUG(LOG_CORE, "%s:start Session failed!", __func__);
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "%s:OK", __func__);
    return true;
}

bool CommandPoller::OnStopSessionCmd(const StopSessionCmd& cmd) const
{
    HILOG_DEBUG(LOG_CORE, "%s:proc", __func__);
    if (cmd.plugin_ids().size() == 0) {
        HILOG_ERROR(LOG_CORE, "%s:cmd invalid!", __func__);
        return false;
    }
    std::vector<uint32_t> pluginIds;
    pluginIds.push_back(cmd.plugin_ids(0));

    auto pluginManager = pluginManager_.lock(); // promote to shared_ptr
    CHECK_NOTNULL(pluginManager, false, "promote FAILED!");

    if (!pluginManager->StopPluginSession(pluginIds)) {
        HILOG_DEBUG(LOG_CORE, "%s:stop Session failed!", __func__);
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "%s:ok", __func__);
    return true;
}

bool CommandPoller::OnReportBasicDataCmd(const RefreshSessionCmd& cmd) const
{
    HILOG_DEBUG(LOG_CORE, "%s:proc", __func__);
    if (cmd.plugin_ids().size() == 0) {
        HILOG_ERROR(LOG_CORE, "%s:cmd invalid!", __func__);
        return false;
    }
    std::vector<uint32_t> pluginIds;
    pluginIds.push_back(cmd.plugin_ids(0));

    auto pluginManager = pluginManager_.lock(); // promote to shared_ptr
    CHECK_NOTNULL(pluginManager, false, "promote FAILED!");

    if (!pluginManager->ReportPluginBasicData(pluginIds)) {
        HILOG_ERROR(LOG_CORE, "%s:report basic data failed!", __func__);
        return false;
    }
    HILOG_INFO(LOG_CORE, "%s:ok", __func__);
    return true;
}

bool CommandPoller::OnGetCommandResponse(SocketContext& context, ::GetCommandResponse& response)
{
    HILOG_DEBUG(LOG_CORE, "%s:proc", __func__);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    NotifyResultRequest nrr;
    nrr.set_request_id(1);
    nrr.set_command_id(response.command_id());
    PluginResult* pr = nrr.add_result();
    ProfilerPluginState* status = pr->mutable_status();

    if (response.has_create_session_cmd()) {
        if (OnCreateSessionCmd(response.create_session_cmd(), context)) {
            status->set_state(ProfilerPluginState::LOADED);
        } else {
            status->set_state(ProfilerPluginState::REGISTERED);
        }
    } else if (response.has_destroy_session_cmd()) {
        if (OnDestroySessionCmd(response.destroy_session_cmd())) {
            status->set_state(ProfilerPluginState::REGISTERED);
        } else {
            status->set_state(ProfilerPluginState::LOADED);
        }
    } else if (response.has_start_session_cmd()) {
        if (OnStartSessionCmd(response.start_session_cmd(), *pr)) {
            status->set_state(ProfilerPluginState::IN_SESSION);
        } else {
            status->set_state(ProfilerPluginState::LOADED);
        }
    } else if (response.has_stop_session_cmd()) {
        if (OnStopSessionCmd(response.stop_session_cmd())) {
            status->set_state(ProfilerPluginState::LOADED);
        } else {
            status->set_state(ProfilerPluginState::IN_SESSION);
        }
    } else if (response.has_refresh_session_cmd()) {
        OnReportBasicDataCmd(response.refresh_session_cmd());
        status->set_state(ProfilerPluginState::IN_SESSION);
    } else {
        HILOG_DEBUG(LOG_CORE, "%s:command Response failed!", __func__);
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "%s:ok id = %d", __func__, nrr.command_id());
    NotifyResult(nrr);
    return true;
}
