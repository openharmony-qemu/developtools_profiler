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
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <iomanip>
#include <sys/time.h>
#include <sys/types.h>
#include <ctime>
#include <unistd.h>
#include <vector>

#include "logging.h"
#include "memory_data_plugin.h"
#include "plugin_module_api.h"
#include "sample_plugin.h"
#include "stream_plugin.h"

using namespace testing::ext;

namespace {
constexpr uint32_t MAX_BUFFER_SIZE = 4 * 1024 * 1024;
std::mutex g_taskMutex;
std::unique_ptr<MemoryDataPlugin> memoryPlugin = nullptr;
std::unique_ptr<StreamPlugin> streamPlugin = nullptr;
std::unique_ptr<SamplePlugin> samplePlugin = nullptr;

class PluginModuleApiTest : public ::testing::Test {
public:
    PluginModuleCallbacks* memoryCallbacks_;
    PluginModuleCallbacks* sampleCallbacks_;
    PluginModuleCallbacks* streamCallbacks_;

    PluginModuleStruct memoryModule_;
    PluginModuleStruct sampleModule_;
    PluginModuleStruct streamModule_;

    WriterStruct* writer_;
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp()
    {
        memoryCallbacks_ = nullptr;
        sampleCallbacks_ = nullptr;
        streamCallbacks_ = nullptr;
        memoryModule_ = {memoryCallbacks_, "memory-plugin", "1.01", MAX_BUFFER_SIZE};
        sampleModule_ = {sampleCallbacks_, "sample-plugin", "1.01", MAX_BUFFER_SIZE};
        streamModule_ = {streamCallbacks_, "stream-plugin", "1.01", MAX_BUFFER_SIZE};
        writer_ = nullptr;
    }

    bool PollPluginStub(MemoryDataPlugin& plugin, MemoryConfig& protoConfig, MemoryData& protoData);
    bool StreamPluginStub(StreamPlugin& plugin, StreamConfig& protoConfig, StreamData& protoData);
    // memory
    static int MemDataPluginSessionStart(const uint8_t* configData, uint32_t configSize);
    static int MemPluginReportResult(uint8_t* bufferData, uint32_t bufferSize);
    static int MemPluginSessionStop();

    // stream
    static int StreamPluginSessionStart(const uint8_t* configData, uint32_t configSize);
    static int StreamPluginSessionStop();
    static int StreamRegisterWriterStruct(WriterStruct* writer);
    static long Write(WriterStruct* writer, const void* data, size_t size);
    static bool Flush(WriterStruct* writer);

    // sample
    static int SamplePluginSessionStart(const uint8_t* configData, uint32_t configSize);
    static int SamplePluginReportResult(uint8_t* bufferData, uint32_t bufferSize);
    static int SamplePluginSessionStop();
    void SetSampleConfig(std::vector<uint8_t>& configData, uint32_t& configSize);
    void SetMemoryConfig(std::vector<uint8_t>& configData, uint32_t& configSize);
    void SetStreamConfig(std::vector<uint8_t>& configData, uint32_t& configSize);
};

// memory
int PluginModuleApiTest::MemDataPluginSessionStart(const uint8_t* configData, uint32_t configSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    memoryPlugin = std::make_unique<MemoryDataPlugin>();
    return memoryPlugin->Start(configData, configSize);
}

int PluginModuleApiTest::MemPluginReportResult(uint8_t* bufferData, uint32_t bufferSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    return memoryPlugin->Report(bufferData, bufferSize);
}

int PluginModuleApiTest::MemPluginSessionStop()
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    CHECK_TRUE(memoryPlugin != nullptr, -1, "memoryPlugin is null!!!");
    memoryPlugin->Stop();
    memoryPlugin = nullptr;
    return 0;
}

// stream
int PluginModuleApiTest::StreamPluginSessionStart(const uint8_t* configData, uint32_t configSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    CHECK_TRUE(streamPlugin != nullptr, -1, "PluginStub::start plugin fail!!!");
    return streamPlugin->Start(configData, configSize);
}

