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
#include <gtest/gtest.h>
#include "google/protobuf/text_format.h"
#include "native_hook_config_standard.pb.h"
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

    std::string CreateNativeHookConfig(int pid, const std::string processName, std::string filterSize,
                                      std::string dumpNmdStr, std::string statisticsInterval = "0") const
    {
        std::ostringstream oss{};
        oss << "pid : " << pid << "\n"
            << "filter_size: " << filterSize << "\n"
            << "save_file: false\n"
            << "smb_pages: 12288\n"
            << "max_stack_depth: 8\n"
            << "process_name: \"" << processName << "\"  \n"
            << "string_compressed: true\n"
            << "fp_unwind: false\n"
            << "blocked: true\n"
            << "callframe_compress: true\n"
            << "record_accurately: true\n"
            << "offline_symbolization: true\n"
            << "startup_mode: false\n"
            << "dump_nmd : " << dumpNmdStr << "\n"
            << "statistics_interval: " << statisticsInterval << "\n";
        return oss.str();
    }

    std::string CreateEnumConfig(std::vector<int32_t> vectPid, std::vector<std::string> tagName)
    {
        std::ostringstream oss;
        for (size_t i = 0; i < vectPid.size(); i++) {
            oss << "expand_pids : " << vectPid[i] << "\n";
        }
        for (size_t i = 0; i < tagName.size(); i++) {
            oss << "restrace_tag : \"" << tagName[i] << "\"\n";
        }
        return oss.str();
    }

    std::string CreateOtherKeyConfig(std::string keyName, std::string keyValue)
    {
        std::ostringstream oss;
        oss << "process_name: \"render_service\" \n" << keyName << ": " << keyValue << "\n";
        return oss.str();
    }
};

/**
 * @tc.name: ParsePluginConfig
 * @tc.desc: Test parse plugin config.
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestParsePluginConfig, TestSize.Level0)
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

/**
 * @tc.name: ParsePluginConfig
 * @tc.desc: check nativehook plugin config parameters
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestNativeHookConfigParameters, TestSize.Level0)
{
    std::string hookConfig = CreateNativeHookConfig(1, "render_service", "4096", "false");
    auto hookConfigNolite = std::make_unique<ForStandard::NativeHookConfig>();
    google::protobuf::TextFormat::Parser hookParser;
    hookParser.AllowUnknownField(true);
    ASSERT_TRUE(hookParser.ParseFromString(hookConfig, hookConfigNolite.get()));
    EXPECT_EQ(hookConfigNolite->process_name(), "render_service");
    EXPECT_EQ(hookConfigNolite->filter_size(), 4096);
    EXPECT_EQ(hookConfigNolite->dump_nmd(), false);
    // check bool type
    hookConfig = CreateNativeHookConfig(123, "render_service", "4096", "Fd#de");
    EXPECT_FALSE(hookParser.ParseFromString(hookConfig, hookConfigNolite.get()));
    // check int32 type
    hookConfig = CreateNativeHookConfig(true, "render_service", "DF4096#", "false");
    EXPECT_FALSE(hookParser.ParseFromString(hookConfig, hookConfigNolite.get()));
    // check other key
    hookConfig = CreateOtherKeyConfig("other_key", "123");
    // ignore other key
    EXPECT_TRUE(hookParser.ParseFromString(hookConfig, hookConfigNolite.get()));

    // check array type
    hookConfig = CreateEnumConfig({1, 2, 3}, {"tag1", "tag2", "tag3"});
    ASSERT_TRUE(hookParser.ParseFromString(hookConfig, hookConfigNolite.get()));
    EXPECT_EQ(hookConfigNolite->expand_pids_size(), 3);
    EXPECT_EQ(hookConfigNolite->restrace_tag_size(), 3);
    EXPECT_EQ(hookConfigNolite->expand_pids(0), 1);
    EXPECT_EQ(hookConfigNolite->expand_pids(1), 2);
    EXPECT_EQ(hookConfigNolite->expand_pids(2), 3);
    EXPECT_EQ(hookConfigNolite->restrace_tag(0), "tag1");
    EXPECT_EQ(hookConfigNolite->restrace_tag(1), "tag2");
    EXPECT_EQ(hookConfigNolite->restrace_tag(2), "tag3");
}

/**
 * @tc.name: ParsePluginConfig
 * @tc.desc: check nativehook plugin config
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestNativeHookConfig, TestSize.Level0)
{
    std::string hookConfig = CreateNativeHookConfig(1, "render_service", "4096", "false");
    ParsePluginConfig parseConfig;
    parseConfig.pluginConfigMap["nativehook"] = hookConfig;
    ProfilerPluginConfig pluginConfig;
    EXPECT_TRUE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));
    // check int32
    uint64_t filterSize = std::numeric_limits<int32_t>::max();
    hookConfig = CreateNativeHookConfig(1, "render_service", std::to_string(filterSize + 1), "true");
    parseConfig.pluginConfigMap["nativehook"] = hookConfig;
    EXPECT_FALSE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));

    hookConfig = CreateNativeHookConfig(1, "render_service", "-3", "true");
    parseConfig.pluginConfigMap["nativehook"] = hookConfig;
    EXPECT_TRUE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));

    hookConfig = CreateNativeHookConfig(1, "render_service", "ui78hook$#", "true");
    parseConfig.pluginConfigMap["nativehook"] = hookConfig;
    EXPECT_FALSE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));

    // check bool
    hookConfig = CreateNativeHookConfig(1, "render_service", "4096", "nativehook");
    parseConfig.pluginConfigMap["nativehook"] = hookConfig;
    EXPECT_FALSE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));
    // check uint32
    uint64_t tempStatic = std::numeric_limits<uint32_t>::max();
    hookConfig = CreateNativeHookConfig(1, "render_service", "4096", "false", std::to_string(tempStatic + 1));
    parseConfig.pluginConfigMap["nativehook"] = hookConfig;
    EXPECT_FALSE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));

    // check uint64 positive number
    hookConfig = CreateNativeHookConfig(1, "render_service", "4096", "false", "-1");
    parseConfig.pluginConfigMap["nativehook"] = hookConfig;
    EXPECT_FALSE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));
}

/**
 * @tc.name: ParsePluginConfig
 * @tc.desc: check restrace tag
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestNativeHookRestraceTag, TestSize.Level0)
{
    std::ostringstream oss{};
    oss << "pid : 123"
        << "\n"
        << "malloc_disable:true"
        << "\n"
        << "memtrace_enable:true"
        << "\n"
        << "restrace_tag: \"RES_DMABUF_MASK_XX\""
        << "\n";
    auto hookConfigNolite = std::make_unique<ForStandard::NativeHookConfig>();
    google::protobuf::TextFormat::Parser hookParser;
    hookParser.AllowUnknownField(true);
    ASSERT_TRUE(hookParser.ParseFromString(oss.str(), hookConfigNolite.get()));
    EXPECT_EQ(hookConfigNolite->restrace_tag(0), "RES_DMABUF_MASK_XX");
    ParsePluginConfig parseConfig;
    ProfilerPluginConfig pluginConfig;
    parseConfig.pluginConfigMap["nativehook"] = oss.str();
    EXPECT_FALSE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));
}
}
