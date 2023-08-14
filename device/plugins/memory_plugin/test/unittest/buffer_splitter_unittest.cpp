/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <memory>

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "buffer_splitter.h"

using namespace testing::ext;

using ConstCharPtr = std::unique_ptr<const char>::pointer;

namespace {
class BufferSplitterUnittest : public ::testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

struct TestElement {
    int32_t index;
    ConstCharPtr curLine;
    size_t curLineSz;
    ConstCharPtr curWord;
    size_t curWordSz;
};

/**
 * @tc.name: memory plugin
 * @tc.desc: Constructed function test.
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, constructorWithNullptr, TestSize.Level1)
{
    char* text = nullptr;
    int readsize = 0;
    BufferSplitter totalbuffer(text, readsize);
    EXPECT_EQ(nullptr, totalbuffer.CurLine());
    EXPECT_EQ((size_t)0, totalbuffer.CurLineSize());
    EXPECT_EQ(nullptr, totalbuffer.CurWord());
    EXPECT_EQ((size_t)0, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Constructed function test.
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, constructorWith1lengthBuffer01, TestSize.Level1)
{
    char text[] = {'\0'};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);
    EXPECT_EQ(nullptr, totalbuffer.CurLine());
    EXPECT_EQ((size_t)0, totalbuffer.CurLineSize());
    EXPECT_EQ(nullptr, totalbuffer.CurWord());
    EXPECT_EQ((size_t)0, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Constructed function test.
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, constructorWith1lengthBuffer02, TestSize.Level1)
{
    char text[] = {'\n'};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);
    EXPECT_EQ(nullptr, totalbuffer.CurLine());
    EXPECT_EQ((size_t)0, totalbuffer.CurLineSize());
    EXPECT_EQ(nullptr, totalbuffer.CurWord());
    EXPECT_EQ((size_t)0, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Constructed function test.
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, constructorWith1lengthBuffer03, TestSize.Level1)
{
    char text[] = {'3'};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);
    EXPECT_EQ(nullptr, totalbuffer.CurLine());
    EXPECT_EQ((size_t)0, totalbuffer.CurLineSize());
    EXPECT_EQ(nullptr, totalbuffer.CurWord());
    EXPECT_EQ((size_t)0, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Constructed function test.
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, constructorWith1LineBuffer, TestSize.Level1)
{
    char text[] = {'a', 'b', 'c', 'd', '\0'};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);

    TestElement expect = {0, text, 4, nullptr, 0};
    EXPECT_EQ(expect.curLine, totalbuffer.CurLine());
    EXPECT_EQ(expect.curLineSz, totalbuffer.CurLineSize());
    EXPECT_EQ(expect.curWord, totalbuffer.CurWord());
    EXPECT_EQ(expect.curWordSz, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Constructed function test.
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, constructorWithMultipleLinesBuffer, TestSize.Level1)
{
    char text[] = {'a', 'b', 'c', 'd', '\n', '1', '2', '3', '4', '5', '\0'};
    TestElement expect = {0, text, 4, nullptr, 0};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);

    EXPECT_EQ(expect.curLine, totalbuffer.CurLine());
    EXPECT_EQ(expect.curLineSz, totalbuffer.CurLineSize());
    EXPECT_EQ(expect.curWord, totalbuffer.CurWord());
    EXPECT_EQ(expect.curWordSz, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextLine().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextLineWithOnlyOneLineBuffer, TestSize.Level1)
{
    char text[] = {'a', 'b', 'c', 'd', '\0'};
    TestElement expect = {0, text, 4, nullptr, 0};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);

    EXPECT_EQ(expect.curLine, totalbuffer.CurLine());
    EXPECT_EQ(expect.curLineSz, totalbuffer.CurLineSize());
    EXPECT_EQ(expect.curWord, totalbuffer.CurWord());
    EXPECT_EQ(expect.curWordSz, totalbuffer.CurWordSize());

    EXPECT_FALSE(totalbuffer.NextLine());
    EXPECT_EQ(nullptr, totalbuffer.CurLine());
    EXPECT_EQ((size_t)0, totalbuffer.CurLineSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextLine().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextLineWithMultipleLinesBuffer, TestSize.Level1)
{
    char text[] = {'a', 'b', 'c', 'd', '\n', '1', '2', '3', '4', '5', '6', '\0'};
    TestElement expect[] = {{0, text, 4, nullptr, 0}, {5, &text[5], 6, nullptr, 0}};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);

    size_t i = 0;
    do {
        EXPECT_EQ(expect[i].curLine, totalbuffer.CurLine());
        EXPECT_EQ(expect[i].curLineSz, totalbuffer.CurLineSize());
        EXPECT_EQ(expect[i].curWord, totalbuffer.CurWord());
        EXPECT_EQ(expect[i].curWordSz, totalbuffer.CurWordSize());
        i++;
    } while (totalbuffer.NextLine());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextWord().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextWordWithOnlyOneLineBuffer01, TestSize.Level1)
{
    char text[] = {'a', 'b', ' ', 'c', 'd', '\0'};
    TestElement expect = {0, text, 5, text, 2};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);

    EXPECT_EQ(expect.curLine, totalbuffer.CurLine());
    EXPECT_EQ(expect.curLineSz, totalbuffer.CurLineSize());
    EXPECT_TRUE(totalbuffer.NextWord(' '));
    EXPECT_EQ(expect.curWord, totalbuffer.CurWord());
    EXPECT_EQ(expect.curWordSz, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextWord().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextWordWithOnlyOneLineBuffer02, TestSize.Level1)
{
    char text[] = {'a', 'b', ' ', 'c', 'd', ' ', '\0'};
    TestElement expect[] = {{0, text, 6, text, 2}, {0, text, 4, &text[3], 2}};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);

    EXPECT_EQ(expect[0].curLine, totalbuffer.CurLine());
    EXPECT_EQ(expect[0].curLineSz, totalbuffer.CurLineSize());
    size_t cnt = sizeof(expect) / sizeof(expect[0]);
    for (size_t i = 0; i < cnt; i++) {
        EXPECT_TRUE(totalbuffer.NextWord(' '));
        EXPECT_EQ(expect[i].curWord, totalbuffer.CurWord());
        EXPECT_EQ(expect[i].curWordSz, totalbuffer.CurWordSize());
    }
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextWord().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextWordWithOnlyOneLineBuffer03, TestSize.Level1)
{
    char text[] = {'a', 'b', ' ', 'c', 'd', ' ', '\0'};
    TestElement expect = {0, text, 6, text, 6};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);

    EXPECT_EQ(expect.curLine, totalbuffer.CurLine());
    EXPECT_EQ(expect.curLineSz, totalbuffer.CurLineSize());
    EXPECT_TRUE(totalbuffer.NextWord('\0'));
    EXPECT_EQ(expect.curWord, totalbuffer.CurWord());
    EXPECT_EQ(expect.curWordSz, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextWord().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextWordWithOnlyOneLineBuffer04, TestSize.Level1)
{
    char text[] = {'a', 'b', ' ', 'c', 'd', ' ', '\0'};
    TestElement expect = {0, text, 6, nullptr, 0};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);

    EXPECT_EQ(expect.curLine, totalbuffer.CurLine());
    EXPECT_EQ(expect.curLineSz, totalbuffer.CurLineSize());
    EXPECT_FALSE(totalbuffer.NextWord('e'));
    EXPECT_EQ(expect.curWord, totalbuffer.CurWord());
    EXPECT_EQ(expect.curWordSz, totalbuffer.CurWordSize());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextWord().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextWordWithMultipleLinesBuffer01, TestSize.Level1)
{
    char text[] = {'a', 'b', ' ', 'c', 'd', ' ', '\n', '1', '2', ' ', '3', '4', '5', ' ', '6', '\0'};
    TestElement expect[] = {
        {0, text, 6, text, 2}, {0, text, 6, &text[3], 2}, {0, text, 6, &text[7], 2}, {0, text, 4, &text[10], 3}};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);
    size_t expectWordCnt = 2;
    size_t curLineCnt = 0;
    do {
        for (size_t i = 0; i < expectWordCnt; i++) {
            EXPECT_TRUE(totalbuffer.NextWord(' '));
            EXPECT_EQ(expect[(curLineCnt * 2) + i].curWord, totalbuffer.CurWord());
            EXPECT_EQ(expect[(curLineCnt * 2) + i].curWordSz, totalbuffer.CurWordSize());
        }
        curLineCnt++;
    } while (totalbuffer.NextLine());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextWord().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextWordWithMultipleLinesBuffer02, TestSize.Level1)
{
    char text[] = {'a', 'b', ' ', 'c', 'd', ' ', '\n', '1', '2', ' ', '3', '4', '5', ' ', '6', '\0'};
    TestElement expect[] = {
        {0, text, 6, text, 2}, {0, text, 6, nullptr, 0}, {0, text, 6, &text[7], 2}, {0, text, 4, nullptr, 0}};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);
    size_t expectWordCnt = 2;
    size_t curLineCnt = 0;
    do {
        for (size_t i = 0; i < expectWordCnt; i++) {
            if (i == 0) {
                EXPECT_TRUE(totalbuffer.NextWord(' '));
            }
            if (i == 1) {
                EXPECT_FALSE(totalbuffer.NextWord('#'));
            }
            EXPECT_EQ(expect[(curLineCnt * 2) + i].curWord, totalbuffer.CurWord());
            EXPECT_EQ(expect[(curLineCnt * 2) + i].curWordSz, totalbuffer.CurWordSize());
        }
        curLineCnt++;
    } while (totalbuffer.NextLine());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test NextWord().
 * @tc.type: FUNC
 */
HWTEST_F(BufferSplitterUnittest, NextWordWithMultipleLinesBuffer03, TestSize.Level1)
{
    char text[] = {'a', 'b', ' ', 'c', 'd', ' ', '\n', '1', '2', ' ', '3', '4', '5', ' ', '6', '\0'};
    TestElement expect[] = {
        {0, text, 6, nullptr, 0}, {0, text, 6, text, 2}, {0, text, 6, nullptr, 0}, {0, text, 6, &text[7], 2}};
    int readsize = sizeof(text);
    BufferSplitter totalbuffer(text, readsize);
    size_t expectWordCnt = 2;
    size_t curLineCnt = 0;
    do {
        for (size_t i = 0; i < expectWordCnt; i++) {
            if (i == 0) {
                EXPECT_FALSE(totalbuffer.NextWord('#'));
            }
            if (i == 1) {
                EXPECT_TRUE(totalbuffer.NextWord(' '));
            }
            EXPECT_EQ(expect[(curLineCnt * 2) + i].curWord, totalbuffer.CurWord());
            EXPECT_EQ(expect[(curLineCnt * 2) + i].curWordSz, totalbuffer.CurWordSize());
        }
        curLineCnt++;
    } while (totalbuffer.NextLine());
}

/**
 * @tc.name: memory plugin
 * @tc.desc: Test the scene that actually used,parse /proc/meminfo.
 * @tc.type: FUNC
 */
namespace {
char g_kMockMeminfo[] = R"(
MemTotal:       16168736 kB
MemFree:         7154492 kB
MemAvailable:   15481028 kB
Buffers:         2397540 kB
Cached:          4711136 kB
SwapCached:        27628 kB
Active:          5556068 kB
Inactive:        1644560 kB
Active(anon):      62580 kB
Inactive(anon):    43352 kB
Active(file):    5493488 kB
Inactive(file):  1601208 kB
Unevictable:         388 kB
Mlocked:               0 kB
SwapTotal:      16777180 kB
SwapFree:       16500700 kB
Dirty:                 0 kB
Writeback:             0 kB
AnonPages:         87672 kB
Mapped:           116988 kB
Shmem:             13980 kB
KReclaimable:    1568904 kB
Slab:            1641176 kB
SReclaimable:    1568904 kB
SUnreclaim:        72272 kB
KernelStack:        7008 kB
PageTables:        28244 kB
NFS_Unstable:          0 kB
Bounce:                0 kB
WritebackTmp:          0 kB
CommitLimit:    24861548 kB
Committed_AS:    2569488 kB
VmallocTotal:   34359738367 kB
VmallocUsed:       34260 kB
VmallocChunk:          0 kB
Percpu:             2912 kB
HardwareCorrupted:     0 kB
AnonHugePages:         0 kB
ShmemHugePages:        0 kB
ShmemPmdMapped:        0 kB
FileHugePages:         0 kB
FilePmdMapped:         0 kB
CmaTotal:              0 kB
CmaFree:               0 kB
HugePages_Total:       0
HugePages_Free:        0
HugePages_Rsvd:        0
HugePages_Surp:        0
Hugepagesize:       2048 kB
Hugetlb:               0 kB
DirectMap4k:     1997488 kB
DirectMap2M:    14548992 kB
DirectMap1G:           0 kB)";
}