int PluginModuleApiTest::StreamPluginSessionStop()
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    streamPlugin->Stop();
    streamPlugin = nullptr;
    return 0;
}

int PluginModuleApiTest::StreamRegisterWriterStruct(WriterStruct* writer)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    streamPlugin = std::make_unique<StreamPlugin>();
    streamPlugin->SetWriter(writer);
    return 0;
}

// write
long PluginModuleApiTest::Write(WriterStruct* writer, const void* data, size_t size)
{
    HILOG_INFO(LOG_CORE, "PluginModuleApiTest::Write no implementation !");
    return 0;
}

bool PluginModuleApiTest::Flush(WriterStruct* writer)
{
    HILOG_INFO(LOG_CORE, "PluginModuleApiTest::Write no implementation !");
    return true;
}

// sample
int PluginModuleApiTest::SamplePluginSessionStart(const uint8_t* configData, uint32_t configSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    samplePlugin = std::make_unique<SamplePlugin>();
    return samplePlugin->Start(configData, configSize);
}

int PluginModuleApiTest::SamplePluginReportResult(uint8_t* bufferData, uint32_t bufferSize)
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    return samplePlugin->Report(bufferData, bufferSize);
}

int PluginModuleApiTest::SamplePluginSessionStop()
{
    std::lock_guard<std::mutex> guard(g_taskMutex);
    samplePlugin->Stop();
    samplePlugin = nullptr;
    return 0;
}

void PluginModuleApiTest::SetSampleConfig(std::vector<uint8_t>& configData, uint32_t& configSize)
{
    SampleConfig protoConfig;
    protoConfig.set_pid(1);
    // serialize
    protoConfig.SerializeToArray(configData.data(), configData.size());
    configSize = protoConfig.ByteSizeLong();
}

void PluginModuleApiTest::SetMemoryConfig(std::vector<uint8_t>& configData, uint32_t& configSize)
{
    MemoryConfig protoConfig;
    protoConfig.set_report_process_mem_info(true);
    protoConfig.add_pid(1);
    // serialize
    protoConfig.SerializeToArray(configData.data(), configSize);
    configSize = protoConfig.ByteSizeLong();
}

void PluginModuleApiTest::SetStreamConfig(std::vector<uint8_t>& configData, uint32_t& configSize)
{
    StreamConfig protoConfig;
    protoConfig.set_pid(1);
    // serialize
    protoConfig.SerializeToArray(configData.data(), configSize);
    configSize = protoConfig.ByteSizeLong();
}

// 流式调用写接口
HWTEST_F(PluginModuleApiTest, StreamPluginRegister1, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        StreamPluginSessionStart,
        0,
        StreamPluginSessionStop,
        (RegisterWriterStructCallback)StreamRegisterWriterStruct,
    };
    streamCallbacks_ = &callbacks;

    EXPECT_NE(streamCallbacks_->onRegisterWriterStruct, nullptr);
    EXPECT_EQ(streamCallbacks_->onRegisterWriterStruct(writer_), 0);
    EXPECT_NE(streamPlugin, nullptr);

    EXPECT_EQ(streamPlugin->resultWriter_, nullptr);
}

// 流式调用写接口
HWTEST_F(PluginModuleApiTest, StreamPluginRegister2, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        StreamPluginSessionStart,
        0,
        StreamPluginSessionStop,
        (RegisterWriterStructCallback)StreamRegisterWriterStruct,
    };
    streamCallbacks_ = &callbacks;

    EXPECT_NE(streamCallbacks_->onRegisterWriterStruct, nullptr);
    WriterStruct writer = {
        Write,
        Flush,
    };
    writer_ = &writer;
    EXPECT_EQ(streamCallbacks_->onRegisterWriterStruct(writer_), 0);
    EXPECT_NE(streamPlugin, nullptr);

    EXPECT_NE(streamPlugin->resultWriter_, nullptr);
    EXPECT_NE(streamPlugin->resultWriter_->write, nullptr);
}

// 流式调用写接口
HWTEST_F(PluginModuleApiTest, StreamPluginRegister3, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        StreamPluginSessionStart,
        0,
        StreamPluginSessionStop,
        (RegisterWriterStructCallback)StreamRegisterWriterStruct,
    };
    streamCallbacks_ = &callbacks;

    EXPECT_NE(streamCallbacks_->onRegisterWriterStruct, nullptr);
    WriterStruct writer = {
        Write,
        Flush,
    };
    writer_ = &writer;
    EXPECT_EQ(streamCallbacks_->onRegisterWriterStruct(writer_), 0);
    EXPECT_NE(streamPlugin, nullptr);

    EXPECT_NE(streamPlugin->resultWriter_, nullptr);
    EXPECT_NE(streamPlugin->resultWriter_->write, nullptr);
    EXPECT_NE(streamPlugin->resultWriter_->flush, nullptr);
}

// 轮询式调用写接口
HWTEST_F(PluginModuleApiTest, SamplePluginRegister, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;

    EXPECT_EQ(sampleCallbacks_->onRegisterWriterStruct, nullptr);
    EXPECT_EQ(samplePlugin, nullptr);
}

HWTEST_F(PluginModuleApiTest, MemPluginRegister, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        MemDataPluginSessionStart,
        MemPluginReportResult,
        MemPluginSessionStop,
        0,
    };
    memoryCallbacks_ = &callbacks;

    EXPECT_EQ(memoryCallbacks_->onRegisterWriterStruct, nullptr);
    EXPECT_EQ(memoryPlugin, nullptr);
}

// 流式插件正常启动
HWTEST_F(PluginModuleApiTest, StreamPluginStartSucc, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        StreamPluginSessionStart,
        0,
        StreamPluginSessionStop,
        (RegisterWriterStructCallback)StreamRegisterWriterStruct,
    };
    streamCallbacks_ = &callbacks;

    EXPECT_NE(streamCallbacks_->onRegisterWriterStruct, nullptr);
    WriterStruct writer = {
        Write,
        Flush,
    };
    writer_ = &writer;
    streamCallbacks_->onRegisterWriterStruct(writer_);
    EXPECT_NE(streamPlugin, nullptr);

    EXPECT_NE(streamCallbacks_->onPluginSessionStart, nullptr);
    uint32_t configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> configData(configLen);
    SetStreamConfig(configData, configLen);
    EXPECT_EQ(streamCallbacks_->onPluginSessionStart(configData.data(), configLen), 0);

    streamCallbacks_->onPluginSessionStop();
}

// 流式异常启动
HWTEST_F(PluginModuleApiTest, StreamPluginStartFail, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        StreamPluginSessionStart,
        0,
        StreamPluginSessionStop,
        (RegisterWriterStructCallback)StreamRegisterWriterStruct,
    };
    streamCallbacks_ = &callbacks;

    EXPECT_NE(streamCallbacks_->onRegisterWriterStruct, nullptr);
    WriterStruct writer;
    writer_ = &writer;
    streamCallbacks_->onRegisterWriterStruct(writer_);
    EXPECT_NE(streamPlugin, nullptr);

    EXPECT_NE(streamCallbacks_->onPluginSessionStart, nullptr);
    std::vector<uint8_t> configData(MAX_BUFFER_SIZE);
    EXPECT_EQ(streamCallbacks_->onPluginSessionStart(configData.data(), configData.size()), -1);
}

// 轮询式异常启动
HWTEST_F(PluginModuleApiTest, SamplePluginStartFail, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;

    EXPECT_NE(sampleCallbacks_->onPluginSessionStart, nullptr);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStart(nullptr, 0), -1);
    EXPECT_NE(samplePlugin, nullptr);
}

