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

#include "native_hook_filter.h"
#include "native_hook_config.pbreader.h"
#include <cstddef>
#include <cinttypes>
namespace SysTuning {
namespace TraceStreamer {
NativeHookFilter::NativeHookFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : OfflineSymbolizationFilter(dataCache, filter), anonMmapData_(nullptr)
{
    invalidLibPathIndexs_.insert(traceDataCache_->dataDict_.GetStringIndex("/system/lib/libc++.so"));
    invalidLibPathIndexs_.insert(traceDataCache_->dataDict_.GetStringIndex("/system/lib64/libc++.so"));
    invalidLibPathIndexs_.insert(traceDataCache_->dataDict_.GetStringIndex("/system/lib/ld-musl-aarch64.so.1"));
    invalidLibPathIndexs_.insert(traceDataCache_->dataDict_.GetStringIndex("/system/lib/ld-musl-arm.so.1"));
}

void NativeHookFilter::ParseConfigInfo(ProtoReader::BytesView& protoData)
{
    auto configReader = ProtoReader::NativeHookConfig_Reader(protoData);
    if (configReader.has_statistics_interval()) {
        isStatisticMode_ = true;
        isCallStackCompressedMode_ = true;
        isStringCompressedMode_ = true;
    }
    if (configReader.has_offline_symbolization()) {
        isOfflineSymbolizationMode_ = true;
        isCallStackCompressedMode_ = true;
        isStringCompressedMode_ = true;
        return;
    }
    if (configReader.has_callframe_compress()) {
        isCallStackCompressedMode_ = true;
        isStringCompressedMode_ = true;
        return;
    }
    if (configReader.has_string_compressed()) {
        isStringCompressedMode_ = true;
        return;
    }
    return;
}
void NativeHookFilter::AppendStackMaps(uint32_t stackid, std::vector<uint64_t>& frames)
{
    auto framesSharedPtr = std::make_shared<std::vector<uint64_t>>(frames);
    stackIdToFramesMap_.emplace(std::make_pair(stackid, framesSharedPtr));
    // allStackIdToFramesMap_ save all offline symbolic call stack
    if (isOfflineSymbolizationMode_) {
        allStackIdToFramesMap_.emplace(std::make_pair(stackid, framesSharedPtr));
    }
}
void NativeHookFilter::AppendFrameMaps(uint32_t id, const ProtoReader::BytesView& bytesView)
{
    auto frames = std::make_shared<const ProtoReader::BytesView>(bytesView);
    frameIdToFrameBytes_.emplace(std::make_pair(id, frames));
}
void NativeHookFilter::AppendFilePathMaps(uint32_t id, uint64_t fileIndex)
{
    filePathIdToFileIndex_.emplace(id, fileIndex);
    fileIndexToFilePathId_.emplace(fileIndex, id);
}
void NativeHookFilter::AppendSymbolMap(uint32_t id, uint64_t symbolIndex)
{
    symbolIdToSymbolIndex_.emplace(id, symbolIndex);
}
void NativeHookFilter::AppendThreadNameMap(uint32_t id, uint64_t threadNameIndex)
{
    threadNameIdToThreadNameIndex_.emplace(id, threadNameIndex);
}

template <class T1, class T2>
void NativeHookFilter::UpdateMap(std::unordered_map<T1, T2>& sourceMap, T1 key, T2 value)
{
    auto itor = sourceMap.find(key);
    if (itor != sourceMap.end()) {
        itor->second = value;
    } else {
        sourceMap.insert(std::make_pair(key, value));
    }
}
std::unique_ptr<NativeHookFrameInfo> NativeHookFilter::ParseFrame(const ProtoReader::DataArea& frame)
{
    ProtoReader::Frame_Reader reader(frame.Data(), frame.Size());
    uint64_t symbolIndex = INVALID_UINT64;
    uint64_t filePathIndex = INVALID_UINT64;
    if (isStringCompressedMode_) {
        if (!symbolIdToSymbolIndex_.count(reader.symbol_name_id())) {
            TS_LOGE("Native hook ParseFrame find symbol id failed!!!");
            return nullptr;
        }
        symbolIndex = symbolIdToSymbolIndex_.at(reader.symbol_name_id());

        if (!filePathIdToFileIndex_.count(reader.file_path_id())) {
            TS_LOGE("Native hook ParseFrame find file path id failed!!!");
            return nullptr;
        }
        filePathIndex = filePathIdToFileIndex_.at(reader.file_path_id());
    } else {
        symbolIndex = traceDataCache_->dataDict_.GetStringIndex(reader.symbol_name().ToStdString());
        filePathIndex = traceDataCache_->dataDict_.GetStringIndex(reader.file_path().ToStdString());
    }
    auto frameInfo = std::make_unique<NativeHookFrameInfo>(reader.ip(), reader.sp(), symbolIndex, filePathIndex,
                                                           reader.offset(), reader.symbol_offset());
    return frameInfo;
}

void NativeHookFilter::CompressStackAndFrames(ProtoReader::RepeatedDataAreaIterator<ProtoReader::BytesView> frames)
{
    std::vector<uint64_t> framesHash;
    std::string framesHashStr = "";
    for (auto itor = frames; itor; itor++) {
        std::string_view frameStr(reinterpret_cast<const char*>(itor->Data()), itor->Size());
        auto frameHash = hashFun_(frameStr);
        if (!frameHashToFrameInfoMap_.count(frameHash)) {
            // the frame compression is completed and the frame is parsed.
            auto frameInfo = ParseFrame(itor.GetDataArea());
            if (!frameInfo) {
                continue;
            }
            frameHashToFrameInfoMap_.emplace(std::make_pair(frameHash, std::move(frameInfo)));
        }
        framesHash.emplace_back(frameHash);
        framesHashStr.append("+");
        framesHashStr.append(std::to_string(frameHash));
    }
    auto stackHashValue = hashFun_(framesHashStr);
    uint32_t callChainId = INVALID_UINT32;
    if (!stackHashValueToCallChainIdMap_.count(stackHashValue)) {
        callChainId = callChainIdToStackHashValueMap_.size() + 1;
        callChainIdToStackHashValueMap_.emplace(std::make_pair(callChainId, stackHashValue));
        stackHashValueToCallChainIdMap_.emplace(std::make_pair(stackHashValue, callChainId));
        stackHashValueToFramesHashMap_.emplace(std::make_pair(stackHashValue, std::move(framesHash)));
    } else {
        callChainId = stackHashValueToCallChainIdMap_[stackHashValue];
    }
    // When compressing the call stack, update the callChainId of the nativeHook table
    auto row = traceDataCache_->GetNativeHookData()->Size() - 1;
    traceDataCache_->GetNativeHookData()->UpdateCallChainId(row, callChainId);
}
void NativeHookFilter::ParseStatisticEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView)
{
    ProtoReader::RecordStatisticsEvent_Reader reader(bytesView);
    if (ipid_ == INVALID_UINT32) {
        ipid_ = streamFilters_->processFilter_->GetOrCreateInternalPid(timeStamp, reader.pid());
    }
    uint32_t callChainId = INVALID_UINT32;
    // When the stack id is zero, there is no matching call stack
    if (isOfflineSymbolizationMode_ && reader.callstack_id()) {
        // The same call stack may have different symbolic results due to changes in the symbol table
        if (stackIdToCallChainIdMap_.count(reader.callstack_id())) {
            callChainId = stackIdToCallChainIdMap_.at(reader.callstack_id());
        } else {
            TS_LOGE("invalid callChainId, can not find stack id : %u in stackIdToCallChainIdMap_!",
                    reader.callstack_id());
        }
    } else if (reader.callstack_id()) { // when isStatisticMode_ is true, isCallStackCompressedMode_ must be true.
        // when isOfflineSymblolizationMode_ is false, the stack id is unique
        callChainId = reader.callstack_id();
    }

    DataIndex memSubType = INVALID_UINT64;
    if (reader.has_tag_name()) {
        memSubType = traceDataCache_->GetDataIndex(reader.tag_name().ToStdString());
    }
    traceDataCache_->GetNativeHookStatisticsData()->AppendNewNativeHookStatistic(
        ipid_, timeStamp, callChainId, reader.type(), memSubType, reader.apply_count(), reader.release_count(),
        reader.apply_size(), reader.release_size());
}
void NativeHookFilter::ParseAllocEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView)
{
    ProtoReader::AllocEvent_Reader allocEventReader(bytesView);
    uint32_t callChainId = INVALID_UINT32;
    // When the stack id is zero, there is no matching call stack
    if (isOfflineSymbolizationMode_ && allocEventReader.stack_id()) {
        // The same call stack may have different symbolic results due to changes in the symbol table
        if (stackIdToCallChainIdMap_.count(allocEventReader.stack_id())) {
            callChainId = stackIdToCallChainIdMap_.at(allocEventReader.stack_id());
        } else {
            TS_LOGE("invalid callChainId, can not find stack id : %u in stackIdToCallChainIdMap_!",
                    allocEventReader.stack_id());
        }
    } else if (isCallStackCompressedMode_ && allocEventReader.stack_id()) {
        // when isOfflineSymblolizationMode_ is false && isCallStackCompressedMode is true, the stack id is unique
        callChainId = allocEventReader.stack_id();
    }

    auto itid =
        streamFilters_->processFilter_->GetOrCreateThreadWithPid(allocEventReader.tid(), allocEventReader.pid());
    if (ipid_ == INVALID_UINT32) {
        ipid_ = streamFilters_->processFilter_->GetOrCreateInternalPid(timeStamp, allocEventReader.pid());
    }
    if (allocEventReader.has_thread_name_id()) {
        UpdateMap(itidToThreadNameId_, itid, allocEventReader.thread_name_id());
    }
    auto row = traceDataCache_->GetNativeHookData()->AppendNewNativeHookData(
        callChainId, ipid_, itid, "AllocEvent", INVALID_UINT64, timeStamp, 0, 0, allocEventReader.addr(),
        allocEventReader.size());
    addrToAllocEventRow_.insert(std::make_pair(allocEventReader.addr(), static_cast<uint64_t>(row)));
    if (allocEventReader.size() != 0) {
        MaybeUpdateCurrentSizeDur(row, timeStamp, true);
    }
    // Uncompressed call stack
    if (allocEventReader.has_frame_info()) {
        CompressStackAndFrames(allocEventReader.frame_info());
    }
}

void NativeHookFilter::ParseFreeEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView)
{
    ProtoReader::FreeEvent_Reader freeEventReader(bytesView);
    uint32_t callChainId = INVALID_UINT32;
    // When the stack id is zero, there is no matching call stack
    if (isOfflineSymbolizationMode_ && freeEventReader.stack_id()) {
        // The same call stack may have different symbolic results due to changes in the symbol table
        if (stackIdToCallChainIdMap_.count(freeEventReader.stack_id())) {
            callChainId = stackIdToCallChainIdMap_.at(freeEventReader.stack_id());
        } else {
            TS_LOGE("invalid callChainId, can not find stack id : %u in stackIdToCallChainIdMap_!",
                    freeEventReader.stack_id());
        }
    } else if (isCallStackCompressedMode_ && freeEventReader.stack_id()) {
        // when isOfflineSymblolizationMode_ is false && isCallStackCompressedMode is true, the stack id is unique
        callChainId = freeEventReader.stack_id();
    }
    auto itid = streamFilters_->processFilter_->GetOrCreateThreadWithPid(freeEventReader.tid(), freeEventReader.pid());
    if (ipid_ == INVALID_UINT32) {
        ipid_ = streamFilters_->processFilter_->GetOrCreateInternalPid(timeStamp, freeEventReader.pid());
    }
    if (freeEventReader.thread_name_id() != 0) {
        UpdateMap(itidToThreadNameId_, itid, freeEventReader.thread_name_id());
    }
    int64_t freeHeapSize = 0;
    // Find a matching malloc event, and if the matching fails, do not write to the database
    uint64_t row = INVALID_UINT64;
    if (addrToAllocEventRow_.count(freeEventReader.addr())) {
        row = addrToAllocEventRow_.at(freeEventReader.addr());
    }
    if (row != INVALID_UINT64 && timeStamp > traceDataCache_->GetNativeHookData()->TimeStampData()[row]) {
        addrToAllocEventRow_.erase(freeEventReader.addr());
        traceDataCache_->GetNativeHookData()->UpdateEndTimeStampAndDuration(row, timeStamp);
        freeHeapSize = traceDataCache_->GetNativeHookData()->MemSizes()[row];
    } else {
        TS_LOGD("func addr:%lu is empty", freeEventReader.addr());
        streamFilters_->statFilter_->IncreaseStat(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_DATA_INVALID);
        return;
    }

    row = traceDataCache_->GetNativeHookData()->AppendNewNativeHookData(
        callChainId, ipid_, itid, "FreeEvent", INVALID_UINT64, timeStamp, 0, 0, freeEventReader.addr(), freeHeapSize);
    if (freeHeapSize != 0) {
        MaybeUpdateCurrentSizeDur(row, timeStamp, true);
    }
    // Uncompressed call stack
    if (freeEventReader.has_frame_info()) {
        CompressStackAndFrames(freeEventReader.frame_info());
    }
}

void NativeHookFilter::ParseMmapEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView)
{
    ProtoReader::MmapEvent_Reader mMapEventReader(bytesView);
    uint32_t callChainId = INVALID_UINT32;
    // When the stack id is zero, there is no matching call stack
    if (isOfflineSymbolizationMode_ && mMapEventReader.stack_id()) {
        // The same call stack may have different symbolic results due to changes in the symbol table
        if (stackIdToCallChainIdMap_.count(mMapEventReader.stack_id())) {
            callChainId = stackIdToCallChainIdMap_.at(mMapEventReader.stack_id());
        } else {
            TS_LOGE("invalid callChainId, can not find stack id : %u in stackIdToCallChainIdMap_!",
                    mMapEventReader.stack_id());
        }
    } else if (isCallStackCompressedMode_ && mMapEventReader.stack_id()) {
        // when isOfflineSymblolizationMode_ is false && isCallStackCompressedMode is true, the stack id is unique
        callChainId = mMapEventReader.stack_id();
    }
    auto itid = streamFilters_->processFilter_->GetOrCreateThreadWithPid(mMapEventReader.tid(), mMapEventReader.pid());
    if (ipid_ == INVALID_UINT32) {
        ipid_ = streamFilters_->processFilter_->GetOrCreateInternalPid(timeStamp, mMapEventReader.pid());
    }
    // Update the mapping of tid to thread name id.
    if (mMapEventReader.thread_name_id() != 0) {
        UpdateMap(itidToThreadNameId_, itid, mMapEventReader.thread_name_id());
    }
    // Gets the index of the mmap event's label in the data dictionary
    DataIndex subType = INVALID_UINT64;
    auto mMapAddr = mMapEventReader.addr();
    auto mMapSize = mMapEventReader.size();
    if (mMapEventReader.has_type()) {
        subType = traceDataCache_->dataDict_.GetStringIndex(mMapEventReader.type().ToStdString());
        // Establish a mapping of addr and size to the mmap tag index.
        addrToMmapTag_[mMapAddr] = subType; // update addr to MemMapSubType
    }
    auto row = traceDataCache_->GetNativeHookData()->AppendNewNativeHookData(
        callChainId, ipid_, itid, "MmapEvent", subType, timeStamp, 0, 0, mMapAddr, mMapSize);
    if (subType == INVALID_UINT64) {
        UpdateAnonMmapDataDbIndex(mMapAddr, mMapSize, static_cast<uint64_t>(row));
    }
    addrToMmapEventRow_.insert(std::make_pair(mMapAddr, static_cast<uint64_t>(row)));
    // update currentSizeDur.
    if (mMapSize) {
        MaybeUpdateCurrentSizeDur(row, timeStamp, false);
    }
    // Uncompressed call stack
    if (mMapEventReader.has_frame_info()) {
        CompressStackAndFrames(mMapEventReader.frame_info());
    }
}

