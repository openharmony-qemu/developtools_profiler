
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
#include <sys/mman.h>
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
constexpr uint32_t MAX_MATCH_INTERVAL = 3600;
constexpr uint32_t MAX_MATCH_CNT = 1000;
constexpr uint64_t DWARF_NAPI_CALLBACK = 999999;
constexpr size_t MAX_BUFFER_SIZE = 1024 * 1024;
constexpr uint32_t MAX_ENUM = 9999;
constexpr uint32_t MAX_BATCH_CNT = 40;
constexpr uint64_t JS_OFFLINE_IP_MASK = 0xFFFFFE0000000000;
constexpr uint32_t NAPI_TEST_TAG_ID = 100;
constexpr int NAPI_CALL_STACK = 2;
constexpr size_t ENCODER_BUFFER_SIZE = 4096;
static uint8_t g_encoderBuffer[ENCODER_BUFFER_SIZE];
static uint32_t g_encoderOffset = 0;
static int32_t g_writerFinishSize = 0;
static uint32_t g_writerFlushCount = 0;

static bool GetEncoderMemory(RandomWriteCtx*, uint32_t size, uint8_t** memory, uint32_t* offset)
{
    if (memory == nullptr || offset == nullptr || g_encoderOffset + size > ENCODER_BUFFER_SIZE) {
        return false;
    }
    *memory = g_encoderBuffer + g_encoderOffset;
    *offset = g_encoderOffset;
    g_encoderOffset += size;
    return true;
}

static bool SeekEncoderMemory(RandomWriteCtx*, uint32_t offset)
{
    if (offset > ENCODER_BUFFER_SIZE) {
        return false;
    }
    g_encoderOffset = offset;
    return true;
}

static RandomWriteCtx MakeEncoderWriteCtx()
{
    g_encoderOffset = 0;
    return {GetEncoderMemory, SeekEncoderMemory};
}

static RandomWriteCtx* StartEncoderReport(WriterStruct*)
{
    static RandomWriteCtx ctx = {GetEncoderMemory, SeekEncoderMemory};
    g_encoderOffset = 0;
    return &ctx;
}

static RandomWriteCtx* StartNullReport(WriterStruct*)
{
    return nullptr;
}

static bool FlushEncoderWriter(WriterStruct*)
{
    ++g_writerFlushCount;
    return true;
}

static void FinishEncoderReport(WriterStruct*, int32_t size)
{
    g_writerFinishSize = size;
}

static WriterStruct MakeEncoderWriter(StartReportFuncPtr startReport)
{
    WriterStruct writer {};
    writer.startReport = startReport;
    writer.flush = FlushEncoderWriter;
    writer.finishReport = FinishEncoderReport;
    return writer;
}

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

static std::shared_ptr<HookRecord> MakeHookRecord(uint32_t nodeType, uint64_t nodeId, uint32_t tagId)
{
    return std::make_shared<HookRecord>(MakeRawStack(nodeType, nodeId, tagId));
}

static HookRecordPtr MakeMallocHookRecord(uint64_t nodeId, uint64_t addr)
{
    auto rawStack = MakeRawStack(1, nodeId, 0);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->mallocSize = 40;
    rawStack->stackContext->addr = reinterpret_cast<void*>(addr);
    return std::make_shared<MallocRecord>(rawStack);
}

static HookRecordPtr MakeFileRecord(uint16_t type, uint64_t addr)
{
    auto rawStack = MakeRawStack(1, type, 0);
    rawStack->stackContext->type = type;
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->addr = reinterpret_cast<void*>(addr);
    if (type == FREE_MSG) {
        return std::make_shared<FreeRecord>(rawStack);
    }
    if (type == MMAP_MSG) {
        return std::make_shared<MmapRecord>(rawStack);
    }
    if (type == MUNMAP_MSG) {
        return std::make_shared<MunmapRecord>(rawStack);
    }
    rawStack->data = reinterpret_cast<uint8_t*>(const_cast<char*>("native_tag"));
    return std::make_shared<PrSetVmaRecord>(rawStack);
}

static HookRecordPtr MakeStatisticsRecord(uint16_t type, uint64_t nodeId)
{
    auto rawStack = MakeRawStack(1, nodeId, NAPI_TEST_TAG_ID);
    rawStack->stackContext->type = type;
    rawStack->stackContext->mallocSize = 32 + nodeId;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x900000 + nodeId * 0x10);
    rawStack->stackContext->newAddr = reinterpret_cast<void*>(0xA00000 + nodeId * 0x10);
    if (type == FREE_MSG) {
        return std::make_shared<FreeRecord>(rawStack);
    }
    if (type == MMAP_MSG) {
        return std::make_shared<MmapRecord>(rawStack);
    }
    if (type == MMAP_FILE_PAGE_MSG) {
        return std::make_shared<MmapFilePageRecord>(rawStack);
    }
    if (type == MUNMAP_MSG) {
        return std::make_shared<MunmapRecord>(rawStack);
    }
    if (type == MEMORY_USING_MSG) {
        return std::make_shared<MemoryUsingRecord>(rawStack);
    }
    if (type == MEMORY_UNUSING_MSG) {
        return std::make_shared<MemoryUnusingRecord>(rawStack);
    }
    if (type == PR_SET_VMA_MSG) {
        return std::make_shared<PrSetVmaRecord>(rawStack);
    }
    if (type == MALLOC_ARKTS) {
        return std::make_shared<ArktsMallocRecord>(rawStack);
    }
    if (type == FREE_ARKTS) {
        return std::make_shared<ArktsFreeRecord>(rawStack);
    }
    if (type == MALLOC_MSG) {
        return std::make_shared<MallocRecord>(rawStack);
    }
    return std::make_shared<HookRecord>(rawStack);
}

static void SeedStatistic(StackPreprocess& preprocess, uint64_t addr, uint32_t stackId)
{
    RecordStatistic statistic;
    statistic.callstackId = stackId;
    preprocess.recordStatisticsMap_[stackId] = statistic;
    preprocess.allocAddrMap_[addr] = AllocAddrInfo {64, stackId, {1, 0}};
    preprocess.addrSet_.insert(addr);
}

static std::shared_ptr<HookRecord> MakeAsyncHookRecord(uint16_t type, uint64_t syncStackId,
    uint64_t asyncStackId, size_t mallocSize = 0)
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
    asyncData->syncStackId = syncStackId;
    asyncData->asyncStackId = asyncStackId;
    asyncData->mallocSize = mallocSize;
    raw->stackContext = asyncData;
    raw->stackSize = 1;
    raw->reportFlag = true;
    return std::make_shared<HookRecord>(raw);
}

static std::vector<CallFrame> MakeFrames(uint64_t callFrameId)
{
    std::vector<CallFrame> frames;
    CallFrame frame(callFrameId);
    frame.callFrameId_ = static_cast<uint32_t>(callFrameId);
    frames.push_back(frame);
    return frames;
}

static std::string GetSelfExePathForTest()
{
    char path[PATH_MAX + 1] = {0};
    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX);
    if (len <= 0 || len > PATH_MAX) {
        return "";
    }
    path[len] = '\0';
    return path;
}

static ReportEventBaseData MakeReportEvent(uint16_t type, uint64_t addr, uint32_t stackId)
{
    BaseStackRawData base = {};
    base.addr = reinterpret_cast<void*>(addr);
    base.type = type;
    base.mallocSize = 128;
    base.tid = 456;
    ReportEventBaseData event(&base, stackId);
    return event;
}

static void AddMatchPeriodEvents(StackPreprocess& preprocess)
{
    preprocess.SaveMemTag(0, "native_tag");
    preprocess.prctlPeriodTags_.push_back("anon_vma");
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MALLOC_MSG, 0x1000, 1));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MMAP_MSG, 0x2000, 2));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MMAP_FILE_PAGE_MSG, 0x3000, 3));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(PR_SET_VMA_MSG, 0x4000, 4));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MEMORY_USING_MSG, 0x5000, 5));
    preprocess.applyAndReleaseMatchPeriodListData_.push_back(MakeReportEvent(MEMORY_UNUSING_MSG, 0x6000, 6));
    preprocess.releasedPeriodListData_.push_back(MakeReportEvent(FREE_MSG, 0x1000, 7));
    preprocess.releasedPeriodListData_.push_back(MakeReportEvent(MUNMAP_MSG, 0x2000, 8));
}

static thread_local std::vector<uint64_t> g_callStack;
static thread_local std::shared_ptr<BuildStackDirector> g_director{nullptr};
class StackPreprocessTest : public testing::Test {
public:
static void SetUpTestCase(void);
static void TearDownTestCase(void);
void SetUp();
void TearDown();
void BuildAbnormalData(BatchNativeHookData& testData);
};
void StackPreprocessTest::SetUpTestCase(void)
{
}

void StackPreprocessTest::TearDownTestCase(void)
{
}

void StackPreprocessTest::SetUp(void)
{
    StackPreprocess::ClearThreadLocalVariables();
}

void StackPreprocessTest::TearDown(void)
{
    StackPreprocess::ClearThreadLocalVariables();
}

void StackPreprocessTest::BuildAbnormalData(BatchNativeHookData& testData)
{
    NativeHookData* event = testData.add_events();
    event->set_tv_sec(0);
    event->set_tv_nsec(-1);
    AllocEvent* allocEvent = event->mutable_alloc_event();
    allocEvent->set_pid(-1);
    allocEvent->set_addr(0);
    allocEvent->clear_frame_info();
    allocEvent->set_thread_name_id(0);

    NativeHookData* traceEvent = testData.add_events();
    TraceAllocEvent* traceAlloc = traceEvent->mutable_trace_alloc_event();
    traceAlloc->set_trace_type(static_cast<TraceType>(MAX_ENUM));
    traceAlloc->set_tag_name("");
    traceAlloc->set_size(0);
}

