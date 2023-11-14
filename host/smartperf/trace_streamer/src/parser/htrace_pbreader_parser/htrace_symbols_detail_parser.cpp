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
#include "htrace_symbols_detail_parser.h"
#include "htrace_event_parser.h"
#include "symbols_filter.h"
namespace SysTuning {
namespace TraceStreamer {
HtraceSymbolsDetailParser::HtraceSymbolsDetailParser(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : streamFilters_(ctx), traceDataCache_(dataCache)
{
    UNUSED(traceDataCache_);
    if (!streamFilters_) {
        TS_LOGF("streamFilters_ should not be null");
        return;
    }
}

HtraceSymbolsDetailParser::~HtraceSymbolsDetailParser() = default;
void HtraceSymbolsDetailParser::Parse(ProtoReader::BytesView tracePacket)
{
    if (traceDataCache_->isSplitFile_) {
        return;
    }
    ProtoReader::TracePluginResult_Reader reader((const uint8_t*)(tracePacket.data_), tracePacket.size_);
    if (!reader.has_symbols_detail()) {
        return;
    }
    for (auto i = reader.symbols_detail(); i; ++i) {
        ProtoReader::SymbolsDetailMsg_Reader reader(i->ToBytes());
        streamFilters_->symbolsFilter_->RegisterFunc(reader.symbol_addr(),
                                                     traceDataCache_->GetDataIndex(reader.symbol_name().ToStdString()));
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