HWTEST_F(BufferSplitterUnittest, MemInfoBufferTEST, TestSize.Level1)
{
    int readsize = sizeof(g_kMockMeminfo);
    BufferSplitter totalbuffer(g_kMockMeminfo, readsize);

    struct CStrCmp {
        bool operator()(const char* a, const char* b) const
        {
            return strcmp(a, b) < 0;
        }
    };
    std::map<const char*, int, CStrCmp> meminfo_counters_ = {{"MemTotal", 16168736},
                                                             {"Active", 5556068},
                                                             {"Inactive(anon)", 43352},
                                                             {"HugePages_Total", 0},
                                                             {"DirectMap1G", 0}};

    do {
        if (!totalbuffer.NextWord(':')) {
            continue;
        }
        const_cast<char *>(totalbuffer.CurWord())[totalbuffer.CurWordSize()] = '\0';
        auto it = meminfo_counters_.find(totalbuffer.CurWord());
        if (it == meminfo_counters_.end()) {
            continue;
        }

        int counterId = it->second;
        if (!totalbuffer.NextWord(' ')) {
            continue;
        }
        auto value = static_cast<uint64_t>(strtoll(totalbuffer.CurWord(), nullptr, 10));
        EXPECT_EQ((uint64_t)counterId, value);
    } while (totalbuffer.NextLine());
}
} // namespace
