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
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <unistd.h>
#include <vector>

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
        HILOG_DEBUG(LOG_CORE, "unlink(%s): %d", path.c_str(), retval);
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
} // namespace
