/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#include "plugin_manager.h"

#include <cstdio>
#include <functional>
#include <iomanip>

#include "common.h"
#include "command_poller.h"
#include "logging.h"
#include "openssl/sha.h"
#include "plugin_service_types.pb.h"

namespace {
constexpr int FILE_READ_CHUNK_SIZE = 4096;
constexpr char HEX_CHARS[] = "0123456789abcdef";

#define HHB(v) (((v) & 0xF0) >> 4)
#define LHB(v)  ((v) & 0x0F)

std::string ComputeFileSha256(const std::string& path)
{
    uint8_t out[SHA256_DIGEST_LENGTH];
    uint8_t buffer[FILE_READ_CHUNK_SIZE];
    char realPath[PATH_MAX + 1] = {0};

    SHA256_CTX sha;
    SHA256_Init(&sha);

    size_t nbytes = 0;
    if ((path.length() >= PATH_MAX) || (realpath(path.c_str(), realPath) == nullptr)) {
        HILOG_ERROR(LOG_CORE, "%s:path is invalid: %s, errno=%d", __func__, path.c_str(), errno);
        return "";
    }
    std::unique_ptr<FILE, decltype(fclose)*> fptr(fopen(realPath, "rb"), fclose);
    while ((nbytes = fread(buffer, 1, sizeof(buffer), fptr.get())) > 0) {
        SHA256_Update(&sha, buffer, nbytes);
    }
    SHA256_Final(out, &sha);

    std::string result;
    result.reserve(SHA256_DIGEST_LENGTH + SHA256_DIGEST_LENGTH);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        result.push_back(HEX_CHARS[HHB(out[i])]);
        result.push_back(HEX_CHARS[LHB(out[i])]);
    }

    HILOG_DEBUG(LOG_CORE, "%s:%s-(%s)", __func__, path.c_str(), result.c_str());
    return result;
}
}  // namespace

PluginManager::~PluginManager() {}

void PluginManager::SetCommandPoller(const CommandPollerPtr& p)
{
    this->commandPoller_ = p;
}

bool PluginManager::AddPlugin(const std::string& pluginPath)
{
    PluginModuleInfo info = {"", 0};
    auto plugin = std::make_shared<PluginModule>(pluginPath);
    CHECK_TRUE(plugin->Load(), false, "%s:load failed!", __func__);

    if (!plugin->BindFunctions()) {
        HILOG_DEBUG(LOG_CORE, "%s:bindFunctions failed %s", __func__, pluginPath.c_str());
        plugin->Unload();
        return false;
    }

    if (!plugin->GetInfo(info)) {
        HILOG_DEBUG(LOG_CORE, "%s:getinfo failed!", __func__);
        plugin->Unload();
        return false;
    }

    std::string pluginName = info.name;
    if (pluginIds_.find(pluginName) != pluginIds_.end()) {
        HILOG_DEBUG(LOG_CORE, "%s:already add", __func__);
        plugin->Unload();
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "%s:add plugin name = %s", __func__, pluginName.c_str());

    if (!plugin->Unload()) {
        HILOG_DEBUG(LOG_CORE, "%s:unload failed!", __func__);
        return false;
    }

    return RegisterPlugin(plugin, pluginPath, info);
}

bool PluginManager::RegisterPlugin(const PluginModulePtr& plugin, const std::string& pluginPath,
                                   const PluginModuleInfo& pluginInfo)
{
    RegisterPluginRequest request;
    request.set_request_id(commandPoller_->GetRequestId());
    request.set_path(pluginPath);
    request.set_sha256(ComputeFileSha256(pluginPath));
    RegisterPluginResponse response;
    request.set_name(pluginInfo.name);
    request.set_buffer_size_hint(pluginInfo.bufferSizeHint);
    request.set_is_standalone_data(pluginInfo.isStandaloneFileData);
    request.set_out_file_name(pluginInfo.outFileName);
    request.set_plugin_version(pluginInfo.pluginVersion);
    if (commandPoller_->RegisterPlugin(request, response)) {
        if (response.status() == ResponseStatus::OK) {
            HILOG_DEBUG(LOG_CORE, "%s:response.plugin_id() = %d", __func__, response.plugin_id());
            pluginIds_[pluginInfo.name] = response.plugin_id();
            pluginModules_.insert(std::pair<uint32_t, std::shared_ptr<PluginModule>>(response.plugin_id(), plugin));
            pluginPathAndNameMap_.insert(
                {pluginPath, pluginInfo.name}
            );
            HILOG_DEBUG(LOG_CORE, "%s:registerPlugin ok", __func__);
        } else {
            HILOG_DEBUG(LOG_CORE, "%s:registerPlugin fail 1", __func__);
            return false;
        }
    } else {
        HILOG_DEBUG(LOG_CORE, "%s:registerPlugin fail 2", __func__);
        return false;
    }

    return true;
}

bool PluginManager::RemovePlugin(const std::string& pluginPath)
{
    CHECK_TRUE(pluginPathAndNameMap_.count(pluginPath) != 0, false,
               "%s:not find plugin: %s", __func__, pluginPath.c_str());

    std::string pluginName = pluginPathAndNameMap_[pluginPath];
    auto it = pluginIds_.find(pluginName);
    if (it == pluginIds_.end()) {
        HILOG_DEBUG(LOG_CORE, "%s:plugin not exist", __func__);
        return false;
    }
    uint32_t index = it->second;

    // stop plugin if plugin running
    if (pluginModules_[index]->IsRunning()) {
        HILOG_WARN(LOG_CORE, "%s:plugin delete while using, stop plugin", __func__);

        // delete schedule task if POLLING mode
        if (pluginModules_[index]->GetSampleMode() == PluginModule::SampleMode::POLLING) {
            HILOG_WARN(LOG_CORE, "%s:delete schedule task plugin name = %s", __func__, pluginName.c_str());
            if (!scheduleTaskManager_.UnscheduleTask(pluginName)) {
                HILOG_WARN(LOG_CORE, "%s:delete schedule task plugin name = %s failed!", __func__, pluginName.c_str());
            }
        }

        if (!pluginModules_[index]->StopSession()) {
            HILOG_WARN(LOG_CORE, "%s:plugin stop failed!", __func__);
        }
    }

    // Unload plugin if plugin loaded
    if (pluginModules_[index]->IsLoaded()) {
        HILOG_WARN(LOG_CORE, "%s:plugin delete while using, unload plugin", __func__);
        if (!pluginModules_[index]->Unload()) {
            HILOG_WARN(LOG_CORE, "%s:unload plugin failed!", __func__);
        }
    }

    UnregisterPluginRequest request;
    request.set_request_id(commandPoller_->GetRequestId());
    request.set_plugin_id(index);
    UnregisterPluginResponse response;
    if (commandPoller_->UnregisterPlugin(request, response)) {
        if (response.status() != ResponseStatus::OK) {
            HILOG_DEBUG(LOG_CORE, "%s:registerPlugin fail 1", __func__);
            return false;
        }
    } else {
        HILOG_DEBUG(LOG_CORE, "%s:registerPlugin fail 2", __func__);
        return false;
    }

    auto itPluginModules = pluginModules_.find(index);
    if (it == pluginIds_.end()) {
        HILOG_DEBUG(LOG_CORE, "%s:plugin not exist", __func__);
        return false;
    }
    pluginModules_.erase(itPluginModules);
    pluginIds_.erase(it);
    return true;
}

bool PluginManager::LoadPlugin(const std::string& pluginName)
{
    HILOG_DEBUG(LOG_CORE, "%s:size = %zu", __func__, pluginIds_.size());
    auto it = pluginIds_.find(pluginName);
    CHECK_TRUE(it != pluginIds_.end(), false, "%s:plugin not exist", __func__);
    uint32_t index = it->second;

    if (!pluginModules_[index]->Load()) {
        return false;
    }
    if (!pluginModules_[index]->BindFunctions()) {
        return false;
    }
    return true;
}

bool PluginManager::UnloadPlugin(const std::string& pluginName)
{
    auto it = pluginIds_.find(pluginName);
    if (it == pluginIds_.end()) {
        HILOG_DEBUG(LOG_CORE, "%s:plugin not exist", __func__);
        return false;
    }

    return UnloadPlugin(it->second);
}

bool PluginManager::UnloadPlugin(const uint32_t pluginId)
{
    HILOG_INFO(LOG_CORE, "%s:ready!", __func__);
    if (pluginModules_.find(pluginId) == pluginModules_.end()) {
        HILOG_DEBUG(LOG_CORE, "%s:plugin not exist", __func__);
        return false;
    }
    if (!pluginModules_[pluginId]->Unload()) {
        return false;
    }
    return true;
}

