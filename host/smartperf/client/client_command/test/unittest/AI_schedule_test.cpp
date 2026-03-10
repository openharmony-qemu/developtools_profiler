/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <functional>
#include "AI_schedule.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonAIScheduleTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: AISchedule::SetProcessId
 * @tc.desc: Test SetProcessId with valid process ID
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, SetProcessIdTest001, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    std::string testPid = "12345";
    
    aiSchedule.SetProcessId(testPid);
    auto result = aiSchedule.ItemData();
    
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: AISchedule::SetProcessId
 * @tc.desc: Test SetProcessId with empty string
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, SetProcessIdTest002, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    std::string testPid = "";
    
    aiSchedule.SetProcessId(testPid);
    auto result = aiSchedule.ItemData();
    
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name: AISchedule::SetProcessId
 * @tc.desc: Test SetProcessId with long process ID string
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, SetProcessIdTest003, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    std::string testPid = "9999999999";
    
    aiSchedule.SetProcessId(testPid);
    auto result = aiSchedule.ItemData();
    
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: AISchedule::ItemData
 * @tc.desc: Test ItemData with valid process ID
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, ItemDataTest001, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    aiSchedule.SetProcessId("10086");
    
    auto result = aiSchedule.ItemData();
    
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: AISchedule::ItemData
 * @tc.desc: Test ItemData without setting process ID
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, ItemDataTest002, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    
    auto result = aiSchedule.ItemData();
    
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: AISchedule::ItemData
 * @tc.desc: Test ItemData with numeric process ID
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, ItemDataTest003, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    aiSchedule.SetProcessId("1");
    
    auto result = aiSchedule.ItemData();
    
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: AISchedule::ItemData
 * @tc.desc: Test ItemData with alphanumeric process ID
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, ItemDataTest004, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    aiSchedule.SetProcessId("pid_123");
    
    auto result = aiSchedule.ItemData();
    
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: AISchedule::ItemData
 * @tc.desc: Test ItemData with maximum process ID value
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, ItemDataTest005, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    aiSchedule.SetProcessId("2147483647");
    
    auto result = aiSchedule.ItemData();
    
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: AISchedule::ItemData
 * @tc.desc: Test ItemData with zero process ID
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, ItemDataTest006, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    aiSchedule.SetProcessId("0");
    
    auto result = aiSchedule.ItemData();
    
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name: AISchedule::GetInstance
 * @tc.desc: Test GetInstance returns singleton instance
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, GetInstanceTest001, TestSize.Level1)
{
    AISchedule &instance1 = AISchedule::GetInstance();
    AISchedule &instance2 = AISchedule::GetInstance();
    
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name: AISchedule::ItemData
 * @tc.desc: Test ItemData multiple calls with same process ID
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, ItemDataTest007, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    aiSchedule.SetProcessId("12345");
    
    auto result1 = aiSchedule.ItemData();
    auto result2 = aiSchedule.ItemData();
    
    EXPECT_FALSE(result1.empty());
    EXPECT_FALSE(result2.empty());
}

/**
 * @tc.name: AISchedule::ItemData
 * @tc.desc: Test ItemData with different process IDs
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonAIScheduleTest, ItemDataTest008, TestSize.Level1)
{
    AISchedule &aiSchedule = AISchedule::GetInstance();
    
    aiSchedule.SetProcessId("1001");
    auto result1 = aiSchedule.ItemData();
    
    aiSchedule.SetProcessId("1002");
    auto result2 = aiSchedule.ItemData();
    
    EXPECT_FALSE(result1.empty());
    EXPECT_FALSE(result2.empty());
}
}
}
