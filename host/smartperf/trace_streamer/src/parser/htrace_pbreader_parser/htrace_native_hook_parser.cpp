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
constexpr static uint32_t MAX_PROTO_BUFFER_SIZE = 4 * 1024 * 1024;

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

bool HtraceNativeHookParser::ParseStackMap(const ProtoReader::BytesView& bytesView)
{
    if (traceDataCache_->isSplitFile_) {
        auto hookData = nativeHookFilter_->GetCommHookData().datas->add_events();
        StackMap* stackMap = hookData->mutable_stack_map();
        stackMap->ParseFromArray(bytesView.Data(), bytesView.Size());
        nativeHookFilter_->GetCommHookData().size += bytesView.Size();
        return false;
    }
    ProtoReader::StackMap_Reader stackMapReader(bytesView);
    bool parseError = false;
    auto ipid = streamFilters_->processFilter_->UpdateOrCreateProcessWithName(stackMapReader.pid(), "");
    // stores frames info. if offlineSymbolization is true, storing ips data, else storing FrameMap id.
    std::vector<uint64_t> frames;
    if (stackMapReader.has_frame_map_id()) {
        auto itor = stackMapReader.frame_map_id(&parseError);
        TS_CHECK_TRUE(!parseError, false, "Parse packed varInt in ParseStackMap function failed!!!");
        while (itor) {
            frames.emplace_back(*itor);
            itor++;
        }
    } else if (stackMapReader.has_ip()) {
        auto itor = stackMapReader.ip(&parseError);
        TS_CHECK_TRUE(!parseError, false, "Parse packed varInt in ParseStackMap function failed!!!");
        // OfflineSymbolization use ipidToStartAddrToMapsInfoMap_ Multi-process differentiation
        while (itor) {
            frames.emplace_back(*itor);
            itor++;
        }
    }
    nativeHookFilter_->AppendStackMaps(ipid, stackMapReader.id(), frames);
    return true;
}

