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
    TS_LOGI("hisysevent ts MIN:%llu, MAX:%llu",
            static_cast<unsigned long long>(streamFilters_->hiSysEventMeasureFilter_->GetPluginStartTime()),
            static_cast<unsigned long long>(streamFilters_->hiSysEventMeasureFilter_->GetPluginEndTime()));
    TS_LOGI("hisysevent real ts MIN:%llu, MAX:%llu",
            static_cast<unsigned long long>(streamFilters_->hiSysEventMeasureFilter_->MinTs()),
            static_cast<unsigned long long>(streamFilters_->hiSysEventMeasureFilter_->MaxTs()));
}
void HtraceHisyseventParser::Finish()
{
    auto startTime = streamFilters_->hiSysEventMeasureFilter_->GetPluginStartTime();
    auto endTime = streamFilters_->hiSysEventMeasureFilter_->GetPluginEndTime();
    if (startTime != endTime) {
        traceDataCache_->MixTraceTime(startTime, endTime);
    }
}

static std::stringstream ss;
void HtraceHisyseventParser::Parse(ProtoReader::HisyseventInfo_Reader* tracePacket, uint64_t ts)
{
    // parse hisysevent device state
    if (tracePacket->has_device_state()) {
        ProtoReader::DeviceStat_Reader deviceStat(tracePacket->device_state());
        ProtoReader::AudioVolumeInfo_Reader audioVolumeInfo(deviceStat.volume_state());
        streamFilters_->hiSysEventMeasureFilter_->AppendNewValue(
            deviceStat.brightness_state(), deviceStat.bt_state(), deviceStat.location_state(), deviceStat.wifi_state(),
            audioVolumeInfo.stream_default(), audioVolumeInfo.voice_call(), audioVolumeInfo.music(),
            audioVolumeInfo.stream_ring(), audioVolumeInfo.media(), audioVolumeInfo.voice_assistant(),
            audioVolumeInfo.system(), audioVolumeInfo.alarm(), audioVolumeInfo.notification(),
            audioVolumeInfo.bluetoolth_sco(), audioVolumeInfo.enforced_audible(), audioVolumeInfo.stream_dtmf(),
            audioVolumeInfo.stream_tts(), audioVolumeInfo.accessibility(), audioVolumeInfo.recording(),
            audioVolumeInfo.stream_all());
    }
    // Parse HisyseventLine info

    for (auto i = tracePacket->info(); i; ++i) {
        ProtoReader::HisyseventLine_Reader hisyseventLine(i->ToBytes());
        json jMessage;
        if (!jMessage.accept(hisyseventLine.raw_content().ToStdString())) {
            continue;
        }
        jMessage = json::parse(hisyseventLine.raw_content().ToStdString());
        streamFilters_->hiSysEventMeasureFilter_->FilterAllHiSysEvent(jMessage, hisyseventLine.id());
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
