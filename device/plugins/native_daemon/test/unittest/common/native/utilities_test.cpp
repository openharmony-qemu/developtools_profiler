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

#include "utilities_test.h"

#include <chrono>
#include <limits>
#include <thread>
#include "debug_logger.h"
#include "utilities.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
const std::string TEST_PATH = "/data/test/resource/testdata/";

class ScopedLogDisabled {
public:
    ScopedLogDisabled() : oldDisabled_(DebugLogger::logDisabled_)
    {
        DebugLogger::logDisabled_ = true;
    }

    ~ScopedLogDisabled()
    {
        DebugLogger::logDisabled_ = oldDisabled_;
    }

private:
    bool oldDisabled_;
};

class UtilitiesTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void TestThread();
    void StartThreads(const size_t count);
    void ExitThreads();
    bool exitThreads_ = true;
    std::vector<pid_t> tids_;
    std::vector<std::thread> threads_;
    const int sleepTime_ = {500};
};

void UtilitiesTest::SetUpTestCase() {}

void UtilitiesTest::TearDownTestCase() {}

void UtilitiesTest::SetUp() {}

void UtilitiesTest::TearDown() {}

void UtilitiesTest::TestThread()
{
    printf("threads %ld create\n", static_cast<long>(gettid()));
    int ret = fflush(nullptr);
    if (ret == EOF) {
        printf("fflush() error\n");
    }

    tids_.emplace_back(gettid());
    while (!exitThreads_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime_));
    }
    printf("threads %ld exited\n", static_cast<long>(gettid()));
    ret = fflush(nullptr);
    if (ret == EOF) {
        printf("fflush() error\n");
    }
}

void UtilitiesTest::StartThreads(const size_t count)
{
    printf("create %zu threads\n", count);
    int ret = fflush(nullptr);
    if (ret == EOF) {
        printf("fflush() error\n");
    }

    exitThreads_ = false;
    size_t created = 0;
    while (created < count) {
        threads_.emplace_back(std::thread(&UtilitiesTest::TestThread, this));
        created++;
    }
    while (tids_.size() < count) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime_));
    }
    printf("all threads created\n");
    ret = fflush(nullptr);
    if (ret == EOF) {
        printf("fflush() error\n");
    }

    std::vector<pid_t> tids = GetSubthreadIDs(getpid());
    tids_.clear();
    for (const auto& tid : tids) {
        tids_.emplace_back(tid);
    }
}

void UtilitiesTest::ExitThreads()
{
    printf("wait all threads exit\n");
    exitThreads_ = true;
    for (std::thread &t : this->threads_) {
        t.join();
    }
    tids_.clear();
    printf("all threads exited\n");
}

