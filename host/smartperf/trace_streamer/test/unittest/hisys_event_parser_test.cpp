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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "htrace_hisysevent_parser.h"
#include "string_to_numerical.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class HtraceHisysEventParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
};
namespace base {
auto num0 = SysTuning::base::number(15, SysTuning::base::INTEGER_RADIX_TYPE_DEC);
auto num1 = SysTuning::base::number(15, SysTuning::base::INTEGER_RADIX_TYPE_HEX);
} // namespace base
/**
 * @tc.name: ParseNoArray
 * @tc.desc: Parse a piece of data without array
 * @tc.type: FUNC
 */
HWTEST_F(HtraceHisysEventParserTest, ParseNoArray, TestSize.Level1)
{
    TS_LOGI("test9-1");
    std::string jsMessage =
        "{\"domain_\":\"POWERTHERMAL\",\"name_\":\"POWER_IDE_BATTERY\",\"type_\":1,\"time_\":22611696002,\"tz_\":\"+"
        "0000\",\"pid_\":722,\"tid_\":3462,\"uid_\":1201,\"START_TIME\":22611696002,\"END_TIME\":23617705010,\"GAS_"
        "GAUGE\":124,\"LEVEL\":33,\"SCREEN\":11,\"CHARGE\":21,\"CURRENT\":-404,\"CAPACITY\":9898,\"level_\":\"MINOR\","
        "\"id_\":\"16494176919818340149\",\"info_\":\"\"}";
    HtraceHisyseventParser::json jMessage;
    HtraceHisyseventParser::JsonData jData;
    size_t maxArraySize = 0;
    uint64_t serial = 1;
    std::vector<size_t> noArrayIndex;
    std::vector<size_t> arrayIndex;
    std::stringstream ss;
    ss << jsMessage;
    ss >> jMessage;
    HtraceHisyseventParser HisysEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    HisysEvent.JGetData(jMessage, jData, maxArraySize, noArrayIndex, arrayIndex);
    EXPECT_TRUE(jData.eventSource == "POWER_IDE_BATTERY");
    EXPECT_EQ(jData.timeStamp, 22611696002);
    EXPECT_EQ(maxArraySize, 0);
    EXPECT_EQ(noArrayIndex.size(), 17);
    EXPECT_EQ(arrayIndex.size(), 0);
    DataIndex eventSourceIndex = stream_.traceDataCache_->GetDataIndex(jData.eventSource);
    HisysEvent.CommonDataParser(jData, eventSourceIndex, serial);
    auto size = stream_.traceDataCache_->GetConstSyseventMeasureData().Size();
    EXPECT_EQ(size, 17);
    auto anticipate = stream_.traceDataCache_->GetConstSyseventMeasureData().Serial()[0];
    EXPECT_EQ(anticipate, serial);
    EXPECT_TRUE(base::num0 == "15");
    EXPECT_TRUE(base::num1 == "f");
}
/**
 * @tc.name: ParseHaveArrayData
 * @tc.desc: Parse a piece of data with array
 * @tc.type: FUNC
 */
HWTEST_F(HtraceHisysEventParserTest, ParseHaveArrayData, TestSize.Level1)
{
    TS_LOGI("test9-2");
    std::string jsMessage =
        "{\"domain_\":\"POWERTHERMAL\",\"name_\":\"POWER_IDE_WIFISCAN\",\"type_\":2,\"time_\":16611696002,\"tz_\":\"+"
        "0000\",\"pid_\":722,\"tid_\":17827,\"uid_\":1201,\"START_TIME\":1661783047454,\"END_TIME\":1661783050455,"
        "\"COUNT\":1,\"APPNAME\":[\"com.ohos.settings\",\"com.ohos.settings_js\",\"com.ohos.settings_app\"],"
        "\"FOREGROUND_COUNT\":[43,41,65],\"FOREGROUND_ENERGY\":[120,134,532],\"BACKGROUND_COUNT\":[27,856,378],"
        "\"BACKGROUND_ENERGY\":[638,65,12],\"SCREEN_ON_COUNT\":[23,558,75],\"SCREEN_ON_ENERGY\":[552,142,120],\"SCREEN_"
        "OFF_COUNT\":[78,354,21],\"SCREEN_OFF_ENERGY\":[352,65,436],\"level_\":\"MINOR\",\"id_\":"
        "\"17560016619580787102\",\"info_\":\"\"}";
    HtraceHisyseventParser::json jMessage;
    HtraceHisyseventParser::JsonData jData;
    size_t maxArraySize = 0;
    uint64_t serial = 1;
    std::vector<size_t> noArrayIndex;
    std::vector<size_t> arrayIndex;
    std::stringstream ss;
    ss << jsMessage;
    ss >> jMessage;
    HtraceHisyseventParser HisysEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    HisysEvent.JGetData(jMessage, jData, maxArraySize, noArrayIndex, arrayIndex);
    EXPECT_TRUE(jData.eventSource == "POWER_IDE_WIFISCAN");
    EXPECT_EQ(jData.timeStamp, 16611696002);
    EXPECT_EQ(maxArraySize, 3);
    EXPECT_EQ(noArrayIndex.size(), 12);
    EXPECT_EQ(arrayIndex.size(), 9);
    DataIndex eventSourceIndex = stream_.traceDataCache_->GetDataIndex(jData.eventSource);
    HisysEvent.NoArrayDataParse(jData, noArrayIndex, eventSourceIndex, serial);
    HisysEvent.ArrayDataParse(jData, arrayIndex, eventSourceIndex, maxArraySize, serial);
    auto size = stream_.traceDataCache_->GetConstSyseventMeasureData().Size();
    EXPECT_EQ(size, (9 * 3 + 12));
    auto anticipate = stream_.traceDataCache_->GetConstSyseventMeasureData().Serial()[0];
    EXPECT_EQ(anticipate, serial);
}
/**
 * @tc.name: MixedDataAnalysis
 * @tc.desc: Mixed data analysis
 * @tc.type: FUNC
 */
