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

#include "offline_symbolization_filter.h"
#include <inttypes.h>

namespace SysTuning {
namespace TraceStreamer {
OfflineSymbolizationFilter::OfflineSymbolizationFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : FilterBase(dataCache, filter),
      filePathIdAndStValueToSymAddr_(nullptr),
      symbolTablePtrAndStValueToSymAddr_(nullptr),
      ipidToStartAddrToMapsInfoMap_(nullptr),
      ipidToIpToFrameInfo_(nullptr),
      ipidTofilePathIdToSymbolTableMap_(nullptr)
{
}
std::shared_ptr<std::vector<std::shared_ptr<FrameInfo>>> OfflineSymbolizationFilter::OfflineSymbolization(
    const std::shared_ptr<std::vector<uint64_t>> ips)
{
    auto ipid = ips->back();
    auto result = std::make_shared<std::vector<std::shared_ptr<FrameInfo>>>();
    for (auto itor = ips->begin(); (itor + 1) != ips->end(); itor++) {
        auto frameInfo = OfflineSymbolizationByIp(ipid, *itor);
        // If the IP in the middle of the call stack cannot be symbolized, the remaining IP is discarded
        if (!frameInfo) {
            break;
        }
        result->emplace_back(frameInfo);
    }
    return result;
}
template <class T>
void OfflineSymbolizationFilter::GetSymbolStartMaybeUpdateFrameInfo(T* elfSym,
                                                                    uint32_t& symbolStart,
                                                                    uint64_t symVaddr,
                                                                    uint64_t ip,
                                                                    FrameInfo* frameInfo)
{
    if (elfSym->st_value + elfSym->st_size >= symVaddr) {
        symbolStart = elfSym->st_name;
        if (frameInfo) {
            frameInfo->offset_ = elfSym->st_value != 0 ? elfSym->st_value : ip;
            frameInfo->symbolOffset_ = symVaddr - elfSym->st_value;
        }
    }
}

bool OfflineSymbolizationFilter::FillFrameInfo(const std::shared_ptr<FrameInfo>& frameInfo,
                                               uint64_t ip,
                                               uint64_t& vmStart,
                                               uint64_t& vmOffset,
                                               uint64_t ipid)
{
    frameInfo->ip_ = ip;
    auto startAddrToMapsInfoItor = ipidToStartAddrToMapsInfoMap_.Find(ipid);
    TS_CHECK_TRUE(startAddrToMapsInfoItor != nullptr, false,
                  "ipidToStartAddrToMapsInfoMap_ can't find the ipid(%" PRIu64 ")", ipid);
    auto endItor = startAddrToMapsInfoItor->upper_bound(ip);
    int64_t length = std::distance(startAddrToMapsInfoItor->begin(), endItor);
    if (length > 0) {
        endItor--;
        // Follow the rules of front closing and rear opening, [start, end)
        if (ip < endItor->second->end()) {
            vmStart = endItor->second->start();
            vmOffset = endItor->second->offset();
            frameInfo->filePathId_ = endItor->second->file_path_id();
        }
    }
    if (frameInfo->filePathId_ == INVALID_UINT32) {
        // find matching MapsInfo failed!!!
        TS_LOGI("find matching Maps Info failed, ip = %" PRIu64 ", length=%" PRId64 "", ip, length);
        return false;
    }
    return true;
}

std::shared_ptr<FrameInfo> OfflineSymbolizationFilter::OfflineSymbolizationByIp(uint64_t ipid, uint64_t ip)
{
    if (isSingleProcData_) {
        ipid = SINGLE_PROC_IPID;
    }
    auto frameInfoPtr = ipidToIpToFrameInfo_.Find(ipid, ip);
    if (frameInfoPtr != nullptr) {
        return frameInfoPtr;
    }
    uint64_t vmStart = INVALID_UINT64;
    uint64_t vmOffset = INVALID_UINT64;
    // start symbolization
    std::shared_ptr<FrameInfo> frameInfo = std::make_shared<FrameInfo>();
    if (!FillFrameInfo(frameInfo, ip, vmStart, vmOffset, ipid)) {
        return nullptr;
    }
    // find SymbolTable by filePathId
    auto symbolTable = ipidTofilePathIdToSymbolTableMap_.Find(ipid, frameInfo->filePathId_);
    if (symbolTable == nullptr) {
        // find matching SymbolTable failed, but filePathId is availiable
        ipidToIpToFrameInfo_.Insert(ipid, ip, frameInfo);
        TS_LOGD("find matching filePathId failed, ipid = %" PRIu64 ", ip = %lu, filePathId = %u", ipid, ip,
                frameInfo->filePathId_);
        return frameInfo;
    }
    // calculate symVaddr = ip - vmStart + vmOffset + phdrVaddr - phdrOffset
    uint64_t symVaddr =
        ip - vmStart + vmOffset + symbolTable->text_exec_vaddr() - symbolTable->text_exec_vaddr_file_offset();
    frameInfo->symVaddr_ = symVaddr;

    // pase sym_table to Elf32_Sym or Elf64_Sym array decided by sym_entry_size.
    auto symEntLen = symbolTable->sym_entry_size();
    auto startValueToSymAddrMap = symbolTablePtrAndStValueToSymAddr_.Find(symbolTable);
    if (!startValueToSymAddrMap) {
        // find matching SymbolTable failed, but symVaddr is availiable
        ipidToIpToFrameInfo_.Insert(ipid, ip, frameInfo);
        // find symbolTable failed!!!
        TS_LOGD("find symbolTalbe failed!!!");
        return frameInfo;
    }
    // Traverse array, st_value <= symVaddr and symVaddr <= st_value + st_size.  then you can get st_name
    auto end = startValueToSymAddrMap->upper_bound(symVaddr);
    auto length = std::distance(startValueToSymAddrMap->begin(), end);
    uint32_t symbolStart = INVALID_UINT32;
    if (length > 0) {
        end--;
        if (symEntLen == ELF32_SYM) {
            GetSymbolStartMaybeUpdateFrameInfo(reinterpret_cast<const Elf32_Sym*>(end->second), symbolStart, symVaddr,
                                               ip, frameInfo.get());
        } else {
            GetSymbolStartMaybeUpdateFrameInfo(reinterpret_cast<const Elf64_Sym*>(end->second), symbolStart, symVaddr,
                                               ip, frameInfo.get());
        }
    }

    if (symbolStart == INVALID_UINT32 || symbolStart >= symbolTable->str_table().Size()) {
        // find symbolStart failed, but some data is availiable.
        frameInfo->offset_ = ip;
        frameInfo->symbolOffset_ = 0;
        ipidToIpToFrameInfo_.Insert(ipid, ip, frameInfo);
        TS_LOGD("symbolStart is %u invaliable!!!", symbolStart);
        return frameInfo;
    }

    auto mangle = reinterpret_cast<const char*>(symbolTable->str_table().Data() + symbolStart);
    auto demangle = GetDemangleSymbolIndex(mangle);
    frameInfo->symbolIndex_ = traceDataCache_->GetDataIndex(demangle);
    if (demangle != mangle) {
        free(demangle);
    }
    ipidToIpToFrameInfo_.Insert(ipid, ip, frameInfo);
    return frameInfo;
}
DataIndex OfflineSymbolizationFilter::OfflineSymbolizationByVaddr(uint64_t symVaddr, DataIndex filePathIndex)
{
    auto& symbolTable = filePathIdToImportSymbolTableMap_.at(filePathIndex);
    // pase sym_table to Elf32_Sym or Elf64_Sym array decided by sym_entry_size.
    auto symEntLen = symbolTable->symEntSize;
    auto startValueToSymAddrMap = filePathIdAndStValueToSymAddr_.Find(filePathIndex);
    if (!startValueToSymAddrMap) {
        return INVALID_DATAINDEX;
    }
    // Traverse array, st_value <= symVaddr and symVaddr <= st_value + st_size.  then you can get st_name
    auto end = startValueToSymAddrMap->upper_bound(symVaddr);
    auto length = std::distance(startValueToSymAddrMap->begin(), end);
    uint32_t symbolStart = INVALID_UINT32;
    if (length > 0) {
        end--;
        if (symEntLen == ELF32_SYM) {
            GetSymbolStartMaybeUpdateFrameInfo(reinterpret_cast<const Elf32_Sym*>(end->second), symbolStart, symVaddr,
                                               0, nullptr);
        } else {
            GetSymbolStartMaybeUpdateFrameInfo(reinterpret_cast<const Elf64_Sym*>(end->second), symbolStart, symVaddr,
                                               0, nullptr);
        }
    }
    if (symbolStart == INVALID_UINT32 || symbolStart >= symbolTable->strTable.size()) {
        TS_LOGD("symbolStart is : %u invaliable!!!", symbolStart);
        return INVALID_DATAINDEX;
    }
    auto mangle = symbolTable->strTable.c_str() + symbolStart;
    auto demangle = GetDemangleSymbolIndex(mangle);
    auto index = traceDataCache_->GetDataIndex(demangle);
    if (demangle != mangle) {
        free(demangle);
    }
    return index;
}
} // namespace TraceStreamer
} // namespace SysTuning
