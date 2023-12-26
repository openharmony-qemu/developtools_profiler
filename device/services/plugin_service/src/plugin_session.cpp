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
#define LOG_TAG "PluginSession"
#include "plugin_session.h"
#include "logging.h"
#include "plugin_service.h"

PluginSession::PluginSession(const ProfilerPluginConfig& pluginConfig,
                             const PluginServiceWeakPtr& pluginService,
                             const ProfilerDataRepeaterPtr& dataRepeater)
    : state_(PluginSession::INITIAL),
      withBufferConfig_(false),
      pluginConfig_(pluginConfig),
      pluginService_(pluginService),
      dataRepeater_(dataRepeater)
{
    Create();
}

PluginSession::PluginSession(const ProfilerPluginConfig& pluginConfig,
                             const ProfilerSessionConfig::BufferConfig& bufferConfig,
                             const PluginServiceWeakPtr& pluginService,
                             const ProfilerDataRepeaterPtr& dataRepeater)
    : state_(PluginSession::INITIAL),
      withBufferConfig_(true),
      pluginConfig_(pluginConfig),
      bufferConfig_(bufferConfig),
      pluginService_(pluginService),
      dataRepeater_(dataRepeater)
{
    Create();
}

PluginSession::~PluginSession()
{
    if (state_ != INITIAL) {
        Destroy();
    }
}

bool PluginSession::Create()
{
    std::unique_lock<std::mutex> lock(mutex_);
    HILOG_INFO(LOG_CORE, "CreatePluginSession for %s...", pluginConfig_.name().c_str());
    CHECK_TRUE(state_ == INITIAL, false, "plugin state %d invalid!", state_);

    auto pluginService = pluginService_.lock(); // promote to shared_ptr
    CHECK_NOTNULL(pluginService, false, "PluginSession::%s pluginService promote failed!", __func__);

    bool retval = false;
    if (withBufferConfig_) {
        retval = pluginService->CreatePluginSession(pluginConfig_, bufferConfig_, dataRepeater_);
        HILOG_INFO(LOG_CORE, "CreatePluginSession with buffer for %s %s!", pluginConfig_.name().c_str(),
                   retval ? "OK" : "FAIL");
    } else {
        retval = pluginService->CreatePluginSession(pluginConfig_, dataRepeater_);
        HILOG_INFO(LOG_CORE, "CreatePluginSession for %s %s!", pluginConfig_.name().c_str(), retval ? "OK" : "FAIL");
    }
    CHECK_TRUE(retval, false, "call PluginService::CreatePluginSession failed!");

    state_ = CREATED;
    return retval;
}

bool PluginSession::Destroy()
{
    std::unique_lock<std::mutex> lock(mutex_);
    HILOG_INFO(LOG_CORE, "DestroyPluginSession for %s...", pluginConfig_.name().c_str());
    RETURN_IF(state_ == INITIAL, false, "plugin state %d, no need to destroy!", state_);
    CHECK_TRUE(state_ == CREATED || state_ == STARTED, false, "plugin state %d invalid!", state_);

    auto pluginService = pluginService_.lock();
    CHECK_NOTNULL(pluginService, false, "PluginSession::%s pluginService promote failed!", __func__);

    if (state_ == STARTED) {
        HILOG_INFO(LOG_CORE, "PluginSession::Destroy state is STARED, need stop!");
        StopLocked();
    }

    bool retval = pluginService->DestroyPluginSession(pluginConfig_.name());
    HILOG_INFO(LOG_CORE, "DestroyPluginSession for %s %s!", pluginConfig_.name().c_str(), retval ? "OK" : "FAIL");
    CHECK_TRUE(retval, false, "call PluginService::DestroyPluginSession failed!");

    state_ = INITIAL;
    return true;
}

bool PluginSession::IsAvailable() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return state_ != INITIAL;
}

PluginSession::State PluginSession::GetState() const
{
    std::unique_lock<std::mutex> lock(mutex_);
    return state_;
}

void PluginSession::Invalidate()
{
    std::unique_lock<std::mutex> lock(mutex_);
    state_ = INVALID;
}

bool PluginSession::Start()
{
    std::unique_lock<std::mutex> lock(mutex_);
    HILOG_INFO(LOG_CORE, "StartPluginSession for %s...", pluginConfig_.name().c_str());
    CHECK_TRUE(state_ == CREATED, false, "plugin state %d invalid!", state_);

    auto pluginService = pluginService_.lock();
    CHECK_NOTNULL(pluginService, false, "PluginSession::%s pluginService promote failed!", __func__);

    bool retval = pluginService->StartPluginSession(pluginConfig_);
    HILOG_INFO(LOG_CORE, "StartPluginSession for %s %s!", pluginConfig_.name().c_str(), retval ? "OK" : "FAIL");
    CHECK_TRUE(retval, false, "call PluginService::StartPluginSession failed!");

    state_ = STARTED;
    return retval;
}

bool PluginSession::Refresh()
{
    std::unique_lock<std::mutex> lock(mutex_);
    HILOG_INFO(LOG_CORE, "Refresh for %s...", pluginConfig_.name().c_str());
    CHECK_TRUE(state_ == STARTED, false, "plugin state %d invalid!", state_);

    auto pluginService = pluginService_.lock();
    CHECK_NOTNULL(pluginService, false, "PluginSession::%s pluginService promote failed!", __func__);

    bool retval = pluginService->RefreshPluginSession(pluginConfig_.name());
    HILOG_INFO(LOG_CORE, "RefreshPluginSession for %s %s!", pluginConfig_.name().c_str(), retval ? "OK" : "FAIL");
    CHECK_TRUE(retval, false, "call PluginService::RefreshPluginSession failed!");

    return retval;
}

bool PluginSession::Stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return StopLocked();
}

bool PluginSession::StopLocked()
{
    HILOG_INFO(LOG_CORE, "StopPluginSession for %s...", pluginConfig_.name().c_str());
    CHECK_TRUE(state_ == STARTED, false, "plugin state %d invalid!", state_);

    auto pluginService = pluginService_.lock();
    CHECK_NOTNULL(pluginService, false, "PluginSession::%s pluginService promote failed!", __func__);

    bool retval = pluginService->StopPluginSession(pluginConfig_.name());
    HILOG_INFO(LOG_CORE, "StopPluginSession for %s %s!", pluginConfig_.name().c_str(), retval ? "OK" : "FAIL");
    CHECK_TRUE(retval, false, "call PluginService::StopPluginSession failed!");

    state_ = CREATED;
    return retval;
}