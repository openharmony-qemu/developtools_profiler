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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "animation_filter.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
constexpr uint8_t GENERATE_VSYNC_EVENT_MAX = 6;
constexpr uint16_t FPS_60 = 60;
constexpr uint16_t FPS_90 = 90;
constexpr uint16_t FPS_120 = 120;
class AnimationFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    TraceStreamerSelector stream_;
};

/**
 * @tc.name: NonRsUniProcessEvent
 * @tc.desc: the current event isn't RsUniProcessEvent
 * @tc.type: FUNC
 */
HWTEST_F(AnimationFilterTest, NonRsUniProcessEvent, TestSize.Level1)
{
    TS_LOGI("test36-1");
    TracePoint point;
    const size_t CALLSTACK_SLICE_ID = 1;
    std::vector<std::string> nonRsUniProcessEvents{
        "H:RSUniRender::Prepare:[xxx]",
        "H:RSUniRender:DrivenRenderPrepare",
        "H:RSDrivenRender:DoPrepareRenderTask",
        "H:ProcessDisplayRenderNode[0](0,",
        "H:RSBaseRenderEngine::RequestFrame(RSSurface)",
        "H:DisplayNode:4",
        "H:AddContainerDirtyToGlobalDirty",
    };
    for (size_t i = 0; i < nonRsUniProcessEvents.size(); i++) {
        point.funcPrefix_ = nonRsUniProcessEvents[i];
        auto res = stream_.streamFilters_->animationFilter_->BeginDynamicFrameEvent(point, CALLSTACK_SLICE_ID);
        EXPECT_FALSE(res);
    }
}

/**
 * @tc.name: InvalidCallStack
 * @tc.desc: the current event isn't RsUniProcessEvent
 * @tc.type: FUNC
 */
HWTEST_F(AnimationFilterTest, InvalidCallStack, TestSize.Level1)
{
    TS_LOGI("test36-2");
    TracePoint point;
    std::string validName("H:RSUniRender::Process:[WindowScene_xxx] (0, 0, 1344, 2772) Alpha: 1.00");
    std::string invalidName("H:RSUniRender::Process:[xxx] (0, 0, 1344, 2772) Alpha: 1.00");
    const size_t CALLSTACK_SLICE_ID = 1;
    CallStack* callStackSlice = stream_.traceDataCache_->GetInternalSlicesData();
    std::vector<DataIndex> callStackNames{
        stream_.traceDataCache_->GetDataIndex("H:RSMainThread::DoComposition"),
        stream_.traceDataCache_->GetDataIndex("H:ProcessDisplayRenderNode[0](0,0,0,0)"),
        stream_.traceDataCache_->GetDataIndex(validName),
        stream_.traceDataCache_->GetDataIndex(invalidName),
    };
    std::vector<std::string> funcPrefixs{
        "H:RSUniRender::Process:[WindowScene_xxx]",
        "H:RSUniRender::Process:[xxx]",
    };
    // invalid parentId
    for (size_t i = 0, depth = 0; i < callStackNames.size(); i++) {
        std::optional<uint64_t> parentId = 0;
        callStackSlice->AppendInternalSlice(INVALID_TIME, INVALID_TIME, INVALID_UINT32, INVALID_UINT64, INVALID_UINT16,
                                            callStackNames[i], ++depth, parentId);
        point.funcPrefix_ = funcPrefixs[1];
        auto res = stream_.streamFilters_->animationFilter_->BeginDynamicFrameEvent(point, CALLSTACK_SLICE_ID);
        EXPECT_FALSE(res);
    }
    // the current or the parent callStackNames haven't WindowScene_
    uint64_t index = INVALID_UINT64;
    for (size_t i = 0, depth = 0; i < callStackNames.size(); i++) {
        std::optional<uint64_t> parentId;
        if (index != INVALID_UINT64) {
            parentId = index;
        }
        depth = i + 1;
        index = callStackSlice->AppendInternalSlice(INVALID_TIME, INVALID_TIME, INVALID_UINT32, INVALID_UINT64,
                                                    INVALID_UINT16, callStackNames[i], depth, parentId);
    }
    point.funcPrefix_ = funcPrefixs[1];
    point.name_ = invalidName;
    auto curStackRow = 1;
    auto res = stream_.streamFilters_->animationFilter_->BeginDynamicFrameEvent(point, curStackRow);
    EXPECT_FALSE(res);
    // valid callStack
    point.funcPrefix_ = funcPrefixs[0];
    point.name_ = validName;
    curStackRow = 2;
    res = stream_.streamFilters_->animationFilter_->BeginDynamicFrameEvent(point, curStackRow);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: UpdateDevicePos
 * @tc.desc: update device pos
 * @tc.type: FUNC
 */
HWTEST_F(AnimationFilterTest, UpdateDevicePos, TestSize.Level1)
{
    TS_LOGI("test36-3");
    TracePoint point;
    BytraceLine line;
    std::string validFuncPrefix{"H:RSUniRender::Process:[SCBDesktop2]"};
    std::vector<std::string> invalidFuncArgs{
        "()",
        "(1,)",
        "[1,2]",
    };
    stream_.traceDataCache_->GetDeviceInfo()->Clear();
    for (size_t i = 0; i < invalidFuncArgs.size(); i++) {
        point.funcPrefix_ = validFuncPrefix;
        point.funcArgs_ = invalidFuncArgs[i];
        point.name_ = point.funcPrefix_ + " " + point.funcArgs_;
        point.funcPrefixId_ = stream_.traceDataCache_->GetDataIndex(point.funcPrefix_);
        auto res = stream_.streamFilters_->animationFilter_->UpdateDeviceInfoEvent(point, line);
        EXPECT_FALSE(res);
    }
    std::vector<std::string> validFuncArgs{
        "(0, 0, 1024, 1920)",
        "(0,0, 628, 720)",
        "(0,0, 628, 720)",
    };
    for (size_t i = 0; i < validFuncArgs.size(); i++) {
        stream_.traceDataCache_->GetDeviceInfo()->Clear();
        point.funcPrefix_ = validFuncPrefix;
        point.funcArgs_ = validFuncArgs[i];
        point.name_ = point.funcPrefix_ + " " + point.funcArgs_;
        point.funcPrefixId_ = stream_.traceDataCache_->GetDataIndex(point.funcPrefix_);
        auto res = stream_.streamFilters_->animationFilter_->UpdateDeviceInfoEvent(point, line);
        EXPECT_TRUE(res);
    }
}
/**
 * @tc.name: UpdateDeviceFps
 * @tc.desc: update device fps
 * @tc.type: FUNC
 */
HWTEST_F(AnimationFilterTest, UpdateDeviceFps, TestSize.Level1)
{
    TS_LOGI("test36-4");
    TracePoint point;
    BytraceLine line;
    std::string validName{"H:GenerateVsyncCount:1"};
    auto timeDiffFps60 = BILLION_NANOSECONDS / FPS_60;
    line.ts = 59557002299000;
    for (uint8_t i = 0; i < GENERATE_VSYNC_EVENT_MAX; i++) {
        point.name_ = validName;
        point.funcPrefixId_ = stream_.traceDataCache_->GetDataIndex(point.name_);
        line.ts += timeDiffFps60;
        auto res = stream_.streamFilters_->animationFilter_->UpdateDeviceInfoEvent(point, line);
        EXPECT_TRUE(res);
    }
    auto fps = stream_.traceDataCache_->GetDeviceInfo()->PhysicalFrameRate();
    EXPECT_EQ(fps, FPS_60);

    stream_.traceDataCache_->GetDeviceInfo()->Clear();
    stream_.streamFilters_->animationFilter_->Clear();
    auto timeDiffFps90 = BILLION_NANOSECONDS / FPS_90;
    for (uint8_t i = 0; i < GENERATE_VSYNC_EVENT_MAX; i++) {
        point.name_ = validName;
        point.funcPrefixId_ = stream_.traceDataCache_->GetDataIndex(point.name_);
        line.ts += timeDiffFps90;
        auto res = stream_.streamFilters_->animationFilter_->UpdateDeviceInfoEvent(point, line);
        EXPECT_TRUE(res);
    }
    fps = stream_.traceDataCache_->GetDeviceInfo()->PhysicalFrameRate();
    EXPECT_EQ(fps, FPS_90);

    stream_.traceDataCache_->GetDeviceInfo()->Clear();
    stream_.streamFilters_->animationFilter_->Clear();
    auto timeDiffFps120 = BILLION_NANOSECONDS / FPS_120;
    for (uint8_t i = 0; i < GENERATE_VSYNC_EVENT_MAX; i++) {
        point.name_ = validName;
        point.funcPrefixId_ = stream_.traceDataCache_->GetDataIndex(point.name_);
        line.ts += timeDiffFps120;
        auto res = stream_.streamFilters_->animationFilter_->UpdateDeviceInfoEvent(point, line);
        EXPECT_TRUE(res);
    }
    fps = stream_.traceDataCache_->GetDeviceInfo()->PhysicalFrameRate();
    EXPECT_EQ(fps, FPS_120);
}
/**
 * @tc.name: UpdateDynamicFrameInfo
 * @tc.desc: update Dynamic Frame pos and endTime
 * @tc.type: FUNC
 */
HWTEST_F(AnimationFilterTest, UpdateDynamicFrameInfo, TestSize.Level1)
{
    TS_LOGI("test36-5");
    TracePoint point;
    CallStack* callStackSlice = stream_.traceDataCache_->GetInternalSlicesData();
    std::vector<DataIndex> callStackNames{
        stream_.traceDataCache_->GetDataIndex("H:RSMainThread::DoComposition"),
        stream_.traceDataCache_->GetDataIndex("H:ProcessDisplayRenderNode[0](0,0,0,0)"),
        stream_.traceDataCache_->GetDataIndex(
            "H:RSUniRender::Process:[WindowScene_xxx] (0, 0, 1344, 2772) Alpha: 1.00"),
    };
    std::string funcPrefix("H:RSUniRender::Process:[WindowScene_xxx]");
    uint64_t index = INVALID_UINT64;
    uint64_t startTime = 59557002299000;
    uint64_t dur = ONE_MILLION_NANOSECONDS;
    for (size_t i = 0, depth = 0; i < callStackNames.size(); i++) {
        std::optional<uint64_t> parentId;
        if (index != INVALID_UINT64) {
            parentId = index;
        }
        depth = i + 1;
        index = callStackSlice->AppendInternalSlice(startTime, dur, INVALID_UINT32, INVALID_UINT64, INVALID_UINT16,
                                                    callStackNames[i], depth, parentId);
    }
    point.funcPrefix_ = funcPrefix;
    point.name_ = stream_.traceDataCache_->GetDataFromDict(callStackNames.back());
    auto res = stream_.streamFilters_->animationFilter_->BeginDynamicFrameEvent(point, index); // for WindowScene_xxx
    EXPECT_TRUE(res);
    stream_.streamFilters_->animationFilter_->UpdateDynamicFrameInfo();
    for (size_t i = 0; i < stream_.traceDataCache_->GetDynamicFrame()->Size(); i++) {
        EXPECT_TRUE(stream_.traceDataCache_->GetDynamicFrame()->EndTimes()[i] != INVALID_TIME);
        EXPECT_TRUE(stream_.traceDataCache_->GetDynamicFrame()->Xs()[i] != INVALID_UINT32);
    }
}
/**
 * @tc.name: AnimationStartAndEnd
 * @tc.desc: update Animation startPoint and endPoint
 * @tc.type: FUNC
 */
HWTEST_F(AnimationFilterTest, AnimationStartAndEnd, TestSize.Level1)
{
    TS_LOGI("test36-6");
    BytraceLine line;
    line.ts = 59557002299000;

    CallStack* callStackSlice = stream_.traceDataCache_->GetInternalSlicesData();
    uint8_t depth = 1;
    uint64_t dur = ONE_MILLION_NANOSECONDS;
    std::optional<uint64_t> parentId;
    DataIndex callStackName = stream_.traceDataCache_->GetDataIndex(
        "H:RSUniRender::Process:[WindowScene_xxx] (0, 0, 1344, 2772) Alpha: 1.00");

    auto callStackRow = callStackSlice->AppendInternalSlice(line.ts, dur, INVALID_UINT32, INVALID_UINT64,
                                                            INVALID_UINT16, callStackName, depth, parentId);

    TracePoint point;
    point.name_ = "H:APP_LIST_FLING, com.taobao.taobao, pages/Index, 1693876205590.";
    stream_.streamFilters_->animationFilter_->StartAnimationEvent(line, point, callStackRow);
    EXPECT_TRUE(!stream_.streamFilters_->animationFilter_->animationCallIds_.empty());
    stream_.streamFilters_->animationFilter_->FinishAnimationEvent(line, callStackRow);
    EXPECT_TRUE(stream_.streamFilters_->animationFilter_->animationCallIds_.empty());
}
} // namespace TraceStreamer
} // namespace SysTuning
