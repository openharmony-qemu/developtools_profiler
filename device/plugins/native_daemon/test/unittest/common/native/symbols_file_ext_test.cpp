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
#include <fcntl.h>
#include <random>
#include <unistd.h>
#include "debug_logger.h"
#include "symbols_file.h"
#include "common.h"

using namespace testing::ext;
using namespace std;
using namespace OHOS::HiviewDFX;

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
class SymbolsFileExtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    default_random_engine rnd_;
};

class ScopedLogEnabled {
public:
    ScopedLogEnabled() : savedLogDisabled_(DebugLogger::logDisabled_)
    {
        DebugLogger::logDisabled_ = false;
    }

    ~ScopedLogEnabled()
    {
        DebugLogger::logDisabled_ = savedLogDisabled_;
    }

private:
    bool savedLogDisabled_ = true;
};

void SymbolsFileExtTest::SetUpTestCase() {}

void SymbolsFileExtTest::TearDownTestCase() {}

void SymbolsFileExtTest::SetUp() {}

void SymbolsFileExtTest::TearDown() {}

/**
 * @tc.name: SymbolsLoaded
 * @tc.desc: Test SymbolsLoaded method for different symbol file types
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, SymbolsLoaded, TestSize.Level0)
{
    auto elfSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    EXPECT_EQ(elfSymbols->SymbolsLoaded(), false);
    
    auto kernelSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_KERNEL_FILE);
    EXPECT_EQ(kernelSymbols->SymbolsLoaded(), false);
    
    auto javaSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_JAVA_FILE);
    EXPECT_EQ(javaSymbols->SymbolsLoaded(), false);
    
    auto jsSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_JS_FILE);
    EXPECT_EQ(jsSymbols->SymbolsLoaded(), false);
    
    auto unknowSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    EXPECT_EQ(unknowSymbols->SymbolsLoaded(), false);
}

/**
 * @tc.name: GetElfFile
 * @tc.desc: Test GetElfFile method for ElfFileSymbols
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, GetElfFile, TestSize.Level0)
{
    auto elfSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    EXPECT_EQ(elfSymbols->GetElfFile(), nullptr);
}

/**
 * @tc.name: IsAbcAndSetBoolValue
 * @tc.desc: Test IsAbc and SetBoolValue methods for HapFileSymbols
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, IsAbcAndSetBoolValue, TestSize.Level0)
{
    auto hapSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, "test.hap", getpid());
    EXPECT_EQ(hapSymbols->IsAbc(), false);
    
    // Test SetBoolValue
    hapSymbols->SetBoolValue(true);
    EXPECT_EQ(hapSymbols->IsAbc(), true);
    
    hapSymbols->SetBoolValue(false);
    EXPECT_EQ(hapSymbols->IsAbc(), false);
}

/**
 * @tc.name: CheckPathReadable
 * @tc.desc: Test CheckPathReadable method with different paths
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, CheckPathReadable, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    // Test with non-existent path
    EXPECT_EQ(symbolsFile->CheckPathReadable("/path/to/non/existent/file"), false);
    
    // Test with current directory (should be readable)
    EXPECT_EQ(symbolsFile->CheckPathReadable("."), true);
}

/**
 * @tc.name: SearchReadableFile
 * @tc.desc: Test SearchReadableFile method with different search paths
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, SearchReadableFile, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    std::vector<std::string> searchPaths;
    searchPaths.push_back(".");
    searchPaths.push_back("/path/to/non/existent/dir");
    
    // Test with current directory (should find ".")
    std::string result = symbolsFile->SearchReadableFile(searchPaths, ".");
    EXPECT_NE(result.empty(), true);
    
    // Test with non-existent file
    result = symbolsFile->SearchReadableFile(searchPaths, "non_existent_file.txt");
    EXPECT_EQ(result.empty(), true);
}

/**
 * @tc.name: SearchReadableFileRejectsEmptyTarget
 * @tc.desc: Test SearchReadableFile returns empty when filePath is empty
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, SearchReadableFileRejectsEmptyTarget, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    std::vector<std::string> searchPaths = {"."};
    EXPECT_TRUE(symbolsFile->SearchReadableFile(searchPaths, "").empty());
}

/**
 * @tc.name: AdjustSymbols
 * @tc.desc: Test AdjustSymbols method with different symbol lists
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, AdjustSymbols, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    // Test with empty symbols list
    symbolsFile->symbols_.clear();
    symbolsFile->AdjustSymbols();
    EXPECT_EQ(symbolsFile->symbols_.empty(), true);
    
    // Test with single symbol
    symbolsFile->symbols_.emplace_back(0x1000, 0x100, "test_symbol", "test_module");
    symbolsFile->AdjustSymbols();
    EXPECT_EQ(symbolsFile->symbols_.size(), 1u);
    
    // Test with multiple symbols (should be sorted)
    symbolsFile->symbols_.clear();
    symbolsFile->symbols_.emplace_back(0x3000, 0x100, "symbol3", "test_module");
    symbolsFile->symbols_.emplace_back(0x1000, 0x100, "symbol1", "test_module");
    symbolsFile->symbols_.emplace_back(0x2000, 0x100, "symbol2", "test_module");
    
    symbolsFile->AdjustSymbols();
    EXPECT_EQ(symbolsFile->symbols_.size(), 3u);
    EXPECT_LT(symbolsFile->symbols_[0].funcVaddr_, symbolsFile->symbols_[1].funcVaddr_);
    EXPECT_LT(symbolsFile->symbols_[1].funcVaddr_, symbolsFile->symbols_[2].funcVaddr_);
}

/**
 * @tc.name: AdjustSymbolsDeduplicatesAndUpdatesRange
 * @tc.desc: Test AdjustSymbols removes duplicate vaddr entries and updates range/index.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, AdjustSymbolsDeduplicatesAndUpdatesRange, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    symbolsFile->symbols_.emplace_back(0x3000, 0x100, "symbol3", "test_module");
    symbolsFile->symbols_.emplace_back(0x1000, 0x100, "symbol1", "test_module");
    symbolsFile->symbols_.emplace_back(0x1000, 0x100, "symbol1_dup", "test_module");
    symbolsFile->symbols_.emplace_back(0x2000, 0x100, "symbol2", "test_module");

    symbolsFile->AdjustSymbols();

    ASSERT_EQ(symbolsFile->symbols_.size(), 3u);
    EXPECT_EQ(symbolsFile->symbols_[0].index_, 0u);
    EXPECT_EQ(symbolsFile->symbols_[1].index_, 1u);
    EXPECT_EQ(symbolsFile->symbols_[2].index_, 2u);
    EXPECT_EQ(symbolsFile->textExecVaddrRange_, 0x2000U);
}

/**
 * @tc.name: SortMatchedSymbols
 * @tc.desc: Test SortMatchedSymbols method
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, SortMatchedSymbols, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    // Add symbols
    symbolsFile->symbols_.emplace_back(0x3000, 0x100, "symbol3", "test_module");
    symbolsFile->symbols_.emplace_back(0x1000, 0x100, "symbol1", "test_module");
    symbolsFile->symbols_.emplace_back(0x2000, 0x100, "symbol2", "test_module");
    
    // Mark all as matched
    for (auto& symbol : symbolsFile->symbols_) {
        symbol.matched_ = true;
        symbolsFile->matchedSymbols_.push_back(&symbol);
    }
    
    // Sort matched symbols
    symbolsFile->SortMatchedSymbols();
    
    // Check if sorted
    EXPECT_EQ(symbolsFile->matchedSymbols_.size(), 3u);
    EXPECT_LT(symbolsFile->matchedSymbols_[0]->funcVaddr_, symbolsFile->matchedSymbols_[1]->funcVaddr_);
    EXPECT_LT(symbolsFile->matchedSymbols_[1]->funcVaddr_, symbolsFile->matchedSymbols_[2]->funcVaddr_);
}

/**
 * @tc.name: GetMatchedSymbols
 * @tc.desc: Test GetMatchedSymbols method
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, GetMatchedSymbols, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    // Test with empty matched symbols
    const auto& matchedSymbols1 = symbolsFile->GetMatchedSymbols();
    EXPECT_EQ(matchedSymbols1.empty(), true);
    
    // Add some matched symbols
    symbolsFile->symbols_.emplace_back(0x1000, 0x100, "symbol1", "test_module");
    symbolsFile->symbols_.emplace_back(0x2000, 0x100, "symbol2", "test_module");
    
    symbolsFile->symbols_[0].matched_ = true;
    symbolsFile->matchedSymbols_.push_back(&symbolsFile->symbols_[0]);
    
    symbolsFile->symbols_[1].matched_ = true;
    symbolsFile->matchedSymbols_.push_back(&symbolsFile->symbols_[1]);
    
    // Test with matched symbols
    const auto& matchedSymbols2 = symbolsFile->GetMatchedSymbols();
    EXPECT_EQ(matchedSymbols2.size(), 2u);
}

/**
 * @tc.name: GetSymbols
 * @tc.desc: Test GetSymbols method
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, GetSymbols, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    // Test with empty symbols
    const auto& symbols1 = symbolsFile->GetSymbols();
    EXPECT_EQ(symbols1.empty(), true);
    
    // Add some symbols
    symbolsFile->symbols_.emplace_back(0x1000, 0x100, "symbol1", "test_module");
    symbolsFile->symbols_.emplace_back(0x2000, 0x100, "symbol2", "test_module");
    
    // Test with symbols
    const auto& symbols2 = symbolsFile->GetSymbols();
    EXPECT_EQ(symbols2.size(), 2u);
}

/**
 * @tc.name: SetMapsInfo
 * @tc.desc: Test SetMapsInfo method
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, SetMapsInfo, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    EXPECT_EQ(symbolsFile->map_, nullptr);
    
    // Create a dummy map
    auto map = std::make_shared<DfxMap>();
    map->name = "test_map";
    
    // Test SetMapsInfo
    symbolsFile->SetMapsInfo(map);
    EXPECT_NE(symbolsFile->map_, nullptr);
    EXPECT_EQ(symbolsFile->map_->name, "test_map");
}

/**
 * @tc.name: KernelModuleSymbolsLoad
 * @tc.desc: Test KernelModuleSymbols LoadSymbols
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, KernelModuleSymbolsLoad, TestSize.Level0)
{
    auto moduleSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_KERNEL_MODULE_FILE, "test_module");
    EXPECT_EQ(moduleSymbols->LoadSymbols(nullptr, ""), false);
}

/**
 * @tc.name: JavaFileSymbolsLoad
 * @tc.desc: Test JavaFileSymbols LoadSymbols
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, JavaFileSymbolsLoad, TestSize.Level0)
{
    auto javaSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_JAVA_FILE, "test.jar");
    EXPECT_EQ(javaSymbols->LoadSymbols(nullptr, ""), false);
}

/**
 * @tc.name: JSFileSymbolsLoad
 * @tc.desc: Test JSFileSymbols LoadSymbols
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, JSFileSymbolsLoad, TestSize.Level0)
{
    auto jsSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_JS_FILE, "test.js");
    EXPECT_EQ(jsSymbols->LoadSymbols(nullptr, ""), false);
}

/**
 * @tc.name: HapFileSymbolsLoad
 * @tc.desc: Test HapFileSymbols LoadSymbols and LoadDebugInfo
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, HapFileSymbolsLoad, TestSize.Level0)
{
    auto hapSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, "test.hap", getpid());
    
    // Test LoadSymbols
    auto map = std::make_shared<DfxMap>();
    map->name = "test_map";
    EXPECT_EQ(hapSymbols->LoadSymbols(map, ""), true);
    
    // Test LoadDebugInfo
    EXPECT_EQ(hapSymbols->LoadDebugInfo(map, ""), true);
}

/**
 * @tc.name: HapFileSymbolsLoadRecognizesArkExtensions
 * @tc.desc: Test HapFileSymbols treats hap-like extensions as ark bytecode in recording mode.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, HapFileSymbolsLoadRecognizesArkExtensions, TestSize.Level0)
{
    const bool previousRecording = SymbolsFile::onRecording_;
    SymbolsFile::onRecording_ = true;
    const std::vector<std::string> suffixes = {"demo.hap", "demo.hsp", "demo.hqf", "demo.abc"};
    auto map = std::make_shared<DfxMap>();
    map->name = "demo.map";
    for (const auto& suffix : suffixes) {
        auto hapSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, suffix, getpid());
        EXPECT_TRUE(hapSymbols->LoadSymbols(map, ""));
        EXPECT_TRUE(hapSymbols->IsAbc());
    }
    SymbolsFile::onRecording_ = previousRecording;
}

/**
 * @tc.name: GetSymbolWithPcAndMap
 * @tc.desc: Test GetSymbolWithPcAndMap method
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, GetSymbolWithPcAndMap, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    // Test with null map
    auto symbol1 = symbolsFile->GetSymbolWithPcAndMap(0x1000, nullptr);
    EXPECT_EQ(symbol1.IsValid(), false);
    
    // Test with map
    auto map = std::make_shared<DfxMap>();
    map->name = "test_map";
    auto symbol2 = symbolsFile->GetSymbolWithPcAndMap(0x1000, map);
    EXPECT_EQ(symbol2.IsValid(), false);
}

/**
 * @tc.name: HapFileSymbolsGetSymbolWithPcAndMapCoversArkExtensions
 * @tc.desc: Test HapFileSymbols ark frame path for hap-like map suffixes.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, HapFileSymbolsGetSymbolWithPcAndMapCoversArkExtensions, TestSize.Level0)
{
    const std::vector<std::string> suffixes = {"demo.hap", "demo.hsp", "demo.hqf", "demo.abc"};
    for (const auto& suffix : suffixes) {
        auto hapSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, suffix, getpid());
        auto map = std::make_shared<DfxMap>();
        map->name = suffix;
        map->begin = 0x1000;
        map->offset = 0x20;
        hapSymbols->SetMapsInfo(map);
        hapSymbols->SetBoolValue(true);
        EXPECT_FALSE(hapSymbols->GetSymbolWithPcAndMap(0x1010, map).IsValid());
    }
}

/**
 * @tc.name: ExportSymbolToFileFormatEmpty
 * @tc.desc: Test ExportSymbolToFileFormat with empty symbols
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, ExportSymbolToFileFormatEmpty, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    symbolsFile->filePath_ = "test_file";
    symbolsFile->symbolFileType_ = SYMBOL_ELF_FILE;
    symbolsFile->textExecVaddrFileOffset_ = 0x1000;
    symbolsFile->buildId_ = "test_build_id";
    
    SymbolFileStruct sfs;
    symbolsFile->ExportSymbolToFileFormat(sfs);
    
    EXPECT_EQ(sfs.filePath_, "test_file");
    EXPECT_EQ(sfs.symbolType_, SYMBOL_ELF_FILE);
    EXPECT_EQ(sfs.textExecVaddrFileOffset_, 0x1000);
    EXPECT_EQ(sfs.buildId_, "test_build_id");
    EXPECT_EQ(sfs.symbolStructs_.empty(), true);
}

/**
 * @tc.name: LoadSymbolsFromSavedEmpty
 * @tc.desc: Test LoadSymbolsFromSaved with empty SymbolFileStruct
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, LoadSymbolsFromSavedEmpty, TestSize.Level0)
{
    SymbolFileStruct sfs;
    sfs.filePath_ = "test_file";
    sfs.symbolType_ = SYMBOL_ELF_FILE;
    sfs.textExecVaddrFileOffset_ = 0x1000;
    sfs.buildId_ = "test_build_id";
    
    auto symbolsFile = SymbolsFile::LoadSymbolsFromSaved(sfs);
    EXPECT_NE(symbolsFile, nullptr);
    EXPECT_EQ(symbolsFile->filePath_, "test_file");
    EXPECT_EQ(symbolsFile->symbolFileType_, SYMBOL_ELF_FILE);
    EXPECT_EQ(symbolsFile->textExecVaddrFileOffset_, 0x1000);
    EXPECT_EQ(symbolsFile->GetBuildId(), "test_build_id");
    EXPECT_EQ(symbolsFile->GetSymbols().empty(), true);
}

/**
 * @tc.name: UpdateBuildIdIfMatchEdgeCases
 * @tc.desc: Test UpdateBuildIdIfMatch with edge cases
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, UpdateBuildIdIfMatchEdgeCases, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    // Test with empty buildId_ and empty input
    EXPECT_EQ(symbolsFile->UpdateBuildIdIfMatch(""), true);
    EXPECT_EQ(symbolsFile->GetBuildId(), "");
    
    // Test with empty buildId_ and non-empty input
    EXPECT_EQ(symbolsFile->UpdateBuildIdIfMatch("build_id_1"), true);
    EXPECT_EQ(symbolsFile->GetBuildId(), "build_id_1");
    
    // Test with matching buildId
    EXPECT_EQ(symbolsFile->UpdateBuildIdIfMatch("build_id_1"), true);
    EXPECT_EQ(symbolsFile->GetBuildId(), "build_id_1");
    
    // Test with non-matching buildId
    EXPECT_EQ(symbolsFile->UpdateBuildIdIfMatch("build_id_2"), false);
    EXPECT_EQ(symbolsFile->GetBuildId(), "build_id_1");
}

/**
 * @tc.name: FindSymbolFileEdgeCases
 * @tc.desc: Test FindSymbolFile with edge cases
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, FindSymbolFileEdgeCases, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    
    std::vector<std::string> searchPaths;
    
    // Test with empty symbolFilePath
    EXPECT_EQ(symbolsFile->FindSymbolFile(searchPaths, ""), "");
    
    // Test with non-existent file
    EXPECT_EQ(symbolsFile->FindSymbolFile(searchPaths, "non_existent_file.txt"), "");
}

/**
 * @tc.name: FindSymbolFileUsesBasenameFallback
 * @tc.desc: Test FindSymbolFile falls back to basename when full path misses search roots.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, FindSymbolFileUsesBasenameFallback, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    char tempPath[] = "/data/local/tmp/symbols_ext_XXXXXX";
    int fd = mkstemp(tempPath);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(close(fd), 0);

    std::string fullPath = tempPath;
    std::string fileName = fullPath.substr(fullPath.find_last_of('/') + 1);
    std::vector<std::string> searchPaths = {"/data/local/tmp"};

    EXPECT_EQ(symbolsFile->FindSymbolFile(searchPaths, "/system/lib/" + fileName), fullPath);
    EXPECT_EQ(unlink(tempPath), 0);
}

/**
 * @tc.name: FindSymbolFileHonorsRecordingFlagForDirectPath
 * @tc.desc: Test FindSymbolFile direct path lookup depends on recording mode.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, FindSymbolFileHonorsRecordingFlagForDirectPath, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    char tempPath[] = "/data/local/tmp/symbols_recording_XXXXXX";
    int fd = mkstemp(tempPath);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(close(fd), 0);

    const bool previousRecording = SymbolsFile::onRecording_;
    SymbolsFile::onRecording_ = false;
    EXPECT_TRUE(symbolsFile->FindSymbolFile({}, tempPath).empty());

    SymbolsFile::onRecording_ = true;
    EXPECT_EQ(symbolsFile->FindSymbolFile({}, tempPath), std::string(tempPath));

    SymbolsFile::onRecording_ = previousRecording;
    EXPECT_EQ(unlink(tempPath), 0);
}

/**
 * @tc.name: FindSymbolFileReturnsHapContainerPath
 * @tc.desc: Test FindSymbolFile returns hap container path for embedded entries.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, FindSymbolFileReturnsHapContainerPath, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    EXPECT_EQ(symbolsFile->FindSymbolFile({}, "/data/local/tmp/demo.hap!ets/modules.abc"),
        "/data/local/tmp/demo.hap");
}

/**
 * @tc.name: CreateSymbolsFileByPathRecognizesKinds
 * @tc.desc: Test CreateSymbolsFile chooses symbol kind by file path.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, CreateSymbolsFileByPathRecognizesKinds, TestSize.Level0)
{
    auto kernelSymbols = SymbolsFile::CreateSymbolsFile(KERNEL_MMAP_NAME);
    auto moduleSymbols = SymbolsFile::CreateSymbolsFile("sample.ko");
    auto arkJsSymbols = SymbolsFile::CreateSymbolsFile("entry.hsp", getpid());
    auto arkwebSymbols = SymbolsFile::CreateSymbolsFile("[anon:v8 runtime]", getpid());
    auto elfSymbols = SymbolsFile::CreateSymbolsFile("/system/lib/libsample.so");

    EXPECT_EQ(kernelSymbols->symbolFileType_, SYMBOL_KERNEL_FILE);
    EXPECT_EQ(moduleSymbols->symbolFileType_, SYMBOL_KERNEL_MODULE_FILE);
    EXPECT_EQ(arkJsSymbols->symbolFileType_, SYMBOL_HAP_FILE);
    EXPECT_EQ(arkwebSymbols->symbolFileType_, SYMBOL_ARKWEB_V8_FILE);
    EXPECT_EQ(elfSymbols->symbolFileType_, SYMBOL_ELF_FILE);
}

/**
 * @tc.name: GetSymbolWithVaddrMarksMatchedOnce
 * @tc.desc: Test GetSymbolWithVaddr records matched symbol only once.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, GetSymbolWithVaddrMarksMatchedOnce, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    symbolsFile->symbols_.emplace_back(0x1000, 0x20, "symbol", "module");

    EXPECT_TRUE(symbolsFile->GetSymbolWithVaddr(0x1008).IsValid());
    EXPECT_TRUE(symbolsFile->GetSymbolWithVaddr(0x100C).IsValid());
    ASSERT_EQ(symbolsFile->matchedSymbols_.size(), 1u);
    EXPECT_EQ(symbolsFile->matchedSymbols_[0]->funcVaddr_, 0x1000u);
}

/**
 * @tc.name: GetSymbolWithVaddrReturnsInvalidForGap
 * @tc.desc: Test GetSymbolWithVaddr keeps gap addresses unmatched.
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, GetSymbolWithVaddrReturnsInvalidForGap, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    symbolsFile->symbols_.emplace_back(0x1000, 0x10, "symbol1", "module");
    symbolsFile->symbols_.emplace_back(0x2000, 0x10, "symbol2", "module");
    symbolsFile->AdjustSymbols();

    EXPECT_FALSE(symbolsFile->GetSymbolWithVaddr(0x1500).IsValid());
    EXPECT_TRUE(symbolsFile->matchedSymbols_.empty());
}

/**
 * @tc.name: GetVaddrInSymbolsForKernel
 * @tc.desc: Test GetVaddrInSymbols for KernelSymbols
 * @tc.type: FUNC
 */
