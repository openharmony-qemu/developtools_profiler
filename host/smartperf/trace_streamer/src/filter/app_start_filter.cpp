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

#include "app_start_filter.h"
#include "string_help.h"

namespace SysTuning {
namespace TraceStreamer {
APPStartupFilter::APPStartupFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter), mAPPStartupData_(0)
{
}

APPStartupFilter::~APPStartupFilter() = default;

void APPStartupFilter::FilterAllAPPStartupData()
{
    ParserAppStartup();
    ParserSoInitalization();
}

bool APPStartupFilter::GetProcessCreate(uint32_t row, uint64_t& startTime, std::string nameString)
{
    auto sliceData = traceDataCache_->GetConstInternalSlicesData();
    auto parentId = sliceData.ParentIdData()[row].value();
    auto depth = sliceData.Depths()[row];
    auto name = traceDataCache_->GetDataFromDict(sliceData.NamesData()[parentId]);
    while (depth--) {
        if (StartWith(name, nameString)) {
            startTime = sliceData.TimeStampData()[parentId];
            return true;
        }
        parentId = sliceData.ParentIdData()[parentId].value();
        name = traceDataCache_->GetDataFromDict(sliceData.NamesData()[parentId]);
    }
    return false;
}

bool APPStartupFilter::CaclRsDataByPid()
{
    auto frameSliceData = traceDataCache_->GetFrameSliceData();
    auto sliceData = traceDataCache_->GetConstInternalSlicesData();
    auto itor = mAPPStartupData_.begin();
    for (const auto& item : mAPPStartupData_) {
        if (item.second.empty()) {
            continue;
        }
        auto itorSecond = item.second.begin();
        auto dataIndex = item.first;
        if (itorSecond->second->ipid_ != INVALID_UINT32) {
            for (int m = 0; m < frameSliceData->Ipids().size(); m++) {
                if (itorSecond->second->ipid_ == frameSliceData->Ipids()[m] && !frameSliceData->Types()[m] &&
                    frameSliceData->Flags()[m] != INVAILD_DATA) {
                    auto startTime = sliceData.TimeStampData()[frameSliceData->CallStackIds()[m]];
                    auto callId = sliceData.CallIds()[frameSliceData->CallStackIds()[m]];
                    auto endTime = startTime + frameSliceData->Durs()[m];
                    mAPPStartupData_[dataIndex].emplace(
                        FIRST_FRAME_APP_PHASE,
                        std::move(std::make_unique<APPStartupData>(callId, itorSecond->second->ipid_,
                                                                   itorSecond->second->tid_, startTime, endTime)));
                    auto dstId = frameSliceData->Dsts()[m];
                    if (dstId == INVALID_UINT64) {
                        continue;
                    }
                    callId = sliceData.CallIds()[frameSliceData->CallStackIds()[dstId]];
                    startTime = frameSliceData->TimeStampData()[dstId];
                    endTime = startTime + frameSliceData->Durs()[dstId];
                    mAPPStartupData_[dataIndex].emplace(
                        FIRST_FRAME_RENDER_PHASE,
                        std::move(std::make_unique<APPStartupData>(callId, itorSecond->second->ipid_,
                                                                   itorSecond->second->tid_, startTime, endTime)));
                    break;
                }
            }
        }
    }
    return false;
}

void APPStartupFilter::UpdatePidByNameIndex()
{
    auto threadData = traceDataCache_->GetConstThreadData();
    for (const auto& item : mAPPStartupData_) {
        auto ipid = INVALID_UINT32;
        auto tid = INVALID_UINT32;
        if (item.second.count(UI_ABILITY_ONFOREGROUND)) {
            ipid = item.second.at(UI_ABILITY_ONFOREGROUND)->ipid_;
            tid = item.second.at(UI_ABILITY_ONFOREGROUND)->tid_;
        } else {
            for (int i = 0; i < threadData.size(); ++i) {
                if (item.first == threadData[i].nameIndex_) {
                    ipid = threadData[i].internalPid_;
                    tid = threadData[i].tid_;
                }
            }
        }
        for (const auto& itemSecond : item.second) {
            itemSecond.second->ipid_ = ipid;
            itemSecond.second->tid_ = tid;
        }
    }
}

void APPStartupFilter::AppendData()
{
    for (auto itor = mAPPStartupData_.begin(); itor != mAPPStartupData_.end(); ++itor) {
        for (auto itorSecond = itor->second.begin(); itorSecond != itor->second.end(); ++itorSecond) {
            auto item = itorSecond;
            auto endTime = INVALID_UINT64;
            if (item->first < VAILD_DATA_COUNT) {
                int num = item->first + 1;
                if ((++item) != itor->second.end() && num == item->first) {
                    endTime = (item)->second->startTime_;
                }
            } else {
                endTime = itorSecond->second->endTime_;
            }
            traceDataCache_->GetAppStartupData()->AppendNewData(
                itorSecond->second->ipid_, itorSecond->second->tid_, itorSecond->second->callid_,
                itorSecond->second->startTime_, endTime, itorSecond->first, itor->first);
        }
    }
}

DataIndex APPStartupFilter::GetThreadNameAndUpdateAPPStartupData(uint32_t row,
                                                                 const std::string& nameString,
                                                                 uint32_t startIndex)
{
    auto sliceData = traceDataCache_->GetConstInternalSlicesData();
    auto vNameString = SplitStringToVec(nameString, "##");
    if (vNameString.size() < MIN_VECTOR_SIZE_) {
        return INVALID_DATAINDEX;
    }
    auto dataIndex = traceDataCache_->GetDataIndex(vNameString[1].c_str());
    auto callId = sliceData.CallIds()[row];
    auto startTime = sliceData.TimeStampData()[row];
    mAPPStartupData_[dataIndex].insert(
        std::make_pair(startIndex, std::move(std::make_unique<APPStartupData>(callId, INVALID_UINT32, INVALID_UINT32,
                                                                              startTime, INVALID_UINT64))));
    return dataIndex;
}

void APPStartupFilter::ParserAppStartup()
{
    auto sliceData = traceDataCache_->GetConstInternalSlicesData();
    std::string mainThreadName = "";
    for (auto i = 0; i < sliceData.NamesData().size(); i++) {
        auto callId = INVALID_UINT32;
        uint64_t startTime = INVALID_UINT64;
        std::string packedName = "";
        auto& nameString = traceDataCache_->GetDataFromDict(sliceData.NamesData()[i]);
        if (StartWith(nameString, PROCESS_CREATE)) {
            auto vNameString = SplitStringToVec(nameString, "##");
            if (vNameString.size() >= MIN_VECTOR_SIZE_) {
                mainThreadName = vNameString[1];
            }
            if (!sliceData.ParentIdData()[i].has_value()) {
                TS_LOGE("callstack data has no parentId");
                return;
            }
            if (!GetProcessCreate(i, startTime, START_ABILITY)) {
                continue;
            }
            callId = sliceData.CallIds()[i];
            mAPPStartupData_[traceDataCache_->GetDataIndex(mainThreadName.c_str())].insert(std::make_pair(
                PROCESS_CREATING, std::move(std::make_unique<APPStartupData>(callId, INVALID_UINT32, INVALID_UINT32,
                                                                             startTime, INVALID_UINT64))));
        } else if (StartWith(nameString, APP_LAUNCH)) {
            GetThreadNameAndUpdateAPPStartupData(i, nameString, APPLICATION_LAUNCHING);
        } else if (StartWith(nameString, LAUNCH)) {
            auto nameIndex = GetThreadNameAndUpdateAPPStartupData(i, nameString, UI_ABILITY_LAUNCHING);
            if (nameIndex != INVALID_DATAINDEX) {
                auto thread = traceDataCache_->GetThreadData(sliceData.CallIds()[i]);
                thread->nameIndex_ = nameIndex;
            }
        } else if (StartWith(nameString, ONFOREGROUND)) {
            // callid is thread table->itid
            callId = sliceData.CallIds()[i];
            startTime = sliceData.TimeStampData()[i];
            auto threadData = traceDataCache_->GetConstThreadData();
            auto nameindex = threadData[callId].nameIndex_;
            auto ipid = threadData[callId].internalPid_;
            auto tid = threadData[callId].tid_;
            mAPPStartupData_[nameindex].insert(std::make_pair(
                UI_ABILITY_ONFOREGROUND,
                std::move(std::make_unique<APPStartupData>(callId, ipid, tid, startTime, INVALID_UINT64))));
        }
    }
    UpdatePidByNameIndex();
    CaclRsDataByPid();
    AppendData();
    return;
}

void APPStartupFilter::CalcDepthByTimeStamp(std::map<uint32_t, std::map<uint64_t, uint32_t>>::iterator it,
                                            uint32_t& depth,
                                            uint64_t endTime,
                                            uint64_t startTime)
{
    if (!it->second.empty()) {
        auto itor = it->second.begin();
        if (itor->first > startTime) {
            depth = it->second.size();
            it->second.insert(std::make_pair(endTime, it->second.size()));
        } else {
            depth = itor->second;
            it->second.erase(itor);
            it->second.insert(std::make_pair(endTime, depth));
        }
        for (const auto& item : it->second) {
            if (item.first < startTime && depth > item.second) {
                depth = item.second;
            }
        }
    } else {
        it->second.insert(std::make_pair(endTime, 0));
    }
}

void APPStartupFilter::ParserSoInitalization()
{
    std::map<uint32_t, std::map<uint64_t, uint32_t>> mMaxTimeAndDepthWithPid;
    auto sliceData = traceDataCache_->GetConstInternalSlicesData();
    auto threadData = traceDataCache_->GetConstThreadData();
    std::string nameString = "";
    for (auto i = 0; i < sliceData.NamesData().size(); i++) {
        nameString = traceDataCache_->GetDataFromDict(sliceData.NamesData()[i]);
        if (nameString.find(DLOPEN) != std::string::npos) {
            uint64_t startTime = sliceData.TimeStampData()[i];
            uint64_t endTime = startTime + sliceData.DursData()[i];
            uint32_t depth = 0;
            auto callId = sliceData.CallIds()[i];
            auto pid = threadData[callId].internalPid_;
            auto tid = threadData[callId].tid_;
            auto it = mMaxTimeAndDepthWithPid.find(pid);
            if (it == mMaxTimeAndDepthWithPid.end()) {
                mMaxTimeAndDepthWithPid.insert(std::make_pair(pid, std::map<uint64_t, uint32_t>{{endTime, 0}}));
                traceDataCache_->GetStaticInitalizationData()->AppendNewData(pid, tid, callId, startTime, endTime,
                                                                             sliceData.NamesData()[i], depth);
                continue;
            } else {
                CalcDepthByTimeStamp(it, depth, endTime, startTime);
                traceDataCache_->GetStaticInitalizationData()->AppendNewData(threadData[callId].internalPid_,
                                                                             threadData[callId].tid_, callId, startTime,
                                                                             endTime, sliceData.NamesData()[i], depth);
            }
        }
    }
    return;
}
} // namespace TraceStreamer
} // namespace SysTuning
