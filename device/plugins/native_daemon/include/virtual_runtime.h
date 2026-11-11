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
#ifndef HIPERF_VIRTUAL_RUNTIME_H
#define HIPERF_VIRTUAL_RUNTIME_H
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <functional>
#include <map>
#if defined(is_ohos) && is_ohos
#include "call_stack.h"
#endif
#include "hashlistpp.h"
#include "perf_event_record.h"
#include "symbols_file.h"
#include "virtual_thread.h"
#include "native_hook_config.pb.h"
#include "smo_map_global.h"

namespace OHOS {
namespace Developtools {
namespace NativeDaemon {
/*
This Class contains userspace thread objects. and kernel space objects
It represents a virtual operating environment, mainly referring to the relationship between pid,
mmaps, and symbols.

It mainly receives data is ip pointer (virtual address), pid
According to these data, it will find the corresponding mmap and its corresponding elf (also called
DSO)

Then find the corresponding symbol in the corresponding elf symbol file according to the offset
recorded in the corresponding mmap.
*/

class VirtualRuntime {
public:
    VirtualRuntime() = default;
    VirtualRuntime(const NativeHookConfig& hookConfig);
    virtual ~VirtualRuntime();
    // thread need hook the record
    // from the record , it will call back to write some Simulated Record
    // case 1. some mmap will be create when it read mmaps for each new process (from record sample)

    // set symbols path , it will send to every symobile file for search
    bool SetSymbolsPaths(const std::vector<std::string> &symbolsPaths);

    // any mode
    static_assert(sizeof(pid_t) == sizeof(int));

    const std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> &GetSymbolsFiles() const
    {
        return symbolsFiles_;
    }

    void GetSymbol(DfxSymbol &symbol, CallFrame& callFrame, pid_t pid, pid_t tid,
        const perf_callchain_context &context = PERF_CONTEXT_MAX,
        std::shared_ptr<DfxMap> knownMap = nullptr, uint32_t knownFilePathId = 0,
        bool cacheChecked = false);

    VirtualThread &GetThread(pid_t pid, pid_t tid);
    const std::map<pid_t, VirtualThread> &GetThreads() const
    {
        return userSpaceThreadMap_;
    }

    bool UnwindStack(std::vector<u64>& regs,
                     const u8* stack_addr,
                     int stack_size,
                     pid_t pid,
                     pid_t tid,
                     std::vector<CallFrame>& callFrames,
                     size_t maxStackLevel);
    bool GetSymbolName(pid_t pid, pid_t tid, std::vector<CallFrame>& callFrames, int offset, bool first,
                       SymbolType type = SymbolType::NATIVE_SYMBOL);
    void ClearMaps();
    void FillMapsCache(std::string& currentFileName, std::shared_ptr<DfxMap> mapItem);
    void HandleMapInfo(std::vector<uint64_t> info, const std::string& filePath, pid_t pid, pid_t tid);
    void RemoveMaps(uint64_t addr);
      // threads
    VirtualThread &UpdateThread(pid_t pid, pid_t tid, const std::string name = "");
    void FillSymbolNameId(CallFrame& callFrame, DfxSymbol& symbol);
    void FillFileSet(CallFrame& callFrame, const DfxSymbol& symbol);
    uint32_t FillArkTsFilePath(std::string_view& jstr);
    uint32_t FindArkTsFilePath(std::string_view& jstr);

    // ArkTS related
    bool IsArktsMap(const std::shared_ptr<DfxMap>& map) const;
    std::shared_ptr<DfxMap> FindArktsMapByAddr(uint64_t ip) const;
    void MaskArktsFrame(CallFrame& callFrame);
    bool SymbolizeFrame(CallFrame& callFrame, pid_t pid, pid_t tid);
    
    bool ArktsGetSymbolCache(CallFrame& callFrame, DfxSymbol &symbol);
    uint32_t GetJsSymbolCacheSize();
    void FillJsSymbolCache(CallFrame& callFrame, const DfxSymbol& symbol);
    std::vector<uint64_t>& GetOfflineMaps()
    {
        return offlineMapAddr_;
    }

    void ClearOfflineMaps()
    {
        if (offlineMapAddr_.size() != 0) {
            offlineMapAddr_.clear();
        }
        offlineMapAddrSet_.clear();
    }

