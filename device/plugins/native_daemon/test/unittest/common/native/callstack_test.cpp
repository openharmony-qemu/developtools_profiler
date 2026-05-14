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

#include "callstack_test.h"
#include "virtual_runtime.h"
#include "get_thread_id.h"
using namespace testing::ext;
using namespace std;
using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
class CallStackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    default_random_engine rnd_;
};

void CallStackTest::SetUpTestCase() {}

void CallStackTest::TearDownTestCase() {}

void CallStackTest::SetUp() {}

void CallStackTest::TearDown() {}

#if defined(HAVE_LIBUNWINDER) && HAVE_LIBUNWINDER
HWTEST_F(CallStackTest, ReadVirtualThreadMemoryUsesCache, TestSize.Level0)
{
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    auto runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    CallStack callStack;
    UnwindInfo info = {thread, nullptr, 0, buildArchType, callStack};
    ADDR_TYPE value = 0;
    callStack.porcessMemoryMap_[thread.pid_][0x2000] = 0xABCD;
    EXPECT_TRUE(CallStack::ReadVirtualThreadMemory(info, 0x2000, &value));
    EXPECT_EQ(value, static_cast<ADDR_TYPE>(0xABCD));
}

HWTEST_F(CallStackTest, AccessMem2HandlesOverflowAndStackData, TestSize.Level0)
{
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    auto runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    CallStack callStack;
    uintptr_t stackData[2] = {0x1234, 0x5678};
    callStack.stackPoint_ = 0x1000;
    callStack.stackEnd_ = callStack.stackPoint_ + sizeof(stackData);
    callStack.stack_ = reinterpret_cast<const u8*>(stackData);
    UnwindInfo info = {thread, nullptr, 0, buildArchType, callStack};
    uintptr_t value = 1;
    EXPECT_EQ(CallStack::AccessMem2(std::numeric_limits<uintptr_t>::max(), &value, &info), -1);
    EXPECT_EQ(CallStack::AccessMem2(0x1000, &value, &info), 0);
    EXPECT_EQ(value, static_cast<uintptr_t>(0x1234));
}

HWTEST_F(CallStackTest, GetMapByPcReturnsExpectedStatus, TestSize.Level0)
{
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    auto runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    thread.CreateMapItem("/data/test/test.so", 0x1000, 0x100, 0);
    CallStack callStack;
    UnwindInfo info = {thread, nullptr, 0, buildArchType, callStack};
    std::shared_ptr<DfxMap> map = nullptr;
    EXPECT_EQ(CallStack::GetMapByPc(0x1010, map, &info), 0);
    ASSERT_NE(map, nullptr);
    EXPECT_EQ(map->begin, 0x1000u);
    EXPECT_EQ(CallStack::GetMapByPc(0x2000, map, &info), -1);
}

HWTEST_F(CallStackTest, GetIpSPRejectsMissingRegsAndZeroIp, TestSize.Level0)
{
    CallStack callStack;
    callStack.arch_ = buildArchType;
    uint64_t ip = 1;
    uint64_t sp = 1;
    std::vector<u64> regs(RegisterGetCount(), 0);
    regs[RegisterGetSP(buildArchType)] = 0x2000;
    EXPECT_FALSE(callStack.GetIpSP(ip, sp, regs.data(), 0));
    EXPECT_FALSE(callStack.GetIpSP(ip, sp, regs.data(), RegisterGetSP(buildArchType)));
    EXPECT_FALSE(callStack.GetIpSP(ip, sp, regs.data(), regs.size()));
    regs[RegisterGetIP(buildArchType)] = 0x3000;
    EXPECT_TRUE(callStack.GetIpSP(ip, sp, regs.data(), regs.size()));
}

HWTEST_F(CallStackTest, UnwindCallStackRejectsInvalidRegs, TestSize.Level0)
{
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    auto runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    std::vector<u64> regs(RegisterGetCount(), 0);
    uint8_t stackData[16] = {};
    std::vector<CallFrame> frames;
    CallStack callStack;
    EXPECT_FALSE(callStack.UnwindCallStack(thread, regs.data(), 0, stackData, sizeof(stackData), frames, 1));
    EXPECT_TRUE(frames.empty());
}

