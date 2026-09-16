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
#include <memory>
#include <string>
#include <sys/time.h>
#include <array>
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

static HookRecordPtr CreateHookRecord(uint16_t type)
{
    RawStackPtr rawData = std::make_shared<RawStack>();
    rawData->baseStackData.resize(sizeof(BaseStackRawData));
    auto* baseData = reinterpret_cast<BaseStackRawData*>(rawData->baseStackData.data());
    *baseData = {};
    baseData->type = type;
    baseData->pid = 10;
    baseData->tid = 11;
    rawData->stackContext = baseData;
    return std::make_shared<HookRecord>(rawData);
}

static NativeHookConfig CreateStackBuilderConfig(bool fpUnwind)
{
    NativeHookConfig hookConfig;
    hookConfig.set_fp_unwind(fpUnwind);
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_max_stack_depth(16);
    hookConfig.set_max_js_stack_depth(8);
    return hookConfig;
}

static RawStackPtr CreateRawStackWithContext(uint16_t type)
{
    RawStackPtr rawData = std::make_shared<RawStack>();
    rawData->baseStackData.resize(sizeof(BaseStackRawData));
    auto* baseData = reinterpret_cast<BaseStackRawData*>(rawData->baseStackData.data());
    *baseData = {};
    baseData->type = type;
    baseData->pid = 10;
    baseData->tid = 11;
    rawData->stackContext = baseData;
    return rawData;
}

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
    runtime->arktsMapTree_[0x1000] = {map, 1};
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
 * @tc.name: FpStackBuilderPrepareUnwindHandlesRuntime
 * @tc.desc: test FpStackBuilder updates thread only when runtime is available.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, FpStackBuilderPrepareUnwindHandlesRuntime, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    HookRecordPtr hookRecord = std::make_shared<HookRecord>(CreateRawStackWithContext(MALLOC_MSG));

    FpStackBuilder builderWithRuntime(&hookConfig, runtime, g_mtx);
    builderWithRuntime.PrepareUnwind(hookRecord);
    EXPECT_NE(runtime->GetThreads().find(11), runtime->GetThreads().end());

    FpStackBuilder builderWithoutRuntime(&hookConfig, nullptr, g_mtx);
    builderWithoutRuntime.PrepareUnwind(hookRecord);
    EXPECT_EQ(runtime->GetThreads().size(), 1u);
}

