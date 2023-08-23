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

#include "animation_filter.h"
#include <optional>
#include <regex>
#include "string_help.h"
#include "string_to_numerical.h"

namespace SysTuning {
namespace TraceStreamer {
constexpr uint8_t GENERATE_VSYNC_EVENT_MAX = 6;
constexpr uint8_t DYNAMIC_STACK_DEPTH_MIN = 4;
constexpr uint16_t FPS_60 = 60;
constexpr uint16_t FPS_70 = 70;
constexpr uint16_t FPS_90 = 90;
constexpr uint16_t FPS_100 = 100;
constexpr uint16_t FPS_120 = 120;

AnimationFilter::AnimationFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter)
{
}

AnimationFilter::~AnimationFilter() {}

bool AnimationFilter::UpdateDeviceInfoEvent(const TracePoint& point, const BytraceLine& line)
{
    if (traceDataCache_->GetConstDeviceInfo().PhysicalFrameRate() == INVALID_UINT32 &&
        StartWith(point.name_, generateVsyncCmd_)) {
        if (generateCurTimePoint_ == 0) {
            generateCurTimePoint_ = line.ts;
        }
        generateVsyncCnt_++;
        // calculate the average frame rate
        if (generateVsyncCnt_ == GENERATE_VSYNC_EVENT_MAX) {
            uint64_t generateTimePeriod = (line.ts - generateCurTimePoint_) / (GENERATE_VSYNC_EVENT_MAX - 1);
            uint32_t fps = BILLION_NANOSECONDS / generateTimePeriod;
            if (fps < FPS_70) {
                traceDataCache_->GetDeviceInfo()->UpdateFrameRate(FPS_60);
            } else if (fps < FPS_100) {
                traceDataCache_->GetDeviceInfo()->UpdateFrameRate(FPS_90);
            } else {
                traceDataCache_->GetDeviceInfo()->UpdateFrameRate(FPS_120);
            }
            TS_LOGI("physical frame rate is %u", fps);
        }
        return true;
    } else if (traceDataCache_->GetConstDeviceInfo().PhysicalWidth() == INVALID_UINT32 &&
               point.funcPrefixId_ == entryViewCmd_) {
        // get width and height, eg:funcArgs=(0, 0, 1344, 2772) Alpha: 1.00
        std::smatch matcheLine;
        std::regex entryViewArgsPattern(R"(\(\d+,\s*\d+,\s*(\d+),\s*(\d+)\))");
        if (std::regex_search(point.funcArgs_, matcheLine, entryViewArgsPattern)) {
            uint8_t index = 0;
            uint32_t width = base::StrToInt<uint32_t>(matcheLine[++index].str()).value();
            uint32_t height = base::StrToInt<uint32_t>(matcheLine[++index].str()).value();
            traceDataCache_->GetDeviceInfo()->UpdateWidthAndHeight(matcheLine);
            TS_LOGI("physical width is %u, height is %u", width, height);
        } else {
            TS_LOGE("Not support this event: %s\n", point.name_.data());
            return false;
        }
        return true;
    }
    return false;
}
bool AnimationFilter::BeginDynamicFrameEvent(const TracePoint& point, size_t callStackRow)
{
    // matches the 'H:RSUniRender::Process:[' event
    if (StartWith(point.funcPrefix_, rsUniProcessCmd_)) {
        // get the parent frame of data
        CallStack* callStackSlice = traceDataCache_->GetInternalSlicesData();
        const std::optional<uint64_t>& parentId = callStackSlice->ParentIdData()[callStackRow];
        uint8_t depth = callStackSlice->Depths()[callStackRow];
        if (depth >= DYNAMIC_STACK_DEPTH_MIN && parentId.has_value()) {
            const std::string& curStackName =
                traceDataCache_->GetDataFromDict(callStackSlice->NamesData()[callStackRow]);
            const std::string& parentStackName =
                traceDataCache_->GetDataFromDict(callStackSlice->NamesData()[parentId.value()]);
            // get name 'xxx' from [xxx], eg:H:RSUniRender::Process:[xxx]
            auto nameSize = point.funcPrefix_.size() - rsUniProcessCmd_.size() - 1;
            if (nameSize <= 0) {
                return false;
            }
            auto nameIndex = traceDataCache_->GetDataIndex(point.funcPrefix_.substr(rsUniProcessCmd_.size(), nameSize));
            if (StartWith(curStackName, leashWindowCmd_)) {
                auto dynamicFramRow = traceDataCache_->GetDynamicFrame()->AppendDynamicFrame(nameIndex);
                callStackRowMap_.emplace(callStackRow, dynamicFramRow);
                return true;
            } else if (StartWith(parentStackName, leashWindowCmd_)) {
                auto iter = callStackRowMap_.find(parentId.value());
                if (iter != callStackRowMap_.end()) {
                    auto dynamicFramRow = iter->second;
                    traceDataCache_->GetDynamicFrame()->UpdateNameIndex(dynamicFramRow, nameIndex);
                    return true;
                } else {
                    TS_LOGE("Can't find the dynamicFramRow from callStackRowMap_");
                }
            }
        }
    }
    return false;
}
void AnimationFilter::StartAnimationEvent(const BytraceLine& line, size_t callStackRow)
{
    auto animationRow = traceDataCache_->GetAnimation()->AppendAnimation(line.ts);
    animationCallIds_.emplace(callStackRow, animationRow);
}
bool AnimationFilter::FinishAnimationEvent(const BytraceLine& line, size_t callStackRow)
{
    auto iter = animationCallIds_.find(callStackRow);
    if (iter != animationCallIds_.end()) {
        auto animationRow = iter->second;
        traceDataCache_->GetAnimation()->UpdateEndPoint(animationRow, line.ts);
        animationCallIds_.erase(iter);
        return true;
    }
    return false;
}
void AnimationFilter::UpdateDynamicFrameInfo()
{
    std::smatch matcheLine;
    std::regex leashWindowPattern(R"((\d+),\s*(\d+),\s*(\d+),\s*(\d+)\)\s+Alpha:\s+-*(\d+\.\d+))");
    DynamicFrame* dynamicFrame = traceDataCache_->GetDynamicFrame();
    CallStack* callStackSlice = traceDataCache_->GetInternalSlicesData();
    uint64_t curStackRow = INVALID_UINT64;
    uint64_t curFrameRow = INVALID_UINT64;
    for (const auto& it : callStackRowMap_) {
        curStackRow = it.first;
        curFrameRow = it.second;
        uint8_t stackDepth = callStackSlice->Depths()[curStackRow];
        // update dynamicFrame pos, eg:H:RSUniRender::Process:[leashWindow25] (0, 0, 1344, 2772) Alpha: 1.00
        auto nameDataIndex = callStackSlice->NamesData()[curStackRow];
        const std::string& curStackName = traceDataCache_->GetDataFromDict(nameDataIndex);
        const std::string& funcArgs = curStackName.substr(leashWindowCmd_.size());
        if (std::regex_search(funcArgs, matcheLine, leashWindowPattern)) {
            dynamicFrame->UpdatePosition(
                curFrameRow, matcheLine,
                traceDataCache_->GetDataIndex((matcheLine[DYNAMICFRAME_MATCH_LAST].str()))); // alpha
        } else {
            TS_LOGE("Not support this event: %s\n", funcArgs.data());
            break;
        }
        // update dynamicFrame endTime, filter up from the curStackRow, until reach the top
        for (uint8_t stackCurDepth = stackDepth; stackCurDepth > 0; stackCurDepth--) {
            if (callStackSlice->ParentIdData()[curStackRow].has_value()) {
                curStackRow = callStackSlice->ParentIdData()[curStackRow].value();
            } else {
                break;
            }
            // use 'H:RSMainThread::DoComposition' endTime as dynamicFrame endTime
            if (rsDoCompCmd_ == callStackSlice->NamesData()[curStackRow]) {
                auto endTime = callStackSlice->TimeStampData()[curStackRow] + callStackSlice->DursData()[curStackRow];
                dynamicFrame->UpdateEndTime(curFrameRow, endTime);
                break;
            }
        }
    }
    TS_LOGI("UpdateDynamicFrame (%zu) endTime and pos finish", callStackRowMap_.size());
    // this can only be cleared by the UpdateDynamicFrameInfo function
    callStackRowMap_.clear();
}
void AnimationFilter::Clear()
{
    generateCurTimePoint_ = 0;
    generateVsyncCnt_ = 0;
    animationCallIds_.clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
