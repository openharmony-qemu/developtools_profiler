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
#ifndef HTRACE_NATIVE_HOOK_PARSER_H
#define HTRACE_NATIVE_HOOK_PARSER_H
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include "common_types.h"
#include "event_parser_base.h"
#include "htrace_event_parser.h"
#include "htrace_plugin_time_parser.h"
#include "native_hook_filter.h"
#include "offline_symbolization_filter.h"
#include "trace_streamer_config.h"
#include "trace_streamer_filters.h"
namespace SysTuning {
namespace TraceStreamer {
class HtraceNativeHookParser : public EventParserBase, public HtracePluginTimeParser {
public:
    HtraceNativeHookParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~HtraceNativeHookParser();
    void ParseConfigInfo(HtraceDataSegment& dataSeg);
    void Parse(HtraceDataSegment& dataSeg, bool& haveSplitSeg);
    void FinishSplitNativeHook();
    void FinishParseNativeHookData();
    void Finish();
    bool NativeHookReloadElfSymbolTable(const std::vector<std::unique_ptr<SymbolsFile>>& symbolsFile)
    {
        return nativeHookFilter_->NativeHookReloadElfSymbolTable(symbolsFile);
    }

private:
    void ParseNativeHookAuxiliaryEvent(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData);
    void ParseFileEvent(const ProtoReader::BytesView& bytesView);
    void ParseSymbolEvent(const ProtoReader::BytesView& bytesView);
    void ParseThreadEvent(const ProtoReader::BytesView& bytesView);
    void ParseFrameMap(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData);
    bool ParseStackMap(const ProtoReader::BytesView& bytesView);
    void SplitHookData(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData, bool& haveSplitSeg);

private:
    std::vector<std::shared_ptr<const std::string>> segs_ = {};
    std::unique_ptr<NativeHookFilter> nativeHookFilter_;
    uint64_t hookBootTime_ = 0;
    bool isCommData_ = false;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // HTRACE_NATIVE_HOOK_PARSER_H
