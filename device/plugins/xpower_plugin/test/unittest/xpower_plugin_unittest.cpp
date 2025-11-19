/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#include "xpower_plugin.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "plugin_module_api.h"

using namespace testing::ext;

namespace {
std::vector<OptimizeReport> g_protoXpower;
long WriteFunc(WriterStruct* writer, const void* data, size_t size)
{
    if (writer == nullptr || data == nullptr || size == 0) {
        return -1;
    }

    OptimizeReport info;
    if (info.ParseFromArray(data, size) <= 0) {
        return -1;
    }
    g_protoXpower.push_back(info);
    return 0;
}

bool FlushFunc(WriterStruct* writer)
{
    if (writer == nullptr) {
        return false;
    }
    return true;
}

class XpowerPluginTest : public ::testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: xpower plugin
 * @tc.desc: Framework test
 * @tc.type: FUNC
 */
HWTEST_F(XpowerPluginTest, TestFramework, TestSize.Level1)
{
#if defined(__LP64__)
    std::string path = std::string("libxpowerplugin.z.so");
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "xpower-plugin");
    g_protoXpower.clear();
    // set config
    XpowerConfig config;
    config.set_bundle_name("com.ohos.sceneboard");
    config.add_message_type(XpowerMessageType::REAL_BATTERY);
    config.add_message_type(XpowerMessageType::APP_STATISTIC);
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);
    // test framework process
    WriterStruct writer = {WriteFunc, FlushFunc};
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onRegisterWriterStruct(&writer), 0);
    plugin->callbacks->onPluginSessionStart(configData.data(), configData.size());
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(4096);
    plugin->callbacks->onPluginReportResult(buffer.get(), 4096);
    plugin->callbacks->onPluginSessionStop();

    // test proto data
    int vectSize = g_protoXpower.size();
    EXPECT_TRUE(vectSize >= 0);
#endif
}

/**
 * @tc.name: xpower plugin
 * @tc.desc: start fail test
 * @tc.type: FUNC
 * @tc.require: issueI5UGTK
 */
HWTEST_F(XpowerPluginTest, TestStartFail, TestSize.Level1)
{
#if defined(__LP64__)
    XpowerConfig config;
    XpowerPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};
    // set config
    config.set_bundle_name("");
    config.add_message_type(XpowerMessageType::REAL_BATTERY);
    // test plugin process
    plugin.SetWriter(&writer);
    // serialize
    int size = config.ByteSizeLong();
    ASSERT_GT(size, 0);
    std::vector<uint8_t> configData(size);
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);
    // start
    int retTestFirst = plugin.Start(configData.data(), configData.size());
    if (plugin.IsListenerHandleValid()) {
        EXPECT_EQ(retTestFirst, 0);
        EXPECT_NE(plugin.Start(nullptr, configData.size()), 0);
        EXPECT_NE(plugin.Start(configData.data(), 0), 0);
    } else {
        EXPECT_EQ(retTestFirst, -1);
    }
#endif
}

/**
 * @tc.name: XpowerPluginTest.TestGetCmdArgs
 * @tc.desc: test get cmd args
 * @tc.type: FUNC
 */
HWTEST_F(XpowerPluginTest, TestGetCmdArgs, TestSize.Level1)
{
    XpowerPlugin plugin;
    XpowerConfig config;
    config.set_bundle_name("com.example.app");
    config.add_message_type(XpowerMessageType::REAL_BATTERY);
    std::string args = plugin.GetCmdArgs(config);
    EXPECT_TRUE(args.find("bundle_name: com.example.app") != std::string::npos);
}

/**
 * @tc.name: XpowerPluginTest.TestStartPowerManager
 * @tc.desc: test StartPowerManager
 * @tc.type: FUNC
 */
