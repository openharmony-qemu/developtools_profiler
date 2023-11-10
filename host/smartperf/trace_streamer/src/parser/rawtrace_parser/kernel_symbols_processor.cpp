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
#include "kernel_symbols_processor.h"

#include "log.h"
#include "symbols_filter.h"
#include "string_help.h"
#include "string_to_numerical.h"

namespace SysTuning {
namespace TraceStreamer {
KernelSymbolsProcessor::KernelSymbolsProcessor(TraceDataCache* dataCache, const TraceStreamerFilters* filters)
    : traceDataCache_(dataCache), streamFilters_(filters)
{
    if (!streamFilters_) {
        TS_LOGE("streamFilters_ should not be null");
        return;
    }
}

KernelSymbolsProcessor::~KernelSymbolsProcessor()
{
    TS_LOGI("KernelSymbolsProcessor destroy!");
}

bool KernelSymbolsProcessor::IsValidKernelSymbol(const KernelSymbol& symbol)
{
    if (symbol.addr == 0 || symbol.name.empty()) {
        return false;
    }
    if (symbol.name[0] == '$') {
        return false;
    }
    // eg: ffffffc010b8daa4 t bio_complete
    if (symbol.type != 't' && symbol.type != 'T') {
        return false;
    }
    return true;
}

bool KernelSymbolsProcessor::CompareSymbolInfo(const KernelSymbol& firstSymbol, const KernelSymbol& secondSymbol)
{
    if (firstSymbol.addr != secondSymbol.addr) {
        return firstSymbol.addr < secondSymbol.addr;
    }
    if (firstSymbol.name != secondSymbol.name) {
        return firstSymbol.name < secondSymbol.name;
    }
    return firstSymbol.type < secondSymbol.type;
}

bool KernelSymbolsProcessor::HandleKallSyms(const std::string& kallsyms)
{
    TS_CHECK_TRUE(!kallsyms.empty(), false, "kallsyms is empty!");
    std::stringstream symsStream(kallsyms);
    std::string line;
    KernelSymbol symbol;
    std::string addrStr;
    std::stringstream strStream;
    while (std::getline(symsStream, line)) {
        strStream.clear();
        strStream.str(line);
        if (strStream >> addrStr >> symbol.type >> symbol.name) {
            symbol.addr = base::StrToInt<uint64_t>(addrStr, base::INTEGER_RADIX_TYPE_HEX).value();
        }
        if (symbol.addr == 0) {
            continue;
        }
        if (EndWith(symbol.name, ".cfi")) {
            symbol.name = symbol.name.substr(0, symbol.name.size() - (sizeof(".cfi") - 1));
        }
        if (IsValidKernelSymbol(symbol)) {
            streamFilters_->symbolsFilter_->RegisterFunc(symbol.addr, traceDataCache_->GetDataIndex(symbol.name));
        }
    }
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning
