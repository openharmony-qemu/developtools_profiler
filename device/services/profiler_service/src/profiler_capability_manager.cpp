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

#include "profiler_capability_manager.h"

#include <algorithm>
#include "logging.h"

ProfilerCapabilityManager& ProfilerCapabilityManager::GetInstance()
{
    static ProfilerCapabilityManager instance;
    return instance;
}

ProfilerCapabilityManager::~ProfilerCapabilityManager()
{
    pluginCapabilities_.clear();
}

bool ProfilerCapabilityManager::AddCapability(const ProfilerPluginCapability& capability)
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto it = pluginCapabilities_.begin(); it != pluginCapabilities_.end(); it++) {
        CHECK_TRUE(it->name() != capability.name(), false,
                   "capability.name conflict with %zu", (it - pluginCapabilities_.begin()));
    }
    pluginCapabilities_.push_back(capability);
    HILOG_INFO(LOG_CORE, "AddCapability({%s, %s}) SUCCESS!", capability.name().c_str(), capability.path().c_str());
    return true;
}

const ProfilerPluginCapability* ProfilerCapabilityManager::GetCapability(const std::string& name) const
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (size_t i = 0; i < pluginCapabilities_.size(); i++) {
        if (pluginCapabilities_[i].name() == name) {
            return &pluginCapabilities_[i];
        }
    }
    return nullptr;
}

std::vector<ProfilerPluginCapability> ProfilerCapabilityManager::GetCapabilities() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return pluginCapabilities_;
}

bool ProfilerCapabilityManager::UpdateCapability(const std::string& name, const ProfilerPluginCapability& capability)
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto it = pluginCapabilities_.begin(); it != pluginCapabilities_.end(); it++) {
        if (it->name() == capability.name()) {
            *it = capability;
            HILOG_WARN(LOG_CORE, "UpdateCapability(%s, %s) SUCCESS!", it->name().c_str(), it->path().c_str());
            return true;
        }
    }
    return false;
}

bool ProfilerCapabilityManager::RemoveCapability(const std::string& name)
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto it = pluginCapabilities_.begin(); it != pluginCapabilities_.end(); it++) {
        if (it->name() == name) {
            HILOG_INFO(LOG_CORE, "RemoveCapability({%s, %s}) SUCCESS!", it->name().c_str(), it->path().c_str());
            pluginCapabilities_.erase(it);
            return true;
        }
    }
    return false;
}
