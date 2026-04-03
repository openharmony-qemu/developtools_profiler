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
class NestedAsyncStackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NestedAsyncStackTest::SetUpTestCase(void) {}
void NestedAsyncStackTest::TearDownTestCase(void) {}
void NestedAsyncStackTest::SetUp() {}
void NestedAsyncStackTest::TearDown() {}

/*
 * @tc.name: FpStackBuilderFillNestedIpsTest
 * @tc.desc: test FpStackBuilder::FillNestedIps with sync and nested async pcs
 * @tc.type: FUNC
 */
#ifdef __aarch64__
HWTEST_F(NestedAsyncStackTest, FpStackBuilderFillNestedIpsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
        std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    
    std::vector<CallFrame> frames;
    std::vector<uintptr_t> syncPcs = {0x1111, 0x2222, 0x0};
    std::vector<std::vector<uintptr_t>> nestedAsyncPcs = {
        {0x3333, 0x4444, 0x0},
        {0x5555, 0x0}
    };
    
    builder->FillNestedIps(frames, syncPcs, nestedAsyncPcs);
    
    EXPECT_EQ(frames.size(), 4u);
    EXPECT_EQ(frames[0].ip_, StripPac(0x1111, 0));
    EXPECT_EQ(frames[1].ip_, StripPac(0x2222, 0));
    EXPECT_EQ(frames[2].ip_, StripPac(0x3333, 0));
    EXPECT_EQ(frames[3].ip_, StripPac(0x4444, 0));
}

/*
 * @tc.name: FpStackBuilderFillNestedIpsEmptyTest
 * @tc.desc: test FpStackBuilder::FillNestedIps with empty vectors
 * @tc.type: FUNC
 */
HWTEST_F(NestedAsyncStackTest, FpStackBuilderFillNestedIpsEmptyTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
        std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    
    std::vector<CallFrame> frames;
    std::vector<uintptr_t> syncPcs;
    std::vector<std::vector<uintptr_t>> nestedAsyncPcs;
    
    builder->FillNestedIps(frames, syncPcs, nestedAsyncPcs);
    
    EXPECT_EQ(frames.size(), 0u);
}

/*
 * @tc.name: FpStackBuilderFillNestedIpsOnlySyncTest
 * @tc.desc: test FpStackBuilder::FillNestedIps with only sync pcs
 * @tc.type: FUNC
 */
HWTEST_F(NestedAsyncStackTest, FpStackBuilderFillNestedIpsOnlySyncTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
        std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    
    std::vector<CallFrame> frames;
    std::vector<uintptr_t> syncPcs = {0x1111, 0x2222};
    std::vector<std::vector<uintptr_t>> nestedAsyncPcs;
    
    builder->FillNestedIps(frames, syncPcs, nestedAsyncPcs);
    
    EXPECT_EQ(frames.size(), 2u);
    EXPECT_EQ(frames[0].ip_, StripPac(0x1111, 0));
    EXPECT_EQ(frames[1].ip_, StripPac(0x2222, 0));
}

/*
 * @tc.name: DirectorConstructNestedCallFramesTest
 * @tc.desc: test BuildStackDirector::ConstructNestedCallFrames normal case

 * @tc.type: FUNC
 */
HWTEST_F(NestedAsyncStackTest, DirectorConstructNestedCallFramesTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(true);
    hookConfig.set_fp_unwind(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
        std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    auto director = std::make_shared<BuildStackDirector>(&hookConfig, runtime, g_mtx);
    director->SetBuilder(builder);
    
    RawStackPtr rawData = std::make_shared<RawStack>();
    BaseStackRawData baseData;
    baseData.pid = 10;
    baseData.tid = 11;
    rawData->stackContext = &baseData;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);
    
    std::vector<uintptr_t> syncPcs = {0x1111, 0x2222};
    std::vector<std::vector<uintptr_t>> nestedAsyncPcs = {
        {0x3333, 0x4444},
        {0x5555}
    };
    
    auto& frames = director->ConstructNestedCallFrames(hookRecord, syncPcs, nestedAsyncPcs);
    
    EXPECT_EQ(frames.size(), 5u);
}

/*
 * @tc.name: DirectorConstructNestedCallFramesEmptyTest
 * @tc.desc: test BuildStackDirector::ConstructNestedCallFrames with empty vectors
 * @tc.type: FUNC
 */
HWTEST_F(NestedAsyncStackTest, DirectorConstructNestedCallFramesEmptyTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(true);
    hookConfig.set_fp_unwind(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
        std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    auto director = std::make_shared<BuildStackDirector>(&hookConfig, runtime, g_mtx);
    director->SetBuilder(builder);
    
    RawStackPtr rawData = std::make_shared<RawStack>();
    BaseStackRawData baseData;
    baseData.pid = 10;
    baseData.tid = 11;
    rawData->stackContext = &baseData;
    std::shared_ptr<HookRecord> hookRecord = std::make_shared<HookRecord>(rawData);
    
    std::vector<uintptr_t> syncPcs;
    std::vector<std::vector<uintptr_t>> nestedAsyncPcs;
    
    auto& frames = director->ConstructNestedCallFrames(hookRecord, syncPcs, nestedAsyncPcs);
    
    EXPECT_EQ(frames.size(), 0u);
}

/*
 * @tc.name: DirectorConstructNestedCallFramesWithJsTest
 * @tc.desc: test BuildStackDirector::ConstructNestedCallFrames with JS symbols
 * @tc.type: FUNC
 */
HWTEST_F(NestedAsyncStackTest, DirectorConstructNestedCallFramesWithJsTest, TestSize.Level0)
{
    NativeHookConfig hookConfig;
    hookConfig.set_offline_symbolization(true);
    hookConfig.set_js_stack_report(true);
    hookConfig.set_fp_unwind(true);
    std::shared_ptr<OHOS::Developtools::NativeDaemon::VirtualRuntime> runtime =
        std::make_shared<OHOS::Developtools::NativeDaemon::VirtualRuntime>(hookConfig);
    auto builder = std::make_shared<FpStackBuilder>(&hookConfig, runtime, g_mtx);
    auto director = std::make_shared<BuildStackDirector>(&hookConfig, runtime, g_mtx);
    director->SetBuilder(builder);
    
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
    
    std::vector<uintptr_t> syncPcs = {0x1111, 0x2222};
    std::vector<std::vector<uintptr_t>> nestedAsyncPcs = {
        {0x3333, 0x4444}
    };
    
    for (uint64_t ip = 0x1111; ip <= 0x4444; ip += 0x1111) {
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
    
    auto& frames = director->ConstructNestedCallFrames(hookRecord, syncPcs, nestedAsyncPcs);
    
    EXPECT_TRUE(frames.size() > 0);
}
#endif
}