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

#include "plugin_module.h"

#include <dlfcn.h>
#include <iostream>

#include "logging.h"
#include "plugin_module_api.h"

PluginModule::PluginModule(const std::string& path)
    : handle_(nullptr), running_(false), path_(path), structPtr_(nullptr) {}

PluginModule::~PluginModule() {}

std::string PluginModule::ComputeSha256()
{
    return "";
}

bool PluginModule::Load()
{
    char realPath[PATH_MAX + 1] = {0};
    if (handle_ != nullptr) {
        HILOG_DEBUG(LOG_CORE, "%s:already open", __func__);
        return false;
    }
    CHECK_TRUE((path_.length() < PATH_MAX) && (realpath(path_.c_str(), realPath) != nullptr), false,
               "%s:so filename invalid, errno=%d", __func__, errno);

    std::string rpath = realPath; // for SC warning
    handle_ = dlopen(rpath.c_str(), RTLD_NOW);
    if (handle_ == nullptr) {
        HILOG_DEBUG(LOG_CORE, "%s:dlopen err:%s.", __func__, dlerror());
        return false;
    }
    return true;
}

bool PluginModule::Unload()
{
    HILOG_INFO(LOG_CORE, "%s:unload ready!", __func__);
    if (handle_ != nullptr) {
        int ret = dlclose(handle_);
        HILOG_INFO(LOG_CORE, "%s:unload plugin ret = %d", __func__, ret);
        handle_ = nullptr;
        structPtr_ = nullptr;
        return true;
    }

    return false;
}

bool PluginModule::GetInfo(PluginModuleInfo& info)
{
    if (handle_ != nullptr) {
        if (structPtr_ == nullptr) {
            return false;
        }
        info.bufferSizeHint = structPtr_->resultBufferSizeHint;
        info.name.assign(structPtr_->name);
        info.isStandaloneFileData = structPtr_->isStandaloneFileData;
        info.outFileName.assign(structPtr_->outFileName);
        info.pluginVersion.assign(structPtr_->version);
        return true;
    }
    return false;
}

PluginModule::SampleMode PluginModule::GetSampleMode() const
{
    if (structPtr_ && structPtr_->callbacks) {
        if (structPtr_->callbacks->onPluginReportResult != nullptr) {
            return SampleMode::POLLING;
        } else if (structPtr_->callbacks->onRegisterWriterStruct != nullptr) {
            return SampleMode::STREAMING;
        }
    }
    return SampleMode::UNKNOWN;
}

void PluginModule::SetConfigData(const std::string& data)
{
    configData_ = data;
}

std::string PluginModule::GetConfigData() const
{
    return configData_;
}

void PluginModule::SetClockId(clockid_t clockId)
{
    clockId_ = clockId;
    HILOG_INFO(LOG_CORE, "SetClockId: plugin=%s, clock=%d", GetPluginName().c_str(), clockId);
    if (writerAdapter_ != nullptr && writerAdapter_->GetWriter() != nullptr) {
        writerAdapter_->GetWriter()->SetClockId(clockId);
    }
}

clockid_t PluginModule::GetClockId() const
{
    return clockId_;
}

bool PluginModule::GetPluginName(std::string& pluginName)
{
    if (handle_ != nullptr) {
        CHECK_NOTNULL(structPtr_, false, "structPtr_ is nullptr");
        pluginName.assign(structPtr_->name);
        return true;
    }
    return false;
}

bool PluginModule::GetBufferSizeHint(uint32_t& bufferSizeHint)
{
    if (handle_ != nullptr) {
        CHECK_NOTNULL(structPtr_, false, "structPtr_ is nullptr");
        bufferSizeHint = structPtr_->resultBufferSizeHint;
        return true;
    }
    return false;
}

bool PluginModule::GetStandaloneFileData()
{
    if (handle_ != nullptr) {
        CHECK_NOTNULL(structPtr_, false, "structPtr_ is nullptr");
        return structPtr_->isStandaloneFileData;
    }
    return false;
}

bool PluginModule::GetOutFileName(std::string& outFileName)
{
    if (handle_ != nullptr) {
        CHECK_NOTNULL(structPtr_, false, "structPtr_ is nullptr");
        outFileName.assign(structPtr_->outFileName);
        return true;
    }
    return false;
}

bool PluginModule::GetPluginVersion(std::string& pluginVersion)
{
    if (handle_ != nullptr) {
        if (structPtr_ == nullptr) {
            return false;
        }
        pluginVersion.assign(structPtr_->version);
        return true;
    }
    return false;
}

std::string PluginModule::GetPath()
{
    return path_;
}

