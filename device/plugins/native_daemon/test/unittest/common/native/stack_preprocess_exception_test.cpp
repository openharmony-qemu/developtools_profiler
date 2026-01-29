/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "stack_preprocess_exception_test.h"
#include "native_hook_config.pb.h"

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {

constexpr int TEST_MAX_SIZE = 1024;

void StackPreprocessExceptionTest::SetUpTestCase(void) {}
void StackPreprocessExceptionTest::TearDownTestCase(void) {}

void StackPreprocessExceptionTest::SetUp()
{
    hookConfig_.Clear();
    dataRepeater_ = std::make_shared<StackDataRepeater>(TEST_MAX_SIZE);
}

void StackPreprocessExceptionTest::TearDown()
{
    stackPreprocess_ = nullptr;
    dataRepeater_ = nullptr;
}

/*
* @tc.name: ConstructorWithExceedMatchInterval
* @tc.desc: test constructor with malloc_free_matching_interval exceeding MAX_MATCH_INTERVAL.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, ConstructorWithExceedMatchInterval, TestSize.Level0)
{
    hookConfig_.set_malloc_free_matching_interval(4000);
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    EXPECT_EQ(stackPreprocess_->hookConfig_.malloc_free_matching_interval(), 3600);
}

/*
* @tc.name: ConstructorWithExceedMatchCnt
* @tc.desc: test constructor with malloc_free_matching_cnt exceeding MAX_MATCH_CNT.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, ConstructorWithExceedMatchCnt, TestSize.Level0)
{
    hookConfig_.set_malloc_free_matching_cnt(2000);
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    EXPECT_EQ(stackPreprocess_->hookConfig_.malloc_free_matching_cnt(), 1000);
}

/*
* @tc.name: ConstructorWithNullDataRepeater
* @tc.desc: test constructor with null data repeater.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, ConstructorWithNullDataRepeater, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(nullptr, hookConfig_, CLOCK_REALTIME);
    EXPECT_EQ(stackPreprocess_->dataRepeater_, nullptr);
}

/*
* @tc.name: StartTakeResultsWithNullDataRepeater
* @tc.desc: test StartTakeResults when dataRepeater_ is nullptr.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, StartTakeResultsWithNullDataRepeater, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(nullptr, hookConfig_, CLOCK_REALTIME);
    bool result = stackPreprocess_->StartTakeResults();
    EXPECT_FALSE(result);
}

/*
* @tc.name: StopTakeResultsWithoutStart
* @tc.desc: test StopTakeResults without calling StartTakeResults first.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, StopTakeResultsWithoutStart, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    bool result = stackPreprocess_->StopTakeResults();
    EXPECT_FALSE(result);
}

/*
* @tc.name: SetWriterWithNullWriter
* @tc.desc: test SetWriter with null shared_ptr writer.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SetWriterWithNullWriter, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    std::shared_ptr<Writer> nullWriter = nullptr;
    stackPreprocess_->SetWriter(nullWriter);
    EXPECT_EQ(stackPreprocess_->writer_, nullptr);
}

/*
* @tc.name: SetWriterStructWithNullWriter
* @tc.desc: test SetWriter with null WriterStructPtr.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SetWriterStructWithNullWriter, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    WriterStructPtr nullWriter = nullptr;
    stackPreprocess_->SetWriter(nullWriter);
    EXPECT_EQ(stackPreprocess_->resultWriter_, nullptr);
}

/*
* @tc.name: SaveAndGetMemTagWithValidData
* @tc.desc: test SaveMemTag and GetMemTag with valid data.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SaveAndGetMemTagWithValidData, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    uint32_t tagId = 100;
    std::string tagName = "test_tag";
    stackPreprocess_->SaveMemTag(tagId, tagName);
    std::string result;
    bool found = stackPreprocess_->GetMemTag(tagId, result);
    EXPECT_TRUE(found);
    EXPECT_EQ(result, tagName);
}

/*
* @tc.name: GetMemTagWithNonExistId
* @tc.desc: test GetMemTag with non-existent tag id.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, GetMemTagWithNonExistId, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    std::string result;
    bool found = stackPreprocess_->GetMemTag(999999, result);
    EXPECT_FALSE(found);
}

/*
* @tc.name: SaveJsRawStackWithNullStack
* @tc.desc: test SaveJsRawStack with null stack pointer.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SaveJsRawStackWithNullStack, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    uint64_t jsChainId = 12345;
    stackPreprocess_->SaveJsRawStack(jsChainId, nullptr);
    const char* result = stackPreprocess_->GetJsRawStack(jsChainId);
    EXPECT_EQ(result, nullptr);
}

/*
* @tc.name: GetJsRawStackWithNonExistId
* @tc.desc: test GetJsRawStack with non-existent chain id.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, GetJsRawStackWithNonExistId, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    const char* result = stackPreprocess_->GetJsRawStack(999999);
    EXPECT_EQ(result, nullptr);
}

/*
* @tc.name: SetPidWithNegativeValue
* @tc.desc: test SetPid with negative value.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SetPidWithNegativeValue, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    stackPreprocess_->SetPid(-1);
    EXPECT_EQ(stackPreprocess_->pid_, -1);
}

/*
* @tc.name: SetFlushSizeWithZero
* @tc.desc: test SetFlushSize with zero value.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SetFlushSizeWithZero, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    stackPreprocess_->SetFlushSize(0);
    EXPECT_EQ(stackPreprocess_->flushSize_, 0);
}

/*
* @tc.name: ForceStopWithNullDataRepeater
* @tc.desc: test ForceStop when dataRepeater_ is nullptr.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, ForceStopWithNullDataRepeater, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(nullptr, hookConfig_, CLOCK_REALTIME);
    stackPreprocess_->ForceStop();
    EXPECT_TRUE(stackPreprocess_->isStopTakeData_);
}

/*
* @tc.name: SetFlushSizeWithMaxValue
* @tc.desc: test SetFlushSize with max value.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SetFlushSizeWithMaxValue, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    stackPreprocess_->SetFlushSize(UINT32_MAX);
    EXPECT_EQ(stackPreprocess_->flushSize_, (UINT32_MAX / 10) + 1);
}

/*
* @tc.name: SetPidWithMaxValue
* @tc.desc: test SetPid with max value.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SetPidWithMaxValue, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    stackPreprocess_->SetPid(INT_MAX);
    EXPECT_EQ(stackPreprocess_->pid_, INT_MAX);
}

/*
* @tc.name: SaveMultipleMemTags
* @tc.desc: test SaveMemTag with multiple tags.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SaveMultipleMemTags, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    stackPreprocess_->SaveMemTag(1, "tag1");
    stackPreprocess_->SaveMemTag(2, "tag2");
    stackPreprocess_->SaveMemTag(3, "tag3");
    std::string result;
    EXPECT_TRUE(stackPreprocess_->GetMemTag(1, result));
    EXPECT_EQ(result, "tag1");
    EXPECT_TRUE(stackPreprocess_->GetMemTag(2, result));
    EXPECT_EQ(result, "tag2");
    EXPECT_TRUE(stackPreprocess_->GetMemTag(3, result));
    EXPECT_EQ(result, "tag3");
}

/*
* @tc.name: SaveMemTagWithEmptyName
* @tc.desc: test SaveMemTag with empty tag name.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SaveMemTagWithEmptyName, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    stackPreprocess_->SaveMemTag(100, "");
    std::string result;
    bool found = stackPreprocess_->GetMemTag(100, result);
    EXPECT_TRUE(found);
    EXPECT_EQ(result, "");
}

/*
* @tc.name: SaveJsRawStackWithValidData
* @tc.desc: test SaveJsRawStack with valid data.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, SaveJsRawStackWithValidData, TestSize.Level0)
{
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    const char* testStack = "test_js_stack_data";
    uint64_t jsChainId = 12345;
    stackPreprocess_->SaveJsRawStack(jsChainId, testStack);
    const char* result = stackPreprocess_->GetJsRawStack(jsChainId);
    EXPECT_NE(result, nullptr);
}

/*
* @tc.name: ConstructorWithValidMatchInterval
* @tc.desc: test constructor with valid malloc_free_matching_interval.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, ConstructorWithValidMatchInterval, TestSize.Level0)
{
    hookConfig_.set_malloc_free_matching_interval(1800);
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    EXPECT_EQ(stackPreprocess_->hookConfig_.malloc_free_matching_interval(), 1800);
}

/*
* @tc.name: ConstructorWithValidMatchCnt
* @tc.desc: test constructor with valid malloc_free_matching_cnt.
* @tc.type: FUNC
*/
HWTEST_F(StackPreprocessExceptionTest, ConstructorWithValidMatchCnt, TestSize.Level0)
{
    hookConfig_.set_malloc_free_matching_cnt(500);
    stackPreprocess_ = std::make_shared<StackPreprocess>(dataRepeater_, hookConfig_, CLOCK_REALTIME);
    EXPECT_EQ(stackPreprocess_->hookConfig_.malloc_free_matching_cnt(), 500);
}
} // namespace OHOS::Developtools::NativeDaemon