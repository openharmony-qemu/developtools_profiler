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

#include "bytrace_parser.h"
#include "file.h"
#include "htrace_parser.h"
#include "trace_streamer_selector.h"
constexpr size_t G_FILE_PERMISSION = 664;
const uint32_t PROFILE_HEADER = 1024;

using namespace testing::ext;
using namespace SysTuning;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class SplitFileDataTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: SplitFileDataByHtraceTest
 * @tc.desc: Test htrace parsing binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(SplitFileDataTest, SplitFileDataByHtraceTest, TestSize.Level1)
{
    TS_LOGI("test43-1");
    const std::string tracePath = "../../test/resource/hiprofiler_data_ability.htrace";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;
    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<TraceStreamerSelector> ta = std::make_unique<TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        ta->minTs_ = 1502026311556913964;
        ta->maxTs_ = 1502026330073755298;
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
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 1, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);

        std::ifstream inputFile(tracePath, std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "Failed to open file: " << tracePath << std::endl;
            EXPECT_TRUE(false);
        }
        uint64_t dataSize = 0;
        auto profilerHeader = ta->GetHtraceData()->GetProfilerHeader();

        for (const auto& itemHtrace : ta->GetHtraceData()->GetTraceDataHtrace()) {
            dataSize += itemHtrace.second;
        }
        profilerHeader.data.length = PROFILE_HEADER + dataSize;
        std::string buffer(reinterpret_cast<char*>(&profilerHeader), sizeof(profilerHeader));
        std::unique_ptr<uint8_t[]> combinedBuf(new uint8_t[dataSize + PROFILE_HEADER]);
        std::copy(buffer.begin(), buffer.end(), combinedBuf.get());
        std::streamsize currentOffset = PROFILE_HEADER;
        for (const auto& itemHtrace : ta->GetHtraceData()->GetTraceDataHtrace()) {
            std::unique_ptr<uint8_t[]> bufParser(new uint8_t[itemHtrace.second]);
            inputFile.seekg(itemHtrace.first);
            inputFile.read(reinterpret_cast<char*>(combinedBuf.get()) + currentOffset, itemHtrace.second);
            currentOffset += itemHtrace.second;
            if (!inputFile) {
                std::cerr << "Error reading from file." << std::endl;
                EXPECT_TRUE(false);
            }
        }
        std::unique_ptr<TraceStreamerSelector> ts = std::make_unique<TraceStreamerSelector>();
        EXPECT_TRUE(ts->ParseTraceDataSegment(std::move(combinedBuf), dataSize + PROFILE_HEADER, 0, 1));
    } else {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: SplitFileDataByHtraceTest
 * @tc.desc: Test htrace parsing binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(SplitFileDataTest, SplitFileDataBySystraceTest, TestSize.Level1)
{
    TS_LOGI("test43-2");
    const std::string tracePath = "../../test/resource/trace_small_10.systrace";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;
    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        ta->minTs_ = 88029692887000;
        ta->maxTs_ = 88032820831000;
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
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 1, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        std::unique_ptr<uint8_t[]> bufParser(new uint8_t[ta->GetBytraceData()->GetTraceDataBytrace().size()]);
        std::copy(ta->GetBytraceData()->GetTraceDataBytrace().begin(),
                  ta->GetBytraceData()->GetTraceDataBytrace().end(), bufParser.get());
        std::unique_ptr<TraceStreamerSelector> ts = std::make_unique<TraceStreamerSelector>();
        EXPECT_TRUE(
            ts->ParseTraceDataSegment(std::move(bufParser), ta->GetBytraceData()->GetTraceDataBytrace().size(), 0, 1));
    } else {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: SplitFileDataByHtraceTest
 * @tc.desc: Test htrace parsing binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(SplitFileDataTest, SplitFileDataByEbpfTest, TestSize.Level1)
{
    TS_LOGI("test43-3");
    const std::string tracePath = "../../test/resource/ebpf_bio.htrace";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;
    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        ta->minTs_ = 800423789228;
        ta->maxTs_ = 810586732842;
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
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 1, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        std::ifstream inputFile(tracePath, std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "Failed to open file: " << tracePath << std::endl;
            EXPECT_TRUE(false);
        }

        auto splitResult = ta->GetHtraceData()->GetEbpfDataParser()->GetEbpfSplitResult();
        uint64_t headDataSize = 0;
        for (const auto& itemHtrace : ta->GetHtraceData()->GetTraceDataHtrace()) {
            headDataSize += itemHtrace.second;
        }
        auto profilerHeader = ta->GetHtraceData()->GetProfilerHeader();
        profilerHeader.data.length = PROFILE_HEADER + headDataSize;
        std::string bufferData(reinterpret_cast<char*>(&profilerHeader), sizeof(profilerHeader));
        uint64_t dataSize = 0;
        for (auto it = splitResult.begin(); it != splitResult.end(); ++it) {
            if (it->type == (int32_t)SplitDataDataType::SPLIT_FILE_JSON) {
                dataSize += it->json.size;
            }
        }
        std::unique_ptr<uint8_t[]> combinedBuf(new uint8_t[dataSize + PROFILE_HEADER + headDataSize]);
        std::copy(bufferData.begin(), bufferData.end(), combinedBuf.get());
        std::streamsize currentOffset = PROFILE_HEADER;
        for (const auto& itemHtrace : ta->GetHtraceData()->GetTraceDataHtrace()) {
            inputFile.seekg(itemHtrace.first);
            inputFile.read(reinterpret_cast<char*>(combinedBuf.get()) + currentOffset, itemHtrace.second);
            currentOffset += itemHtrace.second;
        }
        for (auto it = splitResult.begin(); it != splitResult.end(); ++it) {
            if (it->type == (int32_t)SplitDataDataType::SPLIT_FILE_JSON) {
                inputFile.seekg(it->json.offset);
                inputFile.read(reinterpret_cast<char*>(combinedBuf.get()) + currentOffset, it->json.size);
                currentOffset += it->json.size;
            }
            if (!inputFile) {
                std::cerr << "Error reading from file." << std::endl;
                EXPECT_TRUE(false);
            }
        }
        std::unique_ptr<TraceStreamerSelector> ts = std::make_unique<TraceStreamerSelector>();
        EXPECT_TRUE(ts->ParseTraceDataSegment(std::move(combinedBuf), dataSize + PROFILE_HEADER + headDataSize, 0, 1));
    } else {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: SplitFileDataByNativehookTest
 * @tc.desc: Test htrace parsing binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(SplitFileDataTest, SplitFileDataByNativehookTest, TestSize.Level1)
{
    TS_LOGI("test43-4");
    const std::string tracePath = "../../test/resource/Mmap.htrace";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;
    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        ta->minTs_ = 1502031384794922107;
        ta->maxTs_ = 1502031423412858932;
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
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 1, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
    } else {
        EXPECT_TRUE(false);
    }
}

/**
 * @tc.name: SplitFileDataByPerfTest
 * @tc.desc: Test htrace parsing binary file export database
 * @tc.type: FUNC
 */
HWTEST_F(SplitFileDataTest, SplitFileDataByPerfTest, TestSize.Level1)
{
    TS_LOGI("test43-5");
    const std::string tracePath = "../../test/resource/hiprofiler_data_perf.htrace";
    constexpr size_t readSize = 1024;
    constexpr uint32_t lineLength = 256;
    if (access(tracePath.c_str(), F_OK) == 0) {
        std::unique_ptr<SysTuning::TraceStreamer::TraceStreamerSelector> ta =
            std::make_unique<SysTuning::TraceStreamer::TraceStreamerSelector>();
        ta->EnableMetaTable(false);
        ta->minTs_ = 30389799963682;
        ta->maxTs_ = 30408971157414;
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
            if (!ta->ParseTraceDataSegment(std::move(buf), rsize, 1, 1)) {
                break;
            };
        }
        ta->WaitForParserEnd();
        close(fd);
        std::ifstream inputFile(tracePath, std::ios::binary);
        if (!inputFile.is_open()) {
            std::cerr << "Failed to open file: " << tracePath << std::endl;
            EXPECT_TRUE(false);
        }

        auto splitResult = ta->GetHtraceData()->GetPerfSplitResult();
        uint64_t headDataSize = 0;
        for (const auto& itemHtrace : ta->GetHtraceData()->GetTraceDataHtrace()) {
            headDataSize += itemHtrace.second;
        }
        auto profilerHeader = ta->GetHtraceData()->GetProfilerHeader();
        profilerHeader.data.length = PROFILE_HEADER + headDataSize;
        std::string bufferData(reinterpret_cast<char*>(&profilerHeader), sizeof(profilerHeader));
        uint64_t dataSize = 0;
        for (auto it = splitResult.begin(); it != splitResult.end(); ++it) {
            if (it->type == (int32_t)SplitDataDataType::SPLIT_FILE_JSON) {
                dataSize += it->json.size;
            }
        }
        std::unique_ptr<uint8_t[]> combinedBuf(new uint8_t[dataSize + PROFILE_HEADER + headDataSize]);
        std::copy(bufferData.begin(), bufferData.end(), combinedBuf.get());
        std::streamsize currentOffset = PROFILE_HEADER;
        for (const auto& itemHtrace : ta->GetHtraceData()->GetTraceDataHtrace()) {
            inputFile.seekg(itemHtrace.first);
            inputFile.read(reinterpret_cast<char*>(combinedBuf.get()) + currentOffset, itemHtrace.second);
            currentOffset += itemHtrace.second;
        }
        for (auto it = splitResult.begin(); it != splitResult.end(); ++it) {
            if (it->type == (int32_t)SplitDataDataType::SPLIT_FILE_JSON) {
                inputFile.seekg(it->json.offset);
                inputFile.read(reinterpret_cast<char*>(combinedBuf.get()) + currentOffset, it->json.size);
                currentOffset += it->json.size;
            }
            if (!inputFile) {
                std::cerr << "Error reading from file." << std::endl;
                EXPECT_TRUE(false);
            }
        }
        std::unique_ptr<TraceStreamerSelector> ts = std::make_unique<TraceStreamerSelector>();
        EXPECT_TRUE(ts->ParseTraceDataSegment(std::move(combinedBuf), dataSize + PROFILE_HEADER + headDataSize, 0, 1));
    } else {
        EXPECT_TRUE(false);
    }
}

} // namespace TraceStreamer
} // namespace SysTuning
