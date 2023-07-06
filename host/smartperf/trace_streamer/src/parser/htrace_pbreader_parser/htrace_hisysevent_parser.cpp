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
#include "stat_filter.h"
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

int32_t HtraceHisyseventParser::JGetData(json& jMessage,
                                         JsonData& jData,
                                         size_t& maxArraySize,
                                         std::vector<size_t>& noArrayIndex,
                                         std::vector<size_t>& arrayIndex)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_HISYSEVENT, STAT_EVENT_RECEIVED);
    for (auto i = jMessage.begin(); i != jMessage.end(); i++) {
        if (i.key() == "name_") {
            if (find(eventsAccordingAppNames.begin(), eventsAccordingAppNames.end(), i.value()) ==
                eventsAccordingAppNames.end()) {
                streamFilters_->statFilter_->IncreaseStat(TRACE_HISYSEVENT, STAT_EVENT_NOTMATCH);
                TS_LOGW("event source:%s not supported for hisysevent", std::string(i.value()).c_str());
                return -1;
            }
            jData.eventSource = i.value();
            continue;
        }
        if (i.key() == "time_") {
            jData.timeStamp = i.value();
            continue;
        }
        if (i.key() == "tag_" && i.value() != "PowerStats") {
            streamFilters_->statFilter_->IncreaseStat(TRACE_HISYSEVENT, STAT_EVENT_DATA_INVALID);
            return -1;
        }
        if (i.key() == "APPNAME") {
            jData.appName.assign(i.value().begin(), i.value().end());
        }
        if (i.value().is_array()) {
            maxArraySize = std::max(maxArraySize, i.value().size());
            arrayIndex.push_back(jData.key.size());
        } else {
            noArrayIndex.push_back(jData.key.size());
        }
        jData.key.push_back(i.key());
        jData.value.push_back(i.value());
    }
    return 0;
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
        if (value.is_string()) {
            std::string strValue = value;
            DataIndex valueIndex = traceDataCache_->GetDataIndex(strValue);
            streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(serial, jData.timeStamp, eventSourceIndex,
                                                                     keyIndex, 1, 0, valueIndex);
        } else {
            double valueIndex = value;
            streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(serial, jData.timeStamp, eventSourceIndex,
                                                                     keyIndex, 0, valueIndex, 0);
        }
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
void HtraceHisyseventParser::CommonDataParser(JsonData jData, DataIndex eventSourceIndex, uint64_t serial)
{
    for (int32_t j = 0; j < jData.key.size(); j++) {
        std::string key = jData.key[j];
        auto value = jData.value[j];
        DataIndex keyIndex = traceDataCache_->GetDataIndex(key);
        streamFilters_->hiSysEventMeasureFilter_->GetOrCreateFilterId(eventSourceIndex);
        if (value.is_string()) {
            std::string strValue = value;
            DataIndex valueIndex = traceDataCache_->GetDataIndex(strValue);
            streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(serial, jData.timeStamp, eventSourceIndex,
                                                                     keyIndex, 1, 0, valueIndex);
        } else {
            double valueIndex = value;
            streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(serial, jData.timeStamp, eventSourceIndex,
                                                                     keyIndex, 0, valueIndex, 0);
        }
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
        if (JGetData(jMessage, jData, maxArraySize, noArrayIndex, arrayIndex) < 0) {
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