void NativeHookFilter::ParseMunmapEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView)
{
    ProtoReader::MunmapEvent_Reader mUnmapEventReader(bytesView);
    uint32_t callChainId = INVALID_UINT32;
    // When the stack id is zero, there is no matching call stack
    if (isOfflineSymbolizationMode_ && mUnmapEventReader.stack_id()) {
        // The same call stack may have different symbolic results due to changes in the symbol table
        if (stackIdToCallChainIdMap_.count(mUnmapEventReader.stack_id())) {
            callChainId = stackIdToCallChainIdMap_.at(mUnmapEventReader.stack_id());
        } else {
            TS_LOGE("invalid callChainId, can not find stack id : %u in stackIdToCallChainIdMap_!",
                    mUnmapEventReader.stack_id());
        }
    } else if (isCallStackCompressedMode_ && mUnmapEventReader.stack_id()) {
        // when isOfflineSymblolizationMode_ is false && isCallStackCompressedMode is true, the stack id is unique
        callChainId = mUnmapEventReader.stack_id();
    }
    auto itid =
        streamFilters_->processFilter_->GetOrCreateThreadWithPid(mUnmapEventReader.tid(), mUnmapEventReader.pid());
    if (ipid_ == INVALID_UINT32) {
        ipid_ = streamFilters_->processFilter_->GetOrCreateInternalPid(timeStamp, mUnmapEventReader.pid());
    }
    if (mUnmapEventReader.thread_name_id() != 0) {
        UpdateMap(itidToThreadNameId_, itid, mUnmapEventReader.thread_name_id());
    }
    // Query for MMAP events that match the current data. If there are no matching MMAP events, the current data is not
    // written to the database.
    uint64_t row = INVALID_UINT64;
    auto mUnmapAddr = mUnmapEventReader.addr();
    if (addrToMmapEventRow_.count(mUnmapAddr)) {
        row = addrToMmapEventRow_.at(mUnmapAddr);
    }
    if (row != INVALID_UINT64 && timeStamp > traceDataCache_->GetNativeHookData()->TimeStampData()[row]) {
        addrToMmapEventRow_.erase(mUnmapAddr);
        traceDataCache_->GetNativeHookData()->UpdateEndTimeStampAndDuration(row, timeStamp);
    } else {
        TS_LOGD("func addr:%lu is empty", mUnmapAddr);
        streamFilters_->statFilter_->IncreaseStat(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_DATA_INVALID);
        return;
    }
    auto subType = GetMemMapSubTypeWithAddr(mUnmapAddr);
    row = traceDataCache_->GetNativeHookData()->AppendNewNativeHookData(
        callChainId, ipid_, itid, "MunmapEvent", subType, timeStamp, 0, 0, mUnmapAddr, mUnmapEventReader.size());
    addrToMmapTag_.erase(mUnmapAddr); // earse MemMapSubType with addr
    if (mUnmapEventReader.size() != 0) {
        MaybeUpdateCurrentSizeDur(row, timeStamp, false);
    }
    // Uncompressed call stack
    if (mUnmapEventReader.has_frame_info()) {
        CompressStackAndFrames(mUnmapEventReader.frame_info());
    }
}
void NativeHookFilter::ParseTagEvent(const ProtoReader::BytesView& bytesView)
{
    ProtoReader::MemTagEvent_Reader memTagEventReader(bytesView);
    auto addr = memTagEventReader.addr();
    auto size = memTagEventReader.size();
    auto tagIndex = traceDataCache_->dataDict_.GetStringIndex(memTagEventReader.tag().ToStdString());
    NativeHook* nativeHookPtr = traceDataCache_->GetNativeHookData();
    std::shared_ptr<std::set<uint64_t>> indexSetPtr = anonMmapData_.Find(addr, size); // get anonMmapData dbIndex
    if (indexSetPtr != nullptr) {
        for (auto rowIter = indexSetPtr->begin(); rowIter != indexSetPtr->end(); rowIter++) {
            nativeHookPtr->UpdateMemMapSubType(*rowIter, tagIndex);
        }
        indexSetPtr->clear();            // clear annoMmapData dbIndex
        addrToMmapTag_[addr] = tagIndex; // update addr to MemMapSubType
    }
}
inline uint64_t NativeHookFilter::GetMemMapSubTypeWithAddr(uint64_t addr)
{
    auto iter = addrToMmapTag_.find(addr);
    if (iter != addrToMmapTag_.end()) {
        return iter->second; // subType
    } else {
        return INVALID_UINT64;
    }
}
inline void NativeHookFilter::UpdateAnonMmapDataDbIndex(uint64_t addr, uint32_t size, uint64_t row)
{
    auto indexPtr = anonMmapData_.Find(addr, size);
    if (indexPtr == nullptr) {
        std::shared_ptr<std::set<uint64_t>> rowPtr_ = std::make_shared<std::set<uint64_t>>();
        rowPtr_->insert(row);
        anonMmapData_.Insert(addr, size, std::move(rowPtr_));
    } else {
        indexPtr->insert(row);
    }
}
void NativeHookFilter::FilterNativeHookMainEvent(size_t num)
{
    auto itor = tsToMainEventsMap_.begin();
    for (; itor != tsToMainEventsMap_.end() && num; num--, itor++) {
        auto nativeHookDataReader = itor->second->reader_.get();
        auto timeStamp = itor->first;
        if (nativeHookDataReader->has_alloc_event()) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_NATIVE_HOOK_MALLOC, STAT_EVENT_RECEIVED);
            ParseAllocEvent(timeStamp, nativeHookDataReader->alloc_event());
        } else if (nativeHookDataReader->has_free_event()) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_RECEIVED);
            ParseFreeEvent(timeStamp, nativeHookDataReader->free_event());
        } else if (nativeHookDataReader->has_mmap_event()) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_NATIVE_HOOK_MMAP, STAT_EVENT_RECEIVED);
            ParseMmapEvent(timeStamp, nativeHookDataReader->mmap_event());
        } else if (nativeHookDataReader->has_munmap_event()) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_RECEIVED);
            ParseMunmapEvent(timeStamp, nativeHookDataReader->munmap_event());
        } else if (nativeHookDataReader->has_statistics_event()) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_NATIVE_HOOK_RECORD_STATISTICS, STAT_EVENT_RECEIVED);
            ParseStatisticEvent(timeStamp, nativeHookDataReader->statistics_event());
        } else if (nativeHookDataReader->has_tag_event()) {
            streamFilters_->statFilter_->IncreaseStat(TRACE_NATIVE_HOOK_MEMTAG, STAT_EVENT_RECEIVED);
            ParseTagEvent(nativeHookDataReader->tag_event());
        }
    }
    tsToMainEventsMap_.erase(tsToMainEventsMap_.begin(), itor);
}

void NativeHookFilter::MaybeParseNativeHookMainEvent(uint64_t timeStamp,
                                                     std::unique_ptr<NativeHookMetaData> nativeHookMetaData)
{
    tsToMainEventsMap_.insert(std::make_pair(timeStamp, std::move(nativeHookMetaData)));
    if (tsToMainEventsMap_.size() > MAX_CACHE_SIZE) {
        if (isOfflineSymbolizationMode_) {
            ParseFramesInOfflineSymbolizationMode();
            ReparseStacksWithDifferentMeans();
        }
        FilterNativeHookMainEvent(tsToMainEventsMap_.size() - MAX_CACHE_SIZE);
    }
}

// Returns the address range of memMaps that conflict with start Addr and endAddr, as [start, end).
std::tuple<uint64_t, uint64_t> NativeHookFilter::GetNeedUpdateProcessMapsAddrRange(uint64_t startAddr, uint64_t endAddr)
{
    uint64_t start = INVALID_UINT64;
    uint64_t end = INVALID_UINT64;
    if (startAddr >= endAddr) {
        return std::make_tuple(start, end);
    }
    // Find first item in startAddrToMapsInfoMap_,
    // that startItor->second()->start <= startAddr && startItor->second()->end > startAddr.
    auto startItor = startAddrToMapsInfoMap_.upper_bound(startAddr);
    if (startAddrToMapsInfoMap_.begin() != startItor) {
        startItor--;
        // Follow the rules of front closing and rear opening, [start, end)
        if (startAddr >= startItor->second->end()) {
            startItor++;
        }
    }
    // Forward query for the last item with filePathId == startItor ->filePathId()
    if (startItor != startAddrToMapsInfoMap_.end()) {
        auto startFilePathId = startItor->second->file_path_id();
        while (startAddrToMapsInfoMap_.begin() != startItor) {
            startItor--;
            if (startFilePathId != startItor->second->file_path_id()) {
                startItor++;
                break;
            }
        }
        start = startItor->first;
    }

    // Find first item in startAddrToMapsInfoMap_, that endItor->second()->start > endAddr
    auto endItor = startAddrToMapsInfoMap_.upper_bound(endAddr);
    if (endItor == startAddrToMapsInfoMap_.end()) {
        return std::make_tuple(start, end);
    }
    if (endItor == startAddrToMapsInfoMap_.begin()) {
        start = INVALID_UINT64;
        return std::make_tuple(start, end);
    }
    // Backward query for the last item with filePathId == endItor ->filePathId()
    --endItor;
    auto endFilePathId = endItor->second->file_path_id();
    ++endItor;
    while (endItor != startAddrToMapsInfoMap_.end()) {
        if (endFilePathId != endItor->second->file_path_id()) {
            end = endItor->second->start();
            break;
        }
        endItor++;
    }
    return std::make_tuple(start, end);
}

inline void NativeHookFilter::FillOfflineSymbolizationFrames(
    std::map<uint32_t, std::shared_ptr<std::vector<uint64_t>>>::iterator mapItor)
{
    stackIdToCallChainIdMap_.insert(std::make_pair(mapItor->first, ++callChainId_));
    auto framesInfo = OfflineSymbolization(mapItor->second);
    uint64_t depth = 0;
    uint64_t filePathIndex;
    for (auto itor = framesInfo->rbegin(); itor != framesInfo->rend(); itor++) {
        // Note that the filePathId here is provided for the end side. Not a true TS internal index dictionary.
        auto frameInfo = itor->get();
        if (filePathIdToFileIndex_.count(frameInfo->filePathId_)) {
            filePathIndex = filePathIdToFileIndex_.at(frameInfo->filePathId_);
        } else {
            filePathIndex = INVALID_UINT64;
        }
        std::string vaddr = base::Uint64ToHexText(frameInfo->symVaddr_);

        traceDataCache_->GetNativeHookFrameData()->AppendNewNativeHookFrame(
            callChainId_, depth, frameInfo->ip_, INVALID_UINT64, frameInfo->symbolIndex_, filePathIndex,
            frameInfo->offset_, frameInfo->symbolOffset_, vaddr);
        depth++;
    }
}

void NativeHookFilter::ReparseStacksWithDifferentMeans()
{
    for (auto itor = reparseStackIdToFramesMap_.begin(); itor != reparseStackIdToFramesMap_.end(); itor++) {
        // Items with key equal to stack id should not be retained in stackIdToCallChainIdMap_
        if (stackIdToCallChainIdMap_.count(itor->first)) {
            TS_LOGE("error! The mapping of ambiguous call stack id and callChainId has not been deleted!");
        }
        FillOfflineSymbolizationFrames(itor);
    }
    reparseStackIdToFramesMap_.clear();
}

inline void NativeHookFilter::ReparseStacksWithAddrRange(uint64_t start, uint64_t end)
{
    // Get the list of call stack ids that should be parsed again
    for (auto itor = allStackIdToFramesMap_.begin(); itor != allStackIdToFramesMap_.end(); itor++) {
        auto ips = itor->second;
        for (auto ipsItor = ips->begin(); ipsItor != ips->end(); ipsItor++) {
            if (*ipsItor >= start && *ipsItor < end) {
                // delete the stack ids whitch should be parsed again
                if (stackIdToCallChainIdMap_.count(itor->first)) {
                    stackIdToCallChainIdMap_.erase(itor->first);
                }
                /* update reparseStackIdToFramesMap_. The reparseStackIdToFramesMap_ cannot be parsed immediately.
                Wait until the relevant memmaps and symbolTable updates are completed. After the main event is
                updated and before the main event is about to be parsed, parse reparseStackIdToFramesMap_ first. */
                if (!stackIdToFramesMap_.count(itor->first)) {
                    reparseStackIdToFramesMap_.emplace(std::make_pair(itor->first, itor->second));
                }
                break;
            }
        }
    }
}

// Only called in offline symbolization mode.
void NativeHookFilter::ParseMapsEvent(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData)
{
    segs_.emplace_back(nativeHookMetaData->seg_);
    auto reader = std::make_shared<ProtoReader::MapsInfo_Reader>(nativeHookMetaData->reader_->maps_info());
    // The temporary variable startAddr here is to solve the problem of parsing errors under the window platform
    auto startAddr = reader->start();
    auto endAddr = reader->end();
    uint64_t start = INVALID_UINT64;
    uint64_t end = INVALID_UINT64;
    // Get [start, end) of ips addr range which need to update
    std::tie(start, end) = GetNeedUpdateProcessMapsAddrRange(startAddr, endAddr);
    if (start != INVALID_UINT64 && start != end) { // Conflicting
        /* First parse the updated call stacks, then parse the main events, and finally update Maps or SymbolTable
        Note that when tsToMainEventsMap_.size() > MAX_CACHE_SIZE and main events need to be resolved, this logic
        should also be followed. */
        ParseFramesInOfflineSymbolizationMode();
        // When a main event is updated, the call stack that needs to be parsed again is parsed.
        if (tsToMainEventsMap_.size()) {
            ReparseStacksWithDifferentMeans();
            FilterNativeHookMainEvent(tsToMainEventsMap_.size());
        }

        // Delete IP symbolization results within the conflict range.
        auto ipToFrameInfoItor = ipToFrameInfo_.lower_bound(start);
        while (ipToFrameInfoItor != ipToFrameInfo_.end() && ipToFrameInfoItor->first < end) {
            auto key = ipToFrameInfoItor->first;
            ipToFrameInfoItor++;
            ipToFrameInfo_.erase(key);
        }
        // Delete MapsInfo within the conflict range
        auto startAddrToMapsInfoItor = startAddrToMapsInfoMap_.lower_bound(start);
        while (startAddrToMapsInfoItor != startAddrToMapsInfoMap_.end() && startAddrToMapsInfoItor->first < end) {
            auto key = startAddrToMapsInfoItor->first;
            startAddrToMapsInfoItor++;
            startAddrToMapsInfoMap_.erase(key);
        }
        ReparseStacksWithAddrRange(start, end);
    }
    startAddrToMapsInfoMap_.insert(std::make_pair(startAddr, std::move(reader)));
}
template <class T>
void NativeHookFilter::UpdateSymbolTablePtrAndStValueToSymAddrMap(
    T* firstSymbolAddr,
    const int size,
    std::shared_ptr<ProtoReader::SymbolTable_Reader> reader)
{
    for (auto i = 0; i < size; i++) {
        auto symAddr = firstSymbolAddr + i;
        if ((symAddr->st_info & STT_FUNC) && symAddr->st_value) {
            symbolTablePtrAndStValueToSymAddr_.Insert(reader, symAddr->st_value,
                                                      reinterpret_cast<const uint8_t*>(symAddr));
        }
    }
}
// Only called in offline symbolization mode.
void NativeHookFilter::ParseSymbolTableEvent(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData)
{
    segs_.emplace_back(nativeHookMetaData->seg_);

    auto reader = std::make_shared<ProtoReader::SymbolTable_Reader>(nativeHookMetaData->reader_->symbol_tab());
    auto filePathId = reader->file_path_id();
    if (filePathIdToSymbolTableMap_.count(filePathId)) { // SymbolTable already exists.
        /* First parse the updated call stacks, then parse the main events, and finally update Maps or SymbolTable
        Note that when tsToMainEventsMap_.size() > MAX_CACHE_SIZE and main events need to be resolved, this logic
        should also be followed. */
        ParseFramesInOfflineSymbolizationMode();
        // When a main event is updated, the call stack that needs to be parsed again is parsed.
        if (tsToMainEventsMap_.size()) {
            ReparseStacksWithDifferentMeans();
            FilterNativeHookMainEvent(tsToMainEventsMap_.size());
        }
        // Delete symbolic results with the same filePathId
        for (auto itor = ipToFrameInfo_.begin(); itor != ipToFrameInfo_.end(); itor++) {
            if (itor->second->filePathId_ == filePathId) {
                ipToFrameInfo_.erase(itor->first);
            }
        }
        uint64_t start = INVALID_UINT32;
        uint64_t end = 0;
        for (auto itor = startAddrToMapsInfoMap_.begin(); itor != startAddrToMapsInfoMap_.end(); itor++) {
            if (itor->second->file_path_id() == filePathId) {
                start = std::min(itor->first, start);
                end = std::max(itor->second->end(), end);
            } else if (start != INVALID_UINT32) {
                break;
            }
        }
        ReparseStacksWithAddrRange(start, end);

        filePathIdToSymbolTableMap_.at(filePathId) = reader;
    } else {
        filePathIdToSymbolTableMap_.insert(std::make_pair(filePathId, reader));
    }

    auto symEntrySize = reader->sym_entry_size();
    auto symTable = reader->sym_table();
    auto size = symTable.Size() / symEntrySize;
    if (symEntrySize == ELF32_SYM) {
        UpdateSymbolTablePtrAndStValueToSymAddrMap(reinterpret_cast<const Elf32_Sym*>(symTable.Data()), size, reader);
    } else {
        UpdateSymbolTablePtrAndStValueToSymAddrMap(reinterpret_cast<const Elf64_Sym*>(symTable.Data()), size, reader);
    }
}

void NativeHookFilter::MaybeUpdateCurrentSizeDur(uint64_t row, uint64_t timeStamp, bool isMalloc)
{
    auto& lastAnyEventRaw = isMalloc ? lastMallocEventRaw_ : lastMmapEventRaw_;
    if (lastAnyEventRaw != INVALID_UINT64) {
        traceDataCache_->GetNativeHookData()->UpdateCurrentSizeDur(lastAnyEventRaw, timeStamp);
    }
    lastAnyEventRaw = row;
}
// when symbolization failed, use filePath + vaddr as symbol name
void NativeHookFilter::UpdateSymbolIdsForSymbolizationFailed()
{
    auto size = traceDataCache_->GetNativeHookFrameData()->Size();
    for (size_t i = 0; i < size; ++i) {
        if (traceDataCache_->GetNativeHookFrameData()->SymbolNames()[i] == INVALID_UINT64) {
            auto filePathIndex = traceDataCache_->GetNativeHookFrameData()->FilePaths()[i];
            auto filePathStr = traceDataCache_->dataDict_.GetDataFromDict(filePathIndex);
            auto vaddrStr = traceDataCache_->GetNativeHookFrameData()->Vaddrs()[i];
            traceDataCache_->GetNativeHookFrameData()->UpdateSymbolId(
                i, traceDataCache_->dataDict_.GetStringIndex(filePathStr + "+" + vaddrStr));
        }
    }
}
void NativeHookFilter::ParseFramesInOfflineSymbolizationMode()
{
    for (auto stackIdToFramesItor = stackIdToFramesMap_.begin(); stackIdToFramesItor != stackIdToFramesMap_.end();
         stackIdToFramesItor++) {
        FillOfflineSymbolizationFrames(stackIdToFramesItor);
    }
    // In offline symbolization scenarios, The updated call stack information is saved in stackIdToFramesMap_.
    // After each parsing is completed, it needs to be cleared to avoid repeated parsing.
    stackIdToFramesMap_.clear();
}

void NativeHookFilter::GetNativeHookFrameVaddrs()
{
    vaddrs_.clear();
    auto size = traceDataCache_->GetNativeHookFrameData()->Size();
    // Traverse every piece of native_hook frame data
    for (size_t i = 0; i < size; i++) {
        auto symbolOffset = traceDataCache_->GetNativeHookFrameData()->SymbolOffsets()[i];
        // When the symbol offset is not 0, vaddr=offset+symbol offset
        if (symbolOffset) {
            auto fileOffset = traceDataCache_->GetNativeHookFrameData()->Offsets()[i];
            auto vaddr = base::Uint64ToHexText(fileOffset + symbolOffset);
            vaddrs_.emplace_back(vaddr);
            continue;
        }
        // When the symbol offset is 0, vaddr takes the string after the plus sign in the function name
        auto functionNameIndex = traceDataCache_->GetNativeHookFrameData()->SymbolNames()[i];
        std::string vaddr = "";
        auto itor = functionNameIndexToVaddr_.find(functionNameIndex);
        if (itor == functionNameIndexToVaddr_.end()) {
            auto functionName = traceDataCache_->dataDict_.GetDataFromDict(functionNameIndex);
            auto pos = functionName.rfind("+");
            if (pos != functionName.npos && pos != functionName.length() - 1) {
                vaddr = functionName.substr(++pos);
            }
            // Vaddr keeps "" when lookup failed
            functionNameIndexToVaddr_.emplace(std::make_pair(functionNameIndex, vaddr));
        } else {
            vaddr = itor->second;
        }
        vaddrs_.emplace_back(vaddr);
    }
}
// Called When isCallStackCompressedMode_ is true && isOfflineSymbolizationMode_ is false.
void NativeHookFilter::ParseFramesInCallStackCompressedMode()
{
    for (auto stackIdToFramesItor = stackIdToFramesMap_.begin(); stackIdToFramesItor != stackIdToFramesMap_.end();
         stackIdToFramesItor++) {
        auto frameIds = stackIdToFramesItor->second;
        uint64_t depth = 0;
        for (auto frameIdsItor = frameIds->crbegin(); frameIdsItor != frameIds->crend(); frameIdsItor++) {
            if (!frameIdToFrameBytes_.count(*frameIdsItor)) {
                TS_LOGE("Can not find Frame by frame_map_id!!!");
                continue;
            }
            ProtoReader::Frame_Reader reader(*(frameIdToFrameBytes_.at(*frameIdsItor)));

            if (!reader.has_file_path_id() or !reader.has_symbol_name_id()) {
                TS_LOGE("Data exception, frames should has fil_path_id and symbol_name_id");
                continue;
            }
            if (!filePathIdToFileIndex_.count(reader.file_path_id())) {
                TS_LOGE("Data exception, can not find fil_path_id!!!");
                continue;
            }
            auto& filePathIndex = filePathIdToFileIndex_.at(reader.file_path_id());
            if (!symbolIdToSymbolIndex_.count(reader.symbol_name_id())) {
                TS_LOGE("Data exception, can not find symbol_name_id!!!");
                continue;
            }
            auto& symbolIndex = symbolIdToSymbolIndex_.at(reader.symbol_name_id());
            traceDataCache_->GetNativeHookFrameData()->AppendNewNativeHookFrame(
                stackIdToFramesItor->first, depth, reader.ip(), reader.sp(), symbolIndex, filePathIndex,
                reader.offset(), reader.symbol_offset());
            depth++;
        }
    }
}
// Called When isCallStackCompressedMode_ is false.
void NativeHookFilter::ParseFramesWithOutCallStackCompressedMode()
{
    for (auto itor = callChainIdToStackHashValueMap_.begin(); itor != callChainIdToStackHashValueMap_.end(); itor++) {
        auto callChainId = itor->first;
        if (!stackHashValueToFramesHashMap_.count(itor->second)) {
            continue;
        }
        auto& framesHash = stackHashValueToFramesHashMap_.at(itor->second);
        uint64_t depth = 0;
        for (auto frameHashValueVectorItor = framesHash.crbegin(); frameHashValueVectorItor != framesHash.crend();
             frameHashValueVectorItor++) {
            if (!frameHashToFrameInfoMap_.count(*frameHashValueVectorItor)) {
                TS_LOGE("find matching frameInfo failed!!!!");
                return;
            }
            auto& frameInfo = frameHashToFrameInfoMap_.at(*frameHashValueVectorItor);
            traceDataCache_->GetNativeHookFrameData()->AppendNewNativeHookFrame(
                callChainId, depth, frameInfo->ip_, frameInfo->sp_, frameInfo->symbolIndex_, frameInfo->filePathIndex_,
                frameInfo->offset_, frameInfo->symbolOffset_);
            depth++;
        }
    }
}
void NativeHookFilter::ParseSymbolizedNativeHookFrame()
{
    // isOfflineSymbolizationMode is false, but isCallStackCompressedMode is true.
    if (isCallStackCompressedMode_) {
        ParseFramesInCallStackCompressedMode();
    } else {
        ParseFramesWithOutCallStackCompressedMode();
    }
    GetNativeHookFrameVaddrs();
    traceDataCache_->GetNativeHookFrameData()->UpdateVaddrs(vaddrs_);
    return;
}
void NativeHookFilter::UpdateThreadNameWithNativeHookData() const
{
    if (itidToThreadNameId_.empty() || threadNameIdToThreadNameIndex_.empty()) {
        return;
    }
    for (auto itor = itidToThreadNameId_.begin(); itor != itidToThreadNameId_.end(); ++itor) {
        auto thread = traceDataCache_->GetThreadData(itor->first);
        if (!thread->nameIndex_) {
            auto threadNameMapItor = threadNameIdToThreadNameIndex_.find(itor->second);
            if (threadNameMapItor != threadNameIdToThreadNameIndex_.end()) {
                thread->nameIndex_ = threadNameMapItor->second;
            }
        }
    }
}
void NativeHookFilter::FinishParseNativeHookData()
{
    // In offline symbolization mode Parse all NativeHook main events depends on updated stackIdToCallChainIdMap_ during
    // execute ParseSymbolizedNativeHookFrame or ReparseStacksWithDifferentMeans , So first parse the call stack data
    // and then parse the main event.
    if (isOfflineSymbolizationMode_) {
        ParseFramesInOfflineSymbolizationMode();
        ReparseStacksWithDifferentMeans();
        UpdateSymbolIdsForSymbolizationFailed();
    }
    FilterNativeHookMainEvent(tsToMainEventsMap_.size());
    // In online symbolization mode and callstack is not compressed mode parse stack should after parse main event
    // In online symbolization mode and callstack is compressed mode, there is no need worry about the order
    if (!isOfflineSymbolizationMode_) {
        ParseSymbolizedNativeHookFrame();
    }

    // update last lib id
    GetCallIdToLastLibId();
    if (callIdToLastCallerPathIndex_.size()) {
        traceDataCache_->GetNativeHookData()->UpdateLastCallerPathIndexs(callIdToLastCallerPathIndex_);
    }
    UpdateThreadNameWithNativeHookData();
}
void NativeHookFilter::GetCallIdToLastLibId()
{
    auto size = static_cast<int64_t>(traceDataCache_->GetNativeHookFrameData()->Size());
    uint32_t lastCallChainId = INVALID_UINT32;
    bool foundLast = false;
    for (auto i = size - 1; i > -1; i--) {
        auto callChainId = traceDataCache_->GetNativeHookFrameData()->CallChainIds()[i];
        if (callChainId == lastCallChainId) {
            if (foundLast) {
                continue;
            }
        }
        if (callChainId != lastCallChainId) {
            lastCallChainId = callChainId;
            foundLast = false;
        }
        auto filePathIndex = traceDataCache_->GetNativeHookFrameData()->FilePaths()[i];
        if (!traceDataCache_->GetNativeHookFrameData()->Depths()[i]) {
            callIdToLastCallerPathIndex_.insert(std::make_pair(callChainId, filePathIndex));
            foundLast = true;
            continue;
        }

        auto lower = std::lower_bound(invalidLibPathIndexs_.begin(), invalidLibPathIndexs_.end(), filePathIndex);
        if (lower == invalidLibPathIndexs_.end() || *lower != filePathIndex) { // found
            auto filePath = traceDataCache_->dataDict_.GetDataFromDict(filePathIndex);
            auto ret = filePath.find("libc++_shared.so");
            if (ret == filePath.npos) {
                callIdToLastCallerPathIndex_.insert(std::make_pair(callChainId, filePathIndex));
                foundLast = true;
            }
        }
    }
}
bool NativeHookFilter::GetIpsWitchNeedResymbolization(DataIndex filePathId, std::set<uint64_t>& ips)
{
    bool value = false;
    for (auto itor = ipToFrameInfo_.begin(); itor != ipToFrameInfo_.end(); itor++) {
        if (!itor->second) {
            TS_LOGI("ip :%" PRIu64 " can not symbolization! FrameInfo is nullptr", itor->first);
            continue;
        }
        if (itor->second->filePathId_ == filePathId) {
            ips.insert(itor->first);
            value = true;
        }
    }
    return value;
}

template <class T>
void NativeHookFilter::UpdateFilePathIdAndStValueToSymAddrMap(T* firstSymbolAddr, const int size, uint32_t filePathId)
{
    for (auto i = 0; i < size; i++) {
        auto symAddr = firstSymbolAddr + i;
        if ((symAddr->st_info & STT_FUNC) && (symAddr->st_value)) {
            filePathIdAndStValueToSymAddr_.Insert(filePathId, symAddr->st_value,
                                                  reinterpret_cast<const uint8_t*>(symAddr));
        }
    }
}

bool NativeHookFilter::NativeHookReloadElfSymbolTable(
    std::shared_ptr<std::vector<std::shared_ptr<ElfSymbolTable>>> elfSymbolTables)
{
    auto nativeHookFrame = traceDataCache_->GetNativeHookFrameData();
    auto size = nativeHookFrame->Size();
    auto filePathIndexs = nativeHookFrame->FilePaths();
    auto vaddrs = nativeHookFrame->Vaddrs();
    for (auto elfSymbolTable : *elfSymbolTables) {
        auto filePathIndex = elfSymbolTable->filePathIndex;
        auto symEntrySize = elfSymbolTable->symEntSize;
        auto totalSize = elfSymbolTable->symTable.size() / symEntrySize;
        if (symEntrySize == ELF32_SYM) {
            UpdateFilePathIdAndStValueToSymAddrMap(reinterpret_cast<const Elf32_Sym*>(elfSymbolTable->symTable.data()),
                                                   totalSize, filePathIndex);
        } else {
            UpdateFilePathIdAndStValueToSymAddrMap(reinterpret_cast<const Elf64_Sym*>(elfSymbolTable->symTable.data()),
                                                   totalSize, filePathIndex);
        }
        if (filePathIdToImportSymbolTableMap_.count(filePathIndex)) {
            filePathIdToImportSymbolTableMap_.at(filePathIndex) = elfSymbolTable;
        } else {
            filePathIdToImportSymbolTableMap_.emplace(std::make_pair(filePathIndex, elfSymbolTable));
        }
        for (size_t row = 0; row < size; row++) {
            if (filePathIndexs[row] != filePathIndex) {
                continue;
            }
            auto symVaddr = base::StrToInt<uint32_t>(vaddrs[row], base::INTEGER_RADIX_TYPE_HEX).value();
            auto symbolIndex = OfflineSymbolization(symVaddr, filePathIndex);
            if (symbolIndex != INVALID_UINT64) {
                nativeHookFrame->UpdateSymbolId(row, symbolIndex);
            }
        }
    }
    filePathIdAndStValueToSymAddr_.Clear();
    filePathIdToImportSymbolTableMap_.clear();
    return true;
}
} // namespace TraceStreamer
} // namespace SysTuning
