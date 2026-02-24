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

#define LOG_TAG "TraceFileWriterTest"
#include <fstream>
#include <gtest/gtest.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include "google/protobuf/text_format.h"
#include "common_types.pb.h"
#include "logging.h"
#include "trace_file_writer.h"

using namespace testing::ext;

namespace {
class TraceFileWriterTest : public ::testing::Test {
protected:
    std::string path = "trace.bin";

    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override {}

    void TearDown() override
    {
        int retval = unlink(path.c_str());
        PROFILER_LOG_DEBUG(LOG_CORE, "unlink(%s): %d", path.c_str(), retval);
    }
};

/**
 * @tc.name: server
 * @tc.desc: Class-strengthening.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, CtorDtor, TestSize.Level1)
{
    auto writer = std::make_shared<TraceFileWriter>(path);
    EXPECT_NE(writer, nullptr);
}

/**
 * @tc.name: server
 * @tc.desc: write.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, Write, TestSize.Level1)
{
    path = "trace-write.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);

    std::string testData = "Hello, Wrold!";
    EXPECT_EQ(writer->Write(testData.data(), testData.size()), sizeof(uint32_t) + testData.size());
}

/**
 * @tc.name: server
 * @tc.desc: flush.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, Flush, TestSize.Level1)
{
    std::string testData = "Hello, Wrold!";
    path = "trace-flush.bin";
    {
        auto writer = std::make_shared<TraceFileWriter>(path);
        ASSERT_NE(writer, nullptr);
        EXPECT_EQ(writer->Write(testData.data(), testData.size()), sizeof(uint32_t) + testData.size());
        EXPECT_EQ(writer->Flush(), true);
    }

    uint32_t msgLen = 0;
    std::ifstream fin(path, std::ios_base::in | std::ios_base::binary);
    ASSERT_TRUE(fin.is_open());

    // check file length
    fin.seekg(0, std::ios_base::end);
    EXPECT_EQ(fin.tellg(), TraceFileHeader::HEADER_SIZE + sizeof(msgLen) + testData.size());

    // check msg length
    fin.seekg(TraceFileHeader::HEADER_SIZE, std::ios_base::beg); // skip file header
    fin.read(reinterpret_cast<char*>(&msgLen), sizeof(msgLen));
    EXPECT_EQ(msgLen, testData.size());

    // check msg data
    std::vector<char> outData(testData.size());
    fin.read(outData.data(), outData.size()); // read into outData
    EXPECT_EQ(memcmp(outData.data(), testData.data(), outData.size()), 0);
}

/**
 * @tc.name: server
 * @tc.desc: write message.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteMessage, TestSize.Level1)
{
    path = "trace-write-message.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);

    ProfilerPluginData pluginData;
    pluginData.set_name("ABC");
    pluginData.set_status(0);
    pluginData.set_data("DEF");
    EXPECT_GT(writer->Write(pluginData), 0);
}

/**
 * @tc.name: server
 * @tc.desc: Split file.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SplitFileWriter, TestSize.Level1)
{
    path = "trace-write-test.bin";
    auto writer = std::make_shared<TraceFileWriter>(path, true, 0, 0);
    EXPECT_NE(writer, nullptr);
    writer->Path();
    writer->SetStopSplitFile(false);
    std::string testData = "this is a test case!";
    EXPECT_EQ(writer->Write(testData.data(), testData.size()), sizeof(uint32_t) + testData.size());
    std::string testStr = "test case";
    writer->SetPluginConfig(testStr.data(), testStr.size());

    std::string testPath = "trace-write-bin";
    auto writerTestPath = std::make_shared<TraceFileWriter>(testPath, true, 0, 1);
    EXPECT_NE(writerTestPath, nullptr);
    writerTestPath->splitFilePaths_.push("trace-write-path-1");
    writerTestPath->splitFilePaths_.push("trace-write-path-2");
    writerTestPath->DeleteOldSplitFile();
    EXPECT_TRUE(writerTestPath->IsSplitFile(300 * 1024 * 1024));

    std::string testPathTemp = "/data/local/tmp/trace-write-path";
    auto writerTemp = std::make_shared<TraceFileWriter>(testPathTemp, true, 0, 1);
    EXPECT_NE(writerTemp, nullptr);
}

/**
 * @tc.name: server
 * @tc.desc: remap
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, RemapFile, TestSize.Level1)
{
    path = "remap-test.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    EXPECT_NE(writer, nullptr);
    uint32_t applySize = 1024 * 1024;  // 1MB
    char temp[1024] = {"this is a test ,this is a test ,this is a test, this is a test\n"};
    for (size_t i = 0; i < 2000; i++) {  // 2000: loop count
        uint8_t* memory = nullptr;
        uint32_t offset = 0;
        writer->GetMemory(applySize, &memory, &offset);
        if (i < 1023) {  // 1023: remap critical point
            EXPECT_NE(memory, nullptr);
            if (memcpy_s(memory, applySize, temp, sizeof(temp)) != EOK) {
                PROFILER_LOG_INFO(LOG_CORE, "memcpy_s failed,apply size %u", applySize);
                continue;
            }
            writer->FinishReport(applySize);
        } else {
            EXPECT_EQ(memory, nullptr);
        }
    }
    EXPECT_GE(writer->fileLength_, 1024 * 1024 * 1024);
    close(fd);
}
} // namespace

namespace {
/**
 * @tc.name: CtorWithEmptyPath
 * @tc.desc: test constructor with empty path.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, CtorWithEmptyPath, TestSize.Level1)
{
    path = "";
    auto writer = std::make_shared<TraceFileWriter>(path);
    EXPECT_NE(writer, nullptr);
}

/**
 * @tc.name: CtorWithInvalidFd
 * @tc.desc: test constructor with invalid file descriptor.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, CtorWithInvalidFd, TestSize.Level1)
{
    auto writer = std::make_shared<TraceFileWriter>(-1);
    EXPECT_NE(writer, nullptr);
}

/**
 * @tc.name: CtorWithSplitFileMinSize
 * @tc.desc: test constructor with split file min size.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, CtorWithSplitFileMinSize, TestSize.Level1)
{
    path = "trace-split-min.txt";
    auto writer = std::make_shared<TraceFileWriter>(path, true, 100, 5);
    EXPECT_NE(writer, nullptr);
}

/**
 * @tc.name: TraceFilrWriterCtor001
 * @tc.desc: test constructor
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, TraceFilrWriterCtor001, TestSize.Level1)
{
    path = "trace-split-max.txt";
    auto writer = std::make_shared<TraceFileWriter>(path, true, 200, 100);
    EXPECT_NE(writer, nullptr);
}

/**
 * @tc.name: TraceFilrWriterCtor002
 * @tc.desc: test constructor with zero split params.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, TraceFilrWriterCtor002, TestSize.Level1)
{
    path = "trace-split-zero.txt";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    EXPECT_NE(writer, nullptr);
}

/**
 * @tc.name: WriteWithEmptyData
 * @tc.desc: test write with empty data.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteWithEmptyData, TestSize.Level1)
{
    path = "trace-write-empty.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    std::string emptyData = "";
    long result = writer->Write(emptyData.data(), emptyData.size());
    EXPECT_EQ(result, sizeof(uint32_t));
}

/**
 * @tc.name: WriteWithZeroSize
 * @tc.desc: test write with zero size.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteWithZeroSize, TestSize.Level1)
{
    path = "trace-write-zero.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    std::string testData = "test";
    long result = writer->Write(testData.data(), 0);
    EXPECT_EQ(result, sizeof(uint32_t));
}

/**
 * @tc.name: WriteWithLargeData
 * @tc.desc: test write with large data block.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteWithLargeData, TestSize.Level1)
{
    path = "trace-write-large.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    std::vector<char> largeData(1024 * 1024, 'A');
    long result = writer->Write(largeData.data(), largeData.size());
    EXPECT_EQ(result, sizeof(uint32_t) + largeData.size());
}

/**
 * @tc.name: WriteMultipleTimes
 * @tc.desc: test write multiple times consecutively.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteMultipleTimes, TestSize.Level1)
{
    path = "trace-write-multi.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    std::string testData = "test data";
    for (int i = 0; i < 10; i++) {
        long result = writer->Write(testData.data(), testData.size());
        EXPECT_EQ(result, sizeof(uint32_t) + testData.size());
    }
}

/**
 * @tc.name: SetPluginConfigWithValidData
 * @tc.desc: test SetPluginConfig with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SetPluginConfigWithValidData, TestSize.Level1)
{
    path = "trace-plugin-config.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    std::string configData = "plugin_config_data";
    bool result = writer->SetPluginConfig(configData.data(), configData.size());
    EXPECT_NE(writer, nullptr);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SetPluginConfigWithEmptyData
 * @tc.desc: test SetPluginConfig with empty data.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SetPluginConfigWithEmptyData, TestSize.Level1)
{
    path = "trace-plugin-empty.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    bool result = writer->SetPluginConfig("", 0);
    EXPECT_NE(writer, nullptr);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: WriteMessageWithEmptyName
 * @tc.desc: test write message with empty name.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteMessageWithEmptyName, TestSize.Level1)
{
    path = "trace-msg-empty-name.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    ProfilerPluginData pluginData;
    pluginData.set_name("");
    pluginData.set_status(0);
    pluginData.set_data("test");
    long result = writer->Write(pluginData);
    EXPECT_GT(result, 0);
}

/**
 * @tc.name: WriteMessageWithLargeData
 * @tc.desc: test write message with large data.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteMessageWithLargeData, TestSize.Level1)
{
    path = "trace-msg-large.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    ProfilerPluginData pluginData;
    pluginData.set_name("LargeDataPlugin");
    pluginData.set_status(0);
    std::string largeStr(10240, 'X');
    pluginData.set_data(largeStr);
    long result = writer->Write(pluginData);
    EXPECT_GT(result, 0);
}

/**
 * @tc.name: IsSplitFileWithSmallSize
 * @tc.desc: test IsSplitFile with size smaller than threshold.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, IsSplitFileWithSmallSize, TestSize.Level1)
{
    path = "trace-split-small.bin";
    auto writer = std::make_shared<TraceFileWriter>(path, true, 1024 * 1024, 5);
    ASSERT_NE(writer, nullptr);
    bool result = writer->IsSplitFile(100);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSplitFileWithExactSize
 * @tc.desc: test IsSplitFile with size equal to threshold.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, IsSplitFileWithExactSize, TestSize.Level1)
{
    path = "trace-split-exact.bin";
    uint32_t splitSize = 1024 * 1024;
    auto writer = std::make_shared<TraceFileWriter>(path, true, splitSize, 5);
    ASSERT_NE(writer, nullptr);
    bool result = writer->IsSplitFile(splitSize);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: IsSplitFileWithLargeSize
 * @tc.desc: test IsSplitFile with size larger than threshold.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, IsSplitFileWithLargeSize, TestSize.Level1)
{
    path = "trace-split-large.bin";
    auto writer = std::make_shared<TraceFileWriter>(path, true, 1024, 5);
    ASSERT_NE(writer, nullptr);
    bool result = writer->IsSplitFile(1024 * 1024);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: SetStopSplitFileTrue
 * @tc.desc: test SetStopSplitFile with true value.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SetStopSplitFileTrue, TestSize.Level1)
{
    path = "trace-stop-split.bin";
    auto writer = std::make_shared<TraceFileWriter>(path, true, 1024, 5);
    ASSERT_NE(writer, nullptr);
    writer->SetStopSplitFile(true);
    EXPECT_TRUE(writer->isStop_);
}

/**
 * @tc.name: SetStopSplitFileFalse
 * @tc.desc: test SetStopSplitFile with false value.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SetStopSplitFileFalse, TestSize.Level1)
{
    path = "trace-continue-split.bin";
    auto writer = std::make_shared<TraceFileWriter>(path, true, 1024, 5);
    ASSERT_NE(writer, nullptr);
    writer->SetStopSplitFile(false);
    EXPECT_FALSE(writer->isStop_);
}

/**
 * @tc.name: PathReturnCorrectValue
 * @tc.desc: test Path returns correct file path.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, PathReturnCorrectValue, TestSize.Level1)
{
    path = "trace-path-test.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    std::string returnedPath = writer->Path();
    EXPECT_EQ(returnedPath, path);
}

/**
 * @tc.name: GetMemoryWithValidSize
 * @tc.desc: test GetMemory with valid size.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, GetMemoryWithValidSize, TestSize.Level1)
{
    path = "trace-getmem-valid.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    writer->GetMemory(1024, &memory, &offset);
    EXPECT_NE(memory, nullptr);
    close(fd);
}

/**
 * @tc.name: GetMemoryWithZeroSize
 * @tc.desc: test GetMemory with zero size.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, GetMemoryWithZeroSize, TestSize.Level1)
{
    path = "trace-getmem-zero.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    writer->GetMemory(0, &memory, &offset);
    EXPECT_NE(memory, nullptr);
    close(fd);
}

/**
 * @tc.name: SeekWithValidOffset
 * @tc.desc: test Seek with valid offset.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SeekWithValidOffset, TestSize.Level1)
{
    path = "trace-seek-valid.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    bool result = writer->Seek(100);
    EXPECT_TRUE(result);
    close(fd);
}

/**
 * @tc.name: SeekWithZeroOffset
 * @tc.desc: test Seek with zero offset.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SeekWithZeroOffset, TestSize.Level1)
{
    path = "trace-seek-zero.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    bool result = writer->Seek(0);
    EXPECT_TRUE(result);
    close(fd);
}

/**
 * @tc.name: FinishReportWithValidSize
 * @tc.desc: test FinishReport with valid size.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, FinishReportWithValidSize, TestSize.Level1)
{
    path = "trace-finish-valid.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    writer->GetMemory(1024, &memory, &offset);
    writer->FinishReport(512);
    EXPECT_NE(writer, nullptr);
    EXPECT_NE(writer->fileWriteLength_, 0);
    close(fd);
}

/**
 * @tc.name: FinishReportWithZeroSize
 * @tc.desc: test FinishReport with zero size.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, FinishReportWithZeroSize, TestSize.Level1)
{
    path = "trace-finish-zero.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    writer->GetMemory(1024, &memory, &offset);
    writer->FinishReport(0);
    EXPECT_EQ(writer->writeBytes_, 0);
    close(fd);
}

/**
 * @tc.name: SetTimeSourceTest
 * @tc.desc: test SetTimeSource function.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SetTimeSourceTest, TestSize.Level1)
{
    path = "trace-timesource.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    writer->SetTimeSource();
    EXPECT_NE(writer, nullptr);
    EXPECT_NE(writer->headerDataTime_.boottime, 0);
}

/**
 * @tc.name: SetDurationTimeTest
 * @tc.desc: test SetDurationTime function.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SetDurationTimeTest, TestSize.Level1)
{
    path = "trace-duration.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    writer->SetDurationTime();
    EXPECT_NE(writer, nullptr);
    EXPECT_NE(writer->headerDataTime_.durationNs, 0);
}

/**
 * @tc.name: SetTimeStampTest
 * @tc.desc: test SetTimeStamp function.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SetTimeStampTest, TestSize.Level1)
{
    path = "trace-timestamp.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    writer->SetTimeStamp();
    EXPECT_NE(writer, nullptr);
    EXPECT_EQ(writer->headerDataTime_.realtime, 0);
}

/**
 * @tc.name: FinishWithValidData
 * @tc.desc: test Finish function with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, FinishWithValidData, TestSize.Level1)
{
    path = "trace-finish-data.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    std::string testData = "test data for finish";
    writer->Write(testData.data(), testData.size());
    EXPECT_TRUE(writer->Finish());
}

/**
 * @tc.name: FlushStreamTest
 * @tc.desc: test FlushStream function.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, FlushStreamTest, TestSize.Level1)
{
    path = "trace-flushstream.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    std::string testData = "test data";
    EXPECT_NE(writer, nullptr);
    writer->Write(testData.data(), testData.size());
    EXPECT_TRUE(writer->FlushStream());
}

/**
 * @tc.name: ResetPosTest
 * @tc.desc: test ResetPos function.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, ResetPosTest, TestSize.Level1)
{
    path = "trace-resetpos.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    EXPECT_NE(writer, nullptr);
    EXPECT_TRUE(writer->GetMemory(1024, &memory, &offset));
    writer->ResetPos();
    close(fd);
}

/**
 * @tc.name: WriteHeaderTest
 * @tc.desc: test WriteHeader function.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteHeaderTest, TestSize.Level1)
{
    path = "trace-writeheader.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    EXPECT_TRUE(writer->WriteHeader());
    EXPECT_NE(writer, nullptr);
}

/**
 * @tc.name: GetCtxTest
 * @tc.desc: test GetCtx function.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, GetCtxTest, TestSize.Level1)
{
    path = "trace-getctx.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    ASSERT_GE(fd, 0);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    auto ctx = writer->GetCtx();
    EXPECT_NE(ctx, nullptr);
    close(fd);
}

/**
 * @tc.name: SetDurationTimeZero
 * @tc.desc: test SetDurationTime with zero value.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, SetDurationTimeZero, TestSize.Level1)
{
    path = "trace-duration-zero.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    writer->SetDurationTime();
    EXPECT_NE(writer, nullptr);
    EXPECT_EQ(writer->headerDataTime_.realtime, 0);
}

/**
 * @tc.name: FlushReturnValue
 * @tc.desc: test Flush return value.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, FlushReturnValue, TestSize.Level1)
{
    path = "trace-duration-zero.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    std::string testData = "test";
    writer->Write(testData.data(), testData.size());
    bool result = writer->Flush();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: WriteStandalonePluginFileValid
 * @tc.desc: test WriteStandalonePluginFile with valid data.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteStandalonePluginFileValid, TestSize.Level1)
{
    path = "trace-standalone.bin";
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    auto writer = std::make_shared<TraceFileWriter>(fd);
    ASSERT_NE(writer, nullptr);
    ProfilerPluginData pluginData;
    pluginData.set_name("standalone_plugin");
    pluginData.set_status(0);
    pluginData.set_data("standalone data");
    std::string str;
    google::protobuf::TextFormat::PrintToString(pluginData, &str);
    EXPECT_FALSE(writer->WriteStandalonePluginFile(str, "name1", "version1", DataType::STANDALONE_DATA));
}

/**
 * @tc.name: WriteStandalonePluginFileEmpty
 * @tc.desc: test WriteStandalonePluginFile with empty data.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileWriterTest, WriteStandalonePluginFileEmpty, TestSize.Level1)
{
    path = "trace-standalone-empty.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);
    ProfilerPluginData pluginData;
    std::string str;
    google::protobuf::TextFormat::PrintToString(pluginData, &str);
    EXPECT_FALSE(writer->WriteStandalonePluginFile(str, "aa", "bb", DataType::STANDALONE_DATA));
}
} // namespace
