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
#include "string_help.h"
#include "string_to_numerical.h"

namespace SysTuning {
namespace TraceStreamer {
constexpr uint8_t GENERATE_VSYNC_EVENT_MAX = 5;
constexpr uint8_t DYNAMIC_STACK_DEPTH_MIN = 4;
constexpr uint16_t FPS_60 = 60;
constexpr uint16_t FPS_70 = 70;
constexpr uint16_t FPS_90 = 90;
constexpr uint16_t FPS_100 = 100;
constexpr uint16_t FPS_120 = 120;

AnimationFilter::AnimationFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter)
{
    dynamicFrame_ = traceDataCache_->GetDynamicFrame();
    callStackSlice_ = traceDataCache_->GetInternalSlicesData();
    if (dynamicFrame_ == nullptr || callStackSlice_ == nullptr) {
        TS_LOGE("dynamicFrame_ or callStackSlice_ is nullptr.");
    }
}
AnimationFilter::~AnimationFilter() {}
bool AnimationFilter::UpdateDeviceFps(const BytraceLine& line)
{
    generateVsyncCnt_++;
    if (generateFirstTime_ == INVALID_UINT64) {
        generateFirstTime_ = line.ts;
    }
    if (generateVsyncCnt_ <= GENERATE_VSYNC_EVENT_MAX) {
        return true;
    }
    // calculate the average frame rate
    uint64_t generateTimePeriod = (line.ts - generateFirstTime_) / GENERATE_VSYNC_EVENT_MAX;
    uint32_t fps = BILLION_NANOSECONDS / generateTimePeriod;
    if (fps < FPS_70) {
        traceDataCache_->GetDeviceInfo()->UpdateFrameRate(FPS_60);
    } else if (fps < FPS_100) {
        traceDataCache_->GetDeviceInfo()->UpdateFrameRate(FPS_90);
    } else {
        traceDataCache_->GetDeviceInfo()->UpdateFrameRate(FPS_120);
    }
    TS_LOGI("physical frame rate is %u", fps);
    return true;
}
bool AnimationFilter::UpdateDeviceScreenSize(const TracePoint& point)
{
    // get width and height, eg:funcArgs=(0, 0, 1344, 2772) Alpha: 1.00
    std::smatch matcheLine;
    std::regex entryViewArgsPattern(R"(\(\d+,\s*\d+,\s*(\d+),\s*(\d+)\))");
    if (!std::regex_search(point.funcArgs_, matcheLine, entryViewArgsPattern)) {
        TS_LOGE("Not support this event: %s\n", point.name_.data());
        return false;
    }
    uint8_t index = 0;
    uint32_t width = base::StrToInt<uint32_t>(matcheLine[++index].str()).value();
    uint32_t height = base::StrToInt<uint32_t>(matcheLine[++index].str()).value();
    traceDataCache_->GetDeviceInfo()->UpdateWidthAndHeight(matcheLine);
    TS_LOGI("physical width is %u, height is %u", width, height);
    return true;
}
bool AnimationFilter::UpdateDeviceInfoEvent(const TracePoint& point, const BytraceLine& line)
{
    if (traceDataCache_->GetConstDeviceInfo().PhysicalFrameRate() == INVALID_UINT32 &&
        StartWith(point.name_, generateVsyncCmd_)) {
        return UpdateDeviceFps(line);
    } else if (traceDataCache_->GetConstDeviceInfo().PhysicalWidth() == INVALID_UINT32 &&
               point.funcPrefixId_ == entryViewCmd_) {
        return UpdateDeviceScreenSize(point);
    }
    return false;
}
bool AnimationFilter::BeginDynamicFrameEvent(const TracePoint& point, size_t callStackRow)
{
    // get the parent frame of data
    const std::optional<uint64_t>& parentId = callStackSlice_->ParentIdData()[callStackRow];
    uint8_t depth = callStackSlice_->Depths()[callStackRow];
    if (depth < DYNAMIC_STACK_DEPTH_MIN || !parentId.has_value()) {
        return false;
    }
    const std::string& curStackName = traceDataCache_->GetDataFromDict(callStackSlice_->NamesData()[callStackRow]);
    if (!StartWith(curStackName, leashWindowCmd_)) {
        return false;
    }
    // get name 'xxx' from [xxx], eg:H:RSUniRender::Process:[xxx]
    auto nameSize = point.funcPrefix_.size() - rsUniProcessCmd_.size() - 1;
    if (nameSize <= 0) {
        return false;
    }
    auto nameIndex = traceDataCache_->GetDataIndex(point.funcPrefix_.substr(rsUniProcessCmd_.size(), nameSize));
    auto dynamicFramRow = dynamicFrame_->AppendDynamicFrame(nameIndex);
    callStackRowMap_.emplace(callStackRow, dynamicFramRow);
    return true;
}
void AnimationFilter::StartAnimationEvent(const BytraceLine& line, size_t callStackRow)
{
    auto animationRow = traceDataCache_->GetAnimation()->AppendAnimation(line.ts);
    animationCallIds_.emplace(callStackRow, animationRow);
}
bool AnimationFilter::FinishAnimationEvent(const BytraceLine& line, size_t callStackRow)
{
    auto iter = animationCallIds_.find(callStackRow);
    if (iter == animationCallIds_.end()) {
        return false;
    }
    auto animationRow = iter->second;
    traceDataCache_->GetAnimation()->UpdateEndPoint(animationRow, line.ts);
    animationCallIds_.erase(iter);
    return true;
}
bool AnimationFilter::UpdateDynamicEndTime(const uint64_t curFrameRow, uint64_t curStackRow)
{
    // update dynamicFrame endTime, filter up from the curStackRow, until reach the top
    for (uint8_t stackCurDepth = callStackSlice_->Depths()[curStackRow]; stackCurDepth > 0; stackCurDepth--) {
        if (!callStackSlice_->ParentIdData()[curStackRow].has_value()) {
            return false;
        }
        curStackRow = callStackSlice_->ParentIdData()[curStackRow].value();
        // use 'H:RSMainThread::DoComposition' endTime as dynamicFrame endTime
        if (rsDoCompCmd_ == callStackSlice_->NamesData()[curStackRow]) {
            auto endTime = callStackSlice_->TimeStampData()[curStackRow] + callStackSlice_->DursData()[curStackRow];
            dynamicFrame_->UpdateEndTime(curFrameRow, endTime);
            return true;
        }
    }
    return false;
}
void AnimationFilter::UpdateDynamicFrameInfo()
{
    std::smatch matcheLine;
    std::regex leashWindowPattern(R"((\d+),\s*(\d+),\s*(\d+),\s*(\d+)\)\s+Alpha:\s+-*(\d+\.\d+))");
    uint64_t curStackRow = INVALID_UINT64;
    uint64_t curFrameRow = INVALID_UINT64;
    for (const auto& it : callStackRowMap_) {
        curStackRow = it.first;
        curFrameRow = it.second;
        // update dynamicFrame pos, eg:H:RSUniRender::Process:[leashWindow25] (0, 0, 1344, 2772) Alpha: 1.00
        auto nameDataIndex = callStackSlice_->NamesData()[curStackRow];
        const std::string& curStackName = traceDataCache_->GetDataFromDict(nameDataIndex);
        const std::string& funcArgs = curStackName.substr(leashWindowCmd_.size());
        if (!std::regex_search(funcArgs, matcheLine, leashWindowPattern)) {
            TS_LOGE("Not support this event: %s\n", funcArgs.data());
            continue;
        }
        dynamicFrame_->UpdatePosition(
            curFrameRow, matcheLine,
            traceDataCache_->GetDataIndex((matcheLine[DYNAMICFRAME_MATCH_LAST].str()))); // alpha
        UpdateDynamicEndTime(curFrameRow, curStackRow);
    }
    TS_LOGI("UpdateDynamicFrame (%zu) endTime and pos finish", callStackRowMap_.size());
    // this can only be cleared by the UpdateDynamicFrameInfo function
    callStackRowMap_.clear();
}
void AnimationFilter::Clear()
{
    generateFirstTime_ = INVALID_UINT64;
    generateVsyncCnt_ = 0;
    animationCallIds_.clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
