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

#include "parse_plugin_config.h"

#include "cpu_plugin_config_standard.pb.h"
#include "diskio_plugin_config_standard.pb.h"
#include "hidump_plugin_config_standard.pb.h"
#include "hiebpf_plugin_config_standard.pb.h"
#include "hilog_plugin_config_standard.pb.h"
#include "hiperf_plugin_config_standard.pb.h"
#include "hisysevent_plugin_config_standard.pb.h"
#include "arkts_plugin_config_standard.pb.h"
#include "memory_plugin_common_standard.pb.h"
#include "memory_plugin_config_standard.pb.h"
#include "native_hook_config_standard.pb.h"
#include "network_plugin_config_standard.pb.h"
#include "process_plugin_config_standard.pb.h"
#include "trace_plugin_config_standard.pb.h"

namespace {
constexpr int REMAINDER = 2;
}

ParsePluginConfig::ParsePluginConfig()
{
    parser_.AllowUnknownField(true);
}

ParsePluginConfig& ParsePluginConfig::GetInstance()
{
    static ParsePluginConfig parsePluginConfig;
    return parsePluginConfig;
}

std::string ParsePluginConfig::GetPluginsConfig(std::string& content)
{
    std::string pluginConfig = "";
    std::string pluginName = "";
    size_t beginPos = 0;
    size_t endPos = 0;
    for (int i = 0; content.size() > 0; i++) {
        // 先获取pluginName，再获取configData
        std::string destStr = (i % REMAINDER) ? "config_data" : "plugin_name";
        beginPos = content.find(destStr);
        if (beginPos == std::string::npos) {
            break;
        }
        pluginConfig += content.substr(0, beginPos);
        content = content.substr(beginPos + destStr.size(), content.size());
        destStr = (i % REMAINDER) ? "{" : "\"";
        beginPos = content.find(destStr);
        if (beginPos == std::string::npos) {
            break;
        }
        content = content.substr(beginPos + 1, content.size());
        destStr = (i % REMAINDER) ? "}" : "\"";
        endPos = content.find(destStr);
        if (endPos == std::string::npos) {
            break;
        }
        std::string contentStr = content.substr(0, endPos);
        if (i % REMAINDER == 0) { // set plugin-name
            pluginName = contentStr;

            if (pluginName == "") {
                return "";
            }
            pluginConfig += "name: \"" + pluginName + "\"";
        } else { // save config_data
            pluginConfigMap.insert({pluginName, contentStr});
            pluginConfig += "config_data: \"\"";
        }

        content = content.substr(endPos + 1, content.size());
    }

    pluginConfig += content;
    return pluginConfig;
}

bool ParsePluginConfig::SetSerializePluginsConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    bool ret = false;
    if (pluginConfigMap.count(pluginName) == 0) {
        printf("unknown plugin: %s\n", pluginName.c_str());
        return ret;
    }

    // 将pluginConfigMap中保存的configData序列化后写入pluginConfig
    if (pluginName == "cpu-plugin") {
        ret = SetSerializeCpuConfig(pluginName, pluginConfig);
    } else if (pluginName == "diskio-plugin") {
        ret = SetSerializeDiskioConfig(pluginName, pluginConfig);
    } else if (pluginName == "ftrace-plugin") {
        ret = SetSerializeFtraceConfig(pluginName, pluginConfig);
    } else if (pluginName == "hidump-plugin") {
        ret = SetSerializeHidumpConfig(pluginName, pluginConfig);
    } else if (pluginName == "hiebpf-plugin") {
        ret = SetSerializeHiebpfConfig(pluginName, pluginConfig);
    } else if (pluginName == "hilog-plugin") {
        ret = SetSerializeHilogConfig(pluginName, pluginConfig);
    } else if (pluginName == "memory-plugin") {
        ret = SetSerializeMemoryConfig(pluginName, pluginConfig);
    } else if (pluginName == "nativehook") {
        ret = SetSerializeHookConfig(pluginName, pluginConfig);
    } else if (pluginName == "network-plugin") {
        ret = SetSerializeNetworkConfig(pluginName, pluginConfig);
    } else if (pluginName == "process-plugin") {
        ret = SetSerializeProcessConfig(pluginName, pluginConfig);
    } else if (pluginName == "hiperf-plugin") {
        ret = SetSerializeHiperfConfig(pluginName, pluginConfig);
    } else if (pluginName == "hisysevent-plugin") {
        ret = SetSerializeHisyseventConfig(pluginName, pluginConfig);
    } else if (pluginName == "arkts-plugin") {
        ret = SetSerializeArkTSConfig(pluginName, pluginConfig);
    } else {
        printf("unsupport plugin: %s\n", pluginName.c_str());
    }

    return ret;
}