HWTEST_F(XpowerPluginTest, TestStartPowerManager, TestSize.Level1)
{
    XpowerPlugin plugin;
    std::string bundleName = "com.example.app";
    uint32_t messageType = OptimizeMessageType::MESSAGE_REAL_BATTERY;
    bool result = plugin.StartPowerManager(messageType, bundleName);
    if (plugin.IsListenerHandleValid()) {
        EXPECT_TRUE(result);
    } else {
        EXPECT_FALSE(result);
    }
}

/**
 * @tc.name: XpowerPluginTest.TestOptimizeCallback
 * @tc.desc: test optimize callback
 * @tc.type: FUNC
 */
HWTEST_F(XpowerPluginTest, TestOptimizeCallback, TestSize.Level1)
{
    XpowerPlugin plugin;
    WriterStruct writer = {WriteFunc, FlushFunc};
    plugin.SetWriter(&writer);
    plugin.dataQueuePtr_ = std::make_shared<PowerMessageQueue>(2000);

    uint8_t data[] = {0x01, 0x02, 0x03};
    plugin.OptimizeCallback(OptimizeMessageType::MESSAGE_REAL_BATTERY, data, sizeof(data));

    std::shared_ptr<PowerOptimizeData> result = nullptr;
    plugin.dataQueuePtr_->WaitAndPop(result, std::chrono::milliseconds(100));
    EXPECT_TRUE(result != nullptr);
    EXPECT_EQ(result->length, sizeof(data));
}

/**
 * @tc.name: XpowerPluginTest.TestReport
 * @tc.desc: test report
 * @tc.type: FUNC
 */
HWTEST_F(XpowerPluginTest, TestReport, TestSize.Level1)
{
    XpowerPlugin plugin;
    XpowerConfig config;
    config.set_bundle_name("com.example.app");
    config.add_message_type(XpowerMessageType::REAL_BATTERY);

    int size = config.ByteSizeLong();
    std::vector<uint8_t> configData(size);
    config.SerializeToArray(configData.data(), configData.size());

    WriterStruct writer = {WriteFunc, FlushFunc};
    plugin.SetWriter(&writer);

    plugin.Start(configData.data(), configData.size());

    std::vector<uint8_t> buffer(1024);
    int result = plugin.Report(buffer.data(), buffer.size());
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: xpower plugin
 * @tc.desc: message queue test
 * @tc.type: FUNC
 * @tc.require: issueI5UGTK
 */
HWTEST_F(XpowerPluginTest, TestMessageQueue, TestSize.Level1)
{
    const int msgQueueSize = 2000;
    std::unique_ptr<PowerMessageQueue> dataQueuePtr = std::make_unique<PowerMessageQueue>(msgQueueSize);
    auto rawData = std::make_shared<PowerOptimizeData>();
    rawData->messageType = OptimizeMessageType::MESSAGE_REAL_BATTERY;
    dataQueuePtr->PushBack(rawData);
    EXPECT_GE(dataQueuePtr->Size(), 0);
    EXPECT_TRUE(!dataQueuePtr->Empty());
    std::shared_ptr<PowerOptimizeData> result = nullptr;
    const uint32_t waitDuration = 100;
    EXPECT_TRUE(dataQueuePtr->WaitAndPop(result, std::chrono::milliseconds(waitDuration)));
    EXPECT_TRUE(result != nullptr);
    const int batchSize = 5;
    std::vector<std::shared_ptr<PowerOptimizeData>> araryData(batchSize); // 5: the size of std::vector;
    EXPECT_FALSE(dataQueuePtr->WaitAndPopBatch(araryData, std::chrono::milliseconds(waitDuration), batchSize));
    for (size_t i = 0; i < 3; i++) {
        auto rawData = std::make_shared<PowerOptimizeData>();
        rawData->messageType = OptimizeMessageType::MESSAGE_REAL_BATTERY;
        rawData->length = i;
        dataQueuePtr->PushBack(rawData);
    }
    EXPECT_TRUE(dataQueuePtr->WaitAndPopBatch(araryData, std::chrono::milliseconds(waitDuration), batchSize));
    dataQueuePtr->ShutDown();
}

} // namespace
