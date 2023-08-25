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
#include "ebpf_base.h"

namespace SysTuning {
namespace TraceStreamer {
EbpfBase::EbpfBase(TraceDataCache* dataCache, const TraceStreamerFilters* ctx)
    : EventParserBase(dataCache, ctx),
      pidAndIpToSymbolAndFilePathIndex_(SymbolAndFilePathIndex(false)),
      filePathIndexAndStValueToSymAddr_(nullptr),
      pidAndipsToCallId_(INVALID_UINT64)
{
}
EbpfBase::~EbpfBase()
{
    pidAndipsToCallId_.Clear();
    pidAndIpToSymbolAndFilePathIndex_.Clear();
    filePathIndexToPidAndIpMap_.clear();
    filePathIndexAndStValueToSymAddr_.Clear();
}
bool EbpfBase::InitEbpfDataParser(EbpfDataReader* reader)
{
    auto clockId = reader->GetEbpfDataHeader()->header.clock;
    auto itor = ebpfToTSClockType_.find(clockId);
    if (itor == ebpfToTSClockType_.end()) {
        return false;
    }
    clockId_ = ebpfToTSClockType_.at(clockId);
    reader_ = std::move(reader);
    return true;
}

void EbpfBase::ParseCallStackData(const uint64_t* userIpsAddr, uint16_t count, uint32_t pid, uint32_t callId)
{
    uint64_t depth = 0;
    for (auto i = count - 1; i >= 0; i--) {
        if (userIpsAddr[i] > MIN_USER_IP) {
            auto symbolAndFilePathIndex = GetSymbolAndFilePathIndex(pid, userIpsAddr[i]);
            auto ipIndex = ConvertToHexTextIndex(userIpsAddr[i]);
            ipStrIndexToIpMap_.insert(std::make_pair(ipIndex, userIpsAddr[i]));
            traceDataCache_->GetEbpfCallStack()->AppendNewData(
                callId, depth, ipIndex, symbolAndFilePathIndex.symbolIndex, symbolAndFilePathIndex.filePathIndex);
            depth++;
        }
    }
    // Only one successful insertion is required, without considering repeated insertion failures
    callIdToPid_.insert(std::make_pair(callId, pid));
}

SymbolAndFilePathIndex EbpfBase::GetSymbolAndFilePathIndex(uint32_t pid, uint64_t ip)
{
    auto value = pidAndIpToSymbolAndFilePathIndex_.Find(pid, ip);
    if (value.flag) {
        return value;
    }
    return GetSymbolNameIndexFromElfSym(pid, ip);
}

DataIndex EbpfBase::GetSymbolNameIndexFromSymVaddr(const ElfEventFixedHeader* elfHeaderAddr, uint64_t symVaddr)
{
    uint32_t symbolStart = INVALID_UINT32;
    auto startValueToSymAddr = reader_->GetElfAddrAndStartValueToSymAddr().Find(elfHeaderAddr);
    if (!startValueToSymAddr) {
        return INVALID_UINT64;
    }
    auto end = startValueToSymAddr->upper_bound(symVaddr);
    auto symEntLen = elfHeaderAddr->symEntLen;
    auto length = std::distance(startValueToSymAddr->begin(), end);
    if (length > 0) {
        end--;
        if (symEntLen == ELF32_SYM) {
            GetSymbolStartIndex(reinterpret_cast<const Elf32_Sym*>(end->second), symbolStart, symVaddr);
        } else {
            GetSymbolStartIndex(reinterpret_cast<const Elf64_Sym*>(end->second), symbolStart, symVaddr);
        }
    }
    if (symbolStart == INVALID_UINT32) {
        return INVALID_UINT64;
    }
    // Take out the string according to the subscript
    auto strTabAddr = reinterpret_cast<const char*>(elfHeaderAddr + 1);
    if (symbolStart > elfHeaderAddr->strTabLen) {
        TS_LOGE("symbolStart = %u, elfHeaderAddr->strTabLen = %u", symbolStart, elfHeaderAddr->strTabLen);
        return INVALID_UINT64;
    }
    auto mangle = reinterpret_cast<const char*>(strTabAddr) + symbolStart;
    auto demangle = GetDemangleSymbolIndex(mangle);
    auto index = traceDataCache_->GetDataIndex(demangle);
    if (demangle != mangle) {
        free(demangle);
    }
    return index;
}
void EbpfBase::UpdateFilePathIndexToPidAndIpMap(DataIndex filePathIndex, uint32_t pid, uint64_t ip)
{
    auto itor = filePathIndexToPidAndIpMap_.find(filePathIndex);
    if (itor != filePathIndexToPidAndIpMap_.end()) {
        itor->second->insert(std::make_tuple(pid, ip));
    } else {
        auto pidAndIpSet = std::make_shared<std::set<std::tuple<uint32_t, uint64_t>>>();
        pidAndIpSet->insert(std::make_tuple(pid, ip));
        filePathIndexToPidAndIpMap_.insert(std::make_pair(filePathIndex, pidAndIpSet));
    }
}

SymbolAndFilePathIndex EbpfBase::GetSymbolNameIndexFromElfSym(uint32_t pid, uint64_t ip)
{
    SymbolAndFilePathIndex symbolAndFilePathIndex(false);
    // Follow the rules of front closing and rear opening, [start, end)
    if (ip < reader_->maxKernelAddr_ && ip >= reader_->minKernelAddr_) {
        symbolAndFilePathIndex = reader_->GetSymbolNameIndexFromElfSym(ip);
        pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
        UpdateFilePathIndexToPidAndIpMap(symbolAndFilePathIndex.filePathIndex, pid, ip);
        return symbolAndFilePathIndex;
    }

    auto& pidAndStartAddrToMapsAddr = reader_->GetPidAndStartAddrToMapsAddr();
    auto startToMapsAddr = pidAndStartAddrToMapsAddr.Find(pid);
    if (!startToMapsAddr) {
        symbolAndFilePathIndex.flag = true;
        pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
        UpdateFilePathIndexToPidAndIpMap(symbolAndFilePathIndex.filePathIndex, pid, ip);
        return symbolAndFilePathIndex;
    }

    uint64_t vmStart = INVALID_UINT64;
    uint64_t vmOffset = INVALID_UINT64;
    auto end = startToMapsAddr->upper_bound(ip);
    auto length = std::distance(startToMapsAddr->begin(), end);
    if (length > 0) {
        end--;
        // Follow the rules of front closing and rear opening, [start, end)
        if (ip < end->second->end) {
            vmStart = end->first;
            vmOffset = end->second->offset;
            symbolAndFilePathIndex.filePathIndex =
                traceDataCache_->GetDataIndex(reinterpret_cast<const char*>((end->second) + 1));
        }
    }
    symbolAndFilePathIndex.flag = true;
    if (symbolAndFilePathIndex.filePathIndex == INVALID_INT64) {
        pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
        UpdateFilePathIndexToPidAndIpMap(symbolAndFilePathIndex.filePathIndex, pid, ip);
        return symbolAndFilePathIndex;
    }

    auto itor = reader_->GetElfPathIndexToElfAddr().find(symbolAndFilePathIndex.filePathIndex);
    if (itor == reader_->GetElfPathIndexToElfAddr().end()) {
        pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
        UpdateFilePathIndexToPidAndIpMap(symbolAndFilePathIndex.filePathIndex, pid, ip);
        return symbolAndFilePathIndex;
    }
    uint64_t symVaddr = ip - vmStart + vmOffset + itor->second->textVaddr - itor->second->textOffset;
    auto symbolIndex = GetSymbolNameIndexFromSymVaddr(itor->second, symVaddr);
    if (symbolIndex != INVALID_UINT64) {
        symbolAndFilePathIndex.symbolIndex = symbolIndex;
    }
    pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
    UpdateFilePathIndexToPidAndIpMap(symbolAndFilePathIndex.filePathIndex, pid, ip);
    return symbolAndFilePathIndex;
}

DataIndex EbpfBase::ConvertToHexTextIndex(uint64_t number)
{
    if (number == INVALID_UINT64) {
        return number;
    }
    std::string str = "0x" + base::number(number, base::INTEGER_RADIX_TYPE_HEX);
    return traceDataCache_->GetDataIndex(str.c_str());
}
template <class T>
void EbpfBase::UpdateFilePathIndexAndStValueToSymAddrMap(T* firstSymbolAddr, const int size, uint32_t filePathIndex)
{
    for (auto i = 0; i < size; i++) {
        auto symAddr = firstSymbolAddr + i;
        if ((symAddr->st_info & STT_FUNC) && (symAddr->st_value)) {
            filePathIndexAndStValueToSymAddr_.Insert(filePathIndex, symAddr->st_value,
                                                     reinterpret_cast<const uint8_t*>(symAddr));
        }
    }
}
bool EbpfBase::EBPFReloadElfSymbolTable(std::shared_ptr<std::vector<std::shared_ptr<ElfSymbolTable>>> elfSymbolTables)
{
    std::set<std::tuple<uint32_t, uint64_t>> resymbolizationPidAndIps;
    for (auto elfSymbolTable : *elfSymbolTables) {
        auto filePathIndex = elfSymbolTable->filePathIndex;
        if (!filePathIndexToPidAndIpMap_.count(filePathIndex)) {
            TS_LOGI("ebpf do not support resymbolizaiton use %s",
                    traceDataCache_->GetDataFromDict(filePathIndex).c_str());
            continue;
        }

        uint32_t pid = INVALID_UINT32;
        uint64_t ip = INVALID_UINT64;
        auto pidAndIpSet = filePathIndexToPidAndIpMap_.at(filePathIndex);
        for (auto itor = pidAndIpSet->begin(); itor != pidAndIpSet->end(); itor++) {
            std::tie(pid, ip) = *itor;
            // only userspace ips need re symbolization
            if (ip >= reader_->minKernelAddr_ && ip < reader_->maxKernelAddr_) {
                continue;
            }
            // delete symbolization result
            pidAndIpToSymbolAndFilePathIndex_.Erase(pid, ip);
            // update pids and ips which needs resymbolization
            resymbolizationPidAndIps.insert(std::make_tuple(pid, ip));
        }
        // Update Symbolized Auxiliary Data
        auto symEntrySize = elfSymbolTable->symEntSize;
        auto size = elfSymbolTable->symTable.size() / symEntrySize;
        if (symEntrySize == ELF32_SYM) {
            UpdateFilePathIndexAndStValueToSymAddrMap(
                reinterpret_cast<const Elf32_Sym*>(elfSymbolTable->symTable.data()), size, filePathIndex);
        } else {
            UpdateFilePathIndexAndStValueToSymAddrMap(
                reinterpret_cast<const Elf64_Sym*>(elfSymbolTable->symTable.data()), size, filePathIndex);
        }
        if (filePathIndexToImportSymbolTableMap_.count(filePathIndex)) {
            filePathIndexToImportSymbolTableMap_.at(filePathIndex) = elfSymbolTable;
        } else {
            filePathIndexToImportSymbolTableMap_.emplace(std::make_pair(filePathIndex, elfSymbolTable));
        }
    }
    OfflineSymbolization(resymbolizationPidAndIps);
    // Update Resymbolization Results
    auto ebpfCallStack = traceDataCache_->GetEbpfCallStack();
    for (auto i = 0; i < ebpfCallStack->Size(); i++) {
        auto ipStrIndex = ebpfCallStack->Ips()[i];
        if (!ipStrIndexToIpMap_.count(ipStrIndex)) {
            continue;
        }
        auto ip = ipStrIndexToIpMap_.at(ipStrIndex);
        auto callId = ebpfCallStack->CallChainIds()[i];
        if (callId == INVALID_UINT32) {
            continue;
        }
        if (!callIdToPid_.count(callId)) {
            continue;
        }
        auto pid = callIdToPid_.at(callId);
        auto symbolAndFilePathIndex = pidAndIpToSymbolAndFilePathIndex_.Find(pid, ip);
        if (!symbolAndFilePathIndex.flag) {
            continue;
        }
        ebpfCallStack->UpdateSymbolAndFilePathIndex(i, symbolAndFilePathIndex.symbolIndex,
                                                    symbolAndFilePathIndex.filePathIndex);
    }
    return true;
}

template <class T>
void EbpfBase::GetSymbolStartIndex(T* elfSym, uint32_t& symbolStart, uint64_t symVaddr)
{
    if (elfSym->st_value + elfSym->st_size >= symVaddr) {
        symbolStart = elfSym->st_name;
    }
}
void EbpfBase::OfflineSymbolization(std::set<std::tuple<uint32_t, uint64_t>>& pidAndIps)
{
    uint32_t pid = INVALID_UINT32;
    uint64_t ip = INVALID_UINT64;
    for (auto itor = pidAndIps.begin(); itor != pidAndIps.end(); itor++) {
        SymbolAndFilePathIndex symbolAndFilePathIndex(false);
        std::tie(pid, ip) = *itor;
        auto startToMapsAddr = reader_->GetPidAndStartAddrToMapsAddr().Find(pid);
        if (!startToMapsAddr) {
            continue;
        }
        uint64_t vmStart = INVALID_UINT64;
        uint64_t vmOffset = INVALID_UINT64;
        auto end = startToMapsAddr->upper_bound(ip);
        auto length = std::distance(startToMapsAddr->begin(), end);
        DataIndex filePathIndex = INVALID_DATAINDEX;
        if (length > 0) {
            end--;
            // Follow the rules of front closing and rear opening, [start, end)
            if (ip < end->second->end) {
                vmStart = end->first;
                vmOffset = end->second->offset;
                filePathIndex = traceDataCache_->GetDataIndex(reinterpret_cast<const char*>((end->second) + 1));
            }
        }
        symbolAndFilePathIndex.flag = true;
        symbolAndFilePathIndex.filePathIndex = filePathIndex;
        if (!filePathIndexToImportSymbolTableMap_.count(filePathIndex)) {
            pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
            TS_LOGD("can not find matching symbol table!");
            continue;
        }
        auto& symbolTable = filePathIndexToImportSymbolTableMap_.at(filePathIndex);
        // Calculate virtual address
        uint64_t symVaddr = ip - vmStart + vmOffset + symbolTable->textVaddr - symbolTable->textOffset;
        // pase sym_table to Elf32_Sym or Elf64_Sym array decided by sym_entry_size.
        auto symEntLen = symbolTable->symEntSize;
        auto startValueToSymAddrMap = filePathIndexAndStValueToSymAddr_.Find(filePathIndex);
        if (!startValueToSymAddrMap) {
            // find matching SymbolTable failed, but symVaddr is availiable
            pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
            // find symbolTable failed!!!
            TS_LOGD("find symbolTalbe failed!!!");
            continue;
        }
        // Traverse array, st_value <= symVaddr and symVaddr <= st_value + st_size.  then you can get st_name
        auto endItor = startValueToSymAddrMap->upper_bound(symVaddr);
        length = std::distance(startValueToSymAddrMap->begin(), endItor);
        uint32_t symbolStart = INVALID_UINT32;
        if (length > 0) {
            endItor--;
            if (symEntLen == ELF32_SYM) {
                GetSymbolStartIndex(reinterpret_cast<const Elf32_Sym*>(endItor->second), symbolStart, symVaddr);
            } else {
                GetSymbolStartIndex(reinterpret_cast<const Elf64_Sym*>(endItor->second), symbolStart, symVaddr);
            }
        }
        if (symbolStart == INVALID_UINT32 || symbolStart >= symbolTable->strTable.size()) {
            pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
            // find symbolStart failed, but some data is availiable.
            TS_LOGD("symbolStart is : %lu invaliable!!!", symbolStart);
            continue;
        }
        auto mangle = symbolTable->strTable.c_str() + symbolStart;
        auto demangle = GetDemangleSymbolIndex(mangle);
        symbolAndFilePathIndex.symbolIndex = traceDataCache_->GetDataIndex(demangle);
        if (demangle != mangle) {
            free(demangle);
        }
        pidAndIpToSymbolAndFilePathIndex_.Insert(pid, ip, symbolAndFilePathIndex);
    }
    filePathIndexAndStValueToSymAddr_.Clear();
    filePathIndexToImportSymbolTableMap_.clear();
}
} // namespace TraceStreamer
} // namespace SysTuning