bool PluginManager::CreatePluginSession(const std::vector<ProfilerPluginConfig>& config)
{
    HILOG_DEBUG(LOG_CORE, "%s:ready", __func__);

    for (size_t idx = 0; idx < config.size(); ++idx) {
        HILOG_DEBUG(LOG_CORE, "%s:config->name() = %s", __func__, config[idx].name().c_str());
        auto it = pluginIds_.find(config[idx].name());
        if (it == pluginIds_.end()) {
            HILOG_DEBUG(LOG_CORE, "%s:plugin not find", __func__);
            return false;
        }

        HILOG_INFO(LOG_CORE, "%s:index = %d, clock = %s", __func__, it->second, config[idx].clock().c_str());
        pluginModules_[it->second]->SetConfigData(config[idx].config_data());
        pluginModules_[it->second]->SetClockId(COMMON::GetClockId(config[idx].clock()));
    }
    return true;
}

bool PluginManager::DestroyPluginSession(const std::vector<uint32_t>& pluginIds)
{
    for (uint32_t id : pluginIds) {
        auto it = pluginModules_.find(id);
        if (it == pluginModules_.end()) {
            HILOG_DEBUG(LOG_CORE, "%s:plugin not find", __func__);
            return false;
        }
    }
    return true;
}

bool PluginManager::StartPluginSession(const std::vector<uint32_t>& pluginIds,
                                       const std::vector<ProfilerPluginConfig>& config, PluginResult& result)
{
    HILOG_INFO(LOG_CORE, "%s:ready!", __func__);
    size_t idx = 0;

    for (uint32_t id : pluginIds) {
        auto it = pluginModules_.find(id);
        if (it == pluginModules_.end()) {
            HILOG_DEBUG(LOG_CORE, "%s:plugin not find", __func__);
            return false;
        }
        auto plugin = pluginModules_[id];
        auto cfgData = plugin->GetConfigData();
        if (!plugin->StartSession(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size())) {
            return false;
        }

        if (plugin->GetSampleMode() == PluginModule::SampleMode::POLLING) {
            if (idx > config.size()) {
                HILOG_WARN(LOG_CORE, "%s:idx %zu out of size %zu", __func__, idx, config.size());
                return false;
            }
            auto interval = ScheduleTaskManager::ms(config[idx].sample_interval());
            std::string pluginName = config[idx].name();
            HILOG_DEBUG(LOG_CORE, "%s:interval = %ld", __func__, static_cast<long>(interval.count()));
            HILOG_DEBUG(LOG_CORE, "%s:pluginName = %s", __func__, pluginName.c_str());
            auto callback = std::bind(&PluginManager::PullResult, this, id);
            if (interval.count() == 0) {
                HILOG_DEBUG(LOG_CORE, "%s:scheduleTask interval == 0 error!", __func__);
                return false;
            }
            if (!scheduleTaskManager_.ScheduleTask(pluginName, callback, interval)) {
                HILOG_DEBUG(LOG_CORE, "%s:scheduleTask failed!", __func__);
                return false;
            }
        }

        // need update standalone plugin output file name
        if (plugin->GetStandaloneFileData()) {
            std::string pluginOutFileName = "";
            plugin->GetOutFileName(pluginOutFileName);
            result.set_plugin_id(id);
            result.set_out_file_name(pluginOutFileName);
        }

        idx++;
    }

    return true;
}

bool PluginManager::StopPluginSession(const std::vector<uint32_t>& pluginIds)
{
    HILOG_INFO(LOG_CORE, "%s:ready!", __func__);
    for (uint32_t id : pluginIds) {
        if (pluginModules_.find(id) == pluginModules_.end()) {
            HILOG_DEBUG(LOG_CORE, "%s:plugin not find", __func__);
            return false;
        }
        if (pluginModules_[id]->GetSampleMode() == PluginModule::SampleMode::POLLING) {
            for (auto it : pluginIds_) {
                if (it.second == id) {
                    HILOG_DEBUG(LOG_CORE, "%s:find plugin name = %s", __func__, it.first.c_str());
                    if (!scheduleTaskManager_.UnscheduleTask(it.first)) {
                        return false;
                    }
                }
            }
        }
        if (!pluginModules_[id]->StopSession()) {
            return false;
        }
    }
    return true;
}

bool PluginManager::StopAllPluginSession()
{
    std::vector<uint32_t> vecPluginIds;
    for (std::map<uint32_t, std::shared_ptr<PluginModule>>::iterator it = pluginModules_.begin();
         it != pluginModules_.end(); ++it) {
        vecPluginIds.push_back(it->first);
    }
    return StopPluginSession(vecPluginIds);
}

bool PluginManager::ReportPluginBasicData(const std::vector<uint32_t>& pluginIds)
{
    HILOG_INFO(LOG_CORE, "%s:ready!", __func__);
    for (uint32_t id : pluginIds) {
        CHECK_TRUE(pluginModules_.find(id) != pluginModules_.end(), false, "%s:plugin not find", __func__);
        // notify plugin to report basic data
        CHECK_TRUE(pluginModules_[id]->ReportBasicData(), false, "%s:report basic data failed", __func__);
    }
    return true;
}

bool PluginManager::SubmitResult(const PluginResult& pluginResult)
{
    HILOG_INFO(LOG_CORE, "%s:ready!", __func__);
    NotifyResultRequest request;
    CHECK_NOTNULL(commandPoller_, false, "%s:commandPoller_ is null", __func__);
    request.set_request_id(commandPoller_->GetRequestId());
    request.set_command_id(0);
    PluginResult* p = request.add_result();
    *p = pluginResult;
    NotifyResultResponse response;
    if (!commandPoller_->NotifyResult(request, response)) {
        HILOG_DEBUG(LOG_CORE, "%s:fail 1", __func__);
        return false;
    }
    if (response.status() != ResponseStatus::OK) {
        HILOG_DEBUG(LOG_CORE, "%s:fail 2", __func__);
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "%s:ok", __func__);
    return true;
}

bool PluginManager::PullResult(uint32_t pluginId)
{
    uint32_t size = 0;
    std::string name = "";
    std::string version = "";
    auto it = pluginModules_.find(pluginId);
    if (it == pluginModules_.end()) {
        HILOG_DEBUG(LOG_CORE, "%s:plugin not find", __func__);
        return false;
    }
    pluginModules_[pluginId]->GetBufferSizeHint(size);
    pluginModules_[pluginId]->GetPluginName(name);
    pluginModules_[pluginId]->GetPluginVersion(version);
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(size);

    int length = it->second->ReportResult(buffer.get(), size);
    if (length < 0) {
        return false;
    }

    ProfilerPluginData pluginData;
    pluginData.set_name(name);
    pluginData.set_version(version);
    pluginData.set_status(0);
    pluginData.set_data(buffer.get(), length);

    struct timespec ts;
    clockid_t clockId = pluginModules_[pluginId]->GetClockId();
    clock_gettime(clockId, &ts);

    pluginData.set_clock_id(static_cast<ProfilerPluginData_ClockId>(clockId));
    pluginData.set_tv_sec(ts.tv_sec);
    pluginData.set_tv_nsec(ts.tv_nsec);

    auto writer = std::static_pointer_cast<BufferWriter>(pluginModules_[pluginId]->GetWriter());
    CHECK_NOTNULL(writer, false, "PullResult GetWriter nullptr");

    writer->WriteMessage(pluginData, name);
    writer->Flush();
    return true;
}

bool PluginManager::CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd)
{
    auto it = pluginIds_.find(pluginName);
    if (it == pluginIds_.end()) {
        HILOG_DEBUG(LOG_CORE, "%s:plugin not exist", __func__);
        return false;
    }
    uint32_t index = it->second;

    if (bufferSize > 0) {
        HILOG_DEBUG(LOG_CORE, "%s:%s Use ShareMemory %d", __func__, pluginName.c_str(), bufferSize);
        std::string pluginVersion = "";
        pluginModules_[index]->GetPluginVersion(pluginVersion);
        pluginModules_[index]->RegisterWriter(
            std::make_shared<BufferWriter>(pluginName, pluginVersion, bufferSize, smbFd, eventFd, index));
    } else {
        HILOG_ERROR(LOG_CORE, "%s:no shared memory buffer allocated!", __func__);
        return false;
    }
    return true;
}

bool PluginManager::ResetWriter(uint32_t pluginId)
{
    if (pluginModules_.find(pluginId) == pluginModules_.end()) {
        HILOG_DEBUG(LOG_CORE, "%s:plugin not exist", __func__);
        return false;
    }
    HILOG_DEBUG(LOG_CORE, "%s:resetWriter %u", __func__, pluginId);
    pluginModules_[pluginId]->RegisterWriter(nullptr);
    return true;
}
