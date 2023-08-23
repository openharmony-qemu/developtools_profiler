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
#include "frame_filter.h"
#include <memory>
#include <cinttypes>
#include "log.h"
#define ISINVALIDU32(value) (value == INVALID_UINT32)
namespace SysTuning {
namespace TraceStreamer {
FrameFilter::FrameFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter) : FilterBase(dataCache, filter)
{
}
FrameFilter::~FrameFilter() = default;

void FrameFilter::BeginVsyncEvent(uint64_t ts,
                                  uint32_t ipid,
                                  uint32_t itid,
                                  uint64_t expectStart,
                                  uint64_t expectEnd,
                                  uint32_t vsyncId,
                                  uint32_t callStackSliceId)
{
    auto frame = std::make_shared<FrameSlice>();
    frame->startTs_ = ts;
    frame->callStackSliceId_ = callStackSliceId;
    frame->expectedStartTs_ = expectStart;
    frame->expectedEndTs_ = expectEnd;
    frame->expectedDur_ = expectEnd - expectStart;
    frame->vsyncId_ = vsyncId;
    frame->frameSliceRow_ =
        traceDataCache_->GetFrameSliceData()->AppendFrame(ts, ipid, itid, vsyncId, callStackSliceId);
    frame->frameExpectedSliceRow_ = traceDataCache_->GetFrameSliceData()->AppendFrame(
        expectStart, ipid, itid, vsyncId, callStackSliceId, expectEnd, (uint8_t)TraceStdtype::FrameSlice::EXPECT_SLICE);
    if (vsyncRenderSlice_.count(itid)) {
        vsyncRenderSlice_[itid].push_back(frame);
    } else {
        std::vector<std::shared_ptr<FrameSlice>> frameVec;
        frameVec.push_back(frame);
        vsyncRenderSlice_[itid] = frameVec;
    }
}

bool FrameFilter::MarkRSOnvsyncEvent(uint64_t ts, uint32_t itid)
{
    auto frame = vsyncRenderSlice_.find(itid);
    if (frame == vsyncRenderSlice_.end()) {
        TS_LOGW("BeginOnvsyncEvent find for itid:%u failed, ts:%" PRIu64 "", itid, ts);
        return false;
    }
    if (!frame->second.size()) {
        TS_LOGW("BeginOnvsyncEvent find for itid:%u failed", itid);
        return false;
    }
    auto lastFrameSlice = frame->second.back();
    lastFrameSlice->isRsMainThread_ = true;
    return true;
}
bool FrameFilter::EndOnVsyncEvent(uint64_t ts, uint32_t itid)
{
    auto frame = vsyncRenderSlice_.find(itid);
    if (frame == vsyncRenderSlice_.end()) {
        TS_LOGW("BeginOnvsyncEvent find for itid:%u failed", itid);
        return false;
    }
    if (!frame->second.size()) {
        TS_LOGW("BeginOnvsyncEvent find for itid:%u failed", itid);
        return false;
    }
    auto lastFrameSlice = frame->second.back();
    if (!lastFrameSlice->isRsMainThread_ && ISINVALIDU32(lastFrameSlice->frameNum_)) {
        // if app's frame num not received
        traceDataCache_->GetFrameSliceData()->Erase(lastFrameSlice->frameSliceRow_);
        traceDataCache_->GetFrameSliceData()->Erase(lastFrameSlice->frameExpectedSliceRow_);
        frame->second.pop_back();
        return false;
    }
    return true;
}
// for app
bool FrameFilter::BeginRSTransactionData(uint64_t ts, uint32_t itid, uint32_t franeNum)
{
    auto frame = vsyncRenderSlice_.find(itid);
    if (frame == vsyncRenderSlice_.end()) {
        TS_LOGW("BeginRSTransactionData find for itid:%u failed", itid);
        return false;
    }
    if (!frame->second.size()) {
        TS_LOGW("BeginRSTransactionData find for itid:%u failed", itid);
        return false;
    }
    frame->second.begin()->get()->frameNum_ = franeNum;
    if (!dstRenderSlice_.count(itid)) {
        std::unordered_map<uint32_t /* frameNum */, std::shared_ptr<FrameSlice>> frameMap;
        dstRenderSlice_.emplace(std::make_pair(itid, std::move(frameMap)));
    }
    // dstRenderSlice_.at(itid).insert(std::make_pair(franeNum, frame->second.begin()));
    dstRenderSlice_[itid][franeNum] = frame->second[0];
    return true;
}
// for RS
bool FrameFilter::BeginProcessCommandUni(uint64_t ts,
                                         uint32_t itid,
                                         const std::vector<FrameMap>& frames,
                                         uint32_t sliceIndex)
{
    auto frame = vsyncRenderSlice_.find(itid);
    if (frame == vsyncRenderSlice_.end()) {
        TS_LOGW("BeginProcessCommandUni find for itid:%u failed", itid);
        return false;
    }
    if (!frame->second.size()) {
        TS_LOGW("BeginProcessCommandUni find for itid:%u failed", itid);
        return false;
    }
    auto lastFrameSlice = frame->second.back();
    if (lastFrameSlice->vsyncEnd_) {
        TS_LOGW("BeginProcessCommandUni finished for vsyncId:%u", lastFrameSlice->vsyncId_);
        return false;
    }
    std::vector<uint64_t> fromSlices = {};
    std::vector<uint64_t> fromExpectedSlices = {};
    for (auto& it : frames) {
        auto sourceFrameMap = dstRenderSlice_.find(it.sourceItid);
        if (sourceFrameMap == dstRenderSlice_.end()) {
            // error
            TS_LOGE("BeginProcessCommandUni find for itid:%u framenum:%u failed", it.sourceItid, it.frameNum);
            continue;
        }
        auto srcFrame = sourceFrameMap->second.find(it.frameNum);
        if (srcFrame == sourceFrameMap->second.end()) {
            // error
            TS_LOGE("BeginProcessCommandUni find for itid:%u framenum:%u failed", it.sourceItid, it.frameNum);
            continue;
        }
        fromSlices.push_back(srcFrame->second.get()->frameSliceRow_);
        fromExpectedSlices.push_back(srcFrame->second.get()->frameExpectedSliceRow_);
        srcFrame->second.get()->dstFrameSliceId_ = lastFrameSlice->frameSliceRow_;
        srcFrame->second.get()->dstExpectedFrameSliceId_ = lastFrameSlice->frameExpectedSliceRow_;
        TraceStdtype::FrameSlice* frameSlice = traceDataCache_->GetFrameSliceData();
        (void)traceDataCache_->GetFrameMapsData()->AppendNew(frameSlice, srcFrame->second.get()->frameSliceRow_,
                                                             srcFrame->second.get()->dstFrameSliceId_);
        (void)traceDataCache_->GetFrameMapsData()->AppendNew(frameSlice, srcFrame->second.get()->frameExpectedSliceRow_,
                                                             srcFrame->second.get()->dstExpectedFrameSliceId_);
        frameSlice->SetDst(srcFrame->second.get()->frameSliceRow_, srcFrame->second.get()->dstFrameSliceId_);
        frameSlice->SetDst(srcFrame->second.get()->frameExpectedSliceRow_,
                           srcFrame->second.get()->dstExpectedFrameSliceId_);
        if (srcFrame->second.get()->endTs_ != INVALID_UINT64) {
            // erase Source
            sourceFrameMap->second.erase(it.frameNum);
        }
    }
    if (!fromSlices.size()) {
        return false;
    }
    lastFrameSlice->sourceSlice_ = fromSlices;
    lastFrameSlice->sourceExpectedSlice_ = fromExpectedSlices;
    traceDataCache_->GetFrameSliceData()->SetSrcs(lastFrameSlice->frameSliceRow_, fromSlices);
    traceDataCache_->GetFrameSliceData()->SetSrcs(lastFrameSlice->frameExpectedSliceRow_, fromExpectedSlices);
    return true;
}
bool FrameFilter::EndVsyncEvent(uint64_t ts, uint32_t itid)
{
    auto frame = vsyncRenderSlice_.find(itid);
    if (frame == vsyncRenderSlice_.end()) {
        TS_LOGW("EndVsyncEvent find for itid:%u ts:%" PRIu64 " failed", itid, ts);
        return false;
    }
    if (!frame->second.size()) {
        TS_LOGW("EndVsyncEvent find for itid:%u ts:%" PRIu64 " failed", itid, ts);
        return false;
    }
    auto lastFrameSlice = frame->second.back();
    lastFrameSlice->vsyncEnd_ = true;
    if (lastFrameSlice->isRsMainThread_) {
        if (lastFrameSlice->gpuEnd_) {
            traceDataCache_->GetFrameSliceData()->SetEndTimeAndFlag(
                lastFrameSlice->frameSliceRow_, ts, lastFrameSlice->expectedDur_, lastFrameSlice->expectedEndTs_);
            lastFrameSlice->endTs_ = ts;
            // for Render serivce
            frame->second.pop_back();
        }
    } else { // for app
        traceDataCache_->GetFrameSliceData()->SetEndTimeAndFlag(
            lastFrameSlice->frameSliceRow_, ts, lastFrameSlice->expectedDur_, lastFrameSlice->expectedEndTs_);
        if (ISINVALIDU32(lastFrameSlice->frameNum_)) {
            // if app's frame num not received
            traceDataCache_->GetFrameSliceData()->Erase(lastFrameSlice->frameSliceRow_);
            traceDataCache_->GetFrameSliceData()->Erase(lastFrameSlice->frameExpectedSliceRow_);
            frame->second.pop_back();
            return false;
        }
        lastFrameSlice->endTs_ = ts;
        frame->second.pop_back();
    }
    return true;
}
// only for renderservice
bool FrameFilter::StartFrameQueue(uint64_t ts, uint32_t itid)
{
    auto frame = vsyncRenderSlice_.find(itid);
    if (frame == vsyncRenderSlice_.end()) {
        TS_LOGW("StartFrameQueue find for itid:%u failed", itid);
        return false;
    }
    if (!frame->second.size()) {
        TS_LOGW("StartFrameQueue find for itid:%u failed", itid);
        return false;
    }
    auto firstFrameSlice = frame->second.front();
    firstFrameSlice->gpuEnd_ = false;
    firstFrameSlice->frameQueueStartTs_ = ts;
    return true;
}
bool FrameFilter::EndFrameQueue(uint64_t ts, uint32_t itid)
{
    auto frame = vsyncRenderSlice_.find(itid);
    if (frame == vsyncRenderSlice_.end()) {
        TS_LOGW("EndFrameQueue find for itid:%u ts:%" PRIu64 " failed", itid, ts);
        return false;
    }
    if (!frame->second.size()) {
        TS_LOGW("EndFrameQueue find for itid:%u ts:%" PRIu64 "  failed", itid, ts);
        return false;
    }
    auto firstFrameSlicePos = frame->second.begin();
    (void)traceDataCache_->GetGPUSliceData()->AppendNew(firstFrameSlicePos->get()->frameSliceRow_,
                                                        ts - firstFrameSlicePos->get()->frameQueueStartTs_);
    firstFrameSlicePos->get()->gpuEnd_ = true;
    if (firstFrameSlicePos->get()->vsyncEnd_) {
        firstFrameSlicePos->get()->endTs_ = ts;
        traceDataCache_->GetFrameSliceData()->SetEndTimeAndFlag(firstFrameSlicePos->get()->frameSliceRow_, ts,
                                                                firstFrameSlicePos->get()->expectedDur_,
                                                                firstFrameSlicePos->get()->expectedEndTs_);
        // if vsync ended
        frame->second.erase(firstFrameSlicePos);
    }
    return true;
}
void FrameFilter::Finish()
{
    vsyncRenderSlice_.clear();
    dstRenderSlice_.clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
