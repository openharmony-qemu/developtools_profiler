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
#include "hiperf_plugin_config_standard.pb.h"
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
 * @tc.desc: check default values of string_compressed, callframe_compress, record_accurately
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestNativeHookConfigDefaultValues, TestSize.Level0)
{
    ParsePluginConfig parseConfig;
    ProfilerPluginConfig pluginConfig;

    // When not specified, string_compressed/callframe_compress/record_accurately should default to true
    std::ostringstream oss{};
    oss << "pid : 1\n"
        << "filter_size: 4096\n"
        << "process_name: \"render_service\"\n";
    parseConfig.pluginConfigMap["nativehook"] = oss.str();
    ASSERT_TRUE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));

    ForStandard::NativeHookConfig hookConfig;
    ASSERT_TRUE(hookConfig.ParseFromString(pluginConfig.config_data()));
    EXPECT_TRUE(hookConfig.string_compressed());
    EXPECT_TRUE(hookConfig.callframe_compress());
    EXPECT_TRUE(hookConfig.record_accurately());

    // When explicitly set to false, the values should be overridden to false
    std::ostringstream oss2{};
    oss2 << "pid : 1\n"
         << "filter_size: 4096\n"
         << "process_name: \"render_service\"\n"
         << "string_compressed: false\n"
         << "callframe_compress: false\n"
         << "record_accurately: false\n";
    parseConfig.pluginConfigMap["nativehook"] = oss2.str();
    pluginConfig.Clear();
    ASSERT_TRUE(parseConfig.SetSerializeHookConfig("nativehook", pluginConfig));

    ForStandard::NativeHookConfig hookConfig2;
    ASSERT_TRUE(hookConfig2.ParseFromString(pluginConfig.config_data()));
    EXPECT_FALSE(hookConfig2.string_compressed());
    EXPECT_FALSE(hookConfig2.callframe_compress());
    EXPECT_FALSE(hookConfig2.record_accurately());
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

