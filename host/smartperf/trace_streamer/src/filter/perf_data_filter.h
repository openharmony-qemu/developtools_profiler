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

#ifndef PERF_DATA_FILTER_H
#define PERF_DATA_FILTER_H
#include <map>
#include <set>
#include <unordered_set>
#include "double_map.h"
#include "filter_base.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"
namespace SysTuning {
namespace TraceStreamer {
class PerfDataFilter : private FilterBase {
public:
    PerfDataFilter(TraceDataCache*, const TraceStreamerFilters*);
    PerfDataFilter(const PerfDataFilter&) = delete;
    PerfDataFilter& operator=(const PerfDataFilter&) = delete;
    ~PerfDataFilter() override;

public:
    size_t AppendPerfFiles(uint64_t fileId, uint32_t serial, DataIndex symbols, DataIndex filePath);
    void AppendPerfCallChain(uint32_t callChainId,
                             uint32_t depth,
                             uint64_t ip,
                             uint64_t vaddrInFile,
                             uint64_t fileId,
                             uint64_t symbolId);
    void Finish();
    void BeforeReload();

private:
    DoubleMap<uint64_t, uint32_t, uint64_t> fileIdToRowInFileTable_;
    std::set<uint64_t> fileIds_;
    std::map<uint64_t, uint64_t> fileIdToRow_{};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // PERF_DATA_FILTER_H
