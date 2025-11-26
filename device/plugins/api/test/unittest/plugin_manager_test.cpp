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
#include <gtest/gtest.h>
#include <thread>
#include <sys/syscall.h>
#include <sys/eventfd.h>

#include "command_poller.h"
#include "grpc/impl/codegen/log.h"
#include "logging.h"
#include "parameters.h"
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
constexpr uint32_t SMB_SIZE = 10 * 4096;
const std::string SMB_NAME = "testsmb";
void *g_smbAddr = nullptr;
const std::string SUCCESS_PLUGIN_NAME = "libmemdataplugin.z.so";
const std::string HIDUMPER_PLUGIN_NAME = "libhidumpplugin.z.so";
#if defined(__LP64__)
std::string g_testPluginDir("/system/lib64/");
#else
std::string g_testPluginDir("/system/lib/");
#endif

int InitShareMemory()
{
    int fd = syscall(SYS_memfd_create, SMB_NAME.c_str(), 0);
    CHECK_TRUE(fd >= 0, -1, "CreateBlock FAIL SYS_memfd_create");

    int check = ftruncate(fd, SMB_SIZE);
    if (check < 0) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock ftruncate ERR : %s", buf);
        return -1;
    }

    g_smbAddr = mmap(nullptr, SMB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (g_smbAddr == static_cast<void*>(MAP_FAILED)) {
        close(fd);
        const int bufSize = 256;
        char buf[bufSize] = { 0 };
        strerror_r(errno, buf, bufSize);
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock g_smbAddr mmap ERR : %s", buf);
        return -1;
    }

    ShareMemoryBlock::BlockHeader* header_ = reinterpret_cast<ShareMemoryBlock::BlockHeader*>(g_smbAddr);
    if (header_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "CreateBlock g_smbAddr header_ is null");
        close(fd);
        munmap(g_smbAddr, SMB_SIZE);
        g_smbAddr = nullptr;
        return -1;
    }
    // initialize header infos
    header_->info.readOffset_ = 0;
    header_->info.writeOffset_ = 0;
    header_->info.memorySize_ = SMB_SIZE - sizeof(ShareMemoryBlock::BlockHeader);
    header_->info.bytesCount_ = 0;
    header_->info.chunkCount_ = 0;

    return fd;
}

class PluginManagerTest : public ::testing::Test {
   protected:
    static constexpr auto TEMP_DELAY = std::chrono::milliseconds(20);
    void SetUp() override
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
#ifdef COVERAGE_TEST
        const int coverageSleepTime = DEFAULT_SLEEP_TIME * 5;  // sleep 5s
        std::this_thread::sleep_for(std::chrono::milliseconds(coverageSleepTime));
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
#endif
    }

    void TearDown() override
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
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

    ASSERT_GT(configVec.size(), 0);
    configVec[0].set_sample_interval(0);
    EXPECT_FALSE(pluginManage->StartPluginSession(pluginIdsVector, configVec, result));

    PluginResult plgResult;
    EXPECT_FALSE(pluginManage->SubmitResult(plgResult));
    std::this_thread::sleep_for(TEMP_DELAY);
    EXPECT_FALSE(pluginManage->ReportPluginBasicData(pluginIdsVector));
    EXPECT_TRUE(pluginManage->StopPluginSession(pluginIdsVector));
    EXPECT_TRUE(pluginManage->DestroyPluginSession(pluginIdsVector));
    const std::vector<uint32_t> idsFailed = { 100, 200};
    EXPECT_FALSE(pluginManage->DestroyPluginSession(idsFailed));
}