std::string PluginModule::GetPluginName()
{
    if (pluginName_ == "") {
        if (handle_ != nullptr) {
            CHECK_NOTNULL(structPtr_, "", "structPtr_ is nullptr");
            pluginName_.assign(structPtr_->name);
        }
    }
    return pluginName_;
}

bool PluginModule::IsLoaded()
{
    return (handle_ != nullptr);
}

bool PluginModule::IsRunning()
{
    return running_;
}

bool PluginModule::BindFunctions()
{
    CHECK_NOTNULL(handle_, false, "%s:plugin not load", __func__);
    if (structPtr_ == nullptr) {
        structPtr_ = static_cast<PluginModuleStruct*>(dlsym(handle_, "g_pluginModule"));
        if (structPtr_ == nullptr) {
            HILOG_DEBUG(LOG_CORE, "%s:structPtr_ == nullptr", __func__);
            return false;
        }
    }

    if (structPtr_->callbacks == nullptr) {
        HILOG_DEBUG(LOG_CORE, "%s:structPtr_->callbacks == nullptr", __func__);
        return false;
    }

    if ((structPtr_->callbacks->onPluginSessionStart == nullptr) ||
        (structPtr_->callbacks->onPluginSessionStop == nullptr)) {
        HILOG_DEBUG(LOG_CORE, "%s:onPluginSessionStart == nullptr", __func__);
        return false;
    }

    return true;
}

bool PluginModule::StartSession(const uint8_t* buffer, uint32_t size)
{
    HILOG_DEBUG(LOG_CORE, "StartSession");
    CHECK_NOTNULL(handle_, false, "%s:plugin not load", __func__);
    if (structPtr_ != nullptr && structPtr_->callbacks != nullptr) {
        if (structPtr_->callbacks->onPluginSessionStart) {
            running_ = true;
            return (structPtr_->callbacks->onPluginSessionStart(buffer, size) == 0);
        }
    }
    return false;
}

bool PluginModule::StopSession()
{
    HILOG_INFO(LOG_CORE, "%s:stop Session ready!", __func__);
    CHECK_NOTNULL(handle_, false, "%s:plugin not load", __func__);
    if (structPtr_ != nullptr && structPtr_->callbacks != nullptr) {
        if (structPtr_->callbacks->onPluginSessionStop != nullptr) {
            running_ = false;
            return (structPtr_->callbacks->onPluginSessionStop() == 0);
        }
    }
    return false;
}

bool PluginModule::ReportBasicData()
{
    HILOG_INFO(LOG_CORE, "%s:report basic data ready!", __func__);
    CHECK_NOTNULL(handle_, false, "%s:plugin not load", __func__);
    if (structPtr_ != nullptr && structPtr_->callbacks != nullptr) {
        if (structPtr_->callbacks->onReportBasicDataCallback != nullptr) {
            auto write = GetWriter();
            CHECK_NOTNULL(write, false, "%s:get write falied!", __func__);
            write->Clear(); // clear share memory block
            return (structPtr_->callbacks->onReportBasicDataCallback() == 0);
        }
    }
    return false;
}

int32_t PluginModule::ReportResult(uint8_t* buffer, uint32_t size)
{
    CHECK_NOTNULL(handle_, -1, "%s:plugin not load", __func__);
    if (first_) {
        lastTime_ = std::chrono::steady_clock::now();
        first_ = false;
    } else {
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
        lastTime_ = t1;
    }

    if (structPtr_ != nullptr && structPtr_->callbacks != nullptr) {
        if (structPtr_->callbacks->onPluginReportResult != nullptr) {
            return structPtr_->callbacks->onPluginReportResult(buffer, size);
        }
    }

    return -1;
}

bool PluginModule::RegisterWriter(const BufferWriterPtr writer)
{
    writerAdapter_ = std::make_shared<WriterAdapter>();
    writerAdapter_->SetWriter(writer);

    if (writer == nullptr) {
        HILOG_INFO(LOG_CORE, "%s:bufferWriter is null, update WriterAdapter only!", __func__);
        return true;
    }
    if (structPtr_ != nullptr && structPtr_->callbacks != nullptr) {
        if (structPtr_->callbacks->onRegisterWriterStruct != nullptr) {
            return structPtr_->callbacks->onRegisterWriterStruct(writerAdapter_->GetStruct());
        }
    }
    return true;
}

WriterPtr PluginModule::GetWriter()
{
    CHECK_NOTNULL(writerAdapter_, nullptr, "%s:pluginModule nullptr", __func__);
    return writerAdapter_->GetWriter();
}