/*
@tc.name: StackPreprocessTest001
@tc.desc: test StackPreprocess with overcceeding max matching interval.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StackPreprocessTest001, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_malloc_free_matching_interval(MAX_MATCH_INTERVAL + 1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    ASSERT_TRUE(preprocess.hookConfig_.malloc_free_matching_interval() == MAX_MATCH_INTERVAL);
}

/*
@tc.name: StackPreprocessTest002
@tc.desc: test StackPreprocess with overcceeding max matching cnt.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StackPreprocessTest002, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_malloc_free_matching_cnt(MAX_MATCH_CNT + 1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    ASSERT_TRUE(preprocess.hookConfig_.malloc_free_matching_cnt() == MAX_MATCH_CNT);
}

/*
@tc.name: StackPreprocessTest003
@tc.desc: test StackPreprocess with save_file set to true but no file pointer provided.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StackPreprocessTest003, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_save_file(true);
    FILE* fpHookData = nullptr;
    StackPreprocess preprocess(nullptr, hookConfig, 0, fpHookData);
    ASSERT_TRUE(hookConfig.save_file() && fpHookData == nullptr);
}

/*
@tc.name: ProcessSharedMemoryDataAddressSizeTest
@tc.desc: test ProcessRawData with size equal to sizeof(uint64_t).
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSharedMemoryDataAddressSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    uint64_t testAddr = 0x12345678;
    int8_t addrData[sizeof(uint64_t)];
    int ret = memcpy_s(addrData, sizeof(addrData),  &testAddr, sizeof(uint64_t));
    ASSERT_EQ(ret, 0);
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;
    bool result = preprocess.ProcessRawData(addrData, sizeof(addrData), rawData, hookRecord);
    ASSERT_TRUE(result);
}

/*
@tc.name: ProcessSharedMemoryDataInvalidSizeTest
@tc.desc: test ProcessRawData with invalid data size.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSharedMemoryDataInvalidSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    int8_t invalidData[4] = {0}; // Smaller than any valid data structure
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;
    bool result = preprocess.ProcessRawData(invalidData, sizeof(invalidData), rawData, hookRecord);
    ASSERT_FALSE(result);
}

/*
@tc.name: ProcessSharedMemoryDataStopFlagTest
@tc.desc: test ProcessRawData with isStopTakeData_ set to true.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSharedMemoryDataStopFlagTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    int8_t validData[32] = {0}; // size for valid data
    preprocess.isStopTakeData_ = true;
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;
    bool result = preprocess.ProcessRawData(validData, sizeof(validData), rawData, hookRecord);
    ASSERT_FALSE(result);
}

/*
@tc.name: StackPreprocessTestInitializeDirector
@tc.desc: test InitializeDirector with g_director as nullptr.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StackPreprocessTestInitializeDirector, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    StackPreprocess::SetDirectorForTest(nullptr);
    preprocess.InitializeDirector();
    ASSERT_NE(StackPreprocess::GetDirectorForTest(), nullptr);
    preprocess.InitializeDirector(); // the second call, it has no change
    ASSERT_NE(StackPreprocess::GetDirectorForTest(), nullptr);
    StackPreprocess::SetDirectorForTest(nullptr);
    ASSERT_EQ(StackPreprocess::GetDirectorForTest(), nullptr);
}

/*
@tc.name: MemtagMapTest
@tc.desc: test memtagMap with valid data.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, MemtagMapTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(1, "asd");
    std::string result;
    preprocess.GetMemTag(1, result);
    ASSERT_EQ(result, "asd");
    preprocess.SaveMemTag(1, "qwe");
    preprocess.GetMemTag(1, result);
    ASSERT_EQ(result, "qwe");
}

/*
@tc.name: ValidateAndPrepareDataValidTest
@tc.desc: test ValidateAndPrepareData with valid data.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ValidateAndPrepareDataValidTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;
    int8_t validData[32] = {0}; // A reasonable size for valid data
    ASSERT_FALSE(preprocess.ValidateAndPrepareData(validData, sizeof(validData), hookRecord, rawData));
}

/*
@tc.name: ProcessEndMsgNotEqualTest
@tc.desc: test ProcessEndMsg count not equal.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessEndMsgNotEqualTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.isStopTakeData_.store(false);
    preprocess.endMsgCount_ = SHARED_MEMORY_NUM;
    preprocess.ProcessEndMsg();
    EXPECT_FALSE(preprocess.isStopTakeData_.load());
}

/*
@tc.name: InitializeGpuDataTest
@tc.desc: test GPU data init.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, InitializeGpuDataTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.InitializeGpuData();

    std::vector<std::pair<uint32_t, std::string>> expectedTags = {
        {GPU_VK_INDEX + 1, TAG_RES_GPU_VK},
        {GPU_GLES_IMAGE_INDEX + 1, TAG_RES_GPU_GLES_IMAGE},
        {GPU_GLES_BUFFER_INDEX + 1, TAG_RES_GPU_GLES_BUFFER},
        {GPU_CL_IMAGE_INDEX + 1, TAG_RES_GPU_CL_IMAGE},
        {GPU_CL_BUFFER_INDEX + 1, TAG_RES_GPU_CL_BUFFER},
        {FD_OPEN_INDEX + 1, TAG_RES_FD_OPEN},
        {FD_EPOLL_INDEX + 1, TAG_RES_FD_EPOLL},
        {FD_EVENTFD_INDEX + 1, TAG_RES_FD_EVENTFD},
        {FD_SOCKET_INDEX + 1, TAG_RES_FD_SOCKET},
        {FD_PIPE_INDEX + 1, TAG_RES_FD_PIPE},
        {FD_DUP_INDEX + 1, TAG_RES_FD_DUP},
        {FD_MASK_INDEX + 1, TAG_RES_FD_ALL},
        {THREAD_PTHREAD_INDEX + 1, TAG_RES_THREAD_PTHREAD},
        {THREAD_MASK_INDEX + 1, TAG_RES_THREAD_ALL},
        {DART_HEAP_INDEX + 1, TAG_RES_DART_HEAP},
        {RN_HERMES_HEAP_INDEX + 1, TAG_RES_RN_HERMES_HEAP},
    };

    for (const auto& [tagId, tagName] : expectedTags) {
        std::string result;
        bool found = preprocess.GetMemTag(tagId, result);
        EXPECT_TRUE(found) << "Tag not found: " << tagId;
        EXPECT_EQ(result, tagName) << "Tag name mismatch: " << result << " vs " << tagName;
    }
}

/*
@tc.name: FillNapiStackTest
@tc.desc: test fill NAPI stack.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FillNapiStackTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::string tagName = "testNapiTag";
    std::vector<CallFrame> callFrames;
    uint64_t napiIndex = 123;
    preprocess.FillNapiStack(tagName, callFrames, napiIndex);
    ASSERT_EQ(callFrames.size(), 1);
    const CallFrame& jsCallFrame = callFrames[0];
    EXPECT_EQ(jsCallFrame.symbolName_, tagName);
    EXPECT_TRUE(jsCallFrame.isJsFrame_);
    EXPECT_TRUE(jsCallFrame.needReport_ & CALL_FRAME_REPORT);
    EXPECT_TRUE(jsCallFrame.needReport_ & SYMBOL_NAME_ID_REPORT);
    EXPECT_TRUE(jsCallFrame.needReport_ & FILE_PATH_ID_REPORT);
    EXPECT_EQ(jsCallFrame.callFrameId_, DWARF_NAPI_CALLBACK + napiIndex);
    EXPECT_EQ(jsCallFrame.symbolNameId_, DWARF_NAPI_CALLBACK + napiIndex);
    EXPECT_EQ(jsCallFrame.filePathId_, DWARF_NAPI_CALLBACK + napiIndex);
    EXPECT_EQ(jsCallFrame.filePath_, "no-napi-file-path");
}

/*
@tc.name: ReportOfflineSymbolizationDataTest001
@tc.desc: test report offline symbolization data.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ReportOfflineSymbolizationDataTest001, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFlushSize(100);
    EXPECT_TRUE(preprocess.flushBasicData_);
    preprocess.ReportOfflineSymbolizationData();
    EXPECT_FALSE(preprocess.flushBasicData_);
    preprocess.ReportOfflineSymbolizationData();
    EXPECT_FALSE(preprocess.flushBasicData_);
}

/*
@tc.name: ReportOfflineSymbolizationDataTest002
@tc.desc: test report offline symbolization data.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ReportOfflineSymbolizationDataTest002, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(false);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFlushSize(100);
    EXPECT_TRUE(preprocess.flushBasicData_);
    preprocess.ReportOfflineSymbolizationData();
    EXPECT_TRUE(preprocess.flushBasicData_);
}

/*
@tc.name: FillJsRawStackFound
@tc.desc: test get js raw stack with found branch.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FillJsRawStackFound, TestSize.Level0)
{
    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);
    uint64_t testJsChainId = 12345;
    const char* testJsRawStack = "test_js_stack";
    {
        std::lock_guard<std::mutex> guard(preprocess.jsMapMtx_);
        preprocess.jsStackMap_[testJsChainId] = testJsRawStack;
    }
    const char* result = preprocess.GetJsRawStack(testJsChainId);
    EXPECT_EQ(result, testJsRawStack);
}

/*
@tc.name: GetJsRawStackNotFound
@tc.desc: test get js raw statck with not found branch.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, GetJsRawStackNotFound, TestSize.Level0)
{
    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);
    uint64_t testJsChainId = 12345;
    const char* result = preprocess.GetJsRawStack(testJsChainId);
    EXPECT_EQ(result, nullptr);
}

/*
@tc.name: SaveJsRawStackInsertNewTest
@tc.desc: test save js raw stack insert new element.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SaveJsRawStackInsertNewTest, TestSize.Level0)
{
    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);
    uint64_t testJsChainId = 12345;
    const char* testJsRawStack = "test_js_stack";
    EXPECT_EQ(preprocess.jsStackMap_.find(testJsChainId), preprocess.jsStackMap_.end());
    preprocess.SaveJsRawStack(testJsChainId, testJsRawStack);
    auto iter = preprocess.jsStackMap_.find(testJsChainId);
    EXPECT_NE(iter, preprocess.jsStackMap_.end());
    EXPECT_STREQ(iter->second, testJsRawStack);
    EXPECT_NE(preprocess.jsStackSet_.find(testJsRawStack), preprocess.jsStackSet_.end());
}

/*
@tc.name: SaveJsRawStackReuseExistingTest
@tc.desc: test save js stack reuse existing.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SaveJsRawStackReuseExistingTest, TestSize.Level0)
{
    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);

    uint64_t testJsChainId = 12345;
    const char* testJsRawStack = "test_js_stack";
    preprocess.jsStackSet_.insert(testJsRawStack);
    EXPECT_EQ(preprocess.jsStackMap_.find(testJsChainId), preprocess.jsStackMap_.end());
    preprocess.SaveJsRawStack(testJsChainId, testJsRawStack);
    auto iter = preprocess.jsStackMap_.find(testJsChainId);
    EXPECT_NE(iter, preprocess.jsStackMap_.end());
    EXPECT_STREQ(iter->second, testJsRawStack);
    EXPECT_EQ(preprocess.jsStackSet_.size(), 1);
}

/*
@tc.name: SaveJsRawStackExistingChainIdTest
@tc.desc: test save js raw stack with existing chain id.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SaveJsRawStackExistingChainIdTest, TestSize.Level0)
{
    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);
    uint64_t testJsChainId = 12345;
    const char* testJsRawStack = "test_js_stack";
    preprocess.jsStackMap_[testJsChainId] = testJsRawStack;
    auto iter = preprocess.jsStackMap_.find(testJsChainId);
    EXPECT_NE(iter, preprocess.jsStackMap_.end());
    preprocess.SaveJsRawStack(testJsChainId, "test_js_stack");
    EXPECT_STREQ(iter->second, testJsRawStack);
}

/*
@tc.name: GetOfflineMapsEmptyTest
@tc.desc: test set maps info empty.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, GetOfflineMapsEmptyTest, TestSize.Level0)
{
    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);
    preprocess.runtime_instance->GetOfflineMaps().clear();
    preprocess.SetMapsInfo();
    EXPECT_TRUE(preprocess.stackData_.index() == 0);
    EXPECT_TRUE(preprocess.runtime_instance->GetProcessMaps().empty());
}

/*
@tc.name: GetOfflineMapsMissTest
@tc.desc: test set maps cache miss.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, GetOfflineMapsMissTest, TestSize.Level0)
{
    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);
    preprocess.runtime_instance->GetOfflineMaps().push_back(12345);
    preprocess.runtime_instance->GetMapsCache().clear();
    preprocess.SetMapsInfo();
    EXPECT_TRUE(preprocess.stackData_.index() == 0);
    EXPECT_TRUE(preprocess.runtime_instance->GetProcessMaps().empty());
}

/*
@tc.name: FlushDataNormalProtoTest
@tc.desc: test flush data to file.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FlushDataNormalProtoTest, TestSize.Level0)
{
    const std::string filePath = "/data/local/tmp/hiprofiler_data_test_temp.htrace";
    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);
    preprocess.buffer_ = std::make_unique<uint8_t[]>(MAX_BUFFER_SIZE);
    preprocess.bufferSize_ = MAX_BUFFER_SIZE;
    preprocess.fpHookData_ = fopen(filePath.c_str(), "w");
    ASSERT_NE(preprocess.fpHookData_, nullptr) << "Failed to open file for writing";
    preprocess.isHookStandaloneSerialize_ = true;

    BatchNativeHookData testData;
    NativeHookData *event = testData.add_events();
    event->set_tv_sec(1234567890);
    event->set_tv_nsec(987654321);

    preprocess.FlushData(testData);

    std::ifstream file(filePath);
    ASSERT_TRUE(file.is_open()) << "Failed to open file for reading";
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();

    ForStandard::BatchNativeHookData parseData;
    ASSERT_TRUE(google::protobuf::TextFormat::ParseFromString(fileContent, &parseData))
        << "Failed to parse protobuf text format from file";
    ASSERT_EQ(parseData.events_size(), 1);
    EXPECT_TRUE(parseData.events(0).tv_sec() == 1234567890);
    EXPECT_TRUE(parseData.events(0).tv_nsec() == 987654321);
    int ret = std::remove(filePath.c_str());
    ASSERT_EQ(ret, 0) << "Failed to delete temporary file";
}

/*
@tc.name: FlushDataAbnormalProtoTest
@tc.desc: test flush data to file.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FlushDataAbnormalProtoTest, TestSize.Level0)
{
    const std::string filePath = "/data/local/tmp/hiprofiler_data_test_abnormal.htrace";

    StackPreprocess preprocess(nullptr, NativeHookConfig(), 0);
    preprocess.buffer_ = std::make_unique<uint8_t[]>(MAX_BUFFER_SIZE);
    preprocess.bufferSize_ = MAX_BUFFER_SIZE;
    preprocess.fpHookData_ = fopen(filePath.c_str(), "w");
    ASSERT_NE(preprocess.fpHookData_, nullptr) << "Failed to open file";

    preprocess.isHookStandaloneSerialize_ = true;

    BatchNativeHookData testData;
    BuildAbnormalData(testData);

    preprocess.FlushData(testData);

    fclose(preprocess.fpHookData_);

    std::ifstream file(filePath);
    ASSERT_TRUE(file.is_open()) << "Failed to open file for reading";

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();

    ForStandard::BatchNativeHookData parseData;
    ASSERT_TRUE(google::protobuf::TextFormat::ParseFromString(fileContent, &parseData))
        << "Failed to parse protobuf text format from file";

    ASSERT_EQ(parseData.events_size(), 2);

    const auto &firstEvent = parseData.events(0);
    EXPECT_EQ(firstEvent.alloc_event().pid(), -1);
    EXPECT_EQ(firstEvent.alloc_event().addr(), 0);
    EXPECT_EQ(firstEvent.alloc_event().frame_info_size(), 0);
    EXPECT_EQ(firstEvent.tv_nsec(), 18446744073709551615ULL);

    const auto &secondEvent = parseData.events(1);
    EXPECT_EQ(secondEvent.trace_alloc_event().trace_type(), MAX_ENUM);
    EXPECT_EQ(secondEvent.trace_alloc_event().tag_name(), "");
    EXPECT_EQ(secondEvent.trace_alloc_event().size(), 0);

    int ret = std::remove(filePath.c_str());
    ASSERT_EQ(ret, 0) << "Failed to delete temporary file";
}

/*
@tc.name: CallStackHash01
@tc.desc: Same call stack + same nodeType/nodeId + same tagName should map to the same stack id.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CallStackHash01, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    // Use fp_unwind so GetCallStackId starts from idx=0 (won't skip single-frame stacks).
    // GetCallStackId clears internal g_callStack to avoid cross-test contamination.
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // register tag 0 -> "tagA"
    preprocess.SaveMemTag(0, "tagA");

    auto hookRecord = MakeHookRecord(1, 100, 0);
    auto frames = MakeFrames(1111);
    BatchNativeHookData data;
    uint32_t id1 = preprocess.GetCallStackId(hookRecord, frames, data);
    EXPECT_GT(id1, 0u);
    std::vector<uint64_t> lookupStack {static_cast<uint64_t>(frames[0].callFrameId_)};
    uint32_t id2 = preprocess.FindCallStackId(lookupStack, hookRecord);
    EXPECT_EQ(id1, id2);
}

/*
@tc.name: CallStackHash02
@tc.desc: Same call stack but different nodeType/nodeId should map to different stack ids.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CallStackHash02, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.SaveMemTag(0, "tagA");

    auto hr1 = MakeHookRecord(1, 100, 0);
    auto frames = MakeFrames(2222);

    BatchNativeHookData data1;
    uint32_t id1 = preprocess.GetCallStackId(hr1, frames, data1);
    EXPECT_GT(id1, 0u);

    // same frames but different nodeType/nodeId
    auto hr2 = MakeHookRecord(2, 200, 0);
    BatchNativeHookData data2;
    uint32_t id2 = preprocess.GetCallStackId(hr2, frames, data2);
    EXPECT_GT(id2, 0u);
    EXPECT_NE(id1, id2);
}

/*
@tc.name: CallStackHash03
@tc.desc: Non-statistics stack mapping should ignore tagName for identical call stacks.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CallStackHash03, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // register two different tags
    preprocess.SaveMemTag(0, "tagA");
    preprocess.SaveMemTag(1, "tagB");

    // first record with tagA
    auto hr1 = MakeHookRecord(1, 300, 0);
    auto frames = MakeFrames(3333);
    BatchNativeHookData data1;
    uint32_t id1 = preprocess.GetCallStackId(hr1, frames, data1);
    EXPECT_GT(id1, 0u);

    // same frames but tagB
    auto hr2 = MakeHookRecord(1, 300, 1);
    BatchNativeHookData data2;
    uint32_t id2 = preprocess.GetCallStackId(hr2, frames, data2);
    EXPECT_GT(id2, 0u);
    EXPECT_EQ(id1, id2);
}

/*
@tc.name: CallStackHash04
@tc.desc: nodeType=0 and nodeId=0 should still work as a valid dimension for stack mapping.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CallStackHash04, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    // Use fp_unwind so GetCallStackId starts from idx=0 (won't skip single-frame stacks).
    // Also GetCallStackId clears internal g_callStack to avoid cross-test contamination.
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.SaveMemTag(0, "tagA");

    auto hookRecord = MakeHookRecord(0, 0, 0);
    auto frames = MakeFrames(4444);
    BatchNativeHookData data;
    uint32_t id1 = preprocess.GetCallStackId(hookRecord, frames, data);
    EXPECT_GT(id1, 0u);

    std::vector<uint64_t> lookupStack {static_cast<uint64_t>(frames[0].callFrameId_)};
    uint32_t id2 = preprocess.FindCallStackId(lookupStack, hookRecord);
    EXPECT_EQ(id1, id2);
}

/*
@tc.name: CallStackHash05
@tc.desc: nodeType=0 and nodeId!=0 should still work as a valid dimension for stack mapping.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CallStackHash05, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.SaveMemTag(0, "tagA");

    auto hookRecord = MakeHookRecord(0, 123, 0);
    auto frames = MakeFrames(5555);
    BatchNativeHookData data;
    uint32_t id1 = preprocess.GetCallStackId(hookRecord, frames, data);
    EXPECT_GT(id1, 0u);

    std::vector<uint64_t> lookupStack {static_cast<uint64_t>(frames[0].callFrameId_)};
    uint32_t id2 = preprocess.FindCallStackId(lookupStack, hookRecord);
    EXPECT_EQ(id1, id2);
}

/*
@tc.name: CallStackHash06
@tc.desc: tagName empty string (tagId not registered) should still work for stack mapping.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CallStackHash06, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Do not SaveMemTag for tagId 999, tagName remains empty.
    constexpr uint32_t kUnregisteredTagId = 999;

    auto hookRecord = MakeHookRecord(1, 100, kUnregisteredTagId);
    auto frames = MakeFrames(6666);
    BatchNativeHookData data;
    uint32_t id1 = preprocess.GetCallStackId(hookRecord, frames, data);
    EXPECT_GT(id1, 0u);

    std::vector<uint64_t> lookupStack {static_cast<uint64_t>(frames[0].callFrameId_)};
    uint32_t id2 = preprocess.FindCallStackId(lookupStack, hookRecord);
    EXPECT_EQ(id1, id2);
}

/*
@tc.name: CallStackHash07
@tc.desc: callStack empty should still be supported and map consistently.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CallStackHash07, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.SaveMemTag(0, "tagA");
    auto hookRecord = MakeHookRecord(1, 100, 0);

    // Make callStack empty by calling GetCallStackId with fewer frames than FILTER_STACK_DEPTH,
    // so FillCallStack adds nothing after g_callStack.clear().
    std::vector<CallFrame> emptyFrames;
    BatchNativeHookData data;
    uint32_t id1 = preprocess.GetCallStackId(hookRecord, emptyFrames, data);
    EXPECT_GT(id1, 0u);

    std::vector<uint64_t> emptyCallStack;
    uint32_t id2 = preprocess.FindCallStackId(emptyCallStack, hookRecord);
    EXPECT_EQ(id1, id2);
}

/*
@tc.name: NonStatisticsAsyncStackIgnoresSizeAndTag
@tc.desc: non-statistics async stack mapping should reuse the id when only size and tag differ.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, NonStatisticsAsyncStackIgnoresSizeAndTag, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_async_stack_enable(true);
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto firstRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    auto secondRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 128);
    firstRecord->GetRawStack()->stackContext->tagId = 1;
    secondRecord->GetRawStack()->stackContext->tagId = 2;
    auto firstFrames = MakeFrames(0x1234);
    auto secondFrames = MakeFrames(0x1234);
    BatchNativeHookData data;

    uint32_t firstId = preprocess.GetCallStackId(firstRecord, firstFrames, data);
    uint32_t secondId = preprocess.GetCallStackId(secondRecord, secondFrames, data);

    EXPECT_EQ(firstId, secondId);
}

/*
@tc.name: StatisticsSimplifiedSyncStackIgnoresTagId
@tc.desc: simplified statistics stack mapping should ignore tagId for identical frames.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StatisticsSimplifiedSyncStackIgnoresTagId, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(10);
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto firstRecord = MakeHookRecord(0, 0, 1);
    auto secondRecord = MakeHookRecord(0, 0, 2);
    auto firstFrames = MakeFrames(0x1234);
    auto secondFrames = MakeFrames(0x1234);
    BatchNativeHookData data;

    preprocess.SetAllocStatisticsFrame(firstRecord, firstFrames, data);
    preprocess.SetAllocStatisticsFrame(secondRecord, secondFrames, data);

    EXPECT_EQ(preprocess.simplifiedCallStackIndex_.size(), 1u);
}

/*
@tc.name: StatisticsSimplifiedAsyncStackIgnoresTagId
@tc.desc: simplified async statistics stack mapping should ignore tagId for identical stacks.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StatisticsSimplifiedAsyncStackIgnoresTagId, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(10);
    hookConfig.set_async_stack_enable(true);
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto firstRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    auto secondRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    firstRecord->GetRawStack()->stackContext->tagId = 1;
    secondRecord->GetRawStack()->stackContext->tagId = 2;
    auto firstFrames = MakeFrames(0x1234);
    auto secondFrames = MakeFrames(0x1234);
    BatchNativeHookData data;

    preprocess.SetAllocStatisticsFrame(firstRecord, firstFrames, data);
    preprocess.SetAllocStatisticsFrame(secondRecord, secondFrames, data);

    EXPECT_EQ(preprocess.simplifiedAsyncCallStackIndex_.size(), 1u);
}

/*
@tc.name: AsyncCallStackKeyTest
@tc.desc: async stack should only write AsyncCallStackKey and keep CallStackKey empty.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AsyncCallStackKeyTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    hookConfig.set_statistics_interval(10);
    hookConfig.set_async_stack_enable(true);
    hookConfig.set_fp_unwind(true);
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto hookRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    std::vector<CallFrame> frames;
    frames.emplace_back(0x1000, 0, true);
    frames.back().callFrameId_ = 0x2345;
    BatchNativeHookData data;
    preprocess.SetAllocStatisticsFrame(hookRecord, frames, data);

    ASSERT_TRUE(preprocess.callStackIndex_.empty());
    ASSERT_EQ(preprocess.asyncCallStackIndex_.size(), 1u);
    ASSERT_EQ(preprocess.recordStatisticsMap_.size(), 1u);
    uint32_t stackId = preprocess.recordStatisticsMap_.begin()->first;

    const auto& asyncKey = preprocess.asyncCallStackIndex_.begin()->first;
    EXPECT_EQ(preprocess.asyncCallStackIndex_.begin()->second, stackId);
    EXPECT_EQ(asyncKey.stackHash, 11ULL ^ (22ULL << 32));
    EXPECT_EQ(asyncKey.mallocSize, 64u);
}

/*
@tc.name: AsyncStatisticsLookupTest
@tc.desc: FindAsyncStatisticsStackIdLocked should hit asyncCallStackIndex_ in async scene.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AsyncStatisticsLookupTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    hookConfig.set_statistics_interval(10);
    hookConfig.set_async_stack_enable(true);
    hookConfig.set_fp_unwind(true);
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto hookRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    std::vector<CallFrame> frames;
    frames.emplace_back(0x1000, 0, true);
    frames.back().callFrameId_ = 0x2345;
    BatchNativeHookData data;
    preprocess.SetAllocStatisticsFrame(hookRecord, frames, data);

    ASSERT_EQ(preprocess.recordStatisticsMap_.size(), 1u);
    uint32_t stackId = preprocess.recordStatisticsMap_.begin()->first;
    uint32_t lookupId = preprocess.FindAsyncStatisticsStackIdLocked(hookRecord);
    EXPECT_EQ(lookupId, stackId);
}

/*
@tc.name: GetCallStackIdOfflineWritesIpStackMap
@tc.desc: offline symbolization should serialize stack map ip instead of frame_map_id.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, GetCallStackIdOfflineWritesIpStackMap, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto hookRecord = MakeHookRecord(1, 100, 0);
    auto frames = MakeFrames(0x1234);
    frames[0].ip_ = 0xABCDEF;
    BatchNativeHookData data;
    uint32_t stackId = preprocess.GetCallStackId(hookRecord, frames, data);

    ASSERT_GT(stackId, 0u);
    ASSERT_EQ(data.events_size(), 1);
    const auto& stackMap = data.events(0).stack_map();
    EXPECT_EQ(stackMap.ip_size(), 1);
    EXPECT_EQ(stackMap.ip(0), frames[0].ip_);
    EXPECT_EQ(stackMap.frame_map_id_size(), 0);
}

/*
@tc.name: GetCallStackIdOfflineEmptyDwarfStackStartsAtFirstFrame
@tc.desc: offline dwarf stack with zero raw stack size should not skip first two frames.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, GetCallStackIdOfflineEmptyDwarfStackStartsAtFirstFrame, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto hookRecord = MakeHookRecord(1, 100, 0);
    hookRecord->GetRawStack()->stackSize = 0;
    auto frames = MakeFrames(0x2222);
    frames[0].ip_ = 0x123456;
    BatchNativeHookData data;
    uint32_t stackId = preprocess.GetCallStackId(hookRecord, frames, data);

    ASSERT_GT(stackId, 0u);
    ASSERT_EQ(data.events_size(), 1);
    const auto& stackMap = data.events(0).stack_map();
    ASSERT_EQ(stackMap.ip_size(), 1);
    EXPECT_EQ(stackMap.ip(0), frames[0].ip_);
}

HWTEST_F(StackPreprocessTest, GetCallStackIdSkipsFreeWhenReportDisabled, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_free_stack_report(false);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto hookRecord = MakeHookRecord(1, 100, 0);
    hookRecord->GetRawStack()->stackContext->type = FREE_MSG;
    auto frames = MakeFrames(0x1111);
    BatchNativeHookData data;

    uint32_t stackId = preprocess.GetCallStackId(hookRecord, frames, data);

    EXPECT_EQ(stackId, 0u);
    EXPECT_EQ(data.events_size(), 0);
}

HWTEST_F(StackPreprocessTest, GetCallStackIdSkipsMunmapWhenReportDisabled, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_munmap_stack_report(false);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto hookRecord = MakeHookRecord(1, 100, 0);
    hookRecord->GetRawStack()->stackContext->type = MUNMAP_MSG;
    auto frames = MakeFrames(0x2222);
    BatchNativeHookData data;

    uint32_t stackId = preprocess.GetCallStackId(hookRecord, frames, data);

    EXPECT_EQ(stackId, 0u);
    EXPECT_EQ(data.events_size(), 0);
}

/*
@tc.name: SetAllocStatisticsFrameAddsNapiFrame
@tc.desc: NAPI stack statistics should insert one synthetic JS frame and report it.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetAllocStatisticsFrameAddsNapiFrame, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    hookConfig.set_statistics_interval(10);
    hookConfig.set_fp_unwind(true);
    hookConfig.set_js_stack_report(NAPI_CALL_STACK);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, "napi_callback");

    auto hookRecord = MakeHookRecord(1, 100, NAPI_TEST_TAG_ID);
    hookRecord->GetRawStack()->stackContext->mallocSize = 64;
    auto frames = MakeFrames(0x3333);
    BatchNativeHookData data;
    preprocess.SetAllocStatisticsFrame(hookRecord, frames, data);

    // Note: symbolName_ is string_view pointing to a local variable (tagName) in
    // SetAllocStatisticsFrame. We cannot reliably verify the string content after
    // the function returns. Instead, verify the frame metadata that doesn't dangle.
    ASSERT_EQ(frames.size(), 2);
    EXPECT_TRUE(frames.back().isJsFrame_);
    EXPECT_EQ(frames.back().callFrameId_, 1000000u); // DWARF_NAPI_CALLBACK + napiIndex_(1)
    ASSERT_EQ(data.events_size(), 4);
    EXPECT_TRUE(data.events(0).has_symbol_name());
    EXPECT_EQ(data.events(0).symbol_name().name(), "napi_callback");
    EXPECT_TRUE(data.events(1).has_file_path());
    EXPECT_TRUE(data.events(2).has_frame_map());
    EXPECT_TRUE(data.events(3).has_stack_map());
    EXPECT_EQ(preprocess.recordStatisticsMap_.size(), 1u);
}

/*
@tc.name: SetAllocStatisticsFrameOfflineAddsMaskedNapiFrame
@tc.desc: offline NAPI stack statistics should store masked synthetic frame id in stack map ip.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetAllocStatisticsFrameOfflineAddsMaskedNapiFrame, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(10);
    hookConfig.set_fp_unwind(true);
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(NAPI_CALL_STACK);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, "napi_async");

    auto hookRecord = MakeHookRecord(1, 100, NAPI_TEST_TAG_ID);
    hookRecord->GetRawStack()->stackContext->mallocSize = 64;
    auto frames = MakeFrames(0x4444);
    frames[0].ip_ = 0x654321;
    BatchNativeHookData data;
    preprocess.SetAllocStatisticsFrame(hookRecord, frames, data);

    ASSERT_EQ(data.events_size(), 4);
    EXPECT_TRUE(data.events(0).has_symbol_name());
    EXPECT_TRUE(data.events(1).has_file_path());
    EXPECT_TRUE(data.events(2).has_frame_map());
    const auto& stackMap = data.events(3).stack_map();
    ASSERT_EQ(stackMap.ip_size(), 3);
    EXPECT_EQ(stackMap.ip(0), static_cast<uint64_t>(frames.back().callFrameId_) | JS_OFFLINE_IP_MASK);
    EXPECT_EQ(stackMap.ip(1), SIZE_MASK | 64u);
    EXPECT_EQ(stackMap.ip(2), frames[0].ip_);
}

/*
@tc.name: TryProcessCachedAsyncStatisticsWithoutFileModeTest
@tc.desc: cached async statistics should still work when statistics mode is enabled without file cache mode.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, TryProcessCachedAsyncStatisticsWithoutFileModeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    hookConfig.set_statistics_interval(10);
    hookConfig.set_async_stack_enable(true);
    hookConfig.set_fp_unwind(true);
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(1);
    auto factory = std::make_shared<HookRecordFactory>(hookConfig);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(factory);

    auto firstRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    std::vector<CallFrame> frames;
    frames.emplace_back(0x1000, 0, true);
    frames.back().callFrameId_ = 0x2345;
    BatchNativeHookData data;
    preprocess.SetAllocStatisticsFrame(firstRecord, frames, data);

    ASSERT_EQ(preprocess.recordStatisticsMap_.size(), 1u);
    uint32_t stackId = preprocess.recordStatisticsMap_.begin()->first;
    auto secondRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    bool ret = preprocess.TryProcessCachedAsyncStatistics(secondRecord);

    ASSERT_TRUE(ret);
    ASSERT_EQ(secondRecord, nullptr);
    auto iter = preprocess.recordStatisticsMap_.find(stackId);
    ASSERT_NE(iter, preprocess.recordStatisticsMap_.end());
    EXPECT_EQ(iter->second.applyCount, 2u);
}

HWTEST_F(StackPreprocessTest, TryProcessCachedAsyncStatisticsSkipsSaveFileMode, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(10);
    hookConfig.set_async_stack_enable(true);
    hookConfig.set_save_file(true);
    hookConfig.set_file_name("/data/local/tmp/async_statistics.txt");
    StackPreprocess preprocess(nullptr, hookConfig, 0, nullptr, false);
    auto hookRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);

    bool ret = preprocess.TryProcessCachedAsyncStatistics(hookRecord);

    EXPECT_FALSE(ret);
    EXPECT_NE(hookRecord, nullptr);
    EXPECT_TRUE(preprocess.recordStatisticsMap_.empty());
}

HWTEST_F(StackPreprocessTest, ReconcileDuplicateMallocHandlesOlderAndNewerEvents, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(10);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint64_t addr = 0x7700;
    constexpr uint32_t stackId = 5;
    RecordStatistic record;
    record.callstackId = stackId;
    preprocess.recordStatisticsMap_[stackId] = record;
    preprocess.allocAddrMap_[addr] = AllocAddrInfo {64, stackId, {2, 0}};

    EXPECT_FALSE(preprocess.ReconcileDuplicateMalloc(addr, {1, 0}));
    EXPECT_EQ(preprocess.allocAddrMap_.count(addr), 1u);

    EXPECT_TRUE(preprocess.ReconcileDuplicateMalloc(addr, {3, 0}));
    EXPECT_EQ(preprocess.allocAddrMap_.count(addr), 0u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[stackId].releaseCount, 1u);
    EXPECT_TRUE(preprocess.ReconcileDuplicateMalloc(0x8800, {1, 0}));
}

/*
@tc.name: HandleSimpleMessagesFreeSimpleTest
@tc.desc: test HandleSimpleMessages with FREE_MSG_SIMP type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleSimpleMessagesFreeSimpleTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = FREE_MSG_SIMP;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    bool result = preprocess.HandleSimpleMessages(hookRecord);

    EXPECT_TRUE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: HandleSimpleMessagesFreeArktsTest
@tc.desc: test HandleSimpleMessages with FREE_ARKTS type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleSimpleMessagesFreeArktsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = FREE_ARKTS;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x2000);
    rawStack->stackContext->mallocSize = 1024;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    bool result = preprocess.HandleSimpleMessages(hookRecord);

    EXPECT_FALSE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: HandleSimpleMessagesMallocArktsTest
@tc.desc: test HandleSimpleMessages with MALLOC_ARKTS type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleSimpleMessagesMallocArktsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_ARKTS;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x3000);
    rawStack->stackContext->newAddr = reinterpret_cast<void*>(0x4000);
    rawStack->stackContext->mallocSize = 2048;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    bool result = preprocess.HandleSimpleMessages(hookRecord);

    EXPECT_FALSE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: HandleSimpleMessagesOtherTypeTest
@tc.desc: test HandleSimpleMessages with other message types.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleSimpleMessagesOtherTypeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    bool result = preprocess.HandleSimpleMessages(hookRecord);

    EXPECT_FALSE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: ProcessSingleRecordNullRecordTest
@tc.desc: test ProcessSingleRecord with null hook record.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordNullRecordTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    HookRecordPtr hookRecord = nullptr;
    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_FALSE(result);
}

/*
@tc.name: ProcessSingleRecordStopFlagTest
@tc.desc: test ProcessSingleRecord when stop flag is set.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordStopFlagTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.isStopTakeData_ = true;

    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_FALSE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: ProcessSingleRecordNullStackContextTest
@tc.desc: test ProcessSingleRecord with null stack context.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordNullStackContextTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = nullptr;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_TRUE(result);
}

/*
@tc.name: ProcessSingleRecordNmdMsgTest
@tc.desc: test ProcessSingleRecord with NMD_MSG type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordNmdMsgTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = NMD_MSG;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_FALSE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: ProcessSingleRecordEndMsgTest
@tc.desc: test ProcessSingleRecord with END_MSG type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordEndMsgTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = END_MSG;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.isStopTakeData_ = false;

    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_FALSE(result);
    EXPECT_TRUE(preprocess.isStopTakeData_);
    delete rawStack->stackContext;
}

/*
@tc.name: ProcessSingleRecordSimpleMessageTest
@tc.desc: test ProcessSingleRecord with simple message type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordSimpleMessageTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = FREE_MSG_SIMP;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_TRUE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: CleanupBatchRecordsEmptyArrayTest
@tc.desc: test CleanupBatchRecords with empty array.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CleanupBatchRecordsEmptyArrayTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    auto factory = std::make_shared<HookRecordFactory>(hookConfig);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(factory);

    HookRecordPtr batchRawStack[MAX_BATCH_CNT] = {nullptr};
    preprocess.CleanupBatchRecords(batchRawStack, MAX_BATCH_CNT);

    for (int i = 0; i < MAX_BATCH_CNT; i++) {
        EXPECT_EQ(batchRawStack[i], nullptr);
    }
}

/*
@tc.name: CleanupBatchRecordsSingleRecordTest
@tc.desc: test CleanupBatchRecords with single record.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CleanupBatchRecordsSingleRecordTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    auto factory = std::make_shared<HookRecordFactory>(hookConfig);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(factory);

    HookRecordPtr batchRawStack[MAX_BATCH_CNT] = {nullptr};
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    batchRawStack[0] = std::make_shared<HookRecord>(rawStack);

    preprocess.CleanupBatchRecords(batchRawStack, MAX_BATCH_CNT);

    EXPECT_EQ(batchRawStack[0], nullptr);
    delete rawStack->stackContext;
}

/*
@tc.name: CleanupBatchRecordsMultipleRecordsTest
@tc.desc: test CleanupBatchRecords with multiple records.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CleanupBatchRecordsMultipleRecordsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    auto factory = std::make_shared<HookRecordFactory>(hookConfig);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(factory);

    HookRecordPtr batchRawStack[MAX_BATCH_CNT] = {nullptr};
    BaseStackRawData* stackContexts[5] = {nullptr};

    for (int i = 0; i < 5; i++) {
        auto rawStack = std::make_shared<RawStack>();
        rawStack->stackContext = new BaseStackRawData();
        rawStack->stackContext->type = MALLOC_MSG;
        stackContexts[i] = rawStack->stackContext;
        batchRawStack[i] = std::make_shared<HookRecord>(rawStack);
    }

    preprocess.CleanupBatchRecords(batchRawStack, MAX_BATCH_CNT);

    for (int i = 0; i < 5; i++) {
        EXPECT_EQ(batchRawStack[i], nullptr);
        delete stackContexts[i];
    }
}

/*
@tc.name: ProcessBatchDataNoDataRepeaterTest
@tc.desc: test ProcessBatchData when dataRepeater is null.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessBatchDataNoDataRepeaterTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    bool result = preprocess.ProcessBatchData();

    EXPECT_FALSE(result);
}

/*
@tc.name: ProcessBatchDataStatisticsIntervalTest
@tc.desc: test ProcessBatchData with statistics interval configured.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessBatchDataStatisticsIntervalTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(10);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.InitStatisticsTime();

    bool result = preprocess.ProcessBatchData();

    EXPECT_FALSE(result);
}

/*
@tc.name: SetStopTakeDataTest
@tc.desc: test SetStopTakeData function with file cache mode.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetStopTakeDataTest, TestSize.Level0)
{
    // Test with file cache mode enabled
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    preprocess.isStopTakeData_ = false;
    preprocess.SetStopTakeData();
    EXPECT_TRUE(preprocess.isStopTakeData_);
    
    // Test with file cache mode disabled
    NativeHookConfig hookConfigNoCache;
    hookConfigNoCache.set_use_file_cache_mode(false);
    StackPreprocess preprocessNoCache(nullptr, hookConfigNoCache, 0);
    
    preprocessNoCache.isStopTakeData_ = false;
    preprocessNoCache.SetStopTakeData();
    EXPECT_FALSE(preprocessNoCache.isStopTakeData_);
}

/*
@tc.name: UpdateLastCacheFileActivityTimeTest
@tc.desc: test UpdateLastCacheFileActivityTime function.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, UpdateLastCacheFileActivityTimeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Test when waitingCacheFileStop_ is false
    preprocess.waitingCacheFileStop_ = false;
    auto initialTime = preprocess.lastCacheFileActivityTime_;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    preprocess.UpdateLastCacheFileActivityTime();
    EXPECT_EQ(preprocess.lastCacheFileActivityTime_, initialTime);
    
    // Test when waitingCacheFileStop_ is true
    preprocess.waitingCacheFileStop_ = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    preprocess.UpdateLastCacheFileActivityTime();
    EXPECT_GT(preprocess.lastCacheFileActivityTime_, initialTime);
}

/*
@tc.name: IsCacheFileInactiveWithTimeTest
@tc.desc: test IsCacheFileInactiveWithTime function.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, IsCacheFileInactiveWithTimeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Test when cache file is active
    preprocess.waitingCacheFileStop_ = true;
    preprocess.UpdateLastCacheFileActivityTime();
    EXPECT_FALSE(preprocess.IsCacheFileInactiveWithTime(1));
}

/*
@tc.name: StopCacheFileTaskTest
@tc.desc: test StopCacheFileTask function.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StopCacheFileTaskTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Test StopCacheFileTask
    preprocess.isStopTakeData_ = true;
    preprocess.StopCacheFileTask();
    EXPECT_TRUE(preprocess.isStopTakeData_);
    EXPECT_FALSE(preprocess.waitingCacheFileStop_);
}

/*
@tc.name: ProcessSingleRecordMunmapMsgTest
@tc.desc: test ProcessSingleRecord with MUNMAP_MSG type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordMunmapMsgTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MUNMAP_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x5000);
    rawStack->reportFlag = true;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.InitializeDirector();

    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_TRUE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: ProcessSingleRecordIgnoreReportFlagTest
@tc.desc: test ProcessSingleRecord with reportFlag set to false.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordIgnoreReportFlagTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->reportFlag = false;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    uint32_t initialIgnoreCnts = preprocess.ignoreCnts_;

    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_TRUE(result);
    EXPECT_EQ(preprocess.ignoreCnts_, initialIgnoreCnts + 1);
    delete rawStack->stackContext;
}

/*
@tc.name: CleanupBatchRecordsPartialArrayTest
@tc.desc: test CleanupBatchRecords with partially filled array.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, CleanupBatchRecordsPartialArrayTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    auto factory = std::make_shared<HookRecordFactory>(hookConfig);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SetFactory(factory);

    HookRecordPtr batchRawStack[MAX_BATCH_CNT] = {nullptr};
    BaseStackRawData* stackContexts[3] = {nullptr};

    for (int i = 0; i < 3; i++) {
        auto rawStack = std::make_shared<RawStack>();
        rawStack->stackContext = new BaseStackRawData();
        rawStack->stackContext->type = MALLOC_MSG;
        stackContexts[i] = rawStack->stackContext;
        batchRawStack[i] = std::make_shared<HookRecord>(rawStack);
    }

    preprocess.CleanupBatchRecords(batchRawStack, MAX_BATCH_CNT);

    for (int i = 0; i < 3; i++) {
        EXPECT_EQ(batchRawStack[i], nullptr);
        delete stackContexts[i];
    }
    for (int i = 3; i < MAX_BATCH_CNT; i++) {
        EXPECT_EQ(batchRawStack[i], nullptr);
    }
}

/*
@tc.name: HandleSimpleMessagesNullStackContextTest
@tc.desc: test HandleSimpleMessages behavior with different message types.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleSimpleMessagesNullStackContextTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MMAP_MSG;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    bool result = preprocess.HandleSimpleMessages(hookRecord);

    EXPECT_FALSE(result);
    delete rawStack->stackContext;
}

/*
@tc.name: ProcessSingleRecordWithReportFlagTest
@tc.desc: test ProcessSingleRecord with reportFlag set to true.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSingleRecordWithReportFlagTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->reportFlag = true;

    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.InitializeDirector();

    uint32_t initialEventCnts = preprocess.eventCnts_;
    bool result = preprocess.ProcessSingleRecord(hookRecord);

    EXPECT_TRUE(result);
    EXPECT_EQ(preprocess.eventCnts_, initialEventCnts + 1);
    delete rawStack->stackContext;
}

/*
@tc.name: FindAddrsEmptySetTest
@tc.desc: test FindAddrs with empty address set.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FindAddrsEmptySetTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    std::vector<uint64_t> result = preprocess.FindAddrs(0x1000, 100);
    EXPECT_TRUE(result.empty());
}

/*
@tc.name: FindAddrsSingleMatchTest
@tc.desc: test FindAddrs with single matching address.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FindAddrsSingleMatchTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // First add an address to allocAddrMap, using MEMORY_USING_MSG type to support FindAddrs
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->tagId = 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // Find the range containing this address
    std::vector<uint64_t> result = preprocess.FindAddrs(0x1000, 100);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0x1000);

    delete rawStack->stackContext;
}

/*
@tc.name: FindAddrsRangeMatchTest
@tc.desc: test FindAddrs with multiple addresses in range.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FindAddrsRangeMatchTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Add multiple addresses, using MEMORY_USING_MSG type to support FindAddrs
    for (uint64_t i = 0; i < 5; i++) {
        auto rawStack = std::make_shared<RawStack>();
        rawStack->stackContext = new BaseStackRawData();
        rawStack->stackContext->type = MEMORY_USING_MSG;
        rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000 + i * 100);
        rawStack->stackContext->mallocSize = 64;
        rawStack->stackContext->tagId = i + 1;
        auto hookRecord = std::make_shared<HookRecord>(rawStack);
        preprocess.SetAllocStatisticsData(hookRecord, i + 1);
        delete rawStack->stackContext;
    }

    // Find addresses within the range
    std::vector<uint64_t> result = preprocess.FindAddrs(0x1000, 500);
    EXPECT_EQ(result.size(), 5);
}

/*
@tc.name: FindAddrsNoMatchTest
@tc.desc: test FindAddrs with no matching addresses.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FindAddrsNoMatchTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Add an address, using MEMORY_USING_MSG type to support FindAddrs
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->tagId = 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // Find non-existent address range
    std::vector<uint64_t> result = preprocess.FindAddrs(0x2000, 100);
    EXPECT_TRUE(result.empty());

    delete rawStack->stackContext;
}

/*
@tc.name: HandleDeleteAddrSingleTest
@tc.desc: test HandleDeleteAddr with single address.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleDeleteAddrSingleTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // First add an address, using MEMORY_USING_MSG type to support FindAddrs
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->tagId = 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // Delete this address
    preprocess.HandleDeleteAddr(0x1000, 100);

    // Verify the address has been deleted
    std::vector<uint64_t> result = preprocess.FindAddrs(0x1000, 100);
    EXPECT_TRUE(result.empty());

    delete rawStack->stackContext;
}

/*
@tc.name: HandleDeleteAddrRangeTest
@tc.desc: test HandleDeleteAddr with address range.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleDeleteAddrRangeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Add multiple addresses, using MEMORY_USING_MSG type to support FindAddrs
    for (uint64_t i = 0; i < 5; i++) {
        auto rawStack = std::make_shared<RawStack>();
        rawStack->stackContext = new BaseStackRawData();
        rawStack->stackContext->type = MEMORY_USING_MSG;
        rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000 + i * 100);
        rawStack->stackContext->mallocSize = 64;
        rawStack->stackContext->tagId = 1;
        auto hookRecord = std::make_shared<HookRecord>(rawStack);
        preprocess.SetAllocStatisticsData(hookRecord, i + 1);
        delete rawStack->stackContext;
    }

    // Delete addresses within the range
    preprocess.HandleDeleteAddr(0x1000, 500);

    // Verify addresses within the range have been deleted
    std::vector<uint64_t> result = preprocess.FindAddrs(0x1000, 500);
    EXPECT_TRUE(result.empty());
}

/*
@tc.name: HandleDeleteAddrNotExistTest
@tc.desc: test HandleDeleteAddr with non-existent address.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleDeleteAddrNotExistTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Delete non-existent address, should not crash
    preprocess.HandleDeleteAddr(0x1000, 100);

    // Verify operation completed normally, no address was deleted
    std::vector<uint64_t> result = preprocess.FindAddrs(0x1000, 100);
    EXPECT_TRUE(result.empty());
}

/*
@tc.name: HandleMoveAddrExistTest
@tc.desc: test HandleMoveAddr with existing address.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleMoveAddrExistTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // First add an address, using MEMORY_USING_MSG type to support FindAddrs
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->tagId = 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // Move address
    preprocess.HandleMoveAddr(0x1000, 0x2000, 64);

    // Verify old address has been deleted
    std::vector<uint64_t> oldResult = preprocess.FindAddrs(0x1000, 100);
    EXPECT_TRUE(oldResult.empty());

    // Verify new address has been added
    std::vector<uint64_t> newResult = preprocess.FindAddrs(0x2000, 100);
    EXPECT_EQ(newResult.size(), 1);
    EXPECT_EQ(newResult[0], 0x2000);

    delete rawStack->stackContext;
}

/*
@tc.name: HandleMoveAddrNotExistTest
@tc.desc: test HandleMoveAddr with non-existent address.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleMoveAddrNotExistTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Move non-existent address, should not crash
    preprocess.HandleMoveAddr(0x1000, 0x2000, 64);

    // Verify operation completed normally, no address was added
    std::vector<uint64_t> result = preprocess.FindAddrs(0x2000, 100);
    EXPECT_TRUE(result.empty());
}

/*
@tc.name: HandleMoveAddrSizeChangeTest
@tc.desc: test HandleMoveAddr with size change.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleMoveAddrSizeChangeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // First add an address, using MEMORY_USING_MSG type to support FindAddrs
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->tagId = 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // Move address and change size
    preprocess.HandleMoveAddr(0x1000, 0x2000, 128);

    // Verify new address has been added
    std::vector<uint64_t> newResult = preprocess.FindAddrs(0x2000, 200);
    EXPECT_EQ(newResult.size(), 1);

    delete rawStack->stackContext;
}

/*
@tc.name: SetPidTest
@tc.desc: test SetPid method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetPidTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.SetPid(12345);
    EXPECT_EQ(preprocess.pid_, 12345);
}

/*
@tc.name: SetNmdFdTest
@tc.desc: test SetNmdFd method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetNmdFdTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.SetNmdFd(100);
    EXPECT_EQ(preprocess.nmdFd_, 100);
}

/*
@tc.name: SetFlushSizeTest
@tc.desc: test SetFlushSize method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetFlushSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.SetFlushSize(1000);
    EXPECT_EQ(preprocess.flushSize_, 100);
    EXPECT_EQ(preprocess.bufferSize_, 200);
}

/*
@tc.name: SetFreeStatisticsDataExistTest
@tc.desc: test SetFreeStatisticsData with existing address.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetFreeStatisticsDataExistTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // First allocate address, using MEMORY_USING_MSG type to support FindAddrs
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->tagId = 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // Free address
    bool result = preprocess.SetFreeStatisticsData(0x1000);
    EXPECT_TRUE(result);

    // Verify address has been removed from map
    std::vector<uint64_t> addrs = preprocess.FindAddrs(0x1000, 100);
    EXPECT_TRUE(addrs.empty());

    delete rawStack->stackContext;
}

/*
@tc.name: SetFreeStatisticsDataNotExistTest
@tc.desc: test SetFreeStatisticsData with non-existent address.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetFreeStatisticsDataNotExistTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Free non-existent address
    bool result = preprocess.SetFreeStatisticsData(0x1000);
    EXPECT_FALSE(result);
}

/*
@tc.name: SetFreeStatisticsDataMultipleTest
@tc.desc: test SetFreeStatisticsData with multiple addresses.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetFreeStatisticsDataMultipleTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Allocate multiple addresses
    for (uint64_t i = 0; i < 3; i++) {
        auto rawStack = std::make_shared<RawStack>();
        rawStack->stackContext = new BaseStackRawData();
        rawStack->stackContext->type = MALLOC_MSG;
        rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000 + i * 0x100);
        rawStack->stackContext->mallocSize = 64;
        rawStack->stackContext->pid = 12345;
        auto hookRecord = std::make_shared<HookRecord>(rawStack);
        preprocess.SetAllocStatisticsData(hookRecord, i + 1);
        delete rawStack->stackContext;
    }

    // Free multiple addresses consecutively
    EXPECT_TRUE(preprocess.SetFreeStatisticsData(0x1000));
    EXPECT_TRUE(preprocess.SetFreeStatisticsData(0x1100));
    EXPECT_TRUE(preprocess.SetFreeStatisticsData(0x1200));
}

/*
@tc.name: SetAllocStatisticsDataNewTest
@tc.desc: test SetAllocStatisticsData with new address.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetAllocStatisticsDataNewTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // First allocation of address
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    size_t initialSize = preprocess.recordStatisticsMap_.size();
    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // Verify statistics data has been created
    EXPECT_EQ(preprocess.recordStatisticsMap_.size(), initialSize + 1);

    delete rawStack->stackContext;
}

/*
@tc.name: UpdateAllocStatisticsSameSizeTest
@tc.desc: test UpdateAllocStatistics with same size allocation.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, UpdateAllocStatisticsSameSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // First allocation
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->pid = 12345;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // Get initial statistics data
    auto& stat = preprocess.recordStatisticsMap_[1];
    uint64_t initialApplyCount = stat.applyCount;
    uint64_t initialApplySize = stat.applySize;

    // Update statistics (same size)
    preprocess.UpdateAllocStatistics(hookRecord, 1);

    // Verify applyCount increases, applySize also increases (accumulating same size)
    EXPECT_EQ(stat.applyCount, initialApplyCount + 1);
    EXPECT_EQ(stat.applySize, initialApplySize + 64);

    delete rawStack->stackContext;
}

/*
@tc.name: UpdateAllocStatisticsDifferentSizeTest
@tc.desc: test UpdateAllocStatistics with different size allocation.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, UpdateAllocStatisticsDifferentSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // First allocation
    auto rawStack1 = std::make_shared<RawStack>();
    rawStack1->stackContext = new BaseStackRawData();
    rawStack1->stackContext->type = MALLOC_MSG;
    rawStack1->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack1->stackContext->mallocSize = 64;
    auto hookRecord1 = std::make_shared<HookRecord>(rawStack1);
    preprocess.SetAllocStatisticsData(hookRecord1, 1);

    // Get initial statistics data
    auto& stat = preprocess.recordStatisticsMap_[1];
    uint64_t initialApplyCount = stat.applyCount;
    uint64_t initialApplySize = stat.applySize;

    // Update statistics (different size)
    auto rawStack2 = std::make_shared<RawStack>();
    rawStack2->stackContext = new BaseStackRawData();
    rawStack2->stackContext->type = MALLOC_MSG;
    rawStack2->stackContext->addr = reinterpret_cast<void*>(0x2000);
    rawStack2->stackContext->mallocSize = 128;
    auto hookRecord2 = std::make_shared<HookRecord>(rawStack2);
    preprocess.UpdateAllocStatistics(hookRecord2, 1);

    // Verify both applyCount and applySize increase
    EXPECT_EQ(stat.applyCount, initialApplyCount + 1);
    EXPECT_GT(stat.applySize, initialApplySize);

    delete rawStack1->stackContext;
    delete rawStack2->stackContext;
}

/*
@tc.name: AddAllocStatisticsMallocTest
@tc.desc: test AddAllocStatistics with MALLOC type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AddAllocStatisticsMallocTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    size_t initialSize = preprocess.recordStatisticsMap_.size();
    preprocess.AddAllocStatistics(hookRecord, 1);

    // Verify statistics data has been added
    EXPECT_EQ(preprocess.recordStatisticsMap_.size(), initialSize + 1);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].type, RecordStatisticsEvent::MALLOC);

    delete rawStack->stackContext;
}

/*
@tc.name: AddAllocStatisticsMmapTest
@tc.desc: test AddAllocStatistics with MMAP type.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AddAllocStatisticsMmapTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MMAP_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 4096;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    preprocess.AddAllocStatistics(hookRecord, 1);

    // Verify type field is set correctly
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].type, RecordStatisticsEvent::MMAP);

    delete rawStack->stackContext;
}

/*
@tc.name: AddAllocStatisticsWithTagTest
@tc.desc: test AddAllocStatistics with tag information.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AddAllocStatisticsWithTagTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Use MEMORY_USING_MSG type to support tagId setting
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->pid = 12345;
    rawStack->stackContext->tagId = 100;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    preprocess.AddAllocStatistics(hookRecord, 1);

    // Verify tagId is set correctly
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].tagId, 100);

    delete rawStack->stackContext;
}

/*
@tc.name: FlushRecordStatisticsEmptyTest
@tc.desc: test FlushRecordStatistics with empty statistics data.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FlushRecordStatisticsEmptyTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Flush when there is no statistics data
    bool result = preprocess.FlushRecordStatistics();
    EXPECT_FALSE(result);
}

/*
@tc.name: HandleNoStackEventMallocTest
@tc.desc: test HandleNoStackEvent with MALLOC event.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleNoStackEventMallocTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    // MALLOC_MSG is not a type handled by HandleNoStackEvent, should return false
    bool result = preprocess.HandleNoStackEvent(hookRecord);
    EXPECT_FALSE(result);

    delete rawStack->stackContext;
}

/*
@tc.name: HandleNoStackEventFreeTest
@tc.desc: test HandleNoStackEvent with FREE event.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleNoStackEventFreeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Create hookRecord of FREE_MSG type
    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = FREE_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 0;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    // FREE_MSG is not a type handled by HandleNoStackEvent, should return false
    bool result = preprocess.HandleNoStackEvent(hookRecord);
    EXPECT_FALSE(result);

    delete rawStack->stackContext;
}

/*
@tc.name: ResolveDmaSizeUpdatesSizeTest
@tc.desc: test ResolveDmaSize updates DMA using event size from batch map.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ResolveDmaSizeUpdatesSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_filter_size(1024);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    constexpr uint32_t testPid = 1234;
    constexpr uint32_t testFd = 5;
    constexpr size_t testDmaSize = 4096;
    auto rawStack = MakeRawStack(0, 0, DMABUF_MASK_INDEX + 1);
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->pid = testPid;
    rawStack->stackContext->addr = reinterpret_cast<void*>(testFd | RES_DMABUF_MASK);
    rawStack->stackContext->mallocSize = 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    StackPreprocess::DmaFdSizeMap dmaFdSizeMap;
    dmaFdSizeMap[testFd] = testDmaSize;
    uint32_t dmaFdSizePid = testPid;
    bool result = preprocess.ResolveDmaSize(hookRecord, dmaFdSizeMap, dmaFdSizePid);

    EXPECT_TRUE(result);
    EXPECT_EQ(rawStack->stackContext->mallocSize, testDmaSize);
}

/*
@tc.name: ResolveDmaSizeRejectsFilteredSizeTest
@tc.desc: test ResolveDmaSize rejects DMA using event when resolved size is below filter.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ResolveDmaSizeRejectsFilteredSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_filter_size(1024);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    constexpr uint32_t testPid = 1234;
    constexpr uint32_t testFd = 5;
    auto rawStack = MakeRawStack(0, 0, DMABUF_MASK_INDEX + 1);
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->pid = testPid;
    rawStack->stackContext->addr = reinterpret_cast<void*>(testFd | RES_DMABUF_MASK);
    rawStack->stackContext->mallocSize = 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    StackPreprocess::DmaFdSizeMap dmaFdSizeMap;
    dmaFdSizeMap[testFd] = 512;
    uint32_t dmaFdSizePid = testPid;
    bool result = preprocess.ResolveDmaSize(hookRecord, dmaFdSizeMap, dmaFdSizePid);

    EXPECT_FALSE(result);
    EXPECT_EQ(rawStack->stackContext->mallocSize, 1U);
}

/*
@tc.name: HandleNoStackEventMmapTest
@tc.desc: test HandleNoStackEvent with MMAP event.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleNoStackEventMmapTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MMAP_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 4096;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    // MMAP_MSG is not a type handled by HandleNoStackEvent, should return false
    bool result = preprocess.HandleNoStackEvent(hookRecord);
    EXPECT_FALSE(result);

    delete rawStack->stackContext;
}

/*
@tc.name: HandleNoStackEventMmapFileTypeTest
@tc.desc: test HandleNoStackEvent with MMAP_FILE_TYPE event.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleNoStackEventMmapFileTypeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MMAP_FILE_TYPE;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 4096;
    rawStack->stackContext->pid = 12345;
    rawStack->stackContext->tid = 67890;
    rawStack->stackContext->mmapArgs.flags = 0x01;
    rawStack->stackContext->mmapArgs.offset = 0;

    // Set file path data
    const char* filePath = "/system/lib/libc.so";
    size_t pathLen = strlen(filePath) + 1;
    rawStack->data = new uint8_t[pathLen];
    memcpy_s(rawStack->data, pathLen, filePath, pathLen);

    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    // MMAP_FILE_TYPE is a type handled by HandleNoStackEvent, should return true
    bool result = preprocess.HandleNoStackEvent(hookRecord);
    EXPECT_TRUE(result);

    delete[] rawStack->data;
    delete rawStack->stackContext;
}

/*
@tc.name: HandleNoStackEventThreadNameTest
@tc.desc: test HandleNoStackEvent with THREAD_NAME_MSG event.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleNoStackEventThreadNameTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = THREAD_NAME_MSG;
    rawStack->stackContext->tid = 12345;

    // Set thread name data
    const char* threadName = "TestThread";
    size_t nameLen = strlen(threadName) + 1;
    rawStack->data = new uint8_t[nameLen];
    memcpy_s(rawStack->data, nameLen, threadName, nameLen);

    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    // THREAD_NAME_MSG is a type handled by HandleNoStackEvent, should return true
    bool result = preprocess.HandleNoStackEvent(hookRecord);
    EXPECT_TRUE(result);

    delete[] rawStack->data;
    delete rawStack->stackContext;
}

/*
@tc.name: ForceStopTest
@tc.desc: test ForceStop method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ForceStopTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // Initial state should be false
    EXPECT_FALSE(preprocess.isStopTakeData_.load());

    // Call ForceStop
    preprocess.ForceStop();

    // Verify isStopTakeData_ is set to true
    EXPECT_TRUE(preprocess.isStopTakeData_.load());
}

/*
@tc.name: SetPidAndGetPidTest
@tc.desc: test SetPid and verify pid is set correctly.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetPidAndGetPidTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    pid_t testPid = 99999;
    preprocess.SetPid(testPid);

    EXPECT_EQ(preprocess.pid_, testPid);
}

/*
@tc.name: SetNmdFdAndGetNmdFdTest
@tc.desc: test SetNmdFd and verify nmdFd is set correctly.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetNmdFdAndGetNmdFdTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    int testFd = 12345;
    preprocess.SetNmdFd(testFd);

    EXPECT_EQ(preprocess.nmdFd_, testFd);
}

/*
@tc.name: SetFlushSizeAndVerifyTest
@tc.desc: test SetFlushSize and verify buffer sizes are calculated correctly.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetFlushSizeAndVerifyTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    uint32_t testSize = 2000;
    preprocess.SetFlushSize(testSize);

    // flushSize_ = testSize / 10
    EXPECT_EQ(preprocess.flushSize_, testSize / 10);
    // bufferSize_ = flushSize_ * 2
    EXPECT_EQ(preprocess.bufferSize_, (testSize / 10) * 2);
}

/*
@tc.name: SetEventFrameCompressedAllocSetsStackAndSize
@tc.desc: compressed alloc event should carry stack id, thread name id and size.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetEventFrameCompressedAllocSetsStackAndSize, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint32_t stackId = 7;
    ReportEventBaseData rawStack = MakeReportEvent(MALLOC_MSG, 0x1000, stackId);
    AllocEvent allocEvent;

    preprocess.SetEventFrame(rawStack, &allocEvent, stackId);

    EXPECT_EQ(allocEvent.pid(), -1);
    EXPECT_EQ(allocEvent.tid(), rawStack.tid);
    EXPECT_EQ(allocEvent.addr(), rawStack.addr);
    EXPECT_EQ(allocEvent.size(), rawStack.mallocSize);
    EXPECT_EQ(allocEvent.thread_name_id(), rawStack.tid);
    EXPECT_EQ(allocEvent.stack_id(), stackId);
}

/*
@tc.name: SetEventFrameFreeSkipsSizeField
@tc.desc: free event should set common fields and stack id without size handling.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetEventFrameFreeSkipsSizeField, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint32_t stackId = 9;
    ReportEventBaseData rawStack = MakeReportEvent(FREE_MSG, 0x2000, stackId);
    FreeEvent freeEvent;

    preprocess.SetEventFrame(rawStack, &freeEvent, stackId);

    EXPECT_EQ(freeEvent.pid(), -1);
    EXPECT_EQ(freeEvent.tid(), rawStack.tid);
    EXPECT_EQ(freeEvent.addr(), rawStack.addr);
    EXPECT_EQ(freeEvent.thread_name_id(), rawStack.tid);
    EXPECT_EQ(freeEvent.stack_id(), stackId);
}

/*
@tc.name: SetEventFrameMmapSetsTypeAndSkipsZeroStackId
@tc.desc: mmap event should set type while zero stack id keeps stack_id unset.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetEventFrameMmapSetsTypeAndSkipsZeroStackId, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    ReportEventBaseData rawStack = MakeReportEvent(MMAP_FILE_PAGE_MSG, 0x3000, 0);
    MmapEvent mmapEvent;

    preprocess.SetEventFrame(rawStack, &mmapEvent, 0, "FilePage:ashmem");

    EXPECT_EQ(mmapEvent.type(), "FilePage:ashmem");
    EXPECT_EQ(mmapEvent.size(), rawStack.mallocSize);
    EXPECT_EQ(mmapEvent.thread_name_id(), rawStack.tid);
    EXPECT_EQ(mmapEvent.stack_id(), 0u);
}

/*
@tc.name: AddAllocStatisticsMallocCollectsAddrTest
@tc.desc: AddAllocStatistics with MALLOC type should collect addr in full mode.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AddAllocStatisticsMallocCollectsAddrTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    preprocess.AddAllocStatistics(hookRecord, 1);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].type, RecordStatisticsEvent::MALLOC);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs.size(), 1u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs[0], 0x1000);

    delete rawStack->stackContext;
}

/*
@tc.name: AddAllocStatisticsMallocAddrSimplifiedTest
@tc.desc: AddAllocStatistics with MALLOC type should collect addr in simplified mode.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AddAllocStatisticsMallocAddrSimplifiedTest, TestSize.Level0)
{
    NativeHookConfig hookConfig; // arkui_mem_trace_enable defaults to false -> simplified mode
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x2000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    preprocess.AddAllocStatistics(hookRecord, 1);
    EXPECT_EQ(preprocess.simplifiedRecordStatisticsMap_[1].type, RecordStatisticsEvent::MALLOC);
    EXPECT_EQ(preprocess.simplifiedRecordStatisticsMap_[1].addrs.size(), 1u);
    EXPECT_EQ(preprocess.simplifiedRecordStatisticsMap_[1].addrs[0], 0x2000);

    delete rawStack->stackContext;
}

/*
@tc.name: UpdateAllocStatisticsCollectsAddrTest
@tc.desc: UpdateAllocStatistics should collect addr for MALLOC type on update path.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, UpdateAllocStatisticsCollectsAddrTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack1 = std::make_shared<RawStack>();
    rawStack1->stackContext = new BaseStackRawData();
    rawStack1->stackContext->type = MALLOC_MSG;
    rawStack1->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack1->stackContext->mallocSize = 64;
    auto hookRecord1 = std::make_shared<HookRecord>(rawStack1);
    preprocess.SetAllocStatisticsData(hookRecord1, 1);

    auto rawStack2 = std::make_shared<RawStack>();
    rawStack2->stackContext = new BaseStackRawData();
    rawStack2->stackContext->type = MALLOC_MSG;
    rawStack2->stackContext->addr = reinterpret_cast<void*>(0x2000);
    rawStack2->stackContext->mallocSize = 64;
    auto hookRecord2 = std::make_shared<HookRecord>(rawStack2);
    preprocess.UpdateAllocStatistics(hookRecord2, 1);

    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs.size(), 2u);

    delete rawStack1->stackContext;
    delete rawStack2->stackContext;
}

/*
@tc.name: AddAllocStatisticsMmapNoAddrTest
@tc.desc: AddAllocStatistics with MMAP type should not collect addr.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AddAllocStatisticsMmapNoAddrTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MMAP_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 4096;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    preprocess.AddAllocStatistics(hookRecord, 1);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].type, RecordStatisticsEvent::MMAP);
    EXPECT_TRUE(preprocess.recordStatisticsMap_[1].addrs.empty());

    delete rawStack->stackContext;
}

/*
@tc.name: SetFreeStatisticsDataRemovesAddrTest
@tc.desc: SetFreeStatisticsData should remove addr from record in full mode.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetFreeStatisticsDataRemovesAddrTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs.size(), 1u);

    bool result = preprocess.SetFreeStatisticsData(0x1000);
    EXPECT_TRUE(result);
    EXPECT_TRUE(preprocess.recordStatisticsMap_[1].addrs.empty());

    delete rawStack->stackContext;
}

/*
@tc.name: SetFreeStatisticsDataRemovesAddrSimplifiedTest
@tc.desc: SetFreeStatisticsData should remove addr from record in simplified mode.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, SetFreeStatisticsDataRemovesAddrSimplifiedTest, TestSize.Level0)
{
    NativeHookConfig hookConfig; // simplified mode
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);
    EXPECT_EQ(preprocess.simplifiedRecordStatisticsMap_[1].addrs.size(), 1u);

    bool result = preprocess.SetFreeStatisticsData(0x1000);
    EXPECT_TRUE(result);
    EXPECT_TRUE(preprocess.simplifiedRecordStatisticsMap_[1].addrs.empty());

    delete rawStack->stackContext;
}

/*
@tc.name: AddAllocStatisticsArkHandleCollectsAddrTest
@tc.desc: AddAllocStatistics with ARK_GLOBAL_HANDLE tag should collect addr.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AddAllocStatisticsArkHandleCollectsAddrTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.InitializeGpuData(); // ensure ARK handle tag registered

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x3000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->tagId = ARK_GLOBAL_HANDLE_INDEX + 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    preprocess.AddAllocStatistics(hookRecord, 1);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].type, RecordStatisticsEvent::ARK_GLOBAL_HANDLE);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs.size(), 1u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs[0], 0x3000);

    delete rawStack->stackContext;
}

/*
@tc.name: FlushRecordStatisticsWithAddrTest
@tc.desc: FlushRecordStatistics should run flush path (incl add_addr) without crash when addr collected.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FlushRecordStatisticsWithAddrTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    hookConfig.set_statistics_interval(10);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);
    EXPECT_FALSE(preprocess.statisticsPeriodSet_.empty());

    bool result = preprocess.FlushRecordStatistics();
    EXPECT_TRUE(result);
    EXPECT_TRUE(preprocess.statisticsPeriodSet_.empty());

    // verify addr is flushed into statistics_event
    auto* batchData = std::get_if<BatchNativeHookData>(&preprocess.stackData_);
    ASSERT_NE(batchData, nullptr);
    bool foundAddr = false;
    for (const auto& evt : batchData->events()) {
        if (evt.has_statistics_event() && evt.statistics_event().addr_size() > 0) {
            EXPECT_EQ(evt.statistics_event().addr(0), 0x1000);
            foundAddr = true;
            break;
        }
    }
    EXPECT_TRUE(foundAddr);

    delete rawStack->stackContext;
}

/*
@tc.name: AddAllocStatisticsArkLocalHandleCollectsAddrTest
@tc.desc: AddAllocStatistics with ARK_LOCAL_HANDLE tag should collect addr.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, AddAllocStatisticsArkLocalHandleCollectsAddrTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.InitializeGpuData(); // ensure ARK handle tag registered

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x4000);
    rawStack->stackContext->mallocSize = 64;
    rawStack->stackContext->tagId = ARK_LOCAL_HANDLE_INDEX + 1;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);

    preprocess.AddAllocStatistics(hookRecord, 1);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].type, RecordStatisticsEvent::ARK_LOCAL_HANDLE);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs.size(), 1u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs[0], 0x4000);

    delete rawStack->stackContext;
}

/*
@tc.name: FlushRecordStatisticsWithMultiAddrTest
@tc.desc: FlushRecordStatistics should flush all accumulated addrs into statistics_event.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, FlushRecordStatisticsWithMultiAddrTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    hookConfig.set_statistics_interval(10);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    // first alloc -> addr 0x1000
    auto rawStack1 = std::make_shared<RawStack>();
    rawStack1->stackContext = new BaseStackRawData();
    rawStack1->stackContext->type = MALLOC_MSG;
    rawStack1->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack1->stackContext->mallocSize = 64;
    auto hookRecord1 = std::make_shared<HookRecord>(rawStack1);
    preprocess.SetAllocStatisticsData(hookRecord1, 1);

    // update -> addr 0x2000 accumulated
    auto rawStack2 = std::make_shared<RawStack>();
    rawStack2->stackContext = new BaseStackRawData();
    rawStack2->stackContext->type = MALLOC_MSG;
    rawStack2->stackContext->addr = reinterpret_cast<void*>(0x2000);
    rawStack2->stackContext->mallocSize = 64;
    auto hookRecord2 = std::make_shared<HookRecord>(rawStack2);
    preprocess.UpdateAllocStatistics(hookRecord2, 1);

    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs.size(), 2u);

    bool result = preprocess.FlushRecordStatistics();
    EXPECT_TRUE(result);

    // verify both addrs flushed into the same statistics_event
    auto* batchData = std::get_if<BatchNativeHookData>(&preprocess.stackData_);
    ASSERT_NE(batchData, nullptr);
    bool found = false;
    for (const auto& evt : batchData->events()) {
        if (evt.has_statistics_event() && evt.statistics_event().callstack_id() == 1) {
            EXPECT_EQ(evt.statistics_event().addr_size(), 2u);
            EXPECT_EQ(evt.statistics_event().addr(0), 0x1000);
            EXPECT_EQ(evt.statistics_event().addr(1), 0x2000);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);

    delete rawStack1->stackContext;
    delete rawStack2->stackContext;
}

/*
@tc.name: HandleMoveAddrReplacesAddrInStatisticsTest
@tc.desc: HandleMoveAddr should replace old addr with newAddr in record.addrs.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleMoveAddrReplacesAddrInStatisticsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);
    ASSERT_EQ(preprocess.recordStatisticsMap_[1].addrs.size(), 1u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs[0], 0x1000);

    // move 0x1000 -> 0x2000
    preprocess.HandleMoveAddr(0x1000, 0x2000, 128);

    // addrs replaced: old 0x1000 gone, new 0x2000 present
    ASSERT_EQ(preprocess.recordStatisticsMap_[1].addrs.size(), 1u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[1].addrs[0], 0x2000);

    delete rawStack->stackContext;
}

/*
@tc.name: HandleMoveAddrFlushAndFreeTest
@tc.desc: After move, flush should report newAddr (not stale oldAddr) and free(newAddr) should remove it from addrs.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, HandleMoveAddrFlushAndFreeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_arkui_mem_trace_enable(true);
    hookConfig.set_statistics_interval(10);
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    auto rawStack = std::make_shared<RawStack>();
    rawStack->stackContext = new BaseStackRawData();
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x1000);
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<HookRecord>(rawStack);
    preprocess.SetAllocStatisticsData(hookRecord, 1);

    // move 0x1000 -> 0x2000
    preprocess.HandleMoveAddr(0x1000, 0x2000, 128);

    // flush should report newAddr 0x2000, not stale 0x1000
    EXPECT_TRUE(preprocess.FlushRecordStatistics());
    auto* batchData = std::get_if<BatchNativeHookData>(&preprocess.stackData_);
    ASSERT_NE(batchData, nullptr);
    bool found = false;
    for (const auto& evt : batchData->events()) {
        if (evt.has_statistics_event() && evt.statistics_event().callstack_id() == 1) {
            ASSERT_EQ(evt.statistics_event().addr_size(), 1u);
            EXPECT_EQ(evt.statistics_event().addr(0), 0x2000);
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);

    // free(newAddr) should remove it from addrs (proves newAddr is findable after move)
    EXPECT_TRUE(preprocess.SetFreeStatisticsData(0x2000));
    EXPECT_TRUE(preprocess.recordStatisticsMap_[1].addrs.empty());

    delete rawStack->stackContext;
}

class MockWriter : public Writer {
public:
    long Write(const void* data, size_t size) override
    {
        writeCalled_ = true;
        writeSize_ = size;
        return size;
    }
    long WriteBlock(const void* data, size_t size) override
    {
        writeBlockCalled_ = true;
        writeBlockSize_ = size;
        return size;
    }
    bool Flush() override
    {
        flushCalled_ = true;
        return true;
    }
    bool writeCalled_ = false;
    bool writeBlockCalled_ = false;
    bool flushCalled_ = false;
    size_t writeSize_ = 0;
    size_t writeBlockSize_ = 0;
};

/*
 * @tc.name: FlushWithBlockModeEnabled
 * @tc.desc: test Flush calls WriteBlock when hookConfig.blocked is true.
 * @tc.type: FUNC
 */
