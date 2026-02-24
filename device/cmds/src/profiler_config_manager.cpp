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

#include "profiler_config_manager.h"
#include "file_path_handler.h"
#include "parse_plugin_config.h"
#include "common.h"
#include <iostream>
#include <istream>

ProfilerConfigManager& ProfilerConfigManager::GetInstance()
{
    static ProfilerConfigManager instance;
    return instance;
}

bool ProfilerConfigManager::ParseConfig(const std::string& configFile, std::string& config)
{
    auto handler = FilePathHandlerFactory::CreateHandler();
    std::string configFileWithPath = handler->GetConfigFilePath(configFile);
    
    printf("Read config from %s\n", configFileWithPath.c_str());
    std::vector<std::string> validPaths = handler->GetValidPaths();
    
    if (!COMMON::ReadFile(configFileWithPath, validPaths, config)) {
        printf("Read %s fail, please place it under %s.\n", configFile.c_str(), validPaths[0].c_str());
        return false;
    }
    
    config = ParsePluginConfig::GetInstance().GetPluginsConfig(config);
    if (config.empty()) {
        printf("Error config file: %s\n", configFileWithPath.c_str());
        return false;
    }
    return true;
}

bool ProfilerConfigManager::ReadConfigFromStdin(std::string& config)
{
    std::string content;
    std::istreambuf_iterator<char> begin(std::cin);
    std::istreambuf_iterator<char> end = {};
    content.assign(begin, end);
    
    config = ParsePluginConfig::GetInstance().GetPluginsConfig(content);
    if (config.empty()) {
        printf("Please check the configuration!\n");
        return false;
    }
    return true;
}
