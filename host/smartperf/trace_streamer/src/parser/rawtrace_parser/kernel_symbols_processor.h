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
#ifndef KERNEL_SYMBOLS_PROCESSOR_H
#define KERNEL_SYMBOLS_PROCESSOR_H

#include <functional>
#include <string>
#include <vector>
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class KernelSymbolsProcessor {
public:
    KernelSymbolsProcessor(TraceDataCache* dataCache, const TraceStreamerFilters* filters);
    ~KernelSymbolsProcessor();

    bool HandleKallSyms(const std::string& kallsyms);

private:
    struct KernelSymbol {
        char type{0};
        uint64_t addr{0};
        std::string name;
    };

private:
    static bool IsValidKernelSymbol(const KernelSymbol& symbol);
    static bool CompareSymbolInfo(const KernelSymbol& a, const KernelSymbol& b);

private:
    const TraceStreamerFilters* streamFilters_;
    TraceDataCache* traceDataCache_;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // KERNEL_SYMBOLS_PROCESSOR_H
