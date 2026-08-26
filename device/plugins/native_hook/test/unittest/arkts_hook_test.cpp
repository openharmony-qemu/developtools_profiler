/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include <atomic>
#include <cstring>
#include <gtest/gtest.h>
#include "hook_client.h"
#include "memory_trace.h"

using namespace testing::ext;

namespace {
static std::atomic<int> g_enableCnt{0};
static std::atomic<int> g_disableCnt{0};
static std::atomic<bool> g_resolverReturnNull{false};

static void FakeEnable()
{
    g_enableCnt++;
}

static void FakeDisable()
{
    g_disableCnt++;
}

static ArkTsHookFn FakeResolver(const char* name)
{
    if (g_resolverReturnNull.load()) {
        return nullptr;
    }
    if (strcmp(name, "EnableArkTsHook") == 0) {
        return FakeEnable;
    }
    if (strcmp(name, "DisableArkTsHook") == 0) {
        return FakeDisable;
    }
    return nullptr;
}

void ResetArkTsState()
{
    SetArkTsHookResolver(FakeResolver);
    g_resolverReturnNull = false;
    DisableArkTsHookIfNeeded();
    g_enableCnt = 0;
    g_disableCnt = 0;
}
} // namespace

class ArkTsHookTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override { ResetArkTsState(); }
    void TearDown() override { ResetArkTsHookResolver(); }
};

/*
 * @tc.name: EnableCalledWhenMaskSet
 * @tc.desc: EnableArkTsHookIfNeeded calls the hook when mask set and not yet enabled.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTsHookTest, EnableCalledWhenMaskSet, TestSize.Level0)
{
    EnableArkTsHookIfNeeded(RES_ARKTS_HEAP_MASK);
    EXPECT_EQ(g_enableCnt.load(), 1);
    EXPECT_EQ(g_disableCnt.load(), 0);
}

/*
 * @tc.name: RepeatEnableIsIdempotent
 * @tc.desc: repeated EnableArkTsHookIfNeeded calls the hook only once (flag guard).
 * @tc.type: FUNC
 */
HWTEST_F(ArkTsHookTest, RepeatEnableIsIdempotent, TestSize.Level0)
{
    EnableArkTsHookIfNeeded(RES_ARKTS_HEAP_MASK);
    EnableArkTsHookIfNeeded(RES_ARKTS_HEAP_MASK);
    EXPECT_EQ(g_enableCnt.load(), 1);
    EXPECT_EQ(g_disableCnt.load(), 0);
}

/*
 * @tc.name: NoMaskDoesNotEnable
 * @tc.desc: EnableArkTsHookIfNeeded with mask clear does not call the hook.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTsHookTest, NoMaskDoesNotEnable, TestSize.Level0)
{
    EnableArkTsHookIfNeeded(0);
    EXPECT_EQ(g_enableCnt.load(), 0);
    EXPECT_EQ(g_disableCnt.load(), 0);
}

/*
 * @tc.name: DisableCalledAfterEnable
 * @tc.desc: DisableArkTsHookIfNeeded calls the hook after a prior enable.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTsHookTest, DisableCalledAfterEnable, TestSize.Level0)
{
    EnableArkTsHookIfNeeded(RES_ARKTS_HEAP_MASK);
    DisableArkTsHookIfNeeded();
    EXPECT_EQ(g_enableCnt.load(), 1);
    EXPECT_EQ(g_disableCnt.load(), 1);
}

/*
 * @tc.name: DisableWithoutEnableIsNoop
 * @tc.desc: DisableArkTsHookIfNeeded does not call the hook when not enabled.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTsHookTest, DisableWithoutEnableIsNoop, TestSize.Level0)
{
    DisableArkTsHookIfNeeded();
    EXPECT_EQ(g_enableCnt.load(), 0);
    EXPECT_EQ(g_disableCnt.load(), 0);
}

/*
 * @tc.name: ReenableAfterDisableRoundTrip
 * @tc.desc: enable/disable/enable round trip allows re-enabling after disable.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTsHookTest, ReenableAfterDisableRoundTrip, TestSize.Level0)
{
    EnableArkTsHookIfNeeded(RES_ARKTS_HEAP_MASK);
    DisableArkTsHookIfNeeded();
    EnableArkTsHookIfNeeded(RES_ARKTS_HEAP_MASK);
    EXPECT_EQ(g_enableCnt.load(), 2);
    EXPECT_EQ(g_disableCnt.load(), 1);
}

/*
 * @tc.name: ResolverNullSkipsCall
 * @tc.desc: a null resolver makes CallArkTsHook skip (no call, no crash).
 * @tc.type: FUNC
 */
HWTEST_F(ArkTsHookTest, ResolverNullSkipsCall, TestSize.Level0)
{
    SetArkTsHookResolver(nullptr);
    EnableArkTsHookIfNeeded(RES_ARKTS_HEAP_MASK);
    EXPECT_EQ(g_enableCnt.load(), 0);
    EXPECT_EQ(g_disableCnt.load(), 0);
}

/*
 * @tc.name: SymbolNotFoundSkipsCall
 * @tc.desc: resolver returning nullptr (symbol not found) skips the call.
 * @tc.type: FUNC
 */
HWTEST_F(ArkTsHookTest, SymbolNotFoundSkipsCall, TestSize.Level0)
{
    g_resolverReturnNull = true;
    EnableArkTsHookIfNeeded(RES_ARKTS_HEAP_MASK);
    EXPECT_EQ(g_enableCnt.load(), 0);
    EXPECT_EQ(g_disableCnt.load(), 0);
}
