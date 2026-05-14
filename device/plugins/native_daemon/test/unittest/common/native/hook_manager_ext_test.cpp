/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <gtest/gtest.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include "hook_socket_client.h"
#include "parameters.h"
#include "socket_context.h"
#include "async_stack.h"
#include "command_poller.h"
#include "hook_manager.h"
#include "share_memory_allocator.h"
#include "hook_service.h"
#include "native_memory_profiler_sa_config.h"

using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;
namespace {
const std::string TEST_PROCESS_NAME = "test_process";
const int TEST_PID = 999999;
const int TEST_SMB_PAGES = 1024;
const int TEST_FILTER_SIZE = 512;
const int TEST_MAX_STACK_DEPTH = 20;
const int TEST_MAX_JS_STACK_DEPTH = 10;
const uint32_t TEST_PAGE_BYTES = 4096;
const std::string TEST_NMD_FILE_PATH = "/data/local/tmp/nmd_test_process.txt";

class HookManagerExtTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "1");
        sleep(1);
    }
    
    static void TearDownTestCase()
    {
        OHOS::system::SetParameter("hiviewdfx.hiprofiler.profilerd.start", "0");
        int result = remove(TEST_NMD_FILE_PATH.c_str());
        if (result != 0) {
            std::cout << "Failed to remove file: " << TEST_NMD_FILE_PATH << std::endl;
        }
    }
    
    void SetUp() override
    {
        hookManager_ = std::make_shared<HookManager>();
        ASSERT_TRUE(hookManager_ != nullptr);
        
        commandPoller_ = std::make_shared<CommandPoller>(hookManager_);
        ASSERT_TRUE(commandPoller_ != nullptr);
        ASSERT_TRUE(commandPoller_->OnConnect());
        
        hookManager_->SetCommandPoller(commandPoller_);
    }
    
    void TearDown() override
    {
        hookManager_ = nullptr;
        commandPoller_ = nullptr;
    }
    
protected:
    std::shared_ptr<HookManager> hookManager_;
    std::shared_ptr<CommandPoller> commandPoller_;

    std::shared_ptr<HookManager::HookManagerCtx> CreateFileCacheCtx(int32_t pid)
    {
        NativeHookConfig config;
        config.set_use_file_cache_mode(true);
        config.set_clock("monotonic");
        hookManager_->SetHookConfig(config);
        hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);
        auto ctx = std::make_shared<HookManager::HookManagerCtx>(pid);
        ctx->stackData = std::make_shared<StackDataRepeater>(8);
        ctx->stackPreprocess = std::make_shared<StackPreprocess>(ctx->stackData, hookManager_->hookConfig_,
            CLOCK_MONOTONIC);
        ctx->stackPreprocess->SetFactory(hookManager_->factory_);
        hookManager_->hookCtx_.push_back(ctx);
        return ctx;
    }

    std::shared_ptr<HookManager::HookManagerCtx> CreateSharedMemoryCtx(int32_t pid)
    {
        NativeHookConfig config;
        config.set_use_file_cache_mode(false);
        config.set_clock("monotonic");
        hookManager_->SetHookConfig(config);
        hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);
        auto ctx = std::make_shared<HookManager::HookManagerCtx>(pid);
        ctx->stackData = std::make_shared<StackDataRepeater>(8);
        ctx->stackPreprocess = std::make_shared<StackPreprocess>(ctx->stackData, hookManager_->hookConfig_,
            CLOCK_MONOTONIC);
        ctx->stackPreprocess->SetFactory(hookManager_->factory_);
        hookManager_->hookCtx_.push_back(ctx);
        return ctx;
    }

    std::shared_ptr<HookManager::HookManagerCtx> CreateNoQueueFileCacheCtx(int32_t pid)
    {
        hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);
        auto ctx = std::make_shared<HookManager::HookManagerCtx>(pid);
        ctx->stackPreprocess = std::make_shared<StackPreprocess>(nullptr, hookManager_->hookConfig_,
            CLOCK_MONOTONIC);
        ctx->stackPreprocess->SetFactory(hookManager_->factory_);
        hookManager_->hookCtx_.push_back(ctx);
        return ctx;
    }

    void AppendSharedMemoryRecord(const std::shared_ptr<HookManager::HookManagerCtx>& ctx, uint16_t type)
    {
        static uint32_t blockIndex = 0;
        ASSERT_TRUE(ctx != nullptr);
        auto block = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(
            "hook_mgr_ext_" + std::to_string(++blockIndex), TEST_PAGE_BYTES);
        ASSERT_TRUE(block != nullptr);

        StackRawData rawData = {};
        rawData.type = type;
        rawData.pid = TEST_PID;
        rawData.tid = TEST_PID;
        rawData.mallocSize = 64;
        rawData.nodeType = 1;
        rawData.nodeId = 1;
        rawData.ts = {1, 1};
        ASSERT_TRUE(block->PutRaw(reinterpret_cast<const int8_t*>(&rawData), sizeof(rawData)));
        ctx->shareMemoryBlockList.push_back(block);
    }

    int CreateCacheFileWithRecord(uint16_t type)
    {
        char tempPath[] = "/data/local/tmp/cache_record_XXXXXX";
        int fd = mkstemp(tempPath);
        EXPECT_GE(fd, 0);
        EXPECT_EQ(unlink(tempPath), 0);
        if (fd < 0) {
            return -1;
        }

        StackRawData rawData = {};
        rawData.type = type;
        rawData.pid = TEST_PID;
        rawData.tid = TEST_PID;
        rawData.mallocSize = 64;
        rawData.nodeType = 1;
        rawData.nodeId = 1;
        rawData.ts = {1, 1};
        uint32_t totalSize = sizeof(rawData);
        EXPECT_EQ(write(fd, &totalSize, sizeof(totalSize)), static_cast<ssize_t>(sizeof(totalSize)));
        EXPECT_EQ(write(fd, &rawData, sizeof(rawData)), static_cast<ssize_t>(sizeof(rawData)));
        EXPECT_EQ(lseek(fd, 0, SEEK_SET), 0);
        return fd;
    }
};

/*
 * @tc.name: UpdateAsyncFlagConfigDisabled
 * @tc.desc: test UpdateAsyncFlagConfig when async_stack_enable is false.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateAsyncFlagConfigDisabled, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(false);
    hookManager_->SetHookConfig(config);

    hookManager_->UpdateAsyncFlagConfig();
    
    // asyncFlagMask should remain 0 when async_stack_enable is false
    EXPECT_EQ(hookManager_->asyncFlagMask_, 0ULL);
}

/*
 * @tc.name: UpdateAsyncFlagConfigWithTypes
 * @tc.desc: test UpdateAsyncFlagConfig with async types.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateAsyncFlagConfigWithTypes, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    config.add_async_type(AsyncFlagType::LIBUV_SEND);
    config.add_async_type(AsyncFlagType::FFRT_POOL);
    hookManager_->SetHookConfig(config);
    
    hookManager_->UpdateAsyncFlagConfig();
    
    // asyncFlagMask should include the specified types
    EXPECT_TRUE(hookManager_->asyncFlagMask_ & ASYNC_TYPE_LIBUV_SEND);
    EXPECT_TRUE(hookManager_->asyncFlagMask_ & ASYNC_TYPE_FFRT_POOL);
}

/*
 * @tc.name: UpdateAsyncFlagConfigAllTypes
 * @tc.desc: test UpdateAsyncFlagConfig with ALL_ASYNC_TYPE.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateAsyncFlagConfigAllTypes, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    config.add_async_type(AsyncFlagType::ALL_ASYNC_TYPE);
    hookManager_->SetHookConfig(config);
    
    hookManager_->UpdateAsyncFlagConfig();
    
    // asyncFlagMask should include all async types
    EXPECT_TRUE(hookManager_->asyncFlagMask_ != 0ULL);
}

/*
 * @tc.name: UpdateFileCacheModeConfigSupportsSystemProcess
 * @tc.desc: test file cache mode keeps enabled for non-application process.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateFileCacheModeConfigSupportsSystemProcess, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_process_name("native_daemon");
    config.set_file_cache_buffer_size(MIN_FILE_CACHE_MEMORY_SIZE);
    hookManager_->SetHookConfig(config);

    hookManager_->UpdateFileCacheModeConfig();

    EXPECT_TRUE(hookManager_->hookConfig_.use_file_cache_mode());
    EXPECT_EQ(hookManager_->hookConfig_.file_cache_buffer_size(), MIN_FILE_CACHE_MEMORY_SIZE);
}

/*
 * @tc.name: CreateFileCacheFd
 * @tc.desc: test CreateFileCacheFd creates anonymous writable fd.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CreateFileCacheFd, TestSize.Level0)
{
    hookManager_->fileCacheDir_ = "/data/local/tmp/";
    int fd = hookManager_->CreateFileCacheFd();
    ASSERT_GE(fd, 0);

    const char* content = "cache-fd";
    ASSERT_GT(write(fd, content, strlen(content)), 0);
    ASSERT_EQ(lseek(fd, 0, SEEK_SET), 0);
    char buffer[16] = {0};
    ASSERT_GT(read(fd, buffer, sizeof(buffer)), 0);
    EXPECT_STREQ(buffer, content);
    close(fd);
}

/*
 * @tc.name: HandleReadyCacheFileKeepsPendingStackData
 * @tc.desc: test HandleReadyCacheFile does not close pending stack data on final cache file.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleReadyCacheFileKeepsPendingStackData, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ASSERT_TRUE(ctx->stackData->PutRawStack(std::make_shared<HookRecord>(), true));
    EXPECT_EQ(ctx->stackData->Size(), 1u);

    ctx->cacheFileStopReceived = true;
    ctx->PushBackFdQueue(-1);
    hookManager_->HandleReadyCacheFile(ctx);

    EXPECT_EQ(ctx->GetFdQueueSize(), 0u);
    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
    EXPECT_FALSE(ctx->stackData->closed_);
    EXPECT_EQ(ctx->stackData->Size(), 1u);
}

/*
 * @tc.name: HandleReadyCacheFileIgnoresNullAndEmptyQueue
 * @tc.desc: test HandleReadyCacheFile returns directly for null context or empty fd queue.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleReadyCacheFileIgnoresNullAndEmptyQueue, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    hookManager_->HandleReadyCacheFile(nullptr);
    hookManager_->HandleReadyCacheFile(ctx);

    EXPECT_EQ(ctx->GetFdQueueSize(), 0u);
    EXPECT_FALSE(ctx->cacheFileParsing);
    EXPECT_FALSE(ctx->stackPreprocess->isCacheFileParseDone_);
}

/*
 * @tc.name: FindHookContextSkipsNullEntriesAndMatchesName
 * @tc.desc: test FindHookContext skips null entries and matches pid or process name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FindHookContextSkipsNullEntriesAndMatchesName, TestSize.Level0)
{
    hookManager_->hookCtx_.clear();
    auto nameCtx = std::make_shared<HookManager::HookManagerCtx>(TEST_PROCESS_NAME);
    auto pidCtx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    ASSERT_TRUE(nameCtx != nullptr);
    ASSERT_TRUE(pidCtx != nullptr);
    hookManager_->hookCtx_.push_back(nullptr);
    hookManager_->hookCtx_.push_back(nameCtx);
    hookManager_->hookCtx_.push_back(pidCtx);

    EXPECT_EQ(hookManager_->FindHookContext(TEST_PID, ""), pidCtx);
    EXPECT_EQ(hookManager_->FindHookContext(-1, TEST_PROCESS_NAME), nameCtx);
    EXPECT_EQ(hookManager_->FindHookContext(TEST_PID + 1, "missing"), nullptr);

    hookManager_->hookCtx_.clear();
    hookManager_->hookCtx_.push_back(nameCtx);
    hookManager_->hookCtx_.push_back(pidCtx);
}

/*
 * @tc.name: ReadFromCacheFileFlushesPendingBatch
 * @tc.desc: test ReadFromCacheFile flushes the final partial record batch into stackData.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, ReadFromCacheFileFlushesPendingBatch, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    int fd = CreateCacheFileWithRecord(MALLOC_MSG);
    ASSERT_GE(fd, 0);

    hookManager_->ReadFromCacheFile(ctx, fd);

    EXPECT_EQ(ctx->stackData->Size(), 1u);
}

/*
 * @tc.name: FinishFileCacheStopWaitsForActiveParse
 * @tc.desc: test FinishFileCacheStopIfDrained does not finish while current cache file is still parsing.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinishFileCacheStopWaitsForActiveParse, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    ctx->cacheFileStopReceived = true;
    ctx->cacheFileParsing = true;
    hookManager_->FinishFileCacheStopIfDrained(ctx);

    EXPECT_FALSE(ctx->stackPreprocess->isCacheFileParseDone_);

    ctx->cacheFileParsing = false;
    hookManager_->FinishFileCacheStopIfDrained(ctx);

    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
}

/*
 * @tc.name: HandleCacheFileStopByPidKeepsPendingStackData
 * @tc.desc: test stop callback does not clear pending stack data when fd queue is empty.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleCacheFileStopByPidKeepsPendingStackData, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ASSERT_TRUE(ctx->stackData->PutRawStack(std::make_shared<HookRecord>(), true));
    EXPECT_EQ(ctx->stackData->Size(), 1u);

    EXPECT_TRUE(hookManager_->HandleCacheFileStopByPid(TEST_PID));

    EXPECT_TRUE(ctx->cacheFileStopReceived);
    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
    EXPECT_FALSE(ctx->stackData->closed_);
    EXPECT_EQ(ctx->stackData->Size(), 1u);
}

/*
 * @tc.name: StopTargetProcessWithoutSocketUsesImmediateLocalExit
 * @tc.desc: test shared memory mode marks producer done when control socket is unavailable.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, StopTargetProcessWithoutSocketUsesImmediateLocalExit, TestSize.Level0)
{
    auto ctx = CreateSharedMemoryCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ctx->stackPreprocess->SetIsStartTakeData(true);
    AppendSharedMemoryRecord(ctx, MALLOC_MSG);

    hookManager_->StopTargetProcess(ctx);

    EXPECT_TRUE(ctx->stackPreprocess->IsDataSourceTerminated());
    EXPECT_TRUE(ctx->stackData->IsProducerFinished());
    EXPECT_FALSE(ctx->stackPreprocess->isStopTakeData_);
    EXPECT_EQ(ctx->stackData->Size(), 1u);
}

/*
 * @tc.name: WaitFinalSnapshotIfNeededTriggersCallback
 * @tc.desc: test WaitFinalSnapshotIfNeeded sends final snapshot request when sampling is enabled.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, WaitFinalSnapshotIfNeededTriggersCallback, TestSize.Level0)
{
    auto ctx = CreateSharedMemoryCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    hookManager_->hookConfig_.set_allocator_sampling_enable(true);
    uint32_t requestCmd = 0;
    ctx->stackPreprocess->SetServerCommandCallback([&ctx, &requestCmd](uint32_t cmd) {
        requestCmd = cmd;
        ctx->stackPreprocess->finalSnapshotDone_ = true;
        ctx->stackPreprocess->snapshotCv_.notify_one();
    });

    hookManager_->WaitFinalSnapshotIfNeeded(ctx);

    EXPECT_EQ(requestCmd, FINAL_SNAPSHOT_REQUEST_CMD);
}

/*
 * @tc.name: HandleCacheFileClientDisconnectedByPidDrainsCurrentFiles
 * @tc.desc: test no-queue file cache mode exits after parsing queued files when client is disconnected.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleClientDisconnectedByPidDrainsCurrentFiles, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    hookManager_->noDataQueue_ = true;

    auto ctx = CreateNoQueueFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    char tempPath[] = "/data/local/tmp/file_cache_stop_XXXXXX";
    int fd = mkstemp(tempPath);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(unlink(tempPath), 0);
    ctx->PushBackFdQueue(fd);

    EXPECT_TRUE(hookManager_->HandleClientDisconnectedByPid(TEST_PID));
    EXPECT_TRUE(ctx->cacheFileClientDisconnected);
    EXPECT_FALSE(ctx->stackPreprocess->isStopTakeData_);

    hookManager_->HandleReadyCacheFile(ctx);

    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
    EXPECT_TRUE(ctx->stackPreprocess->isStopTakeData_);
    EXPECT_EQ(ctx->GetFdQueueSize(), 0u);
}

/*
 * @tc.name: HandleReadyCacheFileWithEndMsgStopsImmediately
 * @tc.desc: test file cache END_MSG stops parsing immediately and remaining queued files stay untouched.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleReadyCacheFileWithEndMsgStopsImmediately, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    hookManager_->noDataQueue_ = true;

    auto ctx = CreateNoQueueFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    int endFd = CreateCacheFileWithRecord(END_MSG);
    int mallocFd = CreateCacheFileWithRecord(MALLOC_MSG);
    ASSERT_GE(endFd, 0);
    ASSERT_GE(mallocFd, 0);
    ctx->PushBackFdQueue(endFd);
    ctx->PushBackFdQueue(mallocFd);

    hookManager_->HandleReadyCacheFile(ctx);

    EXPECT_TRUE(ctx->stackPreprocess->isStopTakeData_);
    EXPECT_FALSE(ctx->stackPreprocess->IsDataSourceTerminated());
    EXPECT_EQ(ctx->GetFdQueueSize(), 1u);
}

/*
 * @tc.name: HandleClientDisconnectedByPidMarksSharedMemoryProducerDone
 * @tc.desc: test shared memory mode marks producer done when client is disconnected.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleClientDisconnectedByPidMarksSharedMemoryProducerDone, TestSize.Level0)
{
    auto ctx = CreateSharedMemoryCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ASSERT_TRUE(ctx->stackData != nullptr);
    AppendSharedMemoryRecord(ctx, MALLOC_MSG);

    EXPECT_TRUE(hookManager_->HandleClientDisconnectedByPid(TEST_PID));

    EXPECT_TRUE(ctx->stackPreprocess->IsDataSourceTerminated());
    EXPECT_TRUE(ctx->stackData->IsProducerFinished());
    EXPECT_EQ(ctx->stackData->Size(), 1u);
}

/*
 * @tc.name: FinalizeSharedMemoryTerminationSkipsTerminatedContext
 * @tc.desc: test FinalizeSharedMemoryTermination returns directly when source is already terminated.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinalizeSharedMemoryTerminationSkipsTerminatedContext, TestSize.Level0)
{
    auto ctx = CreateSharedMemoryCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ctx->stackPreprocess->SetDataSourceTerminated(true);

    hookManager_->FinalizeSharedMemoryTermination(ctx);

    EXPECT_FALSE(ctx->stackData->IsProducerFinished());
}

/*
 * @tc.name: FinalizeSharedMemoryTerminationMarksProducerFinished
 * @tc.desc: test FinalizeSharedMemoryTermination marks shared memory source as terminated and producer finished.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinalizeSharedMemoryTerminationMarksProducerFinished, TestSize.Level0)
{
    auto ctx = CreateSharedMemoryCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ASSERT_TRUE(ctx->stackData != nullptr);

    hookManager_->FinalizeSharedMemoryTermination(ctx);

    EXPECT_TRUE(ctx->stackPreprocess->IsDataSourceTerminated());
    EXPECT_TRUE(ctx->stackData->IsProducerFinished());
}

/*
 * @tc.name: DrainPendingSharedMemoryNoQueueSkipsNullBlocks
 * @tc.desc: test DrainPendingSharedMemory uses no-queue path and skips null shared memory blocks.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, DrainPendingSharedMemoryNoQueueSkipsNullBlocks, TestSize.Level0)
{
    auto ctx = CreateSharedMemoryCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    hookManager_->noDataQueue_ = true;
    ctx->shareMemoryBlockList.push_back(nullptr);
    AppendSharedMemoryRecord(ctx, MALLOC_MSG);

    EXPECT_TRUE(hookManager_->DrainPendingSharedMemory(ctx));
}

/*
 * @tc.name: PrepareImmediateStopFileCacheMarksStopReceived
 * @tc.desc: test PrepareImmediateStop routes file cache mode to FinalizeCacheFileStop.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, PrepareImmediateStopFileCacheMarksStopReceived, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    hookManager_->SetHookConfig(config);
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    hookManager_->PrepareImmediateStop(ctx);

    EXPECT_TRUE(ctx->cacheFileStopReceived);
    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
}

/*
 * @tc.name: FinishFileCacheStopIfDrainedStopsNoQueueOnTerminatedInput
 * @tc.desc: test FinishFileCacheStopIfDrained stops no-queue file cache after input termination.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinishFileCacheStopIfDrainedStopsNoQueueOnTerminatedInput, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    hookManager_->SetHookConfig(config);
    hookManager_->noDataQueue_ = true;
    auto ctx = CreateNoQueueFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ctx->stackPreprocess->SetDataSourceTerminated(true);

    hookManager_->FinishFileCacheStopIfDrained(ctx);

    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
    EXPECT_TRUE(ctx->stackPreprocess->isStopTakeData_);
}

/*
 * @tc.name: GetClientConfigBasic
 * @tc.desc: test GetClientConfig with basic configuration.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetClientConfigBasic, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_smb_pages(TEST_SMB_PAGES);
    config.set_filter_size(TEST_FILTER_SIZE);
    config.set_max_stack_depth(TEST_MAX_STACK_DEPTH);
    config.set_max_js_stack_depth(TEST_MAX_JS_STACK_DEPTH);
    config.set_malloc_disable(false);
    config.set_mmap_disable(false);
    config.set_free_stack_report(true);
    config.set_munmap_stack_report(true);
    config.set_fp_unwind(true);
    config.set_blocked(false);
    config.set_memtrace_enable(true);
    config.set_async_stack_enable(true);
    config.set_offline_symbolization(false);
    config.set_response_library_mode(false);
    config.set_js_stack_report(1);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    
    hookManager_->UpdateAsyncFlagConfig();
    
    ClientConfig clientConfig;
    hookManager_->GetClientConfig(clientConfig);
    
    EXPECT_EQ(clientConfig.filterSize, TEST_FILTER_SIZE);
    EXPECT_EQ(clientConfig.maxStackDepth, TEST_MAX_STACK_DEPTH);
    EXPECT_EQ(clientConfig.arktsConfig.maxJsStackDepth, TEST_MAX_JS_STACK_DEPTH);
    EXPECT_FALSE(clientConfig.mallocDisable);
    EXPECT_FALSE(clientConfig.mmapDisable);
    EXPECT_TRUE(clientConfig.freeStackData);
    EXPECT_TRUE(clientConfig.munmapStackData);
    EXPECT_TRUE(clientConfig.fpunwind);
    EXPECT_TRUE(clientConfig.arktsConfig.jsFpunwind);
    EXPECT_FALSE(clientConfig.isBlocked);
    EXPECT_TRUE(clientConfig.memtraceEnable);
    EXPECT_TRUE(clientConfig.asyncStackEnable);
    EXPECT_FALSE(clientConfig.offlineSymbolization);
    EXPECT_FALSE(clientConfig.responseLibraryMode);
    EXPECT_TRUE(clientConfig.arktsConfig.jsStackReport);
}

/*
 * @tc.name: SetHookConfigFromSaConfig
 * @tc.desc: test SetHookConfig with NativeMemoryProfilerSaConfig.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetHookConfigFromSaConfig, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->processName_ = TEST_PROCESS_NAME;
    saConfig->filterSize_ = TEST_FILTER_SIZE;
    saConfig->shareMemorySize_ = TEST_SMB_PAGES;
    saConfig->maxStackDepth_ = TEST_MAX_STACK_DEPTH;
    saConfig->mallocDisable_ = false;
    saConfig->mmapDisable_ = false;
    saConfig->freeStackData_ = true;
    saConfig->munmapStackData_ = true;
    saConfig->mallocFreeMatchingInterval_ = 1000;
    saConfig->mallocFreeMatchingCnt_ = 5;
    saConfig->stringCompressed_ = true;
    saConfig->fpUnwind_ = true;
    saConfig->blocked_ = false;
    saConfig->recordAccurately_ = true;
    saConfig->startupMode_ = false;
    saConfig->memtraceEnable_ = true;
    saConfig->asyncStackEnable_ = true;
    saConfig->traceMask_ = 0x12345678;
    saConfig->offlineSymbolization_ = false;
    saConfig->callframeCompress_ = true;
    saConfig->statisticsInterval_ = 30;
    saConfig->clockId_ = CLOCK_MONOTONIC;
    saConfig->sampleInterval_ = 100;
    saConfig->responseLibraryMode_ = false;
    saConfig->jsStackReport_ = 1;
    saConfig->maxJsStackDepth_ = TEST_MAX_JS_STACK_DEPTH;
    saConfig->filterNapiName_ = "test_napi";
    saConfig->targetSoName_ = "libtest.so";
    saConfig->filterLibList_ = {"libcustom_filter.so"};
    saConfig->printNmd_ = true;
    saConfig->hookstandalone_ = false;
    saConfig->saveFile_ = false;
    saConfig->fileName_ = "test.htrace";
    saConfig->largestSize_ = 1024;
    saConfig->secondLargestSize_ = 512;
    saConfig->maxGrowthSize_ = 2048;
    
    hookManager_->SetHookConfig(saConfig);
    
    const auto& config = hookManager_->GetHookConfig();
    EXPECT_EQ(config.pid(), TEST_PID);
    EXPECT_EQ(config.process_name(), TEST_PROCESS_NAME);
    EXPECT_EQ(config.filter_size(), TEST_FILTER_SIZE);
    EXPECT_EQ(config.smb_pages(), TEST_SMB_PAGES);
    EXPECT_EQ(config.max_stack_depth(), TEST_MAX_STACK_DEPTH);
    EXPECT_FALSE(config.malloc_disable());
    EXPECT_FALSE(config.mmap_disable());
    EXPECT_TRUE(config.free_stack_report());
    EXPECT_TRUE(config.munmap_stack_report());
    EXPECT_EQ(config.malloc_free_matching_interval(), 1000);
    EXPECT_EQ(config.malloc_free_matching_cnt(), 5);
    EXPECT_TRUE(config.string_compressed());
    EXPECT_TRUE(config.fp_unwind());
    EXPECT_FALSE(config.blocked());
    EXPECT_TRUE(config.record_accurately());
    EXPECT_FALSE(config.startup_mode());
    EXPECT_TRUE(config.memtrace_enable());
    EXPECT_TRUE(config.async_stack_enable());
    EXPECT_FALSE(config.offline_symbolization());
    EXPECT_TRUE(config.callframe_compress());
    EXPECT_EQ(config.statistics_interval(), 30);
    EXPECT_EQ(config.sample_interval(), 100);
    EXPECT_FALSE(config.response_library_mode());
    EXPECT_EQ(config.js_stack_report(), 1);
    EXPECT_EQ(config.max_js_stack_depth(), TEST_MAX_JS_STACK_DEPTH);
    EXPECT_EQ(config.filter_napi_name(), "test_napi");
    EXPECT_EQ(config.target_so_name(), "libtest.so");
    ASSERT_EQ(config.filter_lib_list_size(), 1);
    EXPECT_EQ(config.filter_lib_list(0), "libcustom_filter.so");
}

/*
 * @tc.name: CreateWriterAndReset
 * @tc.desc: test CreateWriter and ResetWriter methods.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CreateWriterAndReset, TestSize.Level0)
{
    const std::string pluginName = "test_plugin";
    const uint32_t bufferSize = 1024 * 1024;
    const int smbFd = 1;
    const int eventFd = 2;
    
    EXPECT_TRUE(hookManager_->CreateWriter(pluginName, bufferSize, smbFd, eventFd, true));
    EXPECT_TRUE(hookManager_->ResetWriter(0));
}

/*
 * @tc.name: RegisterFdForSa
 * @tc.desc: test RegisterFdForSa method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, RegisterFdForSa, TestSize.Level0)
{
    const uint32_t saFd = 10;
    hookManager_->RegisterFdForSa(saFd);
    
    // Check if saFd_ was set correctly
    EXPECT_EQ(hookManager_->saFd_, saFd);
}

/*
 * @tc.name: SetSaMode
 * @tc.desc: test SetSaMode method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetSaMode, TestSize.Level0)
{
    hookManager_->SetSaMode(true);
    EXPECT_TRUE(hookManager_->saMode_);
    
    hookManager_->SetSaMode(false);
    EXPECT_FALSE(hookManager_->saMode_);
}

/*
 * @tc.name: SetHookStandalone
 * @tc.desc: test SethookStandalone method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetHookStandalone, TestSize.Level0)
{
    hookManager_->SethookStandalone(true);
    EXPECT_TRUE(hookManager_->isHookStandalone_);
    
    hookManager_->SethookStandalone(false);
    EXPECT_FALSE(hookManager_->isHookStandalone_);
}

/*
 * @tc.name: SetNmdInfo
 * @tc.desc: test SetNmdInfo method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetNmdInfo, TestSize.Level0)
{
    const uint32_t fd = 5;
    const uint32_t type = 1;
    hookManager_->SetNmdInfo(std::make_pair(fd, type));
    
    // Check if nmdParamInfo_ was set correctly
    EXPECT_EQ(hookManager_->nmdParamInfo_.fd, fd);
    EXPECT_EQ(hookManager_->nmdParamInfo_.type, type);
}

/*
 * @tc.name: DumpNmdInfo
 * @tc.desc: test DumpNmdInfo method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, DumpNmdInfo, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    
    const char* nmdResult = "jemalloc statistics:\nAllocated: 1024 bytes\n";
    hookManager_->DumpNmdInfo(nmdResult);
    
    // Check if the file was created and contains the expected content
    std::ifstream file(TEST_NMD_FILE_PATH);
    EXPECT_TRUE(file.is_open());
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    
    EXPECT_NE(content.find("jemalloc statistics:"), std::string::npos);
    EXPECT_NE(content.find("Allocated: 1024 bytes"), std::string::npos);
    
    // Clean up
    remove(TEST_NMD_FILE_PATH.c_str());
}

/*
 * @tc.name: CheckProcessWithProcessName
 * @tc.desc: test CheckProcess with process name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CheckProcessWithProcessName, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.set_startup_mode(true);
    hookManager_->SetHookConfig(config);
    
    // This should return true even if the process doesn't exist
    // because startup_mode is true
    EXPECT_TRUE(hookManager_->CheckProcess());

    // Reset startup param
    hookManager_->ResetStartupParam();
}

/*
 * @tc.name: CheckProcessRejectsMissingPid
 * @tc.desc: test CheckProcess returns false when target pid does not exist.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CheckProcessRejectsMissingPid, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_pid(2000000000);
    hookManager_->SetHookConfig(config);

    EXPECT_FALSE(hookManager_->CheckProcess());
}

/*
 * @tc.name: CheckProcessRejectsTooManyResponseLibraryContexts
 * @tc.desc: test CheckProcess rejects response library mode with more than eight contexts.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CheckProcessRejectsTooManyResponseLibraryContexts, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_response_library_mode(true);
    hookManager_->SetHookConfig(config);
    for (int i = 0; i < 9; ++i) {
        hookManager_->hookCtx_.push_back(std::make_shared<HookManager::HookManagerCtx>(TEST_PID + i));
    }

    EXPECT_FALSE(hookManager_->CheckProcess());
}

/*
 * @tc.name: CheckProcessEnablesProtobufSerializeForMultiContext
 * @tc.desc: test CheckProcess enables protobuf serialization when multiple contexts exist.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CheckProcessEnablesProtobufSerializeForMultiContext, TestSize.Level0)
{
    NativeHookConfig config;
    hookManager_->SetHookConfig(config);
    hookManager_->hookCtx_.push_back(std::make_shared<HookManager::HookManagerCtx>(TEST_PID));
    hookManager_->hookCtx_.push_back(std::make_shared<HookManager::HookManagerCtx>(TEST_PID + 1));

    EXPECT_TRUE(hookManager_->CheckProcess());
    EXPECT_TRUE(hookManager_->isProtobufSerialize_);
}

/*
 * @tc.name: MergeAsyncFlagMaskFromEnum
 * @tc.desc: test MergeAsyncFlagMaskFromEnum method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, MergeAsyncFlagMaskFromEnum, TestSize.Level0)
{
    uint64_t baseMask = 0;
    
    NativeHookConfig config;
    config.add_async_type(AsyncFlagType::LIBUV_SEND);
    config.add_async_type(AsyncFlagType::PROMISE);
    hookManager_->SetHookConfig(config);
    
    uint64_t result = hookManager_->MergeAsyncFlagMaskFromEnum(baseMask);
    EXPECT_TRUE(result & ASYNC_TYPE_LIBUV_SEND);
    EXPECT_TRUE(result & ASYNC_TYPE_PROMISE);
}

/*
 * @tc.name: MergeAsyncFlagMaskFromEnumCoversRemainingValues
 * @tc.desc: test MergeAsyncFlagMaskFromEnum covers remaining enum branches.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, MergeAsyncFlagMaskFromEnumCoversRemainingValues, TestSize.Level0)
{
    NativeHookConfig config;
    config.add_async_type(AsyncFlagType::FFRT_QUEUE);
    config.add_async_type(AsyncFlagType::EVENTHANDLER);
    config.add_async_type(AsyncFlagType::CUSTOMIZE);
    config.add_async_type(AsyncFlagType::ARKTS_WORKER);
    config.add_async_type(AsyncFlagType::ARKTS_TASKPOOL);
    config.add_async_type(AsyncFlagType::ARKWEB_ENGINE);
    config.add_async_type(AsyncFlagType::JSVM_ASYNC);
    config.add_async_type(AsyncFlagType::ALL_ASYNC_NESTED_TYPE);
    config.add_async_type(static_cast<AsyncFlagType>(999));
    hookManager_->SetHookConfig(config);

    uint64_t result = hookManager_->MergeAsyncFlagMaskFromEnum(0);
    EXPECT_TRUE(result & ASYNC_TYPE_FFRT_QUEUE);
    EXPECT_TRUE(result & ASYNC_TYPE_EVENTHANDLER);
    EXPECT_TRUE(result & ASYNC_TYPE_CUSTOMIZE);
    EXPECT_TRUE(result & ASYNC_TYPE_ARKTS_WORKER);
    EXPECT_TRUE(result & ASYNC_TYPE_ARKTS_TASKPOOL);
    EXPECT_TRUE(result & ASYNC_TYPE_ARKWEB);
    EXPECT_TRUE(result & ASYNC_TYPE_JSVM);
    EXPECT_TRUE(result & ASYNC_TYPE_LIBUV_SEND);
    EXPECT_TRUE(result & ASYNC_TYPE_FFRT_POOL);
    EXPECT_TRUE(result & ASYNC_TYPE_EVENTHANDLER);
    EXPECT_TRUE(result & ASYNC_TYPE_PROFILER);
}

/*
 * @tc.name: UpdateFileCacheModeConfigClampsOutOfRange
 * @tc.desc: test UpdateFileCacheModeConfig clamps invalid buffer sizes.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateFileCacheModeConfigClampsOutOfRange, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_file_cache_buffer_size(MIN_FILE_CACHE_MEMORY_SIZE - 1);
    hookManager_->SetHookConfig(config);
    hookManager_->UpdateFileCacheModeConfig();
    EXPECT_EQ(hookManager_->hookConfig_.file_cache_buffer_size(), MAX_FILE_CACHE_MEMORY_SIZE);

    config.set_file_cache_buffer_size(MAX_FILE_CACHE_MEMORY_SIZE + 1);
    hookManager_->SetHookConfig(config);
    hookManager_->UpdateFileCacheModeConfig();
    EXPECT_EQ(hookManager_->hookConfig_.file_cache_buffer_size(), MAX_FILE_CACHE_MEMORY_SIZE);
}

/*
 * @tc.name: UpdateAsyncFlagConfigKeepsExistingMaskWithoutEnumConfig
 * @tc.desc: test UpdateAsyncFlagConfig keeps explicit mask when no enum list is configured.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UpdateAsyncFlagConfigKeepsExistingMaskWithoutEnumConfig, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->asyncFlagMask_ = ASYNC_TYPE_CUSTOMIZE;

    hookManager_->UpdateAsyncFlagConfig();

    EXPECT_EQ(hookManager_->asyncFlagMask_, ASYNC_TYPE_CUSTOMIZE);
}

/*
 * @tc.name: RegisterAgentPluginRejectsEmptyPath
 * @tc.desc: test RegisterAgentPlugin rejects empty path.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, RegisterAgentPluginRejectsEmptyPath, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->RegisterAgentPlugin(""));
}

/*
 * @tc.name: UnregisterAgentPluginRejectsEmptyPath
 * @tc.desc: test UnregisterAgentPlugin rejects empty path.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, UnregisterAgentPluginRejectsEmptyPath, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->UnregisterAgentPlugin(""));
}

/*
 * @tc.name: SetLocalHandleIgnoresNonLocalTags
 * @tc.desc: test SetLocalHandle ignores restrace tags other than local handle.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, SetLocalHandleIgnoresNonLocalTags, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    config.add_restrace_tag(TAG_RES_SO_MASK);
    hookManager_->SetHookConfig(config);

    EXPECT_TRUE(hookManager_->SetLocalHandle("1"));
}

/*
 * @tc.name: GetArktsDiagPathRejectsInvalidPayload
 * @tc.desc: test GetArktsDiagPath rejects malformed preload messages.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetArktsDiagPathRejectsInvalidPayload, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);

    const std::string invalidData = "bad preload data";
    EXPECT_TRUE(hookManager_->GetArktsDiagPath(reinterpret_cast<const int8_t*>(invalidData.data()),
        invalidData.size()).empty());

    const std::string mismatchData = PRELOAD_APP_IDENTIFY + "other.process";
    EXPECT_TRUE(hookManager_->GetArktsDiagPath(reinterpret_cast<const int8_t*>(mismatchData.data()),
        mismatchData.size()).empty());
}

/*
 * @tc.name: CreatePluginSessionEmptyConfig
 * @tc.desc: test CreatePluginSession with empty config.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, CreatePluginSessionEmptyConfig, TestSize.Level0)
{
    std::vector<ProfilerPluginConfig> configVec;
    
    // This should return false because no config is provided
    EXPECT_FALSE(hookManager_->CreatePluginSession(configVec));
}

/*
 * @tc.name: StartPluginSessionEmptyContext
 * @tc.desc: test StartPluginSession with empty hook context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, StartPluginSessionEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {0};
    std::vector<ProfilerPluginConfig> configVec;
    PluginResult result;
    
    // This should return false because hookCtx_ is empty
    EXPECT_FALSE(hookManager_->StartPluginSession(pluginIds, configVec, result));
}

/*
 * @tc.name: StopPluginSessionEmptyContext
 * @tc.desc: test StopPluginSession with empty hook context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, StopPluginSessionEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {0};
    
    // This should return false because hookCtx_ is empty
    EXPECT_FALSE(hookManager_->StopPluginSession(pluginIds));
}

/*
 * @tc.name: DestroyPluginSessionEmptyContext
 * @tc.desc: test DestroyPluginSession with empty hook context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, DestroyPluginSessionEmptyContext, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {0};
    
    // This should return true even with empty hook context
    EXPECT_TRUE(hookManager_->DestroyPluginSession(pluginIds));
}

/*
 * @tc.name: ReportPluginBasicData
 * @tc.desc: test ReportPluginBasicData method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, ReportPluginBasicData, TestSize.Level0)
{
    std::vector<uint32_t> pluginIds = {0};
    
    // This should return true
    EXPECT_TRUE(hookManager_->ReportPluginBasicData(pluginIds));
}

/*
 * @tc.name: ResetStartupParam
 * @tc.desc: test ResetStartupParam method.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, ResetStartupParam, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_startup_mode(true);
    config.set_process_name(TEST_PROCESS_NAME);
    hookManager_->SetHookConfig(config);
    
    // This should not crash
    hookManager_->ResetStartupParam();
}

/*
 * @tc.name: GetCmdArgsWithExpandPids
 * @tc.desc: test GetCmdArgs with expand_pids.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetCmdArgsWithExpandPids, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_pid(TEST_PID);
    config.set_process_name(TEST_PROCESS_NAME);
    config.add_expand_pids(TEST_PID);
    config.add_expand_pids(TEST_PID + 1);
    config.add_expand_pids(TEST_PID + 2);
    
    std::string result = hookManager_->GetCmdArgs(config);
    std::cout << result << std::endl;
    EXPECT_NE(result, "");
    // 进程不存在
    EXPECT_EQ(result.find("pid: " + std::to_string(TEST_PID)), std::string::npos);
    EXPECT_NE(result.find("process_name: " + TEST_PROCESS_NAME), std::string::npos);
    EXPECT_NE(result.find("expand_pids: " + std::to_string(TEST_PID)), std::string::npos);
    EXPECT_NE(result.find("expand_pids: " + std::to_string(TEST_PID + 1)), std::string::npos);
    EXPECT_NE(result.find("expand_pids: " + std::to_string(TEST_PID + 2)), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsWithMallocFreeMatching
 * @tc.desc: test GetCmdArgs with malloc_free_matching parameters.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetCmdArgsWithMallocFreeMatching, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_malloc_free_matching_interval(500);
    config.set_malloc_free_matching_cnt(10);
    
    std::string result = hookManager_->GetCmdArgs(config);
    
    EXPECT_NE(result, "");
    EXPECT_NE(result.find("malloc_free_matching_interval: 500"), std::string::npos);
    EXPECT_NE(result.find("malloc_free_matching_cnt: 10"), std::string::npos);
}

/*
 * @tc.name: GetCmdArgsWithClockAndSample
 * @tc.desc: test GetCmdArgs with clock and sample parameters.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetCmdArgsWithClockAndSample, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_clock("realtime");
    config.set_sample_interval(50);
    
    std::string result = hookManager_->GetCmdArgs(config);
    
    EXPECT_NE(result, "");
    EXPECT_NE(result.find("clock: realtime"), std::string::npos);
    EXPECT_NE(result.find("sample_interval: 50"), std::string::npos);
}


/*
* @tc.name: SetGwpFd
* @tc.desc: test SetGwpFd method.
* @tc.type: FUNC
*/
HWTEST_F(HookManagerExtTest, SetGwpFd, TestSize.Level0)
{
    const uint32_t fd = 8;
    hookManager_->SetGwpFd(fd);
    EXPECT_EQ(hookManager_->gwpFd_, fd);

    hookManager_->SetGwpFd(0);
    EXPECT_EQ(hookManager_->gwpFd_, 0);
}

/*
* @tc.name: SetHookConfigFromSaConfigGwpAsan
* @tc.desc: test SetHookConfig with GWP-ASan fields from SaConfig.
* @tc.type: FUNC
*/
HWTEST_F(HookManagerExtTest, SetHookConfigFromSaConfigGwpAsan, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->gwpAsanHoldTime_ = 30;
    saConfig->gwpAsanMaxEntries_ = 64;
    saConfig->gwpAsanMaxStackDepth_ = 16;

    hookManager_->SetHookConfig(saConfig);

    EXPECT_EQ(hookManager_->gwpAsanHoldTime_, 30);
    EXPECT_EQ(hookManager_->gwpAsanMaxEntries_, 64);
    EXPECT_EQ(hookManager_->gwpAsanMaxStackDepth_, 16);
}

/*
* @tc.name: GetClientConfigGwpAsan
* @tc.desc: test GetClientConfig includes GWP-ASan fields.
* @tc.type: FUNC
*/
HWTEST_F(HookManagerExtTest, GetClientConfigGwpAsan, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->gwpAsanHoldTime_ = 20;
    saConfig->gwpAsanMaxEntries_ = 128;
    saConfig->gwpAsanMaxStackDepth_ = 32;

    hookManager_->SetHookConfig(saConfig);

    ClientConfig clientConfig;
    hookManager_->GetClientConfig(clientConfig);

    EXPECT_EQ(clientConfig.gwpAsanHoldTime, 20);
    EXPECT_EQ(clientConfig.gwpAsanMaxEntries, 128);
    EXPECT_EQ(clientConfig.gwpAsanMaxStackDepth, 32);
}

