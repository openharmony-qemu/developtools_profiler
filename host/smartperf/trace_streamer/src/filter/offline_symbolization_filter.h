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

#ifndef OFFLINE_SYMBOLIZATION_FILTER_H
#define OFFLINE_SYMBOLIZATION_FILTER_H
#include <elf.h>
#include <unordered_map>
#include "native_hook_result.pbreader.h"
#include "process_filter.h"
#include "proto_reader.h"
#include "string_help.h"
#include "ts_common.h"
namespace SysTuning {
namespace TraceStreamer {
class FrameInfo {
public:
    FrameInfo()
    {
        filePathId_ = INVALID_UINT32;
        ip_ = INVALID_UINT64;
        symbolIndex_ = INVALID_UINT64;
        offset_ = INVALID_UINT64;
        symbolOffset_ = INVALID_UINT64;
        symVaddr_ = INVALID_UINT64;
    }
    uint32_t filePathId_;
    uint64_t ip_;
    uint64_t symbolIndex_;
    uint64_t offset_;
    uint64_t symbolOffset_;
    uint64_t symVaddr_;
};
struct NativeHookMetaData {
    NativeHookMetaData(const std::shared_ptr<const std::string>& seg,
                       std::unique_ptr<ProtoReader::NativeHookData_Reader> reader)
        : seg_(seg), reader_(std::move(reader))
    {
    }
    std::shared_ptr<const std::string> seg_;
    std::unique_ptr<ProtoReader::NativeHookData_Reader> reader_;
};
class OfflineSymbolizationFilter : public FilterBase {
public:
    OfflineSymbolizationFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    ~OfflineSymbolizationFilter() = default;
    std::shared_ptr<FrameInfo> OfflineSymbolization(uint64_t ip);
    std::shared_ptr<std::vector<std::shared_ptr<FrameInfo>>> OfflineSymbolization(
        const std::shared_ptr<std::vector<uint64_t>> ips);
    DataIndex OfflineSymbolization(uint64_t symVaddr, DataIndex filePathIndex);

protected:
    enum SYSTEM_ENTRY_VALUE { ELF32_SYM = 16, ELF64_SYM = 24 };
    std::map<uint64_t, std::shared_ptr<ProtoReader::MapsInfo_Reader>> startAddrToMapsInfoMap_ = {};
    std::unordered_map<uint32_t, std::shared_ptr<ProtoReader::SymbolTable_Reader>> filePathIdToSymbolTableMap_ = {};
    std::unordered_map<uint32_t, std::shared_ptr<ElfSymbolTable>> filePathIdToImportSymbolTableMap_ = {};
    DoubleMap<uint32_t, uint64_t, const uint8_t*> filePathIdAndStValueToSymAddr_;
    DoubleMap<std::shared_ptr<ProtoReader::SymbolTable_Reader>, uint64_t, const uint8_t*>
        symbolTablePtrAndStValueToSymAddr_;
    std::map<uint64_t, std::shared_ptr<FrameInfo>> ipToFrameInfo_ = {};
    std::vector<std::shared_ptr<const std::string>> segs_ = {};

private:
    template <class T>
    static void GetSymbolStartMaybeUpdateFrameInfo(T* elfSym,
                                                   uint32_t& symbolStart,
                                                   uint64_t symVaddr,
                                                   uint64_t ip,
                                                   FrameInfo* frameInfo);
    bool FillFrameInfo(const std::shared_ptr<FrameInfo>& frameInfo, uint64_t ip, uint64_t& vmStart, uint64_t& vmOffset);
};

} // namespace TraceStreamer
} // namespace SysTuning
#endif