/**
 * @tc.name: StringReplace
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, StringReplace, TestSize.Level0)
{
    const std::string testString = "1234567890";
    EXPECT_EQ(StringReplace(testString, "1", ""), "234567890");
    EXPECT_EQ(StringReplace(testString, "2", ""), "134567890");
    EXPECT_EQ(StringReplace(testString, "0", ""), "123456789");
    EXPECT_EQ(StringReplace(testString, "1", "0"), "0234567890");
    EXPECT_EQ(StringReplace(testString, "0", "1"), "1234567891");
    EXPECT_EQ(StringReplace(testString, "123", "1"), "14567890");
    EXPECT_EQ(StringReplace(testString, "890", "1"), "12345671");
    EXPECT_EQ(StringReplace(testString, "456", "1"), "12317890");
    EXPECT_EQ(StringReplace(testString, "123", "321"), "3214567890");
    EXPECT_EQ(StringReplace(testString, "890", "098"), "1234567098");
}

/**
 * @tc.name: StringSplit
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, StringSplit, TestSize.Level0)
{
    std::string testString = "1,23,456,7890,";
    size_t testSize = testString.size();
    EXPECT_EQ(StringSplit(testString, "1").size(), 1u);
    EXPECT_EQ(StringSplit(testString, "2").size(), 2u);
    EXPECT_EQ(StringSplit(testString, ",").size(), 4u);
    EXPECT_EQ(StringSplit(testString, "456").size(), 2u);
    EXPECT_EQ(StringSplit(testString, "000").size(), 1u);
    EXPECT_EQ(StringSplit(testString, "").size(), 1u);
    // don't change the input string
    EXPECT_EQ(testString.size(), testSize);

    EXPECT_EQ(StringSplit(testString = "").size(), 0u);
    EXPECT_EQ(StringSplit(testString = "1,2,3").size(), 3u);
    EXPECT_EQ(StringSplit(testString = "1,2,3,,,").size(), 3u);
}

/**
 * @tc.name: SubStringCount
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, SubStringCount, TestSize.Level0)
{
    std::string testString = "1,22,333,4444,";
    EXPECT_EQ(SubStringCount(testString, ""), testString.size());
    EXPECT_EQ(SubStringCount(testString, "1"), 1u);
    EXPECT_EQ(SubStringCount(testString, "2"), 2u);
    EXPECT_EQ(SubStringCount(testString, "3"), 3u);
    EXPECT_EQ(SubStringCount(testString, "4"), 4u);

    EXPECT_EQ(SubStringCount(testString, "22"), 1u);
    EXPECT_EQ(SubStringCount(testString, "33"), 1u);
    EXPECT_EQ(SubStringCount(testString, "333"), 1u);
    EXPECT_EQ(SubStringCount(testString, "4444"), 1u);
    EXPECT_EQ(SubStringCount(testString, "444"), 1u);
    EXPECT_EQ(SubStringCount(testString, "44"), 2u);
}

/**
 * @tc.name: StringEndsWith
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, StringEndsWith, TestSize.Level0)
{
    std::string testString = "1,22,333,4444,";
    EXPECT_EQ(StringEndsWith(testString, ""), true);
    EXPECT_EQ(StringEndsWith(testString, "1"), false);
    EXPECT_EQ(StringEndsWith(testString, ","), true);

    EXPECT_EQ(StringEndsWith("", ""), true);
    EXPECT_EQ(StringEndsWith("", "1"), false);
    EXPECT_EQ(StringEndsWith("", ","), false);
}

/**
 * @tc.name: StringStartsWith
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, StringStartsWith, TestSize.Level0)
{
    std::string testString = "1,22,333,4444,";
    EXPECT_EQ(StringStartsWith(testString, ""), true);
    EXPECT_EQ(StringStartsWith(testString, "1"), true);
    EXPECT_EQ(StringStartsWith(testString, ","), false);

    EXPECT_EQ(StringStartsWith("", ""), true);
    EXPECT_EQ(StringStartsWith("", "1"), false);
    EXPECT_EQ(StringStartsWith("", ","), false);
}

/**
 * @tc.name: VectorToString
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, VectorToString, TestSize.Level0)
{
    EXPECT_EQ(VectorToString<std::string>(
        {}), "<empty>");
    EXPECT_EQ(VectorToString<std::string>(
        {"a", "b", "c"}), "a,b,c");
    EXPECT_EQ(VectorToString<std::string>(
        {"a"}), "a");
    EXPECT_EQ(VectorToString<std::vector<std::string>>(
        { {} }), "[<empty>]");
    EXPECT_EQ(VectorToString<std::vector<std::string>>(
        { { "a", "b", "c" }, }), "[a,b,c]");
    EXPECT_EQ(VectorToString<std::vector<std::string>>(
        {
            {"a", "b", "c"},
            {"a", "b", "c"},
            {"a", "b", "c"},
        }),
        "[a,b,c],[a,b,c],[a,b,c]");

    EXPECT_EQ(VectorToString<int>(
        {}), "<empty>");
    EXPECT_EQ(VectorToString<int>(
        {1}), "1");
    EXPECT_EQ(VectorToString<int>(
        {1, 2, 3}), "1,2,3");

    EXPECT_EQ(VectorToString<float>(
        {}), "<empty>");
    EXPECT_EQ(VectorToString<float>(
        {1.0, 2.0, 3.0}), "1.000000,2.000000,3.000000");
}

/**
 * @tc.name: BufferToHexString
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, BufferToHexString, TestSize.Level0)
{
    const unsigned char buf[] = "12345678";

    EXPECT_STREQ(BufferToHexString(buf, 0).c_str(), "0:");
    EXPECT_STREQ(BufferToHexString(buf, 1).c_str(), "1: 0x31");
    EXPECT_STREQ(BufferToHexString(buf, 4).c_str(), "4: 0x31 0x32 0x33 0x34");
    EXPECT_STREQ(BufferToHexString(buf, 5).c_str(), "5: 0x31 0x32 0x33 0x34 0x35");
    EXPECT_STREQ(BufferToHexString(buf, 8).c_str(), "8: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38");

    const std::vector<unsigned char> vbuf(buf, buf + sizeof(buf) - 1u);

    EXPECT_STREQ(BufferToHexString(vbuf).c_str(), "8: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38");

    const unsigned char buf2[] = "1234567812345678";
    EXPECT_STREQ(BufferToHexString(buf2, 0).c_str(), "0:");
    EXPECT_STREQ(BufferToHexString(buf2, 1).c_str(), "1: 0x31");
    EXPECT_STREQ(BufferToHexString(buf2, 4).c_str(), "4: 0x31 0x32 0x33 0x34");
    EXPECT_STREQ(BufferToHexString(buf2, 5).c_str(), "5: 0x31 0x32 0x33 0x34 0x35");
    EXPECT_STREQ(BufferToHexString(buf2, 8).c_str(), "8: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38");
    EXPECT_STREQ(BufferToHexString(buf2, 9).c_str(),
                 "9: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x31");
    EXPECT_STREQ(
        BufferToHexString(buf2, 16).c_str(),
        "16: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38");

    const std::vector<unsigned char> vbuf2(buf2, buf2 + sizeof(buf2) - 1u);
    EXPECT_STREQ(
        BufferToHexString(vbuf2).c_str(),
        "16: 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38 0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38");
}

/**
 * @tc.name: HexDump
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, HexDump, TestSize.Level0)
{
    const unsigned char buf[] = "12345678";
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);

    StdoutRecord stdoutRecord;
    stdoutRecord.Start();
    HexDump(buf, 0);
    HexDump(buf, 1);
    HexDump(buf, 4);
    HexDump(buf, 5);
    HexDump(buf, 8);
    std::string out = stdoutRecord.Stop();
    EXPECT_EQ(out.empty(), false);
    EXPECT_NE(out.find("0x32"), std::string::npos);
    EXPECT_NE(out.find("0x33"), std::string::npos);
    EXPECT_NE(out.find("0x34"), std::string::npos);
    EXPECT_EQ(out.find("0x39"), std::string::npos);
}

/**
 * @tc.name: StringTrim
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, StringTrim, TestSize.Level0)
{
    std::string test;
    EXPECT_STREQ(StringTrim(test = "").c_str(), "");
    EXPECT_STREQ(StringTrim(test = "   ").c_str(), "");
    EXPECT_STREQ(StringTrim(test = " a ").c_str(), "a");
    EXPECT_STREQ(StringTrim(test = " a").c_str(), "a");
    EXPECT_STREQ(StringTrim(test = "a ").c_str(), "a");
    EXPECT_STREQ(StringTrim(test = " a1a ").c_str(), "a1a");
    EXPECT_STREQ(StringTrim(test = " a1a").c_str(), "a1a");
    EXPECT_STREQ(StringTrim(test = "a1a ").c_str(), "a1a");
    EXPECT_STREQ(StringTrim(test = "   a1a   ").c_str(), "a1a");
    EXPECT_STREQ(StringTrim(test = "   a1a").c_str(), "a1a");
    EXPECT_STREQ(StringTrim(test = "a1a   ").c_str(), "a1a");
}

/**
 * @tc.name: RecordStdout
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, RecordStdout, TestSize.Level0)
{
    StdoutRecord stdoutRecord;

    ASSERT_EQ(stdoutRecord.Start(), true);
    printf("line1: abc\n");
    printf("line2: def\n");
    printf("line3: ghi\n");
    printf("\n");
    std::string out = stdoutRecord.Stop();

    printf("stdoutRecord:\n%s", out.c_str());
    EXPECT_EQ(out.empty(), false);
    EXPECT_NE(out.find("line1:"), std::string::npos);
    EXPECT_NE(out.find("line2:"), std::string::npos);
    EXPECT_NE(out.find("line3:"), std::string::npos);
    EXPECT_EQ(out.find("line4:"), std::string::npos);
}

HWTEST_F(UtilitiesTest, RecordStdoutWithExplicitFile, TestSize.Level0)
{
    const std::string stdoutPath = "./stdout_record_test.txt";
    unlink(stdoutPath.c_str());

    StdoutRecord stdoutRecord(stdoutPath, "w+");
    printf("explicit stdout record\n");
    std::string out = stdoutRecord.Stop();
    EXPECT_NE(out.find("explicit stdout record"), std::string::npos);
    EXPECT_EQ(stdoutRecord.Stop(), out);
    unlink(stdoutPath.c_str());

    StdoutRecord invalidRecord("./missing_dir/stdout.txt", "w+");
    EXPECT_TRUE(invalidRecord.Stop().empty());
}

/**
 * @tc.name: IsDigits
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, IsDigits, TestSize.Level0)
{
    EXPECT_EQ(IsDigits(""), false);
    EXPECT_EQ(IsDigits("1"), true);
    EXPECT_EQ(IsDigits("12"), true);
    EXPECT_EQ(IsDigits("1a"), false);
    EXPECT_EQ(IsDigits("a1"), false);
    EXPECT_EQ(IsDigits("1a2"), false);
    EXPECT_EQ(IsDigits("a1b"), false);
    EXPECT_EQ(IsDigits("_1"), false);
    EXPECT_EQ(IsDigits("1_"), false);
}

/**
 * @tc.name: IsHexxDigits
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, IsHexxDigits, TestSize.Level0)
{
    EXPECT_EQ(IsHexDigits(""), false);
    EXPECT_EQ(IsHexDigits("1"), true);
    EXPECT_EQ(IsHexDigits("12"), true);
    EXPECT_EQ(IsHexDigits("1a"), true);
    EXPECT_EQ(IsHexDigits("f1"), true);
    EXPECT_EQ(IsHexDigits("1f2"), true);
    EXPECT_EQ(IsHexDigits("a1f"), true);
    EXPECT_EQ(IsHexDigits("g1"), false);
    EXPECT_EQ(IsHexDigits("1g"), false);
    EXPECT_EQ(IsHexDigits("_1"), false);
    EXPECT_EQ(IsHexDigits("1_"), false);
}

/**
 * @tc.name: IsSameCommand
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, IsSameCommand, TestSize.Level0)
{
    EXPECT_EQ(IsSameCommand("", ""), false);
    EXPECT_EQ(IsSameCommand("a", ""), false);
    EXPECT_EQ(IsSameCommand("", "b"), false);
    EXPECT_EQ(IsSameCommand("1", "2"), false);
    EXPECT_EQ(IsSameCommand("2", "1"), false);
    EXPECT_EQ(IsSameCommand("1", "1"), true);
    EXPECT_EQ(IsSameCommand("a", "a"), true);
    EXPECT_EQ(IsSameCommand("a:1", "a:2"), false);
    EXPECT_EQ(IsSameCommand("/system/bin/hiprofilerd", "hiprofilerd"), true);
    EXPECT_EQ(IsSameCommand("/system/bin/hiprofilerd", "native_daemon"), false);
}

/**
 * @tc.name: CompressFile
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, CompressFile, TestSize.Level0)
{
    std::string srcPath = "./resource/testdata/elf_test_stripped_broken";
    std::string destPath = "./test.gz";
    EXPECT_EQ(CompressFile(srcPath, destPath), true);
    {
        ScopedLogDisabled scopedLog;
        srcPath = "";
        EXPECT_EQ(CompressFile(srcPath, destPath), false);
        srcPath = "./resource/testdata/elf_test_stripped_broken";
        destPath = "";
        EXPECT_EQ(CompressFile(srcPath, destPath), false);
    }
}

/**
 * @tc.name: UncompressFile
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, UncompressFile, TestSize.Level0)
{
    std::string gzipPath = "./test.gz";
    std::string dataPath = "./test";
    EXPECT_EQ(UncompressFile(gzipPath, dataPath), true);
    {
        ScopedLogDisabled scopedLog;
        gzipPath = "./test.gz";
        dataPath = "";
        EXPECT_EQ(UncompressFile(gzipPath, dataPath), false);
        gzipPath = "";
        dataPath = "./resource/testdata/elf_test_stripped_broken";
        EXPECT_EQ(UncompressFile(gzipPath, dataPath), false);
    }
}

/**
 * @tc.name: StringPrintf
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, StringPrintf, TestSize.Level0)
{
    EXPECT_STREQ(StringPrintf("").c_str(), "");
    EXPECT_STREQ(StringPrintf("123").c_str(), "123");
    EXPECT_STREQ(StringPrintf("%d%s%c", 1, "2", 'c').c_str(), "12c");
    EXPECT_STREQ(StringPrintf("%d%s%c\t\n", 1, "2", 'c').c_str(), "12c\t\n");

    char format[PATH_MAX + 1];
    std::fill(format, format + PATH_MAX, ' ');
    format[PATH_MAX] = 0;
    EXPECT_STRNE(StringPrintf(format).c_str(), format);
    format[PATH_MAX - 1] = 0;
    EXPECT_STREQ(StringPrintf(format).c_str(), format);
    EXPECT_STREQ(StringPrintf(nullptr).c_str(), "");
}

/**
 * @tc.name: GetEntriesInDir
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, GetEntriesInDir, TestSize.Level0)
{
    std::vector<std::string> dirFileInfo;
    dirFileInfo = GetEntriesInDir("./");
    EXPECT_GE(dirFileInfo.size(), 0u);
}

/**
 * @tc.name: GetSubDirs
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, GetSubDirs, TestSize.Level0)
{
    std::vector<std::string> subDirFileInfo;
    subDirFileInfo = GetSubDirs("../");
    EXPECT_GE(subDirFileInfo.size(), 0u);
}

/**
 * @tc.name: IsDir
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, IsDir, TestSize.Level0)
{
    bool ret = IsDir("../");
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsPath
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, IsPath, TestSize.Level0)
{
    bool ret = IsPath("./");
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: PlatformPathConvert
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, PlatformPathConvert, TestSize.Level0)
{
    EXPECT_GE(PlatformPathConvert("./").length(), 0u);
}

/**
 * @tc.name: ToHex
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, ToHex, TestSize.Level0)
{
    unsigned char hVal = 'G';
    EXPECT_STREQ(ToHex(hVal, 1, true).c_str(), "0x47");
}

/**
 * @tc.name: ToHex
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, CopyFromBufferAndMove, TestSize.Level0)
{
    unsigned char *buffer = new unsigned char[4];
    buffer[0] = '1';
    buffer[1] = '2';
    buffer[2] = '3';
    buffer[3] = '4';
    int *dest = new int;
    const unsigned char *srcStr = buffer;
    EXPECT_EQ(CopyFromBufferAndMove(srcStr, dest, 4), 4u);
}

/**
 * @tc.name: ReadIntFromProcFile
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, ReadIntFromProcFile, TestSize.Level0)
{
    std::string strPath = "/proc/sys/kernel/perf_cpu_time_max_percent";
    int strLen = 0;
    EXPECT_EQ(ReadIntFromProcFile(strPath, strLen), true);
}

/**
 * @tc.name: WriteIntToProcFile
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, WriteIntToProcFile, TestSize.Level0)
{
    std::string strPath = "./hiperf_log.txt";
    int strVal = 0;
    EXPECT_EQ(WriteIntToProcFile(strPath, strVal), true);
}

/**
 * @tc.name: ReadFileToString
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, ReadFileToString, TestSize.Level0)
{
    std::string strPath = "./hiperf_log.txt";
    EXPECT_NE(ReadFileToString(strPath).length(), 0u);
}

/**
 * @tc.name: WriteStringToFile
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, WriteStringToFile, TestSize.Level0)
{
    std::string strPath = "./hiperf_log.txt";
    std::string content = "0";
    EXPECT_EQ(WriteStringToFile(strPath, content), true);
}

/**
 * @tc.name: Percentage
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, Percentage, TestSize.Level0)
{
    EXPECT_EQ(Percentage(99, 100), 99);
}

/**
 * @tc.name: PowerOfTwo
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, PowerOfTwo, TestSize.Level0)
{
    EXPECT_EQ(PowerOfTwo(1), true);
}

/**
 * @tc.name: GetSubthreadIDs
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, GetSubthreadIDs, TestSize.Level0)
{
    StartThreads(1);
    std::vector<pid_t> tids = GetSubthreadIDs(getpid());
    EXPECT_EQ(tids.size(), tids_.size());
    if (!HasFailure()) {
        for (pid_t tid : tids) {
            EXPECT_NE(find(tids_.begin(), tids_.end(), tid), tids_.end());
        }
    }
    ExitThreads();
}

/**
 * @tc.name: test funtion GetValueFromJsonFile return normal result
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, GetValueFromJsonFile, TestSize.Level0)
{
    std::string filePath = TEST_PATH + "hiprofiler_cfg.json";
    auto res = GetValueFromJsonFile(filePath, "hiprofiler_hook_process_count");
    EXPECT_EQ(res, 2);
}

/**
 * @tc.name: test funtion GetValueFromJsonFile when path is not exit
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, GetValueFromJsonFile_FileNotFound, TestSize.Level0)
{
    std::string filePath = TEST_PATH + "hiprofiler_cfg22.json";
    auto res = GetValueFromJsonFile(filePath, "hiprofiler_hook_process_count");
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: test funtion GetValueFromJsonFile when key is not exit
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, GetValueFromJsonFile_InvalidKey, TestSize.Level0)
{
    std::string filePath = TEST_PATH + "hiprofiler_cfg.json";
    auto res = GetValueFromJsonFile(filePath, "test");
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: test funtion GetValueFromJsonFile when value is not int
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, GetValueFromJsonFile_InvalidFormat, TestSize.Level0)
{
    std::string filePath = TEST_PATH +"hiprofiler_cfg_2.json";
    auto res = GetValueFromJsonFile(filePath, "hiprofiler_hook_process_count");
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: test funtion IsArkwebV8File
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(UtilitiesTest, IsArkwebV8File, TestSize.Level0)
{
    std::string filepath = "[anon:v8";
    EXPECT_TRUE(IsArkwebV8File(filepath));
    filepath = "[anon:JS_V8";
    EXPECT_TRUE(IsArkwebV8File(filepath));
    filepath = "[anon]";
    EXPECT_FALSE(IsArkwebV8File(filepath));
    filepath = "[anon:JS]";
    EXPECT_FALSE(IsArkwebV8File(filepath));
}

HWTEST_F(UtilitiesTest, AdvancedSplitStringSkipsEmptySegments, TestSize.Level0)
{
    std::vector<std::string> elems;

    AdvancedSplitString(",alpha;beta,,gamma", ",;", elems);
    ASSERT_EQ(elems.size(), 3u);
    EXPECT_EQ(elems[0], "alpha");
    EXPECT_EQ(elems[1], "beta");
    EXPECT_EQ(elems[2], "gamma");

    elems.clear();
    AdvancedSplitString(",,", ",", elems);
    EXPECT_TRUE(elems.empty());
}

HWTEST_F(UtilitiesTest, NumericAndPathUtilitiesHandleBoundaries, TestSize.Level0)
{
    EXPECT_FALSE(IsDigits("1234567890"));
    EXPECT_TRUE(IsHexDigits("0x0"));
    EXPECT_TRUE(IsHexDigits("000"));
    EXPECT_FALSE(IsHexDigits("0x"));
    EXPECT_FALSE(IsHexDigits("0xg"));

    EXPECT_EQ(LgFloor(0), 0u);
    EXPECT_EQ(LgFloor(std::numeric_limits<unsigned long>::max()), 8u * sizeof(unsigned long) - 1u);
    EXPECT_EQ(PowCeil(0), 1u);
    EXPECT_EQ(PowCeil(9), 16u);
    EXPECT_EQ(ComputeAlign(0), 0u);
    EXPECT_EQ(ComputeAlign(1), 8u);
    EXPECT_EQ(ComputeAlign(8), 8u);
    EXPECT_EQ(ComputeAlign(9), 16u);
    EXPECT_EQ(ComputeAlign(17), 32u);
    EXPECT_EQ(ComputeAlign(65), 80u);
    EXPECT_EQ(RoundUp(0, 4), 0u);
    EXPECT_EQ(RoundUp(5, 4), 8u);
    EXPECT_FALSE(PowerOfTwo(0));
    EXPECT_FALSE(PowerOfTwo(3));
    EXPECT_TRUE(PowerOfTwo(1024));

    EXPECT_TRUE(IsPath("/data"));
    EXPECT_FALSE(IsPath("relative/path"));
}

HWTEST_F(UtilitiesTest, FileAndDirectoryUtilitiesHandleFailures, TestSize.Level0)
{
    const std::string basePath = "./utilities_tmp";
    const std::string subDir = basePath + "/sub";
    const std::string dataFile = basePath + "/value.txt";
    const std::string textFile = basePath + "/text.txt";
    rmdir(subDir.c_str());
    unlink(dataFile.c_str());
    unlink(textFile.c_str());
    rmdir(basePath.c_str());

    ASSERT_EQ(mkdir(basePath.c_str(), S_IRWXU), 0);
    ASSERT_EQ(mkdir(subDir.c_str(), S_IRWXU), 0);
    ASSERT_TRUE(WriteStringToFile(dataFile, "42"));
    ASSERT_TRUE(WriteStringToFile(textFile, "abc"));
    EXPECT_FALSE(WriteStringToFile(subDir, "invalid"));
    EXPECT_FALSE(WriteIntToProcFile(subDir, 1));

    auto entries = GetEntriesInDir(basePath);
    EXPECT_NE(std::find(entries.begin(), entries.end(), "sub"), entries.end());
    EXPECT_NE(std::find(entries.begin(), entries.end(), "value.txt"), entries.end());
    EXPECT_TRUE(GetEntriesInDir(dataFile).empty());
    auto subDirs = GetSubDirs(basePath);
    EXPECT_EQ(std::count(subDirs.begin(), subDirs.end(), "sub"), 1);
    EXPECT_TRUE(GetSubDirs(dataFile).empty());
    EXPECT_FALSE(IsDir(dataFile));
    EXPECT_FALSE(IsDir(basePath + "/missing"));

    std::string content;
    EXPECT_TRUE(ReadFileToString(dataFile, content, 16));
    EXPECT_EQ(content, "42");
    EXPECT_FALSE(ReadFileToString(basePath + "/missing", content));
    EXPECT_FALSE(ReadFileToString(subDir, content));
    EXPECT_TRUE(ReadFileToString(basePath + "/missing").empty());

    int value = -1;
    EXPECT_TRUE(ReadIntFromProcFile(dataFile, value));
    EXPECT_EQ(value, 42);
    EXPECT_FALSE(ReadIntFromProcFile(basePath + "/missing", value));
    EXPECT_TRUE(ReadIntFromProcFile(textFile, value));
    EXPECT_EQ(value, 0);

    unlink(dataFile.c_str());
    unlink(textFile.c_str());
    rmdir(subDir.c_str());
    rmdir(basePath.c_str());
}

HWTEST_F(UtilitiesTest, IsArkJsFileRecognizesAllSupportedForms, TestSize.Level0)
{
    EXPECT_TRUE(IsArkJsFile("entry.hap"));
    EXPECT_TRUE(IsArkJsFile("shared.hsp"));
    EXPECT_TRUE(IsArkJsFile("[anon:ArkTS Code:abc"));
    EXPECT_TRUE(IsArkJsFile("module.abc"));
    EXPECT_TRUE(IsArkJsFile("patch.hqf"));
    EXPECT_FALSE(IsArkJsFile("module.txt"));
}

HWTEST_F(UtilitiesTest, GetValueFromJsonFileRejectsBrokenJson, TestSize.Level0)
{
    const std::string jsonPath = "./broken_hiprofiler_cfg.json";
    unlink(jsonPath.c_str());

    ASSERT_TRUE(WriteStringToFile(jsonPath, "{broken json"));
    EXPECT_EQ(GetValueFromJsonFile(jsonPath, "hiprofiler_hook_process_count"), -1);
    unlink(jsonPath.c_str());
}

HWTEST_F(UtilitiesTest, HexDumpUsesMaxSizeLimit, TestSize.Level0)
{
    const unsigned char buf[] = "12345678";
    ScopeDebugLevel tempLogLevel(LEVEL_MUCH, true);

    StdoutRecord stdoutRecord;
    ASSERT_TRUE(stdoutRecord.Start());
    HexDump(buf, sizeof(buf) - 1u, 4u);
    std::string out = stdoutRecord.Stop();
    EXPECT_NE(out.find("0x31"), std::string::npos);
    EXPECT_NE(out.find("0x34"), std::string::npos);
}

HWTEST_F(UtilitiesTest, HeaderUtilitiesHandleStateAndCopyFailures, TestSize.Level0)
{
    auto& holder = StringViewMemoryHold::GetInstance();
    const char* first = holder.HoldStringView("held-string");
    ASSERT_NE(first, nullptr);
    EXPECT_STREQ(first, "held-string");
    holder.Clear();
    const char* second = holder.HoldStringView("held-string");
    ASSERT_NE(second, nullptr);
    EXPECT_STREQ(second, "held-string");

    const unsigned char buffer[] = {1, 2, 3, 4};
    const unsigned char* src = buffer;
    int value = 0;
    EXPECT_EQ(CopyFromBufferAndMove(src, &value, sizeof(value)), sizeof(value));
    EXPECT_EQ(src, buffer + sizeof(value));

    src = buffer;
    int* invalidDest = nullptr;
    EXPECT_EQ(CopyFromBufferAndMove(src, invalidDest, sizeof(value)), sizeof(value));
    EXPECT_EQ(src, buffer);
}

HWTEST_F(UtilitiesTest, StringPrintfHandlesInvalidFormat, TestSize.Level0)
{
    EXPECT_EQ(StringPrintf(nullptr), "");
    EXPECT_EQ(StringPrintf("%999999999999999999999999s", "value"), "");
}
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
