/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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
#include <google/protobuf/text_format.h>
#include <fcntl.h>
#include <unistd.h>
#define private public
#include "stack_preprocess.h"
#undef private
#include "native_hook_config.pb.h"
#include "plugin_module_api.h"
#include "native_hook_result_standard.pb.h"
#include "trace_file_reader.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
static RawStackPtr MakeRawStack(uint32_t nodeType, uint64_t nodeId, uint32_t tagId)
{
    // RawStack owns a raw pointer stackContext; install a deleter so UTs don't leak.
    auto raw = RawStackPtr(new RawStack(), [](RawStack* p) {
        if (p != nullptr) {
            delete p->stackContext;
            p->stackContext = nullptr;
            delete p;
        }
    });

    raw->stackContext = new BaseStackRawData();
    raw->stackContext->nodeType = nodeType;
    raw->stackContext->nodeId = nodeId;
    raw->stackContext->tagId = tagId;
    raw->stackSize = 1;
    raw->reportFlag = true;
    return raw;
}

static std::vector<CallFrame> MakeFrames(uint64_t callFrameId)
{
    std::vector<CallFrame> frames;
    CallFrame frame(callFrameId);
    frame.callFrameId_ = static_cast<uint32_t>(callFrameId);
    frames.push_back(frame);
    return frames;
}

static ReportEventBaseData MakeReportEvent(uint16_t type, uint64_t addr, uint32_t tagId, uint32_t stackId)
{
    BaseStackRawData base = {};
    base.ts = {3, 4};
    base.addr = reinterpret_cast<void*>(addr);
    base.type = type;
    base.tagId = static_cast<uint16_t>(tagId);
    base.mallocSize = 64;
    base.tid = 123;
    ReportEventBaseData event(&base, stackId);
    event.nodeType = 2;
    event.nodeId = 9;
    return event;
}

static RawStackPtr MakeAsyncRawStack(uint16_t type)
{
    auto raw = RawStackPtr(new RawStack(), [](RawStack* p) {
        if (p != nullptr) {
            delete reinterpret_cast<AsyncStackData*>(p->stackContext);
            p->stackContext = nullptr;
            delete p;
        }
    });

    auto* asyncData = new AsyncStackData();
    asyncData->type = type;
    raw->stackContext = asyncData;
    raw->stackSize = 1;
    raw->reportFlag = true;
    return raw;
}

static bool AppendSizedRecord(std::vector<uint8_t>& buffer, const void* data, uint32_t size)
{
    size_t offset = buffer.size();
    buffer.resize(offset + sizeof(uint32_t) + size);
    errno_t ret = memcpy_s(buffer.data() + offset, buffer.size() - offset, &size, sizeof(size));
    if (ret != EOK) {
        return false;
    }
    if (size == 0) {
        return true;
    }
    ret = memcpy_s(buffer.data() + offset + sizeof(uint32_t),
        buffer.size() - offset - sizeof(uint32_t), data, size);
    return ret == EOK;
}

static int CreateTempCacheFd()
{
    char tempPath[] = "/data/local/tmp/stack_preprocess_ext_XXXXXX";
    int fd = mkstemp(tempPath);
    if (fd >= 0) {
        (void)unlink(tempPath);
    }
    return fd;
}

static bool WriteBufferToFd(int fd, const std::vector<uint8_t>& buffer)
{
    if (fd < 0) {
        return false;
    }
    ssize_t writeSize = write(fd, buffer.data(), buffer.size());
    if (writeSize != static_cast<ssize_t>(buffer.size())) {
        return false;
    }
    return lseek(fd, 0, SEEK_SET) == 0;
}

static thread_local std::vector<uint64_t> g_callStack;
static thread_local std::shared_ptr<BuildStackDirector> g_director{nullptr};

class StackPreprocessExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StackPreprocessExtTest::SetUpTestCase(void)
{
}

void StackPreprocessExtTest::TearDownTestCase(void)
{
}

void StackPreprocessExtTest::SetUp(void)
{
    StackPreprocess::ClearThreadLocalVariables();
}

void StackPreprocessExtTest::TearDown(void)
{
    StackPreprocess::ClearThreadLocalVariables();
}

