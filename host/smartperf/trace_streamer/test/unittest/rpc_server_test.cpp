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

#include "rpc/rpc_server.h"

using namespace testing::ext;
namespace SysTuning {
namespace TraceStreamer {
class RpcServerTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }
    void TearDown() {}

public:
    TraceStreamerSelector stream_ = {};
};
std::string g_result;
void res(const std::string result, int32_t finish)
{
    TS_LOGI("%s", result.c_str());
    g_result = result;
}

/**
 * @tc.name: CorrectTraceData
 * @tc.desc: Upload correct trace file data
 * @tc.type: FUNC
 */
HWTEST_F(RpcServerTest, CorrectTraceData, TestSize.Level1)
{
    TS_LOGI("test27-1");
    std::string PARSERDATA("sugov:0-178   (  178) [001] .... 28462.257501: cpu_frequency: state=816000 cpu_id=0 \n");
    std::string SQLQUERY("select * from measure;");

    RpcServer rpcServer;
    auto ret = rpcServer.ParseData((const uint8_t*)PARSERDATA.c_str(), PARSERDATA.length(), res);
    EXPECT_TRUE(res);
    EXPECT_TRUE(ret);
    ret = rpcServer.ParseDataOver((const uint8_t*)PARSERDATA.c_str(), PARSERDATA.length(), res);
    EXPECT_TRUE(res);
    EXPECT_TRUE(ret);
    ret = rpcServer.SqlQuery((const uint8_t*)SQLQUERY.c_str(), SQLQUERY.length(), res);
    EXPECT_TRUE(res);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: WrongTraceData
 * @tc.desc: Upload wrong tracking file data
 * @tc.type: FUNC
 */
HWTEST_F(RpcServerTest, WrongTraceData, TestSize.Level1)
{
    TS_LOGI("test27-2");
    std::string PARSERDATA("sugov:0-178   (  178) [001] .... 28462.277458: cpu_frequency: state=600000 cpu_id=2 \n");
    std::string SQLQUERY("select * from measure_e;");

    RpcServer rpcServer;
    auto ret = rpcServer.ParseData((const uint8_t*)PARSERDATA.c_str(), PARSERDATA.length(), res);
    EXPECT_TRUE(res);
    EXPECT_TRUE(ret);
    ret = rpcServer.ParseDataOver((const uint8_t*)PARSERDATA.c_str(), PARSERDATA.length(), res);
    EXPECT_TRUE(res);
    EXPECT_TRUE(ret);
    ret = rpcServer.SqlQuery((const uint8_t*)SQLQUERY.c_str(), SQLQUERY.length(), res);
    EXPECT_TRUE(g_result == "dberror\r\n");
    EXPECT_FALSE(ret);
}
} // namespace TraceStreamer
} // namespace SysTuning