// 轮询式插件正常启动
HWTEST_F(PluginModuleApiTest, SamplePluginStartSucc, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;

    EXPECT_NE(sampleCallbacks_->onPluginSessionStart, nullptr);
    uint32_t configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> configData(configLen); // config
    SetSampleConfig(configData, configLen);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStart(configData.data(), configLen), 0);
    EXPECT_NE(samplePlugin, nullptr);

    sampleCallbacks_->onPluginSessionStop();
}

// 多个插件启动
HWTEST_F(PluginModuleApiTest, MultPluginStart, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    PluginModuleCallbacks memCallbacks = {
        MemDataPluginSessionStart,
        MemPluginReportResult,
        MemPluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;
    memoryCallbacks_ = &memCallbacks;
    uint32_t configLen = MAX_BUFFER_SIZE;

    EXPECT_NE(sampleCallbacks_->onPluginSessionStart, nullptr);
    std::vector<uint8_t> sampleConfigData(configLen);
    SetSampleConfig(sampleConfigData, configLen);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStart(sampleConfigData.data(), configLen), 0);
    EXPECT_NE(samplePlugin, nullptr);
    sampleCallbacks_->onPluginSessionStop();

    EXPECT_NE(memoryCallbacks_->onPluginSessionStart, nullptr);
    configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> memoryConfigData(configLen);
    SetMemoryConfig(memoryConfigData, configLen);
    EXPECT_EQ(memoryCallbacks_->onPluginSessionStart(memoryConfigData.data(), configLen), 0);
    EXPECT_NE(memoryPlugin, nullptr);
    memoryCallbacks_->onPluginSessionStop();
}

// 流式上报
HWTEST_F(PluginModuleApiTest, StreamPluginReport, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        StreamPluginSessionStart,
        0,
        StreamPluginSessionStop,
        (RegisterWriterStructCallback)StreamRegisterWriterStruct,
    };
    streamCallbacks_ = &callbacks;

    EXPECT_EQ(streamCallbacks_->onPluginReportResult, nullptr);
}

// 轮询式上报
HWTEST_F(PluginModuleApiTest, SamplePluginReport, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;

    EXPECT_NE(sampleCallbacks_->onPluginSessionStart, nullptr);
    uint32_t configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> configData(configLen);
    SetSampleConfig(configData, configLen);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStart(configData.data(), configLen), 0);
    EXPECT_NE(samplePlugin, nullptr);

    EXPECT_NE(sampleCallbacks_->onPluginReportResult, nullptr);
    std::vector<uint8_t> buffer(MAX_BUFFER_SIZE);
    EXPECT_NE(sampleCallbacks_->onPluginReportResult(buffer.data(), buffer.size()), 0);
    sampleCallbacks_->onPluginSessionStop();
}

HWTEST_F(PluginModuleApiTest, MemoryPluginReport, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        MemDataPluginSessionStart,
        MemPluginReportResult,
        MemPluginSessionStop,
        0,
    };
    memoryCallbacks_ = &callbacks;

    EXPECT_NE(memoryCallbacks_->onPluginSessionStart, nullptr);
    uint32_t configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> configData(configLen);
    SetMemoryConfig(configData, configLen);
    EXPECT_EQ(memoryCallbacks_->onPluginSessionStart(configData.data(), configLen), 0);
    EXPECT_NE(memoryPlugin, nullptr);

    EXPECT_NE(memoryCallbacks_->onPluginReportResult, nullptr);
    std::vector<uint8_t> buffer(MAX_BUFFER_SIZE);
    EXPECT_NE(memoryCallbacks_->onPluginReportResult(buffer.data(), buffer.size()), 0);
    memoryCallbacks_->onPluginSessionStop();
}

