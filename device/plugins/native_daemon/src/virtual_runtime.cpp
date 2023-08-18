/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#define HILOG_TAG "Runtime"

#include "virtual_runtime.h"

#include <cinttypes>
#include <iostream>
#include <sstream>
#include <unistd.h>
#if !is_mingw
#include <sys/mman.h>
#endif

#include "register.h"
#include "symbols_file.h"
#include "utilities.h"

using namespace std::chrono;
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
namespace {
std::atomic<uint64_t> callStackErrCnt = 0;
constexpr uint32_t CALL_STACK_ERROR_TIMES = 10;
constexpr uint32_t SYMBOL_FILES_SIZE = 512;
}
// we unable to access 'swapper' from /proc/0/
void VirtualRuntime::ClearMaps()
{
    processMemMaps_.clear();
}

VirtualRuntime::VirtualRuntime(const NativeHookConfig& hookConfig): hookConfig_(hookConfig)
{
    symbolsFiles_.reserve(SYMBOL_FILES_SIZE);
    if (!hookConfig_.offline_symbolization()) {
        userSymbolCache_.reserve(USER_SYMBOL_CACHE_LIMIT);
    }
}

VirtualRuntime::~VirtualRuntime()
{
    HILOG_INFO(LOG_CORE, "%s:%d UserSymbolCache size = %zu", __func__, __LINE__, userSymbolCache_.size());
    HILOG_INFO(LOG_CORE, "Total number of call stack errors: %" PRIu64 "", callStackErrCnt.load());
    ClearMaps();
}

std::string VirtualRuntime::ReadThreadName(pid_t tid)
{
    std::string comm = ReadFileToString(StringPrintf("/proc/%d/comm", tid)).c_str();
    comm.erase(std::remove(comm.begin(), comm.end(), '\r'), comm.end());
    comm.erase(std::remove(comm.begin(), comm.end(), '\n'), comm.end());
    return comm;
}

VirtualThread &VirtualRuntime::UpdateThread(pid_t pid, pid_t tid, const std::string name)
{
#ifdef HIPERF_DEBUG_TIME
    const auto startTime = steady_clock::now();
#endif
    VirtualThread &thread = GetThread(pid, tid);
    if (!name.empty()) {
        thread.name_ = name;
    }
#ifdef HIPERF_DEBUG_TIME
    updateThreadTimes_ += duration_cast<microseconds>(steady_clock::now() - startTime);
#endif
    return thread;
}

VirtualThread &VirtualRuntime::CreateThread(pid_t pid, pid_t tid)
{
    // make a new one
    userSpaceThreadMap_.emplace(std::piecewise_construct, std::forward_as_tuple(tid),
                                std::forward_as_tuple(pid, tid, symbolsFiles_, this));
    VirtualThread& thr = userSpaceThreadMap_.at(tid);
    return thr;
}

VirtualThread &VirtualRuntime::GetThread(pid_t pid, pid_t tid)
{
    HLOGV("find thread %u:%u", pid, tid);
    auto it = userSpaceThreadMap_.find(tid);
    if (it == userSpaceThreadMap_.end()) {
        // we also need thread
        VirtualThread& thr = CreateThread(pid, tid);
        return thr;
    } else {
        VirtualThread& thr = it->second;
        return thr;
    }
}

void VirtualRuntime::MakeCallFrame(Symbol &symbol, CallFrame &callFrame)
{
    callFrame.vaddrInFile_ = symbol.funcVaddr_;
    callFrame.symbolName_ = symbol.symbolName_;
    callFrame.symbolIndex_ = symbol.index_;
    callFrame.filePath_ = symbol.module_.empty() ? symbol.comm_ : symbol.module_;
    callFrame.symbolOffset_ = symbol.offset_;
    callFrame.callFrameId_ = symbol.symbolId_;
    callFrame.symbolNameId_ = symbol.symbolNameId_;
    callFrame.filePathId_ = symbol.filePathId_;
    if (symbol.funcVaddr_ != 0) {
        callFrame.offset_ = symbol.funcVaddr_;
    } else {
        callFrame.offset_ = callFrame.ip_;
    }
}

