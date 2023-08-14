/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#ifndef PLUGIN_MODULE_H
#define PLUGIN_MODULE_H

#include <chrono>
#include <memory>
#include <string>

#include "buffer_writer.h"
#include "writer_adapter.h"

struct PluginModuleInfo {
    std::string name;
    uint32_t bufferSizeHint = 0;
    bool isStandaloneFileData = false;
    std::string outFileName = "";
    std::string pluginVersion = "";
};

struct PluginModuleStruct;
struct WriterStruct;

using BufferWriterPtr = STD_PTR(shared, BufferWriter);

class PluginModule {
public:
    enum class SampleMode {
        UNKNOWN,
        POLLING,
        STREAMING,
    };
    explicit PluginModule(const std::string& path = "");
    ~PluginModule();
    std::string ComputeSha256();

    bool Load();
    bool Unload();

    SampleMode GetSampleMode() const;
    bool GetInfo(PluginModuleInfo& info);
    bool GetPluginName(std::string& pluginName);
    bool GetBufferSizeHint(uint32_t& bufferSizeHint);
    bool GetStandaloneFileData();
    bool GetOutFileName(std::string& outFileName);
    bool GetPluginVersion(std::string& pluginVersion);
    std::string GetPath();
    std::string GetPluginName();
    bool IsRunning();
    bool IsLoaded();
    bool BindFunctions();

    bool StartSession(const uint8_t* buffer, uint32_t size);
    bool StopSession();
    int32_t ReportResult(uint8_t* buffer, uint32_t size);
    bool ReportBasicData();

    bool RegisterWriter(const BufferWriterPtr writer);
    WriterPtr GetWriter();

    void SetConfigData(const std::string& data);
    std::string GetConfigData() const;

    void SetClockId(clockid_t clockId);
    clockid_t GetClockId() const;

private:
    void* handle_;
    bool running_;
    std::string path_;
    std::string pluginName_;
    std::string configData_;
    clockid_t clockId_ = CLOCK_REALTIME;
    PluginModuleStruct* structPtr_;
    std::shared_ptr<WriterAdapter> writerAdapter_;
    std::chrono::steady_clock::time_point lastTime_;
    bool first_ = true;
};

#endif // !PLUGIN_MODULE_H