// 多个插件上报
HWTEST_F(PluginModuleApiTest, MultPluginReport1, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;
    uint32_t configLen = MAX_BUFFER_SIZE;

    EXPECT_NE(sampleCallbacks_->onPluginSessionStart, nullptr);
    std::vector<uint8_t> sampleConfigData(configLen);
    SetSampleConfig(sampleConfigData, configLen);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStart(sampleConfigData.data(), configLen), 0);
    EXPECT_NE(samplePlugin, nullptr);
    EXPECT_NE(sampleCallbacks_->onPluginReportResult, nullptr);
    std::vector<uint8_t> sampleBuffer(MAX_BUFFER_SIZE);
    EXPECT_NE(sampleCallbacks_->onPluginReportResult(sampleBuffer.data(), sampleBuffer.size()), 0);

    PluginModuleCallbacks memCallbacks = {
        MemDataPluginSessionStart,
        MemPluginReportResult,
        MemPluginSessionStop,
        0,
    };
    memoryCallbacks_ = &memCallbacks;

    configLen = MAX_BUFFER_SIZE;
    EXPECT_NE(memoryCallbacks_->onPluginSessionStart, nullptr);
    std::vector<uint8_t> memoryConfigData(configLen);
    SetMemoryConfig(memoryConfigData, configLen);
    EXPECT_EQ(memoryCallbacks_->onPluginSessionStart(memoryConfigData.data(), configLen), 0);
    EXPECT_NE(memoryPlugin, nullptr);
    EXPECT_NE(memoryCallbacks_->onPluginReportResult, nullptr);
    std::vector<uint8_t> memoryBuffer(MAX_BUFFER_SIZE);
    EXPECT_NE(memoryCallbacks_->onPluginReportResult(memoryBuffer.data(), memoryBuffer.size()), 0);

    sampleCallbacks_->onPluginSessionStop();
    memoryCallbacks_->onPluginSessionStop();
}

HWTEST_F(PluginModuleApiTest, MultPluginReport2, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;
    uint32_t configLen = MAX_BUFFER_SIZE;

    EXPECT_NE(sampleCallbacks_->onPluginSessionStart, nullptr);
    std::vector<uint8_t> sampleConfigData(configLen);
    SetSampleConfig(sampleConfigData, configLen);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStart(sampleConfigData.data(), configLen), 0);
    EXPECT_NE(samplePlugin, nullptr);
    EXPECT_NE(sampleCallbacks_->onPluginReportResult, nullptr);
    std::vector<uint8_t> sampleBuffer(MAX_BUFFER_SIZE);
    EXPECT_NE(sampleCallbacks_->onPluginReportResult(sampleBuffer.data(), sampleBuffer.size()), 0);

    PluginModuleCallbacks streamCallbacks = {
        StreamPluginSessionStart,
        0,
        StreamPluginSessionStop,
        (RegisterWriterStructCallback)StreamRegisterWriterStruct,
    };
    streamCallbacks_ = &streamCallbacks;

    EXPECT_NE(streamCallbacks_->onRegisterWriterStruct, nullptr);
    WriterStruct writer = {
        Write,
        Flush,
    };
    writer_ = &writer;
    streamCallbacks_->onRegisterWriterStruct(writer_);
    EXPECT_NE(streamPlugin, nullptr);
    EXPECT_NE(streamPlugin->resultWriter_, nullptr);
    EXPECT_NE(streamPlugin->resultWriter_->write, nullptr);
    EXPECT_NE(streamPlugin->resultWriter_->flush, nullptr);

    EXPECT_NE(streamCallbacks_->onPluginSessionStart, nullptr);
    configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> configData(configLen);
    SetStreamConfig(configData, configLen);
    EXPECT_EQ(streamCallbacks_->onPluginSessionStart(configData.data(), configLen), 0);

    sampleCallbacks_->onPluginSessionStop();
    streamCallbacks_->onPluginSessionStop();
}

// 流式插件停止
HWTEST_F(PluginModuleApiTest, StreamPluginStop, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        StreamPluginSessionStart,
        0,
        StreamPluginSessionStop,
        (RegisterWriterStructCallback)StreamRegisterWriterStruct,
    };
    streamCallbacks_ = &callbacks;

    EXPECT_NE(streamCallbacks_->onRegisterWriterStruct, nullptr);
    streamCallbacks_->onRegisterWriterStruct(writer_);
    EXPECT_NE(streamPlugin, nullptr);

    EXPECT_NE(streamCallbacks_->onPluginSessionStart, nullptr);
    std::vector<uint8_t> configData(1);
    streamCallbacks_->onPluginSessionStart(configData.data(), configData.size());

    EXPECT_NE(streamCallbacks_->onPluginSessionStop, nullptr);
    EXPECT_EQ(streamCallbacks_->onPluginSessionStop(), 0);
}

// 轮询式插件停止
HWTEST_F(PluginModuleApiTest, SamplePluginStop, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;

    EXPECT_NE(sampleCallbacks_->onPluginSessionStart, nullptr);
    uint32_t configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> configData(configLen);
    SetSampleConfig(configData, configLen);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStart(configData.data(), configLen), 0);
    EXPECT_NE(samplePlugin, nullptr);

    EXPECT_NE(sampleCallbacks_->onPluginSessionStop, nullptr);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStop(), 0);
}

HWTEST_F(PluginModuleApiTest, MemoryPluginStop, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        MemDataPluginSessionStart,
        MemPluginReportResult,
        MemPluginSessionStop,
        0,
    };
    memoryCallbacks_ = &callbacks;

    EXPECT_NE(memoryCallbacks_->onPluginSessionStart, nullptr);
    uint32_t configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> memoryConfigData(configLen);
    SetMemoryConfig(memoryConfigData, configLen);
    EXPECT_EQ(memoryCallbacks_->onPluginSessionStart(memoryConfigData.data(), configLen), 0);
    EXPECT_NE(memoryPlugin, nullptr);

    EXPECT_NE(memoryCallbacks_->onPluginSessionStop, nullptr);
    EXPECT_EQ(memoryCallbacks_->onPluginSessionStop(), 0);
}

// 多个插件停止
HWTEST_F(PluginModuleApiTest, MultPluginStop, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        SamplePluginSessionStart,
        SamplePluginReportResult,
        SamplePluginSessionStop,
        0,
    };
    PluginModuleCallbacks memCallbacks = {
        MemDataPluginSessionStart,
        MemPluginReportResult,
        MemPluginSessionStop,
        0,
    };
    sampleCallbacks_ = &callbacks;
    memoryCallbacks_ = &memCallbacks;
    uint32_t configLen = MAX_BUFFER_SIZE;

    EXPECT_NE(sampleCallbacks_->onPluginSessionStart, nullptr);
    std::vector<uint8_t> sampleConfigData(configLen);
    SetSampleConfig(sampleConfigData, configLen);
    EXPECT_EQ(sampleCallbacks_->onPluginSessionStart(sampleConfigData.data(), configLen), 0);
    EXPECT_NE(samplePlugin, nullptr);

    EXPECT_NE(memoryCallbacks_->onPluginSessionStart, nullptr);
    configLen = MAX_BUFFER_SIZE;
    std::vector<uint8_t> memoryConfigData(configLen);
    SetMemoryConfig(memoryConfigData, configLen);
    EXPECT_EQ(memoryCallbacks_->onPluginSessionStart(memoryConfigData.data(), configLen), 0);
    EXPECT_NE(memoryPlugin, nullptr);

    EXPECT_EQ(sampleCallbacks_->onPluginSessionStop(), 0);
    EXPECT_EQ(memoryCallbacks_->onPluginSessionStop(), 0);
}

// 异常停止
HWTEST_F(PluginModuleApiTest, PluginAbnormalStop, TestSize.Level1)
{
    PluginModuleCallbacks callbacks = {
        MemDataPluginSessionStart,
        MemPluginReportResult,
        MemPluginSessionStop,
        0,
    };
    memoryCallbacks_ = &callbacks;

    EXPECT_NE(memoryCallbacks_->onPluginSessionStop, nullptr);
    EXPECT_EQ(memoryCallbacks_->onPluginSessionStop(), -1);
}
} // namespace