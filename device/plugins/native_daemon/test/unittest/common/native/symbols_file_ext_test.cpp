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
#include <random>
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
}
}
}
