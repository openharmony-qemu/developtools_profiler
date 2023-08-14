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

#include "plugin_command_builder.h"
#include "common_types.pb.h"
#include "logging.h"
#include "plugin_service_types.pb.h"

PluginCommandBuilder::PluginCommandBuilder()
{
    cmdIdAutoIncrease_ = 1;
}

PluginCommandBuilder::~PluginCommandBuilder() {}

GetCommandResponsePtr PluginCommandBuilder::BuildCreateSessionCmd(const ProfilerPluginConfig& config,
                                                                  uint32_t bufferSize)
{
    auto cmd = std::make_shared<GetCommandResponse>();
    cmd->set_status(ResponseStatus::OK);
    cmd->set_has_more(false);
    cmd->set_command_id(cmdIdAutoIncrease_);
    CreateSessionCmd* csc = cmd->mutable_create_session_cmd();
    csc->add_buffer_sizes(bufferSize);
    ProfilerPluginConfig* ppc = csc->add_plugin_configs();
    *ppc = config;
    commandHistory_[cmdIdAutoIncrease_] = cmd;
    cmdIdAutoIncrease_++;
    return cmd;
}

GetCommandResponsePtr PluginCommandBuilder::BuildDestroySessionCmd(uint32_t pluginId)
{
    auto cmd = std::make_shared<GetCommandResponse>();
    cmd->set_status(ResponseStatus::OK);
    cmd->set_has_more(false);
    cmd->set_command_id(cmdIdAutoIncrease_);

    DestroySessionCmd* dsc = cmd->mutable_destroy_session_cmd();
    dsc->add_plugin_ids(pluginId);

    commandHistory_[cmdIdAutoIncrease_] = cmd;
    cmdIdAutoIncrease_++;
    return cmd;
}

GetCommandResponsePtr PluginCommandBuilder::BuildStartSessionCmd(const ProfilerPluginConfig& config, uint32_t pluginId)
{
    auto cmd = std::make_shared<GetCommandResponse>();
    cmd->set_status(ResponseStatus::OK);
    cmd->set_has_more(false);
    cmd->set_command_id(cmdIdAutoIncrease_);

    StartSessionCmd* ssc = cmd->mutable_start_session_cmd();
    ssc->add_plugin_ids(pluginId);
    ProfilerPluginConfig* ppc = ssc->add_plugin_configs();
    *ppc = config;

    commandHistory_[cmdIdAutoIncrease_] = cmd;
    cmdIdAutoIncrease_++;
    return cmd;
}

GetCommandResponsePtr PluginCommandBuilder::BuildStopSessionCmd(uint32_t pluginId)
{
    auto cmd = std::make_shared<GetCommandResponse>();
    cmd->set_status(ResponseStatus::OK);
    cmd->set_has_more(false);
    cmd->set_command_id(cmdIdAutoIncrease_);

    StopSessionCmd* ssc = cmd->mutable_stop_session_cmd();
    ssc->add_plugin_ids(pluginId);

    commandHistory_[cmdIdAutoIncrease_] = cmd;
    cmdIdAutoIncrease_++;
    return cmd;
}

GetCommandResponsePtr PluginCommandBuilder::BuildRefreshSessionCmd(uint32_t pluginId)
{
    auto cmd = std::make_shared<GetCommandResponse>();
    cmd->set_status(ResponseStatus::OK);
    cmd->set_has_more(false);
    cmd->set_command_id(cmdIdAutoIncrease_);

    RefreshSessionCmd* rsc = cmd->mutable_refresh_session_cmd();
    rsc->add_plugin_ids(pluginId);

    commandHistory_[cmdIdAutoIncrease_] = cmd;
    cmdIdAutoIncrease_++;
    return cmd;
}

bool PluginCommandBuilder::GetedCommandResponse(uint32_t cmdId)
{
    if (commandHistory_.find(cmdId) == commandHistory_.end()) {
        return false;
    }
    commandHistory_.erase(cmdId);
    return true;
}