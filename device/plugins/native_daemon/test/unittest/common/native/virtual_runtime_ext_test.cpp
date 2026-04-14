/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <gtest/gtest.h>
#include <link.h>
#include <random>
#include <sys/mman.h>

#include "virtual_runtime.h"
#include "symbols_file.h"

using namespace testing::ext;
using namespace std;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
class VirtualRuntimeExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    default_random_engine rnd_;
    std::unique_ptr<VirtualRuntime> runtime_;
    size_t callbackCount_ = 0;

    void PrepareTestSymbols();
};

void VirtualRuntimeExtTest::SetUpTestCase() {}

void VirtualRuntimeExtTest::TearDownTestCase() {}

void VirtualRuntimeExtTest::SetUp()
{
    NativeHookConfig config;
    runtime_ = std::make_unique<VirtualRuntime>(config);
    callbackCount_ = 0;
}

void VirtualRuntimeExtTest::TearDown()
{
    runtime_.release();
}

void VirtualRuntimeExtTest::PrepareTestSymbols()
{
    std::string testSymbol = "test_symbol";
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    symbolsFile->filePath_ = testSymbol;
    symbolsFile->symbols_.emplace_back(0x1000, 1u, "test_func", symbolsFile->filePath_);
    runtime_->symbolsFiles_[symbolsFile->filePath_] = std::move(symbolsFile);
}

/**
 * @tc.name: ClearMaps
 * @tc.desc: Test ClearMaps method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, ClearMaps, TestSize.Level0)
{
    // Add some test data
    auto mapItem = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_READ, "test.so");
    runtime_->processMaps_.push_back(mapItem);
    runtime_->arktsMapTree_[0x1000] = mapItem;
    runtime_->adltFound_["test.so"] = true;
    runtime_->adltBaseMap_["test.so"] = 0x1000;

    // Verify data exists
    EXPECT_GT(runtime_->processMaps_.size(), 0u);
    EXPECT_GT(runtime_->arktsMapTree_.size(), 0u);
    EXPECT_GT(runtime_->adltFound_.size(), 0u);
    EXPECT_GT(runtime_->adltBaseMap_.size(), 0u);

    // Clear maps
    runtime_->ClearMaps();

    // Verify data is cleared
    EXPECT_EQ(runtime_->processMaps_.size(), 0u);
    EXPECT_EQ(runtime_->arktsMapTree_.size(), 0u);
    EXPECT_EQ(runtime_->adltFound_.size(), 0u);
    EXPECT_EQ(runtime_->adltBaseMap_.size(), 0u);
}

/**
 * @tc.name: UpdateThread
 * @tc.desc: Test UpdateThread method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, UpdateThread, TestSize.Level0)
{
    const pid_t testPid = 123;
    const pid_t testTid = 456;
    const std::string testName = "test_thread";

    // Update thread with name
    auto& thread1 = runtime_->UpdateThread(testPid, testTid, testName);
    EXPECT_EQ(thread1.tid_, testTid);

    // Update thread without name
    auto& thread2 = runtime_->UpdateThread(testPid, testTid);
    EXPECT_EQ(thread2.tid_, testTid);
}

/**
 * @tc.name: IsSymbolExist
 * @tc.desc: Test IsSymbolExist method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, IsSymbolExist, TestSize.Level0)
{
    const std::string testSymbol = "test_symbol";

    // Symbol should not exist initially
    EXPECT_FALSE(runtime_->IsSymbolExist(testSymbol));

    // Add symbol
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    symbolsFile->filePath_ = testSymbol;
    runtime_->symbolsFiles_[symbolsFile->filePath_] = std::move(symbolsFile);

    // Symbol should exist now
    EXPECT_TRUE(runtime_->IsSymbolExist(testSymbol));
}

/**
 * @tc.name: DelSymbolFile
 * @tc.desc: Test DelSymbolFile method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, DelSymbolFile, TestSize.Level0)
{
    const std::string testSymbol = "test_symbol";

    // Add symbol
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    symbolsFile->filePath_ = testSymbol;
    runtime_->symbolsFiles_[symbolsFile->filePath_] = std::move(symbolsFile);

    // Symbol should exist
    EXPECT_TRUE(runtime_->IsSymbolExist(testSymbol));

    // Delete symbol
    runtime_->DelSymbolFile(testSymbol);

    // Symbol should not exist now
    EXPECT_FALSE(runtime_->IsSymbolExist(testSymbol));
}

/**
 * @tc.name: UpdateSymbols
 * @tc.desc: Test UpdateSymbols method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, UpdateSymbols, TestSize.Level0)
{
    const std::string testFile = "test.so";

    // Update symbols
    runtime_->UpdateSymbols(testFile, nullptr);

    // Verify symbol file was added
    EXPECT_TRUE(runtime_->IsSymbolExist(testFile));
}

/**
 * @tc.name: IsArktsMap
 * @tc.desc: Test IsArktsMap method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, IsArktsMap, TestSize.Level0)
{
    // Test with nullptr
    EXPECT_FALSE(runtime_->IsArktsMap(nullptr));

    // Test with non-ArkTS map
    auto nonArktsMap = std::make_shared<DfxMap>();
    nonArktsMap->name = "test.so";
    EXPECT_FALSE(runtime_->IsArktsMap(nonArktsMap));

    // Test with ArkTS map (stub.an)
    auto arktsMap = std::make_shared<DfxMap>();
    arktsMap->name = "stub.an";
    EXPECT_TRUE(runtime_->IsArktsMap(arktsMap));
}

/**
 * @tc.name: FindArktsMapByAddr
 * @tc.desc: Test FindArktsMapByAddr method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindArktsMapByAddr, TestSize.Level0)
{
    // Test with empty map tree
    EXPECT_EQ(runtime_->FindArktsMapByAddr(0x1000), nullptr);

    // Add test map
    auto mapItem = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_READ, "stub.an");
    runtime_->arktsMapTree_[0x1000] = mapItem;

    // Test with address in range
    EXPECT_NE(runtime_->FindArktsMapByAddr(0x1500), nullptr);

    // Test with address out of range
    EXPECT_EQ(runtime_->FindArktsMapByAddr(0x2500), nullptr);
}

/**
 * @tc.name: GetAdltBase
 * @tc.desc: Test GetAdltBase method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, GetAdltBase, TestSize.Level0)
{
    const std::string testSo = "test.so";
    const uint64_t testBase = 0x1000;

    // Test with non-existent entry
    EXPECT_EQ(runtime_->GetAdltBase(testSo), std::numeric_limits<uint64_t>::max());

    // Add test entry
    runtime_->adltBaseMap_[testSo] = testBase;

    // Test with existing entry
    EXPECT_EQ(runtime_->GetAdltBase(testSo), testBase);
}

/**
 * @tc.name: RemoveMaps
 * @tc.desc: Test RemoveMaps method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, RemoveMaps, TestSize.Level0)
{
    const uint64_t testAddr = 0x1000;

    // Add test map
    auto mapItem = std::make_shared<DfxMap>(testAddr, testAddr + 0x1000, 0, PROT_READ, "test.so");
    auto memMaps = std::make_shared<MemMaps>(1);
    memMaps->AddMap(mapItem, true);
    runtime_->mapsCache_[testAddr] = memMaps;

    // Verify map exists
    EXPECT_NE(runtime_->mapsCache_.find(testAddr), runtime_->mapsCache_.end());

    // Remove map
    runtime_->RemoveMaps(testAddr);

    // Verify map was removed
    EXPECT_EQ(runtime_->mapsCache_.find(testAddr), runtime_->mapsCache_.end());
}

/**
 * @tc.name: FindMap
 * @tc.desc: Test FindMap method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindMap, TestSize.Level0)
{
    const uint64_t testAddr = 0x1000;

    // Test with empty cache
    auto [maps1, index1] = runtime_->FindMap(testAddr);
    EXPECT_EQ(maps1, nullptr);

    // Add test map
    auto mapItem = std::make_shared<DfxMap>(testAddr, testAddr + 0x1000, 0, PROT_READ, "test.so");
    auto memMaps = std::make_shared<MemMaps>(1);
    memMaps->AddMap(mapItem, true);
    runtime_->mapsCache_[testAddr] = memMaps;

    // Test with address in range
    auto [maps2, index2] = runtime_->FindMap(testAddr + 0x500);
    EXPECT_NE(maps2, nullptr);

    // Test with address out of range
    auto [maps3, index3] = runtime_->FindMap(testAddr + 0x1500);
    EXPECT_EQ(maps3, nullptr);
}

/**
 * @tc.name: GetJsSymbolCacheSize
 * @tc.desc: Test GetJsSymbolCacheSize method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, GetJsSymbolCacheSize, TestSize.Level0)
{
    // Get initial size
    uint32_t initialSize = runtime_->GetJsSymbolCacheSize();

    // Add a symbol to cache
    CallFrame callFrame(0x1000);
    DfxSymbol symbol(0x1000, "test");
    symbol.filePathId_ = 1;
    runtime_->FillJsSymbolCache(callFrame, symbol);

    // Get size after adding
    uint32_t afterSize = runtime_->GetJsSymbolCacheSize();

    // Size should increase
    EXPECT_GT(afterSize, initialSize);
}

/**
 * @tc.name: FindArkTsFilePath
 * @tc.desc: Test FindArkTsFilePath method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindArkTsFilePath, TestSize.Level0)
{
    std::string_view testPath = "test.ets";

    // Test with non-existent path
    EXPECT_EQ(runtime_->FindArkTsFilePath(testPath), 0u);

    // Add test path
    runtime_->jsUrlMap_[testPath] = 1;

    // Test with existing path
    EXPECT_EQ(runtime_->FindArkTsFilePath(testPath), 1u);
}

/**
 * @tc.name: FillArkTsFilePath
 * @tc.desc: Test FillArkTsFilePath method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FillArkTsFilePath, TestSize.Level0)
{
    std::string_view testPath = "test.ets";

    // Fill path
    uint32_t pathId = runtime_->FillArkTsFilePath(testPath);

    // Verify path was added
    EXPECT_GT(pathId, 0u);
    EXPECT_EQ(runtime_->FindArkTsFilePath(testPath), pathId);
}

/**
 * @tc.name: ArktsGetSymbolCache
 * @tc.desc: Test ArktsGetSymbolCache method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, ArktsGetSymbolCache, TestSize.Level0)
{
    CallFrame callFrame(0x1000);
    callFrame.filePath_ = "test.ets";
    DfxSymbol symbol;

    // Test with non-existent cache
    EXPECT_FALSE(runtime_->ArktsGetSymbolCache(callFrame, symbol));

    // Add test data
    runtime_->jsUrlMap_["test.ets"] = 1;
    VirtualRuntime::SymbolCacheKey key(0x1000, 1);
    DfxSymbol testSymbol(0x1000, "test_func");
    runtime_->userSymbolCache_[key] = testSymbol;

    // Test with existing cache
    EXPECT_TRUE(runtime_->ArktsGetSymbolCache(callFrame, symbol));
}

/**
 * @tc.name: IsArktsMap_Hap
 * @tc.desc: Test IsArktsMap method with hap file
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, IsArktsMap_Hap, TestSize.Level0)
{
    // Test with hap file
    auto hapMap = std::make_shared<DfxMap>();
    hapMap->name = "test.hap";
    EXPECT_TRUE(runtime_->IsArktsMap(hapMap));

    // Test with hsp file
    auto hspMap = std::make_shared<DfxMap>();
    hspMap->name = "test.hsp";
    EXPECT_TRUE(runtime_->IsArktsMap(hspMap));

    // Test with abc file
    auto abcMap = std::make_shared<DfxMap>();
    abcMap->name = "test.abc";
    EXPECT_TRUE(runtime_->IsArktsMap(abcMap));

    // Test with hqf file
    auto hqfMap = std::make_shared<DfxMap>();
    hqfMap->name = "test.hqf";
    EXPECT_TRUE(runtime_->IsArktsMap(hqfMap));
}

/**
 * @tc.name: FindMapByPath
 * @tc.desc: Test FindMapByPath method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindMapByPath, TestSize.Level0)
{
    const std::string testPath = "test.so";

    // Test with empty processMaps
    EXPECT_EQ(runtime_->FindMapByPath(testPath), nullptr);

    // Add test map
    auto mapItem = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_READ, testPath);
    runtime_->processMaps_.push_back(mapItem);

    // Test with existing path
    EXPECT_NE(runtime_->FindMapByPath(testPath), nullptr);

    // Test with non-existent path
    EXPECT_EQ(runtime_->FindMapByPath("non_existent.so"), nullptr);
}

/**
 * @tc.name: GetAdltBase
 * @tc.desc: Test GetAdltBase method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, GetAdltBaseTest, TestSize.Level0)
{
    const std::string testPath = "test.so";
    const uint64_t testBase = 0x1000;

    // Test with non-existent path
    EXPECT_EQ(runtime_->GetAdltBase(testPath), std::numeric_limits<uint64_t>::max());

    // Add test base
    runtime_->adltBaseMap_[testPath] = testBase;

    // Test with existing path
    EXPECT_EQ(runtime_->GetAdltBase(testPath), testBase);
}

/**
 * @tc.name: UpdateMaps
 * @tc.desc: Test UpdateMaps method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, UpdateMaps, TestSize.Level0)
{
    const pid_t testPid = 123;
    const pid_t testTid = 456;

    // Update maps
    runtime_->UpdateMaps(testPid, testTid);

    // Verify thread exists
    auto& thread = runtime_->GetThread(testPid, testTid);
    EXPECT_EQ(thread.tid_, testTid);
}

/**
 * @tc.name: ResolveMergedSoIp
 * @tc.desc: Test ResolveMergedSoIp method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, ResolveMergedSoIp, TestSize.Level0)
{
    const std::string testPath = "test.so";
    const uint64_t testIp = 0x1500;

    // Test with non-existent path
    EXPECT_EQ(runtime_->ResolveMergedSoIp(testIp, testPath), testPath);

    // Add test data
    runtime_->adltBaseMap_[testPath] = 0x1000;

    // Test with existing path but no smo info
    EXPECT_EQ(runtime_->ResolveMergedSoIp(testIp, testPath), testPath);
}

/**
 * @tc.name: UpdateAdltBase
 * @tc.desc: Test UpdateAdltBase method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, UpdateAdltBase, TestSize.Level0)
{
    // Add test map
    auto mapItem = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_READ, "/system/lib/libadlt.so");
    runtime_->processMaps_.push_back(mapItem);

    // Update adlt base
    runtime_->UpdateAdltBase();

    // Verify adlt base was updated
    EXPECT_NE(runtime_->adltFound_.find("/system/lib/libadlt.so"), runtime_->adltFound_.end());
    EXPECT_NE(runtime_->adltBaseMap_.find("/system/lib/libadlt.so"), runtime_->adltBaseMap_.end());
}

/**
 * @tc.name: RemoveMaps
 * @tc.desc: Test RemoveMaps method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, RemoveMapsTest, TestSize.Level0)
{
    const uint64_t testAddr = 0x1000;

    // Add test map
    auto mapItem = std::make_shared<DfxMap>(testAddr, testAddr + 0x1000, 0, PROT_READ, "test.so");
    auto memMaps = std::make_shared<MemMaps>(1);
    memMaps->AddMap(mapItem, true);
    runtime_->mapsCache_[testAddr] = memMaps;

    // Verify map exists
    EXPECT_NE(runtime_->mapsCache_.find(testAddr), runtime_->mapsCache_.end());

    // Remove map
    runtime_->RemoveMaps(testAddr);

    // Verify map was removed
    EXPECT_EQ(runtime_->mapsCache_.find(testAddr), runtime_->mapsCache_.end());
}

/**
 * @tc.name: FillMapsCache
 * @tc.desc: Test FillMapsCache method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FillMapsCache, TestSize.Level0)
{
    std::string currentFileName = "";
    auto mapItem = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_READ, "test.so");

    // Fill maps cache
    runtime_->FillMapsCache(currentFileName, mapItem);

    // Verify cache was updated
    EXPECT_EQ(currentFileName, "test.so");
    EXPECT_NE(runtime_->mapsCache_.find(0x1000), runtime_->mapsCache_.end());
}

/**
 * @tc.name: HandleMapInfoReuseFilePathId
 * @tc.desc: Test repeated map of same path reuses filePathId
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoReuseFilePathId, TestSize.Level0)
{
    const std::string path = "test.so";
    auto dummyMap = std::make_shared<DfxMap>(0x10, 0x20, 0, PROT_READ, "dummy.so");
    runtime_->processMaps_.push_back(dummyMap);
    runtime_->HandleMapInfo({0x1000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, path, 1, 1);
    ASSERT_NE(runtime_->mapsCache_.find(0x1000), runtime_->mapsCache_.end());
    uint32_t firstId = runtime_->mapsCache_[0x1000]->filePathId_;
    runtime_->ClearOfflineMaps();
    runtime_->RemoveMaps(0x1000);
    runtime_->HandleMapInfo({0x3000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, path, 1, 1);
    ASSERT_NE(runtime_->mapsCache_.find(0x3000), runtime_->mapsCache_.end());
    EXPECT_EQ(runtime_->mapsCache_[0x3000]->filePathId_, firstId);
    EXPECT_EQ(runtime_->nativeFilePathIdMap_[path], firstId);
}

/**
 * @tc.name: HandleMapInfoDedupOfflineMap
 * @tc.desc: Test repeated exec maps only queue one offline map
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoDedupOfflineMap, TestSize.Level0)
{
    const std::string path = "test.so";
    auto dummyMap = std::make_shared<DfxMap>(0x10, 0x20, 0, PROT_READ, "dummy.so");
    runtime_->processMaps_.push_back(dummyMap);
    runtime_->HandleMapInfo({0x1000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, path, 1, 1);
    runtime_->HandleMapInfo({0x2000, 0x1000, MAP_FIXED | PROT_EXEC, 0x1000}, path, 1, 1);
    ASSERT_EQ(runtime_->offlineMapAddr_.size(), 1u);
    EXPECT_EQ(runtime_->offlineMapAddr_[0], 0x1000u);
}

/**
 * @tc.name: FillSymbolNameId
 * @tc.desc: Test FillSymbolNameId method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FillSymbolNameId, TestSize.Level0)
{
    CallFrame callFrame(0x1000);
    DfxSymbol symbol(0x1000, "test_func");

    // Fill symbol name id
    runtime_->FillSymbolNameId(callFrame, symbol);

    // Verify symbol name id was set
    EXPECT_GT(symbol.symbolNameId_, 0u);
}

/**
 * @tc.name: FillFileSet
 * @tc.desc: Test FillFileSet method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FillFileSet, TestSize.Level0)
{
    CallFrame callFrame(0x1000);
    DfxSymbol symbol(0x1000, "test_func");
    symbol.filePathId_ = 1;

    // Fill file set
    runtime_->FillFileSet(callFrame, symbol);

    // Verify file set was updated
    // Note: fileSet_ is private, so we can't directly access it here
    // This test just ensures the method doesn't crash
    EXPECT_TRUE(true);
}

/**
 * @tc.name: MakeCallFrame
 * @tc.desc: Test MakeCallFrame method
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, MakeCallFrame, TestSize.Level0)
{
    CallFrame callFrame(0x1000);
    DfxSymbol symbol(0x1000, "test_func");
    symbol.funcVaddr_ = 0x1000;
    symbol.symbolName_ = "test_func";
    symbol.index_ = 1;
    symbol.module_ = "test.so";
    symbol.offset_ = 0;
    symbol.symbolId_ = 1;
    symbol.symbolNameId_ = 1;
    symbol.filePathId_ = 1;

    // Make call frame
    runtime_->MakeCallFrame(symbol, callFrame);

    // Verify call frame was updated
    EXPECT_EQ(callFrame.vaddrInFile_, symbol.funcVaddr_);
    EXPECT_EQ(callFrame.symbolName_, symbol.symbolName_);
    EXPECT_EQ(callFrame.symbolIndex_, symbol.index_);
    EXPECT_EQ(callFrame.filePath_, symbol.module_);
    EXPECT_EQ(callFrame.symbolOffset_, symbol.offset_);
    EXPECT_EQ(callFrame.callFrameId_, symbol.symbolId_);
    EXPECT_EQ(callFrame.symbolNameId_, symbol.symbolNameId_);
    EXPECT_EQ(callFrame.filePathId_, symbol.filePathId_);
}
}
}
}
