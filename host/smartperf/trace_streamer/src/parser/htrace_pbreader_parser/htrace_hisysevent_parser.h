
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

#ifndef HTRACE_HISYSEVENT_PARSER_H
#define HTRACE_HISYSEVENT_PARSER_H

#include "clock_filter_ex.h"
#include "event_parser_base.h"
#include "hisysevent_plugin_config.pbreader.h"
#include "hisysevent_plugin_result.pbreader.h"
#include "htrace_plugin_time_parser.h"
#include "json.hpp"
#include "trace_data/trace_data_cache.h"
#include "trace_streamer_filters.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
class HtraceHisyseventParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceHisyseventParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceHisyseventParser();
    void Finish();
    void Parse(ProtoReader::HisyseventInfo_Reader* tracePacket, uint64_t ts);
    void Parse(ProtoReader::HisyseventConfig_Reader* tracePacket, uint64_t ts);

private:
    using json = nlohmann::json;
    typedef struct {
        std::string eventSource;
        uint64_t timeStamp;
        std::vector<std::string> appName;
        std::vector<std::string> appVersions;
        std::vector<std::string> key;
        std::vector<json> value;
    } JsonData;

private:
    void NoArrayDataParse(JsonData jData,
                          std::vector<size_t> noArrayIndex,
                          DataIndex eventSourceIndex,
                          uint64_t serial);
    void ArrayDataParse(JsonData jData,
                        std::vector<size_t> arrayIndex,
                        DataIndex eventSourceIndex,
                        size_t maxArraySize,
                        uint64_t serial);
    void CommonDataParser(JsonData jData, DataIndex eventSourceIndex, uint64_t serial);
    int32_t JGetData(json& jMessage,
                     JsonData& jData,
                     size_t& maxArraySize,
                     std::vector<size_t>& noArrayIndex,
                     std::vector<size_t>& arrayIndex);

    std::vector<std::string> eventsAccordingAppNames = {"POWER_IDE_BATTERY",
                                                        "POWER_IDE_CPU",
                                                        "POWER_IDE_LOCATION",
                                                        "POWER_IDE_GPU",
                                                        "POWER_IDE_DISPLAY",
                                                        "POWER_IDE_CAMERA",
                                                        "POWER_IDE_BLUETOOTH",
                                                        "POWER_IDE_FLASHLIGHT",
                                                        "POWER_IDE_AUDIO",
                                                        "POWER_IDE_WIFISCAN",
                                                        "BRIGHTNESS_NIT",
                                                        "SIGNAL_LEVEL",
                                                        "WIFI_EVENT_RECEIVED",
                                                        "AUDIO_STREAM_CHANGE",
                                                        "AUDIO_VOLUME_CHANGE",
                                                        "WIFI_STATE",
                                                        "BLUETOOTH_BR_SWITCH_STATE",
                                                        "LOCATION_SWITCH_STATE",
                                                        "ENABLE_SENSOR",
                                                        "DISABLE_SENSOR",
                                                        "WORK_REMOVE",
                                                        "WORK_START",
                                                        "WORK_STOP",
                                                        "WORK_ADD",
                                                        "POWER_RUNNINGLOCK",
                                                        "GNSS_STATE",
                                                        "ANOMALY_SCREEN_OFF_ENERGY",
                                                        "ANOMALY_ALARM_WAKEUP",
                                                        "ANOMALY_KERNEL_WAKELOCK",
                                                        "ANOMALY_RUNNINGLOCK",
                                                        "ANORMALY_APP_ENERGY",
                                                        "ANOMALY_GNSS_ENERGY",
                                                        "ANOMALY_CPU_HIGH_FREQUENCY",
                                                        "ANOMALY_CPU_ENERGY",
                                                        "ANOMALY_WAKEUP"};
    const uint64_t MSEC_TO_NS = 1000 * 1000;
    std::vector<uint64_t> hisyseventTS_;
    bool isDeviceState = true;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // HTRACE_HISYSEVENT_PARSER_H
