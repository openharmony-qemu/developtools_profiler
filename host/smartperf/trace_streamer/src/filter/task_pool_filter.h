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

#ifndef TAKS_POOL_FILTER_H
#define TAKS_POOL_FILTER_H

#include <map>
#include <string_view>
#include <unordered_map>

#include "filter_base.h"
#include "string_help.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"

namespace SysTuning {
namespace TraceStreamer {
class TaskPoolFilter : private FilterBase {
public:
    TaskPoolFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    TaskPoolFilter(const TaskPoolFilter&) = delete;
    TaskPoolFilter& operator=(const TaskPoolFilter&) = delete;
    ~TaskPoolFilter() override;
    uint32_t GetIpId(uint32_t index);
    uint32_t CheckTheSameTask(uint32_t executeId, uint32_t index);
    bool TaskPoolEvent(const std::string& taskPoolStr, uint32_t index);
    void TaskPoolFieldSegmentation(const std::string& taskPoolStr, std::unordered_map<std::string, std::string>& args);
    bool UpdateAssignData(const std::unordered_map<std::string, std::string>& args, uint32_t index);
    bool UpdateExecuteData(const std::unordered_map<std::string, std::string>& args, uint32_t index);
    bool UpdateReturnData(const std::unordered_map<std::string, std::string>& args, uint32_t index);
    bool AppendTimeoutRow(uint32_t index);

private:
    const std::string targetStr_ = "H:Task ";
    const std::string allocationStr_ = "H:Task Allocation: ";
    const std::string executeStr_ = "H:Task Perform: ";
    const std::string returnStr_ = "H:Task PerformTask End: ";
    const std::string timeoutStr_ = "H:Thread Timeout Exit";
    DoubleMap<InternalPid, uint32_t, uint32_t> IpidExecuteMap_;
    std::unordered_map<uint32_t, uint32_t> timeoutMap_;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // TAKS_POOL_FILTER_H
