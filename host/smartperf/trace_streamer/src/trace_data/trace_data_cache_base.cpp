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

#include "trace_data_cache_base.h"
#include <string_view>

namespace SysTuning {
namespace TraceStreamer {
TraceDataCacheBase::TraceDataCacheBase()
{
    internalProcessesData_.emplace_back(0);
    internalThreadsData_.emplace_back(0);
    internalThreadsData_.front().internalPid_ = 0;
    GetDataIndex("");
    for (auto it = statusString_.begin(); it != statusString_.end(); it++) {
        threadStatus2Value_[it->second] = it->first;
    }
}
DataIndex TraceDataCacheBase::GetDataIndex(std::string_view str)
{
    return dataDict_.GetStringIndex(str);
}

DataIndex TraceDataCacheBase::GetConstDataIndex(std::string_view str) const
{
    return dataDict_.GetStringIndexNoWrite(str);
}
void TraceDataCacheBase::UpdataZeroThreadInfo()
{
    const std::string ZERO_THREAD_NAME = "swapper";
    internalProcessesData_.front().cmdLine_ = ZERO_THREAD_NAME;
    auto& thread = internalThreadsData_.front();
    thread.internalPid_ = 0;
    thread.nameIndex_ = dataDict_.GetStringIndex(ZERO_THREAD_NAME);
}
} // namespace TraceStreamer
} // namespace SysTuning
