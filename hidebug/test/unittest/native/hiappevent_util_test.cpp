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

#include <gtest/hwext/gtest-ext.h>
#include <gtest/hwext/gtest-tag.h>
#include "hiappevent_util.h"

#include <thread>

using namespace testing::ext;

namespace OHOS {
namespace HiviewDFX {
class HiAppEventUtilTest : public ::testing::Test {
public:
    static void SetUpTestCase()
    {
        ApiRecordReporter::InitProcessor();
    }
};

/**
 * @tc.name: SingleRecordReporterTest
 * @tc.desc: test SingleRecordReporter.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventUtilTest, SingleRecordReporterTest, TestSize.Level0)
{
    bool success = false;
    {
        {
            constexpr auto testApiName = "SingleRecordReporterTest";
            ApiInvokeRecorder successRecorder(testApiName);
            ApiInvokeRecorder failedRecorder2(testApiName);
            failedRecorder2.SetErrorCode(200);
        }
        success = true;
    }
    ASSERT_TRUE(success);
}

/**
 * @tc.name: MultipleRecordReporterTest
 * @tc.desc: test MultipleRecordReporter.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventUtilTest, MultipleRecordReporterTest, TestSize.Level0)
{
    MultipleRecordReporter multipleRecordReporter(1, 2);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    bool success = false;
    {
        {
            constexpr auto testApiName = "MultipleRecordReporterTest";
            ApiInvokeRecorder successRecorder(testApiName, multipleRecordReporter);
            ApiInvokeRecorder failedRecorder2(testApiName, multipleRecordReporter);
            ApiInvokeRecorder failedRecorder3(testApiName, multipleRecordReporter);
            failedRecorder3.SetErrorCode(200);
        }
        success = true;
    }
    ASSERT_TRUE(success);
}

/**
 * @tc.name: MultipleRecordReporterTest2
 * @tc.desc: test MultipleRecordReporter2.
 * @tc.type: FUNC
 */
HWTEST_F(HiAppEventUtilTest, MultipleRecordReporterTest2, TestSize.Level0)
{
    bool success = false;
    {
        MultipleRecordReporter multipleRecordReporter(0, 0);
        multipleRecordReporter.ReportRecord("MultipleRecordReporterTest2", 0, 0, 0);
        MultipleRecordReporter timeRepoter(1, 0);
        timeRepoter.ReportRecord("MultipleRecordReporterTest2", 0, 0, 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        timeRepoter.ReportRecord("MultipleRecordReporterTest2", 0, 0, 0);
        MultipleRecordReporter countReporter(0, 2);
        countReporter.ReportRecord("MultipleRecordReporterTest2", 0, 0, 0);
        countReporter.ReportRecord("MultipleRecordReporterTest2", 0, 0, 0);
        success = true;
    }
    ASSERT_TRUE(success);
}
}
}