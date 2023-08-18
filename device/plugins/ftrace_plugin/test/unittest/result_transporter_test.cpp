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
#include <fcntl.h>
#include <fstream>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "result_transporter.h"
#include "trace_plugin_config.pb.h"
#include "trace_plugin_result.pb.h"

using namespace testing::ext;

namespace {
constexpr int WAIT_MS = 1000;
using WriterStructPtr = std::unique_ptr<WriterStruct>::pointer;
using ConstVoidPtr = std::unique_ptr<const void>::pointer;
class ResultTransporterTest : public ::testing::Test {
protected:
    std::string name_;

    void SetUp() override
    {
        name_ = "Transporter";
    }

    void TearDown() override {}
};

long WriteFunc(WriterStructPtr writer, ConstVoidPtr data, size_t size)
{
    if (writer == nullptr || data == nullptr || size <= 0) {
        return -1;
    }

    return 0;
}

bool FlushFunc(WriterStructPtr writer)
{
    if (writer == nullptr) {
        return false;
    }
    return true;
}

/*
 * @tc.name: IsFlushTime
 * @tc.desc: test ResultTransporter::IsFlushTime.
 * @tc.type: FUNC
 */
HWTEST_F(ResultTransporterTest, IsFlushTimeInvalidParam, TestSize.Level1)
{
    OHOS::Profiler::Plugins::ResultTransporter reporter("", nullptr);
    reporter.SetFlushInterval(WAIT_MS);
    reporter.IsFlushTime();
    usleep(WAIT_MS);
    EXPECT_FALSE(reporter.IsFlushTime());
}

/*
 * @tc.name: IsFlushTime
 * @tc.desc: test ResultTransporter::IsFlushTime.
 * @tc.type: FUNC
 */
HWTEST_F(ResultTransporterTest, IsFlushTimeTrue, TestSize.Level1)
{
    WriterStruct writer = {WriteFunc, FlushFunc};
    OHOS::Profiler::Plugins::ResultTransporter reporter(name_, static_cast<WriterStructPtr>(&writer));
    reporter.SetFlushInterval(1);
    EXPECT_TRUE(reporter.IsFlushTime());
}

/*
 * @tc.name: IsFlushTime
 * @tc.desc: test ResultTransporter::IsFlushTime.
 * @tc.type: FUNC
 */
HWTEST_F(ResultTransporterTest, IsFlushTimeFalse, TestSize.Level1)
{
    WriterStruct writer = {WriteFunc, FlushFunc};
    OHOS::Profiler::Plugins::ResultTransporter reporter(name_, static_cast<WriterStructPtr>(&writer));
    reporter.SetFlushInterval(WAIT_MS);
    reporter.IsFlushTime();
    usleep(WAIT_MS);
    EXPECT_FALSE(reporter.IsFlushTime());
}

/*
 * @tc.name: Write
 * @tc.desc: test ResultTransporter::Submit.
 * @tc.type: FUNC
 */
HWTEST_F(ResultTransporterTest, WriteInvalidParam, TestSize.Level1)
{
    OHOS::Profiler::Plugins::ResultTransporter reporter("", nullptr);
    auto packet = std::make_unique<TracePluginResult>();
    ASSERT_TRUE(reporter.Submit(std::move(packet)));
}

/*
 * @tc.name: Write
 * @tc.desc: test ResultTransporter::Submit.
 * @tc.type: FUNC
 */
HWTEST_F(ResultTransporterTest, FlushInvalidParam, TestSize.Level1)
{
    WriterStruct flush = {WriteFunc, nullptr};
    OHOS::Profiler::Plugins::ResultTransporter reporter(name_, static_cast<WriterStructPtr>(&flush));
    auto packet = std::make_unique<TracePluginResult>();
    reporter.SetFlushInterval(0);
    ASSERT_TRUE(reporter.IsFlushTime());
    reporter.SetFlushThreshold(0);
    EXPECT_TRUE(reporter.Submit(std::move(packet)));
}

/*
 * @tc.name: Write
 * @tc.desc: test ResultTransporter::Submit.
 * @tc.type: FUNCquit
 */
HWTEST_F(ResultTransporterTest, WriteFalse, TestSize.Level1)
{
    WriterStruct writer = {WriteFunc, FlushFunc};
    OHOS::Profiler::Plugins::ResultTransporter reporter(name_, static_cast<WriterStructPtr>(&writer));
    auto packet = std::make_unique<TracePluginResult>();
    reporter.SetFlushInterval(0);
    ASSERT_TRUE(reporter.IsFlushTime());
    reporter.SetFlushThreshold(0);
    EXPECT_TRUE(reporter.Submit(std::move(packet)));
}
} // namespace