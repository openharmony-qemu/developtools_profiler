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

#include "dfx_maps.h"
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
constexpr uint32_t SECOND_INDEX = 2;
constexpr uint32_t THIRD_INDEX = 3;
constexpr uint32_t INFO_SIZE = 4;
}
// we unable to access 'swapper' from /proc/0/
void VirtualRuntime::ClearMaps()
{
    processMaps_.clear();
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
    PROFILER_LOG_INFO(LOG_CORE, "%s:%d UserSymbolCache size = %zu", __func__, __LINE__, userSymbolCache_.size());
    PROFILER_LOG_INFO(LOG_CORE, "Total number of call stack errors: %" PRIu64 "", callStackErrCnt.load());
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
    pid_ = pid;
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

void VirtualRuntime::MakeCallFrame(DfxSymbol &symbol, CallFrame &callFrame)
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

bool VirtualRuntime::GetSymbolName(pid_t pid, pid_t tid, std::vector<CallFrame>& callFrames, int offset, bool first,
                                   SymbolType type)
{
#ifdef HIPERF_DEBUG_TIME
    const auto startTime = steady_clock::now();
#endif
    // Symbolic the Call Stack
    HLOGV("total %zu frames", callFrames.size());

    perf_callchain_context perfCallchainContext = PERF_CONTEXT_MAX;
    for (auto callFrameIt = callFrames.begin() + offset; callFrameIt != callFrames.end(); ++callFrameIt) {
        auto &callFrame = callFrameIt.operator*();
        if (type == SymbolType::JS_SYMBOL && !callFrame.isJsFrame_) {
           // only symbolize arkts frame
            continue;
        }
        if (type == SymbolType::NATIVE_SYMBOL && callFrame.isJsFrame_) {
            continue;
        }
        if (callFrame.ip_ >= PERF_CONTEXT_MAX) {
            // dont care, this is not issue.
            HLOGV("%s", UpdatePerfContext(callFrame.ip_, perfCallchainContext).c_str());
            continue;
        }
        auto symbol = GetSymbol(callFrame, pid, tid,
            perfCallchainContext);
        if (symbol.IsValid()) {
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
                PROFILER_LOG_DEBUG(LOG_CORE, "number of call stack errors: %" PRIu64 "", callStackErrCnt.load());
            }
            if (callFrames.back().isJsFrame_) { //The fp mode js call stack is behind the native
            //call stack, so it can't be deleted entirely
                callFrameIt = callFrames.erase(callFrameIt);
                --callFrameIt;
                continue;
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
    if (thread.ParseMap(processMaps_, true)) {
        PROFILER_LOG_DEBUG(LOG_CORE, "voluntarily update maps succeed");
    } else {
        PROFILER_LOG_DEBUG(LOG_CORE, "voluntarily update maps ignore");
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
    auto &thread = UpdateThread(pid, tid);
    if (stack_size > 0) {
        callstack_.UnwindCallStack(thread, &regs[0], regs.size(), stack_addr, stack_size, callFrames, maxStackLevel,
            hookConfig_.js_stack_report() > 0 ? hookConfig_.max_js_stack_depth() : 0,
            hookConfig_.js_stack_report() > 0);
        if (callFrames.size() <= FILTER_STACK_DEPTH) {
            callFrames.clear();
            return false;
        }
        // Do not symbolize the first two frame, cause the two frame implement by tool itself
#ifdef HIPERF_DEBUG_TIME
        unwindCallStackTimes_ += duration_cast<microseconds>(steady_clock::now() - startTime);
#endif
    }
#ifdef HIPERF_DEBUG_TIME
    unwindFromRecordTimes_ += duration_cast<microseconds>(steady_clock::now() - startTime);
#endif
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

void VirtualRuntime::UpdateSymbols(std::string fileName, std::shared_ptr<DfxMap> map)
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
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(fileName, pid_);
    symbolsFile->SetMapsInfo(map);
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

bool VirtualRuntime::UpdateHapSymbols(std::shared_ptr<DfxMap> map)
{
    auto symbolsFile = SymbolsFile::CreateSymbolsFile(map->name);
    if (symbolsFile == nullptr) {
        HLOGV("Failed to load CreateSymbolsFile for exec section in hap(%s)", map->name.c_str());
        return false;
    }
    symbolsFile->SetMapsInfo(map);
    // update maps name if load debuginfo successfully
    if (!symbolsFile->LoadDebugInfo(map)) {
        HLOGV("Failed to load debuginfo for exec section in hap(%s)", map->name.c_str());
        return false;
    }

    if (!loadSymboleWhenNeeded_) {
        symbolsFile->LoadSymbols(map);
    }
    symbolsFiles_[symbolsFile->filePath_] = (std::move(symbolsFile));
    return true;
}

const DfxSymbol VirtualRuntime::GetKernelSymbol(uint64_t ip, const std::vector<std::shared_ptr<DfxMap>> &maps,
                                                const VirtualThread &thread)
{
    DfxSymbol vaddrSymbol(ip, thread.name_);
    for (auto &map : maps) {
        if (ip > map->begin && ip < map->end) {
            HLOGM("found addr 0x%" PRIx64 " in kernel map 0x%" PRIx64 " - 0x%" PRIx64 " from %s",
                  ip, map->begin, map->end, map->name.c_str());
            vaddrSymbol.module_ = map->name;
            // found symbols by file name
            auto search = symbolsFiles_.find(map->name);
            if (search != symbolsFiles_.end()) {
                auto& symbolsFile = search->second;
                vaddrSymbol.fileVaddr_ =
                        symbolsFile->GetVaddrInSymbols(ip, map->begin, map->offset);
                HLOGV("found symbol vaddr 0x%" PRIx64 " for runtime vaddr 0x%" PRIx64
                        " at '%s'",
                        vaddrSymbol.fileVaddr_, ip, map->name.c_str());
                if (!symbolsFile->SymbolsLoaded()) {
                    symbolsFile->LoadSymbols(map);
                }
                DfxSymbol foundSymbols = symbolsFile->GetSymbolWithVaddr(vaddrSymbol.fileVaddr_);
                foundSymbols.taskVaddr_ = ip;
                if (!foundSymbols.IsValid()) {
                    HLOGW("addr 0x%" PRIx64 " vaddr  0x%" PRIx64 " NOT found in symbol file %s",
                            ip, vaddrSymbol.fileVaddr_, map->name.c_str());
                    return vaddrSymbol;
                } else {
                    return foundSymbols;
                }
            }
            HLOGW("addr 0x%" PRIx64 " in map but NOT found the symbol file %s", ip,
                  map->name.c_str());
        } else {
            HLOGM("addr 0x%" PRIx64 " not in map 0x%" PRIx64 " - 0x%" PRIx64 " from %s", ip,
                  map->begin, map->end, map->name.c_str());
        }
    }
    return vaddrSymbol;
}

const DfxSymbol VirtualRuntime::GetUserSymbol(uint64_t ip, const VirtualThread &thread)
{
    DfxSymbol vaddrSymbol(ip, thread.name_);
    auto [curMaps, itemIndex] = FindMap(ip);
    if (curMaps != nullptr) {
        auto symbolsFilesIter = symbolsFiles_.find((curMaps->GetMaps())[itemIndex]->name);
        if (symbolsFilesIter != symbolsFiles_.end()) {
            auto symbolsFile = symbolsFilesIter->second.get();
            symbolsFile->LoadDebugInfo((curMaps->GetMaps())[itemIndex]);
            vaddrSymbol.fileVaddr_ =
                symbolsFile->GetVaddrInSymbols(ip, (curMaps->GetMaps())[itemIndex]->begin,
                                               (curMaps->GetMaps())[itemIndex]->offset);
            vaddrSymbol.module_ = (curMaps->GetMaps())[itemIndex]->name;
            vaddrSymbol.symbolName_ = vaddrSymbol.GetName();
            if (!symbolsFile->SymbolsLoaded()) {
                symbolsFile->LoadSymbols((curMaps->GetMaps())[itemIndex]);
            }

            DfxSymbol foundSymbols;

            if (!symbolsFile->IsAbc()) {
                foundSymbols = symbolsFile->GetSymbolWithVaddr(vaddrSymbol.fileVaddr_);
            } else {
                HLOGD("symbolsFile:%s is ABC :%d", symbolsFile->filePath_.c_str(), symbolsFile->IsAbc());
                foundSymbols = symbolsFile->GetSymbolWithPcAndMap(ip, curMaps->GetMaps()[itemIndex]);
            }
            foundSymbols.taskVaddr_ = ip;
            foundSymbols.symbolName_ = foundSymbols.GetName();
            if (!foundSymbols.IsValid()) {
                vaddrSymbol.filePathId_ = curMaps->filePathId_;
                return vaddrSymbol;
            } else {
                foundSymbols.filePathId_ = curMaps->filePathId_;
                return foundSymbols;
            }
        } else {
            HLOGW("addr 0x%" PRIx64 " in map but NOT found the symbol file %s", ip,
                  curMaps->name_.c_str());
        }
    } else {
        HLOGW("ReportVaddrMapMiss");
#ifdef HIPERF_DEBUG
        thread.ReportVaddrMapMiss(ip);
#endif
    }
    return vaddrSymbol;
}

bool VirtualRuntime::GetSymbolCache(uint64_t ip, DfxSymbol &symbol, const VirtualThread &thread)
{
    auto [curMaps, itemIndex] = FindMap(ip);
    if (curMaps != nullptr) {
        auto foundSymbolIter = userSymbolCache_.find(std::pair(ip, curMaps->filePathId_));
        if (foundSymbolIter != userSymbolCache_.end()) {
            symbol = foundSymbolIter->second;
            return true;
        }
    }
    return false;
}

void VirtualRuntime::UpdateSymbolCache(uint64_t ip, DfxSymbol &symbol,
    HashList<uint64_t, DfxSymbol> &cache)
{
    // review change to LRU for memmory
    HLOG_ASSERT_MESSAGE(cache.count(ip) == 0, "already have cached ip 0x%" PRIx64 "", ip);
    cache[ip] = symbol;
}

const DfxSymbol VirtualRuntime::GetSymbol(CallFrame& callFrame, pid_t pid, pid_t tid,
                                          const perf_callchain_context &context)
{
    HLOGM("try find tid %u ip 0x%" PRIx64 " in %zu symbolsFiles ", tid, callFrame.ip_, symbolsFiles_.size());
    DfxSymbol symbol;
    if (hookConfig_.fp_unwind() && callFrame.isJsFrame_) {
        if (ArktsGetSymbolCache(callFrame, symbol)) {
            return symbol;
        } else {
            symbol.filePathId_ = FillArkTsFilePath(callFrame.filePath_);
            symbol.module_ = callFrame.filePath_;
            symbol.symbolName_ = callFrame.symbolName_;
            symbol.symbolId_ = userSymbolCache_.size() + 1;
            if (hookConfig_.string_compressed()) {
                FillSymbolNameId(callFrame, symbol);
                FillFileSet(callFrame, symbol);
            }
            callFrame.needReport_ |= CALL_FRAME_REPORT;
            userSymbolCache_[std::pair(callFrame.ip_, symbol.filePathId_)] = symbol;
            return symbol;
        }
    } else if (GetSymbolCache(callFrame.ip_, symbol, GetThread(pid, tid))) {
        return symbol;
    }
    if (context == PERF_CONTEXT_USER || (context == PERF_CONTEXT_MAX && !symbol.IsValid())) {
        // check userspace memmap
        symbol = GetUserSymbol(callFrame.ip_, GetThread(pid, tid));
        if (symbol.IsValid()) {
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

void VirtualRuntime::FillMapsCache(std::string& currentFileName, std::shared_ptr<DfxMap> mapItem)
{
    if (currentFileName.compare(mapItem->name) != 0) {
        currentFileName = mapItem->name;
        soBegin_ = mapItem->begin;
        auto memMaps = std::make_shared<MemMaps>(++memMapFilePathId_);
        memMaps->AddMap(mapItem, true);
        mapsCache_[mapItem->begin] = memMaps;
    } else {
        if (auto curMapsIter = mapsCache_.find(soBegin_);
                curMapsIter != mapsCache_.end()) {
            auto& curMaps = curMapsIter->second;
            curMaps->soEnd_ = mapItem->end;
            curMaps->AddMap(mapItem, false);
            if (mapItem->prots & PROT_EXEC) {
                offlineMapAddr_.push_back(soBegin_);
            }
        }
    }
}

void VirtualRuntime::FillSymbolNameId(CallFrame& callFrame, DfxSymbol& symbol)
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

void VirtualRuntime::FillFileSet(CallFrame& callFrame, const DfxSymbol& symbol)
{
    auto itFile = fileSet_.find(symbol.filePathId_);
    if (itFile == fileSet_.end()) {
        callFrame.needReport_ |= FILE_PATH_ID_REPORT;
        fileSet_.insert(symbol.filePathId_);
    }
}

void VirtualRuntime::HandleMapInfo(std::vector<uint64_t> info, const std::string& filePath, pid_t pid, pid_t tid)
{
    if (info.size() != INFO_SIZE) {
        return;
    }
    uint64_t begin = info[0];
    uint64_t length = info[1];
    uint64_t flags = info[SECOND_INDEX];
    uint64_t offset = info[THIRD_INDEX];
    if (!(flags & MAP_FIXED)) {
        return;
    }
    if (offset == 0 && mapsCache_.find(begin) == mapsCache_.end()) {
        soBegin_ = begin;
        std::shared_ptr<DfxMap> mapItem = std::make_shared<DfxMap>(begin, begin + length, offset, flags, filePath);
        auto memMaps = std::make_shared<MemMaps>(++memMapFilePathId_);
        memMaps->AddMap(mapItem, true);
        mapsCache_[begin] = memMaps;
        UpdateSymbols(filePath, mapItem);
        if (!hookConfig_.fp_unwind() && hookConfig_.startup_mode()) {
            auto &thread = UpdateThread(pid, tid);
            thread.ParseMap(processMaps_, false);
        } else if (!hookConfig_.fp_unwind()) {
            auto &thread = UpdateThread(pid, tid);
            processMaps_.emplace_back(mapItem);
            thread.SortMaps();
        }
    } else {
        auto curMapsIter = mapsCache_.find(soBegin_);
        if (curMapsIter != mapsCache_.end() && (curMapsIter->second->name_ == filePath)) {
            auto& curMaps = curMapsIter->second;
            curMaps->soEnd_ = begin + length;
            std::shared_ptr<DfxMap> mapItem = std::make_shared<DfxMap>(begin, begin + length,
                                                                       offset, flags, curMaps->name_);
            if (mapItem->name.find(".hap") != std::string::npos && (mapItem->prots & PROT_EXEC)) {
                mapItem->prevMap = curMaps->GetMaps().back();
                HLOGD("update hap(%s) symbols", mapItem->name.c_str());
                UpdateHapSymbols(mapItem);
            }
            if (!hookConfig_.fp_unwind() && !hookConfig_.startup_mode()) {
                auto &thread = UpdateThread(pid, tid);
                processMaps_.emplace_back(mapItem);
                thread.SortMaps();
            }
            if (begin == curMaps->soBegin_) {
                if (!curMaps->ReplaceFront(mapItem)) {
                    curMaps->AddMap(mapItem, false);
                }
            } else {
                curMaps->AddMap(mapItem, false);
            }
        }
    }
    if (flags & PROT_EXEC) {
        offlineMapAddr_.push_back(soBegin_);
    }
}

void VirtualRuntime::RemoveMaps(uint64_t addr)
{
    mapsCache_.erase(addr);
}

std::pair<std::shared_ptr<MemMaps>, uint32_t> VirtualRuntime::FindMap(uint64_t addr)
{
    auto iter = mapsCache_.upper_bound(addr);
    if (iter == mapsCache_.begin()) {
        // have map 2 3 4 5
        // find 1 , will return 2 (index 0, begin elem)
        // this same as not found any thins
        return {nullptr, 0};
    }

    std::shared_ptr<MemMaps> curMaps = (--iter)->second;
    if (addr >= curMaps->soBegin_ && addr < curMaps->soEnd_) {
        std::vector<std::shared_ptr<DfxMap>> mapVec = curMaps->GetMaps();
        for (auto curMapItem = mapVec.begin();
            curMapItem != mapVec.end(); ++curMapItem) {
            if (addr >= (*curMapItem)->begin && addr < (*curMapItem)->end) {
                return {curMaps, curMapItem - mapVec.begin()};
            }
        }
    }
    return {nullptr, 0};
}

bool VirtualRuntime::ArktsGetSymbolCache(CallFrame& callFrame, DfxSymbol &symbol)
{
    uint32_t jsfilePathId = FindArkTsFilePath(callFrame.filePath_);
    if (jsfilePathId != 0) {
        auto foundSymbolIter = userSymbolCache_.find(std::pair(callFrame.ip_, jsfilePathId));
        if (foundSymbolIter != userSymbolCache_.end()) {
            symbol = foundSymbolIter->second;
            return true;
        }
    }
    return false;
}

uint32_t VirtualRuntime::FindArkTsFilePath(std::string_view& jstr)
{
    auto iter = jsUrlMap_.find(jstr);
    if (iter == jsUrlMap_.end()) {
        return 0;
    } else {
        return iter->second;
    }
}

uint32_t VirtualRuntime::FillArkTsFilePath(std::string_view& jstr)
{
    auto iter = jsUrlMap_.find(jstr);
    if (iter == jsUrlMap_.end()) {
        jsUrlMap_[jstr] = ++memMapFilePathId_;
    }
    return jsUrlMap_[jstr];
}

void VirtualRuntime::FillJsSymbolCache(CallFrame& callFrame, const DfxSymbol& symbol)
{
    userSymbolCache_[std::pair(callFrame.ip_, symbol.filePathId_)] = symbol;
}

uint32_t VirtualRuntime::GetJsSymbolCacheSize()
{
    return userSymbolCache_.size() + 1;
}

} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
