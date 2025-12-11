
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
#include "stack_preprocess.h"
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
}

void StackPreprocessTest::TearDown(void)
{
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
}