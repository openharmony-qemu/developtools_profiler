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

#include <gtest/gtest.h>
#include "sp_utils.h"
#include "parse_radar.h"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include "Dubai.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class DubaiTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(DubaiTest, DubaiBeginTest, TestSize.Level1)
{
    Dubai &dubai = Dubai::GetInstance();
    std::string result;
    auto ret = OHOS::SmartPerf::SPUtils::LoadCmd("hidumper -s 1213 -a '-b'", result);
    dubai.DumpDubaiBegin();
    
    EXPECT_EQ(ret, true);
}

HWTEST_F(DubaiTest, DubaiFinishTest, TestSize.Level1)
{
    Dubai &dubai = Dubai::GetInstance();
    std::string result;
    auto ret = OHOS::SmartPerf::SPUtils::LoadCmd("hidumper -s 1213 -a '-f'", result);
    dubai.DumpDubaiFinish();
    
    EXPECT_EQ(ret, true);
}

HWTEST_F(DubaiTest, DubaiMoveTest, TestSize.Level1)
{
    Dubai &dubai = Dubai::GetInstance();
    std::string result;
    std::string dubaiName = "dubai.db";
    std::string dubaiPath = "/data/system_ce/0/dubai/" + dubaiName;
    std::string devicePath = "/data/app/el2/100/database/com.ohos.smartperf/entry/rdb";
    auto ret = OHOS::SmartPerf::SPUtils::LoadCmd("cp " + dubaiPath + " " + devicePath, result);
    auto ret1 = OHOS::SmartPerf::SPUtils::LoadCmd("chmod 777 " + devicePath + "/" + dubaiName, result);
    dubai.MoveDubaiDb();
    
    EXPECT_EQ(ret, true);
    EXPECT_EQ(ret1, true);
}
}
}