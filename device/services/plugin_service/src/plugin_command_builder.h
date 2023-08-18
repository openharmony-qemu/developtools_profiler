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

#ifndef PLUGIN_COMMAND_BUILDER_H
#define PLUGIN_COMMAND_BUILDER_H

#include "common_types.pb.h"
#include "logging.h"
#include "plugin_service_types.pb.h"

using GetCommandResponsePtr = STD_PTR(shared, GetCommandResponse);

class ProfilerPluginConfig;

class PluginCommandBuilder {
public:
    PluginCommandBuilder();
    ~PluginCommandBuilder();
    GetCommandResponsePtr BuildCreateSessionCmd(const ProfilerPluginConfig& config, uint32_t buffer_size);
    GetCommandResponsePtr BuildDestroySessionCmd(uint32_t pluginId);
    GetCommandResponsePtr BuildStartSessionCmd(const ProfilerPluginConfig& config, uint32_t pluginId);
    GetCommandResponsePtr BuildStopSessionCmd(uint32_t pluginId);
    GetCommandResponsePtr BuildRefreshSessionCmd(uint32_t pluginId);

    bool GetedCommandResponse(uint32_t cmdId);

private:
    std::map<uint32_t, GetCommandResponsePtr> commandHistory_;
    uint32_t cmdIdAutoIncrease_;
};

#endif // PLUGIN_COMMAND_BUILDER_H