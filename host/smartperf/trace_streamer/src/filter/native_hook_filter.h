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
#ifndef NATIVE_HOOK_FILTER_H
#define NATIVE_HOOK_FILTER_H
#include <set>
#include "numerical_to_string.h"
#include "offline_symbolization_filter.h"
#include "stat_filter.h"
#include "string_help.h"

namespace SysTuning {
namespace TraceStreamer {
class NativeHookFrameInfo {
public:
    NativeHookFrameInfo()
        : ip_(INVALID_UINT64),
          sp_(INVALID_UINT64),
          symbolIndex_(INVALID_UINT64),
          filePathIndex_(INVALID_UINT64),
          offset_(INVALID_UINT64),
          symbolOffset_(INVALID_UINT64)
    {
    }
    NativeHookFrameInfo(uint64_t ip,
                        uint64_t sp,
                        uint64_t symbolIndex,
                        uint64_t filePathIndex,
                        uint64_t offset,
                        uint64_t symbolOffset)
        : ip_(ip),
          sp_(sp),
          symbolIndex_(symbolIndex),
          filePathIndex_(filePathIndex),
          offset_(offset),
          symbolOffset_(symbolOffset)
    {
    }
    ~NativeHookFrameInfo() {}
    uint64_t ip_;
    uint64_t sp_;
    uint64_t symbolIndex_;
    uint64_t filePathIndex_;
    uint64_t offset_;
    uint64_t symbolOffset_;
};

class NativeHookFilter : public OfflineSymbolizationFilter {
public:
    NativeHookFilter(TraceDataCache*, const TraceStreamerFilters*);
    NativeHookFilter(const NativeHookFilter&) = delete;
    NativeHookFilter& operator=(const NativeHookFilter&) = delete;
    ~NativeHookFilter() = default;

public:
    void MaybeParseNativeHookMainEvent(uint64_t timeStamp, std::unique_ptr<NativeHookMetaData> nativeHookMetaData);
    void ParseConfigInfo(ProtoReader::BytesView& protoData);
    void AppendStackMaps(uint32_t stackid, std::vector<uint64_t>& frames);
    void AppendFrameMaps(uint32_t id, const ProtoReader::BytesView& bytesView);
    void AppendFilePathMaps(uint32_t id, uint64_t fileIndex);
    void AppendSymbolMap(uint32_t id, uint64_t symbolIndex);
    void AppendThreadNameMap(uint32_t id, uint64_t threadNameIndex);
    void ParseMapsEvent(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData);
    void ParseSymbolTableEvent(std::unique_ptr<NativeHookMetaData>& nativeHookMetaData);
    void FinishParseNativeHookData();
    bool NativeHookReloadElfSymbolTable(std::shared_ptr<std::vector<std::shared_ptr<ElfSymbolTable>>> elfSymbolTables);
    bool SupportImportSymbolTable()
    {
        return traceDataCache_->GetNativeHookFrameData()->Size();
    }

private:
    void FilterNativeHookMainEvent(size_t num);
    void ParseStatisticEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView);
    template <class T1, class T2>
    void UpdateMap(std::unordered_map<T1, T2>& sourceMap, T1 key, T2 value);
    void ParseAllocEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView);
    void ParseFreeEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView);
    void ParseMmapEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView);
    void ParseMunmapEvent(uint64_t timeStamp, const ProtoReader::BytesView& bytesView);
    void ParseTagEvent(const ProtoReader::BytesView& bytesView);
    void MaybeUpdateCurrentSizeDur(uint64_t row, uint64_t timeStamp, bool isMalloc);
    void UpdateThreadNameWithNativeHookData() const;
    void GetCallIdToLastLibId();
    void GetNativeHookFrameVaddrs();
    void UpdateSymbolIdsForSymbolizationFailed();
    void ParseFramesInOfflineSymbolizationMode();
    void ParseFramesInCallStackCompressedMode();
    void ParseFramesWithOutCallStackCompressedMode();
    void ParseSymbolizedNativeHookFrame();
    bool GetIpsWitchNeedResymbolization(DataIndex filePathId, std::set<uint64_t>& ips);
    template <class T>
    void UpdateSymbolTablePtrAndStValueToSymAddrMap(T* firstSymbolAddr,
                                                    const int size,
                                                    std::shared_ptr<ProtoReader::SymbolTable_Reader> reader);
    void FillOfflineSymbolizationFrames(std::map<uint32_t, std::shared_ptr<std::vector<uint64_t>>>::iterator itor);
    void ReparseStacksWithAddrRange(uint64_t start, uint64_t end);
    void ReparseStacksWithDifferentMeans();
    void CompressStackAndFrames(ProtoReader::RepeatedDataAreaIterator<ProtoReader::BytesView> frames);
    std::tuple<uint64_t, uint64_t> GetNeedUpdateProcessMapsAddrRange(uint64_t startAddr, uint64_t endAddr);
    std::unique_ptr<NativeHookFrameInfo> ParseFrame(const ProtoReader::DataArea& frame);
    template <class T>
    void UpdateFilePathIdAndStValueToSymAddrMap(T* firstSymbolAddr, const int size, uint32_t filePathId);
    uint64_t GetMemMapSubTypeWithAddr(uint64_t addr);
    void UpdateAnonMmapDataDbIndex(uint64_t addr, uint32_t size, uint64_t row);

private:
    std::multimap<uint64_t, std::unique_ptr<NativeHookMetaData>> tsToMainEventsMap_ = {};
    std::map<uint32_t, std::shared_ptr<std::vector<uint64_t>>> reparseStackIdToFramesMap_ = {};
    std::map<uint32_t, std::shared_ptr<std::vector<uint64_t>>> allStackIdToFramesMap_ = {};
    std::map<uint32_t, std::shared_ptr<std::vector<uint64_t>>> stackIdToFramesMap_ = {};
    std::map<uint32_t, uint64_t> callChainIdToStackHashValueMap_ = {};
    std::unordered_map<uint32_t, std::shared_ptr<const ProtoReader::BytesView>> frameIdToFrameBytes_ = {};
    std::unordered_map<uint64_t, std::vector<uint64_t>> stackHashValueToFramesHashMap_ = {};
    std::unordered_map<uint64_t, std::unique_ptr<NativeHookFrameInfo>> frameHashToFrameInfoMap_ = {};
    std::unordered_map<uint32_t, uint64_t> threadNameIdToThreadNameIndex_ = {};
    std::unordered_map<uint32_t, uint64_t> callIdToLastCallerPathIndex_ = {};
    std::unordered_map<uint64_t, std::string> functionNameIndexToVaddr_ = {};
    std::unordered_map<uint32_t, uint64_t> symbolIdToSymbolIndex_ = {};
    std::unordered_map<uint64_t, uint32_t> stackHashValueToCallChainIdMap_ = {};
    std::unordered_map<uint32_t, uint32_t> itidToThreadNameId_ = {};
    std::unordered_map<uint32_t, uint64_t> filePathIdToFileIndex_ = {};
    std::unordered_map<uint64_t, uint32_t> fileIndexToFilePathId_ = {};
    std::unordered_map<uint32_t, uint32_t> stackIdToCallChainIdMap_ = {};
    std::unordered_map<uint64_t, uint64_t> addrToAllocEventRow_;
    std::unordered_map<uint64_t, uint64_t> addrToMmapEventRow_;
    std::set<DataIndex> invalidLibPathIndexs_ = {};
    std::deque<std::string> vaddrs_ = {};
    // munmap update anonymous or named memory tag always use the last addrToMmapTag_ value
    std::unordered_map<uint64_t, uint64_t> addrToMmapTag_ = {};
    // first key is addr, second key is size, value is set<row> in db
    // mmap update anonymous memory tag always use the anonMmapData_ value
    DoubleMap<uint64_t, uint32_t, std::shared_ptr<std::set<uint64_t>>> anonMmapData_;
    std::hash<std::string_view> hashFun_;
    uint64_t lastMallocEventRaw_ = INVALID_UINT64;
    uint64_t lastMmapEventRaw_ = INVALID_UINT64;
    bool isOfflineSymbolizationMode_ = false;
    bool isCallStackCompressedMode_ = false;
    bool isStringCompressedMode_ = false;
    bool isStatisticMode_ = false;
    const size_t MAX_CACHE_SIZE = 200000;
    uint32_t ipid_ = INVALID_UINT32;
    uint32_t callChainId_ = 0;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // NATIVE_HOOK_FILTER_H
