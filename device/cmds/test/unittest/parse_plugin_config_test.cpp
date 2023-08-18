/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#include <iostream>
#include <vector>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "google/protobuf/text_format.h"
#include "parse_plugin_config.h"
#include "logging.h"
using namespace testing::ext;

namespace {
class ParsePluginConfigTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void CreateCommand(string ConfigName, ParsePluginConfig &parseConfig, string &config) const
    {
        std::string cmdStr = " request_id: 1"
            " session_config {"
            "  buffers {"
            "   pages: 16384"
            "  }"
            "  result_file: \"/data/local/tmp/hiprofiler_data.htrace\""
            "  sample_duration: 50000"
            " }"
            " plugin_configs {"
            "  plugin_name: \"" + ConfigName + "\""
            "  sample_interval: 1000"
            "  config_data {"
            "  }"
            " }";
        config = parseConfig.GetPluginsConfig(cmdStr);
    }

    ProfilerPluginConfig GetProfilerPluginConfig(string config)
    {
        ProfilerPluginConfig profilerPluginConfig;
        auto request = std::make_unique<CreateSessionRequest>();
        if (!google::protobuf::TextFormat::ParseFromString(config, request.get())) {
            printf("%s\n", config.c_str());
            return profilerPluginConfig;
        }
        profilerPluginConfig = *(request->mutable_plugin_configs(0));
        return profilerPluginConfig;
    }
};

/**
 * @tc.name: ParsePluginConfig
 * @tc.desc: Test parse plugin config.
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestParsePluginConfig, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    std::string config;
    vector<std::string> pluginNames{
        "cpu-plugin",
        "diskio-plugin",
        "ftrace-plugin",
        "hidump-plugin",
        "hilog-plugin",
        "memory-plugin",
        "nativehook",
        "network-plugin",
        "process-plugin",
        "hiperf-plugin",
        "hisysevent-plugin",
        "hiebpf-plugin",
        "arkts-plugin",
        "invalid-plugin",
    };
    for (const std::string &pluginName : pluginNames) {
        CreateCommand(pluginName, parseConfig, config);
        auto profilerConfig = GetProfilerPluginConfig(config);
        bool res = parseConfig.SetSerializePluginsConfig(pluginName, profilerConfig);
        if (pluginName == "invalid-plugin") {
            EXPECT_FALSE(res);
        } else {
            EXPECT_TRUE(res);
        }
    }
    ProfilerPluginConfig profilerConfig;
    bool res = parseConfig.SetSerializePluginsConfig("testplugin", profilerConfig);
    EXPECT_TRUE(!res);
}
}
