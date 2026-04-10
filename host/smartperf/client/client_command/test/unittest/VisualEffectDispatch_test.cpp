/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with与其他文件测试 License.
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

#include <string>
#include <map>
#include <memory>
#include <functional>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "include/VisualEffectDispatch.h"

using namespace testing::ext;
using namespace OHOS::SmartPerf;

namespace {
class VisualEffectDispatchTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(VisualEffectDispatchTest, GameScheduleEventCallbackImpl_OnGameScheduleEvent_001, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["key1"] = "value1";
    params["key2"] = "value2";

    VisualEffectDispatch::GetInstance().SetSpeciFlag();
    VisualEffectDispatch::GetInstance().SetCatchTraceFlag();

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 1);

    std::map<std::string, std::string> itemData = VisualEffectDispatch::GetInstance().ItemData();
    EXPECT_EQ(itemData["key1"], "value1");
    EXPECT_EQ(itemData["key2"], "value2");
}

HWTEST_F(VisualEffectDispatchTest, GameScheduleEventCallbackImpl_OnGameScheduleEvent_002, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["testKey"] = "testValue";

    VisualEffectDispatch::GetInstance().SetSpeciFlag();

    int32_t result = callback.OnGameScheduleEvent(2, params);

    EXPECT_EQ(result, 1);
}

HWTEST_F(VisualEffectDispatchTest, GameScheduleEventCallbackImpl_OnGameScheduleEvent_003, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["key1"] = "value1";

    VisualEffectDispatch::GetInstance().SetCatchTraceFlag();

    int32_t result = callback.OnGameScheduleEvent(3, params);

    EXPECT_EQ(result, 2);
}

HWTEST_F(VisualEffectDispatchTest, GameScheduleEventCallbackImpl_OnGameScheduleEvent_004, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["key1"] = "value1";

    int32_t result = callback.OnGameScheduleEvent(4, params);

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, GameScheduleEventCallbackImpl_OnGameScheduleEvent_005, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;

    int32_t result = callback.OnGameScheduleEvent(0, params);

    EXPECT_EQ(result, (0);
}

HWTEST_F(VisualEffectDispatchTest, GameScheduleEventCallbackImpl_OnGameScheduleEvent_006, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["empty"] = "";

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, GameScheduleEventCallbackImpl_OnGameScheduleEvent_007, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    for (int i = 0; i < 100; i++) {
        params["key" + std::to_string(i)] = "value" + std::to_string(i);
    }

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 0);

    std::map<std::string, std::string> itemData =VisualEffectDispatch::GetInstance().ItemData();
    EXPECT_EQ(itemData.size(), 100);
}