HWTEST_F(SymbolsFileExtTest, GetVaddrInSymbolsForKernel, TestSize.Level0)
{
    auto kernelSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_KERNEL_FILE);

    // Kernel symbols should return ip directly
    EXPECT_EQ(kernelSymbols->GetVaddrInSymbols(0x12345678, 0x87654321, 0x1000), 0x12345678U);
}

HWTEST_F(SymbolsFileExtTest, SearchReadableFileAcceptsTrailingSlashPath, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    char tempPath[] = "/data/local/tmp/symbols_trailing_XXXXXX";
    int fd = mkstemp(tempPath);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(close(fd), 0);

    std::string fullPath = tempPath;
    std::string fileName = fullPath.substr(fullPath.find_last_of('/') + 1);
    std::vector<std::string> searchPaths = {"/data/local/tmp/"};

    EXPECT_EQ(symbolsFile->SearchReadableFile(searchPaths, fileName), fullPath);
    EXPECT_EQ(unlink(tempPath), 0);
}

HWTEST_F(SymbolsFileExtTest, FindSymbolFileUsesDefaultPath, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    char tempPath[] = "/data/local/tmp/symbols_default_XXXXXX";
    int fd = mkstemp(tempPath);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(close(fd), 0);
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE, tempPath);
    const bool previousRecording = SymbolsFile::onRecording_;
    SymbolsFile::onRecording_ = true;

    EXPECT_EQ(symbolsFile->FindSymbolFile({}, ""), std::string(tempPath));

    SymbolsFile::onRecording_ = previousRecording;
    EXPECT_EQ(unlink(tempPath), 0);
}

HWTEST_F(SymbolsFileExtTest, FindSymbolFileIgnoresNonHapEmbeddedPath, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);

    EXPECT_TRUE(symbolsFile->FindSymbolFile({}, "/data/local/tmp/demo.zip!entry.abc").empty());
}

HWTEST_F(SymbolsFileExtTest, LoadSymbolsFromSavedRestoresSymbolEntries, TestSize.Level0)
{
    SymbolFileStruct symbolFileStruct;
    symbolFileStruct.filePath_ = "saved_symbols";
    symbolFileStruct.symbolType_ = SYMBOL_ELF_FILE;
    symbolFileStruct.textExecVaddr_ = 0;
    symbolFileStruct.textExecVaddrFileOffset_ = 0;
    symbolFileStruct.symbolStructs_.emplace_back(0x2000, 0x10, "second");
    symbolFileStruct.symbolStructs_.emplace_back(0x1000, 0x10, "first");

    auto symbolsFile = SymbolsFile::LoadSymbolsFromSaved(symbolFileStruct);

    ASSERT_NE(symbolsFile, nullptr);
    ASSERT_EQ(symbolsFile->symbols_.size(), 2u);
    EXPECT_EQ(symbolsFile->symbols_[0].funcVaddr_, 0x1000u);
    EXPECT_EQ(symbolsFile->symbols_[1].funcVaddr_, 0x2000u);
}