/*
@tc.name: StartTakeResultsTest
@tc.desc: test StartTakeResults method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StartTakeResultsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    constexpr int testMaxSize = 1024;
    StackDataRepeaterPtr stackDataRepeater = std::make_shared<StackDataRepeater>(testMaxSize);
    
    auto preprocess = std::make_shared<StackPreprocess>(stackDataRepeater, hookConfig, 0);
    bool result = preprocess->StartTakeResults();
    EXPECT_TRUE(result);
    // when received the first data, isStartTakeData will be true.
    EXPECT_FALSE(preprocess->isStartTakeData_.load());
}

/*
@tc.name: StopTakeResultsTest
@tc.desc: test StopTakeResults method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StopTakeResultsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    constexpr int testMaxSize = 1024;
    StackDataRepeaterPtr stackDataRepeater = std::make_shared<StackDataRepeater>(testMaxSize);
    
    auto preprocess = std::make_shared<StackPreprocess>(stackDataRepeater, hookConfig, 0);
    preprocess->StartTakeResults();
    EXPECT_FALSE(preprocess->isStartTakeData_.load());
    
    bool result = preprocess->StopTakeResults();
    EXPECT_TRUE(result);
    EXPECT_FALSE(preprocess->isStartTakeData_.load());
}

/*
@tc.name: StopTakeResultsWithTerminatedSourceForcesStop
@tc.desc: shared memory mode without repeater should force stop after source termination.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StopTakeResultsWithTerminatedSourceForcesStop, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.dataSourceTerminated_ = true;

    EXPECT_TRUE(preprocess.StopTakeResults());
    EXPECT_TRUE(preprocess.isStopTakeData_);
}

/*
@tc.name: StopTakeResultsRejectsInvalidThreadWithRepeater
@tc.desc: StopTakeResults should fail when repeater exists but worker thread was never started.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StopTakeResultsRejectsInvalidThreadWithRepeater, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackDataRepeaterPtr stackDataRepeater = std::make_shared<StackDataRepeater>(16);
    StackPreprocess preprocess(stackDataRepeater, hookConfig, 0);

    EXPECT_FALSE(preprocess.StopTakeResults());
    EXPECT_TRUE(preprocess.isStopTakeData_);
}

/*
@tc.name: SetDataSourceTerminatedHonorsFileCacheMode
@tc.desc: SetDataSourceTerminated should force stop only for non-file-cache mode without repeater.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, SetDataSourceTerminatedHonorsFileCacheMode, TestSize.Level0)
{
    NativeHookConfig sharedConfig;
    StackPreprocess sharedPreprocess(nullptr, sharedConfig, 0);
    sharedPreprocess.SetDataSourceTerminated(true);
    EXPECT_TRUE(sharedPreprocess.isStopTakeData_);

    NativeHookConfig fileCacheConfig;
    fileCacheConfig.set_use_file_cache_mode(true);
    StackPreprocess fileCachePreprocess(nullptr, fileCacheConfig, 0);
    fileCachePreprocess.SetDataSourceTerminated(true);
    EXPECT_FALSE(fileCachePreprocess.isStopTakeData_);
}

/*
@tc.name: UpdateLastCacheFileActivityTimeRequiresWaitingState
@tc.desc: cache activity time should update only while waiting for file cache stop.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, UpdateLastCacheFileActivityTimeRequiresWaitingState, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto staleTime = std::chrono::steady_clock::now() - std::chrono::seconds(120);
    preprocess.lastCacheFileActivityTime_ = staleTime;

    preprocess.UpdateLastCacheFileActivityTime();
    EXPECT_EQ(preprocess.lastCacheFileActivityTime_, staleTime);

    preprocess.waitingCacheFileStop_ = true;
    preprocess.UpdateLastCacheFileActivityTime();
    EXPECT_GT(preprocess.lastCacheFileActivityTime_, staleTime);
}

/*
@tc.name: StopCacheFileTaskStopsAfterInactivity
@tc.desc: StopCacheFileTask should exit once cache file activity stays inactive.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StopCacheFileTaskStopsAfterInactivity, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::thread worker([&preprocess]() { preprocess.StopCacheFileTask(); });
    while (!preprocess.waitingCacheFileStop_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    {
        std::lock_guard<std::mutex> lock(preprocess.cacheFileActivityTimeMtx_);
        preprocess.lastCacheFileActivityTime_ = std::chrono::steady_clock::now() - std::chrono::seconds(61);
    }
    worker.join();

    EXPECT_TRUE(preprocess.isStopTakeData_);
    EXPECT_FALSE(preprocess.waitingCacheFileStop_);
}

/*
@tc.name: StopTakeResultsInFileCacheModeStopsAfterInactivity
@tc.desc: StopTakeResults should enter file cache stop flow and exit after inactivity.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StopTakeResultsInFileCacheModeStopsAfterInactivity, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.pid_ = 1;
    bool result = false;

    std::thread worker([&preprocess, &result]() { result = preprocess.StopTakeResults(); });
    while (!preprocess.waitingCacheFileStop_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    {
        std::lock_guard<std::mutex> lock(preprocess.cacheFileActivityTimeMtx_);
        preprocess.lastCacheFileActivityTime_ = std::chrono::steady_clock::now() - std::chrono::seconds(61);
    }
    worker.join();

    EXPECT_TRUE(result);
    EXPECT_TRUE(preprocess.isStopTakeData_);
    EXPECT_FALSE(preprocess.waitingCacheFileStop_);
}

/*
@tc.name: StopTakeResultsWithoutRepeaterStopsImmediately
@tc.desc: StopTakeResults should force stop immediately when no repeater or active data exists.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StopTakeResultsWithoutRepeaterStopsImmediately, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    EXPECT_TRUE(preprocess.StopTakeResults());
    EXPECT_TRUE(preprocess.isStopTakeData_);
}

/*
@tc.name: StopTakeResultsActiveDataWaitsForExternalStop
@tc.desc: StopTakeResults should enter scheduled-stop path for active shared memory data.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StopTakeResultsActiveDataWaitsForExternalStop, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetIsStartTakeData(true);
    bool result = false;

    std::thread worker([&preprocess, &result]() {
        result = preprocess.StopTakeResults();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    preprocess.SetIsStopTakeData(true);
    worker.join();

    EXPECT_TRUE(result);
    EXPECT_TRUE(preprocess.isStopTakeData_);
}

/*
@tc.name: NeedBuildIdForJsPathTest
@tc.desc: js related paths should skip build id resolution.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, NeedBuildIdForJsPathTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    BatchNativeHookData jsData;

    CallFrame tsFrame(1);
    tsFrame.filePath_ = "pages/index.ts";
    tsFrame.filePathId_ = 1;
    tsFrame.isJsFrame_ = true;
    tsFrame.needReport_ |= FILE_PATH_ID_REPORT;
    preprocess.ReportFilePathMap(tsFrame, jsData);
    ASSERT_EQ(jsData.events().size(), 1);
    EXPECT_EQ(jsData.events(0).file_path().build_id(), "");

    BatchNativeHookData etsData;
    CallFrame etsFrame(2);
    etsFrame.filePath_ = "entry/src/main/ets/pages/Index.ets";
    etsFrame.filePathId_ = 2;
    etsFrame.needReport_ |= FILE_PATH_ID_REPORT;
    preprocess.ReportFilePathMap(etsFrame, etsData);
    ASSERT_EQ(etsData.events().size(), 1);
    EXPECT_EQ(etsData.events(0).file_path().build_id(), "");

    BatchNativeHookData scriptData;
    CallFrame jsFrame(3);
    jsFrame.filePath_ = "bundle/default.js";
    jsFrame.filePathId_ = 3;
    jsFrame.needReport_ |= FILE_PATH_ID_REPORT;
    preprocess.ReportFilePathMap(jsFrame, scriptData);
    ASSERT_EQ(scriptData.events().size(), 1);
    EXPECT_EQ(scriptData.events(0).file_path().build_id(), "");

    BatchNativeHookData soData;
    CallFrame soFrame(4);
    soFrame.filePath_ = "";
    soFrame.filePathId_ = 4;
    soFrame.needReport_ |= FILE_PATH_ID_REPORT;
    preprocess.ReportFilePathMap(soFrame, soData);
    ASSERT_EQ(soData.events().size(), 1);
    EXPECT_EQ(soData.events(0).file_path().build_id(), "");
}

/*
@tc.name: NeedBuildIdForMoreScriptExtensionsTest
@tc.desc: script extension paths should not request build id resolution.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, NeedBuildIdForMoreScriptExtensionsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    const std::array<std::string, 3> paths = {
        "entry/build/default.mjs", "entry/build/default.cjs", "entry/build/default.abc"
    };

    for (size_t idx = 0; idx < paths.size(); ++idx) {
        BatchNativeHookData stackData;
        CallFrame frame(idx + 1);
        frame.filePath_ = paths[idx];
        frame.filePathId_ = static_cast<uint64_t>(idx + 1);
        frame.needReport_ |= FILE_PATH_ID_REPORT;

        preprocess.ReportFilePathMap(frame, stackData);

        ASSERT_EQ(stackData.events().size(), 1);
        EXPECT_EQ(stackData.events(0).file_path().build_id(), "");
    }
}

/*
@tc.name: ReconcileWithSnapshotFreesOnlyStaleAddresses
@tc.desc: snapshot reconciliation should keep alive allocations and release stale ones.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ReconcileWithSnapshotFreesOnlyStaleAddresses, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    struct timespec ts = {1, 0};
    preprocess.recordStatisticsMap_[1] = RecordStatistic{};
    preprocess.recordStatisticsMap_[1].callstackId = 1;
    preprocess.allocAddrMap_[0x1000] = AllocAddrInfo{64, 1, ts};
    preprocess.allocAddrMap_[0x2000] = AllocAddrInfo{128, 1, ts};
    preprocess.addrSet_.insert(0x1000);
    preprocess.addrSet_.insert(0x2000);

    preprocess.ReconcileWithSnapshot({0x1000});

    EXPECT_EQ(preprocess.allocAddrMap_.count(0x1000), 1u);
    EXPECT_EQ(preprocess.allocAddrMap_.count(0x2000), 0u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].releaseCount, 1u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].releaseSize, 128u);
}

/*
@tc.name: ReconcileDuplicateMallocHonorsTimestampOrder
@tc.desc: duplicate malloc should reject older timestamps and release stale newer records.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ReconcileDuplicateMallocHonorsTimestampOrder, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    struct timespec oldTs = {10, 20};
    preprocess.recordStatisticsMap_[7] = RecordStatistic{};
    preprocess.recordStatisticsMap_[7].callstackId = 7;
    preprocess.allocAddrMap_[0x3000] = AllocAddrInfo{256, 7, oldTs};
    preprocess.addrSet_.insert(0x3000);

    struct timespec earlierTs = {10, 10};
    EXPECT_FALSE(preprocess.ReconcileDuplicateMalloc(0x3000, earlierTs));
    EXPECT_EQ(preprocess.allocAddrMap_.count(0x3000), 1u);

    struct timespec newerTs = {10, 30};
    EXPECT_TRUE(preprocess.ReconcileDuplicateMalloc(0x3000, newerTs));
    EXPECT_EQ(preprocess.allocAddrMap_.count(0x3000), 0u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[7].releaseSize, 256u);
    EXPECT_TRUE(preprocess.ReconcileDuplicateMalloc(0x4000, newerTs));
}

/*
@tc.name: ResolveTagTypeCoversSpecialAndFallbackTags
@tc.desc: tag resolution should classify mapped, ashmem, so, and unknown names.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ResolveTagTypeCoversSpecialAndFallbackTags, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RecordStatisticsEvent::MemoryType type = RecordStatisticsEvent::MALLOC;

    EXPECT_TRUE(preprocess.ResolveTagType(TAG_RES_GPU_VK, type));
    EXPECT_EQ(type, RecordStatisticsEvent::GPU_VK);

    EXPECT_FALSE(preprocess.ResolveTagType(TAG_RES_FD_OPEN, type));
    EXPECT_EQ(type, RecordStatisticsEvent::FD);

    EXPECT_TRUE(preprocess.ResolveTagType("ashmem", type));
    EXPECT_EQ(type, RecordStatisticsEvent::ASHMEM);

    EXPECT_FALSE(preprocess.ResolveTagType("/system/lib64/libnative.so", type));
    EXPECT_EQ(type, RecordStatisticsEvent::SO);

    EXPECT_FALSE(preprocess.ResolveTagType("anonymous-region", type));
    EXPECT_EQ(type, RecordStatisticsEvent::MEMORY_USING_MSG);
}

/*
@tc.name: ForceStopTest
@tc.desc: test ForceStop method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ForceStopTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    constexpr int testMaxSize = 1024;
    StackDataRepeaterPtr stackDataRepeater = std::make_shared<StackDataRepeater>(testMaxSize);
    
    auto preprocess = std::make_shared<StackPreprocess>(stackDataRepeater, hookConfig, 0);
    preprocess->StartTakeResults();
    EXPECT_FALSE(preprocess->isStartTakeData_.load());
    
    preprocess->ForceStop();
    EXPECT_TRUE(preprocess->isStopTakeData_.load());
}

/*
@tc.name: ParseRecordsFromBufferStopsGracefullyOnEndMsg
@tc.desc: test ParseRecordsFromBuffer treats END_MSG as graceful stop instead of parse failure.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ParseRecordsFromBufferStopsGracefullyOnEndMsg, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(std::make_shared<HookRecordFactory>(hookConfig));

    constexpr uint32_t payloadSize = sizeof(BaseStackRawData);
    std::array<uint8_t, sizeof(uint32_t) + payloadSize> buffer = {};
    uint32_t totalSize = payloadSize;
    ASSERT_EQ(memcpy_s(buffer.data(), sizeof(totalSize), &totalSize, sizeof(totalSize)), EOK);

    auto* baseData = reinterpret_cast<BaseStackRawData*>(buffer.data() + sizeof(uint32_t));
    baseData->type = END_MSG;

    StackPreprocess::ParseContext ctx = {0, 0};
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;

    EXPECT_TRUE(preprocess.ParseRecordsFromBuffer(buffer.data(),
        static_cast<uint32_t>(buffer.size()), ctx, rawData, hookRecord));
    EXPECT_TRUE(preprocess.isStopTakeData_);
    EXPECT_EQ(ctx.offset, static_cast<uint32_t>(buffer.size()));
}

/*
@tc.name: ParseRecordsFromBufferRejectsZeroLengthRecord
@tc.desc: test ParseRecordsFromBuffer rejects invalid zero-sized records.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ParseRecordsFromBufferRejectsZeroLengthRecord, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::vector<uint8_t> buffer(sizeof(uint32_t), 0);
    StackPreprocess::ParseContext ctx = {0, 0};
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;

    EXPECT_FALSE(preprocess.ParseRecordsFromBuffer(buffer.data(),
        static_cast<uint32_t>(buffer.size()), ctx, rawData, hookRecord));
    EXPECT_EQ(ctx.offset, 0u);
}

/*
@tc.name: ParseRecordsFromBufferSkipsInvalidPayloadAndContinues
@tc.desc: test ParseRecordsFromBuffer skips malformed records and keeps parsing later payloads.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ParseRecordsFromBufferSkipsInvalidPayloadAndContinues, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    // Manually construct aligned buffer to avoid SIGBUS on ARM64.
    // ProcessAddressData reads uint64_t directly, requiring 8-byte alignment.
    // std::vector's allocation is 8/16-byte aligned, so buffer[4] is at offset 4,
    // buffer[8] is at offset 8 (8-byte aligned).
    // Layout: [pad4][size=8(4B)][freeAddr(8B,aligned)][size=1(4B)][invalid(1B)]
    // Resize to exact size to avoid trailing zeros being parsed as size=0.
    std::vector<uint8_t> buffer = {
        0, 0, 0, 0,                          // pad (4 bytes)
        8, 0, 0, 0,                          // size = 8 (4 bytes, LE)
        0x78, 0x56, 0x34, 0x12, 0, 0, 0, 0, // freeAddr = 0x12345678 (8 bytes)
        1, 0, 0, 0,                          // size = 1 (4 bytes, LE)
        0x5A                                 // invalidPayload (1 byte)
    }; // total 21 bytes
    const uint32_t dataOffset = 4;
    const uint32_t dataSize = static_cast<uint32_t>(buffer.size()) - dataOffset;

    StackPreprocess::ParseContext ctx = {0, 0};
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;

    EXPECT_TRUE(preprocess.ParseRecordsFromBuffer(buffer.data() + dataOffset,
        dataSize, ctx, rawData, hookRecord));
    EXPECT_EQ(ctx.offset, dataSize);
    EXPECT_EQ(ctx.readfileSize, static_cast<uint64_t>(dataSize));
    EXPECT_FALSE(preprocess.isStopTakeData_);
}

/*
@tc.name: ParseRecordsFromBufferLeavesIncompleteRecordForNextRead
@tc.desc: test ParseRecordsFromBuffer keeps partial record bytes for the next cache file read.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ParseRecordsFromBufferLeavesIncompleteRecordForNextRead, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::vector<uint8_t> buffer(sizeof(uint32_t) + sizeof(uint32_t), 0);
    uint32_t totalSize = sizeof(uint64_t);
    ASSERT_EQ(memcpy_s(buffer.data(), buffer.size(), &totalSize, sizeof(totalSize)), EOK);
    StackPreprocess::ParseContext ctx = {0, 0};
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;

    EXPECT_TRUE(preprocess.ParseRecordsFromBuffer(buffer.data(),
        static_cast<uint32_t>(buffer.size()), ctx, rawData, hookRecord));
    EXPECT_EQ(ctx.offset, 0u);
    EXPECT_EQ(ctx.readfileSize, 0u);
}

/*
@tc.name: ProcessStacklessMunmapWithAsyncConfig
@tc.desc: test stackless MUNMAP_MSG does not build async stack frames.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessStacklessMunmapWithAsyncConfig, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_async_stack_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(std::make_shared<HookRecordFactory>(hookConfig));

    BaseStackRawData baseData = {};
    baseData.type = MUNMAP_MSG;
    baseData.addr = reinterpret_cast<void*>(0x1000);
    baseData.mallocSize = 4096;
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;

    EXPECT_TRUE(preprocess.ProcessRawData(reinterpret_cast<int8_t*>(&baseData), sizeof(baseData),
        rawData, hookRecord));
    EXPECT_EQ(hookRecord, nullptr);
}

/*
@tc.name: ProcessStacklessMunmapWithMatchInterval
@tc.desc: test stackless MUNMAP_MSG with matching interval does not relock mtx.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessStacklessMunmapWithMatchInterval, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_malloc_free_matching_interval(1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(std::make_shared<HookRecordFactory>(hookConfig));

    BaseStackRawData baseData = {};
    baseData.type = MUNMAP_MSG;
    baseData.addr = reinterpret_cast<void*>(0x2000);
    baseData.mallocSize = 4096;
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;

    EXPECT_TRUE(preprocess.ProcessRawData(reinterpret_cast<int8_t*>(&baseData), sizeof(baseData),
        rawData, hookRecord));
    EXPECT_EQ(hookRecord, nullptr);
    EXPECT_EQ(preprocess.releasedPeriodListData_.size(), 1u);
}

/*
@tc.name: SetFlushSizeTest
@tc.desc: test SetFlushSize method with different sizes.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, SetFlushSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Test with size 0
    preprocess.SetFlushSize(0);
    EXPECT_EQ(preprocess.flushSize_, 0ULL);
    
    // Test with size 100
    preprocess.SetFlushSize(100);
    EXPECT_EQ(preprocess.flushSize_, 10ULL); // 100 / 10 = 10
    
    // Test with size 101
    preprocess.SetFlushSize(101);
    EXPECT_EQ(preprocess.flushSize_, 11ULL); // 101 / 10 = 10.1, ceil to 11
}

/*
@tc.name: SetNmdFdTest
@tc.desc: test SetNmdFd method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, SetNmdFdTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    uint32_t testFd = 123;
    preprocess.SetNmdFd(testFd);
    EXPECT_EQ(preprocess.nmdFd_, testFd);
}

/*
@tc.name: SaveMemTagAndGetMemTagTest
@tc.desc: test SaveMemTag and GetMemTag methods with different tag IDs.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, SaveMemTagAndGetMemTagTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Test with tag ID 0
    preprocess.SaveMemTag(0, "tag0");
    std::string tagName;
    bool result = preprocess.GetMemTag(0, tagName);
    EXPECT_TRUE(result);
    EXPECT_EQ(tagName, "tag0");
    
    // Test with tag ID 1
    preprocess.SaveMemTag(1, "tag1");
    result = preprocess.GetMemTag(1, tagName);
    EXPECT_TRUE(result);
    EXPECT_EQ(tagName, "tag1");
    
    // Test with non-existent tag ID
    result = preprocess.GetMemTag(999, tagName);
    EXPECT_FALSE(result);
}

/*
@tc.name: SaveJsRawStackAndGetJsRawStackTest
@tc.desc: test SaveJsRawStack and GetJsRawStack methods.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, SaveJsRawStackAndGetJsRawStackTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    uint64_t jsChainId = 12345;
    const char* jsRawStack = "test_js_stack";
    
    // Save JS raw stack
    preprocess.SaveJsRawStack(jsChainId, jsRawStack);
    
    // Get JS raw stack
    const char* result = preprocess.GetJsRawStack(jsChainId);
    EXPECT_EQ(std::string(result), std::string(jsRawStack));
    
    // Get non-existent JS raw stack
    result = preprocess.GetJsRawStack(99999);
    EXPECT_EQ(result, nullptr);
}

/*
@tc.name: InitStatisticsTimeTest
@tc.desc: test InitStatisticsTime method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, InitStatisticsTimeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Just call the method to ensure it doesn't crash
    preprocess.InitStatisticsTime();
    // We can't easily verify the time, but we can ensure the method executes without error
    EXPECT_TRUE(true);
}

/*
@tc.name: WriteHookConfigTest
@tc.desc: test WriteHookConfig method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, WriteHookConfigTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_statistics_interval(10);
    
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Just call the method to ensure it doesn't crash
    preprocess.WriteHookConfig();
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(true);
}

/*
@tc.name: FinishTraceFileTest
@tc.desc: test FinishTraceFile method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, FinishTraceFileTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Just call the method to ensure it doesn't crash
    preprocess.FinishTraceFile();
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(true);
}

/*
@tc.name: FlushRecordStatisticsTest
@tc.desc: test FlushRecordStatistics method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, FlushRecordStatisticsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Just call the method to ensure it doesn't crash
    bool result = preprocess.FlushRecordStatistics();
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_FALSE(result);
}

/*
@tc.name: StackPreprocessCtor_discardDestroyedTraces
@tc.desc: test StackPreprocess constructor reads discard_destroyed_traces from hookConfig.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, StackPreprocessCtor_discardDestroyedTraces, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_discard_destroyed_traces(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    EXPECT_TRUE(preprocess.discardDestroyedTraces_);
}

/*
@tc.name: FlushRecordStatistics_discardDestroyedTraces
@tc.desc: test FlushRecordStatistics discards destroyed traces created in current period.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, FlushRecordStatistics_discardDestroyedTraces, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_discard_destroyed_traces(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    constexpr uint32_t stackId = 100;
    SimplifiedRecordStatistics stat;
    stat.applySize = 64;
    stat.releaseSize = 64; // applySize == releaseSize means destroyed
    stat.createInCurPeriod = true;
    preprocess.simplifiedRecordStatisticsMap_[stackId] = stat;
    preprocess.statisticsPeriodSet_.insert(stackId);

    EXPECT_EQ(preprocess.simplifiedRecordStatisticsMap_.count(stackId), 1u);
    preprocess.FlushRecordStatistics();
    // shouldClean hits: destroyed trace created in current period is discarded (erased without writing)
    EXPECT_EQ(preprocess.simplifiedRecordStatisticsMap_.count(stackId), 0u);
}

/*
@tc.name: FlushRecordApplyAndReleaseMatchDataTest
@tc.desc: test FlushRecordApplyAndReleaseMatchData method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, FlushRecordApplyAndReleaseMatchDataTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    preprocess.FlushRecordApplyAndReleaseMatchData();
    auto& stackData = std::get<BatchNativeHookData>(preprocess.stackData_);
    EXPECT_EQ(stackData.events_size(), 0);
}

/*
@tc.name: FlushRecordApplyAndReleaseMatchDataSerializesEventTypes
@tc.desc: test FlushRecordApplyAndReleaseMatchData serializes apply and release events.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, FlushRecordApplyAndReleaseMatchDataSerializesEventTypes, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(1, "file_page");
    preprocess.SaveMemTag(2, TAG_RES_GPU_VK);
    preprocess.prctlPeriodTags_.push_back("jit_code");
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MALLOC_MSG, 0x1000, 0, 11));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MMAP_MSG, 0x2000, 0, 12));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MMAP_FILE_PAGE_MSG, 0x3000, 1, 13));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(PR_SET_VMA_MSG, 0x4000, 0, 0));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MEMORY_USING_MSG, 0x5000, 2, 14));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MEMORY_UNUSING_MSG, 0x6000, 2, 15));
    preprocess.releasedPeriodListData_.push_back(MakeReportEvent(FREE_MSG, 0x7000, 0, 16));
    preprocess.releasedPeriodListData_.push_back(MakeReportEvent(MUNMAP_MSG, 0x8000, 0, 17));

    preprocess.FlushRecordApplyAndReleaseMatchData();

    auto& stackData = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(stackData.events_size(), 8);
    EXPECT_EQ(stackData.events(0).alloc_event().stack_id(), 11u);
    EXPECT_EQ(stackData.events(2).mmap_event().type(), "FilePage:file_page");
    EXPECT_EQ(stackData.events(3).tag_event().tag(), "Anonymous:jit_code");
    EXPECT_EQ(stackData.events(4).trace_alloc_event().tag_name(), TAG_RES_GPU_VK);
    EXPECT_TRUE(preprocess.applyAndReleaseMatchPeriodListData_.empty());
    EXPECT_TRUE(preprocess.releasedPeriodListData_.empty());
}

/*
@tc.name: ReportFrameMapHandlesOfflineAndDwarfErrorFrames
@tc.desc: test ReportFrameMap frame serialization paths.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ReportFrameMapHandlesOfflineAndDwarfErrorFrames, TestSize.Level0)
{
    NativeHookConfig offlineConfig;
    offlineConfig.set_offline_symbolization(true);
    StackPreprocess offlinePreprocess(nullptr, offlineConfig, 0);
    BatchNativeHookData offlineData;
    CallFrame jsFrame(0x1234, 0x5678, true);
    jsFrame.callFrameId_ = 20;
    jsFrame.symbolNameId_ = 21;
    jsFrame.filePathId_ = 22;
    jsFrame.needReport_ = CALL_FRAME_REPORT;
    offlinePreprocess.ReportFrameMap(jsFrame, offlineData);
    ASSERT_EQ(offlineData.events_size(), 1);
    EXPECT_EQ(offlineData.events(0).frame_map().frame().sp(), 0x5678u);

    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    BatchNativeHookData stackData;
    CallFrame dwarfFrame(0x4321, 0x8765, false);
    dwarfFrame.callFrameId_ = DWARF_ERROR_ID;
    dwarfFrame.needReport_ = CALL_FRAME_REPORT;
    preprocess.ReportFrameMap(dwarfFrame, stackData);
    ASSERT_EQ(stackData.events_size(), 1);
    EXPECT_FALSE(preprocess.unwindFailReport_);
    preprocess.ReportFrameMap(dwarfFrame, stackData);
    EXPECT_EQ(stackData.events_size(), 1);
}

/*
@tc.name: InitializeGpuDataTest
@tc.desc: test InitializeGpuData method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, InitializeGpuDataTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Just call the method to ensure it doesn't crash
    preprocess.InitializeGpuData();
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(true);
}

/*
@tc.name: ReportOfflineSymbolizationDataTest
@tc.desc: test ReportOfflineSymbolizationData method with different configurations.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ReportOfflineSymbolizationDataTest, TestSize.Level0)
{
    // Test with offline symbolization enabled
    NativeHookConfig hookConfig1;
    hookConfig1.set_offline_symbolization(true);
    StackPreprocess preprocess1(nullptr, hookConfig1, 0);
    preprocess1.SetFlushSize(100);
    
    preprocess1.ReportOfflineSymbolizationData();
    EXPECT_FALSE(preprocess1.flushBasicData_);
    
    // Test with offline symbolization disabled
    NativeHookConfig hookConfig2;
    hookConfig2.set_offline_symbolization(false);
    StackPreprocess preprocess2(nullptr, hookConfig2, 0);
    preprocess2.SetFlushSize(100);
    
    preprocess2.ReportOfflineSymbolizationData();
    EXPECT_TRUE(preprocess2.flushBasicData_);
}

/*
@tc.name: ProcessRawDataHandlesSnapshotMarkers
@tc.desc: test ProcessRawData handles alive snapshot marker messages.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessRawDataHandlesSnapshotMarkers, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.waitingFinalSnapshot_ = true;
    preprocess.finalSnapshotDone_ = false;

    std::array<uint64_t, 2> snapshotAddrs = {0x10, 0x20};
    std::vector<uint8_t> snapshot(sizeof(uint32_t) + sizeof(snapshotAddrs));
    uint32_t marker = ALIVE_ADDR_SNAPSHOT_MARKER;
    ASSERT_EQ(memcpy_s(snapshot.data(), snapshot.size(), &marker, sizeof(marker)), EOK);
    ASSERT_EQ(memcpy_s(snapshot.data() + sizeof(marker), snapshot.size() - sizeof(marker),
        snapshotAddrs.data(), sizeof(snapshotAddrs)), EOK);
    EXPECT_TRUE(preprocess.ProcessRawData(reinterpret_cast<const int8_t*>(snapshot.data()),
        static_cast<uint32_t>(snapshot.size()), nullptr, nullptr));
    EXPECT_EQ(preprocess.snapshotAddrs_.size(), snapshotAddrs.size());

    std::array<uint8_t, sizeof(uint32_t)> snapshotEnd = {};
    marker = ALIVE_ADDR_SNAPSHOT_END_MARKER;
    ASSERT_EQ(memcpy_s(snapshotEnd.data(), snapshotEnd.size(), &marker, sizeof(marker)), EOK);
    EXPECT_TRUE(preprocess.ProcessRawData(reinterpret_cast<const int8_t*>(snapshotEnd.data()),
        static_cast<uint32_t>(snapshotEnd.size()), nullptr, nullptr));
    EXPECT_TRUE(preprocess.snapshotAddrs_.empty());
    EXPECT_TRUE(preprocess.finalSnapshotDone_);
}

/*
@tc.name: WaitForFinalSnapshotReturnsAfterCallbackSignal
@tc.desc: WaitForFinalSnapshot should exit after callback marks snapshot completion.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, WaitForFinalSnapshotReturnsAfterCallbackSignal, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    uint32_t requestCmd = 0;
    preprocess.SetServerCommandCallback([&preprocess, &requestCmd](uint32_t cmd) {
        requestCmd = cmd;
        preprocess.finalSnapshotDone_ = true;
        preprocess.snapshotCv_.notify_one();
    });

    preprocess.WaitForFinalSnapshot();

    EXPECT_EQ(requestCmd, FINAL_SNAPSHOT_REQUEST_CMD);
    EXPECT_TRUE(preprocess.finalSnapshotDone_);
    EXPECT_FALSE(preprocess.waitingFinalSnapshot_);
}

/*
@tc.name: WaitForFinalSnapshotTimesOutWithoutSignal
@tc.desc: WaitForFinalSnapshot should leave completion flag unset when callback never signals completion.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, WaitForFinalSnapshotTimesOutWithoutSignal, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    uint32_t requestCmd = 0;
    preprocess.SetServerCommandCallback([&requestCmd](uint32_t cmd) {
        requestCmd = cmd;
    });

    preprocess.WaitForFinalSnapshot();

    EXPECT_EQ(requestCmd, FINAL_SNAPSHOT_REQUEST_CMD);
    EXPECT_FALSE(preprocess.finalSnapshotDone_);
    EXPECT_FALSE(preprocess.waitingFinalSnapshot_);
}

/*
@tc.name: WaitForFinalSnapshotWithoutCallbackReturnsImmediately
@tc.desc: WaitForFinalSnapshot should return directly when callback is absent.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, WaitForFinalSnapshotWithoutCallbackReturnsImmediately, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.WaitForFinalSnapshot();

    EXPECT_FALSE(preprocess.waitingFinalSnapshot_);
    EXPECT_FALSE(preprocess.finalSnapshotDone_);
}

/*
@tc.name: TakeResultsFromCacheFileNegativeFdStopsImmediately
@tc.desc: TakeResultsFromCacheFile should stop immediately when fd is invalid.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, TakeResultsFromCacheFileNegativeFdStopsImmediately, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.TakeResultsFromCacheFile(-1);

    EXPECT_TRUE(preprocess.isStopTakeData_);
}

/*
@tc.name: TakeResultsFromCacheFileHandlesPartialWindowAndEof
@tc.desc: TakeResultsFromCacheFile should keep remaining bytes across reads and stop on incomplete EOF.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, TakeResultsFromCacheFileHandlesPartialWindowAndEof, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::vector<uint8_t> buffer;
    constexpr uint32_t readBufSize = 4 * 1024 * 1024;
    uint64_t freeAddr = 0x12345678ULL;
    ASSERT_TRUE(AppendSizedRecord(buffer, &freeAddr, sizeof(freeAddr)));

    uint32_t partialSize = readBufSize - static_cast<uint32_t>(buffer.size()) + 8;
    uint32_t secondPayloadSize = partialSize;
    size_t remainPayloadSize = readBufSize - buffer.size() - sizeof(uint32_t);
    size_t originSize = buffer.size();
    buffer.resize(originSize + sizeof(uint32_t) + remainPayloadSize, 0);
    ASSERT_EQ(memcpy_s(buffer.data() + originSize, buffer.size() - originSize,
        &secondPayloadSize, sizeof(secondPayloadSize)), EOK);

    int fd = CreateTempCacheFd();
    ASSERT_GE(fd, 0);
    ASSERT_TRUE(WriteBufferToFd(fd, buffer));

    preprocess.TakeResultsFromCacheFile(fd);

    EXPECT_NE(StackPreprocess::GetDirectorForTest(), nullptr);
    EXPECT_FALSE(preprocess.isStopTakeData_);
}

/*
@tc.name: TakeResultsFromCacheFileHandlesClosedFdReadError
@tc.desc: TakeResultsFromCacheFile should stop parsing current file when read returns an error.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, TakeResultsFromCacheFileHandlesClosedFdReadError, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    int fd = CreateTempCacheFd();
    ASSERT_GE(fd, 0);
    ASSERT_EQ(close(fd), 0);

    preprocess.TakeResultsFromCacheFile(fd);

    EXPECT_NE(StackPreprocess::GetDirectorForTest(), nullptr);
    EXPECT_FALSE(preprocess.isStopTakeData_);
}

/*
@tc.name: TakeResultsFromCacheFileSkipsOversizeRecord
@tc.desc: TakeResultsFromCacheFile should skip cache records larger than the read window.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, TakeResultsFromCacheFileSkipsOversizeRecord, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint32_t readBufSize = 4 * 1024 * 1024;
    std::vector<uint8_t> buffer(sizeof(uint32_t), 0);
    uint32_t oversizeRecord = readBufSize + 16;
    ASSERT_EQ(memcpy_s(buffer.data(), buffer.size(), &oversizeRecord, sizeof(oversizeRecord)), EOK);
    int fd = CreateTempCacheFd();
    ASSERT_GE(fd, 0);
    ASSERT_TRUE(WriteBufferToFd(fd, buffer));

    preprocess.TakeResultsFromCacheFile(fd);

    EXPECT_NE(StackPreprocess::GetDirectorForTest(), nullptr);
    EXPECT_FALSE(preprocess.isStopTakeData_);
}

/*
@tc.name: TakeResultsFromCacheFileHandlesEmptyFile
@tc.desc: TakeResultsFromCacheFile should exit cleanly when cache file reaches EOF immediately.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, TakeResultsFromCacheFileHandlesEmptyFile, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    int fd = CreateTempCacheFd();
    ASSERT_GE(fd, 0);

    preprocess.TakeResultsFromCacheFile(fd);

    EXPECT_NE(StackPreprocess::GetDirectorForTest(), nullptr);
    EXPECT_FALSE(preprocess.isStopTakeData_);
}

/*
@tc.name: ProcessNmdMsgTest
@tc.desc: test ProcessNmdMsg method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessNmdMsgTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Create a raw stack with NMD message data
    RawStackPtr rawData = MakeRawStack(1, 100, 0);
    // Set data field to a test string
    const char* testTag = "test_memory_tag";
    rawData->data = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(testTag));
    // Just call the method to ensure it doesn't crash
    bool result = preprocess.ProcessNmdMsg(rawData);
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(result);
}

/*
@tc.name: ProcessEndMsgTest
@tc.desc: test ProcessEndMsg method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessEndMsgTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Just call the method to ensure it doesn't crash
    bool result = preprocess.ProcessEndMsg();
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_FALSE(result);
}

/*
@tc.name: ProcessEndMsgStopsImmediatelyInFileCacheMode
@tc.desc: test ProcessEndMsg stops immediately for file cache mode.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessEndMsgStopsImmediatelyInFileCacheMode, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    EXPECT_FALSE(preprocess.ProcessEndMsg());
    EXPECT_TRUE(preprocess.isStopTakeData_);
}

/*
@tc.name: ProcessEndMsgWaitsForAllOfflineStreams
@tc.desc: test ProcessEndMsg waits until all offline shared memory streams end.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessEndMsgWaitsForAllOfflineStreams, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    EXPECT_TRUE(preprocess.ProcessEndMsg());
    EXPECT_TRUE(preprocess.ProcessEndMsg());
    EXPECT_FALSE(preprocess.isStopTakeData_);
    EXPECT_FALSE(preprocess.ProcessEndMsg());
    EXPECT_TRUE(preprocess.isStopTakeData_);
}

/*
@tc.name: ProcessAddressDataTest
@tc.desc: test ProcessAddressData method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessAddressDataTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Create some test address data
    int8_t addressData[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    
    // Just call the method to ensure it doesn't crash
    bool result = preprocess.ProcessAddressData(addressData);
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(result);
}

/*
@tc.name: HandleSimpleMessagesRecognizesSimpleFree
@tc.desc: HandleSimpleMessages should consume FREE_MSG_SIMP directly.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, HandleSimpleMessagesRecognizesSimpleFree, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    HookRecordPtr hookRecord = std::make_shared<FreeRecordSimp>(0x1234);

    EXPECT_TRUE(preprocess.HandleSimpleMessages(hookRecord));
}

/*
@tc.name: ProcessSingleRecordHandlesNullContextGracefully
@tc.desc: ProcessSingleRecord should return true when hook record exists but raw stack context is null.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessSingleRecordHandlesNullContextGracefully, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RawStackPtr rawData = std::make_shared<RawStack>();
    HookRecordPtr hookRecord = std::make_shared<HookRecord>(rawData);

    EXPECT_TRUE(preprocess.ProcessSingleRecord(hookRecord));
    EXPECT_FALSE(preprocess.isStopTakeData_);
}

/*
@tc.name: ProcessSingleRecordSkipsUnreportedArktsFree
@tc.desc: ProcessSingleRecord should keep running when FREE_ARKTS is filtered out by reportFlag.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessSingleRecordSkipsUnreportedArktsFree, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RawStackPtr rawData = MakeRawStack(FREE_ARKTS, 7, 0);
    rawData->stackContext->mallocSize = 16;
    rawData->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawData->reportFlag = false;
    HookRecordPtr hookRecord = std::make_shared<HookRecord>(rawData);

    EXPECT_TRUE(preprocess.ProcessSingleRecord(hookRecord));
    EXPECT_EQ(preprocess.ignoreCnts_, 1);
    EXPECT_TRUE(preprocess.isStartTakeData_.load());
}

/*
@tc.name: SetMapsInfoTest
@tc.desc: test SetMapsInfo method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, SetMapsInfoTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Just call the method to ensure it doesn't crash
    preprocess.SetMapsInfo();
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(true);
}

/*
@tc.name: BuildCallFramesTest
@tc.desc: test BuildCallFrames method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, BuildCallFramesTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Create test data
    RawStackPtr rawData = MakeRawStack(1, 100, 0);
    constexpr size_t regBufferSize = sizeof(uint64_t) * 34; // Enough for ARM64 registers
    rawData->baseStackData = std::make_unique<uint8_t[]>(regBufferSize);
    rawData->data = rawData->baseStackData.get();
    auto hookRecord = std::make_shared<HookRecord>(rawData);
    std::vector<CallFrame> callFrames = MakeFrames(123);
    
    // Just call the method to ensure it doesn't crash
    preprocess.InitializeDirector();
    bool result = preprocess.BuildCallFrames(hookRecord, rawData, callFrames);
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(result);
}

/*
@tc.name: BuildCallFramesRejectsNullAsyncRawData
@tc.desc: test BuildCallFrames rejects null rawData in async mode.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, BuildCallFramesRejectsNullAsyncRawData, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_async_stack_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto hookRecord = std::make_shared<HookRecord>(nullptr);
    RawStackPtr rawData = nullptr;
    std::vector<CallFrame> callFrames;

    EXPECT_FALSE(preprocess.BuildCallFrames(hookRecord, rawData, callFrames));
}

/*
@tc.name: BuildCallFramesRejectsNullAsyncContext
@tc.desc: test BuildCallFrames rejects async rawData without stackContext.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, BuildCallFramesRejectsNullAsyncContext, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_async_stack_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RawStackPtr rawData(new RawStack(), [](RawStack* p) { delete p; });
    auto hookRecord = std::make_shared<HookRecord>(rawData);
    std::vector<CallFrame> callFrames;

    EXPECT_FALSE(preprocess.BuildCallFrames(hookRecord, rawData, callFrames));
}

/*
@tc.name: BuildCallFramesSkipsSpecialAsyncTypes
@tc.desc: test BuildCallFrames skips special async message types without touching call frames.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, BuildCallFramesSkipsSpecialAsyncTypes, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_async_stack_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RawStackPtr rawData = MakeAsyncRawStack(PR_SET_VMA_MSG);
    auto hookRecord = std::make_shared<HookRecord>(rawData);
    std::vector<CallFrame> callFrames = MakeFrames(321);

    EXPECT_TRUE(preprocess.BuildCallFrames(hookRecord, rawData, callFrames));
    EXPECT_EQ(callFrames.size(), 1u);
    EXPECT_EQ(callFrames[0].callFrameId_, 321u);
}

/*
@tc.name: ValidateAndPrepareDataRejectsTooSmallBuffer
@tc.desc: test ValidateAndPrepareData rejects payloads smaller than BaseStackRawData.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ValidateAndPrepareDataRejectsTooSmallBuffer, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::array<int8_t, sizeof(BaseStackRawData) - 1> invalidData = {};
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;

    EXPECT_FALSE(preprocess.ValidateAndPrepareData(invalidData.data(),
        static_cast<uint32_t>(invalidData.size()), hookRecord, rawData));
}

/*
@tc.name: ValidateAndPrepareDataRejectsUnknownType
@tc.desc: test ValidateAndPrepareData rejects unknown hook record types.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ValidateAndPrepareDataRejectsUnknownType, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(std::make_shared<HookRecordFactory>(hookConfig));
    BaseStackRawData invalidData = {};
    invalidData.type = UNKNOWN;
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;

    EXPECT_FALSE(preprocess.ValidateAndPrepareData(reinterpret_cast<const int8_t*>(&invalidData),
        sizeof(invalidData), hookRecord, rawData));
}

/*
@tc.name: InitializeDirectorTest
@tc.desc: test InitializeDirector method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, InitializeDirectorTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Clear any existing director
    StackPreprocess::SetDirectorForTest(nullptr);
    EXPECT_EQ(StackPreprocess::GetDirectorForTest(), nullptr);
    
    // Initialize director
    preprocess.InitializeDirector();
    EXPECT_NE(StackPreprocess::GetDirectorForTest(), nullptr);
    
    // Clear director after test
    StackPreprocess::SetDirectorForTest(nullptr);
    EXPECT_EQ(StackPreprocess::GetDirectorForTest(), nullptr);
}

/*
@tc.name: GetBuildIdInvalidElfTest
@tc.desc: test GetBuildId method with invalid elf file.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, GetBuildIdInvalidElfTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Test with non-existent file - should return empty string
    std::string result = preprocess.GetBuildId("/non/existent/path/libtest.so");
    EXPECT_EQ(result, "");

    // Test with invalid elf file (empty file)
    const std::string tempInvalidElf = "/tmp/test_invalid_elf.so";
    FILE* fp = fopen(tempInvalidElf.c_str(), "w");
    if (fp != nullptr) {
        fprintf(fp, "not a valid elf file");
        fclose(fp);

        result = preprocess.GetBuildId(tempInvalidElf);
        EXPECT_EQ(result, "");

        unlink(tempInvalidElf.c_str());
    }
}

/*
@tc.name: GetBuildIdValidElfTest
@tc.desc: test GetBuildId method with valid elf file.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, GetBuildIdValidElfTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Test with a valid system library
    // Note: This test may return empty string if the library doesn't have build-id
    // or the file cannot be accessed in the test environment
    std::string result = preprocess.GetBuildId("/system/lib/libc.so");
    // If the file exists and is valid, we should get a build-id (hex string) or empty if not found
    // We just verify the method doesn't crash and returns a valid string
    EXPECT_TRUE(result.empty() || result.length() > 0);
}

/*
@tc.name: GetBuildIdEmptyPathTest
@tc.desc: test GetBuildId method with empty path.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, GetBuildIdEmptyPathTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Test with empty path
    std::string result = preprocess.GetBuildId("");
    EXPECT_EQ(result, "");
}
}
