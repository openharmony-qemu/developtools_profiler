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
#include "call_stack.h"
#include "perf_event_record.h"
#include "symbols_file.h"
#include "virtual_thread.h"
#include "native_hook_config.pb.h"

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

    const Symbol GetSymbol(CallFrame& callFrame, pid_t pid, pid_t tid,
                           const perf_callchain_context &context = PERF_CONTEXT_MAX);

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
    bool GetSymbolName(pid_t pid, pid_t tid, std::vector<CallFrame>& callFrames, int offset, bool first);
    void ClearMaps();
    void FillMapsCache(std::string& currentFileName, MemMapItem& memMapItem);
    void HandleMapInfo(uint64_t begin, uint64_t length, uint32_t flags, uint64_t offset, const std::string& filePath);
    void RemoveMaps(uint64_t addr);
    std::vector<uint64_t>& GetOfflineMaps()
    {
        return offlineMapAddr_;
    }

    void ClearOfflineMaps()
    {
        offlineMapAddr_.clear();
    }

    std::map<uint64_t, MemMaps>& GetMapsCache()
    {
        return mapsCache_;
    }

    std::pair<MemMaps*, uint32_t> FindMap(uint64_t addr);
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
    void UpdateSymbols(std::string filename);
    bool IsSymbolExist(const std::string& fileName);
    void DelSymbolFile(const std::string& fileName);
    void UpdateMaps(pid_t pid, pid_t tid);
    std::vector<MemMapItem>& GetProcessMaps()
    {
        return processMemMaps_;
    }

public:
    enum SymbolCacheLimit : std::size_t {
        USER_SYMBOL_CACHE_LIMIT = 10000,
    };

private:
    struct SymbolCacheKey : public std::pair<uint64_t, uint32_t> {
        uint64_t& ip = first;
        uint32_t& filePathId = second;
        explicit SymbolCacheKey() = default;
        virtual ~SymbolCacheKey() = default;
        SymbolCacheKey(const SymbolCacheKey &) = default;
        SymbolCacheKey& operator=(const SymbolCacheKey& sym)
        {
            ip = sym.ip;
            filePathId = sym.filePathId;
            return *this;
        }
        SymbolCacheKey(const std::pair<uint64_t, uint32_t>& arg) : pair(arg), ip(first), filePathId(second) {}
        SymbolCacheKey(uint64_t ip, uint32_t filePathId) : pair(ip, filePathId), ip(first), filePathId(second) {}
    };

    // boost library recommendation algorithm to reduce hash collisions.
    struct HashPair {
        size_t operator() (const SymbolCacheKey& key) const
        {
            std::hash<uint64_t> hasher;
            size_t seed = 0;
            // 6 and 2 is the number of displacements
            seed ^= hasher(key.ip) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hasher(key.filePathId) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
    CallStack callstack_;
    // pid map with user space thread
    pthread_mutex_t threadMapsLock_;
    std::map<pid_t, VirtualThread> userSpaceThreadMap_;
    // not pid , just memmap
    std::vector<MemMapItem> kernelSpaceMemMaps_;
    pthread_mutex_t processSymbolsFileLock_;
    std::unordered_set<uint32_t> fileSet_; // for memMpaItem filePathId_
    std::unordered_map<std::string, uint32_t> functionMap_;
    std::unordered_map<std::string, std::unique_ptr<SymbolsFile>> symbolsFiles_;
    std::unordered_map<SymbolCacheKey, Symbol, HashPair> userSymbolCache_;
    bool GetSymbolCache(uint64_t ip, Symbol &symbol, const VirtualThread &thread);
    void UpdateSymbolCache(uint64_t ip, Symbol &symbol, HashList<uint64_t, Symbol> &cache);

    // find synbols function name
    void MakeCallFrame(Symbol &symbol, CallFrame &callFrame);

    // threads
    VirtualThread &UpdateThread(pid_t pid, pid_t tid, const std::string name = "");
    std::string ReadThreadName(pid_t tid);
    VirtualThread &CreateThread(pid_t pid, pid_t tid);

    const Symbol GetKernelSymbol(uint64_t ip, const std::vector<MemMapItem> &memMaps,
                                 const VirtualThread &thread);
    const Symbol GetUserSymbol(uint64_t ip, const VirtualThread &thread);
    void FillSymbolNameId(CallFrame& callFrame, Symbol& symbol);
    void FillFileSet(CallFrame& callFrame, const Symbol& symbol);

    std::vector<std::string> symbolsPaths_;

    friend class VirtualRuntimeTest;
    friend class VirtualThread;
    std::vector<MemMapItem> processMemMaps_;
    std::unordered_set<uint64_t> failedIPs_;
    const NativeHookConfig hookConfig_;
    uint32_t memMapFilePathId_ = 0;
    std::map<uint64_t, MemMaps> mapsCache_; // key is memMap soBegin, value is MemMaps
    std::vector<uint64_t> offlineMapAddr_; // element is memMap soBegin
};
} // namespace NativeDaemon
} // namespace Developtools
} // namespace OHOS
#endif