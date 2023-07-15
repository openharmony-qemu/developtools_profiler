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
    : FilterBase(dataCache, filter)
{
}
TaskPoolFilter::~TaskPoolFilter() = default;

uint32_t TaskPoolFilter::CheckTheSameTask(int32_t executeId)
{
    if (executeMap_.count(executeId)) {
        return executeMap_[executeId];
    }
    return INVALID_INT32;
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

void TaskPoolFilter::TaskPoolEvent(const std::string& taskPoolStr, int32_t index)
{
    std::string targetStr = "H:Task ";
    if (!taskPoolStr.compare(0, targetStr.length(), targetStr)) {
        std::unordered_map<std::string, std::string> args;
        std::string allocationStr = "H:Task Allocation: ";
        if (StartWith(taskPoolStr, allocationStr)) {
            allocationStr = taskPoolStr.substr(allocationStr.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(allocationStr, args);
            UpdateAssignData(args, index);
            return;
        }
        std::string executeStr = "H:Task Perform: ";
        if (StartWith(taskPoolStr, executeStr)) {
            executeStr = taskPoolStr.substr(executeStr.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(executeStr, args);
            UpdateExecuteData(args, index);
            return;
        }
        std::string returnStr = "H:Task PerformTask End: ";
        if (StartWith(taskPoolStr, returnStr)) {
            returnStr = taskPoolStr.substr(returnStr.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(returnStr, args);
            UpdateReturnData(args, index);
            return;
        }
    }
}

void TaskPoolFilter::UpdateAssignData(const std::unordered_map<std::string, std::string>& args, int32_t index)
{
    auto assignTaskId = base::StrToInt<uint32_t>(args.at("taskId "));
    auto executeId = base::StrToInt<int32_t>(args.at(" executeId "));
    auto priority = base::StrToInt<uint32_t>(args.at(" priority "));
    auto executeState = base::StrToInt<uint32_t>(args.at(" executeState "));

    int32_t returnValue = CheckTheSameTask(executeId.value());
    if (returnValue == INVALID_INT32) {
        int32_t taskIndex = traceDataCache_->GetTaskPoolData()->AppendAllocationTaskData(
            index, assignTaskId.value(), executeId.value(), priority.value(), executeState.value());
        executeMap_.emplace(executeId.value(), taskIndex);
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateAllocationTaskData(returnValue, index, assignTaskId.value(),
                                                                     priority.value(), executeState.value());
    }
}

void TaskPoolFilter::UpdateExecuteData(const std::unordered_map<std::string, std::string>& args, int32_t index)
{
    auto executeTaskId = base::StrToInt<uint32_t>(args.at("taskId "));
    auto executeId = base::StrToInt<int32_t>(args.at(" executeId "));

    int32_t returnValue = CheckTheSameTask(executeId.value());
    if (returnValue == INVALID_INT32) {
        int32_t taskIndex =
            traceDataCache_->GetTaskPoolData()->AppendExecuteTaskData(index, executeTaskId.value(), executeId.value());
        executeMap_.emplace(executeId.value(), taskIndex);
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateExecuteTaskData(returnValue, index, executeTaskId.value());
    }
}

void TaskPoolFilter::UpdateReturnData(const std::unordered_map<std::string, std::string>& args, int32_t index)
{
    int32_t returnState;
    auto returnTaskId = base::StrToInt<uint32_t>(args.at("taskId "));
    auto executeId = base::StrToInt<int32_t>(args.at(" executeId "));
    auto returnStr = std::string_view(args.at(" performResult "));
    if (!returnStr.compare(" Successful")) {
        returnState = 1;
    } else {
        returnState = 0;
    }

    int32_t returnValue = CheckTheSameTask(executeId.value());
    if (returnValue == INVALID_INT32) {
        int32_t taskIndex = traceDataCache_->GetTaskPoolData()->AppendReturnTaskData(index, returnTaskId.value(),
                                                                                     executeId.value(), returnState);
        executeMap_.emplace(executeId.value(), taskIndex);
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateReturnTaskData(returnValue, index, returnTaskId.value(), returnState);
    }
}

} // namespace TraceStreamer
} // namespace SysTuning
