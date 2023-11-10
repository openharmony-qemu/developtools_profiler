/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "file.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

const std::string tracePath = "../../test/resource/trace_small_10.systrace";
constexpr size_t readSize = 1024;
constexpr uint32_t lineLength = 256;
constexpr size_t G_FILE_PERMISSION = 664;

namespace SysTuning {
namespace TraceStreamer {
class QueryMetricsTest : public ::testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

void ParseTraceFile(TraceStreamerSelector& ts)
{
    int32_t fd(base::OpenFile(tracePath, O_RDONLY, G_FILE_PERMISSION));
    if (fd < 0) {
        TS_LOGD("Failed to open trace file (errno: %d, %s)", errno, strerror(errno));
        return;
    }

    while (true) {
        std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(readSize);
        auto rsize = base::Read(fd, buf.get(), readSize);

        if (rsize <= 0) {
            break;
        }

        if (!ts.ParseTraceDataSegment(std::move(buf), rsize, 0, 1)) {
            break;
        }
    }

    close(fd);
    ts.WaitForParserEnd();
}

void ExecuteMetricsTest(TraceStreamerSelector& ts, std::string metricName)
{
    ts.EnableMetaTable(false);
    ts.SetCleanMode(false);
    ParseTraceFile(ts);
    bool result = ts.ParserAndPrintMetrics(metricName);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: QueryMetricsWithTraceMem
 * @tc.desc: Query metrics with trace mem
 * @tc.type: FUNC
 */
HWTEST_F(QueryMetricsTest, QueryMetricsWithTraceMem, TestSize.Level1)
{
    TS_LOGE("test42-1");
    TraceStreamerSelector ts;
    ExecuteMetricsTest(ts, TRACE_MEM);
}

/**
 * @tc.name: QueryMetricsWithTraceMem
 * @tc.desc: Query metrics with trace task names
 * @tc.type: FUNC
 */
HWTEST_F(QueryMetricsTest, QueryMetricsWithTraceMemTop10, TestSize.Level1)
{
    TS_LOGE("test42-2");
    TraceStreamerSelector ts;
    ExecuteMetricsTest(ts, TRACE_MEM_TOP_TEN);
}

/**
 * @tc.name: QueryMetricsWithTraceMem
 * @tc.desc: Query metrics with trace task names
 * @tc.type: FUNC
 */
HWTEST_F(QueryMetricsTest, QueryMetricsWithTraceMemUnagg, TestSize.Level1)
{
    TS_LOGE("test42-3");
    TraceStreamerSelector ts;
    ExecuteMetricsTest(ts, TRACE_MEM_UNAGG);
}

/**
 * @tc.name: QueryMetricsWithTraceTaskNames
 * @tc.desc: Query metrics with trace task names
 * @tc.type: FUNC
 */
HWTEST_F(QueryMetricsTest, QueryMetricsWithTraceTaskNames, TestSize.Level1)
{
    TS_LOGE("test42-4");
    TraceStreamerSelector ts;
    ExecuteMetricsTest(ts, TRACE_TASK_NAMES);
}

/**
 * @tc.name: QueryMetricsWithTraceStats
 * @tc.desc: Query metrics with trace stats
 * @tc.type: FUNC
 */
HWTEST_F(QueryMetricsTest, QueryMetricsWithTraceStats, TestSize.Level1)
{
    TS_LOGE("test42-5");
    TraceStreamerSelector ts;
    ExecuteMetricsTest(ts, TRACE_STATS);
}

/**
 * @tc.name: QueryMetricsWithTraceMetaData
 * @tc.desc: Query metrics with trace metadata
 * @tc.type: FUNC
 */
HWTEST_F(QueryMetricsTest, QueryMetricsWithTraceMetaData, TestSize.Level1)
{
    TS_LOGE("test42-6");
    TraceStreamerSelector ts;
    ExecuteMetricsTest(ts, TRACE_METADATA);
}

/**
 * @tc.name: QueryMetricsWithSysCalls
 * @tc.desc: Query metrics with sys calls
 * @tc.type: FUNC
 */
HWTEST_F(QueryMetricsTest, QueryMetricsWithSysCalls, TestSize.Level1)
{
    TS_LOGE("test42-7");
    TraceStreamerSelector ts;
    ExecuteMetricsTest(ts, SYS_CALLS);
}

} // namespace TraceStreamer
} // namespace SysTuning
