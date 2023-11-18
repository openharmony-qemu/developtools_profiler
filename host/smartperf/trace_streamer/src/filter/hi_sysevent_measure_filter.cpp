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

#include "hi_sysevent_measure_filter.h"
#include "clock_filter_ex.h"
#include "filter_filter.h"
#include "log.h"
#include "stat_filter.h"
#include "system_event_measure_filter.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
HiSysEventMeasureFilter::HiSysEventMeasureFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter), appKey_(INVALID_UINT64), appName_(INVALID_UINT64)
{
}

HiSysEventMeasureFilter::~HiSysEventMeasureFilter() {}

DataIndex HiSysEventMeasureFilter::AppendNewValue(uint64_t serial,
                                                  uint64_t timeStamp,
                                                  DataIndex appNameId,
                                                  DataIndex key,
                                                  int32_t type,
                                                  double numericValue,
                                                  DataIndex strValue)
{
    uint64_t appKeyId = GetOrCreateFilterIdInternal(appNameId, key);
    traceDataCache_->GetHiSysEventMeasureData()->AppendData(serial, timeStamp, appNameId, appKeyId, type, numericValue,
                                                            strValue);
    return appNameId;
}
void HiSysEventMeasureFilter::AppendNewValue(std::string msg, std::string processName)
{
    traceDataCache_->GetTraceConfigData()->AppendNewData("hisys_event", msg, processName);
    return;
}
void HiSysEventMeasureFilter::AppendNewValue(int32_t brightnessState,
                                             int32_t btState,
                                             int32_t locationState,
                                             int32_t wifiState,
                                             int32_t streamDefault,
                                             int32_t voiceCall,
                                             int32_t music,
                                             int32_t streamRing,
                                             int32_t media,
                                             int32_t voiceAssistant,
                                             int32_t system,
                                             int32_t alarm,
                                             int32_t notification,
                                             int32_t bluetoolthSco,
                                             int32_t enforcedAudible,
                                             int32_t streamDtmf,
                                             int32_t streamTts,
                                             int32_t accessibility,
                                             int32_t recording,
                                             int32_t streamAll)
{
    traceDataCache_->GetHiSysEventDeviceStateData()->AppendNewData(
        brightnessState, btState, locationState, wifiState, streamDefault, voiceCall, music, streamRing, media,
        voiceAssistant, system, alarm, notification, bluetoolthSco, enforcedAudible, streamDtmf, streamTts,
        accessibility, recording, streamAll);
    return;
}
void HiSysEventMeasureFilter::FilterAllHiSysEvent(const json& jMessage, uint64_t serial)
{
    SaveAllHiSysEvent(jMessage);
    size_t maxArraySize = 0;
    JsonData jData;
    std::vector<size_t> noArrayIndex = {};
    std::vector<size_t> arrayIndex = {};
    if (!JGetData(jMessage, jData, maxArraySize, noArrayIndex, arrayIndex)) {
        return;
    }
    DataIndex eventSourceIndex = traceDataCache_->GetDataIndex(jData.eventName);
    if (maxArraySize) {
        NoArrayDataParse(jData, noArrayIndex, eventSourceIndex, serial);
        ArrayDataParse(jData, arrayIndex, eventSourceIndex, maxArraySize, serial);
    } else {
        CommonDataParser(jData, eventSourceIndex, serial);
    }
    return;
}
void HiSysEventMeasureFilter::SaveAllHiSysEvent(json jMessage)
{
    DataIndex domainId = INVALID_DATAINDEX;
    DataIndex eventNameId = INVALID_DATAINDEX;
    uint64_t timeStamp = INVALID_UINT64;
    uint32_t type = INVALID_UINT32;
    std::string timeZone = "";
    uint32_t pid = INVALID_UINT32;
    uint32_t tid = INVALID_UINT32;
    uint32_t uid = INVALID_UINT32;
    std::string level = "";
    std::string tag = "";
    std::string eventId = "";
    uint64_t seq = INVALID_UINT64;
    std::string info = "";
    json content;
    for (auto item = jMessage.begin(); item != jMessage.end(); item++) {
        if (item.key() == "domain_") {
            std::string domainName = item.value();
            domainId = traceDataCache_->GetDataIndex(domainName.c_str());
        } else if (item.key() == "name_") {
            std::string eventName = item.value();
            eventNameId = traceDataCache_->GetDataIndex(eventName.c_str());
        } else if (item.key() == "type_") {
            type = item.value();
        } else if (item.key() == "time_") {
            timeStamp = item.value();
            timeStamp *= MSEC_TO_NS;
        } else if (item.key() == "tz_") {
            timeZone = item.value();
        } else if (item.key() == "pid_") {
            pid = item.value();
        } else if (item.key() == "tid_") {
            tid = item.value();
        } else if (item.key() == "uid_") {
            uid = item.value();
        } else if (item.key() == "id_") {
            eventId = item.value();
        } else if (item.key() == "info_") {
            info = item.value();
        } else if (item.key() == "tag_") {
            tag = item.value();
        } else if (item.key() == "level_") {
            level = item.value();
        } else if (item.key() == "seq_") {
            seq = item.value();
        } else {
            content[item.key()] = item.value();
        }
    }
    auto newTimeStamp = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, timeStamp);
    UpdatePluginTimeRange(TS_CLOCK_BOOTTIME, timeStamp, newTimeStamp);
    traceDataCache_->GetHiSysEventAllEventData()->AppendHiSysEventData(domainId, eventNameId, newTimeStamp, type,
                                                                       timeZone, pid, tid, uid, level, tag, eventId,
                                                                       seq, info, content.dump());
}
bool HiSysEventMeasureFilter::JGetData(const json& jMessage,
                                       JsonData& jData,
                                       size_t& maxArraySize,
                                       std::vector<size_t>& noArrayIndex,
                                       std::vector<size_t>& arrayIndex)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_HISYSEVENT, STAT_EVENT_RECEIVED);
    for (auto subItem = jMessage.begin(); subItem != jMessage.end(); subItem++) {
        if (subItem.key() == "name_") {
            jData.eventName = subItem.value();
            if (find(eventsAccordingAppNames_.begin(), eventsAccordingAppNames_.end(), jData.eventName) ==
                eventsAccordingAppNames_.end()) {
                streamFilters_->statFilter_->IncreaseStat(TRACE_HISYSEVENT, STAT_EVENT_NOTMATCH);
                return false;
            }
            continue;
        }
        if (subItem.key() == "time_") {
            jData.timeStamp = subItem.value();
            continue;
        }
        if (subItem.key() == "tag_" && subItem.value() != "PowerStats") {
            TS_LOGW("energy data without PowerStats tag_ would be invalid");
            return false;
        }
        if (subItem.key() == "APPNAME") {
            jData.appName.assign(subItem.value().begin(), subItem.value().end());
        }
        if (subItem.value().is_array()) {
            maxArraySize = std::max(maxArraySize, subItem.value().size());
            arrayIndex.push_back(jData.key.size());
        } else {
            noArrayIndex.push_back(jData.key.size());
        }
        jData.key.push_back(subItem.key());
        jData.value.push_back(subItem.value());
    }
    jData.timeStamp = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, jData.timeStamp * MSEC_TO_NS);
    return true;
}
void HiSysEventMeasureFilter::NoArrayDataParse(JsonData jData,
                                               std::vector<size_t> noArrayIndex,
                                               DataIndex eventSourceIndex,
                                               uint64_t hiSysEventLineId)
{
    for (auto itor = noArrayIndex.begin(); itor != noArrayIndex.end(); itor++) {
        auto value = jData.value[*itor];
        auto key = jData.key[*itor];
        DataIndex keyIndex = traceDataCache_->GetDataIndex(key);
        AppendStringValue(value, hiSysEventLineId, eventSourceIndex, keyIndex, jData.timeStamp);
    }
}
void HiSysEventMeasureFilter::ArrayDataParse(JsonData jData,
                                             std::vector<size_t> arrayIndex,
                                             DataIndex eventSourceIndex,
                                             size_t maxArraySize,
                                             uint64_t hiSysEventLineId)
{
    for (int32_t i = 0; i < maxArraySize; i++) {
        for (auto itor = arrayIndex.begin(); itor != arrayIndex.end(); itor++) {
            auto value = jData.value[*itor][i];
            std::string key = jData.key[*itor];
            DataIndex keyIndex = traceDataCache_->GetDataIndex(key);
            if (value.is_number()) {
                double valueIndex = value;
                streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(hiSysEventLineId, jData.timeStamp,
                                                                         eventSourceIndex, keyIndex, 0, valueIndex, 0);
            } else if (value.is_string()) {
                std::string strValue = value;
                DataIndex valueIndex = traceDataCache_->GetDataIndex(strValue);
                streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(hiSysEventLineId, jData.timeStamp,
                                                                         eventSourceIndex, keyIndex, 1, 0, valueIndex);
            }
        }
    }
}
void HiSysEventMeasureFilter::AppendStringValue(nlohmann::json& value,
                                                uint64_t hiSysEventLineId,
                                                DataIndex eventSourceIndex,
                                                DataIndex keyIndex,
                                                uint64_t timeStamp)
{
    if (value.is_string()) {
        std::string strValue = value;
        DataIndex valueIndex = traceDataCache_->GetDataIndex(strValue);
        streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(hiSysEventLineId, timeStamp, eventSourceIndex,
                                                                 keyIndex, 1, 0, valueIndex);
    } else {
        double valueIndex = value;
        streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(hiSysEventLineId, timeStamp, eventSourceIndex,
                                                                 keyIndex, 0, valueIndex, 0);
    }
}
void HiSysEventMeasureFilter::CommonDataParser(JsonData jData, DataIndex eventSourceIndex, uint64_t hiSysEventLineId)
{
    for (int32_t i = 0; i < jData.key.size(); i++) {
        std::string key = jData.key[i];
        auto value = jData.value[i];
        DataIndex keyIndex = traceDataCache_->GetDataIndex(key);
        AppendStringValue(value, hiSysEventLineId, eventSourceIndex, keyIndex, jData.timeStamp);
    }
}
DataIndex HiSysEventMeasureFilter::GetOrCreateFilterIdInternal(DataIndex appNameId, DataIndex key)
{
    uint64_t appKeyId = appKey_.Find(appNameId, key);
    if (appKeyId == INVALID_DATAINDEX) {
        appKeyId = traceDataCache_->GetHiSysEventSubkeysData()->AppendSysEventSubkey(appNameId, key);
        appKey_.Insert(appNameId, key, appKeyId);
    }
    return appKeyId;
}

void HiSysEventMeasureFilter::Clear()
{
    appKey_.Clear();
    appName_.Clear();
    eventSource_.clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
