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
#include "DDR.h"
#include "sp_utils.h"
#include "common.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class DDRTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(DDRTest, ItemDataTest, TestSize.Level1)
{
    bool ret = false;
    long long curFreq = DDR::GetInstance().GetDdrFreq();
    std::map<std::string, std::string> result = DDR::GetInstance().ItemData();
    EXPECT_EQ(result["ddrFre"], std::to_string(curFreq));

    if (!result.empty()) {
        ret = true;
    }
    EXPECT_EQ(ret, true);
}

long long GetDdrFreq()
{
    long long freq = -1;
    std::string ddrfreq = "123456";
    std::string ddrCurFreqPath = "/bin/config.txt";
    OHOS::SmartPerf::SPUils::LoadFile(ddrCurFreqPath, ddrfreq);
    freq = std::atoll(ddrfreq.c_str());
    return freq;
}
}
}