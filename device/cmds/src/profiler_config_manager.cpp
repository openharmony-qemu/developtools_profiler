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
#include <cstdio>
#include <iostream>
#include <istream>
#include <iterator>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <cstring>
#include "ipc_skeleton.h"
#include "common.h"
#include "parse_plugin_config.h"

namespace {
constexpr int USER_ID_MOD = 200000;
constexpr int APP_ID_THRESH = 20000000;
const std::string DEFAULT_OUTPUT_FILE = "/data/local/tmp/hiprofiler_data.htrace";
const std::string DEFAULT_OUTPUT_ROOT = "/data/local/tmp/";
const std::string SANDBOX_PATH_ROOT("/storage/Users/currentUser/");
}

ProfilerConfigManager& ProfilerConfigManager::GetInstance()
{
    static ProfilerConfigManager instance;
    return instance;
}

bool ProfilerConfigManager::ParseConfig(const std::string& configFile, std::string& config)
{
    std::string configFileWithPath = configFile;
    bool adaptSandbox = false;
#if defined(is_sandbox) && is_sandbox
    int32_t callingUid = OHOS::IPCSkeleton::GetCallingUid();
    if (callingUid > APP_ID_THRESH) {
        adaptSandbox = true;
    }
#endif
    
    if (configFile.find('/') == std::string::npos) {
        if (adaptSandbox) {
            configFileWithPath = SANDBOX_PATH_ROOT + configFile;
        } else {
            std::string defaultPath("/data/local/tmp/");
            configFileWithPath = defaultPath + configFile;
        }
    }

    printf("Read config from %s\n", configFileWithPath.c_str());
    std::vector<std::string> validPaths {};
    if (adaptSandbox) {
        validPaths.push_back(SANDBOX_PATH_ROOT);
    } else {
        validPaths.push_back("/data/local/tmp/");
    }
    
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

