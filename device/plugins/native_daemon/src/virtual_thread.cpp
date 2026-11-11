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
#define HILOG_TAG "RuntimeThread"

#include "virtual_thread.h"

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <sstream>
#if !is_mingw
#include <sys/mman.h>
#endif

#include "common.h"
#include "symbols_file.h"
#include "utilities.h"
#include "virtual_runtime.h"
#include "stack_map_query.h"
namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
#ifdef DEBUG_TIME
bool VirtualThread::IsSorted() const
{
    for (std::size_t index = 1; index < maps_->size(); ++index) {
        if ((*maps_)[index - 1].end > (*maps_)[index].begin) {
            std::cout << "maps_ order error:\n"
                      << "    " << (*maps_)[index - 1].begin << "-" << (*maps_)[index - 1].end
                      << "    " << (*maps_)[index].begin << "-" << (*maps_)[index].end;
            return false;
        }
    }
    return true;
}
#endif

const std::pair<std::shared_ptr<MemMaps>, uint32_t> VirtualThread::FindMemMapsByAddr(uint64_t addr) const
{
    return virtualruntime_->FindMap(addr);
}

const std::shared_ptr<DfxMap> VirtualThread::FindMapByAddr(uint64_t addr) const
{
    HLOGM("try found vaddr 0x%" PRIx64 " in maps %zu ", addr, maps_->size());
    if (maps_->size() == 0) {
        return nullptr;
    }
    auto r = OHOS::Developtools::StackCommon::LookupMapByAddrContiguous(*maps_, addr);
    if (!r.ok) {
        return nullptr;
    }
    if (r.sortedPosition > 0) {
        (*maps_)[r.mapVectorIndex]->prevMap = (*maps_)[r.sortedPosition - 1];
    }
    return (*maps_)[r.mapVectorIndex];
}
VirtualThread::VirtualThread(pid_t pid,
                             pid_t tid,
                             const std::unordered_map<std::string, std::unique_ptr<SymbolsFile>>& symbolsFiles,
                             VirtualRuntime* runtime,
                             bool parseFlag)
    : pid_(pid), tid_(tid), symbolsFiles_(symbolsFiles), virtualruntime_(runtime)
{
    maps_ = &virtualruntime_->processMaps_;
    isContainer_ = virtualruntime_->IsContainer();
    if (parseFlag) {
        if (virtualruntime_->processMaps_.size() == 0) {
            this->ParseMap(virtualruntime_->processMaps_);
        }
    }

    this->name_ = ReadThreadName(pid);
    HLOGM("%d %d map from parent size is %zu", pid, tid, maps_->size());
}

bool VirtualThread::IsContainer() const
{
    return isContainer_.load();
}

std::string VirtualThread::ReadThreadName(pid_t tid)
{
    std::string comm = ReadFileToString(StringPrintf("/proc/%d/comm", tid)).c_str();
    comm.erase(std::remove(comm.begin(), comm.end(), '\r'), comm.end());
    comm.erase(std::remove(comm.begin(), comm.end(), '\n'), comm.end());
    return comm;
}

const std::shared_ptr<DfxMap> VirtualThread::FindMapByFileInfo(const std::string name, uint64_t offset) const
{
    auto map = OHOS::Developtools::StackCommon::FindMapByFileOffset(*maps_, name, offset);
    if (map != nullptr) {
        HLOGMMM("found fileoffset 0x%" PRIx64 " in map (0x%" PRIx64 " - 0x%" PRIx64
                " pageoffset 0x%" PRIx64 ")  from %s",
                offset, map->begin, map->end, map->offset, map->name.c_str());
    } else {
        HLOGM("NOT found offset 0x%" PRIx64 " in maps %zu ", offset, maps_->size());
    }
    return map;
}

SymbolsFile *VirtualThread::FindSymbolsFileByMap(std::shared_ptr<DfxMap> inMap) const
{
    auto search = symbolsFiles_.find(inMap->name);
    if (search != symbolsFiles_.end()) {
        auto& symbolsFile = search->second;
        HLOGM("found symbol for map '%s'", inMap->name.c_str());
        symbolsFile->LoadDebugInfo(inMap);
        return symbolsFile.get();
    }
#ifdef DEBUG_MISS_SYMBOL
    if (find(missedSymbolFile_.begin(), missedSymbolFile_.end(), inMap->name) ==
        missedSymbolFile_.end()) {
        missedSymbolFile_.emplace_back(inMap->name);
        HLOGW("NOT found symbol for map '%s'", inMap->name.c_str());
        for (const auto &file : symbolsFiles_) {
            HLOGW(" we have '%s'", file->filePath_.c_str());
        }
    }
#endif
    return nullptr;
}

SymbolsFile *VirtualThread::FindSymbolsFileByName(const std::string &name) const
{
    auto search = symbolsFiles_.find(name);
    if (search != symbolsFiles_.end()) {
        auto& symbolsFile = search->second;
        HLOGM("found symbol for map '%s'", name.c_str());
        symbolsFile->LoadDebugInfo();
        return symbolsFile.get();
    }
#ifdef DEBUG_MISS_SYMBOL
    if (find(missedSymbolFile_.begin(), missedSymbolFile_.end(), name) ==
        missedSymbolFile_.end()) {
        missedSymbolFile_.emplace_back(name);
        HLOGW("NOT found symbol for map '%s'", name.c_str());
        for (const auto &file : symbolsFiles_) {
            HLOGW(" we have '%s'", file->filePath_.c_str());
        }
    }
#endif
    return nullptr;
}

#ifdef HIPERF_DEBUG
void VirtualThread::ReportVaddrMapMiss(uint64_t vaddr) const
{
    if (DebugLogger::GetInstance()->GetLogLevel() <= LEVEL_VERBOSE) {
        if (missedRuntimeVaddr_.find(vaddr) == missedRuntimeVaddr_.end()) {
            missedRuntimeVaddr_.insert(vaddr);
            HLOGV("vaddr %" PRIx64 " not found in any map", vaddr);
            for (auto &map : *maps_) {
                HLOGV("map %s ", map->ToString().c_str());
            }
        }
    }
}
#endif

bool VirtualThread::ReadRoMemory(uint64_t vaddr, uint8_t *data, size_t size) const
{
    auto [curMemMaps, itemIndex] = virtualruntime_->FindMap(vaddr);
    if (curMemMaps != nullptr) {
        // found symbols by file name
        SymbolsFile *symbolsFile = FindSymbolsFileByMap((curMemMaps->GetMaps())[itemIndex]);
        if (symbolsFile != nullptr) {
            std::shared_ptr<DfxMap> map = (curMemMaps->GetMaps())[itemIndex];
            HLOGM("read vaddr from addr is 0x%" PRIx64 "  mapStart :0x%" PRIx64 " mapOffset :0x%" PRIx64 " at '%s'",
                  vaddr - map->begin, map->begin, map->offset, map->name.c_str());
            map->elf = symbolsFile->GetElfFile();
            if (map->elf != nullptr) {
                auto fileOffset = map->FileOffsetFromAddr(vaddr);
                fileOffset -= map->elf->GetBaseOffset();
                map->elf->Read(fileOffset, data, size);
                return true;
            }
            HLOGE("ElfFile(%s) is null or read file offset from addr fail", curMemMaps->name_.c_str());
            return false;
        } else {
            HLOGE("found addr %" PRIx64 " in map but not loaded symbole %s", vaddr, curMemMaps->name_.c_str());
        }
    } else {
#ifdef HIPERF_DEBUG
        ReportVaddrMapMiss(vaddr);
#endif
    }
    return false;
}

void VirtualThread::RegulateMapPath(std::shared_ptr<DfxMap>& map, const std::string& processName)
{
    if (map->name.empty()) {
        return;
    }
    if (!isContainer_) {
        NeedAdaptHMBundlePath(map->name, processName);
        return;
    }
    std::string containerRoot = StringPrintf("/proc/%d/root", pid_);
    if (!StringStartsWith(map->name, containerRoot)) {
        map->name = containerRoot + map->name;
    }
}

