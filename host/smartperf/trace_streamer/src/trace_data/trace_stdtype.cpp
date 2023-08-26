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

#include "trace_stdtype.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include "string_to_numerical.h"
namespace SysTuning {
namespace TraceStdtype {
constexpr int32_t MAX_SIZE_LEN = 80;
constexpr uint8_t DEVICEINFO_MATCH_LAST = 2;
#define UNUSED(expr)             \
    do {                         \
        static_cast<void>(expr); \
    } while (0)
void CpuCacheBase::SetDur(uint64_t index, uint64_t dur)
{
    durs_[index] = dur;
}
TableRowId ThreadStateData::AppendThreadState(InternalTime ts,
                                              InternalTime dur,
                                              InternalCpu cpu,
                                              InternalTid itid,
                                              TableRowId idState)
{
    timeStamps_.emplace_back(ts);
    durations_.emplace_back(dur);
    itids_.emplace_back(itid);
    tids_.emplace_back(INVALID_UINT32);
    pids_.emplace_back(INVALID_UINT32);
    states_.emplace_back(idState);
    cpus_.emplace_back(cpu);
    argSetIds_.emplace_back(INVALID_UINT32);
    return itids_.size() - 1;
}

void ThreadStateData::SetDuration(TableRowId index, InternalTime dur)
{
    durations_[index] = dur;
}
void DataDict::Finish()
{
    std::string::size_type pos(0);
    for (auto i = 0; i < dataDict_.size(); i++) {
        if (dataDict_[i].empty()) {
            continue;
        }
        while ((pos = dataDict_[i].find("\"")) != std::string::npos) {
            dataDict_[i].replace(pos, 1, "\'");
        }
        while ((dataDict_[i].back() >= SPASCII_START && dataDict_[i].back() <= SPASCII_END) ||
               dataDict_[i].back() == '\r') {
            dataDict_[i].pop_back();
        }
    }
}
TableRowId ThreadStateData::UpdateDuration(TableRowId index, InternalTime ts)
{
    if (durations_[index] == INVALID_TIME) {
        durations_[index] = ts - timeStamps_[index];
    }
    return itids_[index];
}

bool ThreadStateData::End(TableRowId index, InternalTime ts)
{
    if (durations_[index] == INVALID_TIME) {
        durations_[index] = -1;
        return false;
    }
    return true;
}
void ThreadStateData::UpdateState(TableRowId index, TableRowId idState)
{
    states_[index] = idState;
}
void ThreadStateData::SetArgSetId(TableRowId index, uint32_t setId)
{
    argSetIds_[index] = setId;
}

void ThreadStateData::UpdateDuration(TableRowId index, InternalTime ts, TableRowId idState)
{
    durations_[index] = ts - timeStamps_[index];
    states_[index] = idState;
}

void ThreadStateData::UpdateTidAndPid(TableRowId index, InternalTid tid, InternalTid pid)
{
    tids_[index] = tid;
    pids_[index] = pid;
}

TableRowId ThreadStateData::UpdateDuration(TableRowId index, InternalTime ts, InternalCpu cpu, TableRowId idState)
{
    cpus_[index] = cpu;
    durations_[index] = ts - timeStamps_[index];
    states_[index] = idState;
    return itids_[index];
}

size_t SchedSlice::AppendSchedSlice(uint64_t ts,
                                    uint64_t dur,
                                    uint64_t cpu,
                                    uint32_t internalTid,
                                    uint64_t endState,
                                    uint64_t priority)
{
    timeStamps_.emplace_back(ts);
    durs_.emplace_back(dur);
    cpus_.emplace_back(cpu);
    tsEnds_.emplace_back(0);
    internalTids_.emplace_back(internalTid);
    endStates_.emplace_back(endState);
    priority_.emplace_back(priority);
    argSets_.emplace_back(INVALID_UINT32);
    return Size() - 1;
}

void SchedSlice::SetDuration(size_t index, uint64_t duration)
{
    durs_[index] = duration;
    tsEnds_[index] = timeStamps_[index] + duration;
}

void SchedSlice::Update(uint64_t index, uint64_t ts, uint64_t state)
{
    durs_[index] = ts - timeStamps_[index];
    endStates_[index] = state;
}

void SchedSlice::UpdateEndState(uint64_t index, uint64_t state)
{
    endStates_[index] = state;
}

void SchedSlice::UpdateArg(uint64_t index, uint32_t argsetId)
{
    argSets_[index] = argsetId;
}
size_t CallStack::AppendInternalAsyncSlice(uint64_t startT,
                                           uint64_t durationNs,
                                           InternalTid internalTid,
                                           DataIndex cat,
                                           uint16_t nameIdentify,
                                           DataIndex name,
                                           uint8_t depth,
                                           uint64_t cookid,
                                           const std::optional<uint64_t>& parentId)
{
    AppendCommonInfo(startT, durationNs, internalTid);
    AppendCallStack(cat, name, depth, parentId);
    AppendDistributeInfo();
    cookies_.emplace_back(cookid);
    ids_.emplace_back(ids_.size());
    identifys_.emplace_back(nameIdentify + depth);
    return Size() - 1;
}
size_t CallStack::AppendInternalSlice(uint64_t startT,
                                      uint64_t durationNs,
                                      InternalTid internalTid,
                                      DataIndex cat,
                                      uint16_t nameIdentify,
                                      DataIndex name,
                                      uint8_t depth,
                                      const std::optional<uint64_t>& parentId)
{
    AppendCommonInfo(startT, durationNs, internalTid);
    AppendCallStack(cat, name, depth, parentId);
    identifys_.emplace_back(nameIdentify + depth);
    ids_.emplace_back(ids_.size());
    cookies_.emplace_back(INVALID_UINT64);
    AppendDistributeInfo();
    return Size() - 1;
}

void CallStack::AppendCommonInfo(uint64_t startT, uint64_t durationNs, InternalTid internalTid)
{
    timeStamps_.emplace_back(startT);
    durs_.emplace_back(durationNs);
    callIds_.emplace_back(internalTid);
}
void CallStack::AppendCallStack(DataIndex cat, DataIndex name, uint8_t depth, std::optional<uint64_t> parentId)
{
    parentIds_.emplace_back(parentId);
    cats_.emplace_back(cat);
    names_.emplace_back(name);
    depths_.emplace_back(depth);
}
void CallStack::SetDistributeInfo(size_t index,
                                  const std::string& chainId,
                                  const std::string& spanId,
                                  const std::string& parentSpanId,
                                  const std::string& flag,
                                  const std::string& args)
{
    chainIds_[index] = chainId;
    spanIds_[index] = spanId;
    parentSpanIds_[index] = parentSpanId;
    flags_[index] = flag;
    args_[index] = args;
    argSet_[index] = INVALID_UINT32;
}
void CallStack::AppendDistributeInfo(const std::string& chainId,
                                     const std::string& spanId,
                                     const std::string& parentSpanId,
                                     const std::string& flag,
                                     const std::string& args)
{
    chainIds_.emplace_back(chainId);
    spanIds_.emplace_back(spanId);
    parentSpanIds_.emplace_back(parentSpanId);
    flags_.emplace_back(flag);
    args_.emplace_back(args);
    argSet_.emplace_back(INVALID_UINT32);
}
void CallStack::AppendDistributeInfo()
{
    chainIds_.emplace_back("");
    spanIds_.emplace_back("");
    parentSpanIds_.emplace_back("");
    flags_.emplace_back("");
    args_.emplace_back("");
    argSet_.emplace_back(INVALID_UINT32);
}
void CallStack::SetDuration(size_t index, uint64_t timeStamp)
{
    durs_[index] = timeStamp - timeStamps_[index];
}
void CallStack::SetDurationWithFlag(size_t index, uint64_t timeStamp)
{
    durs_[index] = timeStamp - timeStamps_[index];
    flags_[index] = "1";
}

void CallStack::SetFlag(size_t index, uint8_t flag)
{
    flags_[index] = std::to_string(flag);
}
void CallStack::SetDurationEx(size_t index, uint32_t dur)
{
    durs_[index] = dur;
}

void CallStack::SetIrqDurAndArg(size_t index, uint64_t timeStamp, uint32_t argSetId)
{
    SetDuration(index, timeStamp);
    argSet_[index] = argSetId;
}
void CallStack::SetTimeStamp(size_t index, uint64_t timeStamp)
{
    timeStamps_[index] = timeStamp;
}

void CallStack::SetDepth(size_t index, uint8_t depth)
{
    depths_[index] = depth;
}
void CallStack::SetArgSetId(size_t index, uint32_t argSetId)
{
    argSet_[index] = argSetId;
}
const std::deque<std::optional<uint64_t>>& CallStack::ParentIdData() const
{
    return parentIds_;
}
const std::deque<DataIndex>& CallStack::CatsData() const
{
    return cats_;
}
const std::deque<uint16_t>& CallStack::IdentifysData() const
{
    return identifys_;
}
const std::deque<DataIndex>& CallStack::NamesData() const
{
    return names_;
}
const std::deque<uint8_t>& CallStack::Depths() const
{
    return depths_;
}
const std::deque<uint64_t>& CallStack::Cookies() const
{
    return cookies_;
}
const std::deque<uint32_t>& CallStack::CallIds() const
{
    return callIds_;
}
const std::deque<std::string>& CallStack::ChainIds() const
{
    return chainIds_;
}
const std::deque<std::string>& CallStack::SpanIds() const
{
    return spanIds_;
}
const std::deque<std::string>& CallStack::ParentSpanIds() const
{
    return parentSpanIds_;
}
const std::deque<std::string>& CallStack::Flags() const
{
    return flags_;
}
const std::deque<std::string>& CallStack::ArgsData() const
{
    return args_;
}
const std::deque<uint32_t>& CallStack::ArgSetIdsData() const
{
    return argSet_;
}

size_t ArgSet::AppendNewArg(DataIndex nameId, BaseDataType dataType, int64_t value, size_t argSet)
{
    dataTypes_.emplace_back(dataType);
    argset_.emplace_back(argSet);
    ids_.emplace_back(Size());
    values_.emplace_back(value);
    names_.emplace_back(nameId);
    return Size() - 1;
}
const std::deque<BaseDataType>& ArgSet::DataTypes() const
{
    return dataTypes_;
}
const std::deque<int64_t>& ArgSet::ValuesData() const
{
    return values_;
}
const std::deque<uint64_t>& ArgSet::ArgsData() const
{
    return argset_;
}
const std::deque<DataIndex>& ArgSet::NamesData() const
{
    return names_;
}

size_t SysMeasureFilter::AppendNewFilter(uint64_t filterId, DataIndex type, DataIndex nameId)
{
    ids_.emplace_back(filterId);
    names_.emplace_back(nameId);
    types_.emplace_back(type);
    return ids_.size() - 1;
}
const std::deque<DataIndex>& SysMeasureFilter::NamesData() const
{
    return names_;
}

const std::deque<DataIndex>& SysMeasureFilter::TypesData() const
{
    return types_;
}
size_t DataType::AppendNewDataType(BaseDataType dataType, DataIndex dataDescIndex)
{
    ids_.emplace_back(Size());
    dataTypes_.emplace_back(dataType);
    descs_.emplace_back(dataDescIndex);
    return Size() - 1;
}

const std::deque<BaseDataType>& DataType::DataTypes() const
{
    return dataTypes_;
}
const std::deque<DataIndex>& DataType::DataDesc() const
{
    return descs_;
}
size_t Filter::AppendNewFilterData(std::string type, std::string name, uint64_t sourceArgSetId)
{
    nameDeque_.emplace_back(name);
    sourceArgSetId_.emplace_back(sourceArgSetId);
    ids_.emplace_back(Size());
    typeDeque_.emplace_back(type);
    return Size() - 1;
}

size_t Measure::AppendMeasureData(uint32_t type, uint64_t timeStamp, int64_t value, uint32_t filterId)
{
    valuesDeque_.emplace_back(value);
    filterIdDeque_.emplace_back(filterId);
    typeDeque_.emplace_back(type);
    timeStamps_.emplace_back(timeStamp);
    durDeque_.emplace_back(INVALID_UINT64);
    return Size() - 1;
}

void Measure::SetDur(uint32_t row, uint64_t timeStamp)
{
    durDeque_[row] = timeStamp - timeStamps_[row];
}
size_t Raw::AppendRawData(uint32_t id, uint64_t timeStamp, uint32_t name, uint32_t cpu, uint32_t internalTid)
{
    ids_.emplace_back(id);
    timeStamps_.emplace_back(timeStamp);
    nameDeque_.emplace_back(name);
    cpuDeque_.emplace_back(cpu);
    itidDeque_.emplace_back(internalTid);
    return Size() - 1;
}

size_t ThreadMeasureFilter::AppendNewFilter(uint64_t filterId, uint32_t nameIndex, uint64_t internalTid)
{
    filterId_.emplace_back(filterId);
    nameIndex_.emplace_back(nameIndex);
    internalTids_.emplace_back(internalTid);
    return Size() - 1;
}

size_t Instants::AppendInstantEventData(uint64_t timeStamp,
                                        DataIndex nameIndex,
                                        int64_t internalTid,
                                        int64_t wakeupFromInternalPid)
{
    internalTids_.emplace_back(internalTid);
    timeStamps_.emplace_back(timeStamp);
    NameIndexs_.emplace_back(nameIndex);
    wakeupFromInternalPids_.emplace_back(wakeupFromInternalPid);
    return Size() - 1;
}
size_t LogInfo::AppendNewLogInfo(uint64_t seq,
                                 uint64_t timeStamp,
                                 uint32_t pid,
                                 uint32_t tid,
                                 DataIndex level,
                                 DataIndex tag,
                                 DataIndex context,
                                 uint64_t originTs)
{
    hilogLineSeqs_.emplace_back(seq);
    timeStamps_.emplace_back(timeStamp);
    pids_.emplace_back(pid);
    tids_.emplace_back(tid);
    levels_.emplace_back(level);
    tags_.emplace_back(tag);
    contexts_.emplace_back(context);
    originTs_.emplace_back(originTs);
    return Size() - 1;
}
const std::deque<uint64_t>& LogInfo::HilogLineSeqs() const
{
    return hilogLineSeqs_;
}
const std::deque<uint32_t>& LogInfo::Pids() const
{
    return pids_;
}
const std::deque<uint32_t>& LogInfo::Tids() const
{
    return tids_;
}
const std::deque<DataIndex>& LogInfo::Levels() const
{
    return levels_;
}
const std::deque<DataIndex>& LogInfo::Tags() const
{
    return tags_;
}
const std::deque<DataIndex>& LogInfo::Contexts() const
{
    return contexts_;
}
const std::deque<uint64_t>& LogInfo::OriginTimeStamData() const
{
    return originTs_;
}
void NativeHook::UpdateCallChainId(size_t row, uint32_t callChainId)
{
    if (row < Size()) {
        callChainIds_[row] = callChainId;
    } else {
        TS_LOGE("Native hook update callChainId failed!!!");
    }
}
size_t NativeHook::AppendNewNativeHookData(uint32_t callChainId,
                                           uint32_t ipid,
                                           uint32_t itid,
                                           std::string eventType,
                                           DataIndex subType,
                                           uint64_t timeStamp,
                                           uint64_t endTimeStamp,
                                           uint64_t duration,
                                           uint64_t addr,
                                           int64_t memSize)
{
    callChainIds_.emplace_back(callChainId);
    ipids_.emplace_back(ipid);
    itids_.emplace_back(itid);
    eventTypes_.emplace_back(eventType);
    subTypes_.emplace_back(subType);
    timeStamps_.emplace_back(timeStamp);
    endTimeStamps_.emplace_back(endTimeStamp);
    durations_.emplace_back(duration);
    addrs_.emplace_back(addr);
    memSizes_.emplace_back(memSize);
    if (eventType == ALLOC_EVET) {
        countHeapSizes_ += memSize;
        allMemSizes_.emplace_back(countHeapSizes_);
    } else if (eventType == FREE_EVENT) {
        countHeapSizes_ -= memSize;
        allMemSizes_.emplace_back(countHeapSizes_);
    } else if (eventType == MMAP_EVENT) {
        countMmapSizes_ += memSize;
        allMemSizes_.emplace_back(countMmapSizes_);
    } else if (eventType == MUNMAP_EVENT) {
        countMmapSizes_ -= memSize;
        allMemSizes_.emplace_back(countMmapSizes_);
    }
    currentSizeDurs_.emplace_back(0);
    lastCallerPathIndexs_.emplace_back(INVALID_UINT64);
    return Size() - 1;
}
void NativeHook::UpdateEndTimeStampAndDuration(size_t row, uint64_t endTimeStamp)
{
    endTimeStamps_[row] = endTimeStamp;
    durations_[row] = endTimeStamp - timeStamps_[row];
}
void NativeHook::UpdateCurrentSizeDur(size_t row, uint64_t timeStamp)
{
    currentSizeDurs_[row] = timeStamp - timeStamps_[row];
}
void NativeHook::UpdateMemMapSubType(uint64_t row, uint64_t tagId)
{
    if (row < subTypes_.size()) {
        subTypes_[row] = tagId;
    } else {
        TS_LOGE("subTypes_ row is invalid!");
    }
}
void NativeHook::UpdateLastCallerPathIndexs(std::unordered_map<uint32_t, uint64_t>& callIdToLasLibId)
{
    if (callIdToLasLibId.empty()) {
        return;
    }
    for (auto i = 0; i < Size(); ++i) {
        auto symbolIt = callIdToLasLibId.find(callChainIds_[i]);
        if (symbolIt != callIdToLasLibId.end()) {
            lastCallerPathIndexs_[i] = symbolIt->second;
        } else {
            lastCallerPathIndexs_[i] = INVALID_UINT64;
        }
    }
}
const std::deque<uint32_t>& NativeHook::CallChainIds() const
{
    return callChainIds_;
}
const std::deque<uint32_t>& NativeHook::Ipids() const
{
    return ipids_;
}
const std::deque<uint32_t>& NativeHook::Itids() const
{
    return itids_;
}
const std::deque<std::string>& NativeHook::EventTypes() const
{
    return eventTypes_;
}
const std::deque<DataIndex>& NativeHook::SubTypes() const
{
    return subTypes_;
}
const std::deque<uint64_t>& NativeHook::EndTimeStamps() const
{
    return endTimeStamps_;
}
const std::deque<uint64_t>& NativeHook::Durations() const
{
    return durations_;
}
const std::deque<uint64_t>& NativeHook::Addrs() const
{
    return addrs_;
}
const std::deque<int64_t>& NativeHook::MemSizes() const
{
    return memSizes_;
}
const std::deque<int64_t>& NativeHook::AllMemSizes() const
{
    return allMemSizes_;
}
const std::deque<uint64_t>& NativeHook::CurrentSizeDurs() const
{
    return currentSizeDurs_;
}
const std::deque<uint64_t>& NativeHook::LastCallerPathIndexs() const
{
    return lastCallerPathIndexs_;
}
size_t NativeHookFrame::AppendNewNativeHookFrame(uint32_t callChainId,
                                                 uint64_t depth,
                                                 uint64_t ip,
                                                 uint64_t sp,
                                                 DataIndex symbolName,
                                                 DataIndex filePath,
                                                 uint64_t offset,
                                                 uint64_t symbolOffset,
                                                 const std::string& vaddr)
{
    callChainIds_.emplace_back(callChainId);
    ips_.emplace_back(ip);
    sps_.emplace_back(sp);
    depths_.emplace_back(depth);
    symbolNames_.emplace_back(symbolName);
    filePaths_.emplace_back(filePath);
    offsets_.emplace_back(offset);
    symbolOffsets_.emplace_back(symbolOffset);
    vaddrs_.emplace_back(vaddr);
    return Size() - 1;
}
size_t NativeHookFrame::AppendNewNativeHookFrame(uint32_t callChainId,
                                                 uint64_t depth,
                                                 uint64_t ip,
                                                 uint64_t sp,
                                                 DataIndex symbolName,
                                                 DataIndex filePath,
                                                 uint64_t offset,
                                                 uint64_t symbolOffset)
{
    callChainIds_.emplace_back(callChainId);
    ips_.emplace_back(ip);
    sps_.emplace_back(sp);
    depths_.emplace_back(depth);
    symbolNames_.emplace_back(symbolName);
    filePaths_.emplace_back(filePath);
    offsets_.emplace_back(offset);
    symbolOffsets_.emplace_back(symbolOffset);
    return Size() - 1;
}
void NativeHookFrame::UpdateSymbolIdToNameMap(uint64_t originSymbolId, uint64_t symbolId)
{
    symbolIdToSymbolName_.insert(std::make_pair(originSymbolId, symbolId));
}
void NativeHookFrame::UpdateFrameInfo(size_t row,
                                      DataIndex symbolIndex,
                                      DataIndex filePathIndex,
                                      uint64_t offset,
                                      uint64_t symbolOffset)
{
    if (row >= Size()) {
        TS_LOGE("The updated row does not exist!");
        return;
    }
    symbolNames_[row] = symbolIndex;
    filePaths_[row] = filePathIndex;
    offsets_[row] = offset;
    symbolOffsets_[row] = symbolOffset;
}

void NativeHookFrame::UpdateSymbolId()
{
    if (symbolIdToSymbolName_.empty()) {
        return;
    }
    for (auto i = 0; i < Size(); ++i) {
        auto symbolIt = symbolIdToSymbolName_.find(symbolNames_[i]);
        if (symbolIt != symbolIdToSymbolName_.end()) {
            symbolNames_[i] = symbolIt->second;
        }
    }
}
void NativeHookFrame::UpdateSymbolId(size_t index, DataIndex symbolId)
{
    if (index < Size()) {
        symbolNames_[index] = symbolId;
    }
}
void NativeHookFrame::UpdateFileId(std::map<uint32_t, uint64_t>& filePathIdToFilePathName)
{
    if (filePathIdToFilePathName.empty()) {
        return;
    }
    for (auto i = 0; i < Size(); ++i) {
        auto symbolIt = filePathIdToFilePathName.find(filePaths_[i]);
        if (symbolIt != filePathIdToFilePathName.end()) {
            filePaths_[i] = symbolIt->second;
        }
    }
}
void NativeHookFrame::UpdateVaddrs(std::deque<std::string>& vaddrs)
{
    vaddrs_.assign(vaddrs.begin(), vaddrs.end());
}
const std::deque<uint32_t>& NativeHookFrame::CallChainIds() const
{
    return callChainIds_;
}
const std::deque<uint64_t>& NativeHookFrame::Depths() const
{
    return depths_;
}
const std::deque<uint64_t>& NativeHookFrame::Ips() const
{
    return ips_;
}
const std::deque<uint64_t>& NativeHookFrame::Sps() const
{
    return sps_;
}
const std::deque<DataIndex>& NativeHookFrame::SymbolNames() const
{
    return symbolNames_;
}
const std::deque<DataIndex>& NativeHookFrame::FilePaths() const
{
    return filePaths_;
}
const std::deque<uint64_t>& NativeHookFrame::Offsets() const
{
    return offsets_;
}
const std::deque<uint64_t>& NativeHookFrame::SymbolOffsets() const
{
    return symbolOffsets_;
}
const std::deque<std::string>& NativeHookFrame::Vaddrs() const
{
    return vaddrs_;
}

size_t NativeHookStatistic::AppendNewNativeHookStatistic(uint32_t ipid,
                                                         uint64_t timeStamp,
                                                         uint32_t callChainId,
                                                         uint32_t memoryType,
                                                         DataIndex subMemType,
                                                         uint64_t applyCount,
                                                         uint64_t releaseCount,
                                                         uint64_t applySize,
                                                         uint64_t releaseSize)
{
    ids_.emplace_back(Size());
    ipids_.emplace_back(ipid);
    timeStamps_.emplace_back(timeStamp);
    callChainIds_.emplace_back(callChainId);
    memoryTypes_.emplace_back(memoryType);
    applyCounts_.emplace_back(applyCount);
    memSubTypes_.emplace_back(subMemType);
    releaseCounts_.emplace_back(releaseCount);
    applySizes_.emplace_back(applySize);
    releaseSizes_.emplace_back(releaseSize);
    return Size() - 1;
}
const std::deque<uint32_t>& NativeHookStatistic::Ipids() const
{
    return ipids_;
}
const std::deque<uint32_t>& NativeHookStatistic::CallChainIds() const
{
    return callChainIds_;
}
const std::deque<uint32_t>& NativeHookStatistic::MemoryTypes() const
{
    return memoryTypes_;
}
const std::deque<DataIndex>& NativeHookStatistic::MemorySubTypes() const
{
    return memSubTypes_;
}
const std::deque<uint64_t>& NativeHookStatistic::ApplyCounts() const
{
    return applyCounts_;
}
const std::deque<uint64_t>& NativeHookStatistic::ReleaseCounts() const
{
    return releaseCounts_;
}
const std::deque<uint64_t>& NativeHookStatistic::ApplySizes() const
{
    return applySizes_;
}
const std::deque<uint64_t>& NativeHookStatistic::ReleaseSizes() const
{
    return releaseSizes_;
}
size_t Hidump::AppendNewHidumpInfo(uint64_t timeStamp, uint32_t fps)
{
    timeStamps_.emplace_back(timeStamp);
    fpss_.emplace_back(fps);
    return Size() - 1;
}
const std::deque<uint32_t>& Hidump::Fpss() const
{
    return fpss_;
}

size_t PerfCallChain::AppendNewPerfCallChain(uint64_t sampleId,
                                             uint32_t callChainId,
                                             uint64_t vaddrInFile,
                                             uint64_t fileId,
                                             uint64_t symbolId)
{
    ids_.emplace_back(Size());
    sampleIds_.emplace_back(sampleId);
    callChainIds_.emplace_back(callChainId);
    vaddrInFiles_.emplace_back(vaddrInFile);
    fileIds_.emplace_back(fileId);
    symbolIds_.emplace_back(symbolId);
    names_.emplace_back("");
    return Size() - 1;
}
const std::deque<uint64_t>& PerfCallChain::SampleIds() const
{
    return sampleIds_;
}
const std::deque<uint32_t>& PerfCallChain::CallChainIds() const
{
    return callChainIds_;
}
const std::deque<uint64_t>& PerfCallChain::VaddrInFiles() const
{
    return vaddrInFiles_;
}
const std::deque<uint64_t>& PerfCallChain::FileIds() const
{
    return fileIds_;
}
const std::deque<uint64_t>& PerfCallChain::SymbolIds() const
{
    return symbolIds_;
}

const std::deque<std::string>& PerfCallChain::Names() const
{
    return names_;
}
void PerfCallChain::SetName(uint64_t index, const std::string& name)
{
    names_[index] = name;
}
void PerfCallChain::Clear()
{
    CacheBase::Clear();
    sampleIds_.clear();
    callChainIds_.clear();
    vaddrInFiles_.clear();
    fileIds_.clear();
    symbolIds_.clear();
    names_.clear();
}
size_t PerfFiles::AppendNewPerfFiles(uint64_t fileIds, uint32_t serial, DataIndex symbols, DataIndex filePath)
{
    ids_.emplace_back(Size());
    fileIds_.emplace_back(fileIds);
    serials_.emplace_back(serial);
    symbols_.emplace_back(symbols);
    filePaths_.emplace_back(filePath);
    return Size() - 1;
}
const std::deque<uint64_t>& PerfFiles::FileIds() const
{
    return fileIds_;
}

const std::deque<uint32_t>& PerfFiles::Serials() const
{
    return serials_;
}
const std::deque<DataIndex>& PerfFiles::Symbols() const
{
    return symbols_;
}
const std::deque<DataIndex>& PerfFiles::FilePaths() const
{
    return filePaths_;
}

void PerfFiles::Clear()
{
    CacheBase::Clear();
    fileIds_.clear();
    serials_.clear();
    symbols_.clear();
    filePaths_.clear();
}

size_t PerfSample::AppendNewPerfSample(uint32_t sampleId,
                                       uint64_t timeStamp,
                                       uint32_t tid,
                                       uint64_t eventCount,
                                       uint64_t eventTypeId,
                                       uint64_t timestampTrace,
                                       uint64_t cpuId,
                                       uint64_t threadState)
{
    ids_.emplace_back(Size());
    sampleIds_.emplace_back(sampleId);
    timeStamps_.emplace_back(timeStamp);
    tids_.emplace_back(tid);
    eventCounts_.emplace_back(eventCount);
    eventTypeIds_.emplace_back(eventTypeId);
    timestampTraces_.emplace_back(timestampTrace);
    cpuIds_.emplace_back(cpuId);
    threadStates_.emplace_back(threadState);
    return Size() - 1;
}
const std::deque<uint32_t>& PerfSample::SampleIds() const
{
    return sampleIds_;
}
const std::deque<uint32_t>& PerfSample::Tids() const
{
    return tids_;
}
const std::deque<uint64_t>& PerfSample::EventCounts() const
{
    return eventCounts_;
}
const std::deque<uint64_t>& PerfSample::EventTypeIds() const
{
    return eventTypeIds_;
}
const std::deque<uint64_t>& PerfSample::TimestampTraces() const
{
    return timestampTraces_;
}
const std::deque<uint64_t>& PerfSample::CpuIds() const
{
    return cpuIds_;
}
const std::deque<DataIndex>& PerfSample::ThreadStates() const
{
    return threadStates_;
}

void PerfSample::Clear()
{
    CacheBase::Clear();
    sampleIds_.clear();
    tids_.clear();
    eventCounts_.clear();
    eventTypeIds_.clear();
    timestampTraces_.clear();
    cpuIds_.clear();
    threadStates_.clear();
}

size_t PerfThread::AppendNewPerfThread(uint32_t pid, uint32_t tid, DataIndex threadName)
{
    ids_.emplace_back(Size());
    pids_.emplace_back(pid);
    tids_.emplace_back(tid);
    threadNames_.emplace_back(threadName);
    return Size() - 1;
}
const std::deque<uint32_t>& PerfThread::Pids() const
{
    return pids_;
}
const std::deque<uint32_t>& PerfThread::Tids() const
{
    return tids_;
}
const std::deque<DataIndex>& PerfThread::ThreadNames() const
{
    return threadNames_;
}
void PerfThread::Clear()
{
    CacheBase::Clear();
    tids_.clear();
    pids_.clear();
    threadNames_.clear();
}
size_t PerfReport::AppendNewPerfReport(DataIndex type, DataIndex value)
{
    ids_.emplace_back(Size());
    types_.emplace_back(type);
    values_.emplace_back(value);
    return Size() - 1;
}
const std::deque<DataIndex>& PerfReport::Types() const
{
    return types_;
}
const std::deque<DataIndex>& PerfReport::Values() const
{
    return values_;
}
size_t ProcessMeasureFilter::AppendNewFilter(uint64_t id, DataIndex name, uint32_t internalPid)
{
    internalPids_.emplace_back(internalPid);
    ids_.emplace_back(id);
    names_.emplace_back(name);
    return Size() - 1;
}
size_t ClockEventData::AppendNewFilter(uint64_t id, DataIndex type, DataIndex name, uint64_t cpu)
{
    cpus_.emplace_back(cpu);
    ids_.emplace_back(id);
    types_.emplace_back(type);
    names_.emplace_back(name);
    return Size() - 1;
}
size_t ClkEventData::AppendNewFilter(uint64_t id, uint64_t rate, DataIndex name, uint64_t cpu)
{
    ids_.emplace_back(id);
    rates_.emplace_back(rate);
    names_.emplace_back(name);
    cpus_.emplace_back(cpu);
    return Size() - 1;
}
size_t SysCall::AppendSysCallData(int64_t sysCallNum, DataIndex type, uint32_t ipid, uint64_t timeStamp, int64_t ret)
{
    sysCallNums_.emplace_back(sysCallNum);
    types_.emplace_back(type);
    ipids_.emplace_back(ipid);
    timeStamps_.emplace_back(timeStamp);
    rets_.emplace_back(ret);
    return Size() - 1;
}
StatAndInfo::StatAndInfo()
{
    // sched_switch_received | sched_switch_not_match | sched_switch_not_not_supported etc.
    for (int32_t i = TRACE_EVENT_START; i < TRACE_EVENT_MAX; i++) {
        event_[i] = config_.eventNameMap_.at(static_cast<SupportedTraceEventType>(i));
    }
    for (int32_t j = STAT_EVENT_START; j < STAT_EVENT_MAX; j++) {
        stat_[j] = config_.eventErrorDescMap_.at(static_cast<StatType>(j));
    }

    for (int32_t i = TRACE_EVENT_START; i < TRACE_EVENT_MAX; i++) {
        for (int32_t j = STAT_EVENT_START; j < STAT_EVENT_MAX; j++) {
            statSeverity_[i][j] = config_.eventParserStatSeverityDescMap_.at(static_cast<SupportedTraceEventType>(i))
                                      .at(static_cast<StatType>(j));
        }
    }

    for (int32_t i = TRACE_EVENT_START; i < TRACE_EVENT_MAX; i++) {
        for (int32_t j = STAT_EVENT_START; j < STAT_EVENT_MAX; j++) {
            statSeverityDesc_[i][j] = config_.serverityLevelDescMap_.at(statSeverity_[i][j]);
        }
    }

    for (int32_t i = TRACE_EVENT_START; i < TRACE_EVENT_MAX; i++) {
        for (int32_t j = STAT_EVENT_START; j < STAT_EVENT_MAX; j++) {
            statCount_[i][j] = 0;
        }
    }
    clockid2ClockNameMap_ = {
        {TS_CLOCK_UNKNOW, "unknown"},        {TS_CLOCK_BOOTTIME, "boottime"},
        {TS_CLOCK_REALTIME, "realtime"},     {TS_CLOCK_REALTIME_COARSE, "realtime_corse"},
        {TS_MONOTONIC, "monotonic"},         {TS_MONOTONIC_COARSE, "monotonic-coarse"},
        {TS_MONOTONIC_RAW, "monotonic-raw"},
    };
}
void StatAndInfo::IncreaseStat(SupportedTraceEventType eventType, StatType type)
{
    statCount_[eventType][type]++;
}
const uint32_t& StatAndInfo::GetValue(SupportedTraceEventType eventType, StatType type) const
{
    return statCount_[eventType][type];
}
const std::string& StatAndInfo::GetEvent(SupportedTraceEventType eventType) const
{
    return event_[eventType];
}
const std::string& StatAndInfo::GetStat(StatType type) const
{
    return stat_[type];
}
const std::string& StatAndInfo::GetSeverityDesc(SupportedTraceEventType eventType, StatType type) const
{
    return statSeverityDesc_[eventType][type];
}
const StatSeverityLevel& StatAndInfo::GetSeverity(SupportedTraceEventType eventType, StatType type) const
{
    return statSeverity_[eventType][type];
}

uint64_t SymbolsData::Size() const
{
    return addrs_.size();
}
void SymbolsData::InsertSymbol(const DataIndex& name, const uint64_t& addr)
{
    addrs_.emplace_back(addr);
    funcName_.emplace_back(name);
}
const std::deque<DataIndex>& SymbolsData::GetConstFuncNames() const
{
    return funcName_;
}
const std::deque<uint64_t>& SymbolsData::GetConstAddrs() const
{
    return addrs_;
}
MetaData::MetaData()
{
    columnNames_.resize(METADATA_ITEM_MAX);
    values_.resize(METADATA_ITEM_MAX);
    columnNames_[METADATA_ITEM_DATASIZE] = METADATA_ITEM_DATASIZE_COLNAME;
    columnNames_[METADATA_ITEM_PARSETOOL_NAME] = METADATA_ITEM_PARSETOOL_NAME_COLNAME;
    columnNames_[METADATA_ITEM_PARSERTOOL_VERSION] = METADATA_ITEM_PARSERTOOL_VERSION_COLNAME;
    columnNames_[METADATA_ITEM_PARSERTOOL_PUBLISH_DATETIME] = METADATA_ITEM_PARSERTOOL_PUBLISH_DATETIME_COLNAME;
    columnNames_[METADATA_ITEM_SOURCE_FILENAME] = METADATA_ITEM_SOURCE_FILENAME_COLNAME;
    columnNames_[METADATA_ITEM_OUTPUT_FILENAME] = METADATA_ITEM_OUTPUT_FILENAME_COLNAME;
    columnNames_[METADATA_ITEM_PARSERTIME] = METADATA_ITEM_PARSERTIME_COLNAME;
    columnNames_[METADATA_ITEM_TRACE_DURATION] = METADATA_ITEM_TRACE_DURATION_COLNAME;
    columnNames_[METADATA_ITEM_SOURCE_DATETYPE] = METADATA_ITEM_SOURCE_DATETYPE_COLNAME;
    values_[METADATA_ITEM_PARSETOOL_NAME] = "trace_streamer";
}
void MetaData::SetTraceType(const std::string& traceType)
{
    values_[METADATA_ITEM_SOURCE_DATETYPE] = traceType;
}
void MetaData::SetSourceFileName(const std::string& fileName)
{
    MetaData::values_[METADATA_ITEM_SOURCE_FILENAME] = fileName;
}
void MetaData::SetOutputFileName(const std::string& fileName)
{
    MetaData::values_[METADATA_ITEM_OUTPUT_FILENAME] = fileName;
}
void MetaData::SetParserToolVersion(const std::string& version)
{
    values_[METADATA_ITEM_PARSERTOOL_VERSION] = version;
}
void MetaData::SetParserToolPublishDateTime(const std::string& datetime)
{
    values_[METADATA_ITEM_PARSERTOOL_PUBLISH_DATETIME] = datetime;
}
void MetaData::SetTraceDataSize(uint64_t dataSize)
{
    std::stringstream ss;
    ss << dataSize;
    values_[METADATA_ITEM_DATASIZE] = ss.str();
    // 	Function 'time' may return error. It is not allowed to do anything that might fail inside the constructor.
    time_t rawtime;
    struct tm* timeinfo = nullptr;
    void(time(&rawtime));
    timeinfo = localtime(&rawtime);
    char buffer[MAX_SIZE_LEN];
    strftime(buffer, MAX_SIZE_LEN, "%Y-%m-%d %H:%M:%S", timeinfo);
    values_[METADATA_ITEM_PARSERTIME].append(buffer);
    // sometimes there will be a extra \n at last
    values_[METADATA_ITEM_PARSERTIME].pop_back();
}
void MetaData::SetTraceDuration(uint64_t dur)
{
    values_[METADATA_ITEM_TRACE_DURATION] = std::to_string(dur) + " s";
}
const std::string& MetaData::Value(uint64_t row) const
{
    return values_[row];
}
const std::string& MetaData::Name(uint64_t row) const
{
    return columnNames_[row];
}
DataIndex DataDict::GetStringIndex(std::string_view str)
{
    auto hashValue = hashFun(str);
    auto itor = dataDictInnerMap_.find(hashValue);
    if (itor != dataDictInnerMap_.end()) {
        return itor->second;
    }
    mutex_.lock();
    dataDict_.emplace_back(std::string(str));
    DataIndex stringIdentity = dataDict_.size() - 1;
    dataDictInnerMap_.emplace(hashValue, stringIdentity);
    mutex_.unlock();
    return stringIdentity;
}
DataIndex DataDict::GetStringIndexNoWrite(std::string_view str) const
{
    auto hashValue = hashFun(str);
    auto itor = dataDictInnerMap_.find(hashValue);
    if (itor != dataDictInnerMap_.end()) {
        return itor->second;
    }
    return INVALID_UINT64;
}
size_t CpuUsageDetailData::AppendNewData(uint64_t newTimeStamp,
                                         uint64_t dur,
                                         double totalLoad,
                                         double userLoad,
                                         double systemLoad,
                                         int64_t threads)
{
    timeStamps_.emplace_back(newTimeStamp);
    durs_.emplace_back(dur);
    totalLoad_.emplace_back(totalLoad);
    userLoad_.emplace_back(userLoad);
    systemLoad_.emplace_back(systemLoad);
    threads_.emplace_back(threads);
    return Size() - 1;
}
const std::deque<uint64_t>& CpuUsageDetailData::Durs() const
{
    return durs_;
}
const std::deque<double>& CpuUsageDetailData::TotalLoad() const
{
    return totalLoad_;
}
const std::deque<double>& CpuUsageDetailData::UserLoad() const
{
    return userLoad_;
}
const std::deque<double>& CpuUsageDetailData::SystemLoad() const
{
    return systemLoad_;
}
const std::deque<int64_t>& CpuUsageDetailData::Threads() const
{
    return threads_;
}
size_t LiveProcessDetailData::AppendNewData(uint64_t newTimeStamp,
                                            uint64_t dur,
                                            int32_t processID,
                                            std::string processName,
                                            int32_t parentProcessID,
                                            int32_t uid,
                                            std::string userName,
                                            double cpuUsage,
                                            int32_t pssInfo,
                                            uint64_t cpuTime,
                                            int32_t threads,
                                            int64_t diskWrites,
                                            int64_t diskReads)
{
    timeStamps_.emplace_back(newTimeStamp);
    durs_.emplace_back(dur);
    processID_.emplace_back(processID);
    processName_.emplace_back(processName);
    parentProcessID_.emplace_back(parentProcessID);
    uid_.emplace_back(uid);
    userName_.emplace_back(userName);
    cpuUsage_.emplace_back(cpuUsage);
    pssInfo_.emplace_back(pssInfo);
    threads_.emplace_back(threads);
    diskWrites_.emplace_back(diskWrites);
    diskReads_.emplace_back(diskReads);
    cpuTimes_.emplace_back(cpuTime);
    return Size() - 1;
}
const std::deque<uint64_t>& LiveProcessDetailData::Durs() const
{
    return durs_;
}
const std::deque<int32_t>& LiveProcessDetailData::ProcessID() const
{
    return processID_;
}
const std::deque<std::string>& LiveProcessDetailData::ProcessName() const
{
    return processName_;
}
const std::deque<int32_t>& LiveProcessDetailData::ParentProcessID() const
{
    return parentProcessID_;
}
const std::deque<int32_t>& LiveProcessDetailData::Uid() const
{
    return uid_;
}
const std::deque<std::string>& LiveProcessDetailData::UserName() const
{
    return userName_;
}
const std::deque<double>& LiveProcessDetailData::CpuUsage() const
{
    return cpuUsage_;
}
const std::deque<int32_t>& LiveProcessDetailData::PssInfo() const
{
    return pssInfo_;
}
const std::deque<int32_t>& LiveProcessDetailData::Threads() const
{
    return threads_;
}
const std::deque<int64_t>& LiveProcessDetailData::DiskWrites() const
{
    return diskWrites_;
}
const std::deque<int64_t>& LiveProcessDetailData::DiskReads() const
{
    return diskReads_;
}

const std::deque<uint64_t>& LiveProcessDetailData::CpuTimes() const
{
    return cpuTimes_;
}

size_t NetDetailData::AppendNewNetData(uint64_t newTimeStamp,
                                       uint64_t tx,
                                       uint64_t rx,
                                       uint64_t dur,
                                       double rxSpeed,
                                       double txSpeed,
                                       uint64_t packetIn,
                                       double packetInSec,
                                       uint64_t packetOut,
                                       double packetOutSec,
                                       const std::string& netType)
{
    timeStamps_.emplace_back(newTimeStamp);
    txs_.emplace_back(tx);
    rxs_.emplace_back(rx);
    durs_.emplace_back(dur);
    txSpeeds_.emplace_back(txSpeed);
    rxSpeeds_.emplace_back(rxSpeed);
    netTypes_.emplace_back(netType);
    packetIn_.emplace_back(packetIn);
    packetInSec_.emplace_back(packetInSec);
    packetOut_.emplace_back(packetOut);
    packetOutSec_.emplace_back(packetOutSec);

    return Size() - 1;
}
const std::deque<uint64_t>& NetDetailData::Durs() const
{
    return durs_;
}
const std::deque<double>& NetDetailData::RxSpeed() const
{
    return rxSpeeds_;
}
const std::deque<double>& NetDetailData::TxSpeed() const
{
    return txSpeeds_;
}
const std::deque<std::string>& NetDetailData::NetTypes() const
{
    return netTypes_;
}
const std::deque<uint64_t>& NetDetailData::RxDatas() const
{
    return rxs_;
}
const std::deque<uint64_t>& NetDetailData::TxDatas() const
{
    return txs_;
}
const std::deque<uint64_t>& NetDetailData::PacketIn() const
{
    return packetIn_;
}
const std::deque<double>& NetDetailData::PacketInSec() const
{
    return packetInSec_;
}
const std::deque<uint64_t>& NetDetailData::PacketOut() const
{
    return packetOut_;
}
const std::deque<double>& NetDetailData::PacketOutSec() const
{
    return packetOutSec_;
}

void DiskIOData::AppendNewData(uint64_t ts,
                               uint64_t dur,
                               uint64_t rd,
                               uint64_t wr,
                               uint64_t rdPerSec,
                               uint64_t wrPerSec,
                               double rdCountPerSec,
                               double wrCountPerSec,
                               uint64_t rdCount,
                               uint64_t wrCount)
{
    timeStamps_.emplace_back(ts);
    durs_.emplace_back(dur);
    rdDatas_.emplace_back(rd);
    wrDatas_.emplace_back(wr);
    rdPerSec_.emplace_back(rdPerSec);
    wrPerSec_.emplace_back(wrPerSec);
    rdCountPerSec_.emplace_back(rdCountPerSec);
    wrCountPerSec_.emplace_back(wrCountPerSec);
    rdCountDatas_.emplace_back(rdCount);
    wrCountDatas_.emplace_back(wrCount);
}
const std::deque<uint64_t>& DiskIOData::Durs() const
{
    return durs_;
}
const std::deque<uint64_t>& DiskIOData::RdDatas() const
{
    return rdDatas_;
}
const std::deque<uint64_t>& DiskIOData::WrDatas() const
{
    return wrDatas_;
}
const std::deque<double>& DiskIOData::RdSpeedDatas() const
{
    return rdPerSec_;
}
const std::deque<double>& DiskIOData::WrSpeedDatas() const
{
    return wrPerSec_;
}

const std::deque<double>& DiskIOData::RdCountPerSecDatas() const
{
    return rdCountPerSec_;
}
const std::deque<double>& DiskIOData::WrCountPerSecDatas() const
{
    return wrCountPerSec_;
}
const std::deque<uint64_t>& DiskIOData::RdCountDatas() const
{
    return rdCountDatas_;
}
const std::deque<uint64_t>& DiskIOData::WrCountDatas() const
{
    return wrCountDatas_;
}

size_t FileSystemSample::AppendNewData(uint32_t callChainId,
                                       uint16_t type,
                                       uint32_t ipid,
                                       uint32_t itid,
                                       uint64_t startTs,
                                       uint64_t endTs,
                                       uint64_t dur,
                                       DataIndex returnValue,
                                       DataIndex errorCode,
                                       size_t size,
                                       int32_t fd,
                                       DataIndex fileId,
                                       DataIndex firstArgument,
                                       DataIndex secondArgument,
                                       DataIndex thirdArgument,
                                       DataIndex fourthArgument)
{
    callChainIds_.emplace_back(callChainId);
    types_.emplace_back(type);
    ipids_.emplace_back(ipid);
    itids_.emplace_back(itid);
    startTs_.emplace_back(startTs);
    endTs_.emplace_back(endTs);
    durs_.emplace_back(dur);
    returnValues_.emplace_back(returnValue);
    errorCodes_.emplace_back(errorCode);
    fds_.emplace_back(fd);
    fileIds_.emplace_back(fileId);
    Sizes_.emplace_back(size);
    firstArguments_.emplace_back(firstArgument);
    secondArguments_.emplace_back(secondArgument);
    thirdArguments_.emplace_back(thirdArgument);
    fourthArguments_.emplace_back(fourthArgument);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& FileSystemSample::CallChainIds() const
{
    return callChainIds_;
}
const std::deque<uint16_t>& FileSystemSample::Types() const
{
    return types_;
}
const std::deque<uint32_t>& FileSystemSample::Ipids() const
{
    return ipids_;
}
const std::deque<uint32_t>& FileSystemSample::Itids() const
{
    return itids_;
}
const std::deque<uint64_t>& FileSystemSample::StartTs() const
{
    return startTs_;
}
const std::deque<uint64_t>& FileSystemSample::EndTs() const
{
    return endTs_;
}
const std::deque<uint64_t>& FileSystemSample::Durs() const
{
    return durs_;
}
const std::deque<DataIndex>& FileSystemSample::ReturnValues() const
{
    return returnValues_;
}
const std::deque<DataIndex>& FileSystemSample::ErrorCodes() const
{
    return errorCodes_;
}
const std::deque<int32_t>& FileSystemSample::Fds() const
{
    return fds_;
}
const std::deque<DataIndex>& FileSystemSample::FileIds() const
{
    return fileIds_;
}
const std::deque<size_t>& FileSystemSample::Sizes() const
{
    return Sizes_;
}
const std::deque<DataIndex>& FileSystemSample::FirstArguments() const
{
    return firstArguments_;
}
const std::deque<DataIndex>& FileSystemSample::SecondArguments() const
{
    return secondArguments_;
}
const std::deque<DataIndex>& FileSystemSample::ThirdArguments() const
{
    return thirdArguments_;
}
const std::deque<DataIndex>& FileSystemSample::FourthArguments() const
{
    return fourthArguments_;
}

size_t PagedMemorySampleData::AppendNewData(uint32_t callChainId,
                                            uint16_t type,
                                            uint32_t ipid,
                                            uint64_t startTs,
                                            uint64_t endTs,
                                            uint64_t dur,
                                            size_t size,
                                            DataIndex addr,
                                            uint32_t itid)
{
    callChainIds_.emplace_back(callChainId);
    types_.emplace_back(type);
    ipids_.emplace_back(ipid);
    startTs_.emplace_back(startTs);
    endTs_.emplace_back(endTs);
    durs_.emplace_back(dur);
    Sizes_.emplace_back(size);
    addrs_.emplace_back(addr);
    itids_.emplace_back(itid);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& PagedMemorySampleData::CallChainIds() const
{
    return callChainIds_;
}
const std::deque<uint16_t>& PagedMemorySampleData::Types() const
{
    return types_;
}
const std::deque<uint32_t>& PagedMemorySampleData::Ipids() const
{
    return ipids_;
}
const std::deque<uint32_t>& PagedMemorySampleData::Itids() const
{
    return itids_;
}
const std::deque<uint64_t>& PagedMemorySampleData::StartTs() const
{
    return startTs_;
}
const std::deque<uint64_t>& PagedMemorySampleData::EndTs() const
{
    return endTs_;
}
const std::deque<uint64_t>& PagedMemorySampleData::Durs() const
{
    return durs_;
}
const std::deque<size_t>& PagedMemorySampleData::Sizes() const
{
    return Sizes_;
}
const std::deque<DataIndex>& PagedMemorySampleData::Addr() const
{
    return addrs_;
}

size_t EbpfCallStackData::AppendNewData(uint32_t callChainId,
                                        uint32_t depth,
                                        DataIndex ip,
                                        DataIndex symbolId,
                                        DataIndex filePathId)
{
    callChainIds_.emplace_back(callChainId);
    depths_.emplace_back(depth);
    ips_.emplace_back(ip);
    symbolIds_.emplace_back(symbolId);
    filePathIds_.emplace_back(filePathId);
    ids_.emplace_back(Size());
    return Size() - 1;
}
void EbpfCallStackData::UpdateSymbolAndFilePathIndex(size_t row, DataIndex symbolId, DataIndex filePathId)
{
    if (row >= Size()) {
        TS_LOGE("The updated row does not exist!");
        return;
    }
    symbolIds_[row] = symbolId;
    filePathIds_[row] = filePathId;
}
const std::deque<uint32_t>& EbpfCallStackData::CallChainIds() const
{
    return callChainIds_;
}
const std::deque<uint32_t>& EbpfCallStackData::Depths() const
{
    return depths_;
}
const std::deque<DataIndex>& EbpfCallStackData::Ips() const
{
    return ips_;
}
const std::deque<DataIndex>& EbpfCallStackData::SymbolIds() const
{
    return symbolIds_;
}
const std::deque<DataIndex>& EbpfCallStackData::FilePathIds() const
{
    return filePathIds_;
}
#if WITH_EBPF_HELP
size_t EbpfProcessMaps::AppendNewData(uint64_t start,
                                      uint64_t end,
                                      uint32_t offset,
                                      uint32_t pid,
                                      uint32_t fileNameLen,
                                      uint64_t fileNameIndex)
{
    starts_.emplace_back(start);
    ends_.emplace_back(end);
    offsets_.emplace_back(offset);
    pids_.emplace_back(pid);
    fileNameLens_.emplace_back(fileNameLen);
    fileNameIndexs_.emplace_back(fileNameIndex);
    ids_.emplace_back(Size());
    return Size() - 1;
}

const std::deque<uint64_t>& EbpfProcessMaps::Starts() const
{
    return starts_;
}
const std::deque<uint64_t>& EbpfProcessMaps::Ends() const
{
    return ends_;
}
const std::deque<uint32_t>& EbpfProcessMaps::Offsets() const
{
    return offsets_;
}
const std::deque<uint32_t>& EbpfProcessMaps::Pids() const
{
    return pids_;
}
const std::deque<uint32_t>& EbpfProcessMaps::FileNameLens() const
{
    return fileNameLens_;
}
const std::deque<uint64_t>& EbpfProcessMaps::FileNameIndexs() const
{
    return fileNameIndexs_;
}

size_t EbpfElf::AppendNewData(uint64_t elfId,
                              uint64_t textVaddr,
                              uint32_t textOffset,
                              uint32_t strTabLen,
                              uint32_t symTabLen,
                              uint32_t fileNameLen,
                              uint32_t symEntLen,
                              uint64_t fileNameIndex)
{
    elfIds_.emplace_back(elfId);
    textVaddrs_.emplace_back(textVaddr);
    textOffsets_.emplace_back(textOffset);
    strTabLens_.emplace_back(strTabLen);
    symTabLens_.emplace_back(symTabLen);
    fileNameLens_.emplace_back(fileNameLen);
    symEntLens_.emplace_back(symEntLen);
    fileNameIndexs_.emplace_back(fileNameIndex);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint64_t>& EbpfElf::ElfIds() const
{
    return elfIds_;
}
const std::deque<uint64_t>& EbpfElf::TextVaddrs() const
{
    return textVaddrs_;
}
const std::deque<uint32_t>& EbpfElf::TextOffsets() const
{
    return textOffsets_;
}
const std::deque<uint32_t>& EbpfElf::StrTabLens() const
{
    return strTabLens_;
}
const std::deque<uint32_t>& EbpfElf::SymTabLens() const
{
    return symTabLens_;
}
const std::deque<uint32_t>& EbpfElf::FileNameLens() const
{
    return fileNameLens_;
}
const std::deque<uint32_t>& EbpfElf::SymEntLens() const
{
    return symEntLens_;
}
const std::deque<uint64_t>& EbpfElf::FileNameIndexs() const
{
    return fileNameIndexs_;
}

size_t EbpfElfSymbol::AppendNewData(uint64_t elfId, uint32_t stName, uint64_t stValue, uint64_t stSize)
{
    elfIds_.emplace_back(elfId);
    stNames_.emplace_back(stName);
    stValues_.emplace_back(stValue);
    stSizes_.emplace_back(stSize);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint64_t>& EbpfElfSymbol::ElfIds() const
{
    return elfIds_;
}
const std::deque<uint32_t>& EbpfElfSymbol::StNames() const
{
    return stNames_;
}
const std::deque<uint64_t>& EbpfElfSymbol::StValues() const
{
    return stValues_;
}
const std::deque<uint64_t>& EbpfElfSymbol::StSizes() const
{
    return stSizes_;
}
#endif
uint32_t AppNames::AppendAppName(uint8_t flags, DataIndex eventSource, DataIndex appName)
{
    flags_.push_back(flags);
    appNames_.push_back(eventSource);
    keyNames_.push_back(appName);
    ids_.push_back(keyNames_.size() - 1);
    return Size() - 1;
}
const std::deque<uint8_t>& AppNames::Falgs() const
{
    return flags_;
}
const std::deque<DataIndex>& AppNames::EventSourceId() const
{
    return appNames_;
}
const std::deque<DataIndex>& AppNames::AppName() const
{
    return keyNames_;
}

void SysEventMeasureData::AppendData(uint64_t serial,
                                     uint64_t ts,
                                     uint32_t nameId,
                                     uint32_t keyId,
                                     int32_t type,
                                     double numericValue,
                                     DataIndex stringValue)
{
    serial_.emplace_back(serial);
    ts_.emplace_back(ts);
    nameFilterIds_.emplace_back(nameId);
    appKeyFilterIds_.emplace_back(keyId);
    types_.emplace_back(type);
    numValues_.emplace_back(numericValue);
    stringValues_.emplace_back(stringValue);
    ids_.push_back(rowCount_);
    rowCount_++;
}
const std::deque<uint64_t>& SysEventMeasureData::Serial() const
{
    return serial_;
}
const std::deque<uint64_t>& SysEventMeasureData::Ts() const
{
    return ts_;
}
const std::deque<uint32_t>& SysEventMeasureData::NameFilterId() const
{
    return nameFilterIds_;
}
const std::deque<uint32_t>& SysEventMeasureData::AppKeyFilterId() const
{
    return appKeyFilterIds_;
}
const std::deque<int32_t>& SysEventMeasureData::Type() const
{
    return types_;
}
const std::deque<double>& SysEventMeasureData::NumValue() const
{
    return numValues_;
}
const std::deque<DataIndex>& SysEventMeasureData::StringValue() const
{
    return stringValues_;
}
void DeviceStateData::AppendNewData(int32_t brightness,
                                    int32_t btState,
                                    int32_t location,
                                    int32_t wifi,
                                    int32_t streamDefault,
                                    int32_t voiceCall,
                                    int32_t music,
                                    int32_t streamRing,
                                    int32_t media,
                                    int32_t voiceAssistant,
                                    int32_t system,
                                    int32_t alarm,
                                    int32_t notification,
                                    int32_t btSco,
                                    int32_t enforcedAudible,
                                    int32_t streamDtmf,
                                    int32_t streamTts,
                                    int32_t accessibility,
                                    int32_t recording,
                                    int32_t streamAll)
{
    brightness_.emplace_back(brightness);
    btStates_.emplace_back(btState);
    locations_.emplace_back(location);
    wifis_.emplace_back(wifi);
    streamDefaults_.emplace_back(streamDefault);
    voiceCalls_.emplace_back(voiceCall);
    musics_.emplace_back(music);
    streamRings_.emplace_back(streamRing);
    medias_.emplace_back(media);
    voiceAssistants_.emplace_back(voiceAssistant);
    systems_.emplace_back(system);
    alarms_.emplace_back(alarm);
    notifications_.emplace_back(notification);
    btScos_.emplace_back(btSco);
    enforcedAudibles_.emplace_back(enforcedAudible);
    streamDtmfs_.emplace_back(streamDtmf);
    streamTts_.emplace_back(streamTts);
    accessibilitys_.emplace_back(accessibility);
    recordings_.emplace_back(recording);
    streamAlls_.emplace_back(streamAll);
    ids_.push_back(rowCounts_);
    rowCounts_++;
}
const std::deque<int32_t>& DeviceStateData::Brightness() const
{
    return brightness_;
}
const std::deque<int32_t>& DeviceStateData::BtState() const
{
    return btStates_;
}
const std::deque<int32_t>& DeviceStateData::Location() const
{
    return locations_;
}
const std::deque<int32_t>& DeviceStateData::Wifi() const
{
    return wifis_;
}
const std::deque<int32_t>& DeviceStateData::StreamDefault() const
{
    return streamDefaults_;
}
const std::deque<int32_t>& DeviceStateData::VoiceCall() const
{
    return voiceCalls_;
}
const std::deque<int32_t>& DeviceStateData::Music() const
{
    return musics_;
}
const std::deque<int32_t>& DeviceStateData::StreamRing() const
{
    return streamRings_;
}
const std::deque<int32_t>& DeviceStateData::Media() const
{
    return medias_;
}
const std::deque<int32_t>& DeviceStateData::VoiceAssistant() const
{
    return voiceAssistants_;
}
const std::deque<int32_t>& DeviceStateData::System() const
{
    return systems_;
}
const std::deque<int32_t>& DeviceStateData::Alarm() const
{
    return alarms_;
}
const std::deque<int32_t>& DeviceStateData::Notification() const
{
    return notifications_;
}
const std::deque<int32_t>& DeviceStateData::BtSco() const
{
    return btScos_;
}
const std::deque<int32_t>& DeviceStateData::EnforcedAudible() const
{
    return enforcedAudibles_;
}
const std::deque<int32_t>& DeviceStateData::StreamDtmf() const
{
    return streamDtmfs_;
}
const std::deque<int32_t>& DeviceStateData::StreamTts() const
{
    return streamTts_;
}
const std::deque<int32_t>& DeviceStateData::Accessibility() const
{
    return accessibilitys_;
}
const std::deque<int32_t>& DeviceStateData::Recording() const
{
    return recordings_;
}
const std::deque<int32_t>& DeviceStateData::StreamAll() const
{
    return streamAlls_;
}
void TraceConfigData::AppendNewData(std::string traceSource, std::string key, std::string value)
{
    traceSource_.emplace_back(traceSource);
    key_.emplace_back(key);
    value_.emplace_back(value);
    ids_.push_back(rowCounts_);
    rowCounts_++;
}
const std::deque<std::string>& TraceConfigData::TraceSource() const
{
    return traceSource_;
}
const std::deque<std::string>& TraceConfigData::Key() const
{
    return key_;
}
const std::deque<std::string>& TraceConfigData::Value() const
{
    return value_;
}
void SmapsData::AppendNewData(uint64_t timeStamp,
                              uint64_t ipid,
                              std::string startAddr,
                              std::string endAddr,
                              uint64_t dirty,
                              uint64_t swapper,
                              uint64_t rss,
                              uint64_t pss,
                              uint64_t size,
                              double reside,
                              DataIndex protectionId,
                              DataIndex pathId,
                              uint64_t shared_clean,
                              uint64_t shared_dirty,
                              uint64_t private_clean,
                              uint64_t private_dirty,
                              uint64_t swap,
                              uint64_t swap_pss,
                              uint32_t type)
{
    timeStamps_.emplace_back(timeStamp);
    ipids_.emplace_back(ipid);
    startAddrs_.emplace_back(startAddr);
    endAddrs_.emplace_back(endAddr);
    dirtys_.emplace_back(dirty);
    swappers_.emplace_back(swapper);
    rss_.emplace_back(rss);
    pss_.emplace_back(pss);
    sizes_.emplace_back(size);
    resides_.emplace_back(reside);
    protectionIds_.emplace_back(protectionId);
    pathIds_.emplace_back(pathId);
    sharedClean_.emplace_back(shared_clean);
    sharedDirty_.emplace_back(shared_dirty);
    privateClean_.emplace_back(private_clean);
    privateDirty_.emplace_back(private_dirty);
    swap_.emplace_back(swap);
    swapPss_.emplace_back(swap_pss);
    type_.emplace_back(type);
    ids_.push_back(rowCount_);
    rowCount_++;
}
const std::deque<uint64_t>& SmapsData::TimeStamps() const
{
    return timeStamps_;
}
const std::deque<uint64_t>& SmapsData::Ipids() const
{
    return ipids_;
}
const std::deque<std::string>& SmapsData::StartAddrs() const
{
    return startAddrs_;
}
const std::deque<std::string>& SmapsData::EndAddrs() const
{
    return endAddrs_;
}
const std::deque<uint64_t>& SmapsData::Dirtys() const
{
    return dirtys_;
}
const std::deque<uint64_t>& SmapsData::Swappers() const
{
    return swappers_;
}
const std::deque<uint64_t>& SmapsData::Rss() const
{
    return rss_;
}
const std::deque<uint64_t>& SmapsData::Pss() const
{
    return pss_;
}
const std::deque<uint64_t>& SmapsData::Sizes() const
{
    return sizes_;
}
const std::deque<double>& SmapsData::Resides() const
{
    return resides_;
}
const std::deque<DataIndex>& SmapsData::ProtectionIds() const
{
    return protectionIds_;
}
const std::deque<DataIndex>& SmapsData::PathIds() const
{
    return pathIds_;
}
const std::deque<uint64_t>& SmapsData::SharedClean() const
{
    return sharedClean_;
}
const std::deque<uint64_t>& SmapsData::SharedDirty() const
{
    return sharedDirty_;
}
const std::deque<uint64_t>& SmapsData::PrivateClean() const
{
    return privateClean_;
}
const std::deque<uint64_t>& SmapsData::PrivateDirty() const
{
    return privateDirty_;
}
const std::deque<uint64_t>& SmapsData::Swap() const
{
    return swap_;
}
const std::deque<uint64_t>& SmapsData::SwapPss() const
{
    return swapPss_;
}
const std::deque<uint32_t>& SmapsData::Type() const
{
    return type_;
}
void BioLatencySampleData::AppendNewData(uint32_t callChainId,
                                         uint64_t type,
                                         uint32_t ipid,
                                         uint32_t itid,
                                         uint64_t startTs,
                                         uint64_t endTs,
                                         uint64_t latencyDur,
                                         uint32_t tier,
                                         uint64_t size,
                                         uint64_t blockNumber,
                                         uint64_t filePathId,
                                         uint64_t durPer4k)
{
    callChainIds_.emplace_back(callChainId);
    types_.emplace_back(type);
    ipids_.emplace_back(ipid);
    itids_.emplace_back(itid);
    startTs_.emplace_back(startTs);
    endTs_.emplace_back(endTs);
    latencyDurs_.emplace_back(latencyDur);
    tiers_.emplace_back(tier);
    sizes_.emplace_back(size);
    blockNumbers_.emplace_back(blockNumber);
    filePathIds_.emplace_back(filePathId);
    durPer4ks_.emplace_back(durPer4k);
    ids_.emplace_back(rowCount_);
    rowCount_++;
}
const std::deque<uint32_t>& BioLatencySampleData::CallChainIds() const
{
    return callChainIds_;
}
const std::deque<uint64_t>& BioLatencySampleData::Types() const
{
    return types_;
}
const std::deque<uint32_t>& BioLatencySampleData::Ipids() const
{
    return ipids_;
}
const std::deque<uint32_t>& BioLatencySampleData::Itids() const
{
    return itids_;
}
const std::deque<uint64_t>& BioLatencySampleData::StartTs() const
{
    return startTs_;
}
const std::deque<uint64_t>& BioLatencySampleData::EndTs() const
{
    return endTs_;
}
const std::deque<uint64_t>& BioLatencySampleData::LatencyDurs() const
{
    return latencyDurs_;
}
const std::deque<uint32_t>& BioLatencySampleData::Tiers() const
{
    return tiers_;
}
const std::deque<uint64_t>& BioLatencySampleData::Sizes() const
{
    return sizes_;
}
const std::deque<uint64_t>& BioLatencySampleData::BlockNumbers() const
{
    return blockNumbers_;
}
const std::deque<uint64_t>& BioLatencySampleData::FilePathIds() const
{
    return filePathIds_;
}
const std::deque<uint64_t>& BioLatencySampleData::DurPer4k() const
{
    return durPer4ks_;
}
DataSourceClockIdData::DataSourceClockIdData()
    : dataSource2ClockIdMap_({{DATA_SOURCE_TYPE_TRACE, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_MEM, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_HILOG, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_NATIVEHOOK, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_FPS, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_NETWORK, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_DISKIO, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_CPU, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_PROCESS, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_HISYSEVENT, TS_CLOCK_UNKNOW},
                              {DATA_SOURCE_TYPE_JSMEMORY, TS_CLOCK_UNKNOW}}),
      dataSource2PluginNameMap_({
          {DATA_SOURCE_TYPE_TRACE, "ftrace-plugin"},
          {DATA_SOURCE_TYPE_MEM, "memory-plugin"},
          {DATA_SOURCE_TYPE_HILOG, "hilog-plugin"},
          {DATA_SOURCE_TYPE_NATIVEHOOK, "nativehook"},
          {DATA_SOURCE_TYPE_FPS, "hidump-plugin"},
          {DATA_SOURCE_TYPE_NETWORK, "network-plugin"},
          {DATA_SOURCE_TYPE_DISKIO, "diskio-plugin"},
          {DATA_SOURCE_TYPE_CPU, "cpu-plugin"},
          {DATA_SOURCE_TYPE_PROCESS, "process-plugin"},
          {DATA_SOURCE_TYPE_HISYSEVENT, "hisysevent-plugin"},
          {DATA_SOURCE_TYPE_JSMEMORY, "arkts-plugin"},
      })
{
}
void DataSourceClockIdData::Finish()
{
    for (auto i = dataSource2ClockIdMap_.begin(); i != dataSource2ClockIdMap_.end(); i++) {
        if (i->second) { // ignore the empty datasource, for which the clockid is default TS_CLOCK_UNKNOW 0
            dataSourceNames_.emplace_back(dataSource2PluginNameMap_.at(i->first));
            clockIds_.emplace_back(i->second);
        }
    }
}
void DataSourceClockIdData::SetDataSourceClockId(DataSourceType source, uint32_t id)
{
    dataSource2ClockIdMap_.at(source) = id;
}
size_t FrameSlice::AppendFrame(uint64_t ts, uint32_t ipid, uint32_t itid, uint32_t vsyncId, uint64_t callStackSliceId)
{
    timeStamps_.emplace_back(ts);
    ipids_.emplace_back(ipid);
    internalTids_.emplace_back(itid);
    vsyncIds_.emplace_back(vsyncId);
    callStackIds_.emplace_back(callStackSliceId);
    endTss_.emplace_back(INVALID_UINT64);
    dsts_.emplace_back(INVALID_UINT64);
    ids_.emplace_back(ids_.size());
    durs_.emplace_back(INVALID_UINT64);
    types_.emplace_back(0);
    flags_.emplace_back(INVALID_UINT8);
    srcs_.emplace_back("");
    depths_.emplace_back(0);
    frameNos_.emplace_back(0);
    return Size() - 1;
}
size_t FrameSlice::AppendFrame(uint64_t ts,
                               uint32_t ipid,
                               uint32_t itid,
                               uint32_t vsyncId,
                               uint64_t callStackSliceId,
                               uint64_t end,
                               uint8_t type)
{
    auto row = AppendFrame(ts, ipid, itid, vsyncId, callStackSliceId);
    SetEndTime(row, end);
    SetType(row, type);
    depths_.emplace_back(0);
    frameNos_.emplace_back(0);
    durs_[row] = end - ts;
    return row;
}

void FrameSlice::SetEndTime(uint64_t row, uint64_t end)
{
    endTss_[row] = end;
}
void FrameSlice::SetType(uint64_t row, uint8_t type)
{
    types_[row] = type;
}
void FrameSlice::SetDst(uint64_t row, uint64_t dst)
{
    dsts_[row] = dst;
}

void FrameSlice::SetSrcs(uint64_t row, const std::vector<uint64_t>& fromSlices)
{
    std::string s = "";
    for (auto&& i : fromSlices) {
        s += std::to_string(i) + ",";
    }
    s.pop_back();
    srcs_[row] = s;
}
void FrameSlice::SetFlags(uint64_t row, const uint32_t flags)
{
    flags_[row] = flags;
}
const std::deque<uint32_t> FrameSlice::Ipids() const
{
    return ipids_;
}
const std::deque<uint32_t> FrameSlice::VsyncIds() const
{
    return vsyncIds_;
}
const std::deque<uint64_t> FrameSlice::CallStackIds() const
{
    return callStackIds_;
}
const std::deque<uint64_t> FrameSlice::EndTss() const
{
    return endTss_;
}
const std::deque<uint64_t> FrameSlice::Dsts() const
{
    return dsts_;
}
const std::deque<uint64_t> FrameSlice::Durs() const
{
    return durs_;
}
const std::deque<uint8_t> FrameSlice::Types() const
{
    return types_;
}
const std::deque<uint8_t> FrameSlice::Flags() const
{
    return flags_;
}

const std::deque<uint8_t> FrameSlice::Depths() const
{
    return depths_;
}
const std::deque<uint32_t> FrameSlice::FrameNos() const
{
    return frameNos_;
}
const std::deque<std::string>& FrameSlice::Srcs() const
{
    return srcs_;
}
void FrameSlice::UpdateCallStackSliceId(uint64_t row, uint64_t callStackSliceId)
{
    callStackIds_[row] = callStackSliceId;
}
void FrameSlice::SetEndTimeAndFlag(uint64_t row, uint64_t ts, uint64_t expectDur, uint64_t expectEnd)
{
    UNUSED(expectDur);
    durs_[row] = ts - timeStamps_[row];
    if (flags_[row] != abnormalStartEndTimeState_) {
        flags_[row] = expectEnd >= ts ? 0 : 1;
    }
}
void FrameSlice::Erase(uint64_t row)
{
    flags_[row] = INVALID_ROW;
}
size_t FrameMaps::AppendNew(FrameSlice* frameSlice, uint64_t src, uint64_t dst)
{
    timeStamps_.emplace_back(0);
    ids_.emplace_back(ids_.size());
    srcs_.push_back(src);
    dsts_.push_back(dst);
    if (frameSlice->Types().at(dst) == FrameSlice::EXPECT_SLICE) {
        uint64_t expRsStartTime = frameSlice->TimeStampData().at(dst);
        uint64_t expUiEndTime = frameSlice->TimeStampData().at(src) + frameSlice->Durs().at(src);
        if (std::abs(static_cast<long long>(expRsStartTime - expUiEndTime)) >= ONE_MILLION_NANOSECONDS) {
            auto acturalRow = dst - 1;
            frameSlice->SetFlags(acturalRow, FrameSlice::GetAbnormalStartEndTimeState());
        }
    }

    return Size() - 1;
}
const std::deque<uint64_t>& FrameMaps::SrcIndexs() const
{
    return srcs_;
}
const std::deque<uint64_t>& FrameMaps::DstIndexs() const
{
    return dsts_;
}

size_t GPUSlice::AppendNew(uint32_t frameRow, uint64_t dur)
{
    frameRows_.emplace_back(frameRow);
    durs_.emplace_back(dur);
    return Size() - 1;
}
const std::deque<uint32_t>& GPUSlice::FrameRows() const
{
    return frameRows_;
}
const std::deque<uint64_t>& GPUSlice::Durs() const
{
    return durs_;
}
size_t GPUSlice::Size() const
{
    return durs_.size();
}

size_t AppStartup::AppendNewData(uint32_t ipid,
                                 uint32_t tid,
                                 uint32_t callId,
                                 uint64_t startTime,
                                 uint64_t endTime,
                                 uint32_t startName,
                                 DataIndex packedName)
{
    ipids_.emplace_back(ipid);
    tids_.emplace_back(tid);
    callIds_.emplace_back(callId);
    startTimes_.emplace_back(startTime);
    endTimes_.emplace_back(endTime);
    startNames_.emplace_back(startName);
    packedNames_.emplace_back(packedName);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& AppStartup::Pids() const
{
    return ipids_;
}
const std::deque<uint32_t>& AppStartup::Tids() const
{
    return tids_;
}
const std::deque<uint32_t>& AppStartup::CallIds() const
{
    return callIds_;
}
const std::deque<uint64_t>& AppStartup::StartTimes() const
{
    return startTimes_;
}
const std::deque<uint64_t>& AppStartup::EndTimes() const
{
    return endTimes_;
}
const std::deque<uint32_t>& AppStartup::StartNames() const
{
    return startNames_;
}
const std::deque<DataIndex>& AppStartup::PackedNames() const
{
    return packedNames_;
}

size_t SoStaticInitalization::AppendNewData(uint32_t ipid,
                                            uint32_t tid,
                                            uint32_t callId,
                                            uint64_t startTime,
                                            uint64_t endTime,
                                            DataIndex soName,
                                            uint32_t depth)
{
    ipids_.emplace_back(ipid);
    tids_.emplace_back(tid);
    callIds_.emplace_back(callId);
    startTimes_.emplace_back(startTime);
    endTimes_.emplace_back(endTime);
    soNames_.emplace_back(soName);
    depths_.emplace_back(depth);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& SoStaticInitalization::Pids() const
{
    return ipids_;
}
const std::deque<uint32_t>& SoStaticInitalization::Tids() const
{
    return tids_;
}
const std::deque<uint32_t>& SoStaticInitalization::CallIds() const
{
    return callIds_;
}
const std::deque<uint64_t>& SoStaticInitalization::StartTimes() const
{
    return startTimes_;
}
const std::deque<uint64_t>& SoStaticInitalization::EndTimes() const
{
    return endTimes_;
}
const std::deque<DataIndex>& SoStaticInitalization::SoNames() const
{
    return soNames_;
}
const std::deque<uint32_t> SoStaticInitalization::Depths() const
{
    return depths_;
}

size_t JsHeapFiles::AppendNewData(uint32_t id,
                                  std::string filePath,
                                  uint64_t startTime,
                                  uint64_t endTime,
                                  uint64_t selfSizeCount)
{
    fileIds_.emplace_back(id);
    filePaths_.emplace_back(filePath);
    startTimes_.emplace_back(startTime);
    endTimes_.emplace_back(endTime);
    selfSizeCount_.emplace_back(selfSizeCount);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapFiles::IDs() const
{
    return fileIds_;
}
const std::deque<std::string>& JsHeapFiles::FilePaths() const
{
    return filePaths_;
}
const std::deque<uint64_t>& JsHeapFiles::StartTimes() const
{
    return startTimes_;
}
const std::deque<uint64_t>& JsHeapFiles::EndTimes() const
{
    return endTimes_;
}

const std::deque<uint64_t>& JsHeapFiles::SelfSizeCount() const
{
    return selfSizeCount_;
}

size_t JsHeapEdges::AppendNewData(uint32_t fileId,
                                  uint32_t edgeIndex,
                                  uint32_t type,
                                  uint32_t nameOrIndex,
                                  uint32_t toNode,
                                  uint32_t fromNodeId,
                                  uint32_t toNodeId)
{
    fileIds_.emplace_back(fileId);
    edgeIndexs_.emplace_back(edgeIndex);
    types_.emplace_back(type);
    nameOrIndexs_.emplace_back(nameOrIndex);
    toNodes_.emplace_back(toNode);
    fromNodeIds_.emplace_back(fromNodeId);
    toNodeIds_.emplace_back(toNodeId);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapEdges::FileIds() const
{
    return fileIds_;
}
const std::deque<uint32_t>& JsHeapEdges::EdgeIndexs() const
{
    return edgeIndexs_;
}
const std::deque<uint32_t>& JsHeapEdges::Types() const
{
    return types_;
}
const std::deque<uint32_t>& JsHeapEdges::NameOrIndexs() const
{
    return nameOrIndexs_;
}
const std::deque<uint32_t>& JsHeapEdges::ToNodes() const
{
    return toNodes_;
}
const std::deque<uint32_t>& JsHeapEdges::FromNodeIds() const
{
    return fromNodeIds_;
}
const std::deque<uint32_t>& JsHeapEdges::ToNodeIds() const
{
    return toNodeIds_;
}

size_t
    JsHeapInfo::AppendNewData(uint32_t fileId, std::string key, uint32_t type, int32_t intValue, std::string strValue)
{
    fileIds_.emplace_back(fileId);
    keys_.emplace_back(key);
    types_.emplace_back(type);
    intValues_.emplace_back(intValue);
    strValues_.emplace_back(strValue);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapInfo::FileIds() const
{
    return fileIds_;
}
const std::deque<std::string>& JsHeapInfo::Keys() const
{
    return keys_;
}
const std::deque<uint32_t>& JsHeapInfo::Types() const
{
    return types_;
}
const std::deque<int32_t>& JsHeapInfo::IntValues() const
{
    return intValues_;
}
const std::deque<std::string>& JsHeapInfo::StrValues() const
{
    return strValues_;
}

size_t JsHeapLocation::AppendNewData(uint32_t fileId,
                                     uint32_t objectIndex,
                                     uint32_t scriptId,
                                     uint32_t line,
                                     uint32_t column)
{
    fileIds_.emplace_back(fileId);
    objectIndexs_.emplace_back(objectIndex);
    scriptIds_.emplace_back(scriptId);
    lines_.emplace_back(line);
    columns_.emplace_back(column);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapLocation::FileIds() const
{
    return fileIds_;
}
const std::deque<uint32_t>& JsHeapLocation::ObjectIndexs() const
{
    return objectIndexs_;
}
const std::deque<uint32_t>& JsHeapLocation::ScriptIds() const
{
    return scriptIds_;
}
const std::deque<uint32_t>& JsHeapLocation::Lines() const
{
    return lines_;
}
const std::deque<uint32_t>& JsHeapLocation::Columns() const
{
    return columns_;
}

size_t JsHeapNodes::AppendNewData(uint32_t fileId,
                                  uint32_t nodeIndex,
                                  uint32_t type,
                                  uint32_t name,
                                  uint32_t id,
                                  uint32_t selfSize,
                                  uint32_t edgeCount,
                                  uint32_t traceNodeId,
                                  uint32_t detachedNess)
{
    fileIds_.emplace_back(fileId);
    nodeIndexs_.emplace_back(nodeIndex);
    types_.emplace_back(type);
    names_.emplace_back(name);
    nodeIds_.emplace_back(id);
    selfSizes_.emplace_back(selfSize);
    edgeCounts_.emplace_back(edgeCount);
    traceNodeIds_.emplace_back(traceNodeId);
    detachedNess_.emplace_back(detachedNess);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapNodes::FileIds() const
{
    return fileIds_;
}
const std::deque<uint32_t>& JsHeapNodes::NodeIndexs() const
{
    return nodeIndexs_;
}
const std::deque<uint32_t>& JsHeapNodes::Types() const
{
    return types_;
}
const std::deque<uint32_t>& JsHeapNodes::Names() const
{
    return names_;
}
const std::deque<uint32_t>& JsHeapNodes::NodeIds() const
{
    return nodeIds_;
}
const std::deque<uint32_t>& JsHeapNodes::SelfSizes() const
{
    return selfSizes_;
}
const std::deque<uint32_t>& JsHeapNodes::EdgeCounts() const
{
    return edgeCounts_;
}
const std::deque<uint32_t>& JsHeapNodes::TraceNodeIds() const
{
    return traceNodeIds_;
}
const std::deque<uint32_t>& JsHeapNodes::DetachedNess() const
{
    return detachedNess_;
}

size_t JsHeapSample::AppendNewData(uint32_t fileId, uint64_t timeStampUs, uint32_t lastAssignedId)
{
    fileIds_.emplace_back(fileId);
    timeStampUs_.emplace_back(timeStampUs);
    lastAssignedIds_.emplace_back(lastAssignedId);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapSample::FileIds() const
{
    return fileIds_;
}
const std::deque<uint64_t>& JsHeapSample::TimeStampUs() const
{
    return timeStampUs_;
}
const std::deque<uint32_t>& JsHeapSample::LastAssignedIds() const
{
    return lastAssignedIds_;
}

size_t JsHeapString::AppendNewData(uint32_t fileId, uint32_t fileIndex, std::string string)
{
    fileIds_.emplace_back(fileId);
    fileIndexs_.emplace_back(fileIndex);
    strings_.emplace_back(string);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapString::FileIds() const
{
    return fileIds_;
}
const std::deque<uint64_t>& JsHeapString::FileIndexs() const
{
    return fileIndexs_;
}
const std::deque<std::string>& JsHeapString::Strings() const
{
    return strings_;
}

size_t JsHeapTraceFuncInfo::AppendNewData(uint32_t fileId,
                                          uint32_t functionIndex,
                                          uint32_t functionId,
                                          uint32_t name,
                                          uint32_t scriptName,
                                          uint32_t scriptId,
                                          uint32_t line,
                                          uint32_t column)
{
    fileIds_.emplace_back(fileId);
    functionIndexs_.emplace_back(functionIndex);
    functionIds_.emplace_back(functionId);
    names_.emplace_back(name);
    scriptNames_.emplace_back(scriptName);
    scriptIds_.emplace_back(scriptId);
    lines_.emplace_back(line);
    columns_.emplace_back(column);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapTraceFuncInfo::FileIds() const
{
    return fileIds_;
}
const std::deque<uint32_t>& JsHeapTraceFuncInfo::FunctionIndexs() const
{
    return functionIndexs_;
}
const std::deque<uint32_t>& JsHeapTraceFuncInfo::FunctionIds() const
{
    return functionIds_;
}
const std::deque<uint32_t>& JsHeapTraceFuncInfo::Names() const
{
    return names_;
}
const std::deque<uint32_t>& JsHeapTraceFuncInfo::ScriptNames() const
{
    return scriptNames_;
}
const std::deque<uint32_t>& JsHeapTraceFuncInfo::ScriptIds() const
{
    return scriptIds_;
}
const std::deque<uint32_t>& JsHeapTraceFuncInfo::Lines() const
{
    return lines_;
}
const std::deque<uint32_t>& JsHeapTraceFuncInfo::Columns() const
{
    return columns_;
}

size_t JsHeapTraceNode::AppendNewData(uint32_t fileId,
                                      uint32_t traceNodeId,
                                      uint32_t functionInfoIndex,
                                      uint32_t count,
                                      uint32_t size,
                                      int32_t parentId)
{
    fileIds_.emplace_back(fileId);
    traceNodeIds_.emplace_back(traceNodeId);
    functionInfoIndexs_.emplace_back(functionInfoIndex);
    counts_.emplace_back(count);
    sizes_.emplace_back(size);
    parentIds_.emplace_back(parentId);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsHeapTraceNode::FileIds() const
{
    return fileIds_;
}
const std::deque<uint32_t>& JsHeapTraceNode::TraceNodeIDs() const
{
    return traceNodeIds_;
}
const std::deque<uint32_t>& JsHeapTraceNode::FunctionInfoIndexs() const
{
    return functionInfoIndexs_;
}
const std::deque<uint32_t>& JsHeapTraceNode::Counts() const
{
    return counts_;
}
const std::deque<uint32_t>& JsHeapTraceNode::NodeSizes() const
{
    return sizes_;
}
const std::deque<int32_t>& JsHeapTraceNode::ParentIds() const
{
    return parentIds_;
}

size_t JsCpuProfilerNode::AppendNewData(uint32_t functionId,
                                        uint32_t functionName,
                                        std::string scriptId,
                                        uint32_t url,
                                        uint32_t lineNumber,
                                        uint32_t columnNumber,
                                        uint32_t hitCount,
                                        std::string children,
                                        uint32_t parent)
{
    functionIds_.emplace_back(functionId);
    functionNames_.emplace_back(functionName);
    scriptIds_.emplace_back(scriptId);
    urls_.emplace_back(url);
    lineNumbers_.emplace_back(lineNumber);
    columnNumbers_.emplace_back(columnNumber);
    hitCounts_.emplace_back(hitCount);
    children_.emplace_back(children);
    parents_.emplace_back(parent);
    ids_.emplace_back(Size());
    return Size() - 1;
}

const std::deque<uint32_t>& JsCpuProfilerNode::FunctionIds() const
{
    return functionIds_;
}
const std::deque<uint32_t>& JsCpuProfilerNode::FunctionNames() const
{
    return functionNames_;
}
const std::deque<std::string>& JsCpuProfilerNode::ScriptIds() const
{
    return scriptIds_;
}
const std::deque<uint32_t>& JsCpuProfilerNode::Urls() const
{
    return urls_;
}
const std::deque<uint32_t>& JsCpuProfilerNode::LineNumbers() const
{
    return lineNumbers_;
}
const std::deque<int32_t>& JsCpuProfilerNode::ColumnNumbers() const
{
    return columnNumbers_;
}
const std::deque<int32_t>& JsCpuProfilerNode::HitCounts() const
{
    return hitCounts_;
}
const std::deque<std::string>& JsCpuProfilerNode::Children() const
{
    return children_;
}
const std::deque<uint32_t>& JsCpuProfilerNode::Parents() const
{
    return parents_;
}

size_t JsCpuProfilerSample::AppendNewData(uint32_t functionId, uint64_t startTime, uint64_t endTime, uint64_t dur)
{
    functionIds_.emplace_back(functionId);
    startTimes_.emplace_back(startTime);
    endTimes_.emplace_back(endTime);
    durs_.emplace_back(dur);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsCpuProfilerSample::FunctionIds() const
{
    return functionIds_;
}
const std::deque<uint64_t>& JsCpuProfilerSample::StartTimes() const
{
    return startTimes_;
}
const std::deque<uint64_t>& JsCpuProfilerSample::EndTimes() const
{
    return endTimes_;
}
const std::deque<uint64_t>& JsCpuProfilerSample::Durs() const
{
    return durs_;
}

size_t JsConfig::AppendNewData(uint32_t pid,
                               uint64_t type,
                               uint32_t interval,
                               uint32_t captureNumericValue,
                               uint32_t trackAllocation,
                               uint32_t cpuProfiler,
                               uint32_t cpuProfilerInterval)
{
    pids_.emplace_back(pid);
    types_.emplace_back(type);
    intervals_.emplace_back(interval);
    captureNumericValues_.emplace_back(captureNumericValue);
    trackAllocations_.emplace_back(trackAllocation);
    cpuProfilers_.emplace_back(cpuProfiler);
    cpuProfilerIntervals_.emplace_back(cpuProfilerInterval);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& JsConfig::Pids() const
{
    return pids_;
}
const std::deque<uint64_t>& JsConfig::Types() const
{
    return types_;
}
const std::deque<uint32_t>& JsConfig::Intervals() const
{
    return intervals_;
}
const std::deque<uint32_t>& JsConfig::CaptureNumericValue() const
{
    return captureNumericValues_;
}
const std::deque<uint32_t>& JsConfig::TrackAllocations() const
{
    return trackAllocations_;
}
const std::deque<uint32_t>& JsConfig::CpuProfiler() const
{
    return cpuProfilers_;
}
const std::deque<uint32_t>& JsConfig::CpuProfilerInterval() const
{
    return cpuProfilerIntervals_;
}

size_t TaskPoolInfo::AppendAllocationTaskData(uint32_t allocationTaskRow,
                                              uint32_t allocationItid,
                                              uint32_t executeId,
                                              uint32_t priority,
                                              uint32_t executeState)
{
    allocationTaskRows_.emplace_back(allocationTaskRow);
    executeTaskRows_.emplace_back(INVALID_INT32);
    returnTaskRows_.emplace_back(INVALID_INT32);
    allocationItids_.emplace_back(allocationItid);
    executeItids_.emplace_back(INVALID_INT32);
    returnItids_.emplace_back(INVALID_INT32);
    executeIds_.emplace_back(executeId);
    prioritys_.emplace_back(priority);
    executeStates_.emplace_back(executeState);
    returnStates_.emplace_back(INVALID_INT32);
    timeoutRows_.emplace_back(INVALID_INT32);
    ids_.emplace_back(Size());
    return Size() - 1;
}
size_t TaskPoolInfo::AppendExecuteTaskData(uint32_t executeTaskRow, uint32_t executeItid, uint32_t executeId)
{
    allocationTaskRows_.emplace_back(INVALID_INT32);
    executeTaskRows_.emplace_back(executeTaskRow);
    returnTaskRows_.emplace_back(INVALID_INT32);
    allocationItids_.emplace_back(INVALID_INT32);
    executeItids_.emplace_back(executeItid);
    returnItids_.emplace_back(INVALID_INT32);
    executeIds_.emplace_back(executeId);
    prioritys_.emplace_back(INVALID_INT32);
    executeStates_.emplace_back(INVALID_INT32);
    returnStates_.emplace_back(INVALID_INT32);
    timeoutRows_.emplace_back(INVALID_INT32);
    ids_.emplace_back(Size());
    return Size() - 1;
}
size_t TaskPoolInfo::AppendReturnTaskData(uint32_t returnTaskRow,
                                          uint32_t returnItid,
                                          uint32_t executeId,
                                          uint32_t returnState)
{
    allocationTaskRows_.emplace_back(INVALID_INT32);
    executeTaskRows_.emplace_back(INVALID_INT32);
    returnTaskRows_.emplace_back(returnTaskRow);
    allocationItids_.emplace_back(INVALID_INT32);
    executeItids_.emplace_back(INVALID_INT32);
    returnItids_.emplace_back(returnItid);
    executeIds_.emplace_back(executeId);
    prioritys_.emplace_back(INVALID_INT32);
    executeStates_.emplace_back(INVALID_INT32);
    returnStates_.emplace_back(returnState);
    timeoutRows_.emplace_back(INVALID_INT32);
    ids_.emplace_back(Size());
    return Size() - 1;
}
const std::deque<uint32_t>& TaskPoolInfo::AllocationTaskRows() const
{
    return allocationTaskRows_;
}
const std::deque<uint32_t>& TaskPoolInfo::ExecuteTaskRows() const
{
    return executeTaskRows_;
}
const std::deque<uint32_t>& TaskPoolInfo::ReturnTaskRows() const
{
    return returnTaskRows_;
}
const std::deque<uint32_t>& TaskPoolInfo::AllocationItids() const
{
    return allocationItids_;
}
const std::deque<uint32_t>& TaskPoolInfo::ExecuteItids() const
{
    return executeItids_;
}
const std::deque<uint32_t>& TaskPoolInfo::ReturnItids() const
{
    return returnItids_;
}
const std::deque<uint32_t>& TaskPoolInfo::ExecuteIds() const
{
    return executeIds_;
}
const std::deque<uint32_t>& TaskPoolInfo::Prioritys() const
{
    return prioritys_;
}
const std::deque<uint32_t>& TaskPoolInfo::ExecuteStates() const
{
    return executeStates_;
}
const std::deque<uint32_t>& TaskPoolInfo::ReturnStates() const
{
    return returnStates_;
}
const std::deque<uint32_t>& TaskPoolInfo::TimeoutRows() const
{
    return timeoutRows_;
}
void TaskPoolInfo::UpdateAllocationTaskData(uint32_t index,
                                            uint32_t allocationTaskRow,
                                            uint32_t allocationItid,
                                            uint32_t priority,
                                            uint32_t executeState)
{
    if (index <= Size()) {
        allocationTaskRows_[index] = allocationTaskRow;
        allocationItids_[index] = allocationItid;
        prioritys_[index] = priority;
        executeStates_[index] = executeState;
    }
}
void TaskPoolInfo::UpdateExecuteTaskData(uint32_t index, uint32_t executeTaskRow, uint32_t executeItid)
{
    if (index <= Size()) {
        executeTaskRows_[index] = executeTaskRow;
        executeItids_[index] = executeItid;
    }
}
void TaskPoolInfo::UpdateReturnTaskData(uint32_t index,
                                        uint32_t returnTaskRow,
                                        uint32_t returnItid,
                                        uint32_t returnState)
{
    if (index <= Size()) {
        returnTaskRows_[index] = returnTaskRow;
        returnItids_[index] = returnItid;
        returnStates_[index] = returnState;
    }
}
void TaskPoolInfo::AppendTimeoutRow(uint32_t index, uint32_t timeoutRow)
{
    if (index <= Size()) {
        timeoutRows_[index] = timeoutRow;
    }
}
TableRowId Animation::AppendAnimation(InternalTime startPoint)
{
    inputTimes_.emplace_back(INVALID_TIME);
    startPoints_.emplace_back(startPoint);
    endPoins_.emplace_back(INVALID_TIME);
    ids_.emplace_back(Size());
    return ids_.size() - 1;
}
void Animation::UpdateStartPoint(TableRowId index, InternalTime startPoint)
{
    if (index <= Size()) {
        startPoints_[index] = startPoint;
    }
}
void Animation::UpdateEndPoint(TableRowId index, InternalTime endPoint)
{
    if (index <= Size()) {
        endPoins_[index] = endPoint;
    }
}
size_t Animation::Size() const
{
    return ids_.size();
}
const std::deque<InternalTime>& Animation::InputTimes() const
{
    return inputTimes_;
}
const std::deque<InternalTime>& Animation::StartPoints() const
{
    return startPoints_;
}
const std::deque<InternalTime>& Animation::EndPoints() const
{
    return endPoins_;
}
const std::deque<uint64_t>& Animation::IdsData() const
{
    return ids_;
}
void Animation::Clear()
{
    inputTimes_.clear();
    startPoints_.clear();
    endPoins_.clear();
    ids_.clear();
}
uint32_t DeviceInfo::PhysicalWidth() const
{
    return physicalWidth_;
}
uint32_t DeviceInfo::PhysicalHeight() const
{
    return physicalHeight_;
}
uint32_t DeviceInfo::PhysicalFrameRate() const
{
    return physicalFrameRate_;
}
void DeviceInfo::UpdateWidthAndHeight(const std::smatch& matcheLine)
{
    if (matcheLine.size() > DEVICEINFO_MATCH_LAST) {
        uint8_t matcheIndex = 0;
        physicalWidth_ = base::StrToInt<uint32_t>(matcheLine[++matcheIndex].str()).value();
        physicalHeight_ = base::StrToInt<uint32_t>(matcheLine[++matcheIndex].str()).value();
    }
}
void DeviceInfo::UpdateFrameRate(uint32_t frameRate)
{
    physicalFrameRate_ = frameRate;
}
void DeviceInfo::Clear()
{
    physicalWidth_ = INVALID_UINT32;
    physicalHeight_ = INVALID_UINT32;
    physicalFrameRate_ = INVALID_UINT32;
}
TableRowId DynamicFrame::AppendDynamicFrame(DataIndex nameId)
{
    names_.emplace_back(nameId);
    ids_.emplace_back(Size());
    xs_.emplace_back(INVALID_UINT32);
    ys_.emplace_back(INVALID_UINT32);
    widths_.emplace_back(INVALID_UINT32);
    heights_.emplace_back(INVALID_UINT32);
    alphas_.emplace_back(INVALID_UINT64);
    endTimes_.emplace_back(INVALID_TIME);
    return ids_.size() - 1;
}
void DynamicFrame::UpdateNameIndex(TableRowId index, DataIndex nameId)
{
    if (index <= Size()) {
        names_[index] = nameId;
    }
}
void DynamicFrame::UpdatePosition(TableRowId index, const std::smatch& matcheLine, DataIndex alpha)
{
    if (index <= Size() && matcheLine.size() > DYNAMICFRAME_MATCH_LAST) {
        uint8_t matcheIndex = 0;
        xs_[index] = base::StrToInt<uint32_t>(matcheLine[++matcheIndex].str()).value();
        ys_[index] = base::StrToInt<uint32_t>(matcheLine[++matcheIndex].str()).value();
        widths_[index] = base::StrToInt<uint32_t>(matcheLine[++matcheIndex].str()).value();
        heights_[index] = base::StrToInt<uint32_t>(matcheLine[++matcheIndex].str()).value();
        alphas_[index] = alpha;
    }
}
void DynamicFrame::UpdateEndTime(TableRowId index, InternalTime endTime)
{
    if (index <= Size()) {
        endTimes_[index] = endTime;
    }
}
size_t DynamicFrame::Size() const
{
    return ids_.size();
}
const std::deque<uint64_t>& DynamicFrame::IdsData() const
{
    return ids_;
}
const std::deque<uint32_t>& DynamicFrame::Xs() const
{
    return xs_;
}
const std::deque<uint32_t>& DynamicFrame::Ys() const
{
    return ys_;
}
const std::deque<uint32_t>& DynamicFrame::Widths() const
{
    return widths_;
}
const std::deque<uint32_t>& DynamicFrame::Heights() const
{
    return heights_;
}
const std::deque<DataIndex>& DynamicFrame::Alphas() const
{
    return alphas_;
}
const std::deque<DataIndex>& DynamicFrame::Names() const
{
    return names_;
}
const std::deque<InternalTime>& DynamicFrame::EndTimes() const
{
    return endTimes_;
}
void DynamicFrame::Clear()
{
    xs_.clear();
    ys_.clear();
    widths_.clear();
    heights_.clear();
    alphas_.clear();
    names_.clear();
    endTimes_.clear();
    ids_.clear();
}

void AshMemData::AppendNewData(InternalPid ipid,
                               uint64_t ts,
                               uint32_t adj,
                               uint32_t fd,
                               DataIndex ashmemNameId,
                               uint64_t size,
                               uint64_t pss,
                               uint32_t ashmemId,
                               uint64_t time,
                               uint64_t refCount,
                               uint64_t purged,
                               uint32_t flag)
{
    ipids_.emplace_back(ipid);
    timeStamps_.emplace_back(ts);
    adjs_.emplace_back(adj);
    fds_.emplace_back(fd);
    ashmemNameIds_.emplace_back(ashmemNameId);
    sizes_.emplace_back(size);
    psss_.emplace_back(pss);
    ashmemIds_.emplace_back(ashmemId);
    times_.emplace_back(time);
    refCounts_.emplace_back(refCount);
    purgeds_.emplace_back(purged);
    flags_.emplace_back(flag);
    ids_.push_back(rowCount_);
    rowCount_++;
}
void AshMemData::SetFlag(uint64_t rowId, uint32_t Flag)
{
    flags_[rowId] = Flag;
}
const std::deque<InternalPid>& AshMemData::Ipids() const
{
    return ipids_;
}
const std::deque<uint32_t>& AshMemData::Adjs() const
{
    return adjs_;
}
const std::deque<uint32_t>& AshMemData::Fds() const
{
    return fds_;
}
const std::deque<DataIndex>& AshMemData::AshmemNameIds() const
{
    return ashmemNameIds_;
}
const std::deque<uint64_t>& AshMemData::Sizes() const
{
    return sizes_;
}
const std::deque<uint64_t>& AshMemData::Psss() const
{
    return psss_;
}
const std::deque<uint32_t>& AshMemData::AshmemIds() const
{
    return ashmemIds_;
}
const std::deque<uint64_t>& AshMemData::Times() const
{
    return times_;
}
const std::deque<uint64_t>& AshMemData::RefCounts() const
{
    return refCounts_;
}
const std::deque<uint64_t>& AshMemData::Purgeds() const
{
    return purgeds_;
}
const std::deque<uint32_t>& AshMemData::Flags() const
{
    return flags_;
}

void DmaMemData::AppendNewData(InternalPid ipid,
                               uint64_t ts,
                               uint32_t fd,
                               uint64_t size,
                               uint32_t ino,
                               uint32_t expPid,
                               DataIndex expTaskCommId,
                               DataIndex bufNameId,
                               DataIndex expNameId,
                               uint32_t flag)
{
    ipids_.emplace_back(ipid);
    timeStamps_.emplace_back(ts);
    fds_.emplace_back(fd);
    sizes_.emplace_back(size);
    inos_.emplace_back(ino);
    expPids_.emplace_back(expPid);
    expTaskCommIds_.emplace_back(expTaskCommId);
    bufNameIds_.emplace_back(bufNameId);
    expNameIds_.emplace_back(expNameId);
    flags_.emplace_back(flag);
    ids_.push_back(rowCount_);
    rowCount_++;
}
void DmaMemData::SetFlag(uint64_t rowId, uint32_t Flag)
{
    flags_[rowId] = Flag;
}
const std::deque<InternalPid>& DmaMemData::Ipids() const
{
    return ipids_;
}
const std::deque<uint32_t>& DmaMemData::Fds() const
{
    return fds_;
}
const std::deque<uint64_t>& DmaMemData::Sizes() const
{
    return sizes_;
}
const std::deque<uint32_t>& DmaMemData::Inos() const
{
    return inos_;
}
const std::deque<uint32_t>& DmaMemData::ExpPids() const
{
    return expPids_;
}
const std::deque<DataIndex>& DmaMemData::ExpTaskCommIds() const
{
    return expTaskCommIds_;
}
const std::deque<DataIndex>& DmaMemData::BufNameIds() const
{
    return bufNameIds_;
}
const std::deque<DataIndex>& DmaMemData::ExpNameIds() const
{
    return expNameIds_;
}
const std::deque<uint32_t>& DmaMemData::Flags() const
{
    return flags_;
}

void GpuProcessMemData::AppendNewData(uint64_t ts,
                                      DataIndex gpuNameId,
                                      uint64_t allGpuSize,
                                      std::string addr,
                                      InternalPid ipid,
                                      InternalPid itid,
                                      uint64_t usedGpuSize)
{
    timeStamps_.emplace_back(ts);
    gpuNameIds_.emplace_back(gpuNameId);
    allGpuSizes_.emplace_back(allGpuSize);
    addrs_.emplace_back(addr);
    ipids_.emplace_back(ipid);
    itids_.emplace_back(itid);
    usedGpuSizes_.emplace_back(usedGpuSize);
    ids_.push_back(rowCount_);
    rowCount_++;
}
const std::deque<DataIndex>& GpuProcessMemData::GpuNameIds() const
{
    return gpuNameIds_;
}
const std::deque<uint64_t>& GpuProcessMemData::AllGpuSizes() const
{
    return allGpuSizes_;
}
const std::deque<std::string>& GpuProcessMemData::Addrs() const
{
    return addrs_;
}
const std::deque<InternalPid>& GpuProcessMemData::Ipids() const
{
    return ipids_;
}
const std::deque<InternalPid>& GpuProcessMemData::Itids() const
{
    return itids_;
}
const std::deque<uint64_t>& GpuProcessMemData::UsedGpuSizes() const
{
    return usedGpuSizes_;
}

void GpuWindowMemData::AppendNewData(uint64_t ts,
                                     DataIndex windowNameId,
                                     uint64_t windowId,
                                     DataIndex moduleNameId,
                                     DataIndex categoryNameId,
                                     uint64_t size,
                                     uint32_t count,
                                     uint64_t purgeableSize)
{
    timeStamps_.emplace_back(ts);
    windowNameIds_.emplace_back(windowNameId);
    windowIds_.emplace_back(windowId);
    moduleNameIds_.emplace_back(moduleNameId);
    categoryNameIds_.emplace_back(categoryNameId);
    sizes_.emplace_back(size);
    counts_.emplace_back(count);
    purgeableSizes_.emplace_back(purgeableSize);
    ids_.push_back(rowCount_);
    rowCount_++;
}
const std::deque<DataIndex>& GpuWindowMemData::WindowNameIds() const
{
    return windowNameIds_;
}
const std::deque<uint64_t>& GpuWindowMemData::WindowIds() const
{
    return windowIds_;
}
const std::deque<DataIndex>& GpuWindowMemData::ModuleNameIds() const
{
    return moduleNameIds_;
}
const std::deque<DataIndex>& GpuWindowMemData::CategoryNameIds() const
{
    return categoryNameIds_;
}
const std::deque<uint64_t>& GpuWindowMemData::Sizes() const
{
    return sizes_;
}
const std::deque<uint32_t>& GpuWindowMemData::Counts() const
{
    return counts_;
}
const std::deque<uint64_t>& GpuWindowMemData::PurgeableSizes() const
{
    return purgeableSizes_;
}

} // namespace TraceStdtype
} // namespace SysTuning
