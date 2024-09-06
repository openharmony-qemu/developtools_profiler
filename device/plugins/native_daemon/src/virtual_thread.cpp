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
    if (maps_->front()->begin > addr) {
        return nullptr;
    }
    if (maps_->back()->end <= addr) {
        return nullptr;
    }
    constexpr int two {2};
    std::size_t left {0};
    std::size_t right {maps_->size()};
    std::size_t mid = (right - left) / two + left;
    while (left < right) {
        if (addr < (*maps_)[mid]->end) {
            right = mid;
            mid = (right - left) / two + left;
            continue;
        }
        if (addr >= (*maps_)[mid]->end) {
            left = mid + 1;
            mid = (right - left) / two + left;
            continue;
        }
    }
    if (addr >= (*maps_)[left]->begin and addr < (*maps_)[left]->end) {
        if (left > 0) {
            (*maps_)[left]->prevMap = (*maps_)[left - 1];
        }
        return (*maps_)[left];
    }
    return nullptr;
}
VirtualThread::VirtualThread(pid_t pid,
                             pid_t tid,
                             const std::unordered_map<std::string, std::unique_ptr<SymbolsFile>>& symbolsFiles,
                             VirtualRuntime* runtime,
                             bool parseFlag)
    : pid_(pid), tid_(tid), symbolsFiles_(symbolsFiles), virtualruntime_(runtime)
{
    maps_ = &virtualruntime_->processMaps_;
    if (parseFlag) {
        if (virtualruntime_->processMaps_.size() == 0) {
            this->ParseMap(virtualruntime_->processMaps_);
        }
    }

    this->name_ = ReadThreadName(pid);
    HLOGM("%d %d map from parent size is %zu", pid, tid, maps_->size());
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
    for (auto map : *maps_) {
        if (name != map->name) {
            continue;
        }
        // check begin and length
        if (offset >= map->offset && (offset - map->offset) < (map->end - map->begin)) {
            HLOGMMM("found fileoffset 0x%" PRIx64 " in map (0x%" PRIx64 " - 0x%" PRIx64
                    " pageoffset 0x%" PRIx64 ")  from %s",
                    offset, map->begin, map->end, map->offset, map->name.c_str());
            return map;
        }
    }
    HLOGM("NOT found offset 0x%" PRIx64 " in maps %zu ", offset, maps_->size());
    return nullptr;
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

void VirtualThread::ReportVaddrMapMiss(uint64_t vaddr) const
{
#ifdef HIPERF_DEBUG
    if (DebugLogger::GetInstance()->GetLogLevel() <= LEVEL_VERBOSE) {
        if (missedRuntimeVaddr_.find(vaddr) == missedRuntimeVaddr_.end()) {
            missedRuntimeVaddr_.insert(vaddr);
            HLOGV("vaddr %" PRIx64 " not found in any map", vaddr);
            for (auto &map : *maps_) {
                HLOGV("map %s ", map->ToString().c_str());
            }
        }
    }
#endif
}

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

bool VirtualThread::ParseMap(std::vector<std::shared_ptr<DfxMap>>& memMaps, bool update)
{
    std::string mapPath = StringPrintf("/proc/%d/maps", pid_);
    std::shared_ptr<DfxMaps> dfxMaps = OHOS::HiviewDFX::DfxMaps::Create(pid_, mapPath);
    if (dfxMaps == nullptr) {
        HLOGE("VirtualThread Failed to Parse Map.");
        return false;
    }
    memMaps = dfxMaps->GetMaps();
    bool mapsAdded = !update;
    std::vector<std::shared_ptr<DfxMap>> tempMap;
    std::string tempMapName;
    std::shared_ptr<DfxMap> prevMap = nullptr;
    for (auto memMapItem : memMaps) {
        if (!update) {
            virtualruntime_->FillMapsCache(tempMapName, memMapItem);
            bool updateNormalSymbol = true;
            if (memMapItem->name.find(".hap") != std::string::npos && (memMapItem->prots & PROT_EXEC)) {
                memMapItem->prevMap = prevMap;
                HLOGD("update hap(%s) symbols", memMapItem->name.c_str());
                updateNormalSymbol = !virtualruntime_->UpdateHapSymbols(memMapItem);
            }
            if (updateNormalSymbol) {
                virtualruntime_->UpdateSymbols(memMapItem->name, memMapItem);
            }
            prevMap = memMapItem;
        } else if (!virtualruntime_->IsSymbolExist(memMapItem->name)) {
            virtualruntime_->FillMapsCache(tempMapName, memMapItem);
            mapsAdded = true;
            tempMap.push_back(memMapItem);
            bool updateNormalSymbol = true;
            if (memMapItem->name.find(".hap") != std::string::npos && (memMapItem->prots & PROT_EXEC)) {
                memMapItem->prevMap = prevMap;
                HLOGD("update hap(%s) symbols", memMapItem->name.c_str());
                updateNormalSymbol = !virtualruntime_->UpdateHapSymbols(memMapItem);
            }
            if (updateNormalSymbol) {
                virtualruntime_->UpdateSymbols(memMapItem->name, memMapItem);
            }
            prevMap = memMapItem;
        }
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
    for (size_t currPos = 1; currPos < maps_->size(); ++currPos) {
        int targetPos = static_cast<int>(currPos - 1);
        while (targetPos >= 0 && (*maps_)[currPos]->end < (*maps_)[targetPos]->end) {
            --targetPos;
        }
        if (targetPos < static_cast<int>(currPos - 1)) {
            auto target = (*maps_)[currPos];
            for (size_t k = currPos - 1; k > static_cast<size_t>(targetPos); --k) {
                (*maps_)[k + 1] = (*maps_)[k];
            }
            (*maps_)[targetPos + 1] = target;
        }
    }
    return;
}

void VirtualThread::CreateMapItem(const std::string filename, uint64_t begin, uint64_t len,
                                  uint64_t offset)
{
    if (!OHOS::HiviewDFX::DfxMaps::IsLegalMapItem(filename)) {
        return; // skip some memmap
    }
    uint32_t prots =  PROT_EXEC;

    std::shared_ptr<DfxMap> map = std::make_shared<DfxMap>(begin, begin + len, offset, prots, filename);
    maps_->emplace_back(map);
    std::string tempMapName{" "};
    virtualruntime_->FillMapsCache(tempMapName, map);
    SortMaps();
}
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS