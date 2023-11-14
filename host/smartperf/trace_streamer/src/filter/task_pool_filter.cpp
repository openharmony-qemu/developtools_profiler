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

uint32_t TaskPoolFilter::CheckTheSameTask(uint32_t executeId, uint32_t index)
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

bool TaskPoolFilter::TaskPoolEvent(const std::string& taskPoolStr, uint32_t index)
{
    if (StartWith(taskPoolStr, targetStr_)) {
        std::unordered_map<std::string, std::string> args;
        if (StartWith(taskPoolStr, allocationStr_)) {
            const auto& infoStr = taskPoolStr.substr(allocationStr_.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(infoStr, args);
            return UpdateAssignData(args, index);
        }
        if (StartWith(taskPoolStr, executeStr_)) {
            const auto& infoStr = taskPoolStr.substr(executeStr_.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(infoStr, args);
            return UpdateExecuteData(args, index);
        }
        if (StartWith(taskPoolStr, returnStr_)) {
            const auto& infoStr = taskPoolStr.substr(returnStr_.length(), taskPoolStr.length());
            TaskPoolFieldSegmentation(infoStr, args);
            return UpdateReturnData(args, index);
        }
    }
    if (StartWith(taskPoolStr, timeoutStr_)) {
        return AppendTimeoutRow(index);
    }
    return false;
}

bool TaskPoolFilter::UpdateAssignData(const std::unordered_map<std::string, std::string>& args, uint32_t index)
{
    if (index >= traceDataCache_->GetConstInternalSlicesData().CallIds().size()) {
        return false;
    }
    auto allocItid = traceDataCache_->GetConstInternalSlicesData().CallIds()[index];
    auto executeId = base::StrToInt<uint32_t>(args.at(" executeId "));
    auto priority = base::StrToInt<uint32_t>(args.at(" priority "));
    auto executeState = base::StrToInt<uint32_t>(args.at(" executeState "));

    uint32_t returnValue = CheckTheSameTask(executeId.value(), index);
    if (returnValue == INVALID_INT32) {
        uint32_t taskIndex = traceDataCache_->GetTaskPoolData()->AppendAllocationTaskData(
            index, allocItid, executeId.value(), priority.value(), executeState.value());
        IpidExecuteMap_.Insert(GetIpId(index), executeId.value(), taskIndex);
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateAllocationTaskData(returnValue, index, allocItid, priority.value(),
                                                                     executeState.value());
    }
    return true;
}

bool TaskPoolFilter::UpdateExecuteData(const std::unordered_map<std::string, std::string>& args, uint32_t index)
{
    if (index >= traceDataCache_->GetConstInternalSlicesData().CallIds().size()) {
        return false;
    }
    auto executeItid = traceDataCache_->GetConstInternalSlicesData().CallIds()[index];
    auto executeId = base::StrToInt<uint32_t>(args.at(" executeId "));

    uint32_t returnValue = CheckTheSameTask(executeId.value(), index);
    if (returnValue == INVALID_INT32) {
        uint32_t taskIndex =
            traceDataCache_->GetTaskPoolData()->AppendExecuteTaskData(index, executeItid, executeId.value());
        IpidExecuteMap_.Insert(GetIpId(index), executeId.value(), taskIndex);
        timeoutMap_.emplace(executeItid, taskIndex);
        if (timeoutMap_.at(executeItid) < taskIndex) {
            timeoutMap_.at(executeItid) = taskIndex;
        }
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateExecuteTaskData(returnValue, index, executeItid);
        timeoutMap_.emplace(executeItid, returnValue);
        if (timeoutMap_.at(executeItid) < returnValue) {
            timeoutMap_.at(executeItid) = returnValue;
        }
    }
    return true;
}

bool TaskPoolFilter::UpdateReturnData(const std::unordered_map<std::string, std::string>& args, uint32_t index)
{
    if (index >= traceDataCache_->GetConstInternalSlicesData().CallIds().size()) {
        return false;
    }
    auto returnItid = traceDataCache_->GetConstInternalSlicesData().CallIds()[index];
    auto executeId = base::StrToInt<uint32_t>(args.at(" executeId "));
    auto returnStr_ = std::string_view(args.at(" performResult "));
    uint32_t returnState = returnStr_.compare(" Successful") ? 0 : 1;

    uint32_t returnValue = CheckTheSameTask(executeId.value(), index);
    if (returnValue == INVALID_INT32) {
        uint32_t taskIndex =
            traceDataCache_->GetTaskPoolData()->AppendReturnTaskData(index, returnItid, executeId.value(), returnState);
        IpidExecuteMap_.Insert(GetIpId(index), executeId.value(), taskIndex);
    } else {
        traceDataCache_->GetTaskPoolData()->UpdateReturnTaskData(returnValue, index, returnItid, returnState);
    }
    return true;
}

bool TaskPoolFilter::AppendTimeoutRow(uint32_t index)
{
    if (index >= traceDataCache_->GetConstInternalSlicesData().CallIds().size()) {
        return false;
    }
    auto timeoutItid = traceDataCache_->GetConstInternalSlicesData().CallIds()[index];
    traceDataCache_->GetTaskPoolData()->AppendTimeoutRow(timeoutMap_.at(timeoutItid), index);
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning
