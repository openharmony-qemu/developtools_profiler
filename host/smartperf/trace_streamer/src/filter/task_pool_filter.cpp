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

#include "task_pool_filter.h"
#include "parting_string.h"
#include "string_to_numerical.h"

namespace SysTuning {
namespace TraceStreamer {
const uint32_t EXECUTE_DATA_FLAG = 2;

TaskPoolFilter::TaskPoolFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter), IpidExecuteMap_(INVALID_INT32)
{
}
TaskPoolFilter::~TaskPoolFilter() = default;

uint32_t TaskPoolFilter::GetIpId(uint32_t index)
{
    if (index >= traceDataCache_->GetConstInternalSlicesData().CallIds().size()) {
        return INVALID_UINT32;
    }
    auto itid = traceDataCache_->GetConstInternalSlicesData().CallIds()[index];
    auto thread = traceDataCache_->GetThreadData(itid);
    if (!thread) {
        return INVALID_UINT32;
    }
    return thread->internalPid_;
}

uint32_t TaskPoolFilter::CheckTheSameTask(int32_t executeId, uint32_t index)
{
    return IpidExecuteMap_.Find(GetIpId(index), executeId);
}

void TaskPoolFilter::TaskPoolFieldSegmentation(const std::string& taskPoolStr,
                                               std::unordered_map<std::string, std::string>& args)
{
    for (base::PartingString ss(taskPoolStr, ','); ss.Next();) {
        std::string key;
        std::string value;
        for (base::PartingString inner(ss.GetCur(), ':'); inner.Next();) {
            if (key.empty()) {
                key = inner.GetCur();
            } else {
                value = inner.GetCur();
            }
        }
        args.emplace(std::move(key), std::move(value));
    }
}

bool TaskPoolFilter::TaskPoolEvent(const std::string& taskPoolStr, int32_t index)
{
    std::string targetStr = "H:Task ";
    if (!taskPoolStr.compare(0, targetStr.length(), targetStr)) {
        std::unordered_map<std::string, std::string> args;
        std::string allocationStr = "H:Task Allocation: ";
        if (StartWith(taskPoolStr, allocationStr)) {
            allocationStr = taskPoolStr.substr(allocationStr.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(allocationStr, args);
            return UpdateAssignData(args, index);
        }
        std::string executeStr = "H:Task Perform: ";
        if (StartWith(taskPoolStr, executeStr)) {
            executeStr = taskPoolStr.substr(executeStr.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(executeStr, args);
            return UpdateExecuteData(args, index);
        }
        std::string returnStr = "H:Task PerformTask End: ";
        if (StartWith(taskPoolStr, returnStr)) {
            returnStr = taskPoolStr.substr(returnStr.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(returnStr, args);
            return UpdateReturnData(args, index);
        }
    }
    return false;
}

bool TaskPoolFilter::UpdateAssignData(const std::unordered_map<std::string, std::string>& args, int32_t index)
{
    if (index >= traceDataCache_->GetConstInternalSlicesData().CallIds().size()) {
        return false;
    }
    auto allocItid = traceDataCache_->GetConstInternalSlicesData().CallIds()[index];
    auto executeId = base::StrToInt<int32_t>(args.at(" executeId "));
    auto priority = base::StrToInt<uint32_t>(args.at(" priority "));
    auto executeState = base::StrToInt<uint32_t>(args.at(" executeState "));

    int32_t returnValue = CheckTheSameTask(executeId.value(), index);
    if (returnValue == INVALID_INT32) {
        int32_t taskIndex = traceDataCache_->GetTaskPoolData()->AppendAllocationTaskData(
            index, allocItid, executeId.value(), priority.value(), executeState.value());
        IpidExecuteMap_.Insert(GetIpId(index), executeId.value(), taskIndex);
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateAllocationTaskData(returnValue, index, allocItid,
                                                                     priority.value(), executeState.value());
    }
    return true;
}

bool TaskPoolFilter::UpdateExecuteData(const std::unordered_map<std::string, std::string>& args, int32_t index)
{
    if (index >= traceDataCache_->GetConstInternalSlicesData().CallIds().size()) {
        return false;
    }
    auto executeItid = traceDataCache_->GetConstInternalSlicesData().CallIds()[index];
    auto executeId = base::StrToInt<int32_t>(args.at(" executeId "));

    int32_t returnValue = CheckTheSameTask(executeId.value(), index);
    if (returnValue == INVALID_INT32) {
        int32_t taskIndex =
            traceDataCache_->GetTaskPoolData()->AppendExecuteTaskData(index, executeItid, executeId.value());
        IpidExecuteMap_.Insert(GetIpId(index), executeId.value(), taskIndex);
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateExecuteTaskData(returnValue, index, executeItid);
    }
    return true;
}

bool TaskPoolFilter::UpdateReturnData(const std::unordered_map<std::string, std::string>& args, int32_t index)
{
    if (index >= traceDataCache_->GetConstInternalSlicesData().CallIds().size()) {
        return false;
    }
    auto returnItid = traceDataCache_->GetConstInternalSlicesData().CallIds()[index];
    auto executeId = base::StrToInt<int32_t>(args.at(" executeId "));
    auto returnStr = std::string_view(args.at(" performResult "));
    int32_t returnState = returnStr.compare(" Successful") ? 0 : 1;

    int32_t returnValue = CheckTheSameTask(executeId.value(), index);
    if (returnValue == INVALID_INT32) {
        int32_t taskIndex = traceDataCache_->GetTaskPoolData()->AppendReturnTaskData(index, returnItid,
                                                                                     executeId.value(), returnState);
        IpidExecuteMap_.Insert(GetIpId(index), executeId.value(), taskIndex);
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateReturnTaskData(returnValue, index, returnItid, returnState);
    }
    return true;
}

} // namespace TraceStreamer
} // namespace SysTuning