bool ParsePluginConfig::SetSerializeCpuConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto cpuConfigNolite = std::make_unique<ForStandard::CpuConfig>();
    if (!parser_.ParseFromString(configData, cpuConfigNolite.get())) {
        printf("cpu parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(cpuConfigNolite->ByteSizeLong());
    if (cpuConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("cpu serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeDiskioConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto diskioConfigNolite = std::make_unique<ForStandard::DiskioConfig>();
    if (!parser_.ParseFromString(configData, diskioConfigNolite.get())) {
        printf("diskio parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(diskioConfigNolite->ByteSizeLong());
    if (diskioConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("diskio serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeFtraceConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto ftraceConfigNolite = std::make_unique<ForStandard::TracePluginConfig>();
    if (!parser_.ParseFromString(configData, ftraceConfigNolite.get())) {
        printf("ftrace parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configNoLiteDataVec(ftraceConfigNolite->ByteSizeLong());
    if (ftraceConfigNolite->SerializeToArray(configNoLiteDataVec.data(), configNoLiteDataVec.size()) <= 0) {
        printf("ftrace serialize failed!\n");
        return false;
    }

    pluginConfig.set_config_data((const void*)configNoLiteDataVec.data(), configNoLiteDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeHidumpConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto hidumpConfigNolite = std::make_unique<ForStandard::HidumpConfig>();
    if (!parser_.ParseFromString(configData, hidumpConfigNolite.get())) {
        printf("hidump parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(hidumpConfigNolite->ByteSizeLong());
    if (hidumpConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("hidump serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeHiebpfConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    auto iter = pluginConfigMap.find(pluginName);
    if (iter == pluginConfigMap.end()) {
        printf("find %s failed\n", pluginName.c_str());
        return false;
    }
    auto hiebpfConfigNolite = std::make_unique<ForStandard::HiebpfConfig>();
    if (hiebpfConfigNolite == nullptr) {
        printf("hiebpfConfigNolite is nullptr\n");
        return false;
    }
    if (!parser_.ParseFromString(iter->second, hiebpfConfigNolite.get())) {
        printf("hiebpf config parse failed!\n");
        return false;
    }
    std::vector<uint8_t> config(hiebpfConfigNolite->ByteSizeLong());
    if (hiebpfConfigNolite->SerializeToArray(config.data(), config.size()) <= 0) {
        printf("hiebpf serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data(static_cast<const void*>(config.data()), config.size());
    return true;
}

bool ParsePluginConfig::SetSerializeHilogConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto hilogConfigNolite = std::make_unique<ForStandard::HilogConfig>();
    if (!parser_.ParseFromString(configData, hilogConfigNolite.get())) {
        printf("hilog parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(hilogConfigNolite->ByteSizeLong());
    if (hilogConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("hilog serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeMemoryConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto memoryConfigNolite = std::make_unique<ForStandard::MemoryConfig>();
    if (!parser_.ParseFromString(configData, memoryConfigNolite.get())) {
        printf("memory parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(memoryConfigNolite->ByteSizeLong());
    if (memoryConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("memory serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeHookConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto hookConfigNolite = std::make_unique<ForStandard::NativeHookConfig>();
    if (!parser_.ParseFromString(configData, hookConfigNolite.get())) {
        printf("nativedaemon parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(hookConfigNolite->ByteSizeLong());
    if (hookConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("nativedaemon serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeNetworkConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto networkConfigNolite = std::make_unique<ForStandard::NetworkConfig>();
    if (!parser_.ParseFromString(configData, networkConfigNolite.get())) {
        printf("network parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(networkConfigNolite->ByteSizeLong());
    if (networkConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("network serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeProcessConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto processConfigNolite = std::make_unique<ForStandard::ProcessConfig>();
    if (!parser_.ParseFromString(configData, processConfigNolite.get())) {
        printf("process parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(processConfigNolite->ByteSizeLong());
    if (processConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("process serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeHiperfConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto hiperfConfigNolite = std::make_unique<ForStandard::HiperfPluginConfig>();
    if (!parser_.ParseFromString(configData, hiperfConfigNolite.get())) {
        printf("hiperf config parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(hiperfConfigNolite->ByteSizeLong());
    if (hiperfConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("hiperf config failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeHisyseventConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto hisyseventConfigNolite = std::make_unique<ForStandard::HisyseventConfig>();
    if (!parser_.ParseFromString(configData, hisyseventConfigNolite.get())) {
        printf("NODE hisysevent parse failed!\n");
        return false;
    }

    std::vector<uint8_t> configDataVec(hisyseventConfigNolite->ByteSizeLong());
    if (hisyseventConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        printf("NODE hisysevent serialize failed!\n");
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}

bool ParsePluginConfig::SetSerializeArkTSConfig(const std::string& pluginName, ProfilerPluginConfig& pluginConfig)
{
    std::string configData = pluginConfigMap[pluginName];
    auto arkTSConfigNolite = std::make_unique<ForStandard::ArkTSConfig>();
    if (!parser_.ParseFromString(configData, arkTSConfigNolite.get())) {
        return false;
    }

    std::vector<uint8_t> configDataVec(arkTSConfigNolite->ByteSizeLong());
    if (arkTSConfigNolite->SerializeToArray(configDataVec.data(), configDataVec.size()) <= 0) {
        return false;
    }
    pluginConfig.set_config_data((const void*)configDataVec.data(), configDataVec.size());
    return true;
}