HWTEST_F(StackPreprocessTest, FlushWithBlockModeEnabled, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_blocked(true);
    StackPreprocess preprocess(nullptr, config, 0);

    auto mockWriter = std::make_shared<MockWriter>();
    preprocess.SetWriter(std::shared_ptr<Writer>(mockWriter));

    uint8_t testData[] = {1, 2, 3, 4};
    preprocess.Flush(testData, sizeof(testData));

    EXPECT_TRUE(mockWriter->writeBlockCalled_);
    EXPECT_FALSE(mockWriter->writeCalled_);
    EXPECT_TRUE(mockWriter->flushCalled_);
    EXPECT_EQ(mockWriter->writeBlockSize_, sizeof(testData));
}

/*
 * @tc.name: FlushWithBlockModeDisabled
 * @tc.desc: test Flush calls Write when hookConfig.blocked is false.
 * @tc.type: FUNC
 */
HWTEST_F(StackPreprocessTest, FlushWithBlockModeDisabled, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_blocked(false);
    StackPreprocess preprocess(nullptr, config, 0);

    auto mockWriter = std::make_shared<MockWriter>();
    preprocess.SetWriter(std::shared_ptr<Writer>(mockWriter));

    uint8_t testData[] = {1, 2, 3, 4};
    preprocess.Flush(testData, sizeof(testData));

    EXPECT_TRUE(mockWriter->writeCalled_);
    EXPECT_FALSE(mockWriter->writeBlockCalled_);
    EXPECT_TRUE(mockWriter->flushCalled_);
    EXPECT_EQ(mockWriter->writeSize_, sizeof(testData));
}

/*
 * @tc.name: FlushWithNullSrc
 * @tc.desc: test Flush with null src pointer returns early.
 * @tc.type: FUNC
 */
HWTEST_F(StackPreprocessTest, FlushWithNullSrc, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_blocked(true);
    StackPreprocess preprocess(nullptr, config, 0);

    auto mockWriter = std::make_shared<MockWriter>();
    preprocess.SetWriter(std::shared_ptr<Writer>(mockWriter));

    preprocess.Flush(nullptr, 100);

    EXPECT_FALSE(mockWriter->writeCalled_);
    EXPECT_FALSE(mockWriter->writeBlockCalled_);
    EXPECT_FALSE(mockWriter->flushCalled_);
}

/*
 * @tc.name: FlushWithNullWriter
 * @tc.desc: test Flush with null writer returns early.
 * @tc.type: FUNC
 */
HWTEST_F(StackPreprocessTest, FlushWithNullWriter, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_blocked(true);
    StackPreprocess preprocess(nullptr, config, 0);

    uint8_t testData[] = {1, 2, 3, 4};
    preprocess.Flush(testData, sizeof(testData));
}

HWTEST_F(StackPreprocessTest, SetEventFrameTraceEventsCoverStackIdBranches, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    ReportEventBaseData rawStack = MakeReportEvent(MEMORY_USING_MSG, 0x3800, 3);
    TraceAllocEvent traceAllocEvent;
    TraceFreeEvent traceFreeEvent;

    preprocess.SetEventFrame(rawStack, &traceAllocEvent, 3, "gpu-vk");
    preprocess.SetEventFrame(rawStack, &traceFreeEvent, 0);

    EXPECT_EQ(traceAllocEvent.addr(), rawStack.addr);
    EXPECT_EQ(traceAllocEvent.size(), rawStack.mallocSize);
    EXPECT_EQ(traceAllocEvent.stack_id(), 3u);
    EXPECT_EQ(traceFreeEvent.addr(), rawStack.addr);
    EXPECT_EQ(traceFreeEvent.stack_id(), 0u);
}

HWTEST_F(StackPreprocessTest, SetEventFrameUncompressedSkipsStackId, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(false);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint32_t stackId = 11;
    ReportEventBaseData rawStack = MakeReportEvent(MALLOC_MSG, 0x4000, stackId);
    AllocEvent allocEvent;

    preprocess.SetEventFrame(rawStack, &allocEvent, stackId);

    EXPECT_EQ(allocEvent.pid(), -1);
    EXPECT_EQ(allocEvent.tid(), rawStack.tid);
    EXPECT_EQ(allocEvent.addr(), rawStack.addr);
    EXPECT_EQ(allocEvent.size(), rawStack.mallocSize);
    EXPECT_EQ(allocEvent.thread_name_id(), rawStack.tid);
    EXPECT_EQ(allocEvent.stack_id(), 0u);
}

HWTEST_F(StackPreprocessTest, SetEventFrameCompressedFreeSkipsZeroStackId, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    ReportEventBaseData rawStack = MakeReportEvent(FREE_MSG, 0x4100, 0);
    FreeEvent freeEvent;

    preprocess.SetEventFrame(rawStack, &freeEvent, 0);

    EXPECT_EQ(freeEvent.pid(), -1);
    EXPECT_EQ(freeEvent.thread_name_id(), rawStack.tid);
    EXPECT_EQ(freeEvent.stack_id(), 0u);
}

HWTEST_F(StackPreprocessTest, ReportFrameMapSkipsUnsetReportFlag, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    CallFrame callFrame(0x1000);
    callFrame.callFrameId_ = 3;
    BatchNativeHookData data;

    preprocess.ReportFrameMap(callFrame, data);

    EXPECT_EQ(data.events_size(), 0);
}

HWTEST_F(StackPreprocessTest, ReportFrameMapReportsFirstDwarfError, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(false);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    CallFrame callFrame(0x2000);
    callFrame.callFrameId_ = DWARF_ERROR_ID;
    callFrame.needReport_ = CALL_FRAME_REPORT;
    BatchNativeHookData data;

    preprocess.ReportFrameMap(callFrame, data);

    ASSERT_EQ(data.events_size(), 1);
    EXPECT_FALSE(preprocess.unwindFailReport_);
    EXPECT_EQ(data.events(0).frame_map().id(), DWARF_ERROR_ID);
}

HWTEST_F(StackPreprocessTest, ReportFrameMapSkipsRepeatedDwarfError, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(false);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.unwindFailReport_ = false;
    CallFrame callFrame(0x3000);
    callFrame.callFrameId_ = DWARF_ERROR_ID;
    callFrame.needReport_ = CALL_FRAME_REPORT;
    BatchNativeHookData data;

    preprocess.ReportFrameMap(callFrame, data);

    EXPECT_EQ(data.events_size(), 0);
    EXPECT_FALSE(preprocess.unwindFailReport_);
}

HWTEST_F(StackPreprocessTest, ReportFrameMapReportsNonErrorDwarfFrame, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(false);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    CallFrame callFrame(0x3100);
    callFrame.callFrameId_ = 23;
    callFrame.needReport_ = CALL_FRAME_REPORT;
    BatchNativeHookData data;

    preprocess.ReportFrameMap(callFrame, data);

    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(preprocess.unwindFailReport_);
    EXPECT_EQ(data.events(0).frame_map().id(), callFrame.callFrameId_);
}

HWTEST_F(StackPreprocessTest, ReportFrameMapReportsDwarfErrorInFpUnwind, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    CallFrame callFrame(0x3200);
    callFrame.callFrameId_ = DWARF_ERROR_ID;
    callFrame.needReport_ = CALL_FRAME_REPORT;
    BatchNativeHookData data;

    preprocess.ReportFrameMap(callFrame, data);

    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(preprocess.unwindFailReport_);
    EXPECT_EQ(data.events(0).frame_map().id(), DWARF_ERROR_ID);
}

HWTEST_F(StackPreprocessTest, SetFrameInfoOfflineJsStoresCompressedIds, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    Frame frame;
    CallFrame callFrame(0x4000);
    callFrame.isJsFrame_ = true;
    callFrame.sp_ = 0x4100;
    callFrame.offset_ = 0x20;
    callFrame.symbolOffset_ = 0x30;
    callFrame.symbolNameId_ = 5;
    callFrame.filePathId_ = 6;

    preprocess.SetFrameInfo(frame, callFrame);

    EXPECT_EQ(frame.ip(), callFrame.ip_);
    EXPECT_EQ(frame.sp(), callFrame.sp_);
    EXPECT_EQ(frame.offset(), callFrame.offset_);
    EXPECT_EQ(frame.symbol_offset(), callFrame.symbolOffset_);
    EXPECT_EQ(frame.symbol_name_id(), callFrame.symbolNameId_);
    EXPECT_EQ(frame.file_path_id(), callFrame.filePathId_);
    EXPECT_EQ(frame.symbol_name(), "");
    EXPECT_EQ(frame.file_path(), "");
}

HWTEST_F(StackPreprocessTest, SetFrameInfoOfflineSkipsNonJsFrame, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    Frame frame;
    CallFrame callFrame(0x4300);
    callFrame.sp_ = 0x4310;
    callFrame.symbolNameId_ = 8;
    callFrame.filePathId_ = 9;

    preprocess.SetFrameInfo(frame, callFrame);

    EXPECT_EQ(frame.ip(), callFrame.ip_);
    EXPECT_EQ(frame.sp(), 0u);
    EXPECT_EQ(frame.symbol_name_id(), 0u);
    EXPECT_EQ(frame.file_path_id(), 0u);
}

HWTEST_F(StackPreprocessTest, SetFrameInfoOfflineSkipsJsWithoutIds, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    Frame frame;
    CallFrame callFrame(0x4400);
    callFrame.isJsFrame_ = true;
    callFrame.sp_ = 0x4410;
    callFrame.symbolNameId_ = 0;
    callFrame.filePathId_ = 9;

    preprocess.SetFrameInfo(frame, callFrame);

    EXPECT_EQ(frame.ip(), callFrame.ip_);
    EXPECT_EQ(frame.sp(), 0u);
    EXPECT_EQ(frame.symbol_name_id(), 0u);
    EXPECT_EQ(frame.file_path_id(), 0u);
}

HWTEST_F(StackPreprocessTest, SetFrameInfoOnlineStoresCompressedIds, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    Frame frame;
    CallFrame callFrame(0x4500);
    callFrame.sp_ = 0x4510;
    callFrame.filePath_ = "libcompressed.so";
    callFrame.symbolName_ = "CompressedFunc";
    callFrame.symbolNameId_ = 10;
    callFrame.filePathId_ = 11;

    preprocess.SetFrameInfo(frame, callFrame);

    EXPECT_EQ(frame.ip(), callFrame.ip_);
    EXPECT_EQ(frame.sp(), callFrame.sp_);
    EXPECT_EQ(frame.symbol_name(), "");
    EXPECT_EQ(frame.file_path(), "libcompressed.so");
    EXPECT_EQ(frame.symbol_name_id(), callFrame.symbolNameId_);
    EXPECT_EQ(frame.file_path_id(), callFrame.filePathId_);
}

