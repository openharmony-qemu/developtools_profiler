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
#include "clock_filter_ex.h"
#include "hi_sysevent_measure_filter.h"
#include "htrace_event_parser.h"
#include "htrace_hisysevent_parser.h"
#include "htrace_parser.h"
#include "process_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceHisyseventParser::HtraceHisyseventParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx)
{
}
HtraceHisyseventParser::~HtraceHisyseventParser()
{
    TS_LOGI("hisysevent ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(GetPluginStartTime()),
            static_cast<unsigned long long>(GetPluginEndTime()));
    TS_LOGI("hisysevent real ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(MinTs()),
            static_cast<unsigned long long>(MaxTs()));
}

void HtraceHisyseventParser::NoArrayDataParse(JsonData jData,
                                              std::vector<size_t> noArrayIndex,
                                              DataIndex eventSourceIndex,
                                              uint64_t serial)
{
    for (auto itor = noArrayIndex.begin(); itor != noArrayIndex.end(); itor++) {
        auto value = jData.value[*itor];
        auto key = jData.key[*itor];
        streamFilters_->hiSysEventMeasureFilter_->GetOrCreateFilterId(eventSourceIndex);
        DataIndex keyIndex = traceDataCache_->GetDataIndex(key);
        AppendStringValue(value, serial, eventSourceIndex, keyIndex, jData.timeStamp);
    }
}
void HtraceHisyseventParser::ArrayDataParse(JsonData jData,
                                            std::vector<size_t> arrayIndex,
                                            DataIndex eventSourceIndex,
                                            size_t maxArraySize,
                                            uint64_t serial)
{
    for (int32_t j = 0; j < maxArraySize; j++) {
        for (auto itor = arrayIndex.begin(); itor != arrayIndex.end(); itor++) {
            auto value = jData.value[*itor][j];
            std::string key = jData.key[*itor];
            DataIndex keyIndex = traceDataCache_->GetDataIndex(key);
            streamFilters_->hiSysEventMeasureFilter_->GetOrCreateFilterId(eventSourceIndex);
            if (value.is_number()) {
                double valueIndex = value;
                streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(serial, jData.timeStamp, eventSourceIndex,
                                                                         keyIndex, 0, valueIndex, 0);
            } else if (value.is_string()) {
                std::string strValue = value;
                DataIndex valueIndex = traceDataCache_->GetDataIndex(strValue);
                streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(serial, jData.timeStamp, eventSourceIndex,
                                                                         keyIndex, 1, 0, valueIndex);
            }
        }
    }
}
void HtraceHisyseventParser::AppendStringValue(nlohmann::json& value,
                                               uint64_t serial,
                                               DataIndex eventSourceIndex,
                                               DataIndex keyIndex,
                                               uint64_t timeStamp)
{
    if (value.is_string()) {
        std::string strValue = value;
        DataIndex valueIndex = traceDataCache_->GetDataIndex(strValue);
        streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(serial, timeStamp, eventSourceIndex, keyIndex, 1, 0,
                                                                 valueIndex);
    } else {
        double valueIndex = value;
        streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(serial, timeStamp, eventSourceIndex, keyIndex, 0,
                                                                 valueIndex, 0);
    }
}
void HtraceHisyseventParser::CommonDataParser(JsonData jData, DataIndex eventSourceIndex, uint64_t serial)
{
    for (int32_t j = 0; j < jData.key.size(); j++) {
        std::string key = jData.key[j];
        auto value = jData.value[j];
        DataIndex keyIndex = traceDataCache_->GetDataIndex(key);
        streamFilters_->hiSysEventMeasureFilter_->GetOrCreateFilterId(eventSourceIndex);
        AppendStringValue(value, serial, eventSourceIndex, keyIndex, jData.timeStamp);
    }
}
void HtraceHisyseventParser::Finish()
{
    if (GetPluginStartTime() != GetPluginEndTime()) {
        traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
    } else {
        TS_LOGI("hisysevent time is not updated, maybe this trace file only has one piece of hisysevent data");
    }
    TS_LOGI("--------Parse end--------");
}

static std::stringstream ss;
void HtraceHisyseventParser::Parse(ProtoReader::HisyseventInfo_Reader* tracePacket, uint64_t ts)
{
    ProtoReader::DeviceStat_Reader deviceStat(tracePacket->device_state());
    ProtoReader::AudioVolumeInfo_Reader audioVolumeInfo(deviceStat.volume_state());
    if (isDeviceState) {
        streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(
            deviceStat.brightness_state(), deviceStat.bt_state(), deviceStat.location_state(), deviceStat.wifi_state(),
            audioVolumeInfo.stream_default(), audioVolumeInfo.voice_call(), audioVolumeInfo.music(),
            audioVolumeInfo.stream_ring(), audioVolumeInfo.media(), audioVolumeInfo.voice_assistant(),
            audioVolumeInfo.system(), audioVolumeInfo.alarm(), audioVolumeInfo.notification(),
            audioVolumeInfo.bluetoolth_sco(), audioVolumeInfo.enforced_audible(), audioVolumeInfo.stream_dtmf(),
            audioVolumeInfo.stream_tts(), audioVolumeInfo.accessibility(), audioVolumeInfo.recording(),
            audioVolumeInfo.stream_all());
        isDeviceState = false;
    }
    json jMessage;
    for (auto i = tracePacket->info(); i; ++i) {
        ProtoReader::HisyseventLine_Reader hisyseventLine(i->ToBytes());
        if (hisyseventLine.raw_content().ToStdString().front() != '{' ||
            hisyseventLine.raw_content().ToStdString().back() != '}') {
            continue;
        }
        ss << hisyseventLine.raw_content().ToStdString();
        ss >> jMessage;
        size_t maxArraySize = 0;
        JsonData jData;
        std::vector<size_t> noArrayIndex = {};
        std::vector<size_t> arrayIndex = {};
        if (!streamFilters_->hiSysEventMeasureFilter_->JGetData(jMessage, jData, maxArraySize, noArrayIndex,
                                                                arrayIndex)) {
            continue;
        }
        uint64_t serial = hisyseventLine.id();
        DataIndex eventSourceIndex = traceDataCache_->GetDataIndex(jData.eventSource);
        jData.timeStamp *= MSEC_TO_NS;
        auto newTimeStamp = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, jData.timeStamp);
        UpdatePluginTimeRange(TS_CLOCK_BOOTTIME, jData.timeStamp, newTimeStamp);
        jData.timeStamp = newTimeStamp;
        if (maxArraySize) {
            NoArrayDataParse(jData, noArrayIndex, eventSourceIndex, serial);
            ArrayDataParse(jData, arrayIndex, eventSourceIndex, maxArraySize, serial);
        } else {
            CommonDataParser(jData, eventSourceIndex, serial);
        }
    }
}
void HtraceHisyseventParser::Parse(ProtoReader::HisyseventConfig_Reader* tracePacket, uint64_t ts)
{
    streamFilters_->hiSysEventMeasureFilter_->AppendNewValue("message", tracePacket->msg().ToStdString());
    streamFilters_->hiSysEventMeasureFilter_->AppendNewValue("process_name", tracePacket->process_name().ToStdString());
    return;
}
} // namespace TraceStreamer
} // namespace SysTuning
