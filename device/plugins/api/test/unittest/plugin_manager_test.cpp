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

#include <google/protobuf/message.h>
#include <grpcpp/health_check_service_interface.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <thread>

#include "command_poller.h"
#include "grpc/impl/codegen/log.h"
#include "logging.h"
#include "plugin_manager.h"
#include "plugin_service.h"
#include "plugin_service.ipc.h"
#include "profiler_service.h"
#include "socket_context.h"

using google::protobuf::Message;
using namespace testing::ext;

namespace {
constexpr int DEFAULT_BUFFER_SIZE = 4096;
constexpr int DEFAULT_SLEEP_TIME = 1000;
const std::string SUCCESS_PLUGIN_NAME = "libmemdataplugin.z.so";
#if defined(__LP64__)
std::string g_testPluginDir("/system/lib64/");
#else
std::string g_testPluginDir("/system/lib/");
#endif
int g_hiprofilerProcessNum = -1;
const std::string DEFAULT_HIPROFILERD_PATH("/system/bin/hiprofilerd");

class PluginManagerTest : public ::testing::Test {
protected:
    static constexpr auto TEMP_DELAY = std::chrono::milliseconds(20);
    static void SetUpTestCase()
    {
        int processNum = fork();
        if (processNum == 0) {
            // start running hiprofilerd
            execl(DEFAULT_HIPROFILERD_PATH.c_str(), nullptr, nullptr);
            _exit(1);
        } else {
            g_hiprofilerProcessNum = processNum;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
    }

    static void TearDownTestCase()
    {
        std::string stopCmd = "kill " + std::to_string(g_hiprofilerProcessNum);
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(stopCmd.c_str(), "r"), pclose);
    }
};

/**
 * @tc.name: plugin
 * @tc.desc: Plug-in normal loading and removal process test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, SuccessPlugin, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManager>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    EXPECT_TRUE(commandPoller->OnConnect());
    pluginManage->SetCommandPoller(commandPoller);

    const uint8_t configData[] = {0x30, 0x01, 0x38, 0x01, 0x42, 0x01, 0x01};
    std::string pluginName = "memory-plugin";
    ProfilerPluginConfig config;
    const std::vector<uint32_t> pluginIdsVector = {2};
    config.set_name(pluginName);
    config.set_config_data((const void*)configData, 7);
    config.set_sample_interval(DEFAULT_SLEEP_TIME);

    EXPECT_FALSE(pluginManage->LoadPlugin(pluginName));
    EXPECT_FALSE(pluginManage->UnloadPlugin(pluginName));
    EXPECT_TRUE(pluginManage->AddPlugin(g_testPluginDir + SUCCESS_PLUGIN_NAME));
    EXPECT_FALSE(pluginManage->AddPlugin(g_testPluginDir + SUCCESS_PLUGIN_NAME));
    EXPECT_TRUE(pluginManage->RemovePlugin(g_testPluginDir + SUCCESS_PLUGIN_NAME));
    EXPECT_FALSE(pluginManage->RemovePlugin(g_testPluginDir + SUCCESS_PLUGIN_NAME));
    EXPECT_TRUE(pluginManage->AddPlugin(g_testPluginDir + SUCCESS_PLUGIN_NAME));
    EXPECT_TRUE(pluginManage->LoadPlugin(pluginName));
    EXPECT_FALSE(pluginManage->LoadPlugin(pluginName));
    EXPECT_TRUE(pluginManage->UnloadPlugin(pluginName));

    EXPECT_TRUE(pluginManage->LoadPlugin(pluginName));

    std::vector<ProfilerPluginConfig> configVec;
    PluginResult result;
    configVec.push_back(config);
    EXPECT_TRUE(pluginManage->CreatePluginSession(configVec));
    EXPECT_TRUE(pluginManage->StartPluginSession(pluginIdsVector, configVec, result));
    std::this_thread::sleep_for(TEMP_DELAY);
    EXPECT_FALSE(pluginManage->ReportPluginBasicData(pluginIdsVector));
    EXPECT_TRUE(pluginManage->StopPluginSession(pluginIdsVector));
    EXPECT_TRUE(pluginManage->DestroyPluginSession(pluginIdsVector));
}

/**
 * @tc.name: plugin
 * @tc.desc: get sample Mode.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, GetSampleMode, TestSize.Level1)
{
    PluginModule pluginModule;
    if (pluginModule.structPtr_ && pluginModule.structPtr_->callbacks) {
        if (pluginModule.structPtr_->callbacks->onPluginReportResult != nullptr) {
            EXPECT_EQ(pluginModule.GetSampleMode(), PluginModule::SampleMode::POLLING);
        } else if (pluginModule.structPtr_->callbacks->onRegisterWriterStruct != nullptr) {
            EXPECT_EQ(pluginModule.GetSampleMode(), PluginModule::SampleMode::STREAMING);
        }
    }
    EXPECT_EQ(pluginModule.GetSampleMode(), PluginModule::SampleMode::UNKNOWN);
}

/**
 * @tc.name: plugin
 * @tc.desc: Plug-in data acquisition process test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, PluginManager, TestSize.Level1)
{
    PluginManager pluginManager;
    PluginModuleInfo info;
    EXPECT_FALSE(pluginManager.UnloadPlugin(0));
    PluginResult pluginResult;
    EXPECT_FALSE(pluginManager.SubmitResult(pluginResult));
    EXPECT_FALSE(pluginManager.PullResult(0));
    EXPECT_FALSE(pluginManager.CreateWriter("", 0, -1, -1));
    EXPECT_FALSE(pluginManager.ResetWriter(-1));

    PluginModule pluginModule;
    EXPECT_EQ(pluginModule.ComputeSha256(), "");
    EXPECT_FALSE(pluginModule.Unload());
    EXPECT_FALSE(pluginModule.GetInfo(info));
    std::string str("memory-plugin");
    EXPECT_FALSE(pluginModule.GetPluginName(str));
    uint32_t num = 0;
    EXPECT_FALSE(pluginModule.GetBufferSizeHint(num));
    EXPECT_FALSE(pluginModule.IsLoaded());

    BufferWriter bufferWriter("test", "1.01", DEFAULT_BUFFER_SIZE, -1, -1, 0);

    EXPECT_EQ(bufferWriter.shareMemoryBlock_, nullptr);
    EXPECT_FALSE(bufferWriter.Write(str.data(), str.size()));
    bufferWriter.shareMemoryBlock_ =
        ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("test", DEFAULT_BUFFER_SIZE);
    EXPECT_TRUE(bufferWriter.Write(str.data(), str.size()));
    EXPECT_TRUE(bufferWriter.Flush());
}
} // namespace