bool VirtualRuntime::GetSymbolName(pid_t pid, pid_t tid, std::vector<CallFrame>& callFrames, int offset, bool first)
{
#ifdef HIPERF_DEBUG_TIME
    const auto startTime = steady_clock::now();
#endif
    // Symbolic the Call Stack
    HLOGV("total %zu frames", callFrames.size());

    perf_callchain_context perfCallchainContext = PERF_CONTEXT_MAX;
    for (auto callFrameIt = callFrames.begin() + offset; callFrameIt != callFrames.end(); ++callFrameIt) {
        auto &callFrame = callFrameIt.operator*();
        if (callFrame.ip_ >= PERF_CONTEXT_MAX) {
            // dont care, this is not issue.
            HLOGV("%s", UpdatePerfContext(callFrame.ip_, perfCallchainContext).c_str());
            continue;
        }
        auto symbol = GetSymbol(callFrame, pid, tid,
            perfCallchainContext);
        if (symbol.isValid()) {
            MakeCallFrame(symbol, callFrame);
        } else {
#ifdef TRY_UNWIND_TWICE
            if (first) {
                if (failedIPs_.find(callFrame.ip_) == failedIPs_.end()) {
                    return false;
                } else {
                    callFrames.erase(callFrameIt, callFrames.end());
                    return true;
                }
            } else {
                failedIPs_.insert(callFrame.ip_);
                callFrames.erase(callFrameIt, callFrames.end());
                return true;
            }
#else
            ++callStackErrCnt;
            if (callStackErrCnt.load() % CALL_STACK_ERROR_TIMES == 0) {
                HILOG_DEBUG(LOG_CORE, "number of call stack errors: %" PRIu64 "", callStackErrCnt.load());
            }
            callFrames.erase(callFrameIt, callFrames.end());
            return true;
#endif
        }
        int index = callFrameIt - callFrames.begin();
        HLOGV(" (%u)unwind symbol: %*s%s", index, index, "", callFrame.ToSymbolString().c_str());
    }
#ifdef HIPERF_DEBUG_TIME
    auto usedTime = duration_cast<microseconds>(steady_clock::now() - startTime);
    if (usedTime.count() != 0) {
        HLOGV("cost %0.3f ms to symbolic ", usedTime.count() / MS_DUARTION);
    }
    symbolicRecordTimes_ += usedTime;
#endif
    return true;
}

void VirtualRuntime::UpdateMaps(pid_t pid, pid_t tid)
{
    auto &thread = UpdateThread(pid, tid);
    if (thread.ParseMap(processMemMaps_, true)) {
        HILOG_DEBUG(LOG_CORE, "voluntarily update maps succeed");
    } else {
        HILOG_DEBUG(LOG_CORE, "voluntarily update maps ignore");
    }
}

bool VirtualRuntime::UnwindStack(std::vector<u64>& regs,
                                 const u8* stack_addr,
                                 int stack_size,
                                 pid_t pid,
                                 pid_t tid,
                                 std::vector<CallFrame>& callFrames,
                                 size_t maxStackLevel)
{
#ifdef HIPERF_DEBUG_TIME
    const auto startTime = steady_clock::now();
#endif
    // if we have userstack ?
    int offset = 0;
    auto &thread = UpdateThread(pid, tid);
    if (stack_size > 0) {
        callstack_.UnwindCallStack(thread, &regs[0], regs.size(), stack_addr, stack_size, callFrames, maxStackLevel);
        if (callFrames.size() <= FILTER_STACK_DEPTH) {
            callFrames.clear();
            return false;
        }
        // Do not symbolize the first two frame, cause the two frame implement by tool itself
        offset = FILTER_STACK_DEPTH;
#ifdef HIPERF_DEBUG_TIME
        unwindCallStackTimes_ += duration_cast<microseconds>(steady_clock::now() - startTime);
#endif
    }
#ifdef HIPERF_DEBUG_TIME
    unwindFromRecordTimes_ += duration_cast<microseconds>(steady_clock::now() - startTime);
#endif
    if (hookConfig_.offline_symbolization()) {
        return true;
    }
    if (!GetSymbolName(pid, tid, callFrames, offset, true)) {
#ifdef TRY_UNWIND_TWICE
        HLOGD("clear and unwind one more time");
        if (!thread.ParseMap(processMemMaps_, true)) {
            GetSymbolName(pid, tid, callFrames, offset, false);
            return false;
        }
        if (stack_size > 0) {
            callFrames.clear();
            callstack_.UnwindCallStack(thread, &regs[0], regs.size(), stack_addr,
                stack_size, callFrames, maxStackLevel);
        }
        if (callFrames.size() <= FILTER_STACK_DEPTH) {
            callFrames.clear();
            return false;
        }
        if (!GetSymbolName(pid, tid, callFrames, offset, false)) {
            return false;
        }
#endif
    }
    return true;
}

