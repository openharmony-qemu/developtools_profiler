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
#include <dlfcn.h>
#include <fcntl.h>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <type_traits>
#include <unistd.h>

#include "file_utils.h"
#include "flow_controller.h"

using namespace testing::ext;

namespace {
constexpr uint32_t BUFFER_SIZE_KB = 256;
constexpr uint32_t BUFFER_SIZE_MIN_KB = 63;
constexpr uint32_t BUFFER_SIZE_MAX_KB = 64 * 1024 + 1;
constexpr uint32_t FLUSH_INTERVAL_MS = 1000;
constexpr uint32_t FLUSH_THRESHOLD_KB = 1024;
constexpr uint32_t TRACE_PERIOD_MS = 500;
constexpr uint32_t TEST_CPU_NUM = 4;
constexpr uint32_t TRACE_TIME = 3;
constexpr uint32_t BUFFER_SIZE = 2 * 1024 * 1024;
RandomWriteCtx g_writeCtx = {};
std::unique_ptr<uint8_t[]> g_buffer = nullptr;
using WriterStructPtr = std::unique_ptr<WriterStruct>::pointer;
using ConstVoidPtr = std::unique_ptr<const void>::pointer;
class FlowControllerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

long WriteFunc(WriterStructPtr writer, ConstVoidPtr data, size_t size)
{
    if (writer == nullptr || data == nullptr || size <= 0) {
        return -1;
    }

    return 0;
}

bool FlushFunc(WriterStructPtr writer)
{
    if (writer == nullptr) {
        return false;
    }
    return true;
}

RandomWriteCtx* StartReportFunc(WriterStructPtr writer)
{
    if (writer == nullptr) {
        return nullptr;
    }

    g_writeCtx.getMemory = [](RandomWriteCtx* ctx, uint32_t size, uint8_t** memory, uint32_t* offset) -> bool {
        if (size > BUFFER_SIZE) {
            return false;
        }

        *memory = g_buffer.get();
        *offset = 0;
        return true;
    };
    g_writeCtx.seek = [](RandomWriteCtx* ctx, uint32_t offset) -> bool {
        return true;
    };

    return &g_writeCtx;
}

void FinishReportFunc(WriterStructPtr writer, int32_t size)
{
    return;
}

/*
 * @tc.name: SetWriter
 * @tc.desc: test FlowController::SetWriter.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, SetWriter, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    WriterStruct writer = {WriteFunc, FlushFunc};
    EXPECT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);
}

/*
 * @tc.name: LoadConfig
 * @tc.desc: test FlowController::LoadConfig.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, LoadConfig, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    TracePluginConfig config;

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);

    // set config
    config.add_ftrace_events("sched/sched_switch");
    config.set_buffer_size_kb(BUFFER_SIZE_KB);
    config.set_flush_interval_ms(FLUSH_INTERVAL_MS);
    config.set_flush_threshold_kb(FLUSH_THRESHOLD_KB);
    config.set_parse_ksyms(true);
    config.set_clock("global");
    config.set_trace_period_ms(TRACE_PERIOD_MS);
    config.set_raw_data_prefix("/data/local/tmp/raw_trace_");
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), 0);
}

/*
 * @tc.name: LoadConfig
 * @tc.desc: test FlowController::LoadConfig.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, SetMinBufForLoadConfig, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    TracePluginConfig config;

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);

    // set config
    config.set_buffer_size_kb(BUFFER_SIZE_MIN_KB);
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), -1);
}

/*
 * @tc.name: LoadConfig
 * @tc.desc: test FlowController::LoadConfig.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, SetMaxBufForLoadConfig, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    TracePluginConfig config;

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);

    // set config
    config.set_buffer_size_kb(BUFFER_SIZE_MAX_KB);
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), -1);
}

/*
 * @tc.name: LoadConfig
 * @tc.desc: test FlowController::LoadConfig.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, SetTracePeriodForLoadConfig, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    TracePluginConfig config;

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);

    // set config
    config.add_hitrace_apps("ftrace_plugin_ut");
    config.add_hitrace_categories("idle");
    config.add_hitrace_categories("ability");
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), 0);
}

/*
 * @tc.name: LoadConfig
 * @tc.desc: test FlowController::LoadConfig.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, SetHitraceAppForLoadConfig, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    TracePluginConfig config;

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);

    // set config
    config.add_ftrace_events("sched/sched_switch");
    config.add_hitrace_categories("ability");
    config.set_trace_period_ms(0);
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), 0);
}

/*
 * @tc.name: StartCapture
 * @tc.desc: test FlowController::StartCapture.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, StartCapture, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    TracePluginConfig config;

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);

    // set config
    config.add_ftrace_events("sched/sched_switch");
    config.add_hitrace_categories("ability");
    config.add_hitrace_categories("ace");
    config.set_buffer_size_kb(BUFFER_SIZE_KB);
    config.set_flush_interval_ms(FLUSH_INTERVAL_MS);
    config.set_flush_threshold_kb(FLUSH_THRESHOLD_KB);
    config.set_parse_ksyms(true);
    config.set_clock("global");
    config.set_trace_period_ms(TRACE_PERIOD_MS);
    config.set_raw_data_prefix("/data/local/tmp/raw_trace_");
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), 0);

    EXPECT_EQ(controller.StartCapture(), 0);
    EXPECT_EQ(controller.StopCapture(), 0);

    if (OHOS::Profiler::Plugins::FtraceFsOps::GetInstance().IsHmKernel() == false) {
        controller.parseMode_ = TracePluginConfig_ParseMode_DELAY_PARSE;
        EXPECT_EQ(controller.StartCapture(), 0);
        EXPECT_EQ(controller.StopCapture(), 0);
    }
}

/*
 * @tc.name: Hitrace Apps
 * @tc.desc: test FlowController::hitrace_apps.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, HitraceApps, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    TracePluginConfig config;

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);

    // set config
    config.add_hitrace_apps("render_service");
    config.add_hitrace_categories("ability");
    config.add_hitrace_categories("ace");
    config.add_hitrace_categories("binder");
    config.add_hitrace_categories("graphic");
    config.add_hitrace_categories("idle");
    config.set_buffer_size_kb(BUFFER_SIZE_KB);
    config.set_flush_interval_ms(FLUSH_INTERVAL_MS);
    config.set_flush_threshold_kb(FLUSH_THRESHOLD_KB);
    config.set_parse_ksyms(true);
    config.set_clock("global");
    config.set_trace_period_ms(TRACE_PERIOD_MS);
    config.set_raw_data_prefix("/data/local/tmp/raw_trace_");
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), 0);

    EXPECT_EQ(controller.StartCapture(), 0);
    EXPECT_EQ(controller.StopCapture(), 0);

    if (OHOS::Profiler::Plugins::FtraceFsOps::GetInstance().IsHmKernel() == false) {
        controller.parseMode_ = TracePluginConfig_ParseMode_DELAY_PARSE;
        EXPECT_EQ(controller.StartCapture(), 0);
        EXPECT_EQ(controller.StopCapture(), 0);
    }
}

/*
 * @tc.name: StartCapture
 * @tc.desc: test FlowController::StartCaptureWithBinder.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, StartCaptureWithBinder, TestSize.Level1)
{
    OHOS::Profiler::Plugins::FlowController controller;
    TracePluginConfig config;

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);

    // set config
    config.add_ftrace_events("binder/binder_transaction");
    config.add_hitrace_categories("ability");
    config.add_hitrace_categories("ace");
    config.set_buffer_size_kb(BUFFER_SIZE_KB);
    config.set_flush_interval_ms(FLUSH_INTERVAL_MS);
    config.set_flush_threshold_kb(FLUSH_THRESHOLD_KB);
    config.set_parse_ksyms(true);
    config.set_clock("global");
    config.set_trace_period_ms(TRACE_PERIOD_MS);
    config.set_raw_data_prefix("/data/local/tmp/raw_trace_");
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), 0);

    EXPECT_EQ(controller.StartCapture(), 0);
    EXPECT_EQ(controller.StopCapture(), 0);

    if (OHOS::Profiler::Plugins::FtraceFsOps::GetInstance().IsHmKernel() == false) {
        controller.parseMode_ = TracePluginConfig_ParseMode_DELAY_PARSE;
        EXPECT_EQ(controller.StartCapture(), 0);
        EXPECT_EQ(controller.StopCapture(), 0);
    }
}

/*
 * @tc.name: frace_module
 * @tc.desc: test Framework.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, TestFramework, TestSize.Level1)
{
    std::string path = std::string("libftrace_plugin.z.so");
    auto handle = dlopen(path.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    PluginModuleStruct* plugin = reinterpret_cast<PluginModuleStruct*>(dlsym(handle, "g_pluginModule"));
    EXPECT_NE(plugin, nullptr);
    EXPECT_STREQ(plugin->name, "ftrace-plugin");

    // set config
    TracePluginConfig config;
    config.add_ftrace_events("sched/sched_switch");
    config.add_hitrace_categories("ability");
    config.add_hitrace_categories("ace");
    config.set_buffer_size_kb(BUFFER_SIZE_KB);
    config.set_flush_interval_ms(FLUSH_INTERVAL_MS);
    config.set_flush_threshold_kb(FLUSH_THRESHOLD_KB);
    config.set_parse_ksyms(true);
    config.set_clock("global");
    config.set_trace_period_ms(TRACE_PERIOD_MS);
    config.set_raw_data_prefix("/data/local/tmp/raw_trace_");
    std::vector<uint8_t> configData(config.ByteSizeLong());
    ASSERT_GT(config.SerializeToArray(configData.data(), configData.size()), 0);

    // test framework process
    WriterStruct writer = {WriteFunc, FlushFunc};
    std::vector<uint8_t> dataBuffer(plugin->resultBufferSizeHint);
    EXPECT_EQ(plugin->callbacks->onRegisterWriterStruct(&writer), 0);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStart(configData.data(), configData.size()), 0);
    EXPECT_EQ(plugin->callbacks->onPluginSessionStop(), 0);
}

/*
 * @tc.name: frace_module
 * @tc.desc: test ftrace plugin based on resource files.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, TestFrameworkWithFile, TestSize.Level1)
{
    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc};
    OHOS::Profiler::Plugins::FlowController controller;
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);
    controller.SetTestInfo(TEST_CPU_NUM, "/data/local/tmp/");

    // set config
    TracePluginConfig config;
    config.add_ftrace_events("sched/sched_switch");
    config.add_hitrace_categories("ability");
    config.add_hitrace_categories("ace");
    config.set_buffer_size_kb(BUFFER_SIZE);
    config.set_trace_period_ms(TRACE_PERIOD_MS);
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), 0);

    EXPECT_EQ(controller.StartCapture(), 0);
    sleep(TRACE_TIME);
    EXPECT_EQ(controller.StopCapture(), 0);
}

/*
 * @tc.name: frace_module
 * @tc.desc: test ftrace plugin encoder based on resource files.
 * @tc.type: FUNC
 */
HWTEST_F(FlowControllerTest, TestFrameworkEncoder, TestSize.Level1)
{
    g_buffer = std::make_unique<uint8_t[]>(BUFFER_SIZE);
    ASSERT_NE(g_buffer.get(), nullptr);

    // set writer
    WriterStruct writer = {WriteFunc, FlushFunc, StartReportFunc, FinishReportFunc, false};
    OHOS::Profiler::Plugins::FlowController controller;
    ASSERT_EQ(controller.SetWriter(static_cast<WriterStructPtr>(&writer)), 0);
    controller.SetTestInfo(TEST_CPU_NUM, "/data/local/tmp/");

    // set config
    TracePluginConfig config;
    config.add_ftrace_events("sched/sched_switch");
    config.add_hitrace_categories("ability");
    config.add_hitrace_categories("ace");
    config.set_buffer_size_kb(BUFFER_SIZE);
    config.set_trace_period_ms(TRACE_PERIOD_MS);
    std::vector<uint8_t> configData(config.ByteSizeLong());
    int ret = config.SerializeToArray(configData.data(), configData.size());
    ASSERT_GT(ret, 0);
    EXPECT_EQ(controller.LoadConfig(configData.data(), configData.size()), 0);

    EXPECT_EQ(controller.StartCapture(), 0);
    sleep(TRACE_TIME);
    EXPECT_EQ(controller.StopCapture(), 0);
}
} // namespace