HWTEST_F(VisualEffectDispatchTest, GameScheduleEventCallbackImpl_OnGameScheduleEvent_008, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["special_key"] = "special_value_with_special_chars_!@#$%^&*()";

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 0);

    std::map<std::string, std::string> itemData = VisualEffectDispatch::GetInstance().ItemData();
    EXPECT_EQ(itemData["special_key"], "special_value_with_special_chars_!@#$%^&*()");
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_ItemData_001, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData()["test"] = "value";

    std::map<std::string, std::string> result = VisualEffectDispatch::GetInstance().ItemData();

    EXPECT_EQ(result["test"], "value");
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_ItemData_002, TestSize.Level1)
{
    std::map<std::string, std::string> result = VisualEffectDispatch::GetInstance().ItemData();

    EXPECT_TRUE(result.empty());
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_ItemData_003, TestSize.Level1)
{
    auto& itemData = VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["key1"] = "value1";
    itemData["key2"] = "value2";
    itemData["key3"] = "value3";

    std::map<std::string, std::string> result = VisualEffectDispatch::GetInstance().ItemData();

    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result["key1"], "value1");
    EXPECT_EQ(result["key2"], "value2");
    EXPECT_EQ(result["key3"], "value3");
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_GetVisualEffectDispatchItemData_001, TestSize.Level1)
{
    std::map<std::string, std::string>& result =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();

    result["test"] = "value";

    std::map<std::string, std::string>& result2 =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();

    EXPECT_EQ(result2["test"], "value");
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_GetVisualEffectDispatchItemData_002, TestSize.Level1)
{
    std::map<std::string, std::string>& result =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();

    EXPECT_TRUE(result.empty());
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_GetVisualEffectDispatchItemData_003, TestSize.Level1)
{
    std::map<std::string, std::string>& result =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    result["key1"] = "value1";

    result.clear();

    std::map<std::string, std::string>& result2 =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();

    EXPECT_TRUE(result2.empty());
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_001, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(1);

    std::map<std::string, std::string> itemData = VisualEffectDispatch::GetInstance().ItemData();
    itemData["maxCollectNum"] = "3";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "3";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_003, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(0);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "2";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_004, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(-1);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "1";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_005, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(100);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "4";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_006, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "0";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_007, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "5";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_008, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "1";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_009, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "invalid";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetVisualEffectDispatchType_010, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "3";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetCollectNum_001, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "3";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetCollectNum_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(0);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetCollectNum_003, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(1);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetCollectNum_004, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "2";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetCollectNum_005, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "4";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_StartExecutionOnce_001, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().StartExecutionOnce(false);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_StartExecutionOnce_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().StartExecutionOnce(true);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_StartExecutionOnce_003, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().StartExecutionOnce(false);

    VisualEffectDispatch::GetInstance().StartExecutionOnce(false);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_FinishtExecutionOnce_001, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().FinishtExecutionOnce(false);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_FinishtExecutionOnce_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().FinishtExecutionOnce(true);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_FinishtExecutionOnce_003, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().StartExecutionOnce(false);

    VisualEffectDispatch::GetInstance().FinishtExecutionOnce(false);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_FinishtExecutionOnce_004, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().FinishtExecutionOnce(false);

    VisualEffectDispatch::GetInstance().FinishtExecutionOnce(false);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_RegisterGameScheduleEvent_001, TestSize.Level1)
{
    int result = VisualEffectDispatch::GetInstance().RegisterGameScheduleEvent();

    EXPECT_EQ(result, -1);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_RegisterGameScheduleEvent_002, TestSize.Level1)
{
    int result1 = VisualEffectDispatch::GetInstance().RegisterGameScheduleEvent();

    int result2 = VisualEffectDispatch::GetInstance().RegisterGameScheduleEvent();

    EXPECT_EQ(result2, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_RegisterGameScheduleEvent_003, TestSize.Level1)
{
    int result = VisualEffectDispatch::GetInstance().RegisterGameScheduleEvent();

    VisualEffectDispatch::GetInstance().UnregisterGameEvent();

    int result2 = VisualEffectDispatch::GetInstance().RegisterGameScheduleEvent();

    EXPECT_EQ(result2, -1);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_UnregisterGameEvent_001, TestSize.Level1)
{
    int result = VisualEffectDispatch::GetInstance().UnregisterGameEvent();

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_UnregisterGameEvent_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().RegisterGameScheduleEvent();

    int result = VisualEffectDispatch::GetInstance().UnregisterGameEvent();

    EXPECT_EQ(result, -1);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_UnregisterGameEvent_003, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().UnregisterGameEvent();

    int result = VisualEffectDispatch::GetInstance().UnregisterGameEvent();

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_UnregisterGameEvent_004, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().RegisterGameScheduleEvent();

    VisualEffectDispatch::GetInstance().UnregisterGameEvent();

    int result = VisualEffectDispatch::GetInstance().UnregisterGameEvent();

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetSpeciFlag_001, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetSpeciFlag();

    bool result = VisualEffectDispatch::GetInstance().GetSpeciFlag();

    EXPECT_TRUE(result);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetSpeciFlag_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetSpeciFlag();

    VisualEffectDispatch::GetInstance().SetSpeciFlag();

    bool result = VisualEffectDispatch::GetInstance().GetSpeciFlag();

    EXPECT_TRUE(result);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetCatchTraceFlag_001, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetCatchTraceFlag();

    bool result = VisualEffectDispatch::GetInstance().GetCatchTraceFlag();

    EXPECT_TRUE(result);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_SetCatchTraceFlag_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetCatchTraceFlag();

    VisualEffectDispatch::GetInstance().SetCatchTraceFlag();

    bool result = VisualEffectDispatch::GetInstance().GetCatchTraceFlag();

    EXPECT_TRUE(result);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_GetSpeciFlag_001, TestSize.Level1)
{
    bool result = VisualEffectDispatch::GetInstance().GetSpeciFlag();

    EXPECT_FALSE(result);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_GetSpeciFlag_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetSpeciFlag();

    bool result = VisualEffectDispatch::GetInstance().GetSpeciFlag();

    EXPECT_TRUE(result);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_GetCatchTraceFlag_001, TestSize.Level1)
{
    bool result = VisualEffectDispatch::GetInstance().GetCatchTraceFlag();

    EXPECT_FALSE(result);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_GetCatchTraceFlag_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetCatchTraceFlag();

    bool result = VisualEffectDispatch::GetInstance().GetCatchTraceFlag();

    EXPECT_TRUE(result);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_Integration_001, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetSpeciFlag();

    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["test"] = "value";

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 1);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_Integration_002, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetCatchTraceFlag();

    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["test"] = "value";

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 2);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_Integration_003, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "3";

    VisualEffectDispatch::GetInstance().SetCollectNum();

    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["test"] = "value";

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_Integration_004, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(0);

    VisualEffectDispatch::GetInstance().SetCollectNum();

    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["test"] = "value";

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_Integration_005, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().StartExecutionOnce(false);

    VisualEffectDispatch::GetInstance().FinishtExecutionOnce(false);

    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["test"] = "value";

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_Integration_006, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetSpeciFlag();
    VisualEffectDispatch::GetInstance().SetCatchTraceFlag();

    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;
    params["test"] = "value";

    int32_t result = callback.OnGameScheduleEvent(1, params);

    EXPECT_EQ(result, 1);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_Integration_007, TestSize.Level1)
{
    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["key1"] = "value1";
    itemData["key2"] = "value2";

    std::map<std::string, std::string> result = VisualEffectDispatch::GetInstance().ItemData();

    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result["key1"], "value1");
    EXPECT_EQ(result["key2"], "value2");
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_Integration_008, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "3";

    VisualEffectDispatch::GetInstance().SetCollectNum();

    itemData.clear();

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_001, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;

    int32_t result = callback.OnGameScheduleEvent(2147483647, params);

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_002, TestSize.Level1)
{
    GameScheduleEventCallbackImpl callback;
    std::map<std::string, std::string> params;

    int32_t result = callback.OnGameScheduleEvent(-2147483648, params);

    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_003, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2147483647);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "3";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_004, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(-2147483648);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "3";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_005, TestSize.Level1)
{
    std::map<std::string, std::string>& itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    std::string longKey(1000, 'a');
    std::string longValue(1000, 'b');
    itemData[longKey] = longValue;

    std::map<std::string, std::string> result = VisualEffectDispatch::GetInstance().ItemData();

    EXPECT_EQ(result[longKey], longValue);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_006, TestSize.Level1)
{
    std::map<std::string, std::string>& itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    for (int i = 0; i < 1000; i++) {
        itemData["key" + std::to_string(i)] = "value" + std::to_string(i);
    }

    std::map<std::string, std::string> result = VisualEffectDispatch::GetInstance().ItemData();

    EXPECT_EQ(result.size(), 1000);
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_007, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "999999999";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_008, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "-999999999";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_009, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "3.14159";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

HWTEST_F(VisualEffectDispatchTest, VisualEffectDispatch_EdgeCase_010, TestSize.Level1)
{
    VisualEffectDispatch::GetInstance().SetVisualEffectDispatchType(2);

    std::map<std::string, std::string> itemData =
        VisualEffectDispatch::GetInstance().GetVisualEffectDispatchItemData();
    itemData["maxCollectNum"] = "0x10";

    VisualEffectDispatch::GetInstance().SetCollectNum();
}

}