/**
 * @tc.name: TestConvertHiperfSandboxPaths
 * @tc.desc: hiperf plugin config converts sandbox and ~/ outfile to physical Docs path
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiperfSandboxPaths, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiperfPluginConfig hiperfConfig;
    hiperfConfig.set_outfile_name("/storage/Users/currentUser/perf.data");
    hiperfConfig.set_split_outfile_name("~/split.htrace");
    hiperfConfig.set_record_args("-a -d 1");
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiperfSandboxPaths(hiperfConfig, sandboxCallingUid);
    EXPECT_EQ(hiperfConfig.outfile_name(), "/storage/media/100/local/files/Docs/perf.data");
    EXPECT_EQ(hiperfConfig.split_outfile_name(), "/storage/media/100/local/files/Docs/split.htrace");
    EXPECT_EQ(hiperfConfig.record_args(), "-a -d 1");
}

/**
 * @tc.name: TestSerializeHiperfNormalOutfile
 * @tc.desc: hiperf plugin config keeps /data/local/tmp outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestSerializeHiperfNormalOutfile, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    const std::string normalOutfile = "/data/local/tmp/perf.data";
    parseConfig.pluginConfigMap["hiperf-plugin"] =
        "outfile_name: \"" + normalOutfile + "\"\n"
        "record_args: \"-a -d 1\"\n";
    ProfilerPluginConfig pluginConfig;
    ASSERT_TRUE(parseConfig.SetSerializeHiperfConfig("hiperf-plugin", pluginConfig));

    ForStandard::HiperfPluginConfig hiperfConfig;
    ASSERT_TRUE(hiperfConfig.ParseFromString(pluginConfig.config_data()));
    EXPECT_EQ(hiperfConfig.outfile_name(), normalOutfile);
}

/**
 * @tc.name: TestConvertHiperfTmpOutfileForSandboxUid
 * @tc.desc: sandbox uid keeps hiperf /data/local/tmp outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiperfTmpOutfileForSandboxUid, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiperfPluginConfig hiperfConfig;
    const std::string tmpOutfile = "/data/local/tmp/perf.data";
    hiperfConfig.set_outfile_name(tmpOutfile);
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiperfSandboxPaths(hiperfConfig, sandboxCallingUid);
    EXPECT_EQ(hiperfConfig.outfile_name(), tmpOutfile);
}

/**
 * @tc.name: TestConvertHiperfEmptyOutfileForSandboxUid
 * @tc.desc: empty hiperf outfile stays empty for a sandbox uid
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiperfEmptyOutfileForSandboxUid, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiperfPluginConfig hiperfConfig;
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiperfSandboxPaths(hiperfConfig, sandboxCallingUid);
    EXPECT_TRUE(hiperfConfig.outfile_name().empty());
}

/**
 * @tc.name: TestConvertHiperfTmpOutfileForNonSandboxUid
 * @tc.desc: non-sandbox uid keeps hiperf /data/local/tmp outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiperfTmpOutfileForNonSandboxUid, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiperfPluginConfig hiperfConfig;
    const std::string tmpOutfile = "/data/local/tmp/perf.data";
    hiperfConfig.set_outfile_name(tmpOutfile);
    constexpr int32_t nonSandboxCallingUid = 20000000;
    parseConfig.ConvertHiperfSandboxPaths(hiperfConfig, nonSandboxCallingUid);
    EXPECT_EQ(hiperfConfig.outfile_name(), tmpOutfile);
}

/**
 * @tc.name: TestConvertHiperfTmpSplitOutfileForSandboxUid
 * @tc.desc: sandbox uid keeps hiperf tmp split outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiperfTmpSplitOutfileForSandboxUid, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiperfPluginConfig hiperfConfig;
    const std::string tmpOutfile = "/data/local/tmp/perf.data";
    const std::string tmpSplitOutfile = "/data/local/tmp/split.htrace";
    hiperfConfig.set_outfile_name(tmpOutfile);
    hiperfConfig.set_split_outfile_name(tmpSplitOutfile);
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiperfSandboxPaths(hiperfConfig, sandboxCallingUid);
    EXPECT_EQ(hiperfConfig.outfile_name(), tmpOutfile);
    EXPECT_EQ(hiperfConfig.split_outfile_name(), tmpSplitOutfile);
}

/**
 * @tc.name: TestConvertHiebpfSandboxPaths
 * @tc.desc: hiebpf plugin config converts sandbox and ~/ outfile to physical Docs path
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiebpfSandboxPaths, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiebpfConfig hiebpfConfig;
    hiebpfConfig.set_outfile_name("/storage/Users/currentUser/hiebpf.data");
    hiebpfConfig.set_split_outfile_name("~/split.htrace");
    hiebpfConfig.set_cmd_line("hiebpf --start true");
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiebpfSandboxPaths(hiebpfConfig, sandboxCallingUid);
    EXPECT_EQ(hiebpfConfig.outfile_name(), "/storage/media/100/local/files/Docs/hiebpf.data");
    EXPECT_EQ(hiebpfConfig.split_outfile_name(), "/storage/media/100/local/files/Docs/split.htrace");
    EXPECT_EQ(hiebpfConfig.cmd_line(), "hiebpf --start true");
}

/**
 * @tc.name: TestConvertHiebpfTmpOutfileUnchanged
 * @tc.desc: sandbox uid keeps hiebpf /data/local/tmp outfile and empty outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiebpfTmpOutfileUnchanged, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiebpfConfig hiebpfConfig;
    const std::string tmpOutfile = "/data/local/tmp/hiebpf.data";
    hiebpfConfig.set_outfile_name(tmpOutfile);
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiebpfSandboxPaths(hiebpfConfig, sandboxCallingUid);
    EXPECT_EQ(hiebpfConfig.outfile_name(), tmpOutfile);

    ForStandard::HiebpfConfig emptyConfig;
    parseConfig.ConvertHiebpfSandboxPaths(emptyConfig, sandboxCallingUid);
    EXPECT_TRUE(emptyConfig.outfile_name().empty());
}

/**
 * @tc.name: TestConvertHiperfSandboxPathsForNonSandboxUid
 * @tc.desc: non-sandbox uid keeps hiperf sandbox and ~/ outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiperfSandboxPathsForNonSandboxUid, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiperfPluginConfig hiperfConfig;
    const std::string sandboxOutfile = "/storage/Users/currentUser/perf.data";
    const std::string homeSplitOutfile = "~/split.htrace";
    hiperfConfig.set_outfile_name(sandboxOutfile);
    hiperfConfig.set_split_outfile_name(homeSplitOutfile);
    constexpr int32_t nonSandboxCallingUid = 20000000;
    parseConfig.ConvertHiperfSandboxPaths(hiperfConfig, nonSandboxCallingUid);
    EXPECT_EQ(hiperfConfig.outfile_name(), sandboxOutfile);
    EXPECT_EQ(hiperfConfig.split_outfile_name(), homeSplitOutfile);
}

/**
 * @tc.name: TestConvertHiperfSplitOutfileOnly
 * @tc.desc: hiperf converts only the sandbox split outfile and keeps a tmp outfile
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiperfSplitOutfileOnly, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiperfPluginConfig hiperfConfig;
    const std::string tmpOutfile = "/data/local/tmp/perf.data";
    hiperfConfig.set_outfile_name(tmpOutfile);
    hiperfConfig.set_split_outfile_name("~/split.htrace");
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiperfSandboxPaths(hiperfConfig, sandboxCallingUid);
    EXPECT_EQ(hiperfConfig.outfile_name(), tmpOutfile);
    EXPECT_EQ(hiperfConfig.split_outfile_name(), "/storage/media/100/local/files/Docs/split.htrace");
}

/**
 * @tc.name: TestSerializeHiperfSandboxOutfileWithoutMacro
 * @tc.desc: serialize keeps sandbox outfile when is_sandbox is not enabled
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestSerializeHiperfSandboxOutfileWithoutMacro, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    const std::string sandboxOutfile = "/storage/Users/currentUser/perf.data";
    parseConfig.pluginConfigMap["hiperf-plugin"] = "outfile_name: \"" + sandboxOutfile + "\"\n";
    ProfilerPluginConfig pluginConfig;
    ASSERT_TRUE(parseConfig.SetSerializeHiperfConfig("hiperf-plugin", pluginConfig));

    ForStandard::HiperfPluginConfig hiperfConfig;
    ASSERT_TRUE(hiperfConfig.ParseFromString(pluginConfig.config_data()));
    EXPECT_EQ(hiperfConfig.outfile_name(), sandboxOutfile);
}

/**
 * @tc.name: TestSerializeHiebpfNormalOutfile
 * @tc.desc: hiebpf plugin config keeps /data/local/tmp outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestSerializeHiebpfNormalOutfile, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    const std::string normalOutfile = "/data/local/tmp/hiebpf.data";
    parseConfig.pluginConfigMap["hiebpf-plugin"] = "outfile_name: \"" + normalOutfile + "\"\n";
    ProfilerPluginConfig pluginConfig;
    ASSERT_TRUE(parseConfig.SetSerializeHiebpfConfig("hiebpf-plugin", pluginConfig));

    ForStandard::HiebpfConfig hiebpfConfig;
    ASSERT_TRUE(hiebpfConfig.ParseFromString(pluginConfig.config_data()));
    EXPECT_EQ(hiebpfConfig.outfile_name(), normalOutfile);
}

/**
 * @tc.name: TestConvertHiebpfSandboxPathsForNonSandboxUid
 * @tc.desc: non-sandbox uid keeps hiebpf sandbox and ~/ outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiebpfSandboxPathsForNonSandboxUid, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiebpfConfig hiebpfConfig;
    const std::string sandboxOutfile = "/storage/Users/currentUser/hiebpf.data";
    const std::string homeSplitOutfile = "~/split.htrace";
    hiebpfConfig.set_outfile_name(sandboxOutfile);
    hiebpfConfig.set_split_outfile_name(homeSplitOutfile);
    constexpr int32_t nonSandboxCallingUid = 20000000;
    parseConfig.ConvertHiebpfSandboxPaths(hiebpfConfig, nonSandboxCallingUid);
    EXPECT_EQ(hiebpfConfig.outfile_name(), sandboxOutfile);
    EXPECT_EQ(hiebpfConfig.split_outfile_name(), homeSplitOutfile);
}

/**
 * @tc.name: TestConvertHiebpfTmpSplitOutfileUnchanged
 * @tc.desc: sandbox uid keeps hiebpf tmp outfile and tmp split outfile unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiebpfTmpSplitOutfileUnchanged, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiebpfConfig hiebpfConfig;
    const std::string tmpOutfile = "/data/local/tmp/hiebpf.data";
    const std::string tmpSplitOutfile = "/data/local/tmp/split.htrace";
    hiebpfConfig.set_outfile_name(tmpOutfile);
    hiebpfConfig.set_split_outfile_name(tmpSplitOutfile);
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiebpfSandboxPaths(hiebpfConfig, sandboxCallingUid);
    EXPECT_EQ(hiebpfConfig.outfile_name(), tmpOutfile);
    EXPECT_EQ(hiebpfConfig.split_outfile_name(), tmpSplitOutfile);
}

/**
 * @tc.name: TestConvertHiperfEmptySplitStaysEmpty
 * @tc.desc: hiperf converts a sandbox outfile and leaves an empty split outfile empty
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiperfEmptySplitStaysEmpty, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiperfPluginConfig hiperfConfig;
    hiperfConfig.set_outfile_name("/storage/Users/currentUser/perf.data");
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiperfSandboxPaths(hiperfConfig, sandboxCallingUid);
    EXPECT_EQ(hiperfConfig.outfile_name(), "/storage/media/100/local/files/Docs/perf.data");
    EXPECT_TRUE(hiperfConfig.split_outfile_name().empty());
}

/**
 * @tc.name: TestConvertHiebpfSplitOutfileOnly
 * @tc.desc: hiebpf converts only the sandbox split outfile and keeps a tmp outfile
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestConvertHiebpfSplitOutfileOnly, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    ForStandard::HiebpfConfig hiebpfConfig;
    const std::string tmpOutfile = "/data/local/tmp/hiebpf.data";
    hiebpfConfig.set_outfile_name(tmpOutfile);
    hiebpfConfig.set_split_outfile_name("~/split.htrace");
    constexpr int32_t sandboxCallingUid = 20000001;
    parseConfig.ConvertHiebpfSandboxPaths(hiebpfConfig, sandboxCallingUid);
    EXPECT_EQ(hiebpfConfig.outfile_name(), tmpOutfile);
    EXPECT_EQ(hiebpfConfig.split_outfile_name(), "/storage/media/100/local/files/Docs/split.htrace");
}

/**
 * @tc.name: TestSerializeHiebpfSandboxOutfileWithoutMacro
 * @tc.desc: serialize keeps hiebpf sandbox outfile when is_sandbox is not enabled
 * @tc.type: FUNC
 */
HWTEST_F(ParsePluginConfigTest, TestSerializeHiebpfSandboxOutfileWithoutMacro, TestSize.Level1)
{
    ParsePluginConfig parseConfig;
    const std::string sandboxOutfile = "/storage/Users/currentUser/hiebpf.data";
    parseConfig.pluginConfigMap["hiebpf-plugin"] = "outfile_name: \"" + sandboxOutfile + "\"\n";
    ProfilerPluginConfig pluginConfig;
    ASSERT_TRUE(parseConfig.SetSerializeHiebpfConfig("hiebpf-plugin", pluginConfig));

    ForStandard::HiebpfConfig hiebpfConfig;
    ASSERT_TRUE(hiebpfConfig.ParseFromString(pluginConfig.config_data()));
    EXPECT_EQ(hiebpfConfig.outfile_name(), sandboxOutfile);
}
}
