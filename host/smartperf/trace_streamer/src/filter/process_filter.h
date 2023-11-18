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

#ifndef PROCESS_FILTER_H
#define PROCESS_FILTER_H

#include <tuple>

#include "filter_base.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class ProcessFilter : private FilterBase {
public:
    ProcessFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    ~ProcessFilter() override;

    uint32_t UpdateOrCreateProcessWithName(uint32_t pid, std::string_view name);
    uint32_t UpdateOrCreateThreadWithName(uint64_t timeStamp, uint32_t tid, std::string_view name);
    uint32_t UpdateOrCreateThreadWithPidAndName(uint32_t tid, uint32_t pid, std::string_view name);
    uint32_t GetOrCreateThreadWithPid(uint32_t tid, uint32_t pid);
    uint32_t UpdateOrCreateThread(uint64_t timeStamp, uint32_t tid);
    InternalPid GetInternalPid(uint32_t pid) const;
    InternalPid GetOrCreateInternalPid(uint64_t timeStamp, uint32_t pid);
    bool IsThreadNameEmpty(uint32_t tid) const;
    InternalTid GetInternalTid(uint32_t tid) const;
    std::vector<InternalTid>& GetInternalTids(uint32_t tid);
    uint32_t UpdateOrCreateThreadWithNameIndex(uint64_t timeStamp, uint32_t tid, DataIndex threadNameIndex);
    void AddProcessMemory(uint32_t ipid);
    void AddThreadSliceNum(uint32_t itid);
    void AddCpuStateCount(uint32_t itid);
    void AddProcessSliceNum(uint32_t ipid);
    void Clear();

private:
    std::tuple<uint32_t, TraceStdtype::Process*> CreateProcessMaybe(uint32_t pid, uint64_t startT);
    std::tuple<uint32_t, TraceStdtype::Thread*> NewThread(uint32_t tid);
    std::tuple<uint32_t, TraceStdtype::Process*> NewProcess(uint32_t pid);

    InternalTid GetInternalTid(uint32_t tid, uint32_t pid) const;

private:
    std::multimap<uint32_t, uint32_t> tidMappingSet_ = {};
    std::vector<InternalTid> tmpTids_ = {};
    std::map<uint32_t, uint32_t> pidToInternalPidMap_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // PROCESS_FILTER_H