void HtraceNativeHookParser::ParseFrameMap(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData)
{
    segs_.emplace_back(nativeHookMetaData->seg_);
    const ProtoReader::BytesView& frameMapByteView = nativeHookMetaData->reader_->frame_map();
    if (traceDataCache_->isSplitFile_) {
        auto hookData = nativeHookFilter_->GetCommHookData().datas->add_events();
        FrameMap* frameMap = hookData->mutable_frame_map();
        frameMap->ParseFromArray(frameMapByteView.Data(), frameMapByteView.Size());
        nativeHookFilter_->GetCommHookData().size += frameMapByteView.Size();
        return;
    }
    ProtoReader::FrameMap_Reader frameMapReader(frameMapByteView);
    auto ipid = streamFilters_->processFilter_->UpdateOrCreateProcessWithName(frameMapReader.pid(), "");
    // when callstack is compressed, Frame message only has ip data area.
    nativeHookFilter_->AppendFrameMaps(ipid, frameMapReader.id(), frameMapReader.frame());
}
void HtraceNativeHookParser::ParseFileEvent(const ProtoReader::BytesView& bytesView)
{
    if (traceDataCache_->isSplitFile_) {
        auto hookData = nativeHookFilter_->GetCommHookData().datas->add_events();
        FilePathMap* filePathMap = hookData->mutable_file_path();
        filePathMap->ParseFromArray(bytesView.Data(), bytesView.Size());
        nativeHookFilter_->GetCommHookData().size += bytesView.Size();
        return;
    }
    ProtoReader::FilePathMap_Reader filePathMapReader(bytesView);
    auto ipid = streamFilters_->processFilter_->UpdateOrCreateProcessWithName(filePathMapReader.pid(), "");
    auto nameIndex = traceDataCache_->dataDict_.GetStringIndex(filePathMapReader.name().ToStdString());
    nativeHookFilter_->AppendFilePathMaps(ipid, filePathMapReader.id(), nameIndex);
}
void HtraceNativeHookParser::ParseSymbolEvent(const ProtoReader::BytesView& bytesView)
{
    if (traceDataCache_->isSplitFile_) {
        auto hookData = nativeHookFilter_->GetCommHookData().datas->add_events();
        SymbolMap* symbolMap = hookData->mutable_symbol_name();
        symbolMap->ParseFromArray(bytesView.Data(), bytesView.Size());
        nativeHookFilter_->GetCommHookData().size += bytesView.Size();
        return;
    }
    ProtoReader::SymbolMap_Reader symbolMapReader(bytesView);
    auto ipid = streamFilters_->processFilter_->UpdateOrCreateProcessWithName(symbolMapReader.pid(), "");
    auto nameIndex = traceDataCache_->dataDict_.GetStringIndex(symbolMapReader.name().ToStdString());
    nativeHookFilter_->AppendSymbolMap(ipid, symbolMapReader.id(), nameIndex);
}
void HtraceNativeHookParser::ParseThreadEvent(const ProtoReader::BytesView& bytesView)
{
    if (traceDataCache_->isSplitFile_) {
        auto hookData = nativeHookFilter_->GetCommHookData().datas->add_events();
        ThreadNameMap* threadNameMap = hookData->mutable_thread_name_map();
        threadNameMap->ParseFromArray(bytesView.Data(), bytesView.Size());
        nativeHookFilter_->GetCommHookData().size += bytesView.Size();
        return;
    }
    ProtoReader::ThreadNameMap_Reader threadNameMapReader(bytesView);
    auto ipid = streamFilters_->processFilter_->UpdateOrCreateProcessWithName(threadNameMapReader.pid(), "");
    auto nameIndex = traceDataCache_->GetDataIndex(threadNameMapReader.name().ToStdString());
    nativeHookFilter_->AppendThreadNameMap(ipid, threadNameMapReader.id(), nameIndex);
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
    } else if (reader->has_tag_event()) {
        nativeHookFilter_->ParseTagEvent(reader->tag_event());
    } else {
        TS_LOGE("unsupported native_hook data!");
    }
}
void HtraceNativeHookParser::SplitHookData(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData, bool& haveSplitSeg)
{
    if (isCommData_ && hookBootTime_ <= traceDataCache_->SplitFileMinTime()) {
        ParseNativeHookAuxiliaryEvent(nativeHookMetaData);
    } else if (hookBootTime_ >= traceDataCache_->SplitFileMinTime() &&
               hookBootTime_ <= traceDataCache_->SplitFileMaxTime()) {
        haveSplitSeg = true;
    }
}
// In order to improve the accuracy of data, it is necessary to sort the original data.
// Data sorting will be reduced by 5% to 10% Speed of parsing data.
void HtraceNativeHookParser::Parse(HtraceDataSegment& dataSeg, bool& haveSplitSeg)
{
    auto batchNativeHookDataReader = ProtoReader::BatchNativeHookData_Reader(dataSeg.protoData);
    for (auto itor = batchNativeHookDataReader.events(); itor; itor++) {
        auto nativeHookDataReader = std::make_unique<ProtoReader::NativeHookData_Reader>(itor->ToBytes());
        auto nativeHookMetaData = std::make_unique<NativeHookMetaData>(dataSeg.seg, std::move(nativeHookDataReader));
        isCommData_ =
            !(nativeHookMetaData->reader_->has_alloc_event() || nativeHookMetaData->reader_->has_free_event() ||
              nativeHookMetaData->reader_->has_mmap_event() || nativeHookMetaData->reader_->has_munmap_event() ||
              nativeHookMetaData->reader_->has_statistics_event());
        hookBootTime_ = 0;
        if (nativeHookMetaData->reader_->has_tv_sec() || nativeHookMetaData->reader_->has_tv_nsec()) {
            auto timeStamp = nativeHookMetaData->reader_->tv_nsec() + nativeHookMetaData->reader_->tv_sec() * SEC_TO_NS;
            hookBootTime_ = streamFilters_->clockFilter_->ToPrimaryTraceTime(TS_CLOCK_REALTIME, timeStamp);
            UpdatePluginTimeRange(TS_CLOCK_REALTIME, timeStamp, hookBootTime_);
        }
        if (haveSplitSeg) {
            return;
        } else if (traceDataCache_->isSplitFile_) {
            SplitHookData(nativeHookMetaData, haveSplitSeg);
            continue;
        }
        if (!isCommData_ || nativeHookMetaData->reader_->has_tag_event()) {
            nativeHookFilter_->MaybeParseNativeHookMainEvent(hookBootTime_, std::move(nativeHookMetaData));
        } else {
            ParseNativeHookAuxiliaryEvent(nativeHookMetaData);
        }
    }
    if (!traceDataCache_->isSplitFile_ || nativeHookFilter_->GetCommHookData().size < MAX_PROTO_BUFFER_SIZE) {
        return;
    }
    nativeHookFilter_->SerializeHookCommDataToString();
}
void HtraceNativeHookParser::ParseConfigInfo(HtraceDataSegment& dataSeg)
{
    nativeHookFilter_->ParseConfigInfo(dataSeg.protoData);
}
void HtraceNativeHookParser::FinishSplitNativeHook()
{
    nativeHookFilter_->SerializeHookCommDataToString();
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
