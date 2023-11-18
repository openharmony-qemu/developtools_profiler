/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obts_in a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limits_tions under the License.
 */

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "file.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
constexpr size_t G_FILE_PERMISSION = 664;
constexpr uint8_t RAW_TRACE_PARSE_MAX = 2;
constexpr size_t G_CHUNK_SIZE = 1024 * 1024;

class ExportTest : public ::testing::Test {
public:
    void SetUp()
    {
        ts_.InitFilter();
    }

    void TearDown() {}

public:
    TraceStreamerSelector ts_;
};

bool ParseTraceFile(TraceStreamerSelector& ts_, const std::string& tracePath)
{
    int32_t fd(base::OpenFile(tracePath, O_RDONLY, G_FILE_PERMISSION));
    TS_CHECK_TRUE(fd >= 0, false, "Failed to open trace file (errno: %d, %s)", errno, strerror(errno));
    struct stat statBuff;
    stat(tracePath.c_str(), &statBuff);
    size_t curFileSize = statBuff.st_size;
    auto isFinish = false;
    size_t curLoadSize = 0;
    auto curParseCnt = 1;
    while (true) {
        // for rawtrace next parse.the first parse is for last comm dats_;
        if (isFinish && ts_.GetFileType() == TRACE_FILETYPE_RAW_TRACE && curParseCnt < RAW_TRACE_PARSE_MAX) {
            ++curParseCnt;
            isFinish = false;
            curLoadSize = 0;
            TS_CHECK_TRUE(lseek(fd, 0, SEEK_SET) != -1, false, "lseek error:%s", strerror(errno));
        }
        std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(G_CHUNK_SIZE);
        auto rsize = read(fd, buf.get(), G_CHUNK_SIZE);
        if (rsize == 0) {
            break;
        }

        if (rsize < 0) {
            TS_LOGE("Reading trace file failed (errno: %d, %s)", errno, strerror(errno));
            return false;
        }
        curLoadSize += rsize;
        if (curLoadSize == curFileSize) {
            isFinish = true;
        }
        if (!ts_.ParseTraceDataSegment(std::move(buf), static_cast<size_t>(rsize), false, isFinish)) {
            return false;
        };
        printf("\rLoadingFile:\t%.2f MB\r", static_cast<double>(curLoadSize) / 1E6);
    }
    close(fd);
    ts_.WaitForParserEnd();
    return true;
}

/**
 * @tc.name: ExportPerfReadableText
 * @tc.desc: Export Perf Readable Text
 * @tc.type: FUNC
 */
HWTEST_F(ExportTest, ExportPerfReadableText, TestSize.Level1)
{
    TS_LOGE("test44-1");
    std::string perfFilePath("../../test/resource/hiprofiler_data_perf.htrace");
    EXPECT_TRUE(ParseTraceFile(ts_, perfFilePath));
    std::string errFilePath("/err");
    std::string sucessFilePath("export_hiprofiler_data_perf.txt");
    EXPECT_EQ(ts_.ExportPerfReadableText(errFilePath), 1);
    EXPECT_EQ(ts_.ExportPerfReadableText(sucessFilePath), 0);
    if (access(sucessFilePath.c_str(), F_OK) == 0) {
        remove(sucessFilePath.c_str());
    }
}
} // namespace TraceStreamer
} // namespace SysTuning