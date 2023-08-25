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
#include "gpu_counter_object_table.h"
#include "gpu_counter_table.h"
#include "mock_plugin_result.pb.h"
#include "sdk_data_parser.h"
#include "slice_object_table.h"
#include "slice_table.h"
#include "ts_sdk_api.h"

using namespace testing::ext;
namespace SysTuning ::TraceStreamer {
class SDKApiTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }
    void TearDown() {}

public:
    TraceStreamerSelector stream_ = {};
    RpcServer* rpcServer = new RpcServer();
};

std::string g_resultTest;
void res(const std::string result, int32_t finish, int32_t isConfig)
{
    TS_LOGI("%s", result.c_str());
    g_resultTest = result;
}

std::string g_reply;
void QueryResultCallback(const std::string& jsonResult, int32_t finish, int32_t isConfig)
{
    g_reply = jsonResult;
}

/**
 * @tc.name: SetTableName
 * @tc.desc: Set the table name manually
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, SetTableName, TestSize.Level1)
{
    TS_LOGI("test1-1");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName("first_table", "second_table", "third_table", "fouth_table");
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from first_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_TRUE(ret);
    EXPECT_EQ(g_resultTest.find("ok"), 0);

    std::string sqlQueryCounterObj("select * from second_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
    std::string sqlQuerySlice("select * from third_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
    std::string sqlQuerySliceObj("select * from fouth_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DefaultTableName
 * @tc.desc: Use the default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, DefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-2");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from counter_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
    std::string sqlQueryCounterObj("select * from gpu_counter_object;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
    std::string sqlQuerySlice("select * from slice_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
    std::string sqlQuerySliceObj("select * from slice_object_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: NullTableName
 * @tc.desc: Use the null table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, NullTableName, TestSize.Level1)
{
    TS_LOGI("test1-3");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", " ", " ", " ");
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from counter_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
    std::string sqlQueryCounterObj("select * from gpu_counter_object;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
    std::string sqlQuerySlice("select * from slice_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
    std::string sqlQuerySliceObj("select * from slice_object_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: NullAndManuallyCounterTableName
 * @tc.desc: Use the null and manually counter table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, NullAndManuallyCounterTableName, TestSize.Level1)
{
    TS_LOGI("test1-4");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName("first_table", "second_table", " ", " ");
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from first_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
    std::string sqlQueryCounterObj("select * from second_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
    std::string sqlQuerySlice("select * from slice_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
    std::string sqlQuerySliceObj("select * from slice_object_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: NullAndManuallySliceTableName
 * @tc.desc: Use the null and manually slice table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, NullAndManuallySliceTableName, TestSize.Level1)
{
    TS_LOGI("test1-5");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", " ", "first_table", "second_table");
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from counter_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
    std::string sqlQueryCounterObj("select * from gpu_counter_object;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
    std::string sqlQuerySlice("select * from first_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
    std::string sqlQuerySliceObj("select * from second_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CurrentDataForCounterObjectWithDefaultTableName
 * @tc.desc: Use CurrentData for CounterObject table with default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CurrentDataForCounterObjectWithDefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-6");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounterObject(1, "counter_1");
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounterObj("select * from gpu_counter_object;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CurrentDataForCounterObjectWithManuallyTableName
 * @tc.desc: Use CurrentData for CounterObject table with manually table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CurrentDataForCounterObjectWithManuallyTableName, TestSize.Level1)
{
    TS_LOGI("test1-7");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", "second_table", " ", " ");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounterObject(1, "counter_1");
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounterObj("select * from second_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForCounterObjectWithDefaultTableName
 * @tc.desc: Use WrongData for CounterObject table with default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForCounterObjectWithDefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-8");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounterObject(INVALID_INT32, "counter_1");
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounterObj("select * from gpu_counter_object;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForCounterObjectWithManuallyTableName
 * @tc.desc: Use WrongData for CounterObject table with manually table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForCounterObjectWithManuallyTableName, TestSize.Level1)
{
    TS_LOGI("test1-9");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", "second_table", " ", " ");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounterObject(INVALID_INT32, "counter_1");
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounterObj("select * from second_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForCounterObject
 * @tc.desc: Use WrongData for CounterObject table
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForCounterObject, TestSize.Level1)
{
    TS_LOGI("test1-10");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", "second_table", " ", " ");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounterObject(INVALID_INT32, " ");
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounterObj("select * from gpu_counter_object;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounterObj.c_str(), sqlQueryCounterObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CurrentDataForCounterWithDefaultTableName
 * @tc.desc: Use CurrentData for Counter table with default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CurrentDataForCounterWithDefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-11");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounter(1, 100, 100);
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from counter_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CurrentDataForCounterWithManuallyTableName
 * @tc.desc: Use CurrentData for Counter table with manually table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CurrentDataForCounterWithManuallyTableName, TestSize.Level1)
{
    TS_LOGI("test1-12");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName("first_table", " ", " ", " ");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounter(1, 100, 100);
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from first_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForCounterWithDefaultTableName
 * @tc.desc: Use WrongData for Counter table with default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForCounterWithDefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-13");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounter(INVALID_INT32, 100, 100);
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from counter_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForCounterWithManuallyTableName
 * @tc.desc: Use WrongData for Counter table with manually table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForCounterWithManuallyTableName, TestSize.Level1)
{
    TS_LOGI("test1-14");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName("first_table", " ", " ", " ");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounter(INVALID_INT32, 100, 100);
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from first_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CounterWithWrongData
 * @tc.desc: Use wrongData for counter table
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CounterWithWrongData, TestSize.Level1)
{
    TS_LOGI("test1-15");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendCounter(INVALID_INT32, INVALID_UINT64, INVALID_INT32);
    EXPECT_EQ(0, ret);
    std::string sqlQueryCounter("select * from counter_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQueryCounter.c_str(), sqlQueryCounter.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CurrentDataForSliceObjectWithDefaultTableName
 * @tc.desc: Use CurrentData for SliceObject table with default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CurrentDataForSliceObjectWithDefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-16");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSliceObject(1, "slice_1");
    EXPECT_EQ(0, ret);
    std::string sqlQuerySliceObj("select * from slice_object_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CurrentDataForSliceObjectWithManuallyTableName
 * @tc.desc: Use CurrentData for SliceObject table with manually table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CurrentDataForSliceObjectWithManuallyTableName, TestSize.Level1)
{
    TS_LOGI("test1-17");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", " ", " ", "fourth_table");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSliceObject(1, "slice_1");
    EXPECT_EQ(0, ret);
    std::string sqlQuerySliceObj("select * from fourth_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForSliceObjectWithDefaultTableName
 * @tc.desc: Use WrongData for SliceObject table with default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForSliceObjectWithDefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-18");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSliceObject(1, "slice_1");
    EXPECT_EQ(0, ret);
    std::string sqlQuerySliceObj("select * from slice_object_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForSliceObjectWithManuallyTableName
 * @tc.desc: Use WrongData for SliceObject table with manually table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForSliceObjectWithManuallyTableName, TestSize.Level1)
{
    TS_LOGI("test1-19");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", " ", " ", "fourth_table");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSliceObject(INVALID_INT32, "slice_1");
    EXPECT_EQ(0, ret);
    std::string sqlQuerySliceObj("select * from slice_object_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: WrongDataForSliceObject
 * @tc.desc: Use WrongData for SliceObject table
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForSliceObject, TestSize.Level1)
{
    TS_LOGI("test1-20");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", " ", " ", "fourth_table");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSliceObject(INVALID_INT32, " ");
    EXPECT_EQ(0, ret);
    std::string sqlQuerySliceObj("select * from slice_object_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySliceObj.c_str(), sqlQuerySliceObj.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), string::npos);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CurrentDataForSliceWithDefaultTableName
 * @tc.desc: Use CurrentData for Slice table with default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CurrentDataForSliceWithDefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-21");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSlice(1, 100, 100, 100);
    EXPECT_EQ(0, ret);
    std::string sqlQuerySlice("select * from Slice_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: CurrentDataForSliceWithManuallyTableName
 * @tc.desc: Use CurrentData for Slice table with manually table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, CurrentDataForSliceWithManuallyTableName, TestSize.Level1)
{
    TS_LOGI("test1-22");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", " ", "third_table", " ");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSlice(1, 100, 100, 100);
    EXPECT_EQ(0, ret);
    std::string sqlQuerySlice("select * from third_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForSliceWithDefaultTableName
 * @tc.desc: Use WrongData for Slice table with default table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForSliceWithDefaultTableName, TestSize.Level1)
{
    TS_LOGI("test1-23");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSlice(INVALID_INT32, 100, 100, 100);
    EXPECT_EQ(0, ret);
    std::string sqlQuerySlice("select * from Slice_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongDataForSliceWithManuallyTableName
 * @tc.desc: Use WrongData for Slice table with manually table name
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, WrongDataForSliceWithManuallyTableName, TestSize.Level1)
{
    TS_LOGI("test1-24");
    SetRpcServer(rpcServer);
    auto ret = SDK_SetTableName(" ", " ", "third_table", " ");
    ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSlice(INVALID_INT32, 100, 100, 100);
    EXPECT_EQ(0, ret);
    std::string sqlQuerySlice("select * from third_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: SliceWithWrongData
 * @tc.desc: Use wrongData for slice table
 * @tc.type: FUNC
 */
HWTEST_F(SDKApiTest, SliceWithWrongData, TestSize.Level1)
{
    TS_LOGI("test1-25");
    SetRpcServer(rpcServer);
    auto ret = rpcServer->ts_->sdkDataParser_->GetJsonConfig(QueryResultCallback);
    EXPECT_EQ(0, ret);
    ret = rpcServer->ts_->sdkDataParser_->CreateTableByJson();
    EXPECT_EQ(0, ret);
    ret = SDK_AppendSlice(INVALID_INT32, INVALID_UINT64, INVALID_UINT64, INVALID_INT32);
    EXPECT_EQ(0, ret);
    std::string sqlQuerySlice("select * from slice_table;");
    ret = rpcServer->SqlQuery((const uint8_t*)sqlQuerySlice.c_str(), sqlQuerySlice.length(), res);
    EXPECT_EQ(g_resultTest.find("ok"), 0);
    EXPECT_TRUE(ret);
}
} // namespace SysTuning::TraceStreamer