HWTEST_F(CallStackTest, AccessMem2ReadsCachedMmapAndReportsMiss, TestSize.Level0)
{
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    auto runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    CallStack callStack;
    callStack.stackPoint_ = 0x1000;
    callStack.stackEnd_ = 0x1100;
    callStack.porcessMemoryMap_[thread.pid_][0x2000] = 0x4321;
    UnwindInfo info = {thread, nullptr, 0, buildArchType, callStack};
    uintptr_t value = 0;
    EXPECT_EQ(CallStack::AccessMem2(0x2000, &value, &info), 0);
    EXPECT_EQ(value, static_cast<uintptr_t>(0x4321));
    EXPECT_EQ(CallStack::AccessMem2(0x3000, &value, &info), -1);
}

HWTEST_F(CallStackTest, FindUnwindTableRejectsMissingMapAndSymbols, TestSize.Level0)
{
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    auto runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    CallStack callStack;
    UnwindInfo info = {thread, nullptr, 0, buildArchType, callStack};
    UnwindTableInfo tableInfo;
    EXPECT_EQ(CallStack::FindUnwindTable(0x1000, tableInfo, &info), -1);
    thread.CreateMapItem("/data/test/no_symbols.so", 0x1000, 0x100, 0);
    EXPECT_EQ(CallStack::FindUnwindTable(0x1010, tableInfo, &info), -1);
}

HWTEST_F(CallStackTest, FillUnwindTableRejectsSymbolsWithoutElf, TestSize.Level0)
{
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    symbolsFiles["/data/test/test.so"] = std::make_unique<SymbolsFile>(SYMBOL_UNKNOW_FILE, "/data/test/test.so");
    auto runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    thread.CreateMapItem("/data/test/test.so", 0x1000, 0x100, 0);
    CallStack callStack;
    UnwindInfo info = {thread, nullptr, 0, buildArchType, callStack};
    UnwindTableInfo tableInfo;
    auto map = thread.FindMapByAddr(0x1010);
    ASSERT_NE(map, nullptr);
    EXPECT_EQ(CallStack::FillUnwindTable(symbolsFiles["/data/test/test.so"].get(), map, &info, 0x1010, tableInfo), -1);
}
#endif

HWTEST_F(CallStackTest, UnwindCallStackSingleFrame, TestSize.Level0)
{
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    auto runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    std::vector<u64> regs(RegisterGetCount(), 0);
    regs[RegisterGetSP(buildArchType)] = 0x2000;
    regs[RegisterGetIP(buildArchType)] = 0x3000;
    uint8_t stackData[16] = {};
    std::vector<CallFrame> frames;
    CallStack callStack;
    ASSERT_TRUE(callStack.UnwindCallStack(thread, regs.data(), regs.size(), stackData, sizeof(stackData), frames, 1));
    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].ip_, 0x3000u);
    EXPECT_EQ(frames[0].sp_, 0x2000u);
}


#ifndef is_linux
/**
 * @tc.name: UnwindCallStack
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(CallStackTest, UnwindCallStack, TestSize.Level0)
{
    std::vector<u64> regs;
    std::vector<u8> data;
    LoadFromFile(PATH_RESOURCE_TEST_DWARF_DATA + TEST_DWARF_USER_REGS_0, regs);
    LoadFromFile(PATH_RESOURCE_TEST_DWARF_DATA + TEST_DWARF_USER_DATA_0, data);
    if (regs.size() > 0 and data.size() > 0) {
#ifdef __arm__
    ASSERT_EQ(regs.size(), 16u);
#endif
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles;
    std::string symbolFilePath = PATH_RESOURCE_TEST_DWARF_DATA + TEST_DWARF_ELF;
    symbolsFiles[symbolFilePath] = SymbolsFile::CreateSymbolsFile(SYMBOL_ELF_FILE, symbolFilePath);
    auto& symbolsFile = symbolsFiles[symbolFilePath];
    ASSERT_EQ(symbolsFile->LoadSymbols(), true);
    // fix the name
    symbolsFile->filePath_ = TEST_DWARF_MMAP.front().fileName;
    std::shared_ptr<VirtualRuntime> runtime = std::make_shared<VirtualRuntime>();
    VirtualThread thread(getpid(), GetThreadId(), symbolsFiles, runtime.get(), false);
    MakeMaps(thread);
    std::vector<CallFrame> callFrames;
    CallStack callStack;
    callStack.UnwindCallStack(thread, regs.data(), regs.size(), data.data(), data.size(), callFrames);
#ifdef __arm__
    bool ret = callStack.UnwindCallStack(thread, regs.data(), regs.size(), data.data(), data.size(), callFrames);
    ASSERT_TRUE(ret);
#endif
    }
}
#endif
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