bool VirtualThread::ParseMap(std::vector<std::shared_ptr<DfxMap>>& memMaps, bool update)
{
    std::string mapPath = StringPrintf("/proc/%d/maps", pid_);
    std::shared_ptr<DfxMaps> dfxMaps = OHOS::HiviewDFX::DfxMaps::Create(pid_, mapPath);
    if (dfxMaps == nullptr) {
        HLOGE("VirtualThread Failed to Parse Map.");
        return false;
    }
    memMaps = dfxMaps->GetMaps();
    virtualruntime_->UpdateAdltBase();
    bool mapsAdded = !update;
    std::vector<std::shared_ptr<DfxMap>> tempMap;
    std::string tempMapName;
    std::shared_ptr<DfxMap> prevMap = nullptr;
    std::string processName = COMMON::GetBundleNameByPid(pid_);
    for (auto &memMapItem : memMaps) {
        RegulateMapPath(memMapItem, processName);
        if (virtualruntime_->IsArktsMap(memMapItem)) {
            virtualruntime_->arktsMapTree_[memMapItem->begin] = {
                memMapItem, virtualruntime_->GetOrCreateFilePathId(memMapItem->name)};
            virtualruntime_->arktsMapCache_.clear();
        }
        if (update) {
            if (virtualruntime_->IsSymbolExist(memMapItem->name)) {
                continue;
            }
            virtualruntime_->FillMapsCache(tempMapName, memMapItem);
            mapsAdded = true;
            tempMap.push_back(memMapItem);
        } else {
            virtualruntime_->FillMapsCache(tempMapName, memMapItem);
        }
        bool updateNormalSymbol = true;
        if (memMapItem->name.find(".hap") != std::string::npos && (memMapItem->prots & PROT_EXEC)) {
            memMapItem->prevMap = prevMap;
            HLOGD("update hap(%s) symbols", memMapItem->name.c_str());
            updateNormalSymbol = !virtualruntime_->UpdateHapSymbols(memMapItem);
        }
        if (updateNormalSymbol) {
            if (IsJitFile(memMapItem->name)) {
                PROFILER_LOG_INFO(LOG_CORE, "JIT: IsJitFile matched, creating JitFileSymbols for %s",
                    memMapItem->name.c_str());
            }
            virtualruntime_->UpdateSymbols(memMapItem->name, memMapItem);
        }
        prevMap = memMapItem;
    }

    // Find if there are duplicate mapping intervals, and if there are, overwrite the old data with the new data.
    for (auto tempMapIter = tempMap.begin(); tempMapIter != tempMap.end(); ++tempMapIter) {
        auto memMapIter = std::find_if(memMaps.begin(), memMaps.end(), [&](const std::shared_ptr<DfxMap>& map) {
            if ((*tempMapIter)->begin == map->begin && (*tempMapIter)->end == map->end) {
                return true;
            }
            return false;
        });
        if (memMapIter != memMaps.end()) {
            virtualruntime_->DelSymbolFile((*memMapIter)->name);
            memMaps.erase(memMapIter);
        }
    }
    memMaps.insert(memMaps.end(), tempMap.begin(), tempMap.end());

    if (mapsAdded) {
        PROFILER_LOG_DEBUG(LOG_CORE, "maps changed and need sort");
        SortMaps();
    } else {
        PROFILER_LOG_DEBUG(LOG_CORE, "maps no change");
        return false;
    }
    virtualruntime_->soBegin_ = 0;
    return true;
}

void VirtualThread::SortMaps()
{
    OHOS::Developtools::StackCommon::SortSharedMapsByEndAscending(*maps_);
}

void VirtualThread::CreateMapItem(const std::string filename, uint64_t begin, uint64_t len,
                                  uint64_t offset)
{
    std::string adaptedFileName = filename;
    if (isContainer_) {
        std::string containerRoot = StringPrintf("/proc/%d/root", pid_);
        if (!StringStartsWith(adaptedFileName, containerRoot)) {
            adaptedFileName = containerRoot + adaptedFileName;
        }
    } else {
        std::string processName = COMMON::GetBundleNameByPid(pid_);
        NeedAdaptHMBundlePath(adaptedFileName, processName);
    }
    if (!OHOS::HiviewDFX::DfxMaps::IsLegalMapItem(adaptedFileName)) {
        return; // skip some memmap
    }
    uint32_t prots =  PROT_EXEC;

    std::shared_ptr<DfxMap> map = std::make_shared<DfxMap>(begin, begin + len, offset, prots, adaptedFileName);
    maps_->emplace_back(map);
    std::string tempMapName{" "};
    virtualruntime_->FillMapsCache(tempMapName, map);
    SortMaps();
}
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
