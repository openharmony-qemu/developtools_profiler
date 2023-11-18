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

#include "span_join.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
namespace SysTuning {
namespace TraceStreamer {
class SpanJoinTest : public ::testing::Test {
public:
    void SetUp() {}
    void TearDown()
    {
        if (stmt_ != nullptr) {
            sqlite3_finalize(stmt_);
        }
    }

public:
    void Prepare(const std::string& sql)
    {
        int32_t size = static_cast<int32_t>(sql.size());
        sqlite3_prepare_v2(stream_.traceDataCache_->db_, sql.c_str(), size, &stmt_, nullptr);
    }

    void Step(const std::string& sql)
    {
        Prepare(sql);
        sqlite3_step(stmt_);
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }

    void Next(const std::vector<int64_t> column)
    {
        sqlite3_step(stmt_);
        for (size_t i = 0; i < column.size(); ++i) {
            sqlite3_column_int64(stmt_, static_cast<int32_t>(i));
        }
    }
    sqlite3_stmt* stmt_;
    TraceStreamerSelector stream_ = {};
};
/**
 * @tc.name: SpanjoinTwoTable
 * @tc.desc: SpanjoinTwoTable with ts,dur,and partitioned cpu
 * @tc.type: FUNC
 */
HWTEST_F(SpanJoinTest, SpanjoinTwoTable, TestSize.Level1)
{
    TS_LOGI("test30-1");
    Step("CREATE TABLE FirstTable(ts UNSIGNED INT PRIMARY KEY, dur UNSIGNED INT, cpu UNSIGNED INT);");
    Step("CREATE TABLE SecondTable(ts UNSIGNED INT PRIMARY KEY, dur UNSIGNED INT, cpu UNSIGNED INT);");
    Step(
        "CREATE VIRTUAL TABLE SpanjoinTable using span_join(FirstTable partitioned cpu, SecondTable partitioned cpu);");
    Step("INSERT INTO FirstTable VALUES(100, 10, 5);");
    Step("INSERT INTO FirstTable VALUES(110, 50, 5);");
    Step("INSERT INTO FirstTable VALUES(120, 100, 2);");
    Step("INSERT INTO FirstTable VALUES(160, 10, 5);");
    Step("INSERT INTO SecondTable VALUES(100, 5, 5);");
    Step("INSERT INTO SecondTable VALUES(105, 100, 5);");
    Step("INSERT INTO SecondTable VALUES(110, 50, 2);");
    Step("INSERT INTO SecondTable VALUES(160, 100, 2);");
    Prepare("SELECT * FROM SpanjoinTable");
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 120);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 40);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 2);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 160);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 60);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 2);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 100);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 5);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 5);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 105);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 5);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 5);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 110);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 50);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 5);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 160);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 10);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 5);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_DONE);
}
/**
 * @tc.name: SpanjoinTwoTableWithoutPartitioned
 * @tc.desc: SpanjoinTwoTable with ts,dur,and without partitioned cpu
 * @tc.type: FUNC
 */
HWTEST_F(SpanJoinTest, SpanjoinTwoTableWithoutPartitioned, TestSize.Level2)
{
    TS_LOGI("test30-2");
    Step("CREATE TABLE FirstTable(ts UNSIGNED INT PRIMARY KEY, dur UNSIGNED INT);");
    Step("CREATE TABLE SecondTable(ts UNSIGNED INT PRIMARY KEY, dur UNSIGNED INT);");
    Step(
        "CREATE VIRTUAL TABLE SpanjoinTable using span_join(FirstTable partitioned cpu, SecondTable partitioned cpu);");
    Step("INSERT INTO FirstTable VALUES(100, 10);");
    Step("INSERT INTO FirstTable VALUES(110, 50);");
    Step("INSERT INTO FirstTable VALUES(120, 100);");
    Step("INSERT INTO FirstTable VALUES(160, 10);");
    Step("INSERT INTO SecondTable VALUES(100, 5);");
    Step("INSERT INTO SecondTable VALUES(105, 100);");
    Step("INSERT INTO SecondTable VALUES(110, 50);");
    Step("INSERT INTO SecondTable VALUES(160, 100);");
    Prepare("SELECT * FROM SpanjoinTable");
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_DONE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_DONE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_DONE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_DONE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_DONE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_DONE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_DONE);
}
/**
 * @tc.name: SpanjoinTwoTableWithoutTs
 * @tc.desc: SpanjoinTwoTableWithoutTs with dur, partitioned cpu and without ts
 * @tc.type: FUNC
 */
HWTEST_F(SpanJoinTest, SpanjoinTwoTableWithoutTs, TestSize.Level3)
{
    TS_LOGI("test30-3");
    Step("CREATE TABLE FirstTable(dur UNSIGNED INT, cpu UNSIGNED INT);");
    Step("CREATE TABLE SecondTable(dur UNSIGNED INT, cpu UNSIGNED INT);");
    Step(
        "CREATE VIRTUAL TABLE SpanjoinTable using span_join(FirstTable partitioned cpu, SecondTable partitioned cpu);");
    Step("INSERT INTO FirstTable VALUES(10, 5);");
    Step("INSERT INTO FirstTable VALUES(50, 5);");
    Step("INSERT INTO FirstTable VALUES(100, 2);");
    Step("INSERT INTO FirstTable VALUES(10, 5);");
    Step("INSERT INTO SecondTable VALUES(5, 5);");
    Step("INSERT INTO SecondTable VALUES(100, 5);");
    Step("INSERT INTO SecondTable VALUES(50, 2);");
    Step("INSERT INTO SecondTable VALUES(100, 2);");
    Prepare("SELECT * FROM SpanjoinTable");
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
}
/**
 * @tc.name: SpanjoinTwoTableWithoutDur
 * @tc.desc: SpanjoinTwoTableWithoutTs with ts, partitioned cpu and without dur
 * @tc.type: FUNC
 */
HWTEST_F(SpanJoinTest, SpanjoinTwoTableWithoutDur, TestSize.Level4)
{
    TS_LOGI("test30-4");
    Step("CREATE TABLE FirstTable(ts UNSIGNED INT PRIMARY KEY, cpu UNSIGNED INT);");
    Step("CREATE TABLE SecondTable(ts UNSIGNED INT PRIMARY KEY, cpu UNSIGNED INT);");
    Step(
        "CREATE VIRTUAL TABLE SpanjoinTable using span_join(FirstTable partitioned cpu, SecondTable partitioned cpu);");
    Step("INSERT INTO FirstTable VALUES(100, 5);");
    Step("INSERT INTO FirstTable VALUES(110, 5);");
    Step("INSERT INTO FirstTable VALUES(120, 2);");
    Step("INSERT INTO FirstTable VALUES(160, 5);");
    Step("INSERT INTO SecondTable VALUES(100, 5);");
    Step("INSERT INTO SecondTable VALUES(105, 5);");
    Step("INSERT INTO SecondTable VALUES(110, 2);");
    Step("INSERT INTO SecondTable VALUES(160, 2);");
    Prepare("SELECT * FROM SpanjoinTable");
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 0), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 1), 0);
    EXPECT_EQ(sqlite3_column_int64(stmt_, 2), 0);
    EXPECT_EQ(sqlite3_step(stmt_), SQLITE_MISUSE);
}
} // namespace TraceStreamer
} // namespace SysTuning