bool VirtualRuntime::IsSymbolExist(const std::string& fileName)
{
    if (symbolsFiles_.find(fileName) != symbolsFiles_.end()) {
        HLOGV("already have '%s'", fileName.c_str());
        return true;
    }
    return false;
}

void VirtualRuntime::DelSymbolFile(const std::string& fileName)
{
    symbolsFiles_.erase(fileName);
}

void VirtualRuntime::UpdateSymbols(std::string fileName)
{
    HLOGD("try to find symbols for file: %s", fileName.c_str());
#ifdef HIPERF_DEBUG_TIME
    const auto startTime = steady_clock::now();
#endif
    if (symbolsFiles_.find(fileName) != symbolsFiles_.end()) {
        HLOGV("already have '%s'", fileName.c_str());
        return;
    }

    // found it by name
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(fileName);

    // set sybol path If it exists
    if (symbolsPaths_.size() > 0) {
        symbolsFile->setSymbolsFilePath(symbolsPaths_); // also load from search path
    }
    if (loadSymboleWhenNeeded_) {
        // load it when we need it
        symbolsFiles_[symbolsFile->filePath_] = std::move(symbolsFile);
    } else if (symbolsFile->LoadSymbols()) {
        symbolsFiles_[symbolsFile->filePath_] = std::move(symbolsFile);
    } else {
        HLOGW("symbols file for '%s' not found.", fileName.c_str());
    }
#ifdef HIPERF_DEBUG_TIME
    auto usedTime = duration_cast<microseconds>(steady_clock::now() - startTime);
    if (usedTime.count() != 0) {
        HLOGV("cost %0.3f ms to load '%s'", usedTime.count() / MS_DUARTION, fileName.c_str());
    }
    updateSymbolsTimes_ += usedTime;
#endif
}

const Symbol VirtualRuntime::GetKernelSymbol(uint64_t ip, const std::vector<MemMapItem> &memMaps,
                                             const VirtualThread &thread)
{
    Symbol vaddrSymbol(ip, thread.name_);
    for (auto &map : memMaps) {
        if (ip > map.begin_ && ip < map.end_) {
            HLOGM("found addr 0x%" PRIx64 " in kernel map 0x%" PRIx64 " - 0x%" PRIx64 " from %s",
                  ip, map.begin_, map.end_, map.name_.c_str());
            vaddrSymbol.module_ = map.name_;
            // found symbols by file name
            auto search = symbolsFiles_.find(map.name_);
            if (search != symbolsFiles_.end()) {
                auto& symbolsFile = search->second;
                vaddrSymbol.fileVaddr_ =
                        symbolsFile->GetVaddrInSymbols(ip, map.begin_, map.pageoffset_);
                HLOGV("found symbol vaddr 0x%" PRIx64 " for runtime vaddr 0x%" PRIx64
                        " at '%s'",
                        vaddrSymbol.fileVaddr_, ip, map.name_.c_str());
                if (!symbolsFile->SymbolsLoaded()) {
                    symbolsFile->LoadSymbols();
                }
                Symbol foundSymbols = symbolsFile->GetSymbolWithVaddr(vaddrSymbol.fileVaddr_);
                foundSymbols.taskVaddr_ = ip;
                if (!foundSymbols.isValid()) {
                    HLOGW("addr 0x%" PRIx64 " vaddr  0x%" PRIx64 " NOT found in symbol file %s",
                            ip, vaddrSymbol.fileVaddr_, map.name_.c_str());
                    return vaddrSymbol;
                } else {
                    return foundSymbols;
                }
            }
            HLOGW("addr 0x%" PRIx64 " in map but NOT found the symbol file %s", ip,
                  map.name_.c_str());
        } else {
            HLOGM("addr 0x%" PRIx64 " not in map 0x%" PRIx64 " - 0x%" PRIx64 " from %s", ip,
                  map.begin_, map.end_, map.name_.c_str());
        }
    }
    return vaddrSymbol;
}

const Symbol VirtualRuntime::GetUserSymbol(uint64_t ip, const VirtualThread &thread)
{
    Symbol vaddrSymbol(ip, thread.name_);
    auto [curMemMaps, itemIndex] = FindMap(ip);
    if (curMemMaps != nullptr) {
        auto symbolsFilesIter = symbolsFiles_.find(curMemMaps->name_);
        if (symbolsFilesIter != symbolsFiles_.end()) {
            auto symbolsFile = symbolsFilesIter->second.get();
            symbolsFile->LoadDebugInfo();
            vaddrSymbol.fileVaddr_ =
                symbolsFile->GetVaddrInSymbols(ip, curMemMaps->maps_[itemIndex].begin_,
                                               curMemMaps->maps_[itemIndex].pageoffset_);
            vaddrSymbol.module_ = curMemMaps->maps_[itemIndex].nameHold_;
            vaddrSymbol.symbolName_ = vaddrSymbol.Name();
            if (!symbolsFile->SymbolsLoaded()) {
                symbolsFile->LoadSymbols();
            }
            Symbol foundSymbols = symbolsFile->GetSymbolWithVaddr(vaddrSymbol.fileVaddr_);
            foundSymbols.taskVaddr_ = ip;
            foundSymbols.symbolName_ = foundSymbols.Name();
            if (!foundSymbols.isValid()) {
                vaddrSymbol.filePathId_ = curMemMaps->filePathId_;
                return vaddrSymbol;
            } else {
                foundSymbols.filePathId_ = curMemMaps->filePathId_;
                return foundSymbols;
            }
        } else {
            HLOGW("addr 0x%" PRIx64 " in map but NOT found the symbol file %s", ip,
                  curMemMaps->name_.c_str());
        }
    } else {
        HLOGW("ReportVaddrMapMiss");
#ifdef HIPERF_DEBUG
        thread.ReportVaddrMapMiss(ip);
#endif
    }
    return vaddrSymbol;
}

bool VirtualRuntime::GetSymbolCache(uint64_t ip, Symbol &symbol, const VirtualThread &thread)
{
    auto [curMemMaps, itemIndex] = FindMap(ip);
    if (curMemMaps != nullptr) {
        auto foundSymbolIter = userSymbolCache_.find(std::pair(ip, curMemMaps->filePathId_));
        if (foundSymbolIter != userSymbolCache_.end()) {
            symbol = foundSymbolIter->second;
            return true;
        }
    }
    return false;
}

void VirtualRuntime::UpdateSymbolCache(uint64_t ip, Symbol &symbol,
    HashList<uint64_t, Symbol> &cache)
{
    // review change to LRU for memmory
    HLOG_ASSERT_MESSAGE(cache.count(ip) == 0, "already have cached ip 0x%" PRIx64 "", ip);
    cache[ip] = symbol;
}

const Symbol VirtualRuntime::GetSymbol(CallFrame& callFrame, pid_t pid, pid_t tid,
                                       const perf_callchain_context &context)
{
    HLOGM("try find tid %u ip 0x%" PRIx64 " in %zu symbolsFiles ", tid, callFrame.ip_, symbolsFiles_.size());
    Symbol symbol;
    if (GetSymbolCache(callFrame.ip_, symbol, GetThread(pid, tid))) {
        return symbol;
    }
    if (context == PERF_CONTEXT_USER or (context == PERF_CONTEXT_MAX and !symbol.isValid())) {
        // check userspace memmap
        symbol = GetUserSymbol(callFrame.ip_, GetThread(pid, tid));
        if (symbol.isValid()) {
            HLOGM("GetUserSymbol valid tid = %d ip = 0x%" PRIx64 "", tid, callFrame.ip_);
            symbol.symbolId_ = userSymbolCache_.size() + 1;
            if (hookConfig_.string_compressed()) {
                FillSymbolNameId(callFrame, symbol);
                FillFileSet(callFrame, symbol);
            }
            callFrame.needReport_ |= CALL_FRAME_REPORT;
            userSymbolCache_[std::pair(callFrame.ip_, symbol.filePathId_)] = symbol;
        } else {
            HLOGM("GetUserSymbol invalid!");
        }
    }

    return symbol;
}

bool VirtualRuntime::SetSymbolsPaths(const std::vector<std::string> &symbolsPaths)
{
    std::unique_ptr<SymbolsFile> symbolsFile = SymbolsFile::CreateSymbolsFile(SYMBOL_UNKNOW_FILE);
    // we need check if the path is accessable
    bool accessable = symbolsFile->setSymbolsFilePath(symbolsPaths);
    if (accessable) {
        symbolsPaths_ = symbolsPaths;
    } else {
        if (!symbolsPaths.empty()) {
            printf("some symbols path unable access\n");
        }
    }
    return accessable;
}

