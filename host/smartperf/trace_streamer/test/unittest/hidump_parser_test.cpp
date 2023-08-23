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
#include <string>

#include "htrace_hidump_parser.h"
#include "hidump_plugin_result.pb.h"
#include "hidump_plugin_result.pbreader.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "parser/common_types.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
class HidumpParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown()
    {
        if (access(dbPath_.c_str(), F_OK) == 0) {
            remove(dbPath_.c_str());
        }
    }

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
    const std::string dbPath_ = "../../../data/resource/out.db";
};

/**
 * @tc.name: ParseEmptyHidumpInfo
 * @tc.desc: Parse an empty HidumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(HidumpParserTest, ParseEmptyHidumpInfo, TestSize.Level1)
{
    TS_LOGI("test7-1");
    HidumpInfo hidumpInfo;
    HtraceHidumpParser htraceHidumpParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hidumpDatas = "";
    hidumpInfo.SerializeToString(&hidumpDatas);
    ProtoReader::BytesView hidumpInfoData(reinterpret_cast<const uint8_t*>(hidumpDatas.data()), hidumpDatas.size());
    htraceHidumpParser.Parse(hidumpInfoData);
    auto size = stream_.traceDataCache_->GetConstHidumpData().Size();
    EXPECT_EQ(0, size);
}

/**
 * @tc.name: ParseLegalHidumpInfo
 * @tc.desc: Parse a legal HidumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(HidumpParserTest, ParseLegalHidumpInfo, TestSize.Level1)
{
    TS_LOGI("test7-2");
    const uint32_t FPS = 120;
    const uint32_t TV_SEC = 16326755;
    const uint32_t TV_NSEC = 39656070;

    FpsData_TimeSpec timeSpec;
    timeSpec.set_tv_nsec(TV_NSEC);
    timeSpec.set_tv_sec(TV_SEC);

    HidumpInfo* hidumpInfo = new HidumpInfo();
    auto fpsData = hidumpInfo->add_fps_event();
    fpsData->set_fps(FPS);
    fpsData->set_allocated_time(&timeSpec);

    HtraceHidumpParser htraceHidumpParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hidumpDatas = "";
    hidumpInfo->SerializeToString(&hidumpDatas);
    ProtoReader::BytesView hidumpInfoData(reinterpret_cast<const uint8_t*>(hidumpDatas.data()), hidumpDatas.size());
    htraceHidumpParser.Parse(hidumpInfoData);

    auto Fps = stream_.traceDataCache_->GetConstHidumpData().Fpss()[0];
    EXPECT_EQ(FPS, Fps);
    auto TimeSpec = stream_.traceDataCache_->GetConstHidumpData().TimeStampData()[0];
    EXPECT_EQ((TV_NSEC + TV_SEC * SEC_TO_NS), TimeSpec);
    auto Size = stream_.traceDataCache_->GetConstHidumpData().Size();
    EXPECT_EQ(1, Size);
}

/**
 * @tc.name: ParseMultipleReasonableHidumpInfo
 * @tc.desc: parse multiple reasonable HidumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(HidumpParserTest, ParseMultipleReasonableHidumpInfo, TestSize.Level1)
{
    TS_LOGI("test7-3");
    const uint32_t FPS_00 = 120;
    const uint32_t TV_SEC_00 = 1632675525;
    const uint32_t TV_NSEC_00 = 996560700;
    FpsData_TimeSpec timeSpecFirst;
    timeSpecFirst.set_tv_nsec(TV_NSEC_00);
    timeSpecFirst.set_tv_sec(TV_SEC_00);

    const uint32_t FPS_01 = 60;
    const uint32_t TV_SEC_01 = 1632675525;
    const uint32_t TV_NSEC_01 = 996560700;
    FpsData_TimeSpec timeSpecSecond;
    timeSpecSecond.set_tv_nsec(TV_NSEC_01);
    timeSpecSecond.set_tv_sec(TV_SEC_01);

    const uint32_t FPS_02 = 90;
    const uint32_t TV_SEC_02 = 1632688888;
    const uint32_t TV_NSEC_02 = 996588888;
    FpsData_TimeSpec timeSpecThird;
    timeSpecThird.set_tv_nsec(TV_NSEC_02);
    timeSpecThird.set_tv_sec(TV_SEC_02);

    HidumpInfo* hidumpInfo = new HidumpInfo();
    auto fpsDataFirst = hidumpInfo->add_fps_event();
    fpsDataFirst->set_fps(FPS_00);
    fpsDataFirst->set_allocated_time(&timeSpecFirst);

    auto fpsDataSecond = hidumpInfo->add_fps_event();
    fpsDataSecond->set_fps(FPS_01);
    fpsDataSecond->set_allocated_time(&timeSpecSecond);

    auto fpsDataThird = hidumpInfo->add_fps_event();
    fpsDataThird->set_fps(FPS_02);
    fpsDataThird->set_allocated_time(&timeSpecThird);

    HtraceHidumpParser htraceHidumpParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hidumpDatas = "";
    hidumpInfo->SerializeToString(&hidumpDatas);
    ProtoReader::BytesView hidumpInfoData(reinterpret_cast<const uint8_t*>(hidumpDatas.data()), hidumpDatas.size());
    htraceHidumpParser.Parse(hidumpInfoData);

    auto Fps_00 = stream_.traceDataCache_->GetConstHidumpData().Fpss()[0];
    auto Fps_01 = stream_.traceDataCache_->GetConstHidumpData().Fpss()[1];
    auto Fps_02 = stream_.traceDataCache_->GetConstHidumpData().Fpss()[2];
    EXPECT_EQ(FPS_00, Fps_00);
    EXPECT_EQ(FPS_01, Fps_01);
    EXPECT_EQ(FPS_02, Fps_02);

    auto TimeSpec_00 = stream_.traceDataCache_->GetConstHidumpData().TimeStampData()[0];
    auto TimeSpec_01 = stream_.traceDataCache_->GetConstHidumpData().TimeStampData()[1];
    auto TimeSpec_02 = stream_.traceDataCache_->GetConstHidumpData().TimeStampData()[2];
    EXPECT_EQ((TV_NSEC_00 + TV_SEC_00 * SEC_TO_NS), TimeSpec_00);
    EXPECT_EQ((TV_NSEC_01 + TV_SEC_01 * SEC_TO_NS), TimeSpec_01);
    EXPECT_EQ((TV_NSEC_02 + TV_SEC_02 * SEC_TO_NS), TimeSpec_02);

    auto Size = stream_.traceDataCache_->GetConstHidumpData().Size();
    EXPECT_EQ(3, Size);
}
} // namespace TraceStreamer
} // namespace SysTuning
