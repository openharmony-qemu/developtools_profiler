/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <iostream>
#include <string>
#include <unistd.h>

#include "file.h"
#include "trace_streamer_selector.h"
constexpr size_t G_FILE_PERMISSION = 664;

using namespace testing::ext;
using namespace SysTuning;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class ParserPbreaderTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: HtracePbreaderParserTest
 * @tc.desc: Test htrace parsing binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(ParserPbreaderTest, HtracePbreaderParserTest, TestSize.Level1)
{
    TS_LOGI("test34-1");
    const std::string tracePath = "../../test/resource/pbreader.htrace";
    const std::string dbPath = "../../test/resource/test34-1_out.db";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;
    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        int32_t fd(base::OpenFile(tracePath, O_RDONLY, G_FILE_PERMISSION));
        while (true) {
            std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(readSize);
            auto rsize = base::Read(fd, buf.get(), readSize);

            if (rsize == 0) {
                break;
            }
            if (rsize < 0) {
                TS_LOGD("Reading trace file over (errno: %d, %s)", errno, strerror(errno));
                break;
            }
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 0, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        ta->ExportDatabase(dbPath);
        EXPECT_TRUE(access(dbPath.c_str(), F_OK) == 0);
        remove(dbPath.c_str());
    } else {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: BytraceParserTest
 * @tc.desc: Test bytrace parsing TXT file to export database
 * @tc.type: FUNC
 */
HWTEST_F(ParserPbreaderTest, BytraceParserTest, TestSize.Level1)
{
    TS_LOGI("test34-2");
    const std::string tracePath = "../../test/resource/ut_bytrace_input_full.txt";
    const std::string dbPath = "../../test/resource/test34-2_out.db";
    constexpr size_t readSize = 1024 * 1024;
    constexpr uint32_t lineLength = 256;

    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        int32_t fd(base::OpenFile(tracePath, O_RDONLY, G_FILE_PERMISSION));
        while (true) {
            std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(readSize);
            auto rsize = base::Read(fd, buf.get(), readSize);
            if (rsize == 0) {
                break;
            }
            if (rsize < 0) {
                TS_LOGD("Reading trace file failed (errno: %d, %s)", errno, strerror(errno));
                break;
            }
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 0, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        ta->ExportDatabase(dbPath);
        EXPECT_TRUE(access(dbPath.c_str(), F_OK) == 0);
        remove(dbPath.c_str());
    } else {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: HtraceAndPerfParserTest
 * @tc.desc: Test parsing htrace and perf binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(ParserPbreaderTest, HtraceAndPerfParserTest, TestSize.Level1)
{
    TS_LOGI("test34-3");
    const std::string tracePath = "../../test/resource/htrace_perf.bin";
    const std::string dbPath = "../../test/resource/test34-3_out.db";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;

    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        int32_t fd(base::OpenFile(tracePath, O_RDONLY, G_FILE_PERMISSION));
        while (true) {
            std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(readSize);
            auto rsize = base::Read(fd, buf.get(), readSize);

            if (rsize == 0) {
                break;
            }
            if (rsize < 0) {
                TS_LOGD("Reading trace file over (errno: %d, %s)", errno, strerror(errno));
                break;
            }
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 0, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        ta->ExportDatabase(dbPath);
        EXPECT_TRUE(access(dbPath.c_str(), F_OK) == 0);
        remove(dbPath.c_str());
    } else {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: HtraceAndEbpfParserTest
 * @tc.desc: Test parsing htrace and ebpf binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(ParserPbreaderTest, HtraceAndEbpfParserTest, TestSize.Level1)
{
    TS_LOGI("test34-4");
    const std::string tracePath = "../../test/resource/htrace_ebpf.bin";
    const std::string dbPath = "../../test/resource/test34-4_out.db";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;

    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        int32_t fd(base::OpenFile(tracePath, O_RDONLY, G_FILE_PERMISSION));
        while (true) {
            std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(readSize);
            auto rsize = base::Read(fd, buf.get(), readSize);

            if (rsize == 0) {
                break;
            }
            if (rsize < 0) {
                TS_LOGD("Reading trace file over (errno: %d, %s)", errno, strerror(errno));
                break;
            }
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 0, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        ta->ExportDatabase(dbPath);
        EXPECT_TRUE(access(dbPath.c_str(), F_OK) == 0);
        remove(dbPath.c_str());
    } else {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: NoHeaderPerfParseTest
 * @tc.desc: Test parsing perf(no profiler header) binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(ParserPbreaderTest, NoHeaderPerfParseTest, TestSize.Level1)
{
    TS_LOGI("test34-5");
    const std::string noHeaderPerfPath = "../../test/resource/htrace_perf_no_profiler_header.bin";
    const std::string dbPath = "../../test/resource/test34-5_out.db";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;

    if (access(noHeaderPerfPath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        int32_t fd(base::OpenFile(noHeaderPerfPath, O_RDONLY, G_FILE_PERMISSION));
        while (true) {
            std::unique_ptr<uint8_t[]> buff = std::make_unique<uint8_t[]>(readSize);
            auto rsize = base::Read(fd, buff.get(), readSize);
            if (rsize == 0) {
                break;
            }
            if (rsize < 0) {
                TS_LOGD("Reading trace file over (errno: %d, %s)", errno, strerror(errno));
                break;
            }
            if (!ta->ParseTraceDataSegment(std::move(buff), rsize, 0, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        ta->ExportDatabase(dbPath);
        EXPECT_TRUE(access(dbPath.c_str(), F_OK) == 0);
        remove(dbPath.c_str());
    } else {
        EXPECT_TRUE(false);
    }
}
/**
 * @tc.name: PerfCompressedParseTest
 * @tc.desc: Test parsing perf(compressed callstack) binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(ParserPbreaderTest, PerfCompressedParseTest, TestSize.Level1)
{
    TS_LOGI("test34-6");
    const std::string compressedPerfPath = "../../test/resource/perfCompressed.data";
    const std::string dbPath = "../../test/resource/test34-6_out.db";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;

    if (access(compressedPerfPath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        int32_t fd(base::OpenFile(compressedPerfPath, O_RDONLY, G_FILE_PERMISSION));
        while (true) {
            std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(readSize);
            auto rsize = base::Read(fd, buffer.get(), readSize);
            if (rsize == 0) {
                break;
            }
            if (rsize < 0) {
                TS_LOGD("Reading compressed stack perf trace file over (errno: %d, %s)", errno, strerror(errno));
                break;
            }
            if (!ta->ParseTraceDataSegment(std::move(buffer), rsize, 0, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        ta->ExportDatabase(dbPath);
        EXPECT_TRUE(access(dbPath.c_str(), F_OK) == 0);
        remove(dbPath.c_str());
    } else {
        EXPECT_TRUE(false);
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