HWTEST_F(StackPreprocessTest, SetFrameInfoOnlineStoresNamesWithoutCompressedIds, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    Frame frame;
    CallFrame callFrame(0x4600);
    callFrame.sp_ = 0x4610;
    callFrame.offset_ = 0x40;
    callFrame.symbolOffset_ = 0x50;
    callFrame.filePath_ = "libplain.so";
    callFrame.symbolName_ = "PlainFunc";

    preprocess.SetFrameInfo(frame, callFrame);

    EXPECT_EQ(frame.ip(), callFrame.ip_);
    EXPECT_EQ(frame.sp(), callFrame.sp_);
    EXPECT_EQ(frame.symbol_name(), "PlainFunc");
    EXPECT_EQ(frame.file_path(), "libplain.so");
    EXPECT_EQ(frame.symbol_name_id(), 0u);
    EXPECT_EQ(frame.file_path_id(), 0u);
}

HWTEST_F(StackPreprocessTest, SetHookDataReportsStringMapsWithoutCallframeCompress, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_string_compressed(true);
    hookConfig.set_callframe_compress(false);
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, "napi_alloc");
    auto rawStack = MakeRawStack(2, 9, NAPI_TEST_TAG_ID);
    rawStack->stackContext->type = MEMORY_USING_MSG;
    rawStack->stackContext->mallocSize = 64;
    auto hookRecord = std::make_shared<MemoryUsingRecord>(rawStack);
    auto frames = MakeFrames(0x5000);
    frames[0].symbolName_ = "UseTag";
    frames[0].filePath_ = "entry.abc";
    frames[0].symbolNameId_ = 10;
    frames[0].filePathId_ = 11;
    frames[0].needReport_ = SYMBOL_NAME_ID_REPORT | FILE_PATH_ID_REPORT;
    BatchNativeHookData data;

    preprocess.SetHookData(hookRecord, frames, data);

    ASSERT_EQ(data.events_size(), 3);
    EXPECT_TRUE(data.events(0).has_symbol_name());
    EXPECT_EQ(data.events(0).symbol_name().name(), "UseTag");
    EXPECT_TRUE(data.events(1).has_file_path());
    EXPECT_EQ(data.events(1).file_path().name(), "entry.abc");
    EXPECT_TRUE(data.events(2).has_trace_alloc_event());
    EXPECT_EQ(data.events(2).trace_alloc_event().tag_name(), "napi_alloc");
}

HWTEST_F(StackPreprocessTest, SetHookDataSkipsCallstackCompressForMemoryTag, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_string_compressed(true);
    hookConfig.set_callframe_compress(true);
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto rawStack = MakeRawStack(0, 0, 0);
    rawStack->stackContext->type = MEMORY_TAG;
    auto hookRecord = std::make_shared<TagRecord>(rawStack);
    auto frames = MakeFrames(0x6000);
    frames[0].symbolName_ = "TagFrame";
    frames[0].filePath_ = "tag.abc";
    frames[0].symbolNameId_ = 12;
    frames[0].filePathId_ = 13;
    frames[0].needReport_ = SYMBOL_NAME_ID_REPORT | FILE_PATH_ID_REPORT;
    BatchNativeHookData data;

    preprocess.SetHookData(hookRecord, frames, data);

    ASSERT_EQ(data.events_size(), 3);
    EXPECT_TRUE(data.events(0).has_symbol_name());
    EXPECT_TRUE(data.events(1).has_file_path());
    EXPECT_FALSE(data.events(2).has_stack_map());
}

HWTEST_F(StackPreprocessTest, SetHookDataReportsTagForMemoryUnusing, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, "unused_tag");
    auto rawStack = MakeRawStack(0, 0, NAPI_TEST_TAG_ID);
    rawStack->stackContext->type = MEMORY_UNUSING_MSG;
    auto hookRecord = std::make_shared<MemoryUnusingRecord>(rawStack);
    auto frames = MakeFrames(0x7000);
    BatchNativeHookData data;

    preprocess.SetHookData(hookRecord, frames, data);

    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(data.events(0).has_trace_free_event());
    EXPECT_EQ(data.events(0).trace_free_event().tag_name(), "unused_tag");
}

HWTEST_F(StackPreprocessTest, SetHookDataReportsTagForArktsMalloc, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, TAG_RES_ARKTS_HEAP_MASK);
    auto rawStack = MakeRawStack(0, 0, NAPI_TEST_TAG_ID);
    rawStack->stackContext->type = MALLOC_ARKTS;
    rawStack->stackContext->mallocSize = 32;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x7100);
    rawStack->stackContext->newAddr = reinterpret_cast<void*>(0x7200);
    auto hookRecord = std::make_shared<ArktsMallocRecord>(rawStack);
    auto frames = MakeFrames(0x7100);
    BatchNativeHookData data;

    preprocess.SetHookData(hookRecord, frames, data);

    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(data.events(0).has_trace_move_event());
    EXPECT_EQ(data.events(0).trace_move_event().trace_type(), TraceType::ARKTS_HEAP);
    EXPECT_EQ(data.events(0).trace_move_event().addr(), 0x7100);
    EXPECT_EQ(data.events(0).trace_move_event().new_addr(), 0x7200);
    EXPECT_EQ(data.events(0).trace_move_event().size(), 32);
}

HWTEST_F(StackPreprocessTest, SetHookDataReportsTagForArktsFree, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, TAG_RES_ARKTS_HEAP_MASK);
    auto rawStack = MakeRawStack(0, 0, NAPI_TEST_TAG_ID);
    rawStack->stackContext->type = FREE_ARKTS;
    rawStack->stackContext->mallocSize = 32;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x7300);
    auto hookRecord = std::make_shared<ArktsFreeRecord>(rawStack);
    auto frames = MakeFrames(0x7200);
    BatchNativeHookData data;

    preprocess.SetHookData(hookRecord, frames, data);

    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(data.events(0).has_trace_free_region_event());
    EXPECT_EQ(data.events(0).trace_free_region_event().trace_type(), TraceType::ARKTS_HEAP);
    EXPECT_EQ(data.events(0).trace_free_region_event().addr(), 0x7300);
    EXPECT_EQ(data.events(0).trace_free_region_event().size(), 32);
}

HWTEST_F(StackPreprocessTest, SetHookDataCallframeCompressedCreatesStackMap, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto rawStack = MakeRawStack(3, 10, 0);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->mallocSize = 48;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x7400);
    HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);
    auto frames = MakeFrames(0x7500);
    BatchNativeHookData data;

    preprocess.SetHookData(hookRecord, frames, data);

    ASSERT_EQ(data.events_size(), 2);
    EXPECT_TRUE(data.events(0).has_stack_map());
    EXPECT_TRUE(data.events(1).has_alloc_event());
    EXPECT_EQ(data.events(1).alloc_event().stack_id(), 1u);
}

HWTEST_F(StackPreprocessTest, SetMapsInfoReportsExecMapsWhenBasicDataAlreadySent, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint32_t filePathId = 77;
    auto readMap = std::make_shared<DfxMap>(0x8000, 0x9000, 0, PROT_READ, "cached.so");
    auto execMap = std::make_shared<DfxMap>(0x9000, 0xA000, 0x1000, PROT_EXEC, "cached.so");
    auto memMaps = std::make_shared<MemMaps>(filePathId);
    memMaps->AddMap(readMap, true);
    memMaps->AddMap(execMap, false);
    memMaps->soEnd_ = execMap->end;
    preprocess.runtime_instance->GetMapsCache()[readMap->begin] = memMaps;
    preprocess.runtime_instance->GetOfflineMaps().push_back(readMap->begin);
    EXPECT_TRUE(preprocess.runtime_instance->MarkOfflineFileReported(filePathId));

    preprocess.SetMapsInfo();

    const auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(data.events(0).has_maps_info());
    EXPECT_EQ(data.events(0).maps_info().file_path_id(), filePathId);
    EXPECT_TRUE(preprocess.runtime_instance->GetOfflineMaps().empty());
}

HWTEST_F(StackPreprocessTest, SetMapsInfoReportsBasicDataForExecutableMap, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::string exePath = GetSelfExePathForTest();
    ASSERT_FALSE(exePath.empty());
    constexpr uint32_t filePathId = 78;
    auto execMap = std::make_shared<DfxMap>(0xB000, 0xC000, 0, PROT_EXEC, exePath);
    auto memMaps = std::make_shared<MemMaps>(filePathId);
    memMaps->AddMap(execMap, true);
    preprocess.runtime_instance->GetMapsCache()[execMap->begin] = memMaps;
    preprocess.runtime_instance->GetOfflineMaps().push_back(execMap->begin);

    preprocess.SetMapsInfo();

    const auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    bool hasFilePath = false;
    bool hasSymbolTable = false;
    bool hasMapsInfo = false;
    for (const auto& event : data.events()) {
        hasFilePath = hasFilePath || event.has_file_path();
        hasSymbolTable = hasSymbolTable || event.has_symbol_tab();
        hasMapsInfo = hasMapsInfo || event.has_maps_info();
    }
    EXPECT_TRUE(hasFilePath);
    EXPECT_TRUE(hasSymbolTable);
    EXPECT_TRUE(hasMapsInfo);
    EXPECT_TRUE(preprocess.runtime_instance->GetOfflineMaps().empty());
}

HWTEST_F(StackPreprocessTest, ProcessStackDataSelectsSaveFileBranches, TestSize.Level0)
{
    const std::string filePath = "/data/local/tmp/process_stack_data_test.txt";
    FILE* fp = fopen(filePath.c_str(), "w+");
    ASSERT_NE(fp, nullptr);
    NativeHookConfig hookConfig;
    hookConfig.set_save_file(true);
    hookConfig.set_file_name(filePath);
    StackPreprocess standalone(nullptr, hookConfig, 0, fp, true);
    auto rawStack = MakeRawStack(1, 11, 0);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->mallocSize = 64;
    HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);
    auto frames = MakeFrames(0x7600);
    BatchNativeHookData standaloneData;

    standalone.ProcessStackData(hookRecord, frames, standaloneData);
    EXPECT_EQ(standaloneData.events_size(), 1);

    BatchNativeHookData fileData;
    StackPreprocess fileMode(nullptr, hookConfig, 0, fp, false);
    fileMode.ProcessStackData(hookRecord, frames, fileData);
    EXPECT_EQ(fileData.events_size(), 0);
    EXPECT_EQ(fclose(fp), 0);
    EXPECT_EQ(remove(filePath.c_str()), 0);
}

HWTEST_F(StackPreprocessTest, SetEventFrameProtoEncoderCoversCompressedBranches, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    ReportEventBaseData rawStack = MakeReportEvent(MALLOC_MSG, 0x8100, 5);
    ProtoEncoder::AllocEvent allocEvent(&ctx);
    ProtoEncoder::FreeEvent freeEvent(&ctx);
    ProtoEncoder::MmapEvent mmapEvent(&ctx);

    preprocess.SetEventFrame(rawStack, &allocEvent, 5);
    preprocess.SetEventFrame(rawStack, &freeEvent, 6);
    preprocess.SetEventFrame(rawStack, &mmapEvent, 7, "FilePage:proto");

    EXPECT_GT(allocEvent.Finish(), 0);
    EXPECT_GT(freeEvent.Finish(), 0);
    EXPECT_GT(mmapEvent.Finish(), 0);
}

HWTEST_F(StackPreprocessTest, SetEventFrameProtoEncoderCoversTraceEvents, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    ReportEventBaseData rawStack = MakeReportEvent(MEMORY_USING_MSG, 0x8200, 8);
    ProtoEncoder::TraceAllocEvent traceAllocEvent(&ctx);
    ProtoEncoder::TraceFreeEvent traceFreeEvent(&ctx);

    preprocess.SetEventFrame(rawStack, &traceAllocEvent, 8, "gpu-vk");
    preprocess.SetEventFrame(rawStack, &traceFreeEvent, 0);

    EXPECT_GT(traceAllocEvent.Finish(), 0);
    EXPECT_GT(traceFreeEvent.Finish(), 0);
}

HWTEST_F(StackPreprocessTest, SetEventFrameProtoEncoderCoversZeroStackIdBranches, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    NativeHookConfig hookConfig;
    hookConfig.set_callframe_compress(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    ReportEventBaseData rawStack = MakeReportEvent(MMAP_MSG, 0x8250, 0);
    ProtoEncoder::AllocEvent allocEvent(&ctx);
    ProtoEncoder::FreeEvent freeEvent(&ctx);
    ProtoEncoder::MmapEvent mmapEvent(&ctx);

    preprocess.SetEventFrame(rawStack, &allocEvent, 0);
    preprocess.SetEventFrame(rawStack, &freeEvent, 0);
    preprocess.SetEventFrame(rawStack, &mmapEvent, 0, "proto_zero");

    EXPECT_GT(allocEvent.Finish(), 0);
    EXPECT_GT(freeEvent.Finish(), 0);
    EXPECT_GT(mmapEvent.Finish(), 0);
}

HWTEST_F(StackPreprocessTest, SetEventFrameProtoEncoderCoversUncompressedBranches, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    ReportEventBaseData rawStack = MakeReportEvent(MEMORY_USING_MSG, 0x8260, 9);
    ProtoEncoder::AllocEvent allocEvent(&ctx);
    ProtoEncoder::TraceAllocEvent traceAllocEvent(&ctx);
    ProtoEncoder::TraceFreeEvent traceFreeEvent(&ctx);

    preprocess.SetEventFrame(rawStack, &allocEvent, 9);
    preprocess.SetEventFrame(rawStack, &traceAllocEvent, 10, "proto_gpu");
    preprocess.SetEventFrame(rawStack, &traceFreeEvent, 11);

    EXPECT_GT(allocEvent.Finish(), 0);
    EXPECT_GT(traceAllocEvent.Finish(), 0);
    EXPECT_GT(traceFreeEvent.Finish(), 0);
}

HWTEST_F(StackPreprocessTest, SetFrameInfoProtoEncoderCoversOfflineAndOnline, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    CallFrame onlineFrame(0x8200);
    onlineFrame.sp_ = 0x8210;
    onlineFrame.symbolName_ = "ProtoOnline";
    onlineFrame.filePath_ = "proto_online.so";
    onlineFrame.symbolNameId_ = 3;
    onlineFrame.filePathId_ = 4;
    NativeHookConfig onlineConfig;
    StackPreprocess online(nullptr, onlineConfig, 0);
    ProtoEncoder::Frame onlineProto(&ctx);

    online.SetFrameInfo(onlineProto, onlineFrame);
    EXPECT_GT(onlineProto.Finish(), 0);

    NativeHookConfig offlineConfig;
    offlineConfig.set_offline_symbolization(true);
    StackPreprocess offline(nullptr, offlineConfig, 0);
    CallFrame jsFrame(0x8300);
    jsFrame.isJsFrame_ = true;
    jsFrame.symbolNameId_ = 8;
    jsFrame.filePathId_ = 9;
    ProtoEncoder::Frame offlineProto(&ctx);
    offline.SetFrameInfo(offlineProto, jsFrame);
    EXPECT_GT(offlineProto.Finish(), 0);
}

HWTEST_F(StackPreprocessTest, SetHookDataProtoEncoderCreatesOnlineAndOfflineStackMaps, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    auto runCase = [](bool offlineSymbolization) {
        NativeHookConfig hookConfig;
        hookConfig.set_callframe_compress(true);
        hookConfig.set_fp_unwind(true);
        hookConfig.set_offline_symbolization(offlineSymbolization);
        hookConfig.set_arkui_mem_trace_enable(true);
        StackPreprocess preprocess(nullptr, hookConfig, 0);
        auto rawStack = MakeRawStack(4, offlineSymbolization ? 22 : 21, 0);
        rawStack->stackContext->type = MALLOC_MSG;
        rawStack->stackContext->mallocSize = 32;
        HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);
        auto frames = MakeFrames(offlineSymbolization ? 0x8500 : 0x8400);
        RandomWriteCtx ctx = MakeEncoderWriteCtx();
        ProtoEncoder::BatchNativeHookData data(&ctx);

        preprocess.SetHookData(hookRecord, frames, data);
        EXPECT_EQ(preprocess.callStackIndex_.size(), 1u);
        EXPECT_GT(data.Finish(), 0);
    };

    runCase(false);
    runCase(true);
}

HWTEST_F(StackPreprocessTest, ProcessBatchDataConsumesRepeaterRecord, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    auto repeater = std::make_shared<StackDataRepeater>(MAX_BATCH_CNT);
    auto rawStack = MakeRawStack(1, 31, 0);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->mallocSize = 16;
    HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);
    ASSERT_TRUE(repeater->PutRawStack(hookRecord, true));
    StackPreprocess preprocess(repeater, hookConfig, 0);
    preprocess.SetFactory(std::make_shared<HookRecordFactory>(hookConfig));
    preprocess.InitializeDirector();

    EXPECT_TRUE(preprocess.ProcessBatchData());

    const auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(data.events(0).has_alloc_event());
}

HWTEST_F(StackPreprocessTest, FlushRecordStatisticsSerializesCachedRecords, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_use_file_cache_mode(true);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    RecordStatistic statistic;
    statistic.callstackId = 33;
    statistic.applyCount = 2;
    statistic.releaseCount = 1;
    statistic.applySize = 128;
    statistic.releaseSize = 64;
    statistic.tagId = NAPI_TEST_TAG_ID;
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, "napi_cached");
    preprocess.recordStatisticsMap_[statistic.callstackId] = statistic;
    preprocess.statisticsPeriodSet_.insert(statistic.callstackId);
    preprocess.lastCacheStatisticsTime_ = {7, 8};

    EXPECT_TRUE(preprocess.FlushRecordStatistics());

    const auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(data.events_size(), 1);
    EXPECT_EQ(data.events(0).tv_sec(), 7u);
    EXPECT_EQ(data.events(0).statistics_event().tag_name(), "napi_cached");
    EXPECT_TRUE(preprocess.statisticsPeriodSet_.empty());
}

HWTEST_F(StackPreprocessTest, ProcessBatchDataReturnsTrueOnStatisticsTimeout, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    auto repeater = std::make_shared<StackDataRepeater>(MAX_BATCH_CNT);
    StackPreprocess preprocess(repeater, hookConfig, 0);
    preprocess.lastStatisticsTime_ = std::chrono::steady_clock::now() - std::chrono::seconds(2);

    EXPECT_TRUE(preprocess.ProcessBatchData());
}

HWTEST_F(StackPreprocessTest, ProcessBatchDataSkipsStatisticsFlushInFileCacheMode, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_use_file_cache_mode(true);
    auto repeater = std::make_shared<StackDataRepeater>(MAX_BATCH_CNT);
    StackPreprocess preprocess(repeater, hookConfig, 0);
    preprocess.lastStatisticsTime_ = std::chrono::steady_clock::now() - std::chrono::seconds(2);

    EXPECT_TRUE(preprocess.ProcessBatchData());
}

HWTEST_F(StackPreprocessTest, ProcessBatchDataSaveFileModeReturnsBeforeFlush, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_save_file(true);
    hookConfig.set_file_name("/data/local/tmp/process_batch_save_file.txt");
    auto repeater = std::make_shared<StackDataRepeater>(MAX_BATCH_CNT);
    auto rawStack = MakeRawStack(1, 41, 0);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->mallocSize = 16;
    HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);
    ASSERT_TRUE(repeater->PutRawStack(hookRecord, true));
    StackPreprocess preprocess(repeater, hookConfig, 0);
    preprocess.SetFactory(std::make_shared<HookRecordFactory>(hookConfig));
    preprocess.InitializeDirector();

    EXPECT_TRUE(preprocess.ProcessBatchData());

    const auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    EXPECT_EQ(data.events_size(), 0);
}

HWTEST_F(StackPreprocessTest, ProcessBatchDataStandaloneSaveFileSerializesHookData, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_save_file(true);
    hookConfig.set_file_name("/data/local/tmp/process_batch_standalone.txt");
    auto repeater = std::make_shared<StackDataRepeater>(MAX_BATCH_CNT);
    auto rawStack = MakeRawStack(1, 42, 0);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->mallocSize = 32;
    HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);
    ASSERT_TRUE(repeater->PutRawStack(hookRecord, true));
    StackPreprocess preprocess(repeater, hookConfig, 0, nullptr, true);
    preprocess.SetFactory(std::make_shared<HookRecordFactory>(hookConfig));
    preprocess.InitializeDirector();

    EXPECT_TRUE(preprocess.ProcessBatchData());

    const auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(data.events(0).has_alloc_event());
}

HWTEST_F(StackPreprocessTest, ProcessEndMsgHandlesFileCacheAndSharedMemoryCounts, TestSize.Level0)
{
    NativeHookConfig fileCacheConfig;
    fileCacheConfig.set_use_file_cache_mode(true);
    StackPreprocess fileCachePreprocess(nullptr, fileCacheConfig, 0);
    EXPECT_FALSE(fileCachePreprocess.ProcessEndMsg());
    EXPECT_TRUE(fileCachePreprocess.isStopTakeData_.load());

    NativeHookConfig offlineConfig;
    offlineConfig.set_offline_symbolization(true);
    StackPreprocess offlinePreprocess(nullptr, offlineConfig, 0);
    for (int i = 1; i < SHARED_MEMORY_NUM; ++i) {
        EXPECT_TRUE(offlinePreprocess.ProcessEndMsg());
    }
    EXPECT_FALSE(offlinePreprocess.ProcessEndMsg());
    EXPECT_TRUE(offlinePreprocess.isStopTakeData_.load());
}

HWTEST_F(StackPreprocessTest, CheckIsFileCacheFinishRequiresModeRepeaterAndParseDone, TestSize.Level0)
{
    auto repeater = std::make_shared<StackDataRepeater>(MAX_BATCH_CNT);
    NativeHookConfig disabledConfig;
    StackPreprocess disabledPreprocess(repeater, disabledConfig, 0);
    disabledPreprocess.SetIsCacheFileParseDone(true);
    EXPECT_FALSE(disabledPreprocess.CheckIsFileCacheFinsh());

    NativeHookConfig fileCacheConfig;
    fileCacheConfig.set_use_file_cache_mode(true);
    StackPreprocess fileCachePreprocess(repeater, fileCacheConfig, 0);
    EXPECT_FALSE(fileCachePreprocess.CheckIsFileCacheFinsh());
    fileCachePreprocess.SetIsCacheFileParseDone(true);
    EXPECT_TRUE(fileCachePreprocess.CheckIsFileCacheFinsh());
}

HWTEST_F(StackPreprocessTest, ReconcileWithSnapshotFreesOnlyStaleAllocations, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint64_t liveAddr = 0x5010;
    constexpr uint64_t staleAddr = 0x6010;
    constexpr uint32_t stackId = 7;
    RecordStatistic statistic;
    statistic.callstackId = stackId;
    preprocess.recordStatisticsMap_[stackId] = statistic;
    preprocess.allocAddrMap_[liveAddr] = AllocAddrInfo {64, stackId, {1, 0}};
    preprocess.allocAddrMap_[staleAddr] = AllocAddrInfo {96, stackId, {1, 0}};
    preprocess.addrSet_.insert(liveAddr);
    preprocess.addrSet_.insert(staleAddr);

    preprocess.ReconcileWithSnapshot({liveAddr});

    EXPECT_EQ(preprocess.allocAddrMap_.count(liveAddr), 1u);
    EXPECT_EQ(preprocess.allocAddrMap_.count(staleAddr), 0u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[stackId].releaseCount, 1u);
    EXPECT_EQ(preprocess.recordStatisticsMap_[stackId].releaseSize, 96u);
}

HWTEST_F(StackPreprocessTest, BuildCallFramesSkipsAsyncNoStackEvents, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_async_stack_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto hookRecord = MakeHookRecord(1, 55, 0);
    auto rawData = hookRecord->GetRawStack();
    rawData->stackContext->type = PR_SET_VMA_MSG;
    std::vector<CallFrame> callFrames;

    EXPECT_TRUE(preprocess.BuildCallFrames(hookRecord, rawData, callFrames));
    EXPECT_TRUE(callFrames.empty());
}

HWTEST_F(StackPreprocessTest, ProcessStackDataSerializesWhenSaveFileDisabled, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto rawStack = MakeRawStack(1, 61, 0);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->mallocSize = 24;
    HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);
    auto frames = MakeFrames(0x8600);
    BatchNativeHookData data;

    preprocess.ProcessStackData(hookRecord, frames, data);

    ASSERT_EQ(data.events_size(), 1);
    EXPECT_TRUE(data.events(0).has_alloc_event());
    EXPECT_GT(data.events(0).alloc_event().size(), 0u);
}

HWTEST_F(StackPreprocessTest, ProcessStackDataSelectsProtoEncoderBranches, TestSize.Level0)
{
    auto runCase = [](bool saveFile, bool standalone, uint64_t nodeId) {
        NativeHookConfig hookConfig;
        hookConfig.set_fp_unwind(true);
        hookConfig.set_save_file(saveFile);
        if (saveFile) {
            hookConfig.set_file_name("/data/local/tmp/process_stack_proto.txt");
        }
        StackPreprocess preprocess(nullptr, hookConfig, 0, nullptr, standalone);
        auto rawStack = MakeRawStack(1, nodeId, 0);
        rawStack->stackContext->type = MALLOC_MSG;
        rawStack->stackContext->mallocSize = 40;
        HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);
        auto frames = MakeFrames(0x8700 + nodeId);
        RandomWriteCtx ctx = MakeEncoderWriteCtx();
        OHOS::Developtools::Profiler::ProtoEncoder::BatchNativeHookData data(&ctx);

        preprocess.ProcessStackData(hookRecord, frames, data);
        EXPECT_GT(data.Finish(), 0);
    };

    runCase(true, true, 71);
    runCase(false, false, 72);

    NativeHookConfig fileConfig;
    fileConfig.set_save_file(true);
    fileConfig.set_file_name("/data/local/tmp/process_stack_proto_file.txt");
    StackPreprocess fileMode(nullptr, fileConfig, 0, nullptr, false);
    HookRecordPtr hookRecord = std::make_shared<MallocRecord>(MakeRawStack(1, 73, 0));
    auto frames = MakeFrames(0x8800);
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    OHOS::Developtools::Profiler::ProtoEncoder::BatchNativeHookData data(&ctx);
    fileMode.ProcessStackData(hookRecord, frames, data);
    EXPECT_EQ(data.Finish(), 0);
}

HWTEST_F(StackPreprocessTest, FlushDataProtoEncoderUsesResultWriter, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0, nullptr, false, false, false);
    WriterStruct writer = MakeEncoderWriter(StartEncoderReport);
    g_writerFinishSize = 0;
    g_writerFlushCount = 0;
    preprocess.SetWriter(&writer);
    auto& data = std::get<ProtoEncoder::BatchNativeHookData>(preprocess.stackData_);

    data.add_events()->set_tv_sec(1);
    preprocess.FlushData(data);

    EXPECT_GT(g_writerFinishSize, 0);
    EXPECT_EQ(g_writerFlushCount, 1u);
}

HWTEST_F(StackPreprocessTest, FlushDataProtoEncoderHandlesEmptyAndNullReport, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0, nullptr, false, false, false);
    RandomWriteCtx emptyCtx = MakeEncoderWriteCtx();
    ProtoEncoder::BatchNativeHookData emptyData(&emptyCtx);
    preprocess.FlushData(emptyData);
    EXPECT_EQ(emptyData.Size(), 0);

    WriterStruct writer = MakeEncoderWriter(StartNullReport);
    preprocess.resultWriter_ = &writer;
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    ProtoEncoder::BatchNativeHookData data(&ctx);
    data.add_events()->set_tv_sec(2);
    preprocess.FlushData(data);
    EXPECT_EQ(preprocess.resultWriter_, &writer);
}

HWTEST_F(StackPreprocessTest, FlushDataProtoEncoderUpdatesSaWriterHeader, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    const std::string filePath = "/data/local/tmp/flush_proto_sa_writer.htrace";
    int fd = open(filePath.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    ASSERT_GE(fd, 0);
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0, nullptr, false, true, false);
    preprocess.SetWriterForFdSa(static_cast<uint32_t>(fd));
    RandomWriteCtx* ctx = preprocess.StartReport();
    ASSERT_NE(ctx, nullptr);
    ProtoEncoder::BatchNativeHookData data(ctx);
    data.add_events()->set_tv_sec(3);
    preprocess.dataFlushSize_ = 1U << 20;

    preprocess.FlushData(data);

    EXPECT_EQ(preprocess.dataFlushSize_, 0u);
    EXPECT_EQ(remove(filePath.c_str()), 0);
}

HWTEST_F(StackPreprocessTest, FlushRecordApplyReleaseSerializesAllTypes, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    AddMatchPeriodEvents(preprocess);

    preprocess.FlushRecordApplyAndReleaseMatchData();

    const auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    EXPECT_GE(data.events_size(), 8);
    EXPECT_TRUE(preprocess.applyAndReleaseMatchPeriodListData_.empty());
    EXPECT_TRUE(preprocess.releasedPeriodListData_.empty());
}

HWTEST_F(StackPreprocessTest, FlushRecordApplyReleaseUsesProtoEncoder, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0, nullptr, false, false, false);
    WriterStruct writer = MakeEncoderWriter(StartEncoderReport);
    g_writerFinishSize = 0;
    g_writerFlushCount = 0;
    preprocess.SetWriter(&writer);
    AddMatchPeriodEvents(preprocess);

    preprocess.FlushRecordApplyAndReleaseMatchData();

    EXPECT_GT(g_writerFinishSize, 0);
    EXPECT_EQ(g_writerFlushCount, 2u);
    EXPECT_TRUE(preprocess.applyAndReleaseMatchPeriodListData_.empty());
    EXPECT_TRUE(preprocess.releasedPeriodListData_.empty());
}

HWTEST_F(StackPreprocessTest, BuildAndReportStackSerializesStandaloneAndNoSave, TestSize.Level0)
{
    auto runSerializedCase = [&](bool saveFile, bool standalone, uint64_t nodeId) {
        NativeHookConfig hookConfig;
        hookConfig.set_fp_unwind(true);
        hookConfig.set_save_file(saveFile);
        if (saveFile) {
            hookConfig.set_file_name("/data/local/tmp/build_report_stack.txt");
        }
        StackPreprocess preprocess(nullptr, hookConfig, 0, nullptr, standalone);
        StackPreprocess::SetDirectorForTest(nullptr);
        preprocess.InitializeDirector();
        HookRecordPtr hookRecord = MakeMallocHookRecord(nodeId, 0x9000 + nodeId);

        preprocess.BuildAndReportStack(hookRecord);

        const auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
        EXPECT_EQ(data.events_size(), 1);
    };

    runSerializedCase(true, true, 81);
    runSerializedCase(false, false, 82);
}

HWTEST_F(StackPreprocessTest, BuildAndReportStackHandlesFileAndMatchingBranches, TestSize.Level0)
{
    const std::string filePath = "/data/local/tmp/build_report_stack_file.txt";
    FILE* fp = fopen(filePath.c_str(), "w+");
    ASSERT_NE(fp, nullptr);
    NativeHookConfig fileConfig;
    fileConfig.set_fp_unwind(true);
    fileConfig.set_save_file(true);
    fileConfig.set_file_name(filePath);
    StackPreprocess fileMode(nullptr, fileConfig, 0, fp, false);
    StackPreprocess::SetDirectorForTest(nullptr);
    fileMode.InitializeDirector();
    HookRecordPtr fileRecord = MakeMallocHookRecord(83, 0x9053);
    fileMode.BuildAndReportStack(fileRecord);
    EXPECT_EQ(std::get<BatchNativeHookData>(fileMode.stackData_).events_size(), 0);
    EXPECT_EQ(fclose(fp), 0);
    EXPECT_EQ(remove(filePath.c_str()), 0);

    NativeHookConfig matchConfig;
    matchConfig.set_fp_unwind(true);
    matchConfig.set_malloc_free_matching_interval(1);
    StackPreprocess matchMode(nullptr, matchConfig, 0);
    StackPreprocess::SetDirectorForTest(nullptr);
    matchMode.InitializeDirector();
    HookRecordPtr matchRecord = MakeMallocHookRecord(84, 0x9054);
    matchMode.BuildAndReportStack(matchRecord);
    EXPECT_EQ(matchMode.applyAndReleaseMatchPeriodListData_.size(), 1u);
}

HWTEST_F(StackPreprocessTest, WriteFramesSerializesReleaseAndMapRecords, TestSize.Level0)
{
    const std::string filePath = "/data/local/tmp/write_frames_records.txt";
    FILE* rawFile = fopen(filePath.c_str(), "w+");
    ASSERT_NE(rawFile, nullptr);
    std::unique_ptr<FILE, decltype(&fclose)> file(rawFile, fclose);
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_save_file(true);
    hookConfig.set_file_name(filePath);
    StackPreprocess preprocess(nullptr, hookConfig, 0, file.get(), false);
    auto frames = MakeFrames(0x9100);

    preprocess.WriteFrames(MakeFileRecord(FREE_MSG, 0xA000), frames);
    preprocess.WriteFrames(MakeFileRecord(MMAP_MSG, 0xA100), frames);
    preprocess.WriteFrames(MakeFileRecord(MUNMAP_MSG, 0xA200), frames);
    preprocess.WriteFrames(MakeFileRecord(PR_SET_VMA_MSG, 0xA300), frames);
    ASSERT_EQ(fflush(file.get()), 0);
    ASSERT_EQ(fseek(file.get(), 0, SEEK_SET), 0);
    char buffer[512] = {0};
    size_t readSize = fread(buffer, 1, sizeof(buffer) - 1, file.get());
    ASSERT_GT(readSize, 0u);
    std::string content(buffer, readSize);

    EXPECT_NE(content.find("free;"), std::string::npos);
    EXPECT_NE(content.find("mmap;"), std::string::npos);
    EXPECT_NE(content.find("munmap;"), std::string::npos);
    EXPECT_NE(content.find("tag:Anonymous:native_tag"), std::string::npos);
    EXPECT_EQ(remove(filePath.c_str()), 0);
}

HWTEST_F(StackPreprocessTest, BuildCallFramesRejectsInvalidAsyncRawData, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_async_stack_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto hookRecord = MakeHookRecord(1, 62, 0);
    std::vector<CallFrame> callFrames;
    RawStackPtr emptyRawData;

    EXPECT_FALSE(preprocess.BuildCallFrames(hookRecord, emptyRawData, callFrames));

    auto rawData = hookRecord->GetRawStack();
    delete rawData->stackContext;
    rawData->stackContext = nullptr;
    EXPECT_FALSE(preprocess.BuildCallFrames(hookRecord, rawData, callFrames));
}

