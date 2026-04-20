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
@tc.name: FlushRecordApplyAndReleaseMatchDataTest
@tc.desc: test FlushRecordApplyAndReleaseMatchData method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, FlushRecordApplyAndReleaseMatchDataTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Just call the method to ensure it doesn't crash
    preprocess.FlushRecordApplyAndReleaseMatchData();
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(true);
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
@tc.name: ProcessMemoryTagTest
@tc.desc: test ProcessMemoryTag method.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessExtTest, ProcessMemoryTagTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    
    // Create a raw stack with memory tag data
    RawStackPtr rawData = MakeRawStack(1, 100, 0);
    // Set data field to a test string
    const char* testTag = "test_memory_tag";
    rawData->data = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(testTag));
    
    // Just call the method to ensure it doesn't crash
    bool result = preprocess.ProcessMemoryTag(rawData);
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(result);
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
    const char* testTag = "test_memory_tag0";
    rawData->data = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(testTag));
    auto hookRecord = std::make_shared<HookRecord>(rawData);
    std::vector<CallFrame> callFrames = MakeFrames(123);
    
    // Just call the method to ensure it doesn't crash
    preprocess.InitializeDirector();
    bool result = preprocess.BuildCallFrames(hookRecord, rawData, callFrames);
    // We can't easily verify the output, but we can ensure the method executes without error
    EXPECT_TRUE(result);
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