HWTEST_F(SymbolsFileExtTest, HapFileSymbolsReportModeSkipsRuntimeLoad, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    const bool previousRecording = SymbolsFile::onRecording_;
    SymbolsFile::onRecording_ = false;
    auto hapSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, "report_mode.hap", getpid());
    auto map = std::make_shared<DfxMap>();
    map->name = "report_mode.hap";

    EXPECT_TRUE(hapSymbols->LoadSymbols(map, ""));
    EXPECT_TRUE(hapSymbols->LoadDebugInfo(map, ""));

    SymbolsFile::onRecording_ = previousRecording;
}

HWTEST_F(SymbolsFileExtTest, ArkwebV8SymbolsHandleNullMapAndCache, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    auto arkwebSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_ARKWEB_V8_FILE, "[anon:v8 runtime]", getpid());
    auto map = std::make_shared<DfxMap>();
    map->name = "[anon:v8 runtime]";
    constexpr uint64_t ip = 0x1234;

    EXPECT_FALSE(arkwebSymbols->LoadDebugInfo(nullptr, ""));
    EXPECT_TRUE(arkwebSymbols->LoadDebugInfo(map, ""));
    EXPECT_FALSE(arkwebSymbols->LoadSymbols(nullptr, ""));
    EXPECT_TRUE(arkwebSymbols->LoadSymbols(map, ""));
    EXPECT_FALSE(arkwebSymbols->GetSymbolWithPcAndMap(ip, nullptr).IsValid());

    arkwebSymbols->symbolsMap_.emplace(ip, DfxSymbol(ip, 0x20, "cached", "arkweb"));
    DfxSymbol cachedSymbol = arkwebSymbols->GetSymbolWithPcAndMap(ip, map);
    EXPECT_TRUE(cachedSymbol.IsValid());
    EXPECT_EQ(cachedSymbol.funcVaddr_, ip);
}

HWTEST_F(SymbolsFileExtTest, HapFileSymbolsReturnsCachedPcSymbol, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    auto hapSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, "cached.hap", getpid());
    auto map = std::make_shared<DfxMap>();
    map->name = "cached.hap";
    constexpr uint64_t ip = 0x4321;

    hapSymbols->symbolsMap_.emplace(ip, DfxSymbol(ip, 0x20, "cached_hap", "hap"));
    DfxSymbol cachedSymbol = hapSymbols->GetSymbolWithPcAndMap(ip, map);

    EXPECT_TRUE(cachedSymbol.IsValid());
    EXPECT_EQ(cachedSymbol.funcVaddr_, ip);
}

HWTEST_F(SymbolsFileExtTest, KernelSymbolsReportModeUsesVmlinuxFallback, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    const bool previousRecording = SymbolsFile::onRecording_;
    SymbolsFile::onRecording_ = false;
    auto kernelSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_KERNEL_FILE);
    auto moduleSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_KERNEL_MODULE_FILE, "missing_module");

    EXPECT_FALSE(kernelSymbols->LoadSymbols(nullptr, ""));
    EXPECT_FALSE(moduleSymbols->LoadSymbols(nullptr, ""));

    SymbolsFile::onRecording_ = previousRecording;
}

HWTEST_F(SymbolsFileExtTest, UpdateBuildIdIfMatchHandlesAllStates, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);

    EXPECT_TRUE(symbolsFile->UpdateBuildIdIfMatch(""));
    EXPECT_EQ(symbolsFile->GetBuildId(), "");
    EXPECT_TRUE(symbolsFile->UpdateBuildIdIfMatch("build-id-a"));
    EXPECT_EQ(symbolsFile->GetBuildId(), "build-id-a");
    EXPECT_TRUE(symbolsFile->UpdateBuildIdIfMatch("build-id-a"));
    EXPECT_FALSE(symbolsFile->UpdateBuildIdIfMatch("build-id-b"));
    EXPECT_EQ(symbolsFile->GetBuildId(), "build-id-a");
}

HWTEST_F(SymbolsFileExtTest, FindSymbolFileUsesSearchPathBaseName, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    char tempPath[] = "/data/local/tmp/symbols_base_XXXXXX";
    int fd = mkstemp(tempPath);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(close(fd), 0);
    std::string fullPath = tempPath;
    std::string fileName = fullPath.substr(fullPath.find_last_of('/') + 1);

    EXPECT_EQ(symbolsFile->FindSymbolFile({"/data/local/tmp"}, "/system/lib/" + fileName), fullPath);

    EXPECT_EQ(unlink(tempPath), 0);
}