/*
 * @tc.name: GetClientConfigSkipsInvalidLibraryNames
 * @tc.desc: test GetClientConfig filters invalid target so and library names.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetClientConfigSkipsInvalidLibraryNames, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_target_so_name("bad\"name.so");
    config.add_filter_lib_list("libvalid.so");
    config.add_filter_lib_list("bad\nlib.so");
    config.add_filter_lib_list(std::string(PATH_MAX, 'a'));
    hookManager_->SetHookConfig(config);

    ClientConfig clientConfig;
    hookManager_->GetClientConfig(clientConfig);

    EXPECT_TRUE(clientConfig.targetSoName.empty());
    ASSERT_EQ(clientConfig.filterLibList.size(), 1u);
    EXPECT_EQ(clientConfig.filterLibList[0], "libvalid.so");
}

/*
* @tc.name: SetHookConfigAsyncStackDepth
* @tc.desc: test SetHookConfig with async nesting depth and task stack depth from SaConfig.
* @tc.type: FUNC
*/
HWTEST_F(HookManagerExtTest, SetHookConfigAsyncStackDepth, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->asyncNestingDepth_ = 8;
    saConfig->asyncTaskStackDepth_ = 16;

    hookManager_->SetHookConfig(saConfig);

    const auto& config = hookManager_->GetHookConfig();
    EXPECT_EQ(config.async_nesting_depth(), 8);
    EXPECT_EQ(config.async_task_stack_depth(), 16);
}

/*
* @tc.name: GetClientConfigAsyncStackDepth
* @tc.desc: test GetClientConfig includes async nesting depth and task stack depth.
* @tc.type: FUNC
*/
HWTEST_F(HookManagerExtTest, GetClientConfigAsyncStackDepth, TestSize.Level0)
{
    auto saConfig = std::make_shared<NativeMemoryProfilerSaConfig>();
    saConfig->pid_ = TEST_PID;
    saConfig->asyncNestingDepth_ = 10;
    saConfig->asyncTaskStackDepth_ = 20;

    hookManager_->SetHookConfig(saConfig);

    ClientConfig clientConfig;
    hookManager_->GetClientConfig(clientConfig);

    EXPECT_EQ(clientConfig.asyncNestingDepth, 10);
    EXPECT_EQ(clientConfig.asyncTaskStackDepth, 20);
}

/*
 * @tc.name: HandleHookContextRejectsNullContext
 * @tc.desc: test HandleHookContext rejects nullptr context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleHookContextRejectsNullContext, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->HandleHookContext(nullptr));
}

/*
 * @tc.name: HandleSharedMemoryHookContextRejectsAnonymousContext
 * @tc.desc: test shared memory context creation rejects ctx without pid and process name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleSharedMemoryHookContextRejectsAnonymousContext, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(false);
    config.set_clock("monotonic");
    config.set_smb_pages(TEST_SMB_PAGES);
    hookManager_->SetHookConfig(config);

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(-1);
    ASSERT_TRUE(ctx != nullptr);
    EXPECT_FALSE(hookManager_->HandleSharedMemoryHookContext(ctx, 1));
}

/*
 * @tc.name: HandleHookContextSharedMemoryAsyncStackCreatesUniqueTable
 * @tc.desc: test HandleHookContext initializes shared memory resources and unique stack table for async stack.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleHookContextSharedMemoryAsyncStackCreatesUniqueTable, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(false);
    config.set_clock("monotonic");
    config.set_smb_pages(TEST_SMB_PAGES);
    config.set_async_stack_enable(true);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);
    hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    hookManager_->hookCtx_.push_back(ctx);

    EXPECT_TRUE(hookManager_->HandleHookContext(ctx));
    EXPECT_NE(ctx->stackPreprocess, nullptr);
    EXPECT_EQ(ctx->eventPollerList.size(), 1u);
    EXPECT_NE(hookManager_->uniqueTableBlock_, nullptr);
    EXPECT_TRUE(hookManager_->DestroyPluginSession({0}));
}

/*
 * @tc.name: HandleHookContextFileCacheNoQueueCreatesReadyPoller
 * @tc.desc: test HandleHookContext initializes no-queue file cache handler and ready poller.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleHookContextFileCacheNoQueueCreatesReadyPoller, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);
    hookManager_->noDataQueue_ = true;
    hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    hookManager_->hookCtx_.push_back(ctx);

    EXPECT_TRUE(hookManager_->HandleHookContext(ctx));
    EXPECT_NE(ctx->readyFileNotifier, nullptr);
    EXPECT_NE(ctx->readyFilePoller, nullptr);
    EXPECT_NE(ctx->stackPreprocess, nullptr);
    EXPECT_TRUE(hookManager_->DestroyPluginSession({0}));
}

/*
 * @tc.name: PrepareFileCacheContextMissingContextReturnsFalse
 * @tc.desc: test PrepareFileCacheContext fails when target context is absent.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, PrepareFileCacheContextMissingContextReturnsFalse, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->PrepareFileCacheContext(TEST_PID, TEST_PROCESS_NAME));
}

/*
 * @tc.name: PrepareFileCacheContextResetsFlagsAndAssignsPid
 * @tc.desc: test PrepareFileCacheContext resets file cache state for startup-name context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, PrepareFileCacheContextResetsFlagsAndAssignsPid, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PROCESS_NAME);
    ASSERT_TRUE(ctx != nullptr);
    ctx->stackPreprocess = std::make_shared<StackPreprocess>(nullptr, hookManager_->hookConfig_, CLOCK_MONOTONIC);
    ASSERT_TRUE(ctx->stackPreprocess != nullptr);
    ctx->stackPreprocess->SetFactory(hookManager_->factory_);
    ctx->stackPreprocess->SetIsCacheFileParseDone(true);
    ctx->stackPreprocess->SetDataSourceTerminated(true);
    ctx->cacheFileStopReceived = true;
    ctx->cacheFileStopRequested = true;
    ctx->cacheFileClientDisconnected = true;
    ctx->cacheFileParsing = true;
    hookManager_->hookCtx_.push_back(ctx);

    EXPECT_TRUE(hookManager_->PrepareFileCacheContext(TEST_PID, TEST_PROCESS_NAME));
    EXPECT_EQ(ctx->pid, TEST_PID);
    EXPECT_FALSE(ctx->cacheFileStopReceived);
    EXPECT_FALSE(ctx->cacheFileStopRequested);
    EXPECT_FALSE(ctx->cacheFileClientDisconnected);
    EXPECT_FALSE(ctx->cacheFileParsing);
    EXPECT_FALSE(ctx->stackPreprocess->isCacheFileParseDone_);
    EXPECT_FALSE(ctx->stackPreprocess->IsDataSourceTerminated());
}

/*
 * @tc.name: HandleCachedFileFdByPidWithoutNotifierStillQueuesFd
 * @tc.desc: test HandleCachedFileFdByPid succeeds without ready notifier and queues fd.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleCachedFileFdByPidWithoutNotifierStillQueuesFd, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ctx->readyFileNotifier = nullptr;
    int fd = CreateCacheFileWithRecord(MALLOC_MSG);
    ASSERT_GE(fd, 0);

    EXPECT_TRUE(hookManager_->HandleCachedFileFdByPid(TEST_PID, fd, 0));
    EXPECT_EQ(ctx->GetFdQueueSize(), 1u);
    EXPECT_TRUE(ctx->stackPreprocess->isStartTakeData_);
}

/*
 * @tc.name: HandleClientDisconnectedByPidWithoutStackPreprocessReturnsFalse
 * @tc.desc: test shared memory mode rejects disconnected pid when stackPreprocess is absent.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, HandleClientDisconnectedByPidWithoutStackPreprocessReturnsFalse, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(false);
    hookManager_->SetHookConfig(config);

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    hookManager_->hookCtx_.push_back(ctx);

    EXPECT_FALSE(hookManager_->HandleClientDisconnectedByPid(TEST_PID));
}

/*
 * @tc.name: FinalizeCacheFileStopRejectsInvalidContext
 * @tc.desc: test FinalizeCacheFileStop rejects null or incomplete context.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinalizeCacheFileStopRejectsInvalidContext, TestSize.Level0)
{
    EXPECT_FALSE(hookManager_->FinalizeCacheFileStop(nullptr));

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    EXPECT_FALSE(hookManager_->FinalizeCacheFileStop(ctx));
}

/*
 * @tc.name: FinalizeStoppedContextFlushesAndClosesStackData
 * @tc.desc: test FinalizeStoppedContext flushes statistics and closes stack data.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinalizeStoppedContextFlushesAndClosesStackData, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_clock("monotonic");
    config.set_statistics_interval(1);
    config.set_malloc_free_matching_interval(1);
    hookManager_->SetHookConfig(config);

    auto ctx = CreateSharedMemoryCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ASSERT_TRUE(hookManager_->FinalizeStoppedContext(ctx));
    EXPECT_TRUE(ctx->stackPreprocess->IsDataSourceTerminated());
    EXPECT_TRUE(ctx->stackData->closed_);
}

/*
 * @tc.name: FinalizeStoppedContextRejectsMissingPreprocess
 * @tc.desc: test FinalizeStoppedContext rejects context without stack preprocess.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinalizeStoppedContextRejectsMissingPreprocess, TestSize.Level0)
{
    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    EXPECT_FALSE(hookManager_->FinalizeStoppedContext(ctx));
}

/*
 * @tc.name: StopTargetProcessInvalidPidFinalizesFileCache
 * @tc.desc: test invalid pid in file cache mode finalizes stop state immediately.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, StopTargetProcessInvalidPidFinalizesFileCache, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PROCESS_NAME);
    ASSERT_TRUE(ctx != nullptr);
    ctx->stackPreprocess = std::make_shared<StackPreprocess>(nullptr, hookManager_->hookConfig_, CLOCK_MONOTONIC);
    ASSERT_TRUE(ctx->stackPreprocess != nullptr);
    ctx->stackPreprocess->SetFactory(hookManager_->factory_);

    hookManager_->StopTargetProcess(ctx);

    EXPECT_TRUE(ctx->cacheFileStopReceived);
    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
}

/*
 * @tc.name: StopTargetProcessFileCacheRequestsStopWithoutSocket
 * @tc.desc: test file cache mode marks stop requested and finalizes current queue when socket is absent.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, StopTargetProcessFileCacheRequestsStopWithoutSocket, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    hookManager_->StopTargetProcess(ctx);

    EXPECT_TRUE(ctx->cacheFileStopRequested);
    EXPECT_TRUE(ctx->cacheFileStopReceived);
    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
}

/*
 * @tc.name: RequestCacheFileStopMarksContext
 * @tc.desc: test RequestCacheFileStop marks matching context and query reflects the flag.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, RequestCacheFileStopMarksContext, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ctx->processName = TEST_PROCESS_NAME;

    EXPECT_FALSE(hookManager_->IsCacheFileStopRequested(TEST_PID));
    hookManager_->RequestCacheFileStop(TEST_PID);

    EXPECT_TRUE(ctx->cacheFileStopRequested);
    EXPECT_TRUE(hookManager_->IsCacheFileStopRequested(TEST_PID));
    EXPECT_FALSE(hookManager_->IsCacheFileStopRequested(TEST_PID + 1));
}

/*
 * @tc.name: FinishFileCacheStopIfDrainedNoQueueWaitsForInputEnd
 * @tc.desc: test no-queue file cache mode marks parse done but keeps running until input ends.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinishFileCacheStopIfDrainedNoQueueWaitsForInputEnd, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    hookManager_->noDataQueue_ = true;

    auto ctx = CreateNoQueueFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ctx->cacheFileStopReceived = true;

    hookManager_->FinishFileCacheStopIfDrained(ctx);

    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
    EXPECT_FALSE(ctx->stackPreprocess->isStopTakeData_);
}

/*
 * @tc.name: DrainSharedMemoryBlockTakesNotifier
 * @tc.desc: test DrainSharedMemoryBlock consumes notifier and shared memory data.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, DrainSharedMemoryBlockTakesNotifier, TestSize.Level0)
{
    auto ctx = CreateSharedMemoryCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    hookManager_->hookConfig_.set_smb_pages(TEST_SMB_PAGES);
    hookManager_->hookCtx_.clear();
    ASSERT_TRUE(hookManager_->HandleSharedMemoryHookContext(ctx, 1));
    ASSERT_TRUE(ctx->shareMemoryBlockList[0] != nullptr);
    ASSERT_TRUE(ctx->eventNotifierList[0] != nullptr);

    StackRawData rawData = {};
    rawData.type = MALLOC_MSG;
    rawData.pid = TEST_PID;
    rawData.tid = TEST_PID;
    rawData.mallocSize = 64;
    rawData.nodeType = 1;
    rawData.nodeId = 1;
    rawData.ts = {1, 1};
    ASSERT_TRUE(ctx->shareMemoryBlockList[0]->PutRaw(reinterpret_cast<const int8_t*>(&rawData), sizeof(rawData)));
    ASSERT_TRUE(ctx->eventNotifierList[0]->Post(1));

    EXPECT_TRUE(hookManager_->DrainSharedMemoryBlock(ctx, 0, true));
    EXPECT_EQ(ctx->stackData->Size(), 1u);
}

/*
 * @tc.name: DestroyPluginSessionReleasesSharedMemoryResources
 * @tc.desc: test DestroyPluginSession releases shared memory resources and unique table.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, DestroyPluginSessionReleasesSharedMemoryResources, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(false);
    config.set_clock("monotonic");
    config.set_smb_pages(TEST_SMB_PAGES);
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);
    hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);

    auto ctx = std::make_shared<HookManager::HookManagerCtx>("destroy_plugin_session_ctx");
    ASSERT_TRUE(ctx != nullptr);
    ctx->stackData = std::make_shared<StackDataRepeater>(8);
    ctx->stackPreprocess = std::make_shared<StackPreprocess>(ctx->stackData, hookManager_->hookConfig_,
        CLOCK_MONOTONIC);
    ASSERT_TRUE(ctx->stackPreprocess != nullptr);
    ctx->stackPreprocess->SetFactory(hookManager_->factory_);
    ASSERT_TRUE(hookManager_->HandleSharedMemoryHookContext(ctx, 1));
    hookManager_->hookCtx_.push_back(ctx);
    hookManager_->uniqueTableBlock_ =
        ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal("unique_stack_table", TEST_PAGE_BYTES);

    EXPECT_TRUE(hookManager_->DestroyPluginSession({0}));
    EXPECT_TRUE(ctx->stackData->Size() == 0);
    EXPECT_EQ(hookManager_->uniqueTableBlock_, nullptr);
}

/*
 * @tc.name: DestroyPluginSessionReleasesFileCacheQueue
 * @tc.desc: test DestroyPluginSession stops file cache poller and drains queued fds.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, DestroyPluginSessionReleasesFileCacheQueue, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    hookManager_->SetSaMode(true);
    ASSERT_TRUE(hookManager_->HandleFileCacheHookContext(ctx));

    int fd = CreateCacheFileWithRecord(MALLOC_MSG);
    ASSERT_GE(fd, 0);
    ctx->PushBackFdQueue(fd);

    EXPECT_TRUE(hookManager_->DestroyPluginSession({0}));
    EXPECT_EQ(ctx->GetFdQueueSize(), 0u);
}

/*
 * @tc.name: FinalizeCacheFileStopNoQueueDisconnectedStopsImmediately
 * @tc.desc: test no-queue file cache mode stops immediately after disconnect when queue is already drained.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, FinalizeCacheFileStopNoQueueDisconnectedStopsImmediately, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(true);
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    hookManager_->noDataQueue_ = true;

    auto ctx = CreateNoQueueFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);
    ctx->cacheFileClientDisconnected = true;

    EXPECT_TRUE(hookManager_->FinalizeCacheFileStop(ctx));
    EXPECT_TRUE(ctx->stackPreprocess->isCacheFileParseDone_);
    EXPECT_TRUE(ctx->stackPreprocess->isStopTakeData_);
}

/*
 * @tc.name: StartPluginSessionHandlesInvalidTargetState
 * @tc.desc: test StartPluginSession handles invalid pid and uid lookup failure.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, StartPluginSessionHandlesInvalidTargetState, TestSize.Level0)
{
    constexpr int missingPid = 2000000000;
    NativeHookConfig config;
    config.set_clock("monotonic");
    hookManager_->SetHookConfig(config);
    hookManager_->SetSaMode(true);

    auto missingPidCtx = CreateSharedMemoryCtx(missingPid);
    auto invalidPidCtx = CreateSharedMemoryCtx(-1);
    ASSERT_TRUE(missingPidCtx != nullptr);
    ASSERT_TRUE(invalidPidCtx != nullptr);

    std::unordered_map<pid_t, std::pair<uid_t, gid_t>> pidInfo;
    hookManager_->StartPluginSession(&pidInfo);

    EXPECT_FALSE(missingPidCtx->stackPreprocess->isStopTakeData_);
    EXPECT_TRUE(pidInfo.empty());
    missingPidCtx->stackPreprocess->StopTakeResults();
    invalidPidCtx->stackPreprocess->StopTakeResults();
}

/*
 * @tc.name: GetFdsAssignsPidForProcessNameContext
 * @tc.desc: test GetFds resolves startup-name context and assigns pid.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetFdsAssignsPidForProcessNameContext, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_use_file_cache_mode(false);
    config.set_clock("monotonic");
    config.set_smb_pages(TEST_SMB_PAGES);
    hookManager_->SetHookConfig(config);
    hookManager_->factory_ = std::make_shared<HookRecordFactory>(hookManager_->hookConfig_);

    auto ctx = std::make_shared<HookManager::HookManagerCtx>(TEST_PROCESS_NAME);
    ASSERT_TRUE(ctx != nullptr);
    ctx->stackPreprocess = std::make_shared<StackPreprocess>(nullptr, hookManager_->hookConfig_, CLOCK_MONOTONIC);
    ASSERT_TRUE(ctx->stackPreprocess != nullptr);
    ctx->stackPreprocess->SetFactory(hookManager_->factory_);
    ASSERT_TRUE(hookManager_->HandleSharedMemoryHookContext(ctx, 1));
    hookManager_->hookCtx_.push_back(ctx);

    std::vector<int> fds = hookManager_->GetFds(TEST_PID, TEST_PROCESS_NAME, 1);
    ASSERT_EQ(fds.size(), 2u);
    EXPECT_GE(fds[0], 0);
    EXPECT_GE(fds[1], 0);
    EXPECT_EQ(ctx->pid, TEST_PID);
}

/*
 * @tc.name: RequestCacheFileStopMarksFlag
 * @tc.desc: test RequestCacheFileStop and IsCacheFileStopRequested track stop flag.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, RequestCacheFileStopMarksFlag, TestSize.Level0)
{
    auto ctx = CreateFileCacheCtx(TEST_PID);
    ASSERT_TRUE(ctx != nullptr);

    EXPECT_FALSE(hookManager_->IsCacheFileStopRequested(TEST_PID));
    hookManager_->RequestCacheFileStop(TEST_PID);
    EXPECT_TRUE(hookManager_->IsCacheFileStopRequested(TEST_PID));
}

/*
 * @tc.name: GetFdsWithoutMatchedContextReturnsDefaultFdList
 * @tc.desc: test GetFds returns placeholder fds when no context matches pid or name.
 * @tc.type: FUNC
 */
HWTEST_F(HookManagerExtTest, GetFdsWithoutMatchedContextReturnsDefaultFdList, TestSize.Level0)
{
    std::vector<int> fds = hookManager_->GetFds(TEST_PID, TEST_PROCESS_NAME, 2);
    ASSERT_EQ(fds.size(), 4u);
    EXPECT_EQ(fds[0], -1);
    EXPECT_EQ(fds[1], -1);
    EXPECT_EQ(fds[2], -1);
    EXPECT_EQ(fds[3], -1);
}
} // namespace
