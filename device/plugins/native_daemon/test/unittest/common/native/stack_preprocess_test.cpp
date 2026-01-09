
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

static std::vector<CallFrame> MakeFrames(uint64_t callFrameId)
{
    std::vector<CallFrame> frames;
    CallFrame frame(callFrameId);
    frame.callFrameId_ = static_cast<uint32_t>(callFrameId);
    frames.push_back(frame);
    return frames;
}

static thread_local std::vector<uint64_t> g_callStack;
static thread_local std::unordered_map<uint64_t, std::pair<uint64_t, RecordStatistic*>> g_allocAddrMap;
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
@tc.desc: test ProcessSharedMemoryData with size equal to sizeof(uint64_t).
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
    bool result = preprocess.ProcessSharedMemoryData(addrData, sizeof(addrData), rawData, hookRecord);
    ASSERT_TRUE(result);
}

/*
@tc.name: ProcessSharedMemoryDataInvalidSizeTest
@tc.desc: test ProcessSharedMemoryData with invalid data size.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, ProcessSharedMemoryDataInvalidSizeTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    int8_t invalidData[4] = {0}; // Smaller than any valid data structure
    RawStackPtr rawData = nullptr;
    std::shared_ptr<HookRecord> hookRecord = nullptr;
    bool result = preprocess.ProcessSharedMemoryData(invalidData, sizeof(invalidData), rawData, hookRecord);
    ASSERT_FALSE(result);
}

/*
@tc.name: ProcessSharedMemoryDataStopFlagTest
@tc.desc: test ProcessSharedMemoryData with isStopTakeData_ set to true.
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
    bool result = preprocess.ProcessSharedMemoryData(validData, sizeof(validData), rawData, hookRecord);
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
@tc.desc: test ProcessMemoryTag with valid data.
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
    NativeHookData* event = testData.add_events();
    event->set_tv_sec(1234567890);
    event->set_tv_nsec(987654321);

    preprocess.FlushData(testData);

    std::ifstream file(filePath);
    ASSERT_TRUE(file.is_open()) << "Failed to open file for reading";
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();

    ForStandard::BatchNativeHookData parseData;
    ASSERT_TRUE(google::protobuf::TextFormat::ParseFromString(fileContent,
        &parseData)) << "Failed to parse protobuf text format from file";
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
    ASSERT_TRUE(google::protobuf::TextFormat::ParseFromString(fileContent,
        &parseData)) << "Failed to parse protobuf text format from file";

    ASSERT_EQ(parseData.events_size(), 2);

    const auto& firstEvent = parseData.events(0);
    EXPECT_EQ(firstEvent.alloc_event().pid(), -1);
    EXPECT_EQ(firstEvent.alloc_event().addr(), 0);
    EXPECT_EQ(firstEvent.alloc_event().frame_info_size(), 0);
    EXPECT_EQ(firstEvent.tv_nsec(), 18446744073709551615ULL);

    const auto& secondEvent = parseData.events(1);
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
@tc.desc: Same call stack but different tagName should map to different stack ids.
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
    EXPECT_NE(id1, id2);
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
}