/*
 * @tc.name: FpStackBuilderFillIpsHandlesRawIpBoundaries
 * @tc.desc: test FpStackBuilder::FillIps stops at zero and accepts empty raw depth.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, FpStackBuilderFillIpsHandlesRawIpBoundaries, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    FpStackBuilder builder(&hookConfig, runtime, g_mtx);
    std::vector<CallFrame> frames;
    std::array<uint64_t, 3> ips = {0x1111, 0, 0x3333};
    RawStackPtr rawData = CreateRawStackWithContext(MALLOC_MSG);
    rawData->fpDepth = ips.size();
    rawData->data = reinterpret_cast<uint8_t*>(ips.data());

    builder.FillIps(frames, std::make_shared<HookRecord>(rawData));
    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].ip_, StripPac(0x1111, 0));

    rawData->fpDepth = 0;
    frames.clear();
    builder.FillIps(frames, std::make_shared<HookRecord>(rawData));
    EXPECT_TRUE(frames.empty());
}

/*
 * @tc.name: FpStackBuilderSkipsJsSymbolsForStatReleaseRecords
 * @tc.desc: test FpStackBuilder skips JS symbols for release records in statistics mode.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, FpStackBuilderSkipsJsSymbolsForStatReleaseRecords, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    hookConfig.set_statistics_interval(1);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    FpStackBuilder builder(&hookConfig, runtime, g_mtx);
    std::vector<CallFrame> frames;
    RawStackPtr rawData = CreateRawStackWithContext(FREE_MSG);
    rawData->stackContext->jsChainId = 1;
    rawData->jsStackData = "funcA|entry/src/main/ets/pages/Index.ets";

    builder.FillJsSymbols(frames, std::make_shared<HookRecord>(rawData));
    EXPECT_TRUE(frames.empty());

    rawData->stackContext->type = MALLOC_MSG;
    builder.FillJsSymbols(frames, std::make_shared<HookRecord>(rawData));
    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].symbolName_, "funcA");
}

HWTEST_F(StackBuilderTest, FpStackBuilderFillAsyncJsSymbolsHandlesEarlyReturns, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    hookConfig.set_statistics_interval(1);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    FpStackBuilder builder(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = std::make_shared<HookRecord>(CreateRawStackWithContext(FREE_MSG));
    std::vector<CallFrame> shortFrames = {CallFrame(0x1000), CallFrame(0x1001)};

    builder.FillAsyncJsSymbols(shortFrames, hookRecord);
    EXPECT_EQ(shortFrames.size(), FILTER_STACK_DEPTH);

    std::vector<CallFrame> releaseFrames = {CallFrame(0x1000), CallFrame(0x1001), CallFrame(0x1002)};
    builder.FillAsyncJsSymbols(releaseFrames, hookRecord);
    EXPECT_FALSE(releaseFrames[0].isJsFrame_);
    EXPECT_EQ(releaseFrames[2].callFrameId_, 0u);
}

HWTEST_F(StackBuilderTest, FpStackBuilderFillAsyncJsSymbolsSkipsResolvedFrames, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    hookConfig.set_fp_unwind(true);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    FpStackBuilder builder(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = std::make_shared<HookRecord>(CreateRawStackWithContext(MALLOC_MSG));
    std::vector<CallFrame> frames = {CallFrame(0x2000), CallFrame(0x2001), CallFrame(0x2002)};
    frames[0].callFrameId_ = 7;
    frames[1].symbolName_ = "cached";
    frames[1].filePath_ = "cached.hap";

    builder.FillAsyncJsSymbols(frames, hookRecord);

    EXPECT_EQ(frames[0].callFrameId_, 7u);
    EXPECT_EQ(frames[1].symbolName_, "cached");
    EXPECT_EQ(frames[2].callFrameId_, 0u);
}

HWTEST_F(StackBuilderTest, FpStackBuilderFillAsyncJsSymbolsUsesFilteredStartIndex, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(false);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    FpStackBuilder builder(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = std::make_shared<HookRecord>(CreateRawStackWithContext(MALLOC_MSG));
    std::vector<CallFrame> frames = {CallFrame(0x3000), CallFrame(0x3001), CallFrame(0x3002)};

    builder.FillAsyncJsSymbols(frames, hookRecord);

    EXPECT_FALSE(frames[0].isJsFrame_);
    EXPECT_FALSE(frames[1].isJsFrame_);
    EXPECT_EQ(frames[2].callFrameId_, 0u);
}

/*
 * @tc.name: FpStackBuilderFillStatsVirtualFramesUsesSizeMask
 * @tc.desc: test StackBuilder adds malloc size virtual frame for statistics.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, FpStackBuilderFillStatsVirtualFramesUsesSizeMask, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    FpStackBuilder builder(&hookConfig, runtime, g_mtx);
    RawStackPtr rawData = CreateRawStackWithContext(MALLOC_MSG);
    rawData->stackContext->mallocSize = 0x1234;
    std::vector<CallFrame> frames;

    builder.FillStatsVirtualFrames(frames, std::make_shared<HookRecord>(rawData));

    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].ip_, (0x1234 | SIZE_MASK));
}

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
 * @tc.name: DwarfStackBuilderValidatesStackSize
 * @tc.desc: test DwarfStackBuilder validation depends on stack size.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DwarfStackBuilderValidatesStackSize, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(false);
    hookConfig.set_blocked(true);
    hookConfig.set_max_stack_depth(MAX_CALL_FRAME_UNWIND_SIZE + 1);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    DwarfStackBuilder builder(&hookConfig, runtime, g_mtx);
    RawStackPtr rawData = CreateRawStackWithContext(MALLOC_MSG);
    HookRecordPtr hookRecord = std::make_shared<HookRecord>(rawData);

    EXPECT_FALSE(builder.IsRecordInfoValid(hookRecord));
    rawData->stackSize = 8;
    EXPECT_TRUE(builder.IsRecordInfoValid(hookRecord));
}

/*
 * @tc.name: DwarfStackBuilderSkipsShortFrames
 * @tc.desc: test DwarfStackBuilder JS and native symbol filling skip short frames.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DwarfStackBuilderSkipsShortFrames, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(false);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    DwarfStackBuilder builder(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = std::make_shared<HookRecord>(CreateRawStackWithContext(MALLOC_MSG));

    std::vector<CallFrame> frames = {CallFrame(0x1000), CallFrame(0x1001)};
    builder.FillJsSymbols(frames, hookRecord);
    builder.FillNativeSymbols(frames, hookRecord);
    EXPECT_EQ(frames.size(), FILTER_STACK_DEPTH);
}

/*
 * @tc.name: DirectorReturnsEmptyWhenBuilderOrConfigInvalid
 * @tc.desc: test BuildStackDirector returns empty frames for invalid dependencies.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DirectorReturnsEmptyWhenBuilderOrConfigInvalid, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    BuildStackDirector director(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = CreateHookRecord(MALLOC_MSG);

    director.SetBuilder(nullptr);
    auto& framesWithoutBuilder = director.ConstructCallFrames(hookRecord, {0x1000}, {});
    EXPECT_TRUE(framesWithoutBuilder.empty());

    BuildStackDirector directorWithoutConfig(&hookConfig, runtime, g_mtx);
    directorWithoutConfig.hookConfig_ = nullptr;
    auto& framesWithoutConfig = directorWithoutConfig.ConstructCallFrames(hookRecord, {0x1000}, {});
    EXPECT_TRUE(framesWithoutConfig.empty());
}

/*
 * @tc.name: DirectorConstructsFramesFromExplicitPcs
 * @tc.desc: test BuildStackDirector builds call frames from sync and async pcs.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DirectorConstructsFramesFromExplicitPcs, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    BuildStackDirector director(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = CreateHookRecord(MALLOC_MSG);

    auto& frames = director.ConstructCallFrames(hookRecord, {0x1000, 0}, {0x2000, 0x3000});

    ASSERT_EQ(frames.size(), 3u);
    EXPECT_EQ(frames[0].ip_, StripPac(0x1000, 0));
    EXPECT_EQ(frames[1].ip_, StripPac(0x2000, 0));
    EXPECT_EQ(frames[2].ip_, StripPac(0x3000, 0));
}

/*
 * @tc.name: DirectorConstructsNestedFramesWithFullAsyncStack
 * @tc.desc: test BuildStackDirector constructs frames with full async stack depth.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DirectorConstructsNestedFramesWithFullAsyncStack, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    BuildStackDirector director(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = CreateHookRecord(MALLOC_MSG);
    // Sync stack: 1 PC
    // Async stack 1: 2 PCs
    // Async stack 2: 1 PC
    std::vector<std::vector<uintptr_t>> nestedAsyncPcs = {{0x2000, 0x2001}, {0x3000, 0}};

    auto& frames = director.ConstructNestedCallFrames(hookRecord, {0x1000, 0}, nestedAsyncPcs);

    // Expect 4 frames: 1 sync + 2 from async1 + 1 from async2
    ASSERT_EQ(frames.size(), 4u);
    EXPECT_EQ(frames[0].ip_, StripPac(0x1000, 0));
    EXPECT_EQ(frames[1].ip_, StripPac(0x2000, 0));
    EXPECT_EQ(frames[2].ip_, StripPac(0x2001, 0));
    EXPECT_EQ(frames[3].ip_, StripPac(0x3000, 0));
}

/*
 * @tc.name: DirectorSkipsPrSetVmaRecord
 * @tc.desc: test BuildStackDirector skips PR_SET_VMA records.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DirectorSkipsPrSetVmaRecord, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(true);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    BuildStackDirector director(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = CreateHookRecord(PR_SET_VMA_MSG);

    auto& frames = director.ConstructCallFrames(hookRecord, {0x1000}, {});

    EXPECT_TRUE(frames.empty());
    EXPECT_FALSE(director.IsRecordUnwindable(hookRecord));
}

/*
 * @tc.name: DirectorReplacesInvalidDwarfStack
 * @tc.desc: test BuildStackDirector emits dwarf error frame for invalid dwarf stack.
 * @tc.type: FUNC
 */
HWTEST_F(StackBuilderTest, DirectorReplacesInvalidDwarfStack, TestSize.Level0)
{
    NativeHookConfig hookConfig = CreateStackBuilderConfig(false);
    auto runtime = std::make_shared<VirtualRuntime>(hookConfig);
    BuildStackDirector director(&hookConfig, runtime, g_mtx);
    HookRecordPtr hookRecord = CreateHookRecord(MALLOC_MSG);

    auto& frames = director.ConstructCallFrames(hookRecord);

    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].callFrameId_, DWARF_ERROR_ID);
    EXPECT_EQ(frames[0].symbolName_, "UnwindErrorDwarf");
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
