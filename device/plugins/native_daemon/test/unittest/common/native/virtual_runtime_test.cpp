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

#include "virtual_runtime_test.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <link.h>
#include <random>
#include <sys/mman.h>

#include <hilog/log.h>

#include "symbols_file_test.h"

using namespace testing::ext;
using namespace std;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
class VirtualRuntimeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    const std::string TEST_LOG_MESSAGE = "<HELLO_TEST_LOG_MESSAGE>";
    void LogLevelTest(std::vector<std::string> args, DebugLevel level);
    default_random_engine rnd_;
    std::unique_ptr<VirtualRuntime> runtime_;
    size_t callbackCount_ = 0;

    void PrepareKernelSymbol();
    void PrepareUserSymbol();
};

void VirtualRuntimeTest::SetUpTestCase() {}

void VirtualRuntimeTest::TearDownTestCase() {}

void VirtualRuntimeTest::SetUp()
{
    runtime_ = std::make_unique<VirtualRuntime>();
    callbackCount_ = 0;
}

void VirtualRuntimeTest::TearDown()
{
    runtime_.release();
}

/**
 * @tc.name: VirtualRuntimeTest
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeTest, VirtualRuntimeTest, TestSize.Level1) {}

/**
 * @tc.name: SetSymbolsPaths
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeTest, SetSymbolsPaths, TestSize.Level1)
{
    std::vector<std::string> symbolsSearchPaths;
    runtime_->SetSymbolsPaths(symbolsSearchPaths);

    symbolsSearchPaths.clear();
    symbolsSearchPaths.push_back(PATH_DATA_TEMP);
    symbolsSearchPaths.push_back(PATH_NOT_EXISTS);
    EXPECT_EQ(runtime_->SetSymbolsPaths(symbolsSearchPaths), true);

    symbolsSearchPaths.clear();
    symbolsSearchPaths.push_back(PATH_DATA_TEMP);
    symbolsSearchPaths.push_back(PATH_NOT_EXISTS);
    symbolsSearchPaths.push_back(PATH_DATA_TEMP);
    symbolsSearchPaths.push_back(PATH_NOT_EXISTS);
    EXPECT_EQ(runtime_->SetSymbolsPaths(symbolsSearchPaths), true);

    symbolsSearchPaths.clear();
    symbolsSearchPaths.push_back(PATH_DATA_TEMP);
    symbolsSearchPaths.push_back(PATH_NOT_EXISTS);
    symbolsSearchPaths.push_back(PATH_DATA_TEMP);
    symbolsSearchPaths.push_back(PATH_NOT_EXISTS);
    symbolsSearchPaths.push_back(PATH_DATA_TEMP);
    symbolsSearchPaths.push_back(PATH_NOT_EXISTS);
    EXPECT_EQ(runtime_->SetSymbolsPaths(symbolsSearchPaths), true);
}

/**
 * @tc.name: GetSymbolsFiles
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeTest, GetSymbolsFiles, TestSize.Level1)
{
    EXPECT_EQ(runtime_->GetSymbolsFiles().size(), 0u);
    runtime_->UpdateSymbols(TEST_FILE_ELF_FULL_PATH);
    EXPECT_EQ(runtime_->GetSymbolsFiles().size(), 1u);
}

namespace {
constexpr const pid_t TEST_TID = 1;
constexpr const uint64_t TEST_USET_VADDR = 0x1000;
constexpr const uint64_t TEST_KERNEL_VADDR = TEST_USET_VADDR / 4;
constexpr const uint64_t TEST_KERNEL_LEN = TEST_USET_VADDR / 2;
constexpr const uint64_t TEST_USET_MAP_BEGIN = 0x2000;
constexpr const uint64_t TEST_USET_MAP_LEN = 0x4000;
} // namespace

void VirtualRuntimeTest::PrepareKernelSymbol()
{
    std::string kernelSymbol = "kernel_symbol";
    auto kernel = SymbolsFile::CreateSymbolsFile(SYMBOL_KERNEL_FILE);
    kernel->filePath_ = kernelSymbol;
    kernel->symbols_.emplace_back(TEST_KERNEL_VADDR, 1u, "first_kernel_func", kernel->filePath_);
    kernel->symbols_.emplace_back(TEST_KERNEL_VADDR + 1u, 1u, "second_kernel_func",
                                  kernel->filePath_);
    runtime_->symbolsFiles_[kernel->filePath_] = std::move(kernel);

    auto &kernelMap = runtime_->kernelSpaceMemMaps_.emplace_back();
    kernelMap.name_ = kernelSymbol;
    kernelMap.begin_ = 0;
    kernelMap.end_ = 0 + TEST_KERNEL_LEN;
    kernelMap.pageoffset_ = 0;
}

void VirtualRuntimeTest::PrepareUserSymbol()
{
    std::string userSymbol = "user_symbol";
    auto user = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE);
    user->filePath_ = userSymbol;
    user->symbols_.emplace_back(TEST_KERNEL_VADDR, 1u, "first_user_func", user->filePath_);
    user->symbols_.emplace_back(TEST_KERNEL_VADDR + 1u, 1u, "second_user_func", user->filePath_);
    user->textExecVaddrFileOffset_ = TEST_KERNEL_VADDR;
    user->textExecVaddr_ = TEST_KERNEL_VADDR;
    runtime_->symbolsFiles_[user->filePath_] =  std::move(user);

    VirtualThread &thread = runtime_->GetThread(TEST_TID, TEST_TID);
    thread.CreateMapItem(userSymbol, TEST_USET_MAP_BEGIN, TEST_USET_MAP_LEN, 0);
}

/**
 * @tc.name: GetSymbol
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeTest, GetSymbol, TestSize.Level1)
{
    Symbol symbol;
    PrepareKernelSymbol();
    PrepareUserSymbol();

    ScopeDebugLevel tempLogLevel(LEVEL_MUCH);
    CallFrame callFrame(0);

    symbol = runtime_->GetSymbol(callFrame, TEST_TID, TEST_TID);
    EXPECT_EQ(symbol.isValid(), false);

    callFrame.ip_ = TEST_KERNEL_VADDR + TEST_USET_MAP_BEGIN;
    symbol = runtime_->GetSymbol(callFrame, TEST_TID, TEST_TID);
    // in user
    EXPECT_EQ(symbol.isValid(), true);
    EXPECT_EQ(symbol.funcVaddr_, TEST_KERNEL_VADDR);
    EXPECT_STREQ(symbol.name_.data(), "first_user_func");
}

/**
 * @tc.name: GetThread
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(VirtualRuntimeTest, GetThread, TestSize.Level1)
{
    runtime_->GetThread(1, 2);
    runtime_->GetThread(3, 4);
    runtime_->GetThread(5, 6);
    // runtime have 0 thread, so here need +1u
    EXPECT_EQ(runtime_->GetThreads().size(), 3u);
    if (HasFailure()) {
        for (auto &pair : runtime_->GetThreads()) {
            printf("pid %d tid %d\n", pair.second.pid_, pair.second.tid_);
        }
    }
}
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
