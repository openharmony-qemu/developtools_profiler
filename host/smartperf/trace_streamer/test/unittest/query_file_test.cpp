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
namespace SysTuning {
namespace TraceStreamer {
class QueryFileTest : public ::testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: QuerySingleSqlWithFile
 * @tc.desc: Query single sql with file
 * @tc.type: FUNC
 */
HWTEST_F(QueryFileTest, QuerySingleSqlWithFile, TestSize.Level1)
{
    TS_LOGE("test41-1");
    std::string sqlOperatorFilePath = "../../test/resource/query_single_file.sql";
    std::string traceFilePath = "../../test/resource/trace_small_10.systrace";
    TraceStreamerSelector ts;
    ts.EnableMetaTable(false);
    ts.SetCleanMode(false);
    EXPECT_EQ(ts.ReadSqlFileAndPrintResult(sqlOperatorFilePath), true);
}

/**
 * @tc.name: QuerySqlWithFile
 * @tc.desc: Query Sql With File
 * @tc.type: FUNC
 */
HWTEST_F(QueryFileTest, QueryMultiSqlWithFile, TestSize.Level1)
{
    TS_LOGE("test41-2");
    std::string sqlOperatorFilePath = "../../test/resource/query_multi_file.sql";
    std::string traceFilePath = "../../test/resource/trace_small_10.systrace";
    TraceStreamerSelector ts;
    ts.EnableMetaTable(false);
    ts.SetCleanMode(false);
    EXPECT_EQ(ts.ReadSqlFileAndPrintResult(sqlOperatorFilePath), true);
}

/**
 * @tc.name: QuerySqlWithFile
 * @tc.desc: Query Sql With File
 * @tc.type: FUNC
 */
HWTEST_F(QueryFileTest, QuerySingleSqlWithNoSpaceFile, TestSize.Level1)
{
    TS_LOGE("test41-3");
    std::string sqlOperatorFilePath = "../../test/resource/query_single_file_no_space.sql";
    std::string traceFilePath = "../../test/resource/trace_small_10.systrace";
    TraceStreamerSelector ts;
    ts.EnableMetaTable(false);
    ts.SetCleanMode(false);
    EXPECT_EQ(ts.ReadSqlFileAndPrintResult(sqlOperatorFilePath), true);
}

/**
 * @tc.name: QuerySqlWithFile
 * @tc.desc: Query Sql With File
 * @tc.type: FUNC
 */
HWTEST_F(QueryFileTest, QueryMultiSqlWithNoSpaceFile, TestSize.Level1)
{
    TS_LOGE("test41-4");
    std::string sqlOperatorFilePath = "../../test/resource/query_multi_file_no_space.sql";
    std::string traceFilePath = "../../test/resource/trace_small_10.systrace";
    TraceStreamerSelector ts;
    ts.EnableMetaTable(false);
    ts.SetCleanMode(false);
    EXPECT_EQ(ts.ReadSqlFileAndPrintResult(sqlOperatorFilePath), true);
}
} // namespace TraceStreamer
} // namespace SysTuning
