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
#ifndef EBPF_BASE_H
#define EBPF_BASE_H
#include <algorithm>
#include <set>
#include <string>
#include "double_map.h"
#include "ebpf_data_reader.h"
#include "ebpf_stdtype.h"
#include "event_parser_base.h"
#include "string_help.h"
#include "string_to_numerical.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
using namespace SysTuning::base;
using namespace SysTuning::EbpfStdtype;
class EbpfBase : virtual public EventParserBase {
public:
    EbpfBase(TraceDataCache* dataCache, const TraceStreamerFilters* ctx);
    ~EbpfBase();
    bool InitEbpfDataParser(EbpfDataReader* reader);
    bool EBPFReloadElfSymbolTable(std::shared_ptr<std::vector<std::shared_ptr<ElfSymbolTable>>> elfSymbolTables);

protected:
    void ParseCallStackData(const uint64_t* userIpsAddr, uint16_t count, uint32_t pid, uint32_t callId);
    DataIndex GetSymbolNameIndexFromSymVaddr(const ElfEventFixedHeader* elfHeaderAddr, uint64_t symVaddr);
    SymbolAndFilePathIndex GetSymbolAndFilePathIndex(uint32_t pid, uint64_t ip);
    SymbolAndFilePathIndex GetSymbolNameIndexFromElfSym(uint32_t pid, uint64_t ip);
    void UpdateFilePathIndexToPidAndIpMap(DataIndex filePathIndex, uint32_t pid, uint64_t ip);
    DataIndex ConvertToHexTextIndex(uint64_t number);
    template <class T>
    void UpdateFilePathIndexAndStValueToSymAddrMap(T* firstSymbolAddr, const int size, uint32_t filePathIndex);
    template <class T>
    void GetSymbolStartIndex(T* elfSym, uint32_t& symbolStart, uint64_t symVaddr);
    void OfflineSymbolization(std::set<std::tuple<uint32_t, uint64_t>>& pidAndIps);
    ClockId clockId_ = INVALID_UINT32;
    std::hash<std::string_view> hashFun_;
    EbpfDataReader* reader_ = nullptr;
    DoubleMap<uint32_t, uint64_t, SymbolAndFilePathIndex> pidAndIpToSymbolAndFilePathIndex_;
    std::map<DataIndex, std::shared_ptr<std::set<std::tuple<uint32_t, uint64_t>>>> filePathIndexToPidAndIpMap_ = {};
    std::map<DataIndex, uint64_t> ipStrIndexToIpMap_ = {};
    std::map<uint32_t, uint32_t> callIdToPid_ = {};
    DoubleMap<uint32_t, uint64_t, const uint8_t*> filePathIndexAndStValueToSymAddr_;
    std::map<DataIndex, std::shared_ptr<ElfSymbolTable>> filePathIndexToImportSymbolTableMap_ = {};
    DoubleMap<uint32_t, uint64_t, uint64_t> pidAndipsToCallId_;
    uint64_t callChainId_ = 0;
};
} // namespace TraceStreamer
} // namespace SysTuning
#endif // EBPF_BASE_H