    std::map<uint64_t, std::shared_ptr<MemMaps>>& GetMapsCache()
    {
        return mapsCache_;
    }

    std::pair<std::shared_ptr<MemMaps>, uint32_t> FindMap(uint64_t addr);
    bool MarkOfflineFileReported(uint32_t filePathId);
    void ResetOfflineFileReported(uint32_t filePathId);
    uint64_t soBegin_ {0};
    // debug time
#ifdef HIPERF_DEBUG_TIME
    std::chrono::microseconds updateSymbolsTimes_ = std::chrono::microseconds::zero();
    std::chrono::microseconds unwindFromRecordTimes_ = std::chrono::microseconds::zero();
    std::chrono::microseconds unwindCallStackTimes_ = std::chrono::microseconds::zero();
    std::chrono::microseconds symbolicRecordTimes_ = std::chrono::microseconds::zero();
    std::chrono::microseconds updateThreadTimes_ = std::chrono::microseconds::zero();
#endif
    const bool loadSymboleWhenNeeded_ = true; // thie is a feature config
    void UpdateSymbols(std::string filename, std::shared_ptr<DfxMap> map);
    // we don't know whether hap vma mapping is stand for a so
    // thus we need try to parse it first
    bool UpdateHapSymbols(std::shared_ptr<DfxMap> map);
    bool IsSymbolExist(const std::string& fileName);
    void DelSymbolFile(const std::string& fileName);
    void UpdateMaps(pid_t pid, pid_t tid);
    std::string ResolveMergedSoIp(uint64_t ip, const std::string& filePath);
    void UpdateAdltBase();
    std::vector<std::shared_ptr<DfxMap>>& GetProcessMaps()
    {
        return processMaps_;
    }
    void SetContainerInfo(bool isContainer)
    {
        isContainer_ = isContainer;
    }
    bool IsContainer()
    {
        return isContainer_;
    }
    bool EnsureSmoInfoForFile(const std::string& path);
    std::shared_ptr<DfxMap> FindMapByPath(const std::string& path);
    uint64_t GetAdltBase(const std::string& path) const;
    template <typename T>
    void TryEmitSmoMapsInfo(T& stackData)
    {
        std::vector<std::pair<std::string, SMOMapsInfo>> pending;
        {
            std::unique_lock<std::shared_mutex> lk(g_smoMapsLock);

            for (auto& [path, entry] : g_smoMaps) {
                if (!entry.initialized || entry.sent) continue;
                pending.emplace_back(path, entry.info);
            }
        }
        for (auto& p : pending) {
            const auto& info = p.second;

            auto event = stackData.add_events();
            auto* smo = event->mutable_smo_map();

            smo->set_file_path(info.file_path());

            for (int i = 0; i < info.map_entries_size(); i++) {
                const auto& src = info.map_entries(i);
                auto* dst = smo->add_map_entries();

                dst->set_start_addr(src.start_addr());
                dst->set_end_addr(src.end_addr());
                dst->set_so_index(src.so_index());
            }

            for (int i = 0; i < info.id_tlb_entries_size(); i++) {
                const auto& src = info.id_tlb_entries(i);
                auto* dst = smo->add_id_tlb_entries();

                dst->set_so_index(src.so_index());
                dst->set_name(src.name());
            }
        }

        {
            std::unique_lock<std::shared_mutex> lk(g_smoMapsLock);

            for (auto& p : pending) {
                const std::string& path = p.first;
                auto it = g_smoMaps.find(path);
                if (it != g_smoMaps.end()) {
                    it->second.sent = true;
                }
            }
        }
    }

public:
    enum SymbolCacheLimit : std::size_t {
        USER_SYMBOL_CACHE_LIMIT = 10000,
    };

private:
    struct SymbolCacheKey : public std::pair<uint64_t, uint32_t> {
        explicit SymbolCacheKey() = default;
        SymbolCacheKey(const std::pair<uint64_t, uint32_t>& arg) : pair(arg) {}
        SymbolCacheKey(uint64_t ip, uint32_t filePathId) : pair(ip, filePathId) {}
    };

