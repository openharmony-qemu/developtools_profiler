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

#include "native_hook_result.pbencoder.h"
#include "plugin_module_api.h"
#include "virtual_runtime.h"
#include "symbols_file.h"

using namespace testing::ext;
using namespace std;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
constexpr size_t VIRTUAL_RUNTIME_ENCODER_BUFFER_SIZE = 4096;
static uint8_t g_virtualRuntimeEncoderBuffer[VIRTUAL_RUNTIME_ENCODER_BUFFER_SIZE];
static uint32_t g_virtualRuntimeEncoderOffset = 0;

static bool GetVirtualRuntimeEncoderMemory(RandomWriteCtx*, uint32_t size, uint8_t** memory, uint32_t* offset)
{
    if (memory == nullptr || offset == nullptr ||
        g_virtualRuntimeEncoderOffset + size > VIRTUAL_RUNTIME_ENCODER_BUFFER_SIZE) {
        return false;
    }
    *memory = g_virtualRuntimeEncoderBuffer + g_virtualRuntimeEncoderOffset;
    *offset = g_virtualRuntimeEncoderOffset;
    g_virtualRuntimeEncoderOffset += size;
    return true;
}

static bool SeekVirtualRuntimeEncoderMemory(RandomWriteCtx*, uint32_t offset)
{
    if (offset > VIRTUAL_RUNTIME_ENCODER_BUFFER_SIZE) {
        return false;
    }
    g_virtualRuntimeEncoderOffset = offset;
    return true;
}

static RandomWriteCtx MakeVirtualRuntimeEncoderCtx()
{
    g_virtualRuntimeEncoderOffset = 0;
    return {GetVirtualRuntimeEncoderMemory, SeekVirtualRuntimeEncoderMemory};
}

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
    config.set_async_stack_enable(false);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    callbackCount_ = 0;
    std::unique_lock<std::shared_mutex> lock(g_smoMapsLock);
    g_smoMaps.clear();
}

void VirtualRuntimeExtTest::TearDown()
{
    runtime_.reset();
    std::unique_lock<std::shared_mutex> lock(g_smoMapsLock);
    g_smoMaps.clear();
}

void VirtualRuntimeExtTest::PrepareTestSymbols()
{
    std::string testSymbol = "test_symbol";
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    symbolsFile->filePath_ = testSymbol;
    symbolsFile->symbols_.emplace_back(0x1000, 1u, "test_func", symbolsFile->filePath_);
    runtime_->symbolsFiles_[symbolsFile->filePath_] = std::move(symbolsFile);
}

static DfxSymbol GetSymbolForTest(VirtualRuntime& runtime, CallFrame& callFrame, pid_t pid, pid_t tid,
    const perf_callchain_context& context)
{
    DfxSymbol symbol;
    runtime.GetSymbol(symbol, callFrame, pid, tid, context);
    return symbol;
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
    runtime_->arktsMapTree_[0x1000] = {mapItem, 0};
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

HWTEST_F(VirtualRuntimeExtTest, UpdateSymbolsKeepsExistingEntry, TestSize.Level0)
{
    const std::string testFile = "existing.so";
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(testFile);
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = testFile;
    SymbolsFile* existing = symbolsFile.get();
    runtime_->symbolsFiles_[testFile] = std::move(symbolsFile);

    auto map = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_EXEC, testFile);
    runtime_->UpdateSymbols(testFile, map);

    ASSERT_EQ(runtime_->symbolsFiles_.size(), 1u);
    EXPECT_EQ(runtime_->symbolsFiles_[testFile].get(), existing);
}

HWTEST_F(VirtualRuntimeExtTest, UpdateSymbolsCoversLoggedInsertAndExistingEntry, TestSize.Level0)
{
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    const std::string insertedFile = "inserted_logged_symbol.so";
    auto insertedMap = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_EXEC, insertedFile);
    runtime_->UpdateSymbols(insertedFile, insertedMap);
    EXPECT_NE(runtime_->symbolsFiles_.find(insertedFile), runtime_->symbolsFiles_.end());

    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    ASSERT_NE(symbolsFile, nullptr);
    const std::string existingFile = "existing_logged_symbol.so";
    symbolsFile->filePath_ = existingFile;
    SymbolsFile* existing = symbolsFile.get();
    runtime_->symbolsFiles_[existingFile] = std::move(symbolsFile);

    runtime_->UpdateSymbols(existingFile, insertedMap);
    EXPECT_EQ(runtime_->symbolsFiles_[existingFile].get(), existing);
}

HWTEST_F(VirtualRuntimeExtTest, UpdateSymbolsStoresEntryWithSearchPath, TestSize.Level0)
{
    const std::string testFile = "search_path_missing.so";
    runtime_->symbolsPaths_.push_back(".");
    auto map = std::make_shared<DfxMap>(0x3000, 0x4000, 0, PROT_EXEC, testFile);

    runtime_->UpdateSymbols(testFile, map);

    EXPECT_TRUE(runtime_->IsSymbolExist(testFile));
}

HWTEST_F(VirtualRuntimeExtTest, UpdateHapSymbolsReturnsFalseWhenSymbolFileMissing, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_EXEC,
        "/data/local/tmp/nonexistent_update_hap_symbols.so");
    EXPECT_FALSE(runtime_->UpdateHapSymbols(map));
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
    runtime_->arktsMapTree_[0x1000] = {mapItem, 0};
    runtime_->arktsMapCache_.clear();

    // Test with address in range
    EXPECT_NE(runtime_->FindArktsMapByAddr(0x1500), nullptr);

    // Test with address before the first map
    EXPECT_EQ(runtime_->FindArktsMapByAddr(0x500), nullptr);

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
    runtime_->processMaps_.push_back(nullptr);
    auto mapItem = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_READ, testPath);
    runtime_->processMaps_.push_back(mapItem);

    // Test with existing path
    EXPECT_NE(runtime_->FindMapByPath(testPath), nullptr);

    // Test with non-existent path
    EXPECT_EQ(runtime_->FindMapByPath("non_existent.so"), nullptr);
}

