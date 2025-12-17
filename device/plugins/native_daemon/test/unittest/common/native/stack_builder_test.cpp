/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "hook_record_factory_test.h"
#include "stack_builder_test.h"
#include "native_hook_config.pb.h"
#include <string>
#include <sys/time.h>
#include <vector>

using namespace testing::ext;
using namespace std;

namespace OHOS::Developtools::NativeDaemon {
static std::mutex g_mtx;
class StackBuilderTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void StackBuilderTest::SetUpTestCase(void) {}
void StackBuilderTest::TearDownTestCase(void) {}
void StackBuilderTest::SetUp() {}
void StackBuilderTest::TearDown() {}

/*
 * @tc.name: FpStackBuilderFillIpTest
 * @tc.desc: test FpStackBuilder::FillIps normal case.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(StackBuilderTest, FpStackBuilderFillIpTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
    std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    std::vector<CallFrame> frames;
    RawStackPtr rawData = std::make_shared<RawStack>();
    rawData->fpDepth = 1;
    uint8_t ip = 16;
    rawData->data = &ip;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);
    builder->FillIps(frames, hookRecord);
    EXPECT_EQ(frames.size(), 1);
}

/*
 * @tc.name: FpStackBuilderFillJsSymbolsTest
 * @tc.desc: test FpStackBuilder::FillJsSymbols normal case.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, FpStackBuilderFillJsSymbolsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
    std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    std::vector<CallFrame> frames;
    RawStackPtr rawData = std::make_shared<RawStack>();
    rawData->fpDepth = 1;
    uint8_t ip = 16;
    rawData->data = &ip;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);
    builder->FillIps(frames, hookRecord);
    rawData->jsStackData = "funcA|";
    BaseStackRawData baseData;
    baseData.jsChainId = 1;
    rawData->stackContext = &baseData;
    builder->FillJsSymbols(frames, hookRecord);
    EXPECT_EQ(frames.size(), 2);
}

/*
 * @tc.name: FpStackBuilderFillWrongJsSymbolsTest
 * @tc.desc: test FpStackBuilder::FillJsSymbols error case.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, FpStackBuilderFillWrongJsSymbolsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
    std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    std::vector<CallFrame> frames;
    RawStackPtr rawData = std::make_shared<RawStack>();
    rawData->fpDepth = 1;
    uint8_t ip = 16;
    rawData->data = &ip;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);
    builder->FillIps(frames, hookRecord);
    rawData->jsStackData = "funcA";
    BaseStackRawData baseData;
    baseData.jsChainId = 1;
    rawData->stackContext = &baseData;
    builder->FillJsSymbols(frames, hookRecord);
    EXPECT_EQ(frames.size(), 1);
}

/*
 * @tc.name: FpStackBuilderFillAsyncJsSymbolsTest
 * @tc.desc: test FpStackBuilder::FillAsyncJsSymbols normal case.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, FpStackBuilderFillAsyncJsSymbolsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(true);
    hookConfig.set_fp_unwind(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
        std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);

    std::vector<CallFrame> frames;
    frames.emplace_back(0x1000);
    frames.emplace_back(0x1001);
    frames.emplace_back(0x1002);

    RawStackPtr rawData = std::make_shared<RawStack>();
    BaseStackRawData baseData;
    baseData.pid = 10;
    baseData.tid = 11;
    rawData->stackContext = &baseData;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);

    auto map = std::make_shared<DfxMap>();
    map->name = "test.hap";
    map->begin = 0x1000;
    map->end = 0x2000;
    runtime->arktsMapTree_[0x1000] = map;
    runtime->jsUrlMap_["test.hap"] = 1;

    for (uint64_t ip = 0x1000; ip <= 0x1002; ++ip) {
        DfxSymbol sym;
        sym.symbolName_ = "async_func";
        sym.funcVaddr_ = ip - map->begin;
        sym.symbolId_ = 1;
        sym.symbolNameId_ = 1;
        sym.filePathId_ = 1;
        sym.module_ = map->name;
        VirtualRuntime::SymbolCacheKey key(ip, 1);
        runtime->userSymbolCache_[key] = sym;
    }

    builder->FillAsyncJsSymbols(frames, hookRecord);

    EXPECT_TRUE(frames.size() > 0);
    for (size_t i = 0; i < frames.size(); ++i) {
        EXPECT_TRUE(frames[i].isJsFrame_);
        EXPECT_EQ(frames[i].symbolName_, "async_func");
    }
}

/*
 * @tc.name: FpStackBuilderFillIpsSyncAsyncTest
 * @tc.desc: test FpStackBuilder::FillIps with sync and async pcs including zero terminator.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, FpStackBuilderFillIpsSyncAsyncTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
        std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);

    std::vector<CallFrame> frames;
    std::vector<uint64_t> syncPcs = {0x1111, 0x0, 0xdead};
    std::vector<uint64_t> asyncPcs = {0x2222, 0x0};

    builder->FillIps(frames, syncPcs, asyncPcs);

    EXPECT_EQ(frames.size(), 2u);
    EXPECT_EQ(frames[0].ip_, StripPac(0x1111, 0));
    EXPECT_EQ(frames[1].ip_, StripPac(0x2222, 0));
}
#endif

/*
 * @tc.name: DwarfStackBuilderReplaceErrStackTest
 * @tc.desc: test DwarfStackBuilder::ReplaceErrStack
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DwarfStackBuilderReplaceErrStackTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
    std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<DwarfStackBuilder>(&hookConfig, runtime, g_mtx);
    std::vector<CallFrame> frames;
    RawStackPtr rawData = std::make_shared<RawStack>();
    rawData->fpDepth = 1;
    uint8_t ip = 16;
    rawData->data = &ip;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);
    ASSERT_FALSE(builder->IsRecordInfoValid(hookRecord));
    builder->ReplaceErrStack(frames);
    EXPECT_EQ(frames.size(), 1);
}

/*
 * @tc.name: DwarfStackBuilderReplaceErrStackTest
 * @tc.desc: test BuildStackDirector::ConstructCallFrames normal case.
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(StackBuilderTest, DirectorTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
    std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    auto director = std::make_shared<BuildStackDirector>(&hookConfig, runtime, g_mtx);
    director->SetBuilder(builder);
    RawStackPtr rawData = std::make_shared<RawStack>();
    rawData->fpDepth = 1;
    uint8_t ip = 16;
    rawData->data = &ip;
    rawData->jsStackData = "funcA";
    BaseStackRawData baseData;
    baseData.jsChainId = 1;
    rawData->stackContext = &baseData;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);
    ASSERT_TRUE(director->IsRecordUnwindable(hookRecord));
    auto& frames = director->ConstructCallFrames(hookRecord);
    EXPECT_EQ(frames.size(), 1);
}
#endif

/*
 * @tc.name: DirectorTestFalse
 * @tc.desc: test BuildStackDirector::IsRecordUnwindable false case.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DirectorTestFalse, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
    std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto director = std::make_shared<BuildStackDirector>(&hookConfig, runtime, g_mtx);
    RawStackPtr rawData = std::make_shared<RawStack>();
    BaseStackRawData baseData;
    baseData.type = PR_SET_VMA_MSG;
    rawData->stackContext = &baseData;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);
    ASSERT_FALSE(director->IsRecordUnwindable(hookRecord));
}
}