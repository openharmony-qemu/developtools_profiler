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
#ifndef PROFILER_CAPABILITY_MANAGER_H
#define PROFILER_CAPABILITY_MANAGER_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "nocopyable.h"
#include "profiler_service_types.pb.h"

class ProfilerCapabilityManager {
public:
    static ProfilerCapabilityManager& GetInstance();

    bool AddCapability(const ProfilerPluginCapability& capability);

    std::vector<ProfilerPluginCapability> GetCapabilities() const;

    const ProfilerPluginCapability* GetCapability(const std::string& name) const;

    bool UpdateCapability(const std::string& name, const ProfilerPluginCapability& capability);

    bool RemoveCapability(const std::string& name);

private:
    ProfilerCapabilityManager() = default;
    ~ProfilerCapabilityManager();

private:
    mutable std::mutex mutex_;
    std::vector<ProfilerPluginCapability> pluginCapabilities_;

    DISALLOW_COPY_AND_MOVE(ProfilerCapabilityManager);
};

#endif
