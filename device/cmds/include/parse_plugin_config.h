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

#ifndef PARSE_PLUGIN_CONFIG_H
#define PARSE_PLUGIN_CONFIG_H

#include <map>
#include <string>

#include "google/protobuf/text_format.h"
#include "profiler_service.grpc.pb.h"

class ParsePluginConfig {
public:
    static ParsePluginConfig& GetInstance();

    std::string GetPluginsConfig(std::string& content);
    std::string GetPluginsPath(std::string& name);
    bool SetSerializePluginsConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);

    bool SetSerializeCpuConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeDiskioConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeFtraceConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeHidumpConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeHiebpfConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeHilogConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeMemoryConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeHookConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeNetworkConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeProcessConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeHiperfConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeHisyseventConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);
    bool SetSerializeArkTSConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig);

    google::protobuf::TextFormat::Parser& GetParser()
    {
        return parser_;
    }

private:
    ParsePluginConfig();
    ~ParsePluginConfig() {}

    std::map<std::string, std::string> pluginConfigMap;
    google::protobuf::TextFormat::Parser parser_;
};


#endif