/**
 * @tc.name: plugin
 * @tc.desc: Plug-in normal stream mode
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, PluginStreamMode, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManager>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    EXPECT_TRUE(commandPoller->OnConnect());
    pluginManage->SetCommandPoller(commandPoller);
    const uint8_t configData[] = {0x30, 0x01, 0x38, 0x01, 0x42, 0x01, 0x01}; // test data
    std::string pluginName = "hidump-plugin";
    ProfilerPluginConfig config;
    config.set_name(pluginName);
    config.set_config_data((const void*)configData, 7); // 7: configData size
    config.set_sample_interval(0);
    EXPECT_TRUE(pluginManage->AddPlugin(g_testPluginDir + HIDUMPER_PLUGIN_NAME));
    EXPECT_TRUE(pluginManage->LoadPlugin(pluginName));
    std::vector<ProfilerPluginConfig> configVec;
    PluginResult result;
    configVec.push_back(config);
    auto it = pluginManage->pluginIds_.find(pluginName);
    const std::vector<uint32_t> pluginIdsVector = {it->second};
    EXPECT_TRUE(pluginManage->CreatePluginSession(configVec));
    EXPECT_FALSE(pluginManage->StartPluginSession(pluginIdsVector, configVec, result));
    ASSERT_GT(configVec.size(), 0);
    configVec[0].set_sample_interval(DEFAULT_SLEEP_TIME);
    int smbFd = InitShareMemory();
    int eventFd = eventfd(0, O_CLOEXEC | O_NONBLOCK);
    EXPECT_TRUE(pluginManage->CreateWriter("hidump-plugin", SMB_SIZE, smbFd, eventFd, true));
    EXPECT_TRUE(pluginManage->StartPluginSession(pluginIdsVector, configVec, result));
    pluginManage->loadedPlugins_.clear();
    auto pluginModule = pluginManage->pluginModules_[it->second];
    const uint32_t loopCount = 10;
    uint32_t loopIndex = 0;
    while (!pluginModule->CheckDataReady() && loopIndex < loopCount) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); //1: sleep 1 seconds
        loopIndex++;
    }
    EXPECT_TRUE(pluginModule->CheckDataReady());
    EXPECT_TRUE(pluginManage->PullState(it->second));
    EXPECT_TRUE(pluginManage->PullResult(it->second));
    EXPECT_TRUE(pluginManage->StopPluginSession(pluginIdsVector));
    EXPECT_TRUE(pluginManage->DestroyPluginSession(pluginIdsVector));
    munmap(g_smbAddr, SMB_SIZE);
    close(smbFd);
    close(eventFd);
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

/**
 * @tc.name: plugin
 * @tc.desc: network-profiler normal loading and StopPluginSession test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, networkprofilerPlugin, TestSize.Level1)
{
    using namespace OHOS::Developtools::Profiler;
    auto pluginManage = std::make_shared<PluginManager>();
    std::string pluginName = "network-profiler";
    ProfilerPluginConfig config;
    const uint8_t configData[] = {0x55, 0xAA, 0x55, 0xAA};
    const std::vector<uint32_t> pluginIdsVector = {2};
    config.set_name(pluginName);
    config.set_config_data((const void*)configData, 4);
    config.set_sample_interval(DEFAULT_SLEEP_TIME);
    std::shared_ptr<NetworkProfilerManager> networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    networkProfilerMgr->Init();
    pluginManage->AddNetworkProfilerManager(networkProfilerMgr);
    std::vector<ProfilerPluginConfig> configVec;
    PluginResult result;
    configVec.push_back(config);
    EXPECT_FALSE(pluginManage->LoadPlugin(pluginName));
    pluginManage->AddPlugin(g_testPluginDir + "libnetwork_profiler.z.so");
    EXPECT_FALSE(pluginManage->CreatePluginSession(configVec));
    EXPECT_FALSE(pluginManage->StartPluginSession(pluginIdsVector, configVec, result));
    std::this_thread::sleep_for(TEMP_DELAY);
    networkProfilerMgr->StartNetworkProfiler();
    EXPECT_FALSE(pluginManage->ReportPluginBasicData(pluginIdsVector));
    EXPECT_FALSE(pluginManage->StopPluginSession(pluginIdsVector));
}

/**
 * @tc.name: plugin
 * @tc.desc: Plug-in PullState test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, PullState, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManager>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    EXPECT_TRUE(commandPoller->OnConnect());
    pluginManage->SetCommandPoller(commandPoller);
    std::string pluginName = "memory-plugin";
    EXPECT_FALSE(pluginManage->LoadPlugin(pluginName));
    EXPECT_FALSE(pluginManage->UnloadPlugin(pluginName));
    EXPECT_TRUE(pluginManage->AddPlugin(g_testPluginDir + SUCCESS_PLUGIN_NAME));
    EXPECT_TRUE(pluginManage->LoadPlugin(pluginName));
    std::vector<ProfilerPluginConfig> configVec;
    PluginResult result;
    const uint8_t configData[] = {0x30, 0x01, 0x38, 0x01, 0x42, 0x01, 0x01};
    ProfilerPluginConfig config;
    uint32_t pluginId = pluginManage->pluginIds_[pluginName];
    const std::vector<uint32_t> pluginIdsVector = {pluginId};
    config.set_name(pluginName);
    config.set_config_data((const void*)configData, 7);
    config.set_sample_interval(DEFAULT_SLEEP_TIME);

    configVec.push_back(config);
    EXPECT_TRUE(pluginManage->CreatePluginSession(configVec));
    EXPECT_TRUE(pluginManage->StartPluginSession(pluginIdsVector, configVec, result));
    std::this_thread::sleep_for(TEMP_DELAY);

    EXPECT_TRUE(pluginManage->pluginIds_.size() > 0);
    EXPECT_TRUE(pluginManage->pluginIds_.find(pluginName) != pluginManage->pluginIds_.end());
    EXPECT_TRUE(pluginManage->PullState(pluginId));

    EXPECT_TRUE(pluginManage->pluginModules_.find(pluginId) != pluginManage->pluginModules_.end());
    auto pluginModule = pluginManage->pluginModules_[pluginId];
    EXPECT_TRUE(pluginModule->BindFunctions());
    EXPECT_TRUE(pluginManage->UnloadPlugin(pluginName));
    EXPECT_FALSE(pluginManage->PullState(pluginId));
    EXPECT_FALSE(pluginManage->PullState(100)); // 100 is not in pluginIds_

    EXPECT_TRUE(pluginManage->ResetWriter(pluginId));
    EXPECT_TRUE(pluginManage->RemovePlugin(g_testPluginDir + SUCCESS_PLUGIN_NAME));
    EXPECT_FALSE(pluginManage->ResetWriter(pluginId));
    EXPECT_TRUE(pluginManage->StopAllPluginSession());
}

/**
 * @tc.name: plugin
 * @tc.desc: Plug-in PullResult test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginManagerTest, PullResult, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManager>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    EXPECT_TRUE(commandPoller->OnConnect());
    pluginManage->SetCommandPoller(commandPoller);
    std::string pluginName = "memory-plugin";
    EXPECT_FALSE(pluginManage->LoadPlugin(pluginName));
    EXPECT_FALSE(pluginManage->UnloadPlugin(pluginName));
    EXPECT_TRUE(pluginManage->AddPlugin(g_testPluginDir + SUCCESS_PLUGIN_NAME));
    EXPECT_TRUE(pluginManage->LoadPlugin(pluginName));
    EXPECT_TRUE(pluginManage->pluginIds_.find(pluginName) != pluginManage->pluginIds_.end());
    uint32_t pluginId = pluginManage->pluginIds_[pluginName];
    EXPECT_TRUE(pluginManage->CreateWriter("memory-plugin", DEFAULT_BUFFER_SIZE, -1, -1, true));
    EXPECT_FALSE(pluginManage->CreateWriter("memory-plugin", 0, -1, -1, true));

    EXPECT_TRUE(pluginManage->PullResult(pluginId, true));
    EXPECT_FALSE(pluginManage->PullResult(pluginId, false));
    auto pluginModule = pluginManage->pluginModules_[pluginId];
    EXPECT_TRUE(pluginModule != nullptr);
    std::shared_ptr<BufferWriter> writer =
        std::make_shared<BufferWriter>("test", "1.01", DEFAULT_BUFFER_SIZE, -1, -1, 0);
    pluginModule->RegisterWriter(writer, false);
    EXPECT_TRUE(pluginModule->GetWriter() != nullptr);
    EXPECT_FALSE(pluginManage->PullResult(pluginId, false));

    int smbFd = InitShareMemory();
    int eventFd = eventfd(0, O_CLOEXEC | O_NONBLOCK);
    std::vector<ProfilerPluginConfig> configVec;
    PluginResult result;
    const uint8_t configData[] = {0x30, 0x01, 0x38, 0x01, 0x42, 0x01, 0x01};
    ProfilerPluginConfig config;
    const std::vector<uint32_t> pluginIdsVector = {pluginId};
    config.set_name(pluginName);
    config.set_config_data((const void*)configData, 7);
    config.set_sample_interval(DEFAULT_SLEEP_TIME);
    configVec.push_back(config);
    EXPECT_TRUE(pluginManage->CreateWriter("memory-plugin", SMB_SIZE, smbFd, eventFd, false));
    EXPECT_TRUE(pluginManage->StartPluginSession(pluginIdsVector, configVec, result));
    std::this_thread::sleep_for(std::chrono::seconds(2));  // 2: sleep 2 seconds
    EXPECT_TRUE(pluginManage->PullResult(pluginId, false));
    EXPECT_TRUE(pluginManage->StopPluginSession(pluginIdsVector));
    EXPECT_TRUE(pluginManage->DestroyPluginSession(pluginIdsVector));
    munmap(g_smbAddr, SMB_SIZE);
    close(smbFd);
    close(eventFd);
}
} // namespace
