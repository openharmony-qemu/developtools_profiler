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
#include "GpuCounter.h"
#include "sp_utils.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonGpuTest : public testing::Test {
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
HWTEST_F(SPdaemonGpuTest, Init001, TestSize.Level1)
{
    std::string result;
    std::string constConfigFile = "/bin/config.txt";
    std::string constExecFile = "/bin/counters_collector";
    std::string constLibFile = "/bin/libGPU_PCM.so";
    std::string constV2File = "/bin/counters_collector_v2.txt";
    std::string constCheckProductInfo = "param get const.product.name";
    std::string constProductInfo = "60";
    std::string constWhoami = "whoami";
    std::string constUserInfo = "root";

    std::string recvBuf;
    std::map<std::string, std::string> data;
    int ret = GpuCounter::GetInstance().Init(recvBuf, data);

    SPUtils::LoadCmd(constWhoami, result);
    if (result.empty() || result.find(constUserInfo) == std::string::npos) {
        EXPECT_EQ(ret, -3);
        return;
    }

    SPUtils::LoadCmd(constCheckProductInfo, result);
    if (result.empty() || result.find(constProductInfo) == std::string::npos) {
        EXPECT_EQ(ret, -4);
        return;
    }
    if ((!SPUtils::FileAccess(constV2File)) || (!SPUtils::FileAccess(constExecFile)) ||
        (!SPUtils::FileAccess(constConfigFile)) || (!SPUtils::FileAccess(constLibFile))) {
        EXPECT_EQ(ret, -1);
        return;
    }

    EXPECT_EQ(ret, -2);
    return;
}

/*
// 002屏蔽GpuCounter.cpp 第35、78-83行进行测试（后续测试添加依赖文件，文件齐全，所以屏蔽该测试）
HWTEST_F(SPdaemonGpuTest, Init002, TestSize.Level1)
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
HWTEST_F(SPdaemonGpuTest, ItemData001, TestSize.Level1)
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
HWTEST_F(SPdaemonGpuTest, GetCounterDuration001, TestSize.Level1)
{
    int ret = -1;
    std::string fileName = "/bin/config.txt";
    bool isExist = SPUtils::FileAccess(fileName);

    long long lg = GpuCounter::GetInstance().GetCounterDuration();
    if (isExist) {
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
HWTEST_F(SPdaemonGpuTest, Start001, TestSize.Level1)
{
    int ret = GpuCounter::GetInstance().Start();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: GpuCounter::Rest
 * @tc.desc: Test Rest
 * @tc.type: FUNC
 */
// HWTEST_F(SPdaemonGpuTest, Rest001, TestSize.Level1)
// {
//     GpuCounter::GetInstance().Rest();
// }

/**
 * @tc.name: GpuCounter::Check
 * @tc.desc: Test Check
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonGpuTest, Check001, TestSize.Level1)
{
    std::string recvBuf = "catch_gpu_counter";
    std::map<std::string, std::string> data;
    GpuCounter::GetInstance().Init(recvBuf, data);
    GpuCounter::GetInstance().Check();
}
}
}
