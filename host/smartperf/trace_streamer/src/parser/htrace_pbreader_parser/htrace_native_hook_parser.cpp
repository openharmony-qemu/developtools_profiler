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
#include "htrace_native_hook_parser.h"
#include "clock_filter_ex.h"
#include "process_filter.h"
#include "stat_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceNativeHookParser::HtraceNativeHookParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx), nativeHookFilter_(std::make_unique<NativeHookFilter>(dataCache, ctx))
{
}

HtraceNativeHookParser::~HtraceNativeHookParser()
{
    TS_LOGI("native hook data ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(GetPluginStartTime()),
            static_cast<unsigned long long>(GetPluginEndTime()));
    TS_LOGI("native real ts MIN:%llu, MAX:%llu", static_cast<unsigned long long>(MinTs()),
            static_cast<unsigned long long>(MaxTs()));
}

void HtraceNativeHookParser::ParseStackMap(const ProtoReader::BytesView& bytesView)
{
    ProtoReader::StackMap_Reader stackMapReader(bytesView);
    auto stackId = stackMapReader.id();
    bool parseError = false;
    // stores frames info. if offlineSymbolization is true, storing ips data, else storing FrameMap id.
    std::vector<uint64_t> frames;
    if (stackMapReader.has_frame_map_id()) {
        auto itor = stackMapReader.frame_map_id(&parseError);
        if (parseError) {
            TS_LOGE("Parse packed varInt in ParseStackMap function failed!!!");
            return;
        }
        while (itor) {
            frames.emplace_back(*itor);
            itor++;
        }
    } else if (stackMapReader.has_ip()) {
        auto itor = stackMapReader.ip(&parseError);
        if (parseError) {
            TS_LOGE("Parse packed varInt in ParseStackMap function failed!!!");
            return;
        }
        while (itor) {
            frames.emplace_back(*itor);
            itor++;
        }
    }
    nativeHookFilter_->AppendStackMaps(stackId, frames);
    return;
}

void HtraceNativeHookParser::ParseFrameMap(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData)
{
    segs_.emplace_back(nativeHookMetaData->seg_);
    ProtoReader::FrameMap_Reader frameMapReader(nativeHookMetaData->reader_->frame_map());
    // when callstack is compressed, Frame message only has ip data area.
    nativeHookFilter_->AppendFrameMaps(frameMapReader.id(), frameMapReader.frame());
}
void HtraceNativeHookParser::ParseFileEvent(const ProtoReader::BytesView& bytesView)
{
    ProtoReader::FilePathMap_Reader filePathMapReader(bytesView);
    auto id = filePathMapReader.id();
    auto nameIndex = traceDataCache_->dataDict_.GetStringIndex(filePathMapReader.name().ToStdString());
    nativeHookFilter_->AppendFilePathMaps(id, nameIndex);
}
void HtraceNativeHookParser::ParseSymbolEvent(const ProtoReader::BytesView& bytesView)
{
    ProtoReader::SymbolMap_Reader symbolMapReader(bytesView);
    auto id = symbolMapReader.id();
    auto nameIndex = traceDataCache_->dataDict_.GetStringIndex(symbolMapReader.name().ToStdString());
    nativeHookFilter_->AppendSymbolMap(id, nameIndex);
}
void HtraceNativeHookParser::ParseThreadEvent(const ProtoReader::BytesView& bytesView)
{
    ProtoReader::ThreadNameMap_Reader threadNameMapReader(bytesView);
    auto id = threadNameMapReader.id();
    auto nameIndex = traceDataCache_->dataDict_.GetStringIndex(threadNameMapReader.name().ToStdString());
    nativeHookFilter_->AppendThreadNameMap(id, nameIndex);
}

void HtraceNativeHookParser::ParseNativeHookAuxiliaryEvent(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData)
{
    auto& reader = nativeHookMetaData->reader_;
    if (reader->has_stack_map()) {
        ParseStackMap(reader->stack_map());
    } else if (reader->has_frame_map()) {
        ParseFrameMap(nativeHookMetaData);
    } else if (reader->has_file_path()) {
        ParseFileEvent(reader->file_path());
    } else if (reader->has_symbol_name()) {
        ParseSymbolEvent(reader->symbol_name());
    } else if (reader->has_thread_name_map()) {
        ParseThreadEvent(reader->thread_name_map());
    } else if (reader->has_maps_info()) {
        nativeHookFilter_->ParseMapsEvent(nativeHookMetaData);
    } else if (reader->has_symbol_tab()) {
        nativeHookFilter_->ParseSymbolTableEvent(nativeHookMetaData);
    } else {
        TS_LOGE("unsupported native_hook data!");
    }
}
// In order to improve the accuracy of data, it is necessary to sort the original data.
// Data sorting will be reduced by 5% to 10% Speed of parsing data.
void HtraceNativeHookParser::Parse(HtraceDataSegment& dataSeg)
{
    auto batchNativeHookDataReader = ProtoReader::BatchNativeHookData_Reader(dataSeg.protoData);
    for (auto itor = batchNativeHookDataReader.events(); itor; itor++) {
        auto nativeHookDataReader = std::make_unique<ProtoReader::NativeHookData_Reader>(itor->ToBytes());
        auto timeStamp = nativeHookDataReader->tv_nsec() + nativeHookDataReader->tv_sec() * SEC_TO_NS;
        if (nativeHookDataReader->has_alloc_event() || nativeHookDataReader->has_free_event() ||
            nativeHookDataReader->has_mmap_event() || nativeHookDataReader->has_munmap_event() ||
            nativeHookDataReader->has_statistics_event() || nativeHookDataReader->has_tag_event()) {
            uint64_t newTimeStamp = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, timeStamp);
            UpdatePluginTimeRange(TS_CLOCK_REALTIME, timeStamp, newTimeStamp);
            auto nativeHookMetaData =
                std::make_unique<NativeHookMetaData>(dataSeg.seg, std::move(nativeHookDataReader));
            nativeHookFilter_->MaybeParseNativeHookMainEvent(newTimeStamp, std::move(nativeHookMetaData));
        } else {
            auto nativeHookMetaData =
                std::make_unique<NativeHookMetaData>(dataSeg.seg, std::move(nativeHookDataReader));
            ParseNativeHookAuxiliaryEvent(nativeHookMetaData);
        }
    }
}
void HtraceNativeHookParser::ParseConfigInfo(HtraceDataSegment& dataSeg)
{
    nativeHookFilter_->ParseConfigInfo(dataSeg.protoData);
}
void HtraceNativeHookParser::FinishParseNativeHookData()
{
    nativeHookFilter_->FinishParseNativeHookData();
}
void HtraceNativeHookParser::Finish()
{
    if (GetPluginStartTime() != GetPluginEndTime()) {
        traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginEndTime());
    } else {
        traceDataCache_->MixTraceTime(GetPluginStartTime(), GetPluginStartTime() + 1);
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
