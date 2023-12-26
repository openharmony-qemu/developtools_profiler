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

#define LOG_TAG "TraceFileReaderTest"
#include <fstream>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <memory>
#include <unistd.h>
#include <vector>

#include "common_types.pb.h"
#include "logging.h"
#include "trace_file_reader.h"
#include "trace_file_writer.h"

using namespace testing::ext;

namespace {
#if defined(__i386__) || defined(__x86_64__)
const std::string DEFAULT_TEST_PATH("./");
#else
const std::string DEFAULT_TEST_PATH("/data/local/tmp/");
#endif

class TraceFileReaderTest : public ::testing::Test {
protected:
    std::string path = "trace.bin";

    static void SetUpTestCase() {}
    static void TearDownTestCase()
    {
        std::string name = "325.ht";
        std::string path = DEFAULT_TEST_PATH + name;

        if (access(path.c_str(), F_OK) == 0) {
            system(std::string("rm " + path).c_str());
        }
    }

    void SetUp() override {}

    void TearDown() override {}
};

/**
 * @tc.name: server
 * @tc.desc: write read.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileReaderTest, WriteRead, TestSize.Level1)
{
    std::string path = "trace-write-msg.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);

    constexpr int n = 100;
    for (int i = 1; i <= n; i++) {
        ProfilerPluginData pluginData{};
        pluginData.set_name("test_name");
        pluginData.set_status(i);
        pluginData.set_data("Hello, World!");
        long bytes = writer->Write(pluginData);
        EXPECT_EQ(bytes, sizeof(uint32_t) + pluginData.ByteSizeLong());
        HILOG_INFO(LOG_CORE, "[%d/%d] write %ld bytes to %s.", i, n, bytes, path.c_str());
    }
    writer.reset(); // make sure write done!

    auto reader = std::make_shared<TraceFileReader>();
    ASSERT_NE(reader, nullptr);
    ASSERT_TRUE(reader->Open(path));

    for (int i = 1; i <= n; i++) {
        ProfilerPluginData data{};
        long bytes = reader->Read(data);
        HILOG_INFO(LOG_CORE, "data = {%s, %d, %s}", data.name().c_str(), data.status(), data.data().c_str());
        HILOG_INFO(LOG_CORE, "read %ld bytes from %s", bytes, path.c_str());
    }
}

/**
 * @tc.name: server
 * @tc.desc: write read break.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileReaderTest, WriteReadBreak, TestSize.Level1)
{
    std::string path = "trace-write-msg.bin";
    auto writer = std::make_shared<TraceFileWriter>(path);
    ASSERT_NE(writer, nullptr);

    constexpr int n = 100;
    for (int i = 1; i <= n; i++) {
        ProfilerPluginData pluginData{};
        pluginData.set_name("test_name");
        pluginData.set_status(i);
        pluginData.set_data("Hello, World!");
        long bytes = writer->Write(pluginData);
        EXPECT_EQ(bytes, sizeof(uint32_t) + pluginData.ByteSizeLong());
        HILOG_INFO(LOG_CORE, "[%d/%d] write %ld bytes to %s.", i, n, bytes, path.c_str());
    }
    writer.reset(); // make sure write done!

    auto reader = std::make_shared<TraceFileReader>();
    ASSERT_NE(reader, nullptr);
    ASSERT_TRUE(reader->Open(path));

    long bytes = 0;
    do {
        ProfilerPluginData data{};
        bytes = reader->Read(data);
        HILOG_INFO(LOG_CORE, "data = {%s, %d, %s}", data.name().c_str(), data.status(), data.data().c_str());
        HILOG_INFO(LOG_CORE, "read %ld bytes from %s", bytes, path.c_str());
    } while (bytes > 0);
}

/**
 * @tc.name: server
 * @tc.desc: read.
 * @tc.type: FUNC
 */
HWTEST_F(TraceFileReaderTest, Read, TestSize.Level1)
{
    std::string name = "325.ht";
    std::string path = DEFAULT_TEST_PATH + name;

    if (access(path.c_str(), F_OK) != 0) {
        std::unique_ptr<FILE, decltype(fclose)*> fptr(fopen(path.c_str(), "wb+"), fclose);
        TraceFileHeader header = {}; // default header data
        fwrite(&header, sizeof(char), sizeof(header), fptr.get());
    }
    auto reader = std::make_shared<TraceFileReader>();
    ASSERT_NE(reader, nullptr);
    ASSERT_TRUE(reader->Open(path));

    long bytes = 0;
    do {
        ProfilerPluginData data{};
        bytes = reader->Read(data);
        fprintf(stderr, "data = [%s, %d, ...%zu], len = %zu, bytes = %ld\n", data.name().c_str(), data.status(),
                data.data().size(), data.ByteSizeLong(), bytes);
    } while (bytes > 0);
    if (access(path.c_str(), F_OK) == 0) {
        system(std::string("rm " + path).c_str());
    }
}
} // namespace