HWTEST_F(VirtualRuntimeExtTest, FindMapReturnsInnerSegmentIndex, TestSize.Level0)
{
    constexpr uint64_t begin = 0x1000;
    auto firstMap = std::make_shared<DfxMap>(begin, 0x2000, 0, PROT_READ, "multi.so");
    auto secondMap = std::make_shared<DfxMap>(0x2000, 0x3000, 0x1000, PROT_EXEC, "multi.so");
    auto memMaps = std::make_shared<MemMaps>(1);
    memMaps->AddMap(firstMap, true);
    memMaps->AddMap(secondMap, false);
    memMaps->soEnd_ = secondMap->end;
    runtime_->mapsCache_[begin] = memMaps;

    auto [maps, index] = runtime_->FindMap(0x2500);
    ASSERT_NE(maps, nullptr);
    EXPECT_EQ(index, 1u);
    EXPECT_EQ(maps->GetMaps()[index]->name, "multi.so");

    auto [missingMaps, missingIndex] = runtime_->FindMap(0x3500);
    EXPECT_EQ(missingMaps, nullptr);
    EXPECT_EQ(missingIndex, 0u);
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

HWTEST_F(VirtualRuntimeExtTest, UnwindStackReturnsTrueWhenNoStackPayload, TestSize.Level0)
{
    std::vector<u64> regs;
    std::vector<CallFrame> callFrames;

    EXPECT_TRUE(runtime_->UnwindStack(regs, nullptr, 0, 1, 2, callFrames, 8));
    EXPECT_TRUE(callFrames.empty());
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

HWTEST_F(VirtualRuntimeExtTest, ResolveMergedSoIpHandlesSmoHitAndMisses, TestSize.Level0)
{
    const std::string path = "libadlt-test.so";
    runtime_->adltBaseMap_[path] = 0x1000;
    {
        std::unique_lock<std::shared_mutex> lock(g_smoMapsLock);
        auto& entry = g_smoMaps[path];
        entry.initialized = true;
        entry.info.set_file_path(path);
        auto* mapEntry = entry.info.add_map_entries();
        mapEntry->set_start_addr(0x100);
        mapEntry->set_end_addr(0x300);
        mapEntry->set_so_index(7);
        auto* idEntry = entry.info.add_id_tlb_entries();
        idEntry->set_so_index(7);
        idEntry->set_name("libreal.so");
    }

    EXPECT_EQ(runtime_->ResolveMergedSoIp(0x1100, path), path + ":libreal.so");
    EXPECT_EQ(runtime_->ResolveMergedSoIp(0x1500, path), path);

    {
        std::unique_lock<std::shared_mutex> lock(g_smoMapsLock);
        g_smoMaps[path].info.clear_id_tlb_entries();
    }
    EXPECT_EQ(runtime_->ResolveMergedSoIp(0x1100, path), path);
}

HWTEST_F(VirtualRuntimeExtTest, ResolveMergedSoIpHandlesInvalidBaseAndEmptySoName, TestSize.Level0)
{
    const std::string invalidBasePath = "invalid_base.so";
    runtime_->adltBaseMap_[invalidBasePath] = std::numeric_limits<uint64_t>::max();
    EXPECT_EQ(runtime_->ResolveMergedSoIp(0x1100, invalidBasePath), invalidBasePath);

    const std::string emptyNamePath = "empty_name.so";
    runtime_->adltBaseMap_[emptyNamePath] = 0x1000;
    {
        std::unique_lock<std::shared_mutex> lock(g_smoMapsLock);
        auto& entry = g_smoMaps[emptyNamePath];
        entry.initialized = true;
        entry.info.set_file_path(emptyNamePath);
        auto* mapEntry = entry.info.add_map_entries();
        mapEntry->set_start_addr(0x100);
        mapEntry->set_end_addr(0x200);
        mapEntry->set_so_index(5);
        entry.info.add_id_tlb_entries()->set_so_index(5);
    }

    EXPECT_EQ(runtime_->ResolveMergedSoIp(0x1100, emptyNamePath), emptyNamePath);
}

HWTEST_F(VirtualRuntimeExtTest, TryEmitSmoMapsInfoEmitsOnceAndSkipsUninitialized, TestSize.Level0)
{
    const std::string readyPath = "ready.so";
    const std::string skippedPath = "skipped.so";
    {
        std::unique_lock<std::shared_mutex> lock(g_smoMapsLock);
        g_smoMaps[skippedPath].initialized = false;
        auto& entry = g_smoMaps[readyPath];
        entry.initialized = true;
        entry.info.set_file_path(readyPath);
        auto* mapEntry = entry.info.add_map_entries();
        mapEntry->set_start_addr(0x10);
        mapEntry->set_end_addr(0x20);
        mapEntry->set_so_index(3);
        auto* idEntry = entry.info.add_id_tlb_entries();
        idEntry->set_so_index(3);
        idEntry->set_name("inner.so");
    }

    BatchNativeHookData data;
    runtime_->TryEmitSmoMapsInfo(data);
    ASSERT_EQ(data.events_size(), 1);
    const auto& smo = data.events(0).smo_map();
    EXPECT_EQ(smo.file_path(), readyPath);
    ASSERT_EQ(smo.map_entries_size(), 1);
    EXPECT_EQ(smo.map_entries(0).so_index(), 3u);
    ASSERT_EQ(smo.id_tlb_entries_size(), 1);
    EXPECT_EQ(smo.id_tlb_entries(0).name(), "inner.so");

    BatchNativeHookData secondData;
    runtime_->TryEmitSmoMapsInfo(secondData);
    EXPECT_EQ(secondData.events_size(), 0);
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

HWTEST_F(VirtualRuntimeExtTest, UpdateAdltBaseSkipsExecAndAlreadyFoundMaps, TestSize.Level0)
{
    const std::string adltPath = "/system/lib/libadlt_branch.so";
    auto execMap = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_EXEC, adltPath);
    auto readMap = std::make_shared<DfxMap>(0x3000, 0x4000, 0, PROT_READ, adltPath);
    runtime_->processMaps_.push_back(execMap);
    runtime_->processMaps_.push_back(readMap);

    runtime_->UpdateAdltBase();

    ASSERT_NE(runtime_->adltBaseMap_.find(adltPath), runtime_->adltBaseMap_.end());
    EXPECT_EQ(runtime_->adltBaseMap_[adltPath], readMap->begin);
    runtime_->UpdateAdltBase();
    EXPECT_TRUE(runtime_->adltFound_[adltPath]);
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

HWTEST_F(VirtualRuntimeExtTest, FillMapsCacheAppendsExecSegment, TestSize.Level0)
{
    std::string currentFileName;
    auto headMap = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_READ, "append.so");
    auto execMap = std::make_shared<DfxMap>(0x2000, 0x3000, 0x1000, PROT_EXEC, "append.so");

    runtime_->FillMapsCache(currentFileName, headMap);
    runtime_->FillMapsCache(currentFileName, execMap);

    ASSERT_NE(runtime_->mapsCache_.find(0x1000), runtime_->mapsCache_.end());
    EXPECT_EQ(runtime_->mapsCache_[0x1000]->soEnd_, 0x3000u);
    ASSERT_EQ(runtime_->offlineMapAddr_.size(), 1u);
    EXPECT_EQ(runtime_->offlineMapAddr_[0], 0x1000u);
}

HWTEST_F(VirtualRuntimeExtTest, FillMapsCacheKeepsExistingSegmentWhenNameMatches, TestSize.Level0)
{
    std::string currentFileName;
    auto mapItem = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_READ, "repeat.so");
    runtime_->FillMapsCache(currentFileName, mapItem);
    runtime_->FillMapsCache(currentFileName, mapItem);
    EXPECT_EQ(runtime_->mapsCache_.size(), 1u);
}

HWTEST_F(VirtualRuntimeExtTest, FillMapsCacheSkipsSameNameWhenCacheMissing, TestSize.Level0)
{
    std::string currentFileName = "orphan.so";
    runtime_->soBegin_ = 0xDEAD;
    auto mapItem = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_EXEC, currentFileName);

    runtime_->FillMapsCache(currentFileName, mapItem);

    EXPECT_TRUE(runtime_->mapsCache_.empty());
    EXPECT_EQ(runtime_->offlineMapAddr_.size(), 0u);
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoAppendsExistingHapExecSegment, TestSize.Level0)
{
    const std::string filePath = "branch_handle.hap";
    std::vector<uint64_t> headInfo = {0x1000, 0x1000, MAP_FIXED | PROT_READ, 0};
    std::vector<uint64_t> execInfo = {0x2000, 0x1000, MAP_FIXED | PROT_EXEC, 0x1000};

    runtime_->HandleMapInfo(headInfo, filePath, 11, 12);
    ASSERT_NE(runtime_->mapsCache_.find(0x1000), runtime_->mapsCache_.end());
    runtime_->HandleMapInfo(execInfo, filePath, 11, 12);

    const auto& maps = runtime_->mapsCache_[0x1000]->GetMaps();
    ASSERT_FALSE(maps.empty());
    if (maps.size() > 1) {
        EXPECT_EQ(maps.back()->name, filePath);
        EXPECT_EQ(maps.back()->prevMap, maps[maps.size() - 2]);
    }
    EXPECT_FALSE(runtime_->offlineMapAddr_.empty());
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

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoRejectsInvalidAndNonFixedMaps, TestSize.Level0)
{
    runtime_->HandleMapInfo({0x1000, 0x1000, PROT_EXEC}, "bad.so", 1, 1);
    EXPECT_TRUE(runtime_->mapsCache_.empty());

    runtime_->HandleMapInfo({0x1000, 0x1000, PROT_EXEC, 0}, "not_fixed.so", 1, 1);
    EXPECT_TRUE(runtime_->mapsCache_.empty());
}

HWTEST_F(VirtualRuntimeExtTest, EnsureSmoInfoForFileRejectsMissingMapAndEmptyAdlt, TestSize.Level0)
{
    EXPECT_FALSE(runtime_->EnsureSmoInfoForFile("missing.so"));

    auto map = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_EXEC, "empty.so");
    runtime_->processMaps_.push_back(map);
    EXPECT_FALSE(runtime_->EnsureSmoInfoForFile("empty.so"));
}

HWTEST_F(VirtualRuntimeExtTest, OfflineFileReportedTracksDuplicates, TestSize.Level0)
{
    EXPECT_TRUE(runtime_->MarkOfflineFileReported(7));
    EXPECT_FALSE(runtime_->MarkOfflineFileReported(7));
    runtime_->ResetOfflineFileReported(7);
    EXPECT_TRUE(runtime_->MarkOfflineFileReported(7));
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
    EXPECT_TRUE(callFrame.needReport_ & SYMBOL_NAME_ID_REPORT);

    DfxSymbol secondSymbol(0x2000, "test_func");
    callFrame.needReport_ = 0;
    runtime_->FillSymbolNameId(callFrame, secondSymbol);
    EXPECT_EQ(secondSymbol.symbolNameId_, symbol.symbolNameId_);
    EXPECT_FALSE(callFrame.needReport_ & SYMBOL_NAME_ID_REPORT);
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
    EXPECT_TRUE(callFrame.needReport_ & FILE_PATH_ID_REPORT);

    callFrame.needReport_ = 0;
    runtime_->FillFileSet(callFrame, symbol);
    EXPECT_TRUE(callFrame.needReport_ & FILE_PATH_ID_REPORT);
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

HWTEST_F(VirtualRuntimeExtTest, MakeCallFrameUsesFallbackFileAndOffset, TestSize.Level0)
{
    CallFrame callFrame(0x3000);
    DfxSymbol symbol(0x3000, "fallback_func");
    symbol.funcVaddr_ = 0;
    symbol.comm_ = "fallback_comm";

    runtime_->MakeCallFrame(symbol, callFrame);

    EXPECT_EQ(callFrame.filePath_, "fallback_comm");
    EXPECT_EQ(callFrame.offset_, callFrame.ip_);
}

HWTEST_F(VirtualRuntimeExtTest, UpdateThreadKeepsExistingNameWhenNewNameEmpty, TestSize.Level0)
{
    auto& thread = runtime_->UpdateThread(1, 2, "worker");
    ASSERT_EQ(thread.name_, "worker");

    auto& sameThread = runtime_->UpdateThread(1, 2, "");

    EXPECT_EQ(&sameThread, &thread);
    EXPECT_EQ(sameThread.name_, "worker");
}

HWTEST_F(VirtualRuntimeExtTest, ConstructorAndReadThreadNameHandleOfflineAndInvalidTid, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_offline_symbolization(true);
    VirtualRuntime offlineRuntime(config);

    EXPECT_EQ(offlineRuntime.GetJsSymbolCacheSize(), 1u);
    EXPECT_TRUE(runtime_->ReadThreadName(-1).empty());
}

HWTEST_F(VirtualRuntimeExtTest, GetKernelSymbolFallsBackWhenSymbolFileMissing, TestSize.Level0)
{
    constexpr uint64_t ip = 0x1800;
    std::vector<std::shared_ptr<DfxMap>> maps;
    maps.push_back(std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_EXEC, "kernel_missing.so"));
    auto& thread = runtime_->UpdateThread(1, 2, "kernel_thread");

    DfxSymbol symbol = runtime_->GetKernelSymbol(ip, maps, thread);

    EXPECT_EQ(symbol.module_, "kernel_missing.so");
    EXPECT_EQ(symbol.taskVaddr_, ip);
}

HWTEST_F(VirtualRuntimeExtTest, GetKernelSymbolFallsBackWhenLookupMisses, TestSize.Level0)
{
    constexpr uint64_t ip = 0x2800;
    const std::string module = "kernel_empty_symbols.so";
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(module);
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = module;
    runtime_->symbolsFiles_[module] = std::move(symbolsFile);
    std::vector<std::shared_ptr<DfxMap>> maps;
    maps.push_back(std::make_shared<DfxMap>(0x2000, 0x3000, 0, PROT_EXEC, module));
    auto& thread = runtime_->UpdateThread(1, 2, "kernel_thread");

    DfxSymbol symbol = runtime_->GetKernelSymbol(ip, maps, thread);

    EXPECT_EQ(symbol.module_, module);
    EXPECT_EQ(symbol.taskVaddr_, ip);
}

HWTEST_F(VirtualRuntimeExtTest, GetKernelSymbolLogsMapMissBeforeHit, TestSize.Level0)
{
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    constexpr uint64_t ip = 0x2800;
    std::vector<std::shared_ptr<DfxMap>> maps;
    maps.push_back(std::make_shared<DfxMap>(0x1000, 0x1800, 0, PROT_EXEC, "kernel_left.so"));
    maps.push_back(std::make_shared<DfxMap>(0x2000, 0x3000, 0, PROT_EXEC, "kernel_target.so"));
    auto& thread = runtime_->UpdateThread(1, 2, "kernel_thread");

    DfxSymbol symbol = runtime_->GetKernelSymbol(ip, maps, thread);

    EXPECT_EQ(symbol.module_, "kernel_target.so");
    EXPECT_EQ(symbol.taskVaddr_, ip);
}

HWTEST_F(VirtualRuntimeExtTest, GetUserSymbolSetsFilePathIdWhenLookupMisses, TestSize.Level0)
{
    constexpr uint64_t ip = 0x3500;
    constexpr uint32_t filePathId = 23;
    const std::string module = "user_empty_symbols.so";
    auto map = std::make_shared<DfxMap>(0x3000, 0x4000, 0, PROT_EXEC, module);
    auto memMaps = std::make_shared<MemMaps>(filePathId);
    memMaps->AddMap(map, true);
    runtime_->mapsCache_[map->begin] = memMaps;
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(module);
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = module;
    runtime_->symbolsFiles_[module] = std::move(symbolsFile);
    auto& thread = runtime_->UpdateThread(1, 2, "user_thread");

    DfxSymbol symbol = runtime_->GetUserSymbol(ip, memMaps, 0, thread);

    EXPECT_EQ(symbol.module_, module);
    EXPECT_EQ(symbol.filePathId_, filePathId);
}

HWTEST_F(VirtualRuntimeExtTest, GetUserSymbolFallsBackWhenSymbolFileMissing, TestSize.Level0)
{
    constexpr uint64_t ip = 0x4500;
    constexpr uint32_t filePathId = 24;
    auto map = std::make_shared<DfxMap>(0x4000, 0x5000, 0, PROT_EXEC, "missing_user_symbol.so");
    auto memMaps = std::make_shared<MemMaps>(filePathId);
    memMaps->AddMap(map, true);
    memMaps->soEnd_ = map->end;
    runtime_->mapsCache_[map->begin] = memMaps;
    auto& thread = runtime_->UpdateThread(1, 2, "user_thread");

    DfxSymbol symbol = runtime_->GetUserSymbol(ip, memMaps, 0, thread);

    EXPECT_TRUE(symbol.module_.empty());
    EXPECT_EQ(symbol.filePathId_, 0u);
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolReturnsUserCacheHit, TestSize.Level0)
{
    constexpr uint64_t ip = 0x2500;
    constexpr uint32_t filePathId = 12;
    auto map = std::make_shared<DfxMap>(0x2000, 0x3000, 0, PROT_EXEC, "cached_user.so");
    auto memMaps = std::make_shared<MemMaps>(filePathId);
    memMaps->AddMap(map, true);
    memMaps->soEnd_ = map->end;
    runtime_->mapsCache_[map->begin] = memMaps;

    DfxSymbol cached(ip, "cached_func");
    cached.symbolName_ = "cached_func";
    cached.filePathId_ = filePathId;
    cached.symbolId_ = 3;
    runtime_->userSymbolCache_[std::pair(ip, filePathId)] = cached;
    CallFrame callFrame(ip);

    DfxSymbol symbol = GetSymbolForTest(*runtime_, callFrame, 1, 2, PERF_CONTEXT_USER);

    EXPECT_EQ(symbol.symbolName_, "cached_func");
    EXPECT_EQ(symbol.symbolId_, cached.symbolId_);
    EXPECT_EQ(callFrame.needReport_, 0u);
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolSkipsUserLookupForKernelContext, TestSize.Level0)
{
    CallFrame callFrame(0x1200);

    DfxSymbol symbol = GetSymbolForTest(*runtime_, callFrame, 1, 2, PERF_CONTEXT_KERNEL);

    EXPECT_FALSE(symbol.IsValid());
    EXPECT_EQ(callFrame.needReport_, 0u);
    EXPECT_TRUE(runtime_->userSymbolCache_.empty());
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolCreatesJsCacheForFpUnwindFrame, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_fp_unwind(true);
    config.set_string_compressed(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    CallFrame callFrame(0x7000);
    callFrame.isJsFrame_ = true;
    callFrame.filePath_ = "entry/src/main.ets";
    callFrame.symbolName_ = "onClick";

    DfxSymbol symbol = GetSymbolForTest(*runtime_, callFrame, 1, 2, PERF_CONTEXT_USER);

    EXPECT_EQ(symbol.module_, "entry/src/main.ets");
    EXPECT_EQ(symbol.symbolName_, "onClick");
    EXPECT_GT(symbol.filePathId_, 0u);
    EXPECT_GT(symbol.symbolNameId_, 0u);
    EXPECT_TRUE(callFrame.needReport_ & CALL_FRAME_REPORT);

    CallFrame cachedFrame(0x7000);
    cachedFrame.isJsFrame_ = true;
    cachedFrame.filePath_ = "entry/src/main.ets";
    cachedFrame.symbolName_ = "onClick";
    DfxSymbol cached = GetSymbolForTest(*runtime_, cachedFrame, 1, 2, PERF_CONTEXT_USER);
    EXPECT_EQ(cached.symbolId_, symbol.symbolId_);
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolCreatesJsCacheWithoutStringCompression, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_fp_unwind(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    CallFrame callFrame(0x7100);
    callFrame.isJsFrame_ = true;
    callFrame.filePath_ = "entry/src/plain.ets";
    callFrame.symbolName_ = "plainClick";

    DfxSymbol symbol = GetSymbolForTest(*runtime_, callFrame, 1, 2, PERF_CONTEXT_USER);

    EXPECT_EQ(symbol.module_, "entry/src/plain.ets");
    EXPECT_EQ(symbol.symbolName_, "plainClick");
    EXPECT_EQ(symbol.symbolNameId_, 0u);
    EXPECT_TRUE(callFrame.needReport_ & CALL_FRAME_REPORT);
    EXPECT_FALSE(callFrame.needReport_ & SYMBOL_NAME_ID_REPORT);
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolUsesUserLookupForPerfContextMax, TestSize.Level0)
{
    constexpr uint64_t mapBegin = 0xB000;
    constexpr uint64_t symbolVaddr = 0x80;
    const std::string filePath = "context_max_user.so";
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = filePath;
    symbolsFile->symbols_.emplace_back(symbolVaddr, 0x20, "context_max_func", filePath);
    symbolsFile->textExecVaddrFileOffset_ = symbolVaddr;
    symbolsFile->textExecVaddr_ = symbolVaddr;
    runtime_->symbolsFiles_[filePath] = std::move(symbolsFile);
    runtime_->GetThread(1, 2).CreateMapItem(filePath, mapBegin, 0x1000, 0);
    CallFrame callFrame(mapBegin + symbolVaddr);

    DfxSymbol symbol = GetSymbolForTest(*runtime_, callFrame, 1, 2, PERF_CONTEXT_MAX);

    EXPECT_TRUE(symbol.IsValid());
    EXPECT_EQ(symbol.symbolName_, "context_max_func");
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolReportsCompressedUserSymbolMapsOnce, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_string_compressed(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    constexpr uint64_t mapBegin = 0xC000;
    constexpr uint64_t symbolVaddr = 0x40;
    constexpr uint32_t filePathId = 31;
    const std::string filePath = "compressed_user.so";
    auto map = std::make_shared<DfxMap>(mapBegin, mapBegin + 0x1000, 0, PROT_EXEC, filePath);
    auto memMaps = std::make_shared<MemMaps>(filePathId);
    memMaps->AddMap(map, true);
    memMaps->soEnd_ = map->end;
    runtime_->mapsCache_[mapBegin] = memMaps;
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = filePath;
    symbolsFile->symbols_.emplace_back(symbolVaddr, 0x20, "compressed_func", filePath);
    symbolsFile->textExecVaddrFileOffset_ = symbolVaddr;
    symbolsFile->textExecVaddr_ = symbolVaddr;
    runtime_->symbolsFiles_[filePath] = std::move(symbolsFile);

    CallFrame firstFrame(mapBegin + symbolVaddr);
    DfxSymbol first = GetSymbolForTest(*runtime_, firstFrame, 1, 2, PERF_CONTEXT_USER);
    EXPECT_TRUE(first.IsValid());
    EXPECT_TRUE(firstFrame.needReport_ & CALL_FRAME_REPORT);
    EXPECT_TRUE(firstFrame.needReport_ & SYMBOL_NAME_ID_REPORT);
    EXPECT_TRUE(firstFrame.needReport_ & FILE_PATH_ID_REPORT);

    CallFrame secondFrame(mapBegin + symbolVaddr + 4);
    DfxSymbol second = GetSymbolForTest(*runtime_, secondFrame, 1, 2, PERF_CONTEXT_USER);
    EXPECT_TRUE(second.IsValid());
    EXPECT_TRUE(secondFrame.needReport_ & CALL_FRAME_REPORT);
    EXPECT_FALSE(secondFrame.needReport_ & SYMBOL_NAME_ID_REPORT);
    EXPECT_TRUE(secondFrame.needReport_ & FILE_PATH_ID_REPORT);
}

HWTEST_F(VirtualRuntimeExtTest, FindArktsMapByAddrHandlesBeforeBeginAndNullMap, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0x2000, 0x3000, 0, PROT_EXEC, "entry.abc");
    runtime_->arktsMapTree_[map->begin] = {map, 0};
    runtime_->arktsMapTree_[0x4000] = {nullptr, 0};

    EXPECT_EQ(runtime_->FindArktsMapByAddr(0x1000), nullptr);
    EXPECT_EQ(runtime_->FindArktsMapByAddr(0x4001), nullptr);
}

HWTEST_F(VirtualRuntimeExtTest, SymbolizeFrameRejectsMapWithoutName, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0x1000, 0x2000, 0, PROT_EXEC, "");
    runtime_->arktsMapTree_[map->begin] = {map, 0};
    CallFrame callFrame(0x1008);

    EXPECT_FALSE(runtime_->SymbolizeFrame(callFrame, 1, 2));
}

HWTEST_F(VirtualRuntimeExtTest, SymbolizeFrameUsesJsSymbolCache, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0x3000, 0x4000, 0, PROT_EXEC, "entry/src/main.ets");
    std::string_view moduleName = map->name;
    uint32_t filePathId = runtime_->FillArkTsFilePath(moduleName);
    runtime_->arktsMapTree_[map->begin] = {map, filePathId};
    DfxSymbol cachedSymbol(0x3008, "onClick");
    cachedSymbol.module_ = map->name;
    cachedSymbol.symbolName_ = "onClick";
    cachedSymbol.filePathId_ = filePathId;
    cachedSymbol.symbolId_ = 77;
    runtime_->userSymbolCache_[VirtualRuntime::SymbolCacheKey(0x3008, filePathId)] = cachedSymbol;
    CallFrame callFrame(0x3008);

    EXPECT_TRUE(runtime_->SymbolizeFrame(callFrame, 1, 2));
    EXPECT_TRUE(callFrame.isJsFrame_);
    EXPECT_EQ(callFrame.filePath_, map->name);
    EXPECT_EQ(callFrame.symbolName_, "onClick");
    EXPECT_EQ(callFrame.callFrameId_, cachedSymbol.symbolId_);
}

HWTEST_F(VirtualRuntimeExtTest, SymbolizeFrameUsesCachedFilePathId, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0x5000, 0x6000, 0, PROT_EXEC, "entry/src/cached.ets");
    constexpr uint32_t filePathId = 31;
    runtime_->arktsMapTree_[map->begin] = {map, filePathId};
    DfxSymbol cachedSymbol(0x5008, "cachedClick");
    cachedSymbol.module_ = map->name;
    cachedSymbol.symbolName_ = "cachedClick";
    cachedSymbol.filePathId_ = filePathId;
    cachedSymbol.symbolId_ = 78;
    runtime_->userSymbolCache_[VirtualRuntime::SymbolCacheKey(0x5008, filePathId)] = cachedSymbol;
    CallFrame callFrame(0x5008);

    EXPECT_TRUE(runtime_->SymbolizeFrame(callFrame, 1, 2));
    EXPECT_EQ(callFrame.filePath_, map->name);
    EXPECT_EQ(callFrame.symbolName_, "cachedClick");
    EXPECT_EQ(callFrame.callFrameId_, cachedSymbol.symbolId_);
}

HWTEST_F(VirtualRuntimeExtTest, SymbolizeFrameRejectsMissingArktsMap, TestSize.Level0)
{
    CallFrame callFrame(0x6008);

    EXPECT_FALSE(runtime_->SymbolizeFrame(callFrame, 1, 2));
    EXPECT_FALSE(callFrame.isJsFrame_);
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolNameHonorsOffsetForJsLookupMiss, TestSize.Level0)
{
    std::vector<CallFrame> callFrames = {CallFrame(0x6100), CallFrame(0x6200)};
    callFrames[1].isJsFrame_ = true;

    EXPECT_TRUE(runtime_->GetSymbolName(1, 2, callFrames, 1, true, SymbolType::JS_SYMBOL));

    ASSERT_EQ(callFrames.size(), 1u);
    EXPECT_EQ(callFrames[0].ip_, 0x6100u);
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolNameSkipsMismatchedFrameTypes, TestSize.Level0)
{
    std::vector<CallFrame> nativeFrames = {CallFrame(0x1000)};
    EXPECT_TRUE(runtime_->GetSymbolName(1, 2, nativeFrames, 0, true, SymbolType::JS_SYMBOL));
    ASSERT_EQ(nativeFrames.size(), 1u);
    EXPECT_FALSE(nativeFrames[0].isJsFrame_);

    CallFrame jsFrame(0x2000);
    jsFrame.isJsFrame_ = true;
    std::vector<CallFrame> jsFrames = {jsFrame};
    EXPECT_TRUE(runtime_->GetSymbolName(1, 2, jsFrames, 0, true, SymbolType::NATIVE_SYMBOL));
    ASSERT_EQ(jsFrames.size(), 1u);
    EXPECT_TRUE(jsFrames[0].isJsFrame_);
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolNameSkipsPerfContextMarker, TestSize.Level0)
{
    std::vector<CallFrame> callFrames = {CallFrame(PERF_CONTEXT_USER)};

    EXPECT_TRUE(runtime_->GetSymbolName(1, 2, callFrames, 0, false, SymbolType::NATIVE_SYMBOL));

    ASSERT_EQ(callFrames.size(), 1u);
    EXPECT_EQ(callFrames[0].ip_, static_cast<uint64_t>(PERF_CONTEXT_USER));
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolNameDropsFramesAfterNativeLookupMiss, TestSize.Level0)
{
    std::vector<CallFrame> callFrames = {CallFrame(0x7100), CallFrame(0x7200)};

    EXPECT_TRUE(runtime_->GetSymbolName(1, 2, callFrames, 0, true, SymbolType::NATIVE_SYMBOL));

    EXPECT_TRUE(callFrames.empty());
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolNameReportsPeriodicLookupMiss, TestSize.Level0)
{
    for (int i = 0; i < 10; ++i) {
        std::vector<CallFrame> callFrames = {CallFrame(0x8100 + i)};
        EXPECT_TRUE(runtime_->GetSymbolName(1, 2, callFrames, 0, true, SymbolType::NATIVE_SYMBOL));
        EXPECT_TRUE(callFrames.empty());
    }
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolNameKeepsTrailingJsFrameOnNativeMiss, TestSize.Level0)
{
    CallFrame jsFrame(0x7400);
    jsFrame.isJsFrame_ = true;
    std::vector<CallFrame> callFrames = {CallFrame(0x7300), jsFrame};

    EXPECT_TRUE(runtime_->GetSymbolName(1, 2, callFrames, 0, true, SymbolType::NATIVE_SYMBOL));

    ASSERT_EQ(callFrames.size(), 1u);
    EXPECT_TRUE(callFrames[0].isJsFrame_);
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolNameAppliesResolvedUserSymbol, TestSize.Level0)
{
    constexpr uint64_t mapBegin = 0x9000;
    constexpr uint64_t symbolVaddr = 0x100;
    const std::string filePath = "resolved_user_symbol.so";
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = filePath;
    symbolsFile->symbols_.emplace_back(symbolVaddr, 1u, "resolved_func", filePath);
    symbolsFile->textExecVaddrFileOffset_ = symbolVaddr;
    symbolsFile->textExecVaddr_ = symbolVaddr;
    runtime_->symbolsFiles_[filePath] = std::move(symbolsFile);
    runtime_->GetThread(1, 2).CreateMapItem(filePath, mapBegin, 0x1000, 0);
    std::vector<CallFrame> callFrames = {CallFrame(mapBegin + symbolVaddr)};

    EXPECT_TRUE(runtime_->GetSymbolName(1, 2, callFrames, 0, true, SymbolType::NATIVE_SYMBOL));

    ASSERT_EQ(callFrames.size(), 1u);
    EXPECT_EQ(callFrames[0].symbolName_, "resolved_func");
}

HWTEST_F(VirtualRuntimeExtTest, GetSymbolReturnsResolvedUserSymbolWithCompressedIds, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_string_compressed(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    constexpr uint64_t mapBegin = 0xA000;
    constexpr uint64_t symbolVaddr = 0x100;
    const std::string filePath = "direct_user_symbol.so";
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = filePath;
    symbolsFile->symbols_.emplace_back(symbolVaddr, 0x20, "direct_func", filePath);
    symbolsFile->textExecVaddrFileOffset_ = symbolVaddr;
    symbolsFile->textExecVaddr_ = symbolVaddr;
    runtime_->symbolsFiles_[filePath] = std::move(symbolsFile);
    runtime_->GetThread(1, 2).CreateMapItem(filePath, mapBegin, 0x1000, 0);
    CallFrame callFrame(mapBegin + symbolVaddr);

    DfxSymbol symbol = GetSymbolForTest(*runtime_, callFrame, 1, 2, PERF_CONTEXT_USER);

    EXPECT_TRUE(symbol.IsValid());
    EXPECT_EQ(symbol.symbolName_, "direct_func");
    EXPECT_GT(symbol.symbolId_, 0u);
    EXPECT_TRUE(callFrame.needReport_ & CALL_FRAME_REPORT);
    EXPECT_TRUE(callFrame.needReport_ & SYMBOL_NAME_ID_REPORT);
    EXPECT_TRUE(callFrame.needReport_ & FILE_PATH_ID_REPORT);
}

HWTEST_F(VirtualRuntimeExtTest, GetKernelSymbolReturnsResolvedSymbol, TestSize.Level0)
{
    constexpr uint64_t ip = 0x1800;
    const std::string module = "kernel_hit.so";
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = module;
    symbolsFile->symbols_.emplace_back(ip, 0x20, "kernel_func", module);
    runtime_->symbolsFiles_[module] = std::move(symbolsFile);
    std::vector<std::shared_ptr<DfxMap>> maps;
    maps.push_back(std::make_shared<DfxMap>(0x1000, 0x3000, 0, PROT_EXEC, module));
    auto& thread = runtime_->UpdateThread(1, 2, "kernel_thread");

    DfxSymbol symbol = runtime_->GetKernelSymbol(ip, maps, thread);

    EXPECT_TRUE(symbol.IsValid());
    EXPECT_EQ(symbol.GetName(), "kernel_func");
    EXPECT_EQ(symbol.taskVaddr_, ip);
}

HWTEST_F(VirtualRuntimeExtTest, UpdateSymbolCacheInsertsMissingIp, TestSize.Level0)
{
    HashList<uint64_t, DfxSymbol> cache(4);
    DfxSymbol symbol(0x8100, "cached_symbol");
    symbol.symbolName_ = "cached_symbol";

    runtime_->UpdateSymbolCache(0x8100, symbol, cache);

    ASSERT_EQ(cache.count(0x8100), 1u);
    EXPECT_EQ((*cache.find(0x8100)).symbolName_, "cached_symbol");
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoCreatesArktsMapWhenAsyncEnabled, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    const std::string path = "entry/src/main.abc";

    runtime_->HandleMapInfo({0x1000, 0x1000, PROT_EXEC, 0}, path, 11, 22);

    auto iter = runtime_->mapsCache_.find(0x1000);
    ASSERT_NE(iter, runtime_->mapsCache_.end());
    EXPECT_EQ(runtime_->arktsMapTree_.count(0x1000), 1u);
    EXPECT_NE(runtime_->userSpaceThreadMap_.find(22), runtime_->userSpaceThreadMap_.end());
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoReplacesFrontMapForSameBegin, TestSize.Level0)
{
    const std::string path = "replace_front.so";
    runtime_->processMaps_.push_back(std::make_shared<DfxMap>(0x100, 0x200, 0, PROT_READ, "seed.so"));
    runtime_->HandleMapInfo({0x2000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, path, 1, 2);
    runtime_->HandleMapInfo({0x2000, 0x800, MAP_FIXED | PROT_READ, 0x100}, path, 1, 2);

    auto iter = runtime_->mapsCache_.find(0x2000);
    ASSERT_NE(iter, runtime_->mapsCache_.end());
    auto maps = iter->second;
    ASSERT_NE(maps, nullptr);
    ASSERT_EQ(maps->GetMaps().size(), 1u);
    EXPECT_EQ(maps->GetMaps()[0]->begin, 0x2000u);
    EXPECT_EQ(maps->GetMaps()[0]->end, 0x2800u);
    EXPECT_EQ(maps->soEnd_, 0x2800u);
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoAppendsNonExecMapWithoutOfflineQueue, TestSize.Level0)
{
    const std::string path = "append_readonly.so";
    runtime_->processMaps_.push_back(std::make_shared<DfxMap>(0x100, 0x200, 0, PROT_READ, "seed.so"));
    runtime_->HandleMapInfo({0x3000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, path, 1, 2);
    runtime_->GetOfflineMaps().clear();
    runtime_->HandleMapInfo({0x4000, 0x1000, MAP_FIXED | PROT_READ, 0x1000}, path, 1, 2);

    auto iter = runtime_->mapsCache_.find(0x3000);
    ASSERT_NE(iter, runtime_->mapsCache_.end());
    auto maps = iter->second;
    ASSERT_NE(maps, nullptr);
    ASSERT_EQ(maps->GetMaps().size(), 2u);
    EXPECT_EQ(maps->GetMaps()[1]->prevMap, maps->GetMaps()[0]);
    EXPECT_TRUE(runtime_->GetOfflineMaps().empty());
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoAsyncRecognizesArktsPathKinds, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    runtime_->processMaps_.push_back(std::make_shared<DfxMap>(0x100, 0x200, 0, PROT_READ, "seed.so"));
    std::vector<std::string> paths = {
        "entry/module.hap", "[anon:ArkTS Code]", "stub.an", "feature.hsp", "entry/page.hqf"
    };

    uint64_t begin = 0x5000;
    for (const auto& path : paths) {
        runtime_->HandleMapInfo({begin, 0x1000, PROT_EXEC, 0}, path, 11, 22);
        EXPECT_EQ(runtime_->arktsMapTree_.count(begin), 1u);
        EXPECT_NE(runtime_->mapsCache_.find(begin), runtime_->mapsCache_.end());
        begin += 0x2000;
    }
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoAsyncFallsThroughForNativePath, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    runtime_->processMaps_.push_back(std::make_shared<DfxMap>(0x100, 0x200, 0, PROT_READ, "seed.so"));

    runtime_->HandleMapInfo({0xF000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, "native_only.so", 1, 2);

    EXPECT_EQ(runtime_->arktsMapTree_.count(0xF000), 0u);
    EXPECT_NE(runtime_->mapsCache_.find(0xF000), runtime_->mapsCache_.end());
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoAsyncSkipsDuplicateArktsBegin, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_async_stack_enable(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);
    const std::string path = "entry/duplicate.abc";

    constexpr uint64_t begin = 0x700000000000ULL;
    runtime_->HandleMapInfo({begin, 0x1000, PROT_EXEC, 0}, path, 1, 2);
    const size_t cacheSize = runtime_->mapsCache_.size();
    runtime_->HandleMapInfo({begin, 0x1000, PROT_EXEC, 0}, path, 1, 2);

    EXPECT_EQ(runtime_->mapsCache_.size(), cacheSize);
    EXPECT_EQ(runtime_->arktsMapTree_.count(begin), 1u);
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoSkipsThreadUpdateWhenFpUnwind, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_fp_unwind(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);

    runtime_->HandleMapInfo({0x11000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, "fp_unwind.so", 1, 2);

    EXPECT_NE(runtime_->mapsCache_.find(0x11000), runtime_->mapsCache_.end());
    EXPECT_TRUE(runtime_->userSpaceThreadMap_.empty());
    EXPECT_TRUE(runtime_->processMaps_.empty());
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoStartupModeParsesWithoutAppendingMap, TestSize.Level0)
{
    NativeHookConfig config;
    config.set_startup_mode(true);
    runtime_ = std::make_unique<VirtualRuntime>(config);

    runtime_->HandleMapInfo({0x12000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, "startup_mode.so", 1, 2);

    EXPECT_NE(runtime_->mapsCache_.find(0x12000), runtime_->mapsCache_.end());
    EXPECT_FALSE(runtime_->processMaps_.empty());
    EXPECT_NE(runtime_->userSpaceThreadMap_.find(2), runtime_->userSpaceThreadMap_.end());
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoHapSegmentSetsPrevMap, TestSize.Level0)
{
    const std::string path = "module.hap";
    runtime_->processMaps_.push_back(std::make_shared<DfxMap>(0x100, 0x200, 0, PROT_READ, "seed.so"));
    runtime_->HandleMapInfo({0x13000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, path, 1, 2);
    runtime_->HandleMapInfo({0x14000, 0x1000, MAP_FIXED | PROT_EXEC, 0x1000}, path, 1, 2);

    auto iter = runtime_->mapsCache_.find(0x13000);
    ASSERT_NE(iter, runtime_->mapsCache_.end());
    auto maps = iter->second;
    ASSERT_EQ(maps->GetMaps().size(), 2u);
    EXPECT_EQ(maps->GetMaps()[1]->prevMap, maps->GetMaps()[0]);
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoSameBeginMultiSegmentFallsBackToAddMap, TestSize.Level0)
{
    const std::string path = "multi_replace.so";
    runtime_->processMaps_.push_back(std::make_shared<DfxMap>(0x100, 0x200, 0, PROT_READ, "seed.so"));
    runtime_->HandleMapInfo({0x16000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, path, 1, 2);
    runtime_->HandleMapInfo({0x17000, 0x1000, MAP_FIXED | PROT_READ, 0x1000}, path, 1, 2);
    runtime_->HandleMapInfo({0x16000, 0x800, MAP_FIXED | PROT_READ, 0x2000}, path, 1, 2);

    auto iter = runtime_->mapsCache_.find(0x16000);
    ASSERT_NE(iter, runtime_->mapsCache_.end());
    auto maps = iter->second;
    ASSERT_EQ(maps->GetMaps().size(), 3u);
    EXPECT_EQ(maps->GetMaps()[2]->begin, 0x16000u);
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoIgnoresMismatchedSegmentName, TestSize.Level0)
{
    const std::string path = "base_name.so";
    runtime_->processMaps_.push_back(std::make_shared<DfxMap>(0x100, 0x200, 0, PROT_READ, "seed.so"));
    runtime_->HandleMapInfo({0x19000, 0x1000, MAP_FIXED | PROT_EXEC, 0}, path, 1, 2);
    runtime_->HandleMapInfo({0x1A000, 0x1000, MAP_FIXED | PROT_READ, 0x1000}, "other_name.so", 1, 2);

    auto iter = runtime_->mapsCache_.find(0x19000);
    ASSERT_NE(iter, runtime_->mapsCache_.end());
    EXPECT_EQ(iter->second->GetMaps().size(), 1u);
    EXPECT_EQ(runtime_->mapsCache_.find(0x1A000), runtime_->mapsCache_.end());
}

HWTEST_F(VirtualRuntimeExtTest, HandleMapInfoIgnoresOrphanFixedOffsetSegment, TestSize.Level0)
{
    runtime_->HandleMapInfo({0x1B000, 0x1000, MAP_FIXED | PROT_EXEC, 0x1000}, "orphan_segment.so", 1, 2);

    EXPECT_TRUE(runtime_->mapsCache_.empty());
    ASSERT_EQ(runtime_->offlineMapAddr_.size(), 1u);
    EXPECT_EQ(runtime_->offlineMapAddr_[0], 0u);
}

HWTEST_F(VirtualRuntimeExtTest, FindMapReturnsNullForGapInsideSoRange, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0x20000, 0x21000, 0, PROT_EXEC, "gap.so");
    auto memMaps = std::make_shared<MemMaps>(1);
    memMaps->AddMap(map, true);
    memMaps->soEnd_ = 0x23000;
    runtime_->mapsCache_[map->begin] = memMaps;

    auto [maps, index] = runtime_->FindMap(0x22000);

    EXPECT_EQ(maps, nullptr);
    EXPECT_EQ(index, 0u);
}

HWTEST_F(VirtualRuntimeExtTest, TryEmitSmoMapsInfoSupportsProtoEncoder, TestSize.Level0)
{
    const std::string path = "proto_ready.so";
    {
        std::unique_lock<std::shared_mutex> lock(g_smoMapsLock);
        auto& entry = g_smoMaps[path];
        entry.initialized = true;
        entry.sent = false;
        entry.info.set_file_path(path);
        auto* mapEntry = entry.info.add_map_entries();
        mapEntry->set_start_addr(0x10);
        mapEntry->set_end_addr(0x20);
        mapEntry->set_so_index(4);
        auto* idEntry = entry.info.add_id_tlb_entries();
        idEntry->set_so_index(4);
        idEntry->set_name("proto_inner.so");
    }

    RandomWriteCtx ctx = MakeVirtualRuntimeEncoderCtx();
    OHOS::Developtools::Profiler::ProtoEncoder::BatchNativeHookData data(&ctx);
    runtime_->TryEmitSmoMapsInfo(data);

    EXPECT_GT(data.Finish(), 0);
    std::unique_lock<std::shared_mutex> lock(g_smoMapsLock);
    EXPECT_TRUE(g_smoMaps[path].sent);
}

HWTEST_F(VirtualRuntimeExtTest, GetUserSymbolUsesAbcSymbolLookup, TestSize.Level0)
{
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    constexpr uint64_t mapBegin = 0x24000;
    constexpr uint64_t ip = mapBegin + 0x80;
    constexpr uint32_t filePathId = 42;
    const std::string filePath = "entry/module.abc";
    auto map = std::make_shared<DfxMap>(mapBegin, mapBegin + 0x1000, 0, PROT_EXEC, filePath);
    auto memMaps = std::make_shared<MemMaps>(filePathId);
    memMaps->AddMap(map, true);
    memMaps->soEnd_ = map->end;
    runtime_->mapsCache_[mapBegin] = memMaps;

    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, filePath, getpid());
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = filePath;
    symbolsFile->SetBoolValue(true);
    DfxSymbol cached(ip, 0x20, "abc_func", filePath);
    cached.symbolName_ = "abc_func";
    symbolsFile->symbolsMap_.emplace(ip, cached);
    runtime_->symbolsFiles_[filePath] = std::move(symbolsFile);
    auto& thread = runtime_->UpdateThread(1, 2, "abc_thread");

    DfxSymbol symbol = runtime_->GetUserSymbol(ip, memMaps, 0, thread);

    EXPECT_TRUE(symbol.IsValid());
    EXPECT_EQ(symbol.symbolName_, "abc_func");
    EXPECT_EQ(symbol.filePathId_, filePathId);
}

/*
 * @tc.name: FindArktsMapByAddrPopulatesPageCacheOnMiss
 * @tc.desc: verify arkts page cache is filled on first lookup (cache miss branch).
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindArktsMapByAddrPopulatesPageCacheOnMiss, TestSize.Level0)
{
    constexpr uint32_t filePathId = 7;
    auto map = std::make_shared<DfxMap>(0x3000, 0x4000, 0, PROT_EXEC, "entry.abc");
    runtime_->arktsMapTree_[map->begin] = {map, filePathId};
    ASSERT_TRUE(runtime_->arktsMapCache_.empty());

    auto found = runtime_->FindArktsMapByAddr(0x3008);

    EXPECT_EQ(found, map);
    ASSERT_EQ(runtime_->arktsMapCache_.size(), 1u);
    auto cached = runtime_->arktsMapCache_[0x3];
    EXPECT_EQ(cached.map, map);
    EXPECT_EQ(cached.filePathId, filePathId);
}

/*
 * @tc.name: FindArktsMapByAddrServesPositiveCacheOnRepeatedCall
 * @tc.desc: verify repeated lookups in the same page hit the cache without re-running upper_bound.
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindArktsMapByAddrServesPositiveCacheOnRepeatedCall, TestSize.Level0)
{
    constexpr uint32_t filePathId = 9;
    auto map = std::make_shared<DfxMap>(0x5000, 0x6000, 0, PROT_EXEC, "entry.abc");
    runtime_->arktsMapTree_[map->begin] = {map, filePathId};

    auto first = runtime_->FindArktsMapByAddr(0x5010);
    auto second = runtime_->FindArktsMapByAddr(0x5080);

    EXPECT_EQ(first, map);
    EXPECT_EQ(second, map);
    EXPECT_EQ(runtime_->arktsMapCache_.size(), 1u);
    EXPECT_EQ(runtime_->arktsMapCache_[0x5].map, map);
    EXPECT_EQ(runtime_->arktsMapCache_[0x5].filePathId, filePathId);
}

/*
 * @tc.name: FindArktsMapByAddrCachesNegativeResultForPageWithoutMap
 * @tc.desc: verify a page with no arkts map is cached as negative and reused on the next lookup.
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindArktsMapByAddrCachesNegativeResultForPageWithoutMap, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0x7000, 0x8000, 0, PROT_EXEC, "entry.abc");
    runtime_->arktsMapTree_[map->begin] = {map, 0};

    EXPECT_EQ(runtime_->FindArktsMapByAddr(0x9000), nullptr);
    ASSERT_EQ(runtime_->arktsMapCache_.size(), 1u);
    EXPECT_EQ(runtime_->arktsMapCache_[0x9].map, nullptr);

    EXPECT_EQ(runtime_->FindArktsMapByAddr(0x9abc), nullptr);
    EXPECT_EQ(runtime_->arktsMapCache_.size(), 1u);
    EXPECT_EQ(runtime_->arktsMapCache_[0x9].map, nullptr);
}

/*
 * @tc.name: FindArktsMapByAddrReturnsNullptrWhenIpBeforeFirstMap
 * @tc.desc: verify upper_bound == begin branch caches a negative result.
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindArktsMapByAddrReturnsNullptrWhenIpBeforeFirstMap, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0xB000, 0xC000, 0, PROT_EXEC, "entry.abc");
    runtime_->arktsMapTree_[map->begin] = {map, 3};

    EXPECT_EQ(runtime_->FindArktsMapByAddr(0xA000), nullptr);
    ASSERT_EQ(runtime_->arktsMapCache_.size(), 1u);
    EXPECT_EQ(runtime_->arktsMapCache_[0xA].map, nullptr);
}

/*
 * @tc.name: FindArktsMapByAddrReturnsNullptrWhenTreeEmpty
 * @tc.desc: verify the empty-tree branch still caches a negative result.
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, FindArktsMapByAddrReturnsNullptrWhenTreeEmpty, TestSize.Level0)
{
    EXPECT_TRUE(runtime_->arktsMapTree_.empty());

    EXPECT_EQ(runtime_->FindArktsMapByAddr(0x1234), nullptr);

    ASSERT_EQ(runtime_->arktsMapCache_.size(), 1u);
    EXPECT_EQ(runtime_->arktsMapCache_[0x1].map, nullptr);
}

/*
 * @tc.name: ClearMapsClearsArktsMapPageCache
 * @tc.desc: verify ClearMaps drops the arkts page cache together with the map tree.
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, ClearMapsClearsArktsMapPageCache, TestSize.Level0)
{
    auto map = std::make_shared<DfxMap>(0xD000, 0xE000, 0, PROT_EXEC, "entry.abc");
    runtime_->arktsMapTree_[map->begin] = {map, 5};
    runtime_->FindArktsMapByAddr(0xD010);
    ASSERT_EQ(runtime_->arktsMapCache_.size(), 1u);

    runtime_->ClearMaps();

    EXPECT_TRUE(runtime_->arktsMapCache_.empty());
    EXPECT_TRUE(runtime_->arktsMapTree_.empty());
}

/*
 * @tc.name: SymbolizeFrameCachesResolvedSymbolAndShortCircuits
 * @tc.desc: verify a miss resolves via GetSymbol(knownMap) and a subsequent frame short-circuits the cache.
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, SymbolizeFrameCachesResolvedSymbolAndShortCircuits, TestSize.Level0)
{
    constexpr uint64_t mapBegin = 0x24000;
    constexpr uint64_t ip = mapBegin + 0x80;
    constexpr uint32_t filePathId = 42;
    const std::string filePath = "entry/module.abc";
    auto map = std::make_shared<DfxMap>(mapBegin, mapBegin + 0x1000, 0, PROT_EXEC, filePath);
    runtime_->arktsMapTree_[map->begin] = {map, filePathId};

    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, filePath, getpid());
    ASSERT_NE(symbolsFile, nullptr);
    symbolsFile->filePath_ = filePath;
    symbolsFile->SetBoolValue(true);
    DfxSymbol cached(ip, 0x20, "abc_func", filePath);
    cached.symbolName_ = "abc_func";
    symbolsFile->symbolsMap_.emplace(ip, cached);
    runtime_->symbolsFiles_[filePath] = std::move(symbolsFile);
    runtime_->UpdateThread(1, 2, "abc_thread");

    auto cacheKey = std::pair<uint64_t, uint32_t>(ip, filePathId);
    ASSERT_EQ(runtime_->userSymbolCache_.find(cacheKey), runtime_->userSymbolCache_.end());

    CallFrame first(ip);
    EXPECT_TRUE(runtime_->SymbolizeFrame(first, 1, 2));
    EXPECT_TRUE(first.isJsFrame_);
    EXPECT_EQ(first.symbolName_, "abc_func");
    EXPECT_NE(runtime_->userSymbolCache_.find(cacheKey), runtime_->userSymbolCache_.end());

    CallFrame second(ip);
    EXPECT_TRUE(runtime_->SymbolizeFrame(second, 1, 2));
    EXPECT_TRUE(second.isJsFrame_);
    EXPECT_EQ(second.symbolName_, "abc_func");
}

/*
 * @tc.name: GetSymbolUsesKnownMapFilePathIdForCacheLookup
 * @tc.desc: verify GetSymbol uses knownFilePathId and skips FindMap when knownMap is provided.
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeExtTest, GetSymbolUsesKnownMapFilePathIdForCacheLookup, TestSize.Level0)
{
    constexpr uint64_t ip = 0x32080;
    constexpr uint32_t filePathId = 77;
    auto map = std::make_shared<DfxMap>(0x32000, 0x33000, 0, PROT_EXEC, "entry/known.abc");
    runtime_->arktsMapTree_[map->begin] = {map, filePathId};

    DfxSymbol cached(ip, "known_func");
    cached.symbolName_ = "known_func";
    cached.filePathId_ = filePathId;
    cached.symbolId_ = 9;
    runtime_->userSymbolCache_[VirtualRuntime::SymbolCacheKey(ip, filePathId)] = cached;

    DfxSymbol symbol;
    CallFrame callFrame(ip);
    runtime_->GetSymbol(symbol, callFrame, 1, 2, PERF_CONTEXT_MAX, map, filePathId, false);

    EXPECT_EQ(symbol.symbolName_, "known_func");
    EXPECT_EQ(symbol.symbolId_, cached.symbolId_);
    EXPECT_EQ(callFrame.needReport_, 0u);
}
}
}
}
