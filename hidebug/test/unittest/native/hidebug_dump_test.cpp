/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstring>
#include <unistd.h>

#include <gtest/hwext/gtest-ext.h>
#include <gtest/hwext/gtest-tag.h>

#include "hidebug/hidebug.h"
#include "hidebug/hidebug_type.h"
#include "hidebug_dump.h"

using namespace testing::ext;
using namespace OHOS::HiviewDFX;

class HidebugDumpTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: OH_HiDebug_RegisterMemDumpListener_NullName
 * @tc.desc: Register a listener with null name should return HIDEBUG_INVALID_ARGUMENT.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugDumpTest, OH_HiDebug_RegisterMemDumpListener_NullName, TestSize.Level0)
{
    auto listener = [](int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg) -> bool {
        return true;
    };
    HiDebug_ErrorCode ret = OH_HiDebug_RegisterMemDumpListener(nullptr, listener);
    EXPECT_EQ(ret, HIDEBUG_INVALID_ARGUMENT);
}

/**
 * @tc.name: OH_HiDebug_RegisterMemDumpListener_NullListener
 * @tc.desc: Register a null listener should return HIDEBUG_INVALID_ARGUMENT.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugDumpTest, OH_HiDebug_RegisterMemDumpListener_NullListener, TestSize.Level0)
{
    HiDebug_ErrorCode ret = OH_HiDebug_RegisterMemDumpListener("testListener", nullptr);
    EXPECT_EQ(ret, HIDEBUG_INVALID_ARGUMENT);
}

/**
 * @tc.name: OH_HiDebug_RegisterMemDumpListener_UpdateExisting
 * @tc.desc: Re-registering with the same name should update the listener and return HIDEBUG_SUCCESS.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugDumpTest, OH_HiDebug_RegisterMemDumpListener_UpdateExisting, TestSize.Level1)
{
    auto listener1 = [](int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg) -> bool {
        return true;
    };
    auto listener2 = [](int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg) -> bool {
        return false;
    };
    EXPECT_EQ(OH_HiDebug_RegisterMemDumpListener("updateListener", listener1), HIDEBUG_SUCCESS);
    EXPECT_EQ(OH_HiDebug_RegisterMemDumpListener("updateListener", listener2), HIDEBUG_SUCCESS);
}

/**
 * @tc.name: HidebugMemDumpListener_FindListener_Found
 * @tc.desc: FindListener should return the registered listener.
 * @tc.type: FUNC
 */

HWTEST_F(HidebugDumpTest, HidebugMemDumpListener_FindListener_Found, TestSize.Level1)
{
    auto listener = [](int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg) -> bool {
        return true;
    };
    EXPECT_EQ(OH_HiDebug_RegisterMemDumpListener("findableListener", listener), HIDEBUG_SUCCESS);
    auto found = HidebugMemDumpListener::GetInstance().FindListener("findableListener");
    EXPECT_NE(found, nullptr);
}

/**
 * @tc.name: HidebugMemDumpListener_TriggerListener_EmptyName
 * @tc.desc: TriggerListener with empty name should return false.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugDumpTest, HidebugMemDumpListener_TriggerListener_EmptyName, TestSize.Level1)
{
    bool ret = HidebugMemDumpListener::GetInstance().TriggerListener("", 1, OH_HiDebug_DO_NOTHING, "arg");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: HidebugMemDumpListener_TriggerListener_NullArg
 * @tc.desc: TriggerListener with null arg should return false.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugDumpTest, HidebugMemDumpListener_TriggerListener_NullArg, TestSize.Level1)
{
    bool ret = HidebugMemDumpListener::GetInstance().TriggerListener("anyListener", 1, OH_HiDebug_DO_NOTHING, nullptr);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: HidebugMemDumpListener_TriggerListener_DoNothingTag
 * @tc.desc: TriggerListener with DO_NOTHING tag should invoke the listener and return its result.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugDumpTest, HidebugMemDumpListener_TriggerListener_DoNothingTag, TestSize.Level1)
{
    auto listener = [](int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg) -> bool {
        return tag == OH_HiDebug_DO_NOTHING;
    };
    EXPECT_EQ(OH_HiDebug_RegisterMemDumpListener("doNothingListener", listener), HIDEBUG_SUCCESS);
    bool ret = HidebugMemDumpListener::GetInstance().TriggerListener(
        "doNothingListener", 1, OH_HiDebug_DO_NOTHING, "testArg");
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: HidebugMemDumpListener_TriggerListener_RunningGCTag
 * @tc.desc: TriggerListener with RUNNING_GC tag should invoke the listener with correct tag.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugDumpTest, HidebugMemDumpListener_TriggerListener_RunningGCTag, TestSize.Level1)
{
    auto listener = [](int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg) -> bool {
        return tag == OH_HiDebug_RUNNING_GC;
    };
    EXPECT_EQ(OH_HiDebug_RegisterMemDumpListener("gcListener", listener), HIDEBUG_SUCCESS);
    bool ret = HidebugMemDumpListener::GetInstance().TriggerListener(
        "gcListener", 1, OH_HiDebug_RUNNING_GC, "gcArg");
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: HidebugMemDumpListener_TriggerListener_DumpSnapshotTag
 * @tc.desc: TriggerListener with DUMP_SNAPSHOT tag should invoke the listener with correct tag.
 * @tc.type: FUNC
 */
HWTEST_F(HidebugDumpTest, HidebugMemDumpListener_TriggerListener_DumpSnapshotTag, TestSize.Level1)
{
    auto listener = [](int32_t fd, OH_HiDebug_MemListenerType tag, const char* arg) -> bool {
        return tag == OH_HiDebug_DUMP_SNAPSHOT;
    };
    EXPECT_EQ(OH_HiDebug_RegisterMemDumpListener("snapshotListener", listener), HIDEBUG_SUCCESS);
    bool ret = HidebugMemDumpListener::GetInstance().TriggerListener(
        "snapshotListener", 1, OH_HiDebug_DUMP_SNAPSHOT, "snapshotArg");
    EXPECT_TRUE(ret);
}
