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

#include "ebpf_data_reader.h"
#include "file_system_data_parser.h"
#include "string_help.h"
#include <cinttypes>

namespace SysTuning {
namespace TraceStreamer {
using namespace SysTuning::base;
using namespace SysTuning::EbpfStdtype;
EbpfDataReader::EbpfDataReader(TraceDataCache* dataCache, const TraceStreamerFilters* filter)
    : EventParserBase(dataCache, filter),
      ebpfDataHeader_(nullptr),
      pidAndStartAddrToMapsAddr_(nullptr),
      elfAddrAndStValueToSymAddr_(nullptr),
      tracerEventToStrIndex_(INVALID_UINT64),
      kernelFilePath_(traceDataCache_->GetDataIndex("/proc/kallsyms"))
{
}
bool EbpfDataReader::InitEbpfData(const std::deque<uint8_t>& dequeBuffer, uint64_t size)
{
    buffer_ = std::make_unique<uint8_t[]>(size);
    std::copy(dequeBuffer.begin(), dequeBuffer.begin() + size, buffer_.get());

    startAddr_ = buffer_.get();
    bufferSize_ = size;
    unresolvedLen_ = size;
    if (!InitEbpfHeader() || !ReadEbpfData()) {
        return false;
    }
    return true;
}

bool EbpfDataReader::InitEbpfHeader()
{
    if (bufferSize_ < EbpfDataHeader::EBPF_DATA_HEADER_SIZE) {
        TS_LOGE("buffer size less than ebpf data header!!!, bufferSize_ = %" PRIu64 " ", bufferSize_);
        return false;
    }
    ebpfDataHeader_ = reinterpret_cast<EbpfDataHeader*>(startAddr_);

    if (ebpfDataHeader_->header.magic != EbpfDataHeader::HEADER_MAGIC) {
        TS_LOGE("Get EBPF file header failed! magic = %" PRIx64 "", ebpfDataHeader_->header.magic);
        return false;
    }
    if (ebpfDataHeader_->header.headSize != EbpfDataHeader::EBPF_DATA_HEADER_SIZE) {
        TS_LOGE("Get ebpf file header failed! headSize = %u", ebpfDataHeader_->header.headSize);
        return false;
    }
    TS_LOGI("EBPF data header : magic = %" PRIu64 ", headSize = %u, clock = %u, cmdline = %s",
            ebpfDataHeader_->header.magic, ebpfDataHeader_->header.headSize, ebpfDataHeader_->header.clock,
            ebpfDataHeader_->cmdline);
    startAddr_ += EbpfDataHeader::EBPF_DATA_HEADER_SIZE;
    unresolvedLen_ -= EbpfDataHeader::EBPF_DATA_HEADER_SIZE;
    return true;
}

bool EbpfDataReader::ReadEbpfData()
{
    while (unresolvedLen_ > EBPF_TITLE_SIZE) {
        EbpfTypeAndLength* dataTitle = reinterpret_cast<EbpfTypeAndLength*>(startAddr_);
        startAddr_ += EBPF_TITLE_SIZE;
        unresolvedLen_ -= EBPF_TITLE_SIZE;
        if (dataTitle->length > unresolvedLen_) {
            TS_LOGE("Get EBPF data Title failed!");
            TS_LOGE("type = %x, length = %x", dataTitle->type, dataTitle->length);
            return false;
        }
        if (dataTitle->length == 0) {
            continue;
        }

        bool ret = true;
        switch (dataTitle->type) {
            case ITEM_EVENT_MAPS: {
                ret = ReadItemEventMaps(startAddr_, dataTitle->length);
                break;
            }
            case ITEM_SYMBOL_INFO: {
                ret = ReadItemSymbolInfo(startAddr_, dataTitle->length);
                break;
            }
            case ITEM_EVENT_FS: {
                ret = ReadItemEventFs(startAddr_, dataTitle->length);
                break;
            }
            case ITEM_EVENT_VM: {
                ret = ReadItemEventPagedMemory(startAddr_, dataTitle->length);
                break;
            }
            case ITEM_EVENT_BIO: {
                ret = ReadItemEventBIO(startAddr_, dataTitle->length);
                break;
            }
            case ITEM_EVENT_STR: {
                ret = ReadItemEventStr(startAddr_, dataTitle->length);
                break;
            }
            case ITEM_EVENT_KENEL_SYMBOL_INFO: {
                ret = ReaItemKernelSymbolInfo(startAddr_, dataTitle->length);
                break;
            }
            default:
                TS_LOGI("Do not support EBPF type: %d, length: %d", dataTitle->type, dataTitle->length);
        }
        if (!ret) {
            return false;
        }
        startAddr_ += dataTitle->length;
        unresolvedLen_ -= dataTitle->length;
    }
    return true;
}

bool EbpfDataReader::ReadItemEventMaps(const uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(MapsFixedHeader)) {
        TS_LOGE("get maps addr Failed!!!");
        return false;
    }
    auto procMapsAddr = reinterpret_cast<const MapsFixedHeader*>(buffer);
    pidAndStartAddrToMapsAddr_.Insert(procMapsAddr->pid, procMapsAddr->start, procMapsAddr);
#if WITH_EBPF_HELP
    if ((procMapsAddr->fileNameLen > size - sizeof(MapsFixedHeader)) || !procMapsAddr->fileNameLen) {
        TS_LOGE("maps fileNameLen error!!!");
        return false;
    }
    auto fileNameAddr = const_cast<char*>(reinterpret_cast<const char*>(procMapsAddr + 1));
    fileNameAddr[procMapsAddr->fileNameLen - 1] = '\0';
    auto fileNameIndex = traceDataCache_->GetDataIndex(fileNameAddr);