HWTEST_F(SymbolsFileExtTest, SetSymbolsFilePathKeepsReadablePathsOnly, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    std::vector<std::string> searchPaths = {"/data/local/tmp", "/path/to/missing/symbols"};

    EXPECT_TRUE(symbolsFile->setSymbolsFilePath(searchPaths));
    ASSERT_EQ(symbolsFile->symbolsFileSearchPaths_.size(), 1u);
    EXPECT_EQ(symbolsFile->symbolsFileSearchPaths_[0], "/data/local/tmp");
}

HWTEST_F(SymbolsFileExtTest, ExportSymbolToFileFormatSortsMatchedSymbols, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE, "export_symbols");
    symbolsFile->symbols_.emplace_back(0x3000, 0x20, "third", "module");
    symbolsFile->symbols_.emplace_back(0x1000, 0x20, "first", "module");
    symbolsFile->symbols_.emplace_back(0x1000, 0x20, "duplicate", "module");
    symbolsFile->AdjustSymbols();

    EXPECT_TRUE(symbolsFile->GetSymbolWithVaddr(0x3004).IsValid());
    EXPECT_TRUE(symbolsFile->GetSymbolWithVaddr(0x1004).IsValid());
    SymbolFileStruct symbolFileStruct;
    symbolsFile->ExportSymbolToFileFormat(symbolFileStruct);

    ASSERT_EQ(symbolFileStruct.symbolStructs_.size(), 2u);
    EXPECT_EQ(symbolFileStruct.symbolStructs_[0].vaddr_, 0x1000u);
    EXPECT_EQ(symbolFileStruct.symbolStructs_[1].vaddr_, 0x3000u);
}

HWTEST_F(SymbolsFileExtTest, ElfFileSymbolsRejectsInvalidReadableFile, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    char tempPath[] = "/data/local/tmp/symbols_invalid_elf_XXXXXX";
    int fd = mkstemp(tempPath);
    ASSERT_GE(fd, 0);
    const char content[] = "not an elf";
    ASSERT_EQ(write(fd, content, sizeof(content)), static_cast<ssize_t>(sizeof(content)));
    ASSERT_EQ(close(fd), 0);
    const bool previousRecording = SymbolsFile::onRecording_;
    SymbolsFile::onRecording_ = true;
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE, tempPath);

    EXPECT_FALSE(symbolsFile->LoadSymbols(nullptr, ""));
    EXPECT_TRUE(symbolsFile->SymbolsLoaded());

    SymbolsFile::onRecording_ = previousRecording;
    EXPECT_EQ(unlink(tempPath), 0);
}

HWTEST_F(SymbolsFileExtTest, BaseSymbolsFileVirtualMethodsReturnDefaultsWithLog, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    SymbolsFile symbolsFile(SYMBOL_UNKNOW_FILE, "base_symbol");
    uint64_t sectionVaddr = 0;
    uint64_t sectionSize = 0;
    uint64_t sectionOffset = 0;

    EXPECT_FALSE(symbolsFile.LoadSymbols(nullptr, ""));
    EXPECT_TRUE(symbolsFile.SymbolsLoaded());
    EXPECT_FALSE(symbolsFile.LoadDebugInfo(nullptr, ""));
    EXPECT_FALSE(symbolsFile.GetSectionInfo(".text", sectionVaddr, sectionSize, sectionOffset));
#ifndef __arm__
    EXPECT_FALSE(symbolsFile.GetHDRSectionInfo(sectionVaddr, sectionSize, sectionOffset));
#endif
    EXPECT_FALSE(symbolsFile.GetSymbolWithPcAndMap(0x1234, nullptr).IsValid());
    EXPECT_EQ(symbolsFile.GetVaddrInSymbols(0x1234, 0x10, 0x20), 0x1234u);

    SymbolFileStruct symbolFileStruct;
    symbolsFile.ExportSymbolToFileFormat(symbolFileStruct);
    EXPECT_EQ(symbolFileStruct.filePath_, "base_symbol");
    EXPECT_EQ(symbolFileStruct.symbolType_, SYMBOL_UNKNOW_FILE);
}

HWTEST_F(SymbolsFileExtTest, BaseSymbolsFileVirtualMethodsReturnDefaultsWithLogDisabled, TestSize.Level0)
{
    const bool savedLogDisabled = DebugLogger::logDisabled_;
    DebugLogger::logDisabled_ = true;
    SymbolsFile symbolsFile(SYMBOL_UNKNOW_FILE, "base_symbol_disabled");
    uint64_t sectionVaddr = 0;
    uint64_t sectionSize = 0;
    uint64_t sectionOffset = 0;

    EXPECT_FALSE(symbolsFile.LoadSymbols(nullptr, ""));
    EXPECT_FALSE(symbolsFile.LoadDebugInfo(nullptr, ""));
    EXPECT_FALSE(symbolsFile.GetSectionInfo(".text", sectionVaddr, sectionSize, sectionOffset));
    EXPECT_EQ(sectionVaddr, 0u);
    EXPECT_EQ(sectionSize, 0u);
    EXPECT_EQ(sectionOffset, 0u);

    DebugLogger::logDisabled_ = savedLogDisabled;
}

HWTEST_F(SymbolsFileExtTest, FindSymbolFileCoversEmptySearchAndPlainName, TestSize.Level0)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    const bool previousRecording = SymbolsFile::onRecording_;
    SymbolsFile::onRecording_ = false;

    EXPECT_TRUE(symbolsFile->SearchReadableFile({}, "missing_symbol.so").empty());
    EXPECT_TRUE(symbolsFile->FindSymbolFile({"/data/local/tmp"}, "plain_symbol.so").empty());

    SymbolsFile::onRecording_ = previousRecording;
}

HWTEST_F(SymbolsFileExtTest, HapFileSymbolsCacheRepeatedLoadsAndNullMapLookup, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    const bool previousRecording = SymbolsFile::onRecording_;
    SymbolsFile::onRecording_ = true;
    auto hapSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_HAP_FILE, "cache_test.hap", getpid());
    auto map = std::make_shared<DfxMap>(0x1000, 0x2000, 0, 0, "cache_test.hap");
    hapSymbols->SetMapsInfo(map);

    EXPECT_TRUE(hapSymbols->LoadDebugInfo(map, ""));
    EXPECT_TRUE(hapSymbols->LoadDebugInfo(map, ""));
    EXPECT_TRUE(hapSymbols->LoadSymbols(map, ""));
    EXPECT_TRUE(hapSymbols->LoadSymbols(map, ""));
    EXPECT_FALSE(hapSymbols->GetSymbolWithPcAndMap(0x1010, nullptr).IsValid());

    SymbolsFile::onRecording_ = previousRecording;
}

HWTEST_F(SymbolsFileExtTest, ArkwebV8SymbolsRejectNonV8PathAndReuseCache, TestSize.Level0)
{
    ScopedLogEnabled logEnabled;
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);
    auto arkwebSymbols = SymbolsFile::CreateSymbolsFile(SYMBOL_ARKWEB_V8_FILE, "plain_symbol.so", getpid());
    auto map = std::make_shared<DfxMap>(0x3000, 0x4000, 0, 0, "plain_symbol.so");

    EXPECT_FALSE(arkwebSymbols->IsArkwebV8());
    arkwebSymbols->SetBoolValue(true);
    EXPECT_FALSE(arkwebSymbols->IsArkwebV8());
    EXPECT_TRUE(arkwebSymbols->LoadDebugInfo(map, ""));
    EXPECT_TRUE(arkwebSymbols->LoadDebugInfo(map, ""));
    EXPECT_TRUE(arkwebSymbols->LoadSymbols(map, ""));
    EXPECT_FALSE(arkwebSymbols->GetSymbolWithPcAndMap(0x3010, map).IsValid());
}

HWTEST_F(SymbolsFileExtTest, SymbolLookupAndExportRunWithLogDisabled, TestSize.Level0)
{
    const bool savedLogDisabled = DebugLogger::logDisabled_;
    DebugLogger::logDisabled_ = true;
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE, "log_disabled_symbols");
    symbolsFile->symbols_.emplace_back(0x3000, 0x20, "third", "module");
    symbolsFile->symbols_.emplace_back(0x1000, 0x20, "first", "module");
    symbolsFile->symbols_.emplace_back(0x1000, 0x20, "duplicate", "module");

    symbolsFile->AdjustSymbols();
    DfxSymbol found = symbolsFile->GetSymbolWithVaddr(0x1004);
    DfxSymbol missing = symbolsFile->GetSymbolWithVaddr(0x2000);
    EXPECT_TRUE(symbolsFile->UpdateBuildIdIfMatch(""));
    EXPECT_TRUE(symbolsFile->UpdateBuildIdIfMatch("build-id-disabled"));
    EXPECT_FALSE(symbolsFile->UpdateBuildIdIfMatch("build-id-other"));
    SymbolFileStruct symbolFileStruct;
    symbolsFile->ExportSymbolToFileFormat(symbolFileStruct);

    EXPECT_TRUE(found.IsValid());
    EXPECT_FALSE(missing.IsValid());
    EXPECT_EQ(symbolFileStruct.symbolStructs_.size(), 1u);
    if (!symbolFileStruct.symbolStructs_.empty()) {
        EXPECT_EQ(symbolFileStruct.symbolStructs_[0].vaddr_, 0x1000u);
    }
    EXPECT_FALSE(symbolsFile->CheckPathReadable("/data/local/tmp/missing_symbols_file_for_cov"));

    DebugLogger::logDisabled_ = savedLogDisabled;
}
}
}
}