HWTEST_F(HtraceHisysEventParserTest, MixedDataAnalysis, TestSize.Level1)
{
    TS_LOGI("test9-3");
    std::string jsMessage1 =
        "{\"domain_\":\"POWERTHERMAL\",\"name_\":\"POWER_IDE_BATTERY\",\"type_\":1,\"time_\":22611696002,\"tz_\":\"+"
        "0000\",\"pid_\":722,\"tid_\":3462,\"uid_\":1201,\"START_TIME\":22611696002,\"END_TIME\":23617705010,\"GAS_"
        "GAUGE\":124,\"LEVEL\":33,\"SCREEN\":11,\"CHARGE\":21,\"CURRENT\":-404,\"CAPACITY\":9898,\"level_\":\"MINOR\","
        "\"id_\":\"16494176919818340149\",\"info_\":\"\"}";
    std::string jsMessage2 =
        "{\"domain_\":\"POWERTHERMAL\",\"name_\":\"POWER_IDE_WIFISCAN\",\"type_\":2,\"time_\":16611696002,\"tz_\":\"+"
        "0000\",\"pid_\":722,\"tid_\":17827,\"uid_\":1201,\"START_TIME\":1661783047454,\"END_TIME\":1661783050455,"
        "\"COUNT\":1,\"APPNAME\":[\"com.ohos.settings\",\"com.ohos.settings_js\",\"com.ohos.settings_app\"],"
        "\"FOREGROUND_COUNT\":[43,41,65],\"FOREGROUND_ENERGY\":[120,134,532],\"BACKGROUND_COUNT\":[27,856,378],"
        "\"BACKGROUND_ENERGY\":[638,65,12],\"SCREEN_ON_COUNT\":[23,558,75],\"SCREEN_ON_ENERGY\":[552,142,120],\"SCREEN_"
        "OFF_COUNT\":[78,354,21],\"SCREEN_OFF_ENERGY\":[352,65,436],\"level_\":\"MINOR\",\"id_\":"
        "\"17560016619580787102\",\"info_\":\"\"}";
    std::vector<std::string> jsMessage;
    jsMessage.push_back(jsMessage1);
    jsMessage.push_back(jsMessage2);
    uint64_t serial = 1;
    for (auto i = jsMessage.begin(); i != jsMessage.end(); i++) {
        HtraceHisyseventParser::json jMessage;
        HtraceHisyseventParser::JsonData jData;
        size_t maxArraySize = 0;
        std::vector<size_t> noArrayIndex;
        std::vector<size_t> arrayIndex;
        std::stringstream ss;
        ss << *i;
        ss >> jMessage;
        HtraceHisyseventParser HisysEvent(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
        HisysEvent.JGetData(jMessage, jData, maxArraySize, noArrayIndex, arrayIndex);
        if (jData.eventSource == "POWER_IDE_WIFISCAN") {
            EXPECT_TRUE(jData.eventSource == "POWER_IDE_WIFISCAN");
            EXPECT_EQ(jData.timeStamp, 16611696002);
            EXPECT_EQ(maxArraySize, 3);
            EXPECT_EQ(noArrayIndex.size(), 12);
            EXPECT_EQ(arrayIndex.size(), 9);
        } else {
            EXPECT_TRUE(jData.eventSource == "POWER_IDE_BATTERY");
            EXPECT_EQ(jData.timeStamp, 22611696002);
            EXPECT_EQ(maxArraySize, 0);
            EXPECT_EQ(noArrayIndex.size(), 17);
            EXPECT_EQ(arrayIndex.size(), 0);
        }
        DataIndex eventSourceIndex = stream_.traceDataCache_->GetDataIndex(jData.eventSource);
        if (maxArraySize) {
            HisysEvent.NoArrayDataParse(jData, noArrayIndex, eventSourceIndex, serial);
            HisysEvent.ArrayDataParse(jData, arrayIndex, eventSourceIndex, maxArraySize, serial);
        } else {
            HisysEvent.CommonDataParser(jData, eventSourceIndex, serial);
        }
    }
    auto size = stream_.traceDataCache_->GetConstSyseventMeasureData().Size();
    EXPECT_EQ(size, 17 + (9 * 3 + 12));
    auto anticipate = stream_.traceDataCache_->GetConstSyseventMeasureData().Serial()[0];
    EXPECT_EQ(anticipate, serial);
}
} // namespace TraceStreamer
} // namespace SysTuning
