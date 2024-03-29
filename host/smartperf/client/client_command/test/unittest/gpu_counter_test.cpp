/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <gtest/gtest.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <functional>
#include <stdio.h>
#include "GpuCounter.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: GpuCounter::Init
 * @tc.desc: Test Init
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, Init001, TestSize.Level1)
{
    std::string recvBuf;
    std::map<std::string, std::string> data;
    int ret = GpuCounter::GetInstance().Init(recvBuf, data);

    EXPECT_EQ(ret, -4);
}

/*
// 002屏蔽GpuCounter.cpp 第35、78-83行进行测试（后续测试添加依赖文件，文件齐全，所以屏蔽该测试）
HWTEST_F(SPdaemonTest, Init002, TestSize.Level1)
{
    std::string recvBuf;
    std::map<std::string, std::string> data;
    int ret = GpuCounter::GetInstance().Init(recvBuf, data);   // 外部依赖文件不全，返回-1

    EXPECT_EQ(ret, -1);
}

// 003屏蔽GpuCounter.cpp 第35、78-83行进行测试
HWTEST_F(SPdaemonTest, Init003, TestSize.Level1)
{
    std::string recvBuf = "";
    std::map<std::string, std::string> data;
    int ret = GpuCounter::GetInstance().Init(recvBuf, data);   // recvBuf为空，返回-2

    EXPECT_EQ(ret, -2);
}

// 004屏蔽GpuCounter.cpp 第35、78-83行进行测试
HWTEST_F(SPdaemonTest, Init004, TestSize.Level1)
{
    std::string recvBuf = "ABCDE";
    std::map<std::string, std::string> data;
    int ret = GpuCounter::GetInstance().Init(recvBuf, data);   // 无法访问sandBoxPath路径,返回-3

    EXPECT_EQ(ret, -3);
}

// 005屏蔽GpuCounter.cpp 第35、78-83行进行测试
HWTEST_F(SPdaemonTest, Init005, TestSize.Level1)
{
    std::string recvBuf = "com.ohos.smartperf1";
    std::map<std::string, std::string> data;
    int ret = GpuCounter::GetInstance().Init(recvBuf, data);

    EXPECT_EQ(ret, -1);   // 此处的-1为接口中调用Start所返回的-1
}
*/

/**
 * @tc.name: GpuCounter::ItemData
 * @tc.desc: Test ItemData
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, ItemData001, TestSize.Level1)
{
    bool ret = false;
    std::string recvBuf = "catch_gpu_counter";
    std::map<std::string, std::string> data;
    std::map<std::string, std::string> mapInfo;

    GpuCounter::GetInstance().Init(recvBuf, data);
    mapInfo = GpuCounter::GetInstance().ItemData();

    if (!mapInfo.empty()) {
        ret = true;
    }

    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: GpuCounter::GetCounterDuration
 * @tc.desc: Test GetCounterDuration
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, GetCounterDuration001, TestSize.Level1)
{
    int ret = -1;
    const char* cmd = "cd /data/local/tmp";
    std::string fileName = "config.txt";

    ret = system(cmd);
    FILE *file = fopen(fileName.c_str(), "r+");

    long long lg = GpuCounter::GetInstance().GetCounterDuration();
    if (ret == 0 && file != NULL) {
        if (lg >= 0) {
            ret = 1;
        }
        EXPECT_EQ(ret, 1);
    } else {
        EXPECT_EQ(lg, -1);
    }
}

/**
 * @tc.name: GpuCounter::Start
 * @tc.desc: Test Start
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, Start001, TestSize.Level1)
{
    int ret = GpuCounter::GetInstance().Start();
    EXPECT_EQ(ret, -1);
}

/**
 * @tc.name: GpuCounter::Rest
 * @tc.desc: Test Rest
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, Rest001, TestSize.Level1)
{
    GpuCounter::GetInstance().Rest();
}

/**
 * @tc.name: GpuCounter::Check
 * @tc.desc: Test Check
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonTest, Check001, TestSize.Level1)
{
    std::string recvBuf = "catch_gpu_counter";
    std::map<std::string, std::string> data;
    GpuCounter::GetInstance().Init(recvBuf, data);
    GpuCounter::GetInstance().Check();
}

}
}