HWTEST_F(StackPreprocessTest, BuildAndReportStackUsesApplyReleaseMatchMode, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_malloc_free_matching_interval(1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    StackPreprocess::SetDirectorForTest(nullptr);
    preprocess.InitializeDirector();
    auto rawStack = MakeRawStack(1, 63, 0);
    rawStack->stackContext->type = MALLOC_MSG;
    rawStack->stackContext->mallocSize = 40;
    rawStack->stackContext->addr = reinterpret_cast<void*>(0x8700);
    HookRecordPtr hookRecord = std::make_shared<MallocRecord>(rawStack);

    preprocess.BuildAndReportStack(hookRecord);

    ASSERT_EQ(preprocess.applyAndReleaseMatchPeriodListData_.size(), 1u);
    EXPECT_EQ(preprocess.applyAndReleaseMatchIntervallMap_.count(0x8700), 1u);
}

HWTEST_F(StackPreprocessTest, SetEventFrameProtobufCoversUncompressedBranches, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    ReportEventBaseData rawStack = MakeReportEvent(MMAP_MSG, 0x8800, 0);
    AllocEvent allocEvent;
    FreeEvent freeEvent;
    MmapEvent mmapEvent;

    preprocess.SetEventFrame(rawStack, &allocEvent, 0);
    preprocess.SetEventFrame(rawStack, &freeEvent, 0);
    preprocess.SetEventFrame(rawStack, &mmapEvent, 0, "file_page");

    EXPECT_EQ(allocEvent.pid(), preprocess.pid_);
    EXPECT_GT(allocEvent.size(), 0u);
    EXPECT_EQ(freeEvent.tid(), rawStack.tid);
    EXPECT_EQ(mmapEvent.type(), "file_page");
    EXPECT_EQ(mmapEvent.thread_name_id(), rawStack.tid);
}

HWTEST_F(StackPreprocessTest, SetEventFrameProtobufCoversCompressionMatrix, TestSize.Level0)
{
    auto runCase = [](bool compressed, uint32_t stackId) {
        NativeHookConfig hookConfig;
        hookConfig.set_callframe_compress(compressed);
        StackPreprocess preprocess(nullptr, hookConfig, 0);
        ReportEventBaseData rawStack = MakeReportEvent(MMAP_MSG, 0x8900 + stackId, stackId);
        AllocEvent allocEvent;
        FreeEvent freeEvent;
        MmapEvent mmapEvent;

        preprocess.SetEventFrame(rawStack, &allocEvent, stackId);
        preprocess.SetEventFrame(rawStack, &freeEvent, stackId);
        preprocess.SetEventFrame(rawStack, &mmapEvent, stackId, "matrix");

        uint32_t expectedStackId = (compressed && stackId != 0) ? stackId : 0;
        EXPECT_EQ(allocEvent.stack_id(), expectedStackId);
        EXPECT_EQ(freeEvent.stack_id(), expectedStackId);
        EXPECT_EQ(mmapEvent.stack_id(), expectedStackId);
        EXPECT_EQ(mmapEvent.type(), "matrix");
    };

    runCase(false, 3);
    runCase(true, 0);
    runCase(true, 4);
}

HWTEST_F(StackPreprocessTest, IntervalFlushRecordStatisticsHandlesFileCacheTiming, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_use_file_cache_mode(true);
    hookConfig.set_allocator_sampling_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    uint32_t commandCount = 0;
    preprocess.SetServerCommandCallback([&commandCount](uint32_t) { commandCount++; });

    preprocess.IntervalFlushRecordStatistics({2, 0});
    EXPECT_EQ(preprocess.lastCacheStatisticsTime_.tv_sec, 2);
    EXPECT_EQ(commandCount, 0u);

    preprocess.IntervalFlushRecordStatistics({4, 0});
    EXPECT_EQ(preprocess.lastCacheStatisticsTime_.tv_sec, 4);
    EXPECT_EQ(commandCount, 1u);
}

HWTEST_F(StackPreprocessTest, IntervalFlushRecordStatisticsHandlesRealtimeCallback, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_allocator_sampling_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    uint32_t commandCount = 0;
    preprocess.SetServerCommandCallback([&commandCount](uint32_t) { commandCount++; });
    preprocess.lastStatisticsTime_ = std::chrono::steady_clock::now() - std::chrono::seconds(2);

    preprocess.IntervalFlushRecordStatistics({0, 0});

    EXPECT_EQ(commandCount, 1u);
}

HWTEST_F(StackPreprocessTest, IntervalFlushRecordStatisticsFlushesFileCacheData, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_use_file_cache_mode(true);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint32_t stackId = 91;
    RecordStatistic statistic;
    statistic.callstackId = stackId;
    statistic.applyCount = 1;
    preprocess.recordStatisticsMap_[stackId] = statistic;
    preprocess.statisticsPeriodSet_.insert(stackId);
    preprocess.lastCacheStatisticsTime_ = {3, 0};

    preprocess.IntervalFlushRecordStatistics({5, 0});

    auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(data.events_size(), 1);
    EXPECT_EQ(data.events(0).statistics_event().callstack_id(), stackId);
    EXPECT_TRUE(preprocess.statisticsPeriodSet_.empty());
}

HWTEST_F(StackPreprocessTest, IntervalFlushRecordStatisticsFlushesRealtimeData, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint32_t stackId = 92;
    RecordStatistic statistic;
    statistic.callstackId = stackId;
    statistic.applyCount = 1;
    preprocess.recordStatisticsMap_[stackId] = statistic;
    preprocess.statisticsPeriodSet_.insert(stackId);
    preprocess.lastStatisticsTime_ = std::chrono::steady_clock::now() - std::chrono::seconds(2);

    preprocess.IntervalFlushRecordStatistics({0, 0});

    auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(data.events_size(), 1);
    EXPECT_EQ(data.events(0).statistics_event().callstack_id(), stackId);
    EXPECT_TRUE(preprocess.statisticsPeriodSet_.empty());
}

HWTEST_F(StackPreprocessTest, GetPcsByStackIdHandlesZeroAndNonZeroIds, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::vector<uintptr_t> pcs;

    EXPECT_TRUE(preprocess.GetPcsByStackId({0}, pcs, "zero stack id"));
    EXPECT_TRUE(preprocess.GetPcsByStackId({0xFFFFFF}, pcs, "non-zero stack id"));
}

HWTEST_F(StackPreprocessTest, FindCallStackIdHandlesAsyncCacheHitAndMiss, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_async_stack_enable(true);
    hookConfig.set_arkui_mem_trace_enable(true);
    hookConfig.set_statistics_interval(10);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto hookRecord = MakeAsyncHookRecord(MALLOC_MSG, 3, 4, 64);
    auto rawStack = hookRecord->GetRawStack();
    rawStack->stackContext->tagId = 5;
    uint64_t asyncHash = 3ULL ^ (4ULL << 32);
    preprocess.asyncCallStackIndex_[{asyncHash, 64ULL, 5u}] = 88;
    std::vector<uint64_t> callStack = {0x1234};

    EXPECT_EQ(preprocess.FindCallStackId(callStack, hookRecord), 88u);
    rawStack->stackContext->mallocSize = 65;
    EXPECT_EQ(preprocess.FindCallStackId(callStack, hookRecord), 0u);
}

HWTEST_F(StackPreprocessTest, BuildAsyncCallFramesKeepFramesForEmptyIds, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_async_stack_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto hookRecord = MakeAsyncHookRecord(MALLOC_MSG, 0, 0, 64);
    auto* asyncData = reinterpret_cast<AsyncStackData*>(hookRecord->GetRawStack()->stackContext);

    std::vector<CallFrame> regularFrames = MakeFrames(0x9000);
    EXPECT_TRUE(preprocess.BuildRegularAsyncCallFrames(asyncData, hookRecord, regularFrames));
    EXPECT_EQ(regularFrames.size(), 1u);

    std::vector<CallFrame> nestedFrames = MakeFrames(0x9001);
    EXPECT_TRUE(preprocess.BuildNestedAsyncCallFrames(asyncData, hookRecord, nestedFrames));
    EXPECT_EQ(nestedFrames.size(), 1u);
}

HWTEST_F(StackPreprocessTest, GetCallStackIdUsesNapiSyntheticFrame, TestSize.Level0)
{
    auto runCase = [](bool offlineSymbolization) {
        NativeHookConfig hookConfig;
        hookConfig.set_fp_unwind(true);
        hookConfig.set_js_stack_report(NAPI_CALL_STACK);
        hookConfig.set_offline_symbolization(offlineSymbolization);
        hookConfig.set_arkui_mem_trace_enable(true);
        StackPreprocess preprocess(nullptr, hookConfig, 0);
        preprocess.SaveMemTag(NAPI_TEST_TAG_ID, "napi_direct");
        auto rawStack = MakeRawStack(1, offlineSymbolization ? 93 : 92, NAPI_TEST_TAG_ID);
        rawStack->stackContext->type = MEMORY_USING_MSG;
        HookRecordPtr hookRecord = std::make_shared<HookRecord>(rawStack);
        auto frames = MakeFrames(offlineSymbolization ? 0x9020 : 0x901F);
        BatchNativeHookData data;

        uint32_t stackId = preprocess.GetCallStackId(hookRecord, frames, data);

        EXPECT_NE(stackId, 0u);
        EXPECT_EQ(preprocess.callStackIndex_.size(), 1u);
    };

    runCase(false);
    runCase(true);
}

HWTEST_F(StackPreprocessTest, TryProcessCachedAsyncStatisticsRejectsDisabledModes, TestSize.Level0)
{
    NativeHookConfig noStatisticsConfig;
    noStatisticsConfig.set_async_stack_enable(true);
    StackPreprocess noStatistics(nullptr, noStatisticsConfig, 0);
    auto noStatisticsRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    EXPECT_FALSE(noStatistics.TryProcessCachedAsyncStatistics(noStatisticsRecord));

    NativeHookConfig noAsyncConfig;
    noAsyncConfig.set_statistics_interval(1);
    StackPreprocess noAsync(nullptr, noAsyncConfig, 0);
    auto noAsyncRecord = MakeAsyncHookRecord(MALLOC_MSG, 11, 22, 64);
    EXPECT_FALSE(noAsync.TryProcessCachedAsyncStatistics(noAsyncRecord));

    NativeHookConfig nonStatisticsTypeConfig;
    nonStatisticsTypeConfig.set_statistics_interval(1);
    nonStatisticsTypeConfig.set_async_stack_enable(true);
    StackPreprocess nonStatisticsType(nullptr, nonStatisticsTypeConfig, 0);
    auto freeRecord = MakeAsyncHookRecord(FREE_MSG, 11, 22, 64);
    EXPECT_FALSE(nonStatisticsType.TryProcessCachedAsyncStatistics(freeRecord));
}

HWTEST_F(StackPreprocessTest, ProcessSingleRecordHandlesEarlyReturnBranches, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    HookRecordPtr nullRecord;
    EXPECT_FALSE(preprocess.ProcessSingleRecord(nullRecord));

    preprocess.isStopTakeData_ = true;
    HookRecordPtr stoppedRecord = std::make_shared<MallocRecord>(MakeRawStack(1, 71, 0));
    EXPECT_FALSE(preprocess.ProcessSingleRecord(stoppedRecord));
    preprocess.isStopTakeData_ = false;

    auto invalidRaw = MakeRawStack(1, 72, 0);
    delete invalidRaw->stackContext;
    invalidRaw->stackContext = nullptr;
    HookRecordPtr invalidRecord = std::make_shared<HookRecord>(invalidRaw);
    EXPECT_TRUE(preprocess.ProcessSingleRecord(invalidRecord));

    auto nmdRaw = MakeRawStack(1, 73, 0);
    nmdRaw->stackContext->type = NMD_MSG;
    HookRecordPtr nmdRecord = std::make_shared<HookRecord>(nmdRaw);
    EXPECT_FALSE(preprocess.ProcessSingleRecord(nmdRecord));

    auto ignoredRaw = MakeRawStack(1, 74, 0);
    ignoredRaw->stackContext->type = MALLOC_MSG;
    ignoredRaw->reportFlag = false;
    HookRecordPtr ignoredRecord = std::make_shared<MallocRecord>(ignoredRaw);
    EXPECT_TRUE(preprocess.ProcessSingleRecord(ignoredRecord));
    EXPECT_EQ(preprocess.ignoreCnts_, 1u);
}

HWTEST_F(StackPreprocessTest, ReportThreadNameMapSkipsUnchangedNames, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);

    preprocess.ReportThreadNameMap(7, "worker");
    auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(data.events_size(), 1);
    EXPECT_EQ(data.events(0).thread_name_map().name(), "worker");

    preprocess.ReportThreadNameMap(7, "worker");
    EXPECT_EQ(data.events_size(), 1);

    preprocess.ReportThreadNameMap(7, "renamed");
    ASSERT_EQ(data.events_size(), 2);
    EXPECT_EQ(data.events(1).thread_name_map().name(), "renamed");
}

HWTEST_F(StackPreprocessTest, SetHookDataStatisticsModeTracksAllocTypes, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_fp_unwind(true);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, "ashmem");
    std::vector<uint16_t> types = {MALLOC_MSG, MMAP_MSG, MMAP_FILE_PAGE_MSG, MEMORY_USING_MSG};
    auto frames = MakeFrames(0x9300);
    BatchNativeHookData data;

    for (size_t index = 0; index < types.size(); ++index) {
        preprocess.SetHookData(MakeStatisticsRecord(types[index], 101 + index), frames, data);
    }

    EXPECT_EQ(preprocess.recordStatisticsMap_.size(), types.size());
    EXPECT_EQ(preprocess.statisticsPeriodSet_.size(), types.size());
    EXPECT_TRUE(preprocess.statisticsModelFlushCallstack_);
    EXPECT_GT(data.events_size(), 0);
}

HWTEST_F(StackPreprocessTest, SetHookDataStatisticsModeUpdatesExistingStack, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_fp_unwind(true);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto frames = MakeFrames(0x9400);
    BatchNativeHookData data;

    preprocess.SetHookData(MakeStatisticsRecord(MALLOC_MSG, 111), frames, data);
    preprocess.SetHookData(MakeStatisticsRecord(MALLOC_MSG, 111), frames, data);

    ASSERT_EQ(preprocess.recordStatisticsMap_.size(), 1u);
    const auto& statistic = preprocess.recordStatisticsMap_.begin()->second;
    EXPECT_EQ(statistic.applyCount, 2u);
    EXPECT_EQ(preprocess.statisticsPeriodSet_.count(statistic.callstackId), 1u);
}

HWTEST_F(StackPreprocessTest, SetHookDataStatisticsModeHandlesReleaseTypes, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::vector<uint16_t> types = {FREE_MSG, MUNMAP_MSG, MEMORY_UNUSING_MSG};
    auto frames = MakeFrames(0x9500);
    BatchNativeHookData data;

    for (size_t index = 0; index < types.size(); ++index) {
        uint32_t stackId = static_cast<uint32_t>(201 + index);
        auto record = MakeStatisticsRecord(types[index], stackId);
        uint64_t addr = record->GetAddr();
        SeedStatistic(preprocess, addr, stackId);
        preprocess.SetHookData(record, frames, data);
        EXPECT_EQ(preprocess.allocAddrMap_.count(addr), 0u);
        EXPECT_EQ(preprocess.recordStatisticsMap_[stackId].releaseCount, 1u);
    }
}

HWTEST_F(StackPreprocessTest, SetHookDataStatisticsModeIgnoresUnsupportedTypes, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    std::vector<uint16_t> types = {PR_SET_VMA_MSG, MALLOC_ARKTS, FREE_ARKTS, MAX_ENUM};
    auto frames = MakeFrames(0x9600);
    BatchNativeHookData data;

    for (size_t index = 0; index < types.size(); ++index) {
        preprocess.SetHookData(MakeStatisticsRecord(types[index], 301 + index), frames, data);
    }

    EXPECT_EQ(preprocess.recordStatisticsMap_.size(), 0u);
    EXPECT_EQ(preprocess.statisticsPeriodSet_.size(), 0u);
    EXPECT_EQ(data.events_size(), 0);
}

HWTEST_F(StackPreprocessTest, FlushRecordStatisticsUsesCacheTimestamp, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_statistics_interval(1);
    hookConfig.set_use_file_cache_mode(true);
    hookConfig.set_arkui_mem_trace_enable(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    constexpr uint32_t stackId = 77;
    RecordStatistic statistic;
    statistic.callstackId = stackId;
    statistic.applyCount = 2;
    statistic.releaseCount = 1;
    preprocess.recordStatisticsMap_[stackId] = statistic;
    preprocess.statisticsPeriodSet_.insert(stackId);
    preprocess.lastCacheStatisticsTime_ = {9, 8};

    EXPECT_TRUE(preprocess.FlushRecordStatistics());

    auto& data = std::get<BatchNativeHookData>(preprocess.stackData_);
    ASSERT_EQ(data.events_size(), 1);
    EXPECT_EQ(data.events(0).tv_sec(), 9);
    EXPECT_EQ(data.events(0).tv_nsec(), 8);
    EXPECT_EQ(data.events(0).statistics_event().callstack_id(), stackId);
}

HWTEST_F(StackPreprocessTest, SetApplyAndReleaseMatchFrameCoversMatchAndPrctl, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    auto frames = MakeFrames(0x9700);
    BatchNativeHookData data;

    auto mallocRecord = MakeMallocHookRecord(701, 0xABC000);
    preprocess.SetApplyAndReleaseMatchFrame(mallocRecord, frames, data);
    EXPECT_EQ(preprocess.applyAndReleaseMatchPeriodListData_.size(), 1u);
    EXPECT_EQ(preprocess.applyAndReleaseMatchIntervallMap_.size(), 1u);

    auto freeRecord = MakeFileRecord(FREE_MSG, 0xABC000);
    preprocess.SetApplyAndReleaseMatchFrame(freeRecord, frames, data);
    EXPECT_TRUE(preprocess.applyAndReleaseMatchPeriodListData_.empty());
    EXPECT_TRUE(preprocess.applyAndReleaseMatchIntervallMap_.empty());

    auto munmapMiss = MakeFileRecord(MUNMAP_MSG, 0xDEF000);
    preprocess.SetApplyAndReleaseMatchFrame(munmapMiss, frames, data);
    EXPECT_EQ(preprocess.releasedPeriodListData_.size(), 1u);

    auto prctlRecord = MakeFileRecord(PR_SET_VMA_MSG, 0x12000);
    preprocess.SetApplyAndReleaseMatchFrame(prctlRecord, frames, data);
    ASSERT_EQ(preprocess.prctlPeriodTags_.size(), 1u);
    EXPECT_EQ(preprocess.prctlPeriodTags_[0], "native_tag");
}

HWTEST_F(StackPreprocessTest, BuildAndReportStackProtoEncoderCoversVisitBranches, TestSize.Level0)
{
    auto runCase = [](bool matchingMode, uint64_t nodeId) {
        NativeHookConfig hookConfig;
        hookConfig.set_fp_unwind(true);
        if (matchingMode) {
            hookConfig.set_malloc_free_matching_interval(1);
        }
        StackPreprocess preprocess(nullptr, hookConfig, 0, nullptr, false, false, false);
        WriterStruct writer = MakeEncoderWriter(StartEncoderReport);
        preprocess.SetWriter(&writer);
        StackPreprocess::SetDirectorForTest(nullptr);
        preprocess.InitializeDirector();
        HookRecordPtr hookRecord = MakeMallocHookRecord(nodeId, 0xB000 + nodeId);

        preprocess.BuildAndReportStack(hookRecord);

        auto& data = std::get<OHOS::Developtools::Profiler::ProtoEncoder::BatchNativeHookData>(preprocess.stackData_);
        EXPECT_GT(data.Finish(), 0);
        if (matchingMode) {
            EXPECT_EQ(preprocess.applyAndReleaseMatchPeriodListData_.size(), 1u);
        } else {
            EXPECT_TRUE(preprocess.applyAndReleaseMatchPeriodListData_.empty());
        }
    };

    runCase(false, 901);
    runCase(true, 902);
}

HWTEST_F(StackPreprocessTest, GetCallStackIdProtoEncoderHandlesNapiAndSuppressedTypes, TestSize.Level0)
{
    using namespace OHOS::Developtools::Profiler;
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(true);
    hookConfig.set_js_stack_report(NAPI_CALL_STACK);
    hookConfig.set_offline_symbolization(true);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    preprocess.SaveMemTag(NAPI_TEST_TAG_ID, "napi_proto");
    auto rawStack = MakeRawStack(1, 903, NAPI_TEST_TAG_ID);
    rawStack->stackContext->type = MEMORY_USING_MSG;
    HookRecordPtr hookRecord = std::make_shared<MemoryUsingRecord>(rawStack);
    auto frames = MakeFrames(0xA100);
    RandomWriteCtx ctx = MakeEncoderWriteCtx();
    ProtoEncoder::BatchNativeHookData data(&ctx);

    uint32_t stackId = preprocess.GetCallStackId(hookRecord, frames, data);
    EXPECT_NE(stackId, 0u);
    EXPECT_GT(data.Finish(), 0);

    NativeHookConfig suppressConfig;
    suppressConfig.set_free_stack_report(false);
    suppressConfig.set_munmap_stack_report(false);
    StackPreprocess suppress(nullptr, suppressConfig, 0);
    RandomWriteCtx suppressCtx = MakeEncoderWriteCtx();
    ProtoEncoder::BatchNativeHookData suppressData(&suppressCtx);
    EXPECT_EQ(suppress.GetCallStackId(MakeFileRecord(FREE_MSG, 0xC100), frames, suppressData), 0u);
    EXPECT_EQ(suppress.GetCallStackId(MakeFileRecord(MUNMAP_MSG, 0xC200), frames, suppressData), 0u);
    EXPECT_EQ(suppressData.Finish(), 0);
}

HWTEST_F(StackPreprocessTest, SetFrameInfoProtobufCoversOnlineAndOfflineIds, TestSize.Level0)
{
    NativeHookConfig onlineConfig;
    StackPreprocess online(nullptr, onlineConfig, 0);
    Frame plainFrame;
    CallFrame plainCallFrame(0xD100, 0xD110, false);
    plainCallFrame.symbolName_ = "plain_func";
    plainCallFrame.filePath_ = "plain.abc";
    online.SetFrameInfo(plainFrame, plainCallFrame);
    EXPECT_EQ(plainFrame.symbol_name(), "plain_func");
    EXPECT_EQ(plainFrame.file_path(), "plain.abc");

    Frame idFrame;
    CallFrame idCallFrame(0xD200, 0xD210, false);
    idCallFrame.symbolNameId_ = 11;
    idCallFrame.filePathId_ = 12;
    online.SetFrameInfo(idFrame, idCallFrame);
    EXPECT_EQ(idFrame.symbol_name_id(), 11u);
    EXPECT_EQ(idFrame.file_path_id(), 12u);

    NativeHookConfig offlineConfig;
    offlineConfig.set_offline_symbolization(true);
    StackPreprocess offline(nullptr, offlineConfig, 0);
    Frame offlineNativeFrame;
    CallFrame nativeCallFrame(0xD300, 0xD310, false);
    nativeCallFrame.symbolNameId_ = 21;
    nativeCallFrame.filePathId_ = 22;
    offline.SetFrameInfo(offlineNativeFrame, nativeCallFrame);
    EXPECT_EQ(offlineNativeFrame.ip(), 0xD300u);
    EXPECT_EQ(offlineNativeFrame.sp(), 0u);

    Frame offlineJsFrame;
    CallFrame jsCallFrame(0xD400, 0xD410, true);
    jsCallFrame.symbolNameId_ = 31;
    jsCallFrame.filePathId_ = 32;
    offline.SetFrameInfo(offlineJsFrame, jsCallFrame);
    EXPECT_EQ(offlineJsFrame.sp(), 0xD410u);
    EXPECT_EQ(offlineJsFrame.symbol_name_id(), 31u);
}

HWTEST_F(StackPreprocessTest, ReportFrameMapProtobufEmitsSymbolAndFileMaps, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    BatchNativeHookData data;
    CallFrame callFrame(0xE100, 0xE110, false);
    callFrame.callFrameId_ = 41;
    callFrame.symbolNameId_ = 42;
    callFrame.filePathId_ = 43;
    callFrame.symbolName_ = "reported_func";
    callFrame.filePath_ = "reported.abc";
    callFrame.needReport_ = CALL_FRAME_REPORT | SYMBOL_NAME_ID_REPORT | FILE_PATH_ID_REPORT;

    preprocess.ReportFrameMap(callFrame, data);

    ASSERT_EQ(data.events_size(), 3);
    EXPECT_TRUE(data.events(0).has_symbol_name());
    EXPECT_TRUE(data.events(1).has_file_path());
    EXPECT_TRUE(data.events(2).has_frame_map());
    EXPECT_EQ(data.events(2).frame_map().id(), 41u);
}

}
