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

#ifndef PLUGIN_SESSION_H
#define PLUGIN_SESSION_H

#include <mutex>
#include "logging.h"
#include "nocopyable.h"
#include "plugin_service_types.pb.h"
#include "profiler_service_types.pb.h"

class PluginService;
class ProfilerDataRepeater;

using PluginServiceWeakPtr = STD_PTR(weak, PluginService);
using ProfilerDataRepeaterPtr = STD_PTR(shared, ProfilerDataRepeater);

class PluginSession {
public:
    PluginSession(const ProfilerPluginConfig& pluginConfig,
                  const PluginServiceWeakPtr& pluginService,
                  const ProfilerDataRepeaterPtr& dataRepeater);
    PluginSession(const ProfilerPluginConfig& pluginConfig,
                  const ProfilerSessionConfig::BufferConfig& bufferConfig,
                  const PluginServiceWeakPtr& pluginService,
                  const ProfilerDataRepeaterPtr& dataRepeater);
    ~PluginSession();

    bool Start();
    bool Stop();
    bool Refresh();

    bool IsAvailable() const;

    enum State {
        INVALID = -1,
        INITIAL = 0,
        CREATED = 1,
        STARTED = 2,
    };

    void Invalidate();

    State GetState() const;

private:
    bool Create();
    bool StopLocked();
    bool Destroy();

private:
    mutable std::mutex mutex_;
    State state_;
    bool withBufferConfig_;
    ProfilerPluginConfig pluginConfig_;
    ProfilerSessionConfig::BufferConfig bufferConfig_;
    PluginServiceWeakPtr pluginService_;
    ProfilerDataRepeaterPtr dataRepeater_;

    DISALLOW_COPY_AND_MOVE(PluginSession);
};

#endif // !PLUGIN_SESSION_H