    // boost library recommendation algorithm to reduce hash collisions.
    struct HashPair {
        // 0x9e3779b9 is the golden ratio fractional part used by boost hash_combine.
        static constexpr size_t MAGIC_NUMBER = 0x9e3779b9;
        static constexpr size_t LEFT_SHIFT_BITS = 6;  // bit displacement in boost hash_combine
        static constexpr size_t RIGHT_SHIFT_BITS = 2; // bit displacement in boost hash_combine
        size_t operator() (const SymbolCacheKey& key) const
        {
            std::hash<uint64_t> hasher;
            size_t seed = 0;
            seed ^= hasher(key.first) + MAGIC_NUMBER + (seed << LEFT_SHIFT_BITS) + (seed >> RIGHT_SHIFT_BITS);
            seed ^= hasher(key.second) + MAGIC_NUMBER + (seed << LEFT_SHIFT_BITS) + (seed >> RIGHT_SHIFT_BITS);
            return seed;
        }
    };
#if defined(is_ohos) && is_ohos
    CallStack callstack_;
#endif
    // pid map with user space thread
    pid_t pid_ = 0;
    std::atomic_bool isContainer_{false};
    pthread_mutex_t threadMapsLock_;
    std::map<pid_t, VirtualThread> userSpaceThreadMap_;
    // not pid , just map
    std::vector<DfxMap> kernelSpaceMaps_;
    pthread_mutex_t processSymbolsFileLock_;
    std::unordered_set<uint32_t> fileSet_; // for mapItem filePathId_
    std::unordered_map<std::string, uint32_t> functionMap_;
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles_;
    std::unordered_map<SymbolCacheKey, DfxSymbol, HashPair> userSymbolCache_;

    bool GetSymbolCache(uint64_t ip, uint32_t filePathId, DfxSymbol &symbol);
    void UpdateSymbolCache(uint64_t ip, DfxSymbol &symbol, HashList<uint64_t, DfxSymbol> &cache);
    uint32_t GetOrCreateFilePathId(const std::string& filePath);
    void QueueOfflineMap(uint64_t soBegin);

    // find symbols function name
    void MakeCallFrame(const DfxSymbol &symbol, CallFrame &callFrame);
    void FillFpUnwindJsSymbol(CallFrame& callFrame, DfxSymbol &symbol);

    std::string ReadThreadName(pid_t tid);

    VirtualThread &CreateThread(pid_t pid, pid_t tid);

    const DfxSymbol GetKernelSymbol(uint64_t ip, const std::vector<std::shared_ptr<DfxMap>> &maps,
                                 const VirtualThread &thread);
    const DfxSymbol GetUserSymbol(uint64_t ip, std::shared_ptr<MemMaps> curMaps, uint32_t itemIndex,
        const VirtualThread &thread);
    const DfxSymbol GetUserSymbol(uint64_t ip, std::shared_ptr<DfxMap> map, uint32_t filePathId,
        const VirtualThread &thread);

    std::vector<std::string> symbolsPaths_;

    friend class VirtualRuntimeTest;
    friend class VirtualThread;
    std::vector<std::shared_ptr<DfxMap>> processMaps_;
    std::unordered_set<uint64_t> failedIPs_;
    const NativeHookConfig hookConfig_;
    uint32_t memMapFilePathId_ = 0;
    std::map<uint64_t, std::shared_ptr<MemMaps>> mapsCache_; // key is memMap soBegin, value is MemMaps
    struct ArktsMapEntry {
        std::shared_ptr<DfxMap> map;
        uint32_t filePathId = 0;
    };
    std::map<uint64_t, ArktsMapEntry> arktsMapTree_;
    mutable std::unordered_map<uint64_t, ArktsMapEntry> arktsMapCache_;
    ArktsMapEntry FindArktsMapEntryByAddr(uint64_t ip) const;
    std::vector<uint64_t> offlineMapAddr_; // element is memMap soBegin
    std::unordered_set<uint64_t> offlineMapAddrSet_;
    std::unordered_map<std::string, uint32_t> nativeFilePathIdMap_;
    std::unordered_set<uint32_t> reportedOfflineFileIds_;
    std::unordered_map<std::string_view, uint32_t> jsUrlMap_; // Key is js url , value is filePathId
    std::unordered_map<std::string, bool> adltFound_;
    std::unordered_map<std::string, uint64_t> adltBaseMap_;
};
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
#endif