void VirtualRuntime::FillMapsCache(std::string& currentFileName, MemMapItem& memMapItem)
{
    if (currentFileName.compare(memMapItem.name_) != 0) {
        currentFileName = memMapItem.name_;
        soBegin_ = memMapItem.begin_;
        mapsCache_[memMapItem.begin_] =
            MemMaps(memMapItem.begin_, memMapItem.end_, memMapItem.pageoffset_,
                    memMapItem.type_, ++memMapFilePathId_, memMapItem.name_);
    } else {
        if (auto curMemMapsIter = mapsCache_.find(soBegin_);
                curMemMapsIter != mapsCache_.end()) {
            auto& curMemMaps = curMemMapsIter->second;
            curMemMaps.soEnd_ = memMapItem.end_;
            curMemMaps.maps_.back().end_ = memMapItem.begin_;
            curMemMaps.maps_.emplace_back(memMapItem.begin_, memMapItem.end_, memMapItem.type_,
                                          memMapItem.pageoffset_, curMemMaps.name_);
            if (memMapItem.type_ & PROT_EXEC) {
                offlineMapAddr_.push_back(soBegin_);
            }
        }
    }
}

inline void VirtualRuntime::FillSymbolNameId(CallFrame& callFrame, Symbol& symbol)
{
    auto itFuntion = functionMap_.find(std::string(symbol.symbolName_));
    if (itFuntion != functionMap_.end()) {
        symbol.symbolNameId_ = itFuntion->second;
    } else {
        symbol.symbolNameId_ = functionMap_.size() + 1;
        functionMap_[std::string(symbol.symbolName_)] = symbol.symbolNameId_;
        callFrame.needReport_ |= SYMBOL_NAME_ID_REPORT;
    }
}

inline void VirtualRuntime::FillFileSet(CallFrame& callFrame, const Symbol& symbol)
{
    auto itFile = fileSet_.find(symbol.filePathId_);
    if (itFile == fileSet_.end()) {
        callFrame.needReport_ |= FILE_PATH_ID_REPORT;
        fileSet_.insert(symbol.filePathId_);
    }
}

void VirtualRuntime::HandleMapInfo(uint64_t begin, uint64_t length, uint32_t flags,
                                   uint64_t offset, const std::string& filePath)
{
    if (!(flags & MAP_FIXED) || mapsCache_.empty()) {
        if (mapsCache_.find(begin) != mapsCache_.end()) {
            return;
        }
        soBegin_ = begin;
        mapsCache_[begin] = MemMaps(begin, begin + length, offset, flags, ++memMapFilePathId_, filePath);
        UpdateSymbols(filePath);
    } else {
        auto curMemMapsIter = mapsCache_.find(soBegin_);
        if (curMemMapsIter != mapsCache_.end()) {
            auto& curMemMaps = curMemMapsIter->second;
            curMemMaps.soEnd_ = begin + length;
            curMemMaps.maps_.back().end_ = begin;
            curMemMaps.maps_.emplace_back(begin, begin + length, (uint16_t)flags, offset, curMemMaps.name_);
            if (flags & PROT_EXEC) {
                offlineMapAddr_.push_back(soBegin_);
            }
        }
    }
}

void VirtualRuntime::RemoveMaps(uint64_t addr)
{
    mapsCache_.erase(addr);
}

std::pair<MemMaps*, uint32_t> VirtualRuntime::FindMap(uint64_t addr)
{
    auto iter = mapsCache_.upper_bound(addr);
    if (iter == mapsCache_.begin()) {
        // have map 2 3 4 5
        // find 1 , will return 2 (index 0, begin elem)
        // this same as not found any thins
        return {nullptr, 0};
    }

    auto& curMemMaps = (--iter)->second;
    if (addr >= curMemMaps.soBegin_ && addr < curMemMaps.soEnd_) {
        for (auto curMemMapItem = curMemMaps.maps_.begin();
            curMemMapItem != curMemMaps.maps_.end(); ++curMemMapItem) {
            if (addr >= curMemMapItem->begin_ && addr < curMemMapItem->end_) {
                return {&(curMemMaps), curMemMapItem - curMemMaps.maps_.begin()};
            }
        }
    }
    return {nullptr, 0};
}
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS