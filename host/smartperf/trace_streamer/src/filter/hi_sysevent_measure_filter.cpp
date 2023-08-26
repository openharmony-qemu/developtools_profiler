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
    traceDataCache_->GetSyseventMeasureData()->AppendData(serial, timeStamp, appNameId, appKeyId, type, numericValue,
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
    traceDataCache_->GetDeviceStateData()->AppendNewData(
        brightnessState, btState, locationState, wifiState, streamDefault, voiceCall, music, streamRing, media,
        voiceAssistant, system, alarm, notification, bluetoolthSco, enforcedAudible, streamDtmf, streamTts,
        accessibility, recording, streamAll);
    return;
}
bool HiSysEventMeasureFilter::JGetData(const json& jMessage,
                                       JsonData& jData,
                                       size_t& maxArraySize,
                                       std::vector<size_t>& noArrayIndex,
                                       std::vector<size_t>& arrayIndex)
{
    streamFilters_->statFilter_->IncreaseStat(TRACE_HISYSEVENT, STAT_EVENT_RECEIVED);
    for (auto i = jMessage.begin(); i != jMessage.end(); i++) {
        if (i.key() == "name_") {
            jData.eventSource = i.value();
            if (find(eventsAccordingAppNames_.begin(), eventsAccordingAppNames_.end(), jData.eventSource) ==
                eventsAccordingAppNames_.end()) {
                streamFilters_->statFilter_->IncreaseStat(TRACE_HISYSEVENT, STAT_EVENT_NOTMATCH);
                TS_LOGW("event source:%s not supported for hisysevent", jData.eventSource.c_str());
                return false;
            }
            continue;
        }
        if (i.key() == "time_") {
            jData.timeStamp = i.value();
            continue;
        }
        if (i.key() == "tag_" && i.value() != "PowerStats") {
            TS_LOGW("energy data without PowerStats tag_ would be invalid");
            return false;
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
    return true;
}
DataIndex HiSysEventMeasureFilter::GetOrCreateFilterIdInternal(DataIndex appNameId, DataIndex key)
{
    uint64_t appKeyId = appKey_.Find(appNameId, key);
    if (appKeyId == INVALID_DATAINDEX) {
        appKeyId = traceDataCache_->GetAppNamesData()->AppendAppName(1, appNameId, key);
        appKey_.Insert(appNameId, key, appKeyId);
    }
    return appKeyId;
}

DataIndex HiSysEventMeasureFilter::GetOrCreateFilterId(DataIndex eventSource)
{
    DataIndex eventSourceFilterId = INVALID_DATAINDEX;
    if (eventSource_.find(eventSource) == eventSource_.end()) {
        eventSourceFilterId = streamFilters_->sysEventSourceFilter_->AppendNewMeasureFilter(eventSource);
        eventSource_.insert(std::make_pair(eventSource, eventSourceFilterId));
    } else {
        eventSourceFilterId = eventSource_.at(eventSource);
    }
    return eventSourceFilterId;
}
DataIndex HiSysEventMeasureFilter::GetOrCreateFilterId(DataIndex eventSource, DataIndex appName)
{
    DataIndex eventSourceFilterId = INVALID_DATAINDEX;
    DataIndex appNameId = INVALID_DATAINDEX;
    if (eventSource_.find(eventSource) == eventSource_.end()) {
        eventSourceFilterId = streamFilters_->sysEventSourceFilter_->AppendNewMeasureFilter(eventSource);
        eventSource_.insert(std::make_pair(eventSource, eventSourceFilterId));
    } else {
        eventSourceFilterId = eventSource_.at(eventSource);
    }
    appNameId = appName_.Find(eventSourceFilterId, appName);
    if (appNameId == INVALID_DATAINDEX) {
        appNameId = traceDataCache_->GetAppNamesData()->AppendAppName(0, eventSourceFilterId, appName);
        appName_.Insert(eventSourceFilterId, appName, appNameId);
    }
    return appNameId;
}
std::tuple<DataIndex, DataIndex> HiSysEventMeasureFilter::GetOrCreateFilterId(DataIndex eventSource,
                                                                              DataIndex appName,
                                                                              DataIndex key)
{
    DataIndex appNameId = GetOrCreateFilterId(eventSource, appName);
    uint64_t appKeyId = appKey_.Find(appNameId, key);
    if (appKeyId == INVALID_DATAINDEX) {
        appKeyId = traceDataCache_->GetAppNamesData()->AppendAppName(1, appNameId, key);
        appKey_.Insert(appNameId, key, appKeyId);
    }
    return std::make_tuple(appNameId, appKeyId);
}

void HiSysEventMeasureFilter::Clear()
{
    appKey_.Clear();
    appName_.Clear();
    eventSource_.clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