    // add proc Maps Data
    traceDataCache_->GetEbpfProcessMaps()->AppendNewData(procMapsAddr->start, procMapsAddr->end, procMapsAddr->offset,
                                                         procMapsAddr->pid, procMapsAddr->fileNameLen, fileNameIndex);
#endif
    return true;
}
template <class T>
void EbpfDataReader::AddSymbolsToTable(T* firstSymbolAddr, const int size, const ElfEventFixedHeader* elfAddr)
{
    for (auto i = 0; i < size; i++) {
        auto symAddr = firstSymbolAddr + i;
        if ((symAddr->st_info & STT_FUNC) && symAddr->st_value) {
            elfAddrAndStValueToSymAddr_.Insert(elfAddr, symAddr->st_value, reinterpret_cast<const uint8_t*>(symAddr));
        }
    }
}
void EbpfDataReader::UpdateElfAddrAndStValueToSymAddrMap(const ElfEventFixedHeader* elfAddr, uint32_t size)
{
    if (size < sizeof(ElfEventFixedHeader) + elfAddr->strTabLen + elfAddr->symTabLen + elfAddr->fileNameLen) {
        TS_LOGE("elf addr size error!!!");
        return;
    }
    auto symEntLen = elfAddr->symEntLen;
    auto symTabHeadAddr = reinterpret_cast<const uint8_t*>(elfAddr + 1) + elfAddr->strTabLen;
    if (symEntLen == ELF32_SYM) {
        AddSymbolsToTable(reinterpret_cast<const Elf32_Sym*>(symTabHeadAddr), elfAddr->symTabLen / symEntLen, elfAddr);
    } else {
        AddSymbolsToTable(reinterpret_cast<const Elf64_Sym*>(symTabHeadAddr), elfAddr->symTabLen / symEntLen, elfAddr);
    }
}
void EbpfDataReader::ReadKernelSymAddrMap(const KernelSymbolInfoHeader* elfAddr, uint32_t size)
{
    if (size < sizeof(KernelSymbolInfoHeader) + elfAddr->symTabLen + elfAddr->strTabLen) {
        TS_LOGE("elf addr size error!!!, size is:%u and the symTabLen is:%u, strTabLen is:%u", size, elfAddr->symTabLen,
                elfAddr->strTabLen);
        return;
    }
    auto symTabLen = elfAddr->symTabLen;
    auto sysItemSize = symTabLen / sizeof(KernelSymItem);
    auto start = reinterpret_cast<const KernelSymItem*>(elfAddr + 1);
    auto strTab = reinterpret_cast<const char*>(start + sysItemSize);
    maxKernelAddr_ = elfAddr->vaddrEnd;
    minKernelAddr_ = elfAddr->vaddrStart;
    for (uint32_t i = 0; i < sysItemSize; i++) {
        (void)memset_s(strSymbolName_, MAX_SYMBOL_LENGTH, 0, MAX_SYMBOL_LENGTH);
        auto item = start + i;
        if (strncpy_s(strSymbolName_, MAX_SYMBOL_LENGTH, strTab + item->nameOffset, MAX_SYMBOL_LENGTH) < 0) {
            TS_LOGE("get kernel symbol name error");
        }
        AddrDesc desc{item->size, traceDataCache_->dataDict_.GetStringIndex(strSymbolName_)};
        kernelSymbolMap_.insert(std::make_pair(item->value, desc));
    }
}

void EbpfDataReader::UpdateElfPathIndexToElfAddrMap(const ElfEventFixedHeader* elfAddr, uint32_t size)
{
    if ((elfAddr->fileNameLen > size - sizeof(ElfEventFixedHeader) - elfAddr->strTabLen - elfAddr->symTabLen) ||
        !elfAddr->fileNameLen) {
        TS_LOGE("elf filename len is error!!!");
        return;
    }

    uint64_t fileNameIndex = INVALID_UINT64;
    auto fileNameAddr = const_cast<char*>(reinterpret_cast<const char*>(reinterpret_cast<const uint8_t*>(elfAddr + 1) +
                                                                        elfAddr->strTabLen + elfAddr->symTabLen));
    fileNameAddr[elfAddr->fileNameLen - 1] = '\0';
    fileNameIndex = traceDataCache_->GetDataIndex(std::string(fileNameAddr));
    elfPathIndexToElfFixedHeaderAddr_.insert(std::make_pair(fileNameIndex, elfAddr));

#if WITH_EBPF_HELP
    // add Elf symbol Data
    traceDataCache_->GetEbpfElf()->AppendNewData(elfId_, elfAddr->textVaddr, elfAddr->textOffset, elfAddr->strTabLen,
                                                 elfAddr->symTabLen, elfAddr->fileNameLen, elfAddr->symEntLen,
                                                 fileNameIndex);
#endif
}

#if WITH_EBPF_HELP
template <class T>
void EbpfDataReader::AppendSymbolsToTable(T* firstSymbolAddr, const int size)
{
    for (auto i = 0; i < size; i++) {
        auto symAddr = firstSymbolAddr + i;
        if ((symAddr->st_info & STT_FUNC) && symAddr->st_value) {
            traceDataCache_->GetEbpfElfSymbol()->AppendNewData(elfId_, symAddr->st_name, symAddr->st_value,
                                                               symAddr->st_size);
        }
    }
}
void EbpfDataReader::UpdateEbpfElfSymbolTable(const ElfEventFixedHeader* elfAddr, uint32_t size)
{
    if (size < sizeof(ElfEventFixedHeader) + elfAddr->strTabLen + elfAddr->symTabLen + elfAddr->fileNameLen) {
        TS_LOGE("elf addr size error!!!");
        return;
    }
    auto symEntLen = elfAddr->symEntLen;
    if (symEntLen == ELF32_SYM) {
        AppendSymbolsToTable(
            reinterpret_cast<const Elf32_Sym*>(reinterpret_cast<const uint8_t*>(elfAddr + 1) + elfAddr->strTabLen),
            elfAddr->symTabLen / symEntLen);
    } else {
        AppendSymbolsToTable(
            reinterpret_cast<const Elf64_Sym*>(reinterpret_cast<const uint8_t*>(elfAddr + 1) + elfAddr->strTabLen),
            elfAddr->symTabLen / symEntLen);
    }
}
#endif

bool EbpfDataReader::ReadItemSymbolInfo(const uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(ElfEventFixedHeader)) {
        TS_LOGE("get symbol addr failed!!!");
        return false;
    }
    auto elfAddr = reinterpret_cast<const ElfEventFixedHeader*>(buffer);
    UpdateElfAddrAndStValueToSymAddrMap(elfAddr, size);
    UpdateElfPathIndexToElfAddrMap(elfAddr, size);
#if WITH_EBPF_HELP
    UpdateEbpfElfSymbolTable(elfAddr, size);
    elfId_++;
#endif
    return true;
}

bool EbpfDataReader::ReaItemKernelSymbolInfo(const uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(KernelSymbolInfoHeader)) {
        TS_LOGE("get symbol addr failed!!!");
        return false;
    }
    auto elfAddr = reinterpret_cast<const KernelSymbolInfoHeader*>(buffer);
    ReadKernelSymAddrMap(elfAddr, size);
    return true;
}
bool EbpfDataReader::ReadItemEventFs(const uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(FsFixedHeader)) {
        TS_LOGE("get file system event addr failed!!!");
        return false;
    }
    auto fsFixedHeaderAddr = reinterpret_cast<const FsFixedHeader*>(buffer);
    endTsToFsFixedHeader_.insert(std::make_pair(fsFixedHeaderAddr->endTime, fsFixedHeaderAddr));
    return true;
}

bool EbpfDataReader::ReadItemEventPagedMemory(const uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(PagedMemoryFixedHeader)) {
        TS_LOGE("get page memory event addr failed!!!");
        return false;
    }
    auto pagedMemoryFixedHeaderAddr = reinterpret_cast<const PagedMemoryFixedHeader*>(buffer);
    endTsToPagedMemoryFixedHeader_.insert(
        std::make_pair(pagedMemoryFixedHeaderAddr->endTime, pagedMemoryFixedHeaderAddr));
    return true;
}

bool EbpfDataReader::ReadItemEventBIO(const uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(BIOFixedHeader)) {
        TS_LOGE("get Block IO event addr failed!!!");
        return false;
    }
    auto bioFixedHeaderAddr = reinterpret_cast<const BIOFixedHeader*>(buffer);
    endTsToBIOFixedHeader_.insert(std::make_pair(bioFixedHeaderAddr->endTime, bioFixedHeaderAddr));
    return true;
}

bool EbpfDataReader::ReadItemEventStr(const uint8_t* buffer, uint32_t size)
{
    if (size < sizeof(StrEventFixedHeader)) {
        TS_LOGE("get str event addr failed!!!");
        return false;
    }
    auto strFixedHeaderAddr = reinterpret_cast<const StrEventFixedHeader*>(buffer);
    auto itid =
        streamFilters_->processFilter_->GetOrCreateThreadWithPid(strFixedHeaderAddr->tid, strFixedHeaderAddr->pid);
    auto strAddr = const_cast<char*>(reinterpret_cast<const char*>(strFixedHeaderAddr + 1));
    if ((strFixedHeaderAddr->strLen > size - sizeof(StrEventFixedHeader)) || !strFixedHeaderAddr->strLen) {
        TS_LOGE("invalid str event, strEventFixedHeader = %zu, strlen = %d, size = %d", sizeof(StrEventFixedHeader),
                strFixedHeaderAddr->strLen, size);
        return true;
    }
    strAddr[strFixedHeaderAddr->strLen - 1] = '\0';
    auto strIndex = traceDataCache_->GetDataIndex(strAddr);
    tracerEventToStrIndex_.Insert(strFixedHeaderAddr->srcTracer, strFixedHeaderAddr->srcType, itid,
                                  strFixedHeaderAddr->startTime, strIndex);
    return true;
}

QuatraMap<uint32_t, uint32_t, uint32_t, uint64_t, DataIndex>& EbpfDataReader::GetTracerEventToStrIndexMap()
{
    return tracerEventToStrIndex_;
}

SymbolAndFilePathIndex EbpfDataReader::GetSymbolNameIndexFromElfSym(uint64_t ip)
{
    SymbolAndFilePathIndex symbolAndFilePathIndex(false);
    auto end = kernelSymbolMap_.upper_bound(ip);
    auto length = std::distance(kernelSymbolMap_.begin(), end);
    if (length > 0) {
        end--;
        // Follow the rules of front closing and rear opening, [start, end)
        if (ip < end->first + end->second.size) {
            symbolAndFilePathIndex.flag = true;
            symbolAndFilePathIndex.symbolIndex = end->second.name;
            symbolAndFilePathIndex.filePathIndex = kernelFilePath_;
        } else {
            TS_LOGD("failed for ip:%lu, kernelip:%lu, size:%lu", ip, end->first, end->second.size);
        }
    }
    if (!symbolAndFilePathIndex.flag) {
        TS_LOGD("failed for ip:%lu", ip);
    }
    return symbolAndFilePathIndex;
}
const DoubleMap<uint32_t, uint64_t, const MapsFixedHeader*>& EbpfDataReader::GetPidAndStartAddrToMapsAddr() const
{
    return pidAndStartAddrToMapsAddr_;
}

const DoubleMap<const ElfEventFixedHeader*, uint64_t, const uint8_t*>&
    EbpfDataReader::GetElfAddrAndStartValueToSymAddr() const
{
    return elfAddrAndStValueToSymAddr_;
}

const std::multimap<uint64_t, const FsFixedHeader*>& EbpfDataReader::GetFileSystemEventMap() const
{
    return endTsToFsFixedHeader_;
}

const std::multimap<uint64_t, const PagedMemoryFixedHeader*>& EbpfDataReader::GetPagedMemoryMap() const
{
    return endTsToPagedMemoryFixedHeader_;
}

const std::multimap<uint64_t, const BIOFixedHeader*>& EbpfDataReader::GetBIOSampleMap() const
{
    return endTsToBIOFixedHeader_;
}

const std::map<DataIndex, const ElfEventFixedHeader*>& EbpfDataReader::GetElfPathIndexToElfAddr() const
{
    return elfPathIndexToElfFixedHeaderAddr_;
}
const EbpfDataHeader* EbpfDataReader::GetEbpfDataHeader() const
{
    return const_cast<const EbpfDataHeader*>(ebpfDataHeader_);
}
} // namespace TraceStreamer
} // namespace SysTuning
