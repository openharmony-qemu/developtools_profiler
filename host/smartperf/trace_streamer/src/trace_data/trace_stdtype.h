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

#ifndef TRACE_STDTYPE_H
#define TRACE_STDTYPE_H

#include <array>
#include <deque>
#include <limits>
#include <map>
#include <mutex>
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "cfg/trace_streamer_config.h"
#include "double_map.h"
#include "log.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStdtype {
using namespace SysTuning::TraceCfg;
using namespace SysTuning::TraceStreamer;
constexpr uint32_t ONE_MILLION_NANOSECONDS = 1000000;
constexpr uint32_t BILLION_NANOSECONDS = 1000000000;
constexpr uint8_t DYNAMICFRAME_MATCH_LAST = 5;
class CacheBase {
public:
    size_t Size() const
    {
        return std::max(timeStamps_.size(), ids_.size());
    }
    const std::deque<uint64_t>& IdsData() const
    {
        return ids_;
    }
    const std::deque<uint64_t>& TimeStampData() const
    {
        return timeStamps_;
    }
    const std::deque<InternalTid>& InternalTidsData() const
    {
        return internalTids_;
    }
    virtual void Clear()
    {
        internalTids_.clear();
        timeStamps_.clear();
        ids_.clear();
    }

public:
    std::deque<InternalTid> internalTids_ = {};
    std::deque<uint64_t> timeStamps_ = {};
    std::deque<uint64_t> ids_ = {};
};

class CpuCacheBase {
public:
    const std::deque<uint64_t>& DursData() const
    {
        return durs_;
    }

    const std::deque<uint32_t>& CpusData() const
    {
        return cpus_;
    }
    virtual void Clear()
    {
        durs_.clear();
        cpus_.clear();
    }
    void SetDur(uint64_t index, uint64_t dur);

public:
    std::deque<uint64_t> durs_;
    std::deque<uint32_t> cpus_;
};
class Thread {
public:
    explicit Thread(uint32_t t) : tid_(t) {}
    InternalPid internalPid_ = INVALID_UINT32;
    uint32_t tid_ = 0;
    DataIndex nameIndex_ = 0;
    InternalTime startT_ = 0;
    InternalTime endT_ = 0;
    uint32_t switchCount_ = 0;
    uint32_t sliceSize_ = 0;
    uint32_t cpuStatesCount_ = 0;
};

class Process {
public:
    explicit Process(uint32_t p) : pid_(p) {}
    std::string cmdLine_ = "";
    InternalTime startT_ = 0;
    uint32_t pid_ = 0;
    uint32_t memSize_ = 0;
    uint32_t threadSize_ = 0;
    uint32_t sliceSize_ = 0;
    uint32_t switchCount_ = 0;
    int32_t threadCount_ = -1;
    uint32_t cpuStatesCount_ = 0;
};

class ThreadStateData {
public:
    TableRowId
        AppendThreadState(InternalTime ts, InternalTime dur, InternalCpu cpu, InternalTid itid, TableRowId idState);
    void SetDuration(TableRowId index, InternalTime dur);
    TableRowId UpdateDuration(TableRowId index, InternalTime ts);
    bool End(TableRowId index, InternalTime ts);
    void UpdateState(TableRowId index, TableRowId idState);
    void SetArgSetId(TableRowId index, uint32_t setId);
    void UpdateDuration(TableRowId index, InternalTime ts, TableRowId idState);
    void UpdateTidAndPid(TableRowId index, InternalTid tid, InternalTid pid);
    TableRowId UpdateDuration(TableRowId index, InternalTime ts, InternalCpu cpu, TableRowId idState);
    void Clear()
    {
        timeStamps_.clear();
        durations_.clear();
        itids_.clear();
        tids_.clear();
        pids_.clear();
        states_.clear();
        cpus_.clear();
    }
    uint32_t Size() const
    {
        return itids_.size();
    }

    const std::deque<InternalTime>& TimeStamsData() const
    {
        return timeStamps_;
    }
    const std::deque<InternalTime>& DursData() const
    {
        return durations_;
    }
    const std::deque<InternalTid>& ItidsData() const
    {
        return itids_;
    }
    const std::deque<InternalTid>& TidsData() const
    {
        return tids_;
    }
    const std::deque<InternalPid>& PidsData() const
    {
        return pids_;
    }
    const std::deque<DataIndex>& StatesData() const
    {
        return states_;
    }
    const std::deque<InternalCpu>& CpusData() const
    {
        return cpus_;
    }
    const std::deque<uint32_t>& ArgSetsData() const
    {
        return argSetIds_;
    }

private:
    std::deque<InternalTime> timeStamps_;
    std::deque<InternalTime> durations_;
    std::deque<InternalTid> itids_;
    std::deque<InternalTid> tids_;
    std::deque<InternalPid> pids_;
    std::deque<DataIndex> states_;
    std::deque<InternalCpu> cpus_;
    std::deque<uint32_t> argSetIds_;
};

class SchedSlice : public CacheBase, public CpuCacheBase {
public:
    size_t AppendSchedSlice(uint64_t ts,
                            uint64_t dur,
                            uint64_t cpu,
                            uint32_t internalTid,
                            uint64_t endState,
                            uint64_t priority);
    void SetDuration(size_t index, uint64_t duration);
    void Update(uint64_t index, uint64_t ts, uint64_t state);
    void UpdateEndState(uint64_t index, uint64_t state);
    void UpdateArg(uint64_t index, uint32_t argsetId);

    const std::deque<uint64_t>& EndStatesData() const
    {
        return endStates_;
    }

    const std::deque<uint64_t>& PriorityData() const
    {
        return priority_;
    }

    const std::deque<uint32_t>& ArgSetData() const
    {
        return argSets_;
    }
    const std::deque<uint64_t>& TsEndData() const
    {
        return tsEnds_;
    }
    const std::deque<InternalPid>& InternalPidsData() const
    {
        return internalPids_;
    }
    void AppendInternalPid(InternalPid ipid)
    {
        internalPids_.emplace_back(ipid);
    }
    void Clear() override
    {
        CacheBase::Clear();
        CpuCacheBase::Clear();
        endStates_.clear();
        priority_.clear();
        internalPids_.clear();
        tsEnds_.clear();
    }

private:
    std::deque<InternalPid> internalPids_ = {};
    std::deque<uint64_t> tsEnds_ = {};
    std::deque<uint64_t> endStates_ = {};
    std::deque<uint64_t> priority_ = {};
    std::deque<uint32_t> argSets_ = {};
};

class CallStack : public CacheBase, public CpuCacheBase {
public:
    size_t AppendInternalAsyncSlice(uint64_t startT,
                                    uint64_t durationNs,
                                    InternalTid internalTid,
                                    DataIndex cat,
                                    uint16_t nameIdentify,
                                    DataIndex name,
                                    uint8_t depth,
                                    uint64_t cookid,
                                    const std::optional<uint64_t>& parentId);
    size_t AppendInternalSlice(uint64_t startT,
                               uint64_t durationNs,
                               InternalTid internalTid,
                               DataIndex cat,
                               uint16_t nameIdentify,
                               DataIndex name,
                               uint8_t depth,
                               const std::optional<uint64_t>& parentId);
    void SetDistributeInfo(size_t index,
                           const std::string& chainId,
                           const std::string& spanId,
                           const std::string& parentSpanId,
                           const std::string& flag,
                           const std::string& args);
    void AppendDistributeInfo(const std::string& chainId,
                              const std::string& spanId,
                              const std::string& parentSpanId,
                              const std::string& flag,
                              const std::string& args);
    void AppendDistributeInfo();
    void SetDuration(size_t index, uint64_t timeStamp);
    void SetDurationWithFlag(size_t index, uint64_t timeStamp);
    void SetFlag(size_t index, uint8_t flag);
    void SetDurationEx(size_t index, uint32_t dur);
    void SetIrqDurAndArg(size_t index, uint64_t timeStamp, uint32_t argSetId);
    void SetTimeStamp(size_t index, uint64_t timeStamp);
    void SetDepth(size_t index, uint8_t depth);
    void SetArgSetId(size_t index, uint32_t argSetId);
    void Clear() override
    {
        CacheBase::Clear();
        CpuCacheBase::Clear();
        cats_.clear();
        cookies_.clear();
        callIds_.clear();
        names_.clear();
        depths_.clear();
        chainIds_.clear();
        spanIds_.clear();
        parentSpanIds_.clear();
        flags_.clear();
        args_.clear();
        argSet_.clear();
    }

    const std::deque<std::optional<uint64_t>>& ParentIdData() const;
    const std::deque<DataIndex>& CatsData() const;
    const std::deque<DataIndex>& NamesData() const;
    const std::deque<uint8_t>& Depths() const;
    const std::deque<uint64_t>& Cookies() const;
    const std::deque<uint32_t>& CallIds() const;
    const std::deque<uint16_t>& IdentifysData() const;
    const std::deque<std::string>& ChainIds() const;
    const std::deque<std::string>& SpanIds() const;
    const std::deque<std::string>& ParentSpanIds() const;
    const std::deque<std::string>& Flags() const;
    const std::deque<std::string>& ArgsData() const;
    const std::deque<uint32_t>& ArgSetIdsData() const;

private:
    void AppendCommonInfo(uint64_t startT, uint64_t durationNs, InternalTid internalTid);
    void AppendCallStack(DataIndex cat, DataIndex name, uint8_t depth, std::optional<uint64_t> parentId);

private:
    std::deque<std::optional<uint64_t>> parentIds_;
    std::deque<DataIndex> cats_ = {};
    std::deque<uint64_t> cookies_ = {};
    std::deque<uint32_t> callIds_ = {};
    std::deque<uint16_t> identifys_ = {};
    std::deque<DataIndex> names_ = {};
    std::deque<uint8_t> depths_ = {};

    std::deque<std::string> chainIds_ = {};
    std::deque<std::string> spanIds_ = {};
    std::deque<std::string> parentSpanIds_ = {};
    std::deque<std::string> flags_ = {};
    std::deque<std::string> args_ = {};
    std::deque<uint32_t> argSet_ = {};
};

class Filter : public CacheBase {
public:
    size_t AppendNewFilterData(std::string type, std::string name, uint64_t sourceArgSetId);
    const std::deque<std::string>& NameData() const
    {
        return nameDeque_;
    }
    const std::deque<std::string>& TypeData() const
    {
        return typeDeque_;
    }
    const std::deque<uint64_t>& SourceArgSetIdData() const
    {
        return sourceArgSetId_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        nameDeque_.clear();
        typeDeque_.clear();
        sourceArgSetId_.clear();
    }

private:
    std::deque<std::string> nameDeque_ = {};
    std::deque<std::string> typeDeque_ = {};
    std::deque<uint64_t> sourceArgSetId_ = {};
};

class Measure : public CacheBase {
public:
    size_t AppendMeasureData(uint32_t type, uint64_t timeStamp, int64_t value, uint32_t filterId);
    const std::deque<uint32_t>& TypeData() const
    {
        return typeDeque_;
    }
    const std::deque<int64_t>& ValuesData() const
    {
        return valuesDeque_;
    }
    const std::deque<uint64_t>& DursData() const
    {
        return durDeque_;
    }
    void SetDur(uint32_t row, uint64_t timeStamp);
    const std::deque<uint32_t>& FilterIdData() const
    {
        return filterIdDeque_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        typeDeque_.clear();
        durDeque_.clear();
        valuesDeque_.clear();
        filterIdDeque_.clear();
    }

private:
    std::deque<uint32_t> typeDeque_ = {};
    std::deque<uint64_t> durDeque_ = {};
    std::deque<int64_t> valuesDeque_ = {};
    std::deque<uint32_t> filterIdDeque_ = {};
};

class Raw : public CacheBase {
public:
    size_t AppendRawData(uint32_t id, uint64_t timeStamp, uint32_t name, uint32_t cpu, uint32_t internalTid);
    const std::deque<uint32_t>& NameData() const
    {
        return nameDeque_;
    }
    const std::deque<uint32_t>& CpuData() const
    {
        return cpuDeque_;
    }
    const std::deque<uint32_t>& InternalTidData() const
    {
        return itidDeque_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        nameDeque_.clear();
        cpuDeque_.clear();
        itidDeque_.clear();
    }

private:
    std::deque<uint32_t> nameDeque_ = {};
    std::deque<uint32_t> cpuDeque_ = {};
    std::deque<uint32_t> itidDeque_ = {};
};

class ThreadMeasureFilter {
public:
    size_t AppendNewFilter(uint64_t filterId, uint32_t nameIndex, uint64_t internalTid);
    size_t Size() const
    {
        return filterId_.size();
    }
    const std::deque<uint64_t>& FilterIdData() const
    {
        return filterId_;
    }
    const std::deque<uint64_t>& InternalTidData() const
    {
        return internalTids_;
    }
    const std::deque<uint32_t>& NameIndexData() const
    {
        return nameIndex_;
    }
    void Clear()
    {
        filterId_.clear();
        internalTids_.clear();
        nameIndex_.clear();
    }

private:
    std::deque<uint64_t> filterId_ = {};
    std::deque<uint64_t> internalTids_ = {};
    std::deque<uint32_t> nameIndex_ = {};
};

class CpuMeasureFilter : public CacheBase {
public:
    inline size_t AppendNewFilter(uint64_t filterId, DataIndex name, uint32_t cpu)
    {
        ids_.emplace_back(filterId);
        cpu_.emplace_back(cpu);
        name_.emplace_back(name);
        return Size() - 1;
    }

    const std::deque<uint32_t>& CpuData() const
    {
        return cpu_;
    }

    const std::deque<DataIndex>& TypeData() const
    {
        return type_;
    }

    const std::deque<DataIndex>& NameData() const
    {
        return name_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        cpu_.clear();
        type_.clear();
        name_.clear();
    }

private:
    std::deque<uint32_t> cpu_ = {};
    std::deque<DataIndex> type_ = {};
    std::deque<DataIndex> name_ = {};
};

class Instants : public CacheBase {
public:
    size_t AppendInstantEventData(uint64_t timeStamp,
                                  DataIndex nameIndex,
                                  int64_t internalTid,
                                  int64_t wakeupFromInternalPid);

    const std::deque<DataIndex>& NameIndexsData() const
    {
        return NameIndexs_;
    }
    const std::deque<int64_t>& WakeupFromPidsData() const
    {
        return wakeupFromInternalPids_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        NameIndexs_.clear();
        wakeupFromInternalPids_.clear();
    }

private:
    std::deque<DataIndex> NameIndexs_;
    std::deque<int64_t> wakeupFromInternalPids_;
};

class ProcessMeasureFilter : public CacheBase {
public:
    size_t AppendNewFilter(uint64_t id, DataIndex name, uint32_t internalPid);

    const std::deque<uint32_t>& UpidsData() const
    {
        return internalPids_;
    }

    const std::deque<DataIndex>& NamesData() const
    {
        return names_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        internalPids_.clear();
        names_.clear();
    }

private:
    std::deque<uint32_t> internalPids_ = {};
    std::deque<DataIndex> names_ = {};
};
class ClockEventData : public CacheBase {
public:
    size_t AppendNewFilter(uint64_t id, DataIndex type, DataIndex name, uint64_t cpu);

    const std::deque<uint64_t>& CpusData() const
    {
        return cpus_;
    }

    const std::deque<DataIndex>& NamesData() const
    {
        return names_;
    }
    const std::deque<DataIndex>& TypesData() const
    {
        return types_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        cpus_.clear();
        names_.clear();
        types_.clear();
    }

private:
    std::deque<uint64_t> cpus_ = {}; // in clock_set_rate event, it save cpu
    std::deque<DataIndex> names_ = {};
    std::deque<DataIndex> types_ = {};
};
class ClkEventData : public CacheBase {
public:
    size_t AppendNewFilter(uint64_t id, uint64_t rate, DataIndex name, uint64_t cpu);

    const std::deque<DataIndex>& NamesData() const
    {
        return names_;
    }
    const std::deque<uint64_t>& RatesData() const
    {
        return rates_;
    }
    const std::deque<uint64_t>& CpusData() const
    {
        return cpus_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        names_.clear();
        rates_.clear();
        cpus_.clear();
    }

private:
    std::deque<DataIndex> names_;
    std::deque<uint64_t> rates_;
    std::deque<uint64_t> cpus_;
};
class SysCall : public CacheBase {
public:
    size_t AppendSysCallData(int64_t sysCallNum, DataIndex type, uint32_t ipid, uint64_t timeStamp, int64_t ret);
    const std::deque<int64_t>& SysCallsData() const
    {
        return sysCallNums_;
    }
    const std::deque<DataIndex>& TypesData() const
    {
        return types_;
    }
    const std::deque<uint32_t>& IpidsData() const
    {
        return ipids_;
    }
    const std::deque<uint64_t>& RetsData() const
    {
        return rets_;
    }
    void Clear() override
    {
        CacheBase::Clear();
        sysCallNums_.clear();
        types_.clear();
        ipids_.clear();
        rets_.clear();
    }

private:
    std::deque<int64_t> sysCallNums_ = {};
    std::deque<DataIndex> types_ = {};
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint64_t> rets_ = {};
};
class ArgSet : public CacheBase {
public:
    size_t AppendNewArg(DataIndex nameId, BaseDataType dataType, int64_t value, size_t argSet);
    const std::deque<BaseDataType>& DataTypes() const;
    const std::deque<int64_t>& ValuesData() const;
    const std::deque<uint64_t>& ArgsData() const;
    const std::deque<DataIndex>& NamesData() const;

    void Clear() override
    {
        CacheBase::Clear();
        names_.clear();
        dataTypes_.clear();
        values_.clear();
        argset_.clear();
    }

private:
    std::deque<uint64_t> names_ = {};
    std::deque<BaseDataType> dataTypes_ = {};
    std::deque<int64_t> values_ = {};
    std::deque<uint64_t> argset_ = {};
};
class SysMeasureFilter : public CacheBase {
public:
    size_t AppendNewFilter(uint64_t filterId, DataIndex type, DataIndex nameId);
    const std::deque<DataIndex>& NamesData() const;
    const std::deque<DataIndex>& TypesData() const;
    void Clear() override
    {
        CacheBase::Clear();
        types_.clear();
        names_.clear();
    }

private:
    std::deque<DataIndex> types_ = {};
    std::deque<DataIndex> names_ = {};
};
class DataType : public CacheBase {
public:
    size_t AppendNewDataType(BaseDataType dataType, DataIndex dataDescIndex);
    const std::deque<BaseDataType>& DataTypes() const;
    const std::deque<DataIndex>& DataDesc() const;
    void Clear() override
    {
        CacheBase::Clear();
        dataTypes_.clear();
        descs_.clear();
    }

private:
    std::deque<BaseDataType> dataTypes_ = {};
    std::deque<DataIndex> descs_ = {};
};
class LogInfo : public CacheBase {
public:
    size_t AppendNewLogInfo(uint64_t seq,
                            uint64_t timeStamp,
                            uint32_t pid,
                            uint32_t tid,
                            DataIndex level,
                            DataIndex tag,
                            DataIndex context,
                            uint64_t originTs);
    const std::deque<uint64_t>& HilogLineSeqs() const;
    const std::deque<uint32_t>& Pids() const;
    const std::deque<uint32_t>& Tids() const;
    const std::deque<DataIndex>& Levels() const;
    const std::deque<DataIndex>& Tags() const;
    const std::deque<DataIndex>& Contexts() const;
    const std::deque<uint64_t>& OriginTimeStamData() const;
    void Clear() override
    {
        CacheBase::Clear();
        hilogLineSeqs_.clear();
        pids_.clear();
        levels_.clear();
        tags_.clear();
        contexts_.clear();
        originTs_.clear();
    }

private:
    std::deque<uint64_t> hilogLineSeqs_ = {};
    std::deque<uint32_t> pids_ = {};
    std::deque<uint32_t> tids_ = {};
    std::deque<DataIndex> levels_ = {};
    std::deque<DataIndex> tags_ = {};
    std::deque<DataIndex> contexts_ = {};
    std::deque<uint64_t> originTs_ = {};
};

class NativeHook : public CacheBase {
public:
    size_t AppendNewNativeHookData(uint32_t callChainId,
                                   uint32_t ipid,
                                   uint32_t itid,
                                   std::string eventType,
                                   DataIndex subType,
                                   uint64_t timeStamp,
                                   uint64_t endTimeStamp,
                                   uint64_t duration,
                                   uint64_t addr,
                                   int64_t memSize);
    void UpdateCallChainId(size_t row, uint32_t callChainId);
    void UpdateEndTimeStampAndDuration(size_t row, uint64_t endTimeStamp);
    void UpdateCurrentSizeDur(size_t row, uint64_t timeStamp);
    void UpdateMemMapSubType(uint64_t row, uint64_t tagId);
    void UpdateLastCallerPathIndexs(std::unordered_map<uint32_t, uint64_t>& callIdToLasLibId);
    const std::deque<uint32_t>& CallChainIds() const;
    const std::deque<uint32_t>& Ipids() const;
    const std::deque<uint32_t>& Itids() const;
    const std::deque<std::string>& EventTypes() const;
    const std::deque<DataIndex>& SubTypes() const;
    const std::deque<uint64_t>& EndTimeStamps() const;
    const std::deque<uint64_t>& Durations() const;
    const std::deque<uint64_t>& Addrs() const;
    const std::deque<int64_t>& MemSizes() const;
    const std::deque<int64_t>& AllMemSizes() const;
    const std::deque<uint64_t>& CurrentSizeDurs() const;
    const std::deque<uint64_t>& LastCallerPathIndexs() const;
    void Clear() override
    {
        CacheBase::Clear();
        callChainIds_.clear();
        ipids_.clear();
        itids_.clear();
        eventTypes_.clear();
        subTypes_.clear();
        endTimeStamps_.clear();
        durations_.clear();
        addrs_.clear();
        memSizes_.clear();
        allMemSizes_.clear();
        currentSizeDurs_.clear();
        lastCallerPathIndexs_.clear();
    }

private:
    std::deque<uint32_t> callChainIds_ = {};
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint32_t> itids_ = {};
    std::deque<std::string> eventTypes_ = {};
    std::deque<DataIndex> subTypes_ = {};
    std::deque<uint64_t> endTimeStamps_ = {};
    std::deque<uint64_t> durations_ = {};
    std::deque<uint64_t> addrs_ = {};
    std::deque<int64_t> memSizes_ = {};
    std::deque<int64_t> allMemSizes_ = {};
    std::deque<uint64_t> currentSizeDurs_ = {};
    std::deque<uint64_t> lastCallerPathIndexs_ = {};
    int64_t countHeapSizes_ = 0;
    int64_t countMmapSizes_ = 0;
    const std::string ALLOC_EVET = "AllocEvent";
    const std::string FREE_EVENT = "FreeEvent";
    const std::string MMAP_EVENT = "MmapEvent";
    const std::string MUNMAP_EVENT = "MunmapEvent";
};

class NativeHookFrame {
public:
    size_t AppendNewNativeHookFrame(uint32_t callChainId,
                                    uint64_t depth,
                                    uint64_t ip,
                                    uint64_t sp,
                                    DataIndex symbolName,
                                    DataIndex filePath,
                                    uint64_t offset,
                                    uint64_t symbolOffset);
    size_t AppendNewNativeHookFrame(uint32_t callChainId,
                                    uint64_t depth,
                                    uint64_t ip,
                                    uint64_t sp,
                                    DataIndex symbolName,
                                    DataIndex filePath,
                                    uint64_t offset,
                                    uint64_t symbolOffset,
                                    const std::string& vaddr);
    void UpdateFrameInfo(size_t row,
                         DataIndex symbolIndex,
                         DataIndex filePathIndex,
                         uint64_t offset,
                         uint64_t symbolOffset);
    void UpdateSymbolIdToNameMap(uint64_t originSymbolId, uint64_t symbolId);
    void UpdateSymbolId();
    void UpdateSymbolId(size_t index, DataIndex symbolId);
    void UpdateFileId(std::map<uint32_t, uint64_t>& filePathIdToFilePathName);
    void UpdateVaddrs(std::deque<std::string>& vaddrs);
    const std::deque<uint32_t>& CallChainIds() const;
    const std::deque<uint64_t>& Depths() const;
    const std::deque<uint64_t>& Ips() const;
    const std::deque<uint64_t>& Sps() const;
    const std::deque<DataIndex>& SymbolNames() const;
    const std::deque<DataIndex>& FilePaths() const;
    const std::deque<uint64_t>& Offsets() const;
    const std::deque<uint64_t>& SymbolOffsets() const;
    const std::deque<std::string>& Vaddrs() const;
    size_t Size() const
    {
        return callChainIds_.size();
    }
    void Clear()
    {
        callChainIds_.clear();
        depths_.clear();
        ips_.clear();
        sps_.clear();
        symbolNames_.clear();
        filePaths_.clear();
        offsets_.clear();
        symbolOffsets_.clear();
        vaddrs_.clear();
    }

private:
    std::deque<uint32_t> callChainIds_ = {};
    std::deque<uint64_t> depths_ = {};
    std::deque<uint64_t> ips_ = {};
    std::deque<uint64_t> sps_ = {};
    std::deque<DataIndex> symbolNames_ = {};
    std::deque<DataIndex> filePaths_ = {};
    std::deque<uint64_t> offsets_ = {};
    std::deque<uint64_t> symbolOffsets_ = {};
    std::deque<std::string> vaddrs_ = {};
    std::map<uint32_t, uint64_t> symbolIdToSymbolName_ = {};
};

class NativeHookStatistic : public CacheBase {
public:
    size_t AppendNewNativeHookStatistic(uint32_t ipid,
                                        uint64_t timeStamp,
                                        uint32_t callChainId,
                                        uint32_t memoryType,
                                        DataIndex subMemType,
                                        uint64_t applyCount,
                                        uint64_t releaseCount,
                                        uint64_t applySize,
                                        uint64_t releaseSize);

    const std::deque<uint32_t>& Ipids() const;
    const std::deque<uint32_t>& CallChainIds() const;
    const std::deque<uint32_t>& MemoryTypes() const;
    const std::deque<DataIndex>& MemorySubTypes() const;
    const std::deque<uint64_t>& ApplyCounts() const;
    const std::deque<uint64_t>& ReleaseCounts() const;
    const std::deque<uint64_t>& ApplySizes() const;
    const std::deque<uint64_t>& ReleaseSizes() const;
    void Clear() override
    {
        CacheBase::Clear();
        ids_.clear();
        ipids_.clear();
        callChainIds_.clear();
        timeStamps_.clear();
        memoryTypes_.clear();
        applyCounts_.clear();
        releaseCounts_.clear();
        applySizes_.clear();
        releaseSizes_.clear();
    }

private:
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint32_t> callChainIds_ = {};
    std::deque<uint32_t> memoryTypes_ = {};
    std::deque<DataIndex> memSubTypes_ = {};
    std::deque<uint64_t> applyCounts_ = {};
    std::deque<uint64_t> releaseCounts_ = {};
    std::deque<uint64_t> applySizes_ = {};
    std::deque<uint64_t> releaseSizes_ = {};
};

class Hidump : public CacheBase {
public:
    size_t AppendNewHidumpInfo(uint64_t timeStamp, uint32_t fps);
    const std::deque<uint32_t>& Fpss() const;

private:
    std::deque<uint32_t> fpss_ = {};
};

class PerfCallChain : public CacheBase {
public:
    size_t AppendNewPerfCallChain(uint64_t sampleId,
                                  uint32_t callChainId,
                                  uint64_t vaddrInFile,
                                  uint64_t fileId,
                                  uint64_t symbolId);
    const std::deque<uint64_t>& SampleIds() const;
    const std::deque<uint32_t>& CallChainIds() const;
    const std::deque<uint64_t>& VaddrInFiles() const;
    const std::deque<uint64_t>& FileIds() const;
    const std::deque<uint64_t>& SymbolIds() const;
    const std::deque<std::string>& Names() const;
    void SetName(uint64_t index, const std::string& name);
    void Clear() override;

private:
    std::deque<uint64_t> sampleIds_ = {};
    std::deque<uint32_t> callChainIds_ = {};
    std::deque<uint64_t> vaddrInFiles_ = {};
    std::deque<uint64_t> fileIds_ = {};
    std::deque<uint64_t> symbolIds_ = {};
    std::deque<std::string> names_ = {};
};

class PerfFiles : public CacheBase {
public:
    size_t AppendNewPerfFiles(uint64_t fileIds, uint32_t serial, DataIndex symbols, DataIndex filePath);
    const std::deque<uint64_t>& FileIds() const;
    const std::deque<DataIndex>& Symbols() const;
    const std::deque<DataIndex>& FilePaths() const;
    const std::deque<uint32_t>& Serials() const;
    void Clear() override;

private:
    std::deque<uint64_t> fileIds_ = {};
    std::deque<uint32_t> serials_ = {};
    std::deque<DataIndex> symbols_ = {};
    std::deque<DataIndex> filePaths_ = {};
};

class PerfSample : public CacheBase {
public:
    size_t AppendNewPerfSample(uint32_t sampleId,
                               uint64_t timeStamp,
                               uint32_t tid,
                               uint64_t eventCount,
                               uint64_t eventTypeId,
                               uint64_t timestampTrace,
                               uint64_t cpuId,
                               uint64_t threadState);
    const std::deque<uint32_t>& SampleIds() const;
    const std::deque<uint32_t>& Tids() const;
    const std::deque<uint64_t>& EventCounts() const;
    const std::deque<uint64_t>& EventTypeIds() const;
    const std::deque<uint64_t>& TimestampTraces() const;
    const std::deque<uint64_t>& CpuIds() const;
    const std::deque<DataIndex>& ThreadStates() const;
    void Clear() override;

private:
    std::deque<uint32_t> sampleIds_ = {};
    std::deque<uint32_t> tids_ = {};
    std::deque<uint64_t> eventCounts_ = {};
    std::deque<uint64_t> eventTypeIds_ = {};
    std::deque<uint64_t> timestampTraces_ = {};
    std::deque<uint64_t> cpuIds_ = {};
    std::deque<DataIndex> threadStates_ = {};
};

class PerfThread : public CacheBase {
public:
    size_t AppendNewPerfThread(uint32_t pid, uint32_t tid, DataIndex threadName);
    const std::deque<uint32_t>& Pids() const;
    const std::deque<uint32_t>& Tids() const;
    const std::deque<DataIndex>& ThreadNames() const;
    void Clear() override;

private:
    std::deque<uint32_t> tids_ = {};
    std::deque<uint32_t> pids_ = {};
    std::deque<DataIndex> threadNames_ = {};
};

class PerfReport : public CacheBase {
public:
    size_t AppendNewPerfReport(DataIndex type, DataIndex value);
    const std::deque<DataIndex>& Types() const;
    const std::deque<DataIndex>& Values() const;

private:
    std::deque<DataIndex> types_ = {};
    std::deque<DataIndex> values_ = {};
};
class StatAndInfo {
public:
    StatAndInfo();
    ~StatAndInfo() = default;
    void IncreaseStat(SupportedTraceEventType eventType, StatType type);
    const uint32_t& GetValue(SupportedTraceEventType eventType, StatType type) const;
    const std::string& GetEvent(SupportedTraceEventType eventType) const;
    const std::string& GetStat(StatType type) const;
    const std::string& GetSeverityDesc(SupportedTraceEventType eventType, StatType type) const;
    const StatSeverityLevel& GetSeverity(SupportedTraceEventType eventType, StatType type) const;
    std::map<BuiltinClocks, std::string> clockid2ClockNameMap_ = {};

private:
    uint32_t statCount_[TRACE_EVENT_MAX][STAT_EVENT_MAX];
    std::string event_[TRACE_EVENT_MAX];
    std::string stat_[STAT_EVENT_MAX];
    std::string statSeverityDesc_[TRACE_EVENT_MAX][STAT_EVENT_MAX];
    StatSeverityLevel statSeverity_[TRACE_EVENT_MAX][STAT_EVENT_MAX];
    TraceStreamerConfig config_{};
};
class SymbolsData {
public:
    SymbolsData() = default;
    ~SymbolsData() = default;
    uint64_t Size() const;
    void InsertSymbol(const DataIndex& name, const uint64_t& addr);
    const std::deque<DataIndex>& GetConstFuncNames() const;
    const std::deque<uint64_t>& GetConstAddrs() const;
    void Clear()
    {
        addrs_.clear();
        funcName_.clear();
    }

private:
    std::deque<uint64_t> addrs_ = {};
    std::deque<DataIndex> funcName_ = {};
};
class DiskIOData : public CacheBase {
public:
    DiskIOData() = default;
    ~DiskIOData() = default;
    void AppendNewData(uint64_t ts,
                       uint64_t dur,
                       uint64_t rd,
                       uint64_t wr,
                       uint64_t rdPerSec,
                       uint64_t wrPerSec,
                       double rdCountPerSec,
                       double wrCountPerSec,
                       uint64_t rdCount,
                       uint64_t wrCount);
    const std::deque<uint64_t>& Durs() const;
    const std::deque<uint64_t>& RdDatas() const;
    const std::deque<uint64_t>& WrDatas() const;
    const std::deque<double>& RdSpeedDatas() const;
    const std::deque<double>& WrSpeedDatas() const;
    const std::deque<double>& RdCountPerSecDatas() const;
    const std::deque<double>& WrCountPerSecDatas() const;
    const std::deque<uint64_t>& RdCountDatas() const;
    const std::deque<uint64_t>& WrCountDatas() const;

private:
    std::deque<uint64_t> durs_ = {};
    std::deque<uint64_t> rdDatas_ = {};
    std::deque<uint64_t> wrDatas_ = {};
    std::deque<double> wrPerSec_ = {};
    std::deque<double> rdPerSec_ = {};
    std::deque<double> wrCountPerSec_ = {};
    std::deque<double> rdCountPerSec_ = {};
    std::deque<uint64_t> rdCountDatas_ = {};
    std::deque<uint64_t> wrCountDatas_ = {};
};
class MetaData {
public:
    MetaData();
    ~MetaData() = default;
    void SetTraceType(const std::string& traceType);
    void SetSourceFileName(const std::string& fileName);
    void SetOutputFileName(const std::string& fileName);
    void SetParserToolVersion(const std::string& version);
    void SetParserToolPublishDateTime(const std::string& datetime);
    void SetTraceDataSize(uint64_t dataSize);
    void SetTraceDuration(uint64_t dur);
    const std::string& Value(uint64_t row) const;
    const std::string& Name(uint64_t row) const;
    void Clear()
    {
        columnNames_.clear();
        values_.clear();
    }

private:
    const std::string METADATA_ITEM_DATASIZE_COLNAME = "datasize";
    const std::string METADATA_ITEM_PARSETOOL_NAME_COLNAME = "parse_tool";
    const std::string METADATA_ITEM_PARSERTOOL_VERSION_COLNAME = "tool_version";
    const std::string METADATA_ITEM_PARSERTOOL_PUBLISH_DATETIME_COLNAME = "tool_publish_time";
    const std::string METADATA_ITEM_SOURCE_FILENAME_COLNAME = "source_name";
    const std::string METADATA_ITEM_OUTPUT_FILENAME_COLNAME = "output_name";
    const std::string METADATA_ITEM_PARSERTIME_COLNAME = "runtime";
    const std::string METADATA_ITEM_TRACE_DURATION_COLNAME = "trace_duration";
    const std::string METADATA_ITEM_SOURCE_DATETYPE_COLNAME = "source_type";

    std::deque<std::string> columnNames_ = {};
    std::deque<std::string> values_ = {};
};
class DataDict {
public:
    size_t Size() const
    {
        return dataDict_.size();
    }
    DataIndex GetStringIndex(std::string_view str);
    DataIndex GetStringIndexNoWrite(std::string_view str) const;
    const std::string& GetDataFromDict(DataIndex id) const
    {
        TS_ASSERT(id < dataDict_.size());
        return dataDict_[id];
    }
    void Finish();
    void Clear()
    {
        dataDict_.clear();
    }

public:
    std::deque<std::string> dataDict_;
    std::unordered_map<uint64_t, DataIndex> dataDictInnerMap_;

private:
    std::hash<std::string_view> hashFun;
    std::mutex mutex_;
    const int8_t SPASCII_START = 0;
    const int8_t SPASCII_END = 32;
};
class NetDetailData : public CacheBase {
public:
    size_t AppendNewNetData(uint64_t newTimeStamp,
                            uint64_t tx,
                            uint64_t rx,
                            uint64_t dur,
                            double rxSpeed,
                            double txSpeed,
                            uint64_t packetIn,
                            double packetInSec,
                            uint64_t packetOut,
                            double packetOutSec,
                            const std::string& netType);
    const std::deque<uint64_t>& Durs() const;
    const std::deque<double>& RxSpeed() const;
    const std::deque<double>& TxSpeed() const;
    const std::deque<std::string>& NetTypes() const;
    const std::deque<uint64_t>& RxDatas() const;
    const std::deque<uint64_t>& TxDatas() const;
    const std::deque<uint64_t>& PacketIn() const;
    const std::deque<double>& PacketInSec() const;
    const std::deque<uint64_t>& PacketOut() const;
    const std::deque<double>& PacketOutSec() const;
    void Clear() override
    {
        CacheBase::Clear();
        durs_.clear();
        rxSpeeds_.clear();
        txSpeeds_.clear();
        netTypes_.clear();
        packetIn_.clear();
        packetInSec_.clear();
        packetOut_.clear();
        packetOutSec_.clear();
    }

private:
    std::deque<uint64_t> rxs_ = {};
    std::deque<uint64_t> txs_ = {};
    std::deque<uint64_t> durs_ = {};
    std::deque<double> rxSpeeds_ = {};
    std::deque<double> txSpeeds_ = {};
    std::deque<uint64_t> packetIn_ = {};
    std::deque<double> packetInSec_ = {};
    std::deque<uint64_t> packetOut_ = {};
    std::deque<double> packetOutSec_ = {};
    std::deque<std::string> netTypes_ = {};
};
class LiveProcessDetailData : public CacheBase {
public:
    size_t AppendNewData(uint64_t newTimeStamp,
                         uint64_t dur,
                         int32_t processID,
                         std::string processName,
                         int32_t parentProcessID,
                         int32_t uid,
                         std::string userName,
                         double cpuUsage,
                         int32_t pssInfo,
                         uint64_t cpuTime,
                         int32_t threads,
                         int64_t diskWrites,
                         int64_t diskReads);
    const std::deque<uint64_t>& Durs() const;
    const std::deque<int32_t>& ProcessID() const;
    const std::deque<std::string>& ProcessName() const;
    const std::deque<int32_t>& ParentProcessID() const;
    const std::deque<int32_t>& Uid() const;
    const std::deque<std::string>& UserName() const;
    const std::deque<double>& CpuUsage() const;
    const std::deque<int32_t>& PssInfo() const;
    const std::deque<int32_t>& Threads() const;
    const std::deque<int64_t>& DiskWrites() const;
    const std::deque<int64_t>& DiskReads() const;
    const std::deque<uint64_t>& CpuTimes() const;
    void Clear() override
    {
        CacheBase::Clear();
        durs_.clear();
        processID_.clear();
        processName_.clear();
        parentProcessID_.clear();
        uid_.clear();
        userName_.clear();
        cpuUsage_.clear();
        pssInfo_.clear();
        threads_.clear();
        diskWrites_.clear();
        diskReads_.clear();
    }

private:
    std::deque<uint64_t> durs_ = {};
    std::deque<int32_t> processID_ = {};
    std::deque<std::string> processName_ = {};
    std::deque<int32_t> parentProcessID_ = {};
    std::deque<int32_t> uid_ = {};
    std::deque<std::string> userName_ = {};
    std::deque<double> cpuUsage_ = {};
    std::deque<int32_t> pssInfo_ = {};
    std::deque<int32_t> threads_ = {};
    std::deque<int64_t> diskWrites_ = {};
    std::deque<int64_t> diskReads_ = {};
    std::deque<uint64_t> cpuTimes_ = {};
};
class CpuUsageDetailData : public CacheBase {
public:
    size_t AppendNewData(uint64_t newTimeStamp,
                         uint64_t dur,
                         double totalLoad,
                         double userLoad,
                         double systemLoad,
                         int64_t threads);
    const std::deque<uint64_t>& Durs() const;
    const std::deque<double>& TotalLoad() const;
    const std::deque<double>& UserLoad() const;
    const std::deque<double>& SystemLoad() const;
    const std::deque<int64_t>& Threads() const;
    void Clear() override
    {
        CacheBase::Clear();
        durs_.clear();
        totalLoad_.clear();
        userLoad_.clear();
        systemLoad_.clear();
        threads_.clear();
    }

private:
    std::deque<uint64_t> durs_ = {};
    std::deque<double> totalLoad_ = {};
    std::deque<double> userLoad_ = {};
    std::deque<double> systemLoad_ = {};
    std::deque<int64_t> threads_ = {};
};
class FileSystemSample : public CacheBase {
public:
    size_t AppendNewData(uint32_t callChainId,
                         uint16_t type,
                         uint32_t ipid,
                         uint32_t itid,
                         uint64_t startTs,
                         uint64_t endTs,
                         uint64_t dur,
                         DataIndex returnValue,
                         DataIndex errorCode,
                         size_t size,
                         int32_t fd,
                         DataIndex fileId,
                         DataIndex firstArgument,
                         DataIndex secondArgument,
                         DataIndex thirdArgument,
                         DataIndex fourthArgument);
    const std::deque<uint32_t>& CallChainIds() const;
    const std::deque<uint16_t>& Types() const;
    const std::deque<uint32_t>& Ipids() const;
    const std::deque<uint32_t>& Itids() const;
    const std::deque<uint64_t>& StartTs() const;
    const std::deque<uint64_t>& EndTs() const;
    const std::deque<uint64_t>& Durs() const;
    const std::deque<DataIndex>& ReturnValues() const;
    const std::deque<DataIndex>& ErrorCodes() const;
    const std::deque<int32_t>& Fds() const;
    const std::deque<DataIndex>& FileIds() const;
    const std::deque<size_t>& Sizes() const;
    const std::deque<DataIndex>& FirstArguments() const;
    const std::deque<DataIndex>& SecondArguments() const;
    const std::deque<DataIndex>& ThirdArguments() const;
    const std::deque<DataIndex>& FourthArguments() const;
    void Clear() override
    {
        CacheBase::Clear();
        callChainIds_.clear();
        types_.clear();
        ipids_.clear();
        itids_.clear();
        startTs_.clear();
        endTs_.clear();
        durs_.clear();
        returnValues_.clear();
        errorCodes_.clear();
        fds_.clear();
        Sizes_.clear();
        firstArguments_.clear();
        secondArguments_.clear();
        thirdArguments_.clear();
        fourthArguments_.clear();
    }

private:
    std::deque<uint32_t> callChainIds_ = {};
    std::deque<uint16_t> types_ = {};
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint32_t> itids_ = {};
    std::deque<uint64_t> startTs_ = {};
    std::deque<uint64_t> endTs_ = {};
    std::deque<uint64_t> durs_ = {};
    std::deque<DataIndex> returnValues_ = {};
    std::deque<DataIndex> errorCodes_ = {};
    std::deque<int32_t> fds_ = {};
    std::deque<DataIndex> fileIds_ = {};
    std::deque<size_t> Sizes_ = {};
    std::deque<DataIndex> firstArguments_ = {};
    std::deque<DataIndex> secondArguments_ = {};
    std::deque<DataIndex> thirdArguments_ = {};
    std::deque<DataIndex> fourthArguments_ = {};
};
class EbpfCallStackData : public CacheBase {
public:
    size_t AppendNewData(uint32_t callChainId, uint32_t depth, DataIndex ip, DataIndex symbolId, DataIndex filePathId);
    void UpdateSymbolAndFilePathIndex(size_t row, DataIndex symbolId, DataIndex filePathId);
    const std::deque<uint32_t>& CallChainIds() const;
    const std::deque<uint32_t>& Depths() const;
    const std::deque<DataIndex>& Ips() const;
    const std::deque<DataIndex>& SymbolIds() const;
    const std::deque<DataIndex>& FilePathIds() const;
    void Clear() override
    {
        CacheBase::Clear();
        callChainIds_.clear();
        depths_.clear();
        symbolIds_.clear();
        filePathIds_.clear();
    }

private:
    std::deque<uint32_t> callChainIds_ = {};
    std::deque<uint32_t> depths_ = {};
    std::deque<DataIndex> ips_ = {};
    std::deque<DataIndex> symbolIds_ = {};
    std::deque<DataIndex> filePathIds_ = {};
};
class PagedMemorySampleData : public CacheBase {
public:
    size_t AppendNewData(uint32_t callChainId,
                         uint16_t type,
                         uint32_t ipid,
                         uint64_t startTs,
                         uint64_t endTs,
                         uint64_t dur,
                         size_t size,
                         DataIndex addr,
                         uint32_t itid);
    const std::deque<uint32_t>& CallChainIds() const;
    const std::deque<uint16_t>& Types() const;
    const std::deque<uint32_t>& Ipids() const;
    const std::deque<uint64_t>& StartTs() const;
    const std::deque<uint64_t>& EndTs() const;
    const std::deque<uint64_t>& Durs() const;
    const std::deque<size_t>& Sizes() const;
    const std::deque<DataIndex>& Addr() const;
    const std::deque<uint32_t>& Itids() const;
    void Clear() override
    {
        CacheBase::Clear();
        callChainIds_.clear();
        types_.clear();
        ipids_.clear();
        startTs_.clear();
        endTs_.clear();
        durs_.clear();
        Sizes_.clear();
        addrs_.clear();
        itids_.clear();
    }

private:
    std::deque<uint32_t> callChainIds_ = {};
    std::deque<uint16_t> types_ = {};
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint64_t> startTs_ = {};
    std::deque<uint64_t> endTs_ = {};
    std::deque<uint64_t> durs_ = {};
    std::deque<size_t> Sizes_ = {};
    std::deque<DataIndex> addrs_ = {};
    std::deque<uint32_t> itids_ = {};
};
#if WITH_EBPF_HELP
class EbpfProcessMaps : public CacheBase {
public:
    size_t AppendNewData(uint64_t start,
                         uint64_t end,
                         uint32_t offset,
                         uint32_t pid,
                         uint32_t fileNameLen,
                         uint64_t fileNameIndex);
    const std::deque<uint64_t>& Starts() const;
    const std::deque<uint64_t>& Ends() const;
    const std::deque<uint32_t>& Offsets() const;
    const std::deque<uint32_t>& Pids() const;
    const std::deque<uint32_t>& FileNameLens() const;
    const std::deque<uint64_t>& FileNameIndexs() const;
    void Clear() override
    {
        CacheBase::Clear();
        starts_.clear();
        ends_.clear();
        offsets_.clear();
        pids_.clear();
        fileNameLens_.clear();
        fileNameIndexs_.clear();
    }

private:
    std::deque<uint64_t> starts_ = {};
    std::deque<uint64_t> ends_ = {};
    std::deque<uint32_t> offsets_ = {};
    std::deque<uint32_t> pids_ = {};
    std::deque<uint32_t> fileNameLens_ = {};
    std::deque<uint64_t> fileNameIndexs_ = {};
};

class EbpfElf : public CacheBase {
public:
    size_t AppendNewData(uint64_t elfId,
                         uint64_t textVaddr,
                         uint32_t textOffset,
                         uint32_t strTabLen,
                         uint32_t symTabLen,
                         uint32_t fileNameLen,
                         uint32_t symEntLen,
                         uint64_t fileNameIndex);
    const std::deque<uint64_t>& ElfIds() const;
    const std::deque<uint64_t>& TextVaddrs() const;
    const std::deque<uint32_t>& TextOffsets() const;
    const std::deque<uint32_t>& StrTabLens() const;
    const std::deque<uint32_t>& SymTabLens() const;
    const std::deque<uint32_t>& FileNameLens() const;
    const std::deque<uint32_t>& SymEntLens() const;
    const std::deque<uint64_t>& FileNameIndexs() const;

    void Clear() override
    {
        CacheBase::Clear();
        elfIds_.clear();
        textVaddrs_.clear();
        textOffsets_.clear();
        strTabLens_.clear();
        symTabLens_.clear();
        fileNameLens_.clear();
        symEntLens_.clear();
        fileNameIndexs_.clear();
    }

private:
    std::deque<uint64_t> elfIds_ = {};
    std::deque<uint64_t> textVaddrs_ = {};
    std::deque<uint32_t> textOffsets_ = {};
    std::deque<uint32_t> strTabLens_ = {};
    std::deque<uint32_t> symTabLens_ = {};
    std::deque<uint32_t> fileNameLens_ = {};
    std::deque<uint32_t> symEntLens_ = {};
    std::deque<uint64_t> fileNameIndexs_ = {};
};

class EbpfElfSymbol : public CacheBase {
public:
    size_t AppendNewData(uint64_t elfId, uint32_t stName, uint64_t stValue, uint64_t stSize);
    const std::deque<uint64_t>& ElfIds() const;
    const std::deque<uint32_t>& StNames() const;
    const std::deque<uint64_t>& StValues() const;
    const std::deque<uint64_t>& StSizes() const;

    void Clear() override
    {
        CacheBase::Clear();
        elfIds_.clear();
        stNames_.clear();
        stValues_.clear();
        stSizes_.clear();
    }

private:
    std::deque<uint64_t> elfIds_ = {};
    std::deque<uint32_t> stNames_ = {};
    std::deque<uint64_t> stValues_ = {};
    std::deque<uint64_t> stSizes_ = {};
};
#endif
class AppNames : public CacheBase {
public:
    uint32_t AppendAppName(uint8_t flags, DataIndex eventSource, DataIndex appName);
    const std::deque<uint8_t>& Falgs() const;
    const std::deque<DataIndex>& EventSourceId() const;
    const std::deque<DataIndex>& AppName() const;
    void Clear() override
    {
        CacheBase::Clear();
        flags_.clear();
        appNames_.clear();
        keyNames_.clear();
    }

private:
    std::deque<uint8_t> flags_ = {};
    std::deque<DataIndex> appNames_ = {};
    std::deque<DataIndex> keyNames_ = {};
};
class SysEventMeasureData : public CacheBase {
public:
    void AppendData(uint64_t serial,
                    uint64_t ts,
                    uint32_t nameId,
                    uint32_t keyId,
                    int32_t type,
                    double numericValue,
                    DataIndex stringValue);
    const std::deque<uint64_t>& Serial() const;
    const std::deque<uint64_t>& Ts() const;
    const std::deque<uint32_t>& NameFilterId() const;
    const std::deque<uint32_t>& AppKeyFilterId() const;
    const std::deque<int32_t>& Type() const;
    const std::deque<double>& NumValue() const;
    const std::deque<DataIndex>& StringValue() const;
    void Clear() override
    {
        CacheBase::Clear();
        serial_.clear();
        ts_.clear();
        nameFilterIds_.clear();
        appKeyFilterIds_.clear();
        types_.clear();
        numValues_.clear();
        stringValues_.clear();
    }

private:
    std::deque<uint64_t> serial_ = {};
    std::deque<uint64_t> ts_ = {};
    std::deque<uint32_t> nameFilterIds_ = {};
    std::deque<uint32_t> appKeyFilterIds_ = {};
    std::deque<int32_t> types_ = {};
    std::deque<double> numValues_ = {};
    std::deque<DataIndex> stringValues_ = {};
    uint32_t rowCount_ = 0;
};
class DeviceStateData : public CacheBase {
public:
    void AppendNewData(int32_t brightness,
                       int32_t btState,
                       int32_t location,
                       int32_t wifi,
                       int32_t streamDefault,
                       int32_t voiceCall,
                       int32_t music,
                       int32_t streamRing,
                       int32_t media,
                       int32_t voiceAssistant,
                       int32_t system,
                       int32_t alarm,
                       int32_t notification,
                       int32_t btSco,
                       int32_t enforcedAudible,
                       int32_t streamDtmf,
                       int32_t streamTts,
                       int32_t accessibility,
                       int32_t recording,
                       int32_t streamAll);
    const std::deque<int32_t>& Brightness() const;
    const std::deque<int32_t>& BtState() const;
    const std::deque<int32_t>& Location() const;
    const std::deque<int32_t>& Wifi() const;
    const std::deque<int32_t>& StreamDefault() const;
    const std::deque<int32_t>& VoiceCall() const;
    const std::deque<int32_t>& Music() const;
    const std::deque<int32_t>& StreamRing() const;
    const std::deque<int32_t>& Media() const;
    const std::deque<int32_t>& VoiceAssistant() const;
    const std::deque<int32_t>& System() const;
    const std::deque<int32_t>& Alarm() const;
    const std::deque<int32_t>& Notification() const;
    const std::deque<int32_t>& BtSco() const;
    const std::deque<int32_t>& EnforcedAudible() const;
    const std::deque<int32_t>& StreamDtmf() const;
    const std::deque<int32_t>& StreamTts() const;
    const std::deque<int32_t>& Accessibility() const;
    const std::deque<int32_t>& Recording() const;
    const std::deque<int32_t>& StreamAll() const;
    void Clear() override
    {
        CacheBase::Clear();
        brightness_.clear();
        btStates_.clear();
        locations_.clear();
        wifis_.clear();
        streamDefaults_.clear();
        voiceCalls_.clear();
        musics_.clear();
        streamRings_.clear();
        medias_.clear();
        voiceAssistants_.clear();
        systems_.clear();
        alarms_.clear();
        notifications_.clear();
        btScos_.clear();
        enforcedAudibles_.clear();
        streamDtmfs_.clear();
        streamTts_.clear();
        accessibilitys_.clear();
        recordings_.clear();
        streamAlls_.clear();
    }

private:
    std::deque<uint32_t> stringValues_ = {};
    std::deque<int32_t> brightness_ = {};
    std::deque<int32_t> btStates_ = {};
    std::deque<int32_t> locations_ = {};
    std::deque<int32_t> wifis_ = {};
    std::deque<int32_t> streamDefaults_ = {};
    std::deque<int32_t> voiceCalls_ = {};
    std::deque<int32_t> musics_ = {};
    std::deque<int32_t> streamRings_ = {};
    std::deque<int32_t> medias_ = {};
    std::deque<int32_t> voiceAssistants_ = {};
    std::deque<int32_t> systems_ = {};
    std::deque<int32_t> alarms_ = {};
    std::deque<int32_t> notifications_ = {};
    std::deque<int32_t> btScos_ = {};
    std::deque<int32_t> enforcedAudibles_ = {};
    std::deque<int32_t> streamDtmfs_ = {};
    std::deque<int32_t> streamTts_ = {};
    std::deque<int32_t> accessibilitys_ = {};
    std::deque<int32_t> recordings_ = {};
    std::deque<int32_t> streamAlls_ = {};
    uint32_t rowCounts_ = 0;
};
class TraceConfigData : public CacheBase {
public:
    void AppendNewData(std::string traceSource, std::string key, std::string value);
    const std::deque<std::string>& TraceSource() const;
    const std::deque<std::string>& Key() const;
    const std::deque<std::string>& Value() const;
    void Clear() override
    {
        CacheBase::Clear();
        traceSource_.clear();
        key_.clear();
        value_.clear();
    }

private:
    std::deque<std::string> traceSource_ = {};
    std::deque<std::string> key_ = {};
    std::deque<std::string> value_ = {};
    uint32_t rowCounts_ = 0;
};
class SmapsData : public CacheBase {
public:
    void AppendNewData(uint64_t timeStamp,
                       uint64_t ipid,
                       std::string startAddr,
                       std::string endAddr,
                       uint64_t dirty,
                       uint64_t swapper,
                       uint64_t rss,
                       uint64_t pss,
                       uint64_t size,
                       double reside,
                       DataIndex protectionId,
                       DataIndex pathId,
                       uint64_t shared_clean,
                       uint64_t shared_dirty,
                       uint64_t private_clean,
                       uint64_t private_dirty,
                       uint64_t swap,
                       uint64_t swap_pss,
                       uint32_t type);
    const std::deque<uint64_t>& Id() const;
    const std::deque<uint64_t>& TimeStamps() const;
    const std::deque<uint64_t>& Ipids() const;
    const std::deque<std::string>& StartAddrs() const;
    const std::deque<std::string>& EndAddrs() const;
    const std::deque<uint64_t>& Dirtys() const;
    const std::deque<uint64_t>& Swappers() const;
    const std::deque<uint64_t>& Rss() const;
    const std::deque<uint64_t>& Pss() const;
    const std::deque<uint64_t>& Sizes() const;
    const std::deque<double>& Resides() const;
    const std::deque<DataIndex>& ProtectionIds() const;
    const std::deque<DataIndex>& PathIds() const;
    const std::deque<uint64_t>& SharedClean() const;
    const std::deque<uint64_t>& SharedDirty() const;
    const std::deque<uint64_t>& PrivateClean() const;
    const std::deque<uint64_t>& PrivateDirty() const;
    const std::deque<uint64_t>& Swap() const;
    const std::deque<uint64_t>& SwapPss() const;
    const std::deque<uint32_t>& Type() const;
    void Clear() override
    {
        CacheBase::Clear();
        ipids_.clear();
        startAddrs_.clear();
        endAddrs_.clear();
        dirtys_.clear();
        swappers_.clear();
        rss_.clear();
        pss_.clear();
        sizes_.clear();
        resides_.clear();
        protectionIds_.clear();
        pathIds_.clear();
        sharedClean_.clear();
        sharedDirty_.clear();
        privateClean_.clear();
        privateDirty_.clear();
        swap_.clear();
        swapPss_.clear();
        type_.clear();
    }

private:
    std::deque<uint64_t> ipids_ = {};
    std::deque<std::string> startAddrs_ = {};
    std::deque<std::string> endAddrs_ = {};
    std::deque<uint64_t> dirtys_ = {};
    std::deque<uint64_t> swappers_ = {};
    std::deque<uint64_t> rss_ = {};
    std::deque<uint64_t> pss_ = {};
    std::deque<uint64_t> sizes_ = {};
    std::deque<double> resides_ = {};
    std::deque<DataIndex> protectionIds_ = {};
    std::deque<DataIndex> pathIds_ = {};
    std::deque<uint64_t> sharedClean_ = {};
    std::deque<uint64_t> sharedDirty_ = {};
    std::deque<uint64_t> privateClean_ = {};
    std::deque<uint64_t> privateDirty_ = {};
    std::deque<uint64_t> swap_ = {};
    std::deque<uint64_t> swapPss_ = {};
    std::deque<uint32_t> type_ = {};
    uint32_t rowCount_ = 0;
};
class BioLatencySampleData : public CacheBase {
public:
    void AppendNewData(uint32_t callChainId,
                       uint64_t type,
                       uint32_t ipid,
                       uint32_t itid,
                       uint64_t startTs,
                       uint64_t endTs,
                       uint64_t latencyDur,
                       uint32_t tier,
                       uint64_t size,
                       uint64_t blockNumber,
                       uint64_t filePathId,
                       uint64_t durPer4k);
    const std::deque<uint64_t>& Id() const;
    const std::deque<uint32_t>& CallChainIds() const;
    const std::deque<uint64_t>& Types() const;
    const std::deque<uint32_t>& Ipids() const;
    const std::deque<uint32_t>& Itids() const;
    const std::deque<uint64_t>& StartTs() const;
    const std::deque<uint64_t>& EndTs() const;
    const std::deque<uint64_t>& LatencyDurs() const;
    const std::deque<uint32_t>& Tiers() const;
    const std::deque<uint64_t>& Sizes() const;
    const std::deque<uint64_t>& BlockNumbers() const;
    const std::deque<uint64_t>& FilePathIds() const;
    const std::deque<uint64_t>& DurPer4k() const;
    void Clear() override
    {
        CacheBase::Clear();
        callChainIds_.clear();
        types_.clear();
        ipids_.clear();
        itids_.clear();
        startTs_.clear();
        endTs_.clear();
        latencyDurs_.clear();
        tiers_.clear();
        sizes_.clear();
        blockNumbers_.clear();
        filePathIds_.clear();
        durPer4ks_.clear();
    }

private:
    std::deque<uint32_t> callChainIds_ = {};
    std::deque<uint64_t> types_ = {};
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint32_t> itids_ = {};
    std::deque<uint64_t> startTs_ = {};
    std::deque<uint64_t> endTs_ = {};
    std::deque<uint64_t> latencyDurs_ = {};
    std::deque<uint32_t> tiers_ = {};
    std::deque<uint64_t> sizes_ = {};
    std::deque<uint64_t> blockNumbers_ = {};
    std::deque<uint64_t> filePathIds_ = {};
    std::deque<uint64_t> durPer4ks_ = {};
    uint32_t rowCount_ = 0;
};
class ClockSnapshotData {
public:
    size_t AppendNewSnapshot(uint8_t clockId, uint64_t ts, const std::string& name)
    {
        clockIds_.emplace_back(clockId);
        ts_.emplace_back(ts);
        names_.emplace_back(name);
        return ts_.size();
    }
    size_t Size() const
    {
        return ts_.size();
    }
    const std::deque<uint8_t>& ClockIds() const
    {
        return clockIds_;
    }
    const std::deque<uint64_t>& Ts() const
    {
        return ts_;
    }
    const std::deque<std::string>& Names() const
    {
        return names_;
    }

private:
    std::deque<uint8_t> clockIds_ = {};
    std::deque<uint64_t> ts_ = {};
    std::deque<std::string> names_ = {};
};
class DataSourceClockIdData {
public:
    DataSourceClockIdData();
    size_t AppendNewDataSourceClockId(const std::string& dataSoruceName, uint8_t clockId)
    {
        dataSourceNames_.emplace_back(dataSoruceName);
        clockIds_.emplace_back(clockId);
        return dataSourceNames_.size();
    }
    size_t Size() const
    {
        return dataSourceNames_.size();
    }
    const std::deque<uint8_t>& ClockIds() const
    {
        return clockIds_;
    }
    const std::deque<std::string>& Names() const
    {
        return dataSourceNames_;
    }
    void SetDataSourceClockId(DataSourceType source, uint32_t id);
    void Finish();

private:
    std::deque<std::string> dataSourceNames_ = {};
    std::deque<uint8_t> clockIds_ = {};
    std::map<DataSourceType, uint8_t> dataSource2ClockIdMap_ = {};
    std::map<DataSourceType, std::string> dataSource2PluginNameMap_ = {};
};

class FrameSlice : public CacheBase {
public:
    size_t AppendFrame(uint64_t ts, uint32_t ipid, uint32_t itid, uint32_t vsyncId, uint64_t callStackSliceId);
    size_t AppendFrame(uint64_t ts,
                       uint32_t ipid,
                       uint32_t itid,
                       uint32_t vsyncId,
                       uint64_t callStackSliceId,
                       uint64_t end,
                       uint8_t type);
    void SetEndTime(uint64_t row, uint64_t end);
    void SetType(uint64_t row, uint8_t type);
    void SetDst(uint64_t row, uint64_t dst);
    void SetSrcs(uint64_t row, const std::vector<uint64_t>& fromSlices);
    void SetFlags(uint64_t row, const uint32_t flags);
    const std::deque<uint32_t> Ipids() const;
    const std::deque<uint32_t> VsyncIds() const;
    const std::deque<uint64_t> CallStackIds() const;
    const std::deque<uint64_t> EndTss() const;
    const std::deque<uint64_t> Dsts() const;
    const std::deque<uint64_t> Durs() const;
    const std::deque<uint8_t> Types() const;
    const std::deque<uint8_t> Flags() const;
    const std::deque<uint8_t> Depths() const;
    const std::deque<uint32_t> FrameNos() const;
    const std::deque<std::string>& Srcs() const;
    void UpdateCallStackSliceId(uint64_t row, uint64_t callStackSliceId);
    void SetEndTimeAndFlag(uint64_t row, uint64_t ts, uint64_t expectDur, uint64_t expectEnd);
    void Erase(uint64_t row);
    static uint32_t GetAbnormalStartEndTimeState()
    {
        return abnormalStartEndTimeState_;
    }

public:
    typedef enum FrameSliceType { ACTURAL_SLICE, EXPECT_SLICE } FrameSliceType;

private:
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint64_t> dsts_ = {};
    std::deque<std::string> srcs_ = {};
    std::deque<uint32_t> vsyncIds_ = {};
    std::deque<uint64_t> callStackIds_ = {};
    std::deque<uint64_t> endTss_ = {};
    std::deque<uint64_t> durs_ = {};
    std::deque<uint8_t> types_ = {};
    std::deque<uint8_t> flags_ = {};
    std::deque<uint8_t> depths_ = {};
    std::deque<uint32_t> frameNos_ = {};
    const uint32_t INVALID_ROW = 2;
    static const uint32_t abnormalStartEndTimeState_ = 3;
};
class FrameMaps : public CacheBase {
public:
    size_t AppendNew(FrameSlice* frameSlice, uint64_t src, uint64_t dst);
    const std::deque<uint64_t>& SrcIndexs() const;
    const std::deque<uint64_t>& DstIndexs() const;

private:
    std::deque<uint64_t> srcs_ = {};
    std::deque<uint64_t> dsts_ = {};
};

class AppStartup : public CacheBase {
public:
    size_t AppendNewData(uint32_t ipid,
                         uint32_t tid,
                         uint32_t callId,
                         uint64_t startTime,
                         uint64_t endTime,
                         uint32_t startName,
                         DataIndex packedName);
    const std::deque<uint32_t>& Pids() const;
    const std::deque<uint32_t>& Tids() const;
    const std::deque<uint32_t>& CallIds() const;
    const std::deque<uint64_t>& StartTimes() const;
    const std::deque<uint64_t>& EndTimes() const;
    const std::deque<uint32_t>& StartNames() const;
    const std::deque<DataIndex>& PackedNames() const;

    void Clear() override
    {
        CacheBase::Clear();
        ipids_.clear();
        tids_.clear();
        callIds_.clear();
        startTimes_.clear();
        endTimes_.clear();
        startNames_.clear();
        packedNames_.clear();
    }

private:
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint32_t> tids_ = {};
    std::deque<uint32_t> callIds_ = {};
    std::deque<uint64_t> startTimes_ = {};
    std::deque<uint64_t> endTimes_ = {};
    std::deque<uint32_t> startNames_ = {};
    std::deque<DataIndex> packedNames_ = {};
};

class SoStaticInitalization : public CacheBase {
public:
    size_t AppendNewData(uint32_t ipid,
                         uint32_t tid,
                         uint32_t callId,
                         uint64_t startTime,
                         uint64_t endTime,
                         DataIndex soName,
                         uint32_t depth);
    const std::deque<uint32_t>& Pids() const;
    const std::deque<uint32_t>& Tids() const;
    const std::deque<uint32_t>& CallIds() const;
    const std::deque<uint64_t>& StartTimes() const;
    const std::deque<uint64_t>& EndTimes() const;
    const std::deque<DataIndex>& SoNames() const;
    const std::deque<uint32_t> Depths() const;

    void Clear() override
    {
        CacheBase::Clear();
        ipids_.clear();
        tids_.clear();
        callIds_.clear();
        startTimes_.clear();
        endTimes_.clear();
        soNames_.clear();
        depths_.clear();
    }

private:
    std::deque<uint32_t> ipids_ = {};
    std::deque<uint32_t> tids_ = {};
    std::deque<uint32_t> callIds_ = {};
    std::deque<uint64_t> startTimes_ = {};
    std::deque<uint64_t> endTimes_ = {};
    std::deque<DataIndex> soNames_ = {};
    std::deque<uint32_t> depths_ = {};
};

class JsHeapFiles : public CacheBase {
public:
    size_t
        AppendNewData(uint32_t id, std::string filePath, uint64_t startTime, uint64_t endTime, uint64_t selfSizeCount);
    const std::deque<uint32_t>& IDs() const;
    const std::deque<std::string>& FilePaths() const;
    const std::deque<uint64_t>& StartTimes() const;
    const std::deque<uint64_t>& EndTimes() const;
    const std::deque<uint64_t>& SelfSizeCount() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        filePaths_.clear();
        startTimes_.clear();
        endTimes_.clear();
        selfSizeCount_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<std::string> filePaths_ = {};
    std::deque<uint64_t> startTimes_ = {};
    std::deque<uint64_t> endTimes_ = {};
    std::deque<uint64_t> selfSizeCount_ = {};
};

class JsHeapEdges : public CacheBase {
public:
    size_t AppendNewData(uint32_t fileId,
                         uint32_t edgeIndex,
                         uint32_t type,
                         uint32_t nameOrIndex,
                         uint32_t toNode,
                         uint32_t fromNodeId,
                         uint32_t toNodeId);
    const std::deque<uint32_t>& FileIds() const;
    const std::deque<uint32_t>& EdgeIndexs() const;
    const std::deque<uint32_t>& Types() const;
    const std::deque<uint32_t>& NameOrIndexs() const;
    const std::deque<uint32_t>& ToNodes() const;
    const std::deque<uint32_t>& FromNodeIds() const;
    const std::deque<uint32_t>& ToNodeIds() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        edgeIndexs_.clear();
        types_.clear();
        nameOrIndexs_.clear();
        toNodes_.clear();
        fromNodeIds_.clear();
        toNodeIds_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<uint32_t> edgeIndexs_ = {};
    std::deque<uint32_t> types_ = {};
    std::deque<uint32_t> nameOrIndexs_ = {};
    std::deque<uint32_t> toNodes_ = {};
    std::deque<uint32_t> fromNodeIds_ = {};
    std::deque<uint32_t> toNodeIds_ = {};
};

class JsHeapInfo : public CacheBase {
public:
    size_t AppendNewData(uint32_t fileId, std::string key, uint32_t type, int32_t intValue, std::string strValue);
    const std::deque<uint32_t>& FileIds() const;
    const std::deque<std::string>& Keys() const;
    const std::deque<uint32_t>& Types() const;
    const std::deque<int32_t>& IntValues() const;
    const std::deque<std::string>& StrValues() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        keys_.clear();
        types_.clear();
        intValues_.clear();
        strValues_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<std::string> keys_ = {};
    std::deque<uint32_t> types_ = {};
    std::deque<int32_t> intValues_ = {};
    std::deque<std::string> strValues_ = {};
};

class JsHeapLocation : public CacheBase {
public:
    size_t AppendNewData(uint32_t fileId, uint32_t objectIndex, uint32_t scriptId, uint32_t line, uint32_t column);
    const std::deque<uint32_t>& FileIds() const;
    const std::deque<uint32_t>& ObjectIndexs() const;
    const std::deque<uint32_t>& ScriptIds() const;
    const std::deque<uint32_t>& Lines() const;
    const std::deque<uint32_t>& Columns() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        objectIndexs_.clear();
        scriptIds_.clear();
        lines_.clear();
        columns_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<uint32_t> objectIndexs_ = {};
    std::deque<uint32_t> scriptIds_ = {};
    std::deque<uint32_t> lines_ = {};
    std::deque<uint32_t> columns_ = {};
};

class JsHeapNodes : public CacheBase {
public:
    size_t AppendNewData(uint32_t fileId,
                         uint32_t nodeIndex,
                         uint32_t type,
                         uint32_t name,
                         uint32_t id,
                         uint32_t selfSize,
                         uint32_t edgeCount,
                         uint32_t traceNodeId,
                         uint32_t detachedNess);
    const std::deque<uint32_t>& FileIds() const;
    const std::deque<uint32_t>& NodeIndexs() const;
    const std::deque<uint32_t>& Types() const;
    const std::deque<uint32_t>& Names() const;
    const std::deque<uint32_t>& NodeIds() const;
    const std::deque<uint32_t>& SelfSizes() const;
    const std::deque<uint32_t>& EdgeCounts() const;
    const std::deque<uint32_t>& TraceNodeIds() const;
    const std::deque<uint32_t>& DetachedNess() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        nodeIndexs_.clear();
        types_.clear();
        names_.clear();
        nodeIds_.clear();
        selfSizes_.clear();
        edgeCounts_.clear();
        traceNodeIds_.clear();
        detachedNess_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<uint32_t> nodeIndexs_ = {};
    std::deque<uint32_t> types_ = {};
    std::deque<uint32_t> names_ = {};
    std::deque<uint32_t> nodeIds_ = {};
    std::deque<uint32_t> selfSizes_ = {};
    std::deque<uint32_t> edgeCounts_ = {};
    std::deque<uint32_t> traceNodeIds_ = {};
    std::deque<uint32_t> detachedNess_ = {};
};

class JsHeapSample : public CacheBase {
public:
    size_t AppendNewData(uint32_t fileId, uint64_t timeStampUs, uint32_t lastAssignedId);
    const std::deque<uint32_t>& FileIds() const;
    const std::deque<uint64_t>& TimeStampUs() const;
    const std::deque<uint32_t>& LastAssignedIds() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        timeStampUs_.clear();
        lastAssignedIds_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<uint64_t> timeStampUs_ = {};
    std::deque<uint32_t> lastAssignedIds_ = {};
};

class JsHeapString : public CacheBase {
public:
    size_t AppendNewData(uint32_t fileId, uint32_t fileIndex, std::string string);
    const std::deque<uint32_t>& FileIds() const;
    const std::deque<uint64_t>& FileIndexs() const;
    const std::deque<std::string>& Strings() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        fileIndexs_.clear();
        strings_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<uint64_t> fileIndexs_ = {};
    std::deque<std::string> strings_ = {};
};

class JsHeapTraceFuncInfo : public CacheBase {
public:
    size_t AppendNewData(uint32_t fileId,
                         uint32_t functionIndex,
                         uint32_t functionId,
                         uint32_t name,
                         uint32_t scriptName,
                         uint32_t scriptId,
                         uint32_t line,
                         uint32_t column);
    const std::deque<uint32_t>& FileIds() const;
    const std::deque<uint32_t>& FunctionIndexs() const;
    const std::deque<uint32_t>& FunctionIds() const;
    const std::deque<uint32_t>& Names() const;
    const std::deque<uint32_t>& ScriptNames() const;
    const std::deque<uint32_t>& ScriptIds() const;
    const std::deque<uint32_t>& Lines() const;
    const std::deque<uint32_t>& Columns() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        functionIndexs_.clear();
        functionIds_.clear();
        names_.clear();
        scriptNames_.clear();
        scriptIds_.clear();
        lines_.clear();
        columns_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<uint32_t> functionIndexs_ = {};
    std::deque<uint32_t> functionIds_ = {};
    std::deque<uint32_t> names_ = {};
    std::deque<uint32_t> scriptNames_ = {};
    std::deque<uint32_t> scriptIds_ = {};
    std::deque<uint32_t> lines_ = {};
    std::deque<uint32_t> columns_ = {};
};

class JsHeapTraceNode : public CacheBase {
public:
    size_t AppendNewData(uint32_t fileId,
                         uint32_t traceNodeId,
                         uint32_t functionInfoIndex,
                         uint32_t count,
                         uint32_t size,
                         int32_t parentId);
    const std::deque<uint32_t>& FileIds() const;
    const std::deque<uint32_t>& TraceNodeIDs() const;
    const std::deque<uint32_t>& FunctionInfoIndexs() const;
    const std::deque<uint32_t>& Counts() const;
    const std::deque<uint32_t>& NodeSizes() const;
    const std::deque<int32_t>& ParentIds() const;
    void Clear() override
    {
        CacheBase::Clear();
        fileIds_.clear();
        traceNodeIds_.clear();
        functionInfoIndexs_.clear();
        counts_.clear();
        sizes_.clear();
        parentIds_.clear();
    }

private:
    std::deque<uint32_t> fileIds_ = {};
    std::deque<uint32_t> traceNodeIds_ = {};
    std::deque<uint32_t> functionInfoIndexs_ = {};
    std::deque<uint32_t> counts_ = {};
    std::deque<uint32_t> sizes_ = {};
    std::deque<int32_t> parentIds_ = {};
};

class JsConfig : public CacheBase {
public:
    size_t AppendNewData(uint32_t pid,
                         uint64_t type,
                         uint32_t interval,
                         uint32_t captureNumericValue,
                         uint32_t trackAllocation,
                         uint32_t cpuProfiler,
                         uint32_t cpuProfilerInterval);
    const std::deque<uint32_t>& Pids() const;
    const std::deque<uint64_t>& Types() const;
    const std::deque<uint32_t>& Intervals() const;
    const std::deque<uint32_t>& CaptureNumericValue() const;
    const std::deque<uint32_t>& TrackAllocations() const;
    const std::deque<uint32_t>& CpuProfiler() const;
    const std::deque<uint32_t>& CpuProfilerInterval() const;
    void Clear() override
    {
        CacheBase::Clear();
        pids_.clear();
        types_.clear();
        intervals_.clear();
        captureNumericValues_.clear();
        trackAllocations_.clear();
        cpuProfilers_.clear();
        cpuProfilerIntervals_.clear();
    }

private:
    std::deque<uint32_t> pids_ = {};
    std::deque<uint64_t> types_ = {};
    std::deque<uint32_t> intervals_ = {};
    std::deque<uint32_t> captureNumericValues_ = {};
    std::deque<uint32_t> trackAllocations_ = {};
    std::deque<uint32_t> cpuProfilers_ = {};
    std::deque<uint32_t> cpuProfilerIntervals_ = {};
};

class JsCpuProfilerNode : public CacheBase {
public:
    size_t AppendNewData(uint32_t functionId,
                         uint32_t functionName,
                         std::string scriptId,
                         uint32_t url,
                         uint32_t lineNumber,
                         uint32_t columnNumber,
                         uint32_t hitCount,
                         std::string children,
                         uint32_t parent);
    const std::deque<uint32_t>& FunctionIds() const;
    const std::deque<uint32_t>& FunctionNames() const;
    const std::deque<std::string>& ScriptIds() const;
    const std::deque<uint32_t>& Urls() const;
    const std::deque<uint32_t>& LineNumbers() const;
    const std::deque<int32_t>& ColumnNumbers() const;
    const std::deque<int32_t>& HitCounts() const;
    const std::deque<std::string>& Children() const;
    const std::deque<uint32_t>& Parents() const;
    void Clear() override
    {
        CacheBase::Clear();
        functionIds_.clear();
        functionNames_.clear();
        scriptIds_.clear();
        urls_.clear();
        lineNumbers_.clear();
        columnNumbers_.clear();
        hitCounts_.clear();
        children_.clear();
        parents_.clear();
    }

private:
    std::deque<uint32_t> functionIds_ = {};
    std::deque<uint32_t> functionNames_ = {};
    std::deque<std::string> scriptIds_ = {};
    std::deque<uint32_t> urls_ = {};
    std::deque<uint32_t> lineNumbers_ = {};
    std::deque<int32_t> columnNumbers_ = {};
    std::deque<int32_t> hitCounts_ = {};
    std::deque<std::string> children_ = {};
    std::deque<uint32_t> parents_ = {};
};

class JsCpuProfilerSample : public CacheBase {
public:
    size_t AppendNewData(uint32_t functionId, uint64_t startTime, uint64_t endTime, uint64_t dur);
    const std::deque<uint32_t>& FunctionIds() const;
    const std::deque<uint64_t>& StartTimes() const;
    const std::deque<uint64_t>& EndTimes() const;
    const std::deque<uint64_t>& Durs() const;
    void Clear() override
    {
        CacheBase::Clear();
        functionIds_.clear();
        startTimes_.clear();
        endTimes_.clear();
        durs_.clear();
    }

private:
    std::deque<uint32_t> functionIds_ = {};
    std::deque<uint64_t> startTimes_ = {};
    std::deque<uint64_t> endTimes_ = {};
    std::deque<uint64_t> durs_ = {};
};

class GPUSlice {
public:
    size_t AppendNew(uint32_t frameRow, uint64_t dur);
    const std::deque<uint32_t>& FrameRows() const;
    const std::deque<uint64_t>& Durs() const;
    size_t Size() const;

private:
    std::deque<uint32_t> frameRows_ = {};
    std::deque<uint64_t> durs_ = {};
};

class TaskPoolInfo : public CacheBase {
public:
    size_t AppendAllocationTaskData(uint32_t allocationTaskRow,
                                    uint32_t allocationItid,
                                    uint32_t executeId,
                                    uint32_t priority,
                                    uint32_t executeState);
    size_t AppendExecuteTaskData(uint32_t executeTaskRow, uint32_t executeItid, uint32_t executeId);
    size_t AppendReturnTaskData(uint32_t returnTaskRow, uint32_t returnItid, uint32_t executeId, uint32_t returnState);
    void UpdateAllocationTaskData(uint32_t index,
                                  uint32_t allocationTaskRow,
                                  uint32_t allocationItid,
                                  uint32_t priority,
                                  uint32_t executeState);
    void UpdateExecuteTaskData(uint32_t index, uint32_t executeTaskRow, uint32_t executeItid);
    void UpdateReturnTaskData(uint32_t index, uint32_t returnTaskRow, uint32_t returnItid, uint32_t returnState);
    void AppendTimeoutRow(uint32_t index, uint32_t timeoutRow);

    const std::deque<uint32_t>& AllocationTaskRows() const;
    const std::deque<uint32_t>& ExecuteTaskRows() const;
    const std::deque<uint32_t>& ReturnTaskRows() const;
    const std::deque<uint32_t>& AllocationItids() const;
    const std::deque<uint32_t>& ExecuteItids() const;
    const std::deque<uint32_t>& ReturnItids() const;
    const std::deque<uint32_t>& ExecuteIds() const;
    const std::deque<uint32_t>& Prioritys() const;
    const std::deque<uint32_t>& ExecuteStates() const;
    const std::deque<uint32_t>& ReturnStates() const;
    const std::deque<uint32_t>& TimeoutRows() const;
    void Clear() override
    {
        CacheBase::Clear();
        allocationTaskRows_.clear();
        executeTaskRows_.clear();
        returnTaskRows_.clear();
        allocationItids_.clear();
        executeItids_.clear();
        returnItids_.clear();
        executeIds_.clear();
        prioritys_.clear();
        executeStates_.clear();
        returnStates_.clear();
        timeoutRows_.clear();
    }

private:
    std::deque<uint32_t> allocationTaskRows_ = {};
    std::deque<uint32_t> executeTaskRows_ = {};
    std::deque<uint32_t> returnTaskRows_ = {};
    std::deque<uint32_t> allocationItids_ = {};
    std::deque<uint32_t> executeItids_ = {};
    std::deque<uint32_t> returnItids_ = {};
    std::deque<uint32_t> executeIds_ = {};
    std::deque<uint32_t> prioritys_ = {};
    std::deque<uint32_t> executeStates_ = {};
    std::deque<uint32_t> returnStates_ = {};
    std::deque<uint32_t> timeoutRows_ = {};
};
class Animation {
public:
    TableRowId AppendAnimation(InternalTime startPoint);
    void UpdateStartPoint(TableRowId index, InternalTime startPoint);
    void UpdateEndPoint(TableRowId index, InternalTime endPoint);
    size_t Size() const;
    const std::deque<InternalTime>& InputTimes() const;
    const std::deque<InternalTime>& StartPoints() const;
    const std::deque<InternalTime>& EndPoints() const;
    const std::deque<uint64_t>& IdsData() const;
    void Clear();

private:
    std::deque<InternalTime> inputTimes_ = {};
    std::deque<InternalTime> startPoints_ = {};
    std::deque<InternalTime> endPoins_ = {};
    std::deque<uint64_t> ids_ = {};
};
class DeviceInfo {
public:
    uint32_t PhysicalWidth() const;
    uint32_t PhysicalHeight() const;
    uint32_t PhysicalFrameRate() const;
    void UpdateWidthAndHeight(const std::smatch& matcheLine);
    void UpdateFrameRate(uint32_t frameRate);

    void Clear();

private:
    uint32_t physicalWidth_ = INVALID_UINT32;
    uint32_t physicalHeight_ = INVALID_UINT32;
    uint32_t physicalFrameRate_ = INVALID_UINT32;
};
class DynamicFrame {
public:
    TableRowId AppendDynamicFrame(DataIndex nameId);
    void UpdateNameIndex(TableRowId index, DataIndex nameId);
    void UpdatePosition(TableRowId index, const std::smatch& matcheLine, DataIndex alpha);
    void UpdateEndTime(TableRowId index, InternalTime endTime);

    size_t Size() const;
    const std::deque<uint64_t>& IdsData() const;
    const std::deque<uint32_t>& Xs() const;
    const std::deque<uint32_t>& Ys() const;
    const std::deque<uint32_t>& Widths() const;
    const std::deque<uint32_t>& Heights() const;
    const std::deque<DataIndex>& Alphas() const;
    const std::deque<DataIndex>& Names() const;
    const std::deque<InternalTime>& EndTimes() const;
    void Clear();

private:
    std::deque<uint32_t> xs_ = {};
    std::deque<uint32_t> ys_ = {};
    std::deque<uint32_t> widths_ = {};
    std::deque<uint32_t> heights_ = {};
    std::deque<DataIndex> alphas_ = {};
    std::deque<DataIndex> names_ = {};
    std::deque<InternalTime> endTimes_ = {};
    std::deque<uint64_t> ids_ = {};
};

class AshMemData : public CacheBase {
public:
    void AppendNewData(InternalPid ipid,
                       uint64_t ts,
                       uint32_t adj,
                       uint32_t fd,
                       DataIndex ashmemNameId,
                       uint64_t size,
                       uint64_t pss,
                       uint32_t ashmemId,
                       uint64_t time,
                       uint64_t refCount,
                       uint64_t purged,
                       uint32_t flag);
    const std::deque<InternalPid>& Ipids() const;
    const std::deque<uint32_t>& Adjs() const;
    const std::deque<uint32_t>& Fds() const;
    const std::deque<DataIndex>& AshmemNameIds() const;
    const std::deque<uint64_t>& Sizes() const;
    const std::deque<uint64_t>& Psss() const;
    const std::deque<uint32_t>& AshmemIds() const;
    const std::deque<uint64_t>& Times() const;
    const std::deque<uint64_t>& RefCounts() const;
    const std::deque<uint64_t>& Purgeds() const;
    const std::deque<uint32_t>& Flags() const;
    void Clear() override
    {
        CacheBase::Clear();
        ipids_.clear();
        adjs_.clear();
        fds_.clear();
        ashmemNameIds_.clear();
        sizes_.clear();
        psss_.clear();
        ashmemIds_.clear();
        times_.clear();
        refCounts_.clear();
        purgeds_.clear();
        flags_.clear();
    }
    void SetFlag(uint64_t rowId, uint32_t Flag);

private:
    std::deque<InternalPid> ipids_ = {};
    std::deque<uint32_t> adjs_ = {};
    std::deque<uint32_t> fds_ = {};
    std::deque<DataIndex> ashmemNameIds_ = {};
    std::deque<uint64_t> sizes_ = {};
    std::deque<uint64_t> psss_ = {};
    std::deque<uint32_t> ashmemIds_ = {};
    std::deque<uint64_t> times_ = {};
    std::deque<uint64_t> refCounts_ = {};
    std::deque<uint64_t> purgeds_ = {};
    std::deque<uint32_t> flags_ = {};
    uint32_t rowCount_ = 0;
};

class DmaMemData : public CacheBase {
public:
    void AppendNewData(InternalPid ipid,
                       uint64_t ts,
                       uint32_t fd,
                       uint64_t size,
                       uint32_t ino,
                       uint32_t expPid,
                       DataIndex expTaskCommId,
                       DataIndex bufNameId,
                       DataIndex expNameId,
                       uint32_t flag);
    const std::deque<InternalPid>& Ipids() const;
    const std::deque<uint32_t>& Fds() const;
    const std::deque<uint64_t>& Sizes() const;
    const std::deque<uint32_t>& Inos() const;
    const std::deque<uint32_t>& ExpPids() const;
    const std::deque<DataIndex>& ExpTaskCommIds() const;
    const std::deque<DataIndex>& BufNameIds() const;
    const std::deque<DataIndex>& ExpNameIds() const;
    const std::deque<uint32_t>& Flags() const;
    void Clear() override
    {
        CacheBase::Clear();
        ipids_.clear();
        fds_.clear();
        sizes_.clear();
        inos_.clear();
        expPids_.clear();
        expTaskCommIds_.clear();
        bufNameIds_.clear();
        expNameIds_.clear();
        flags_.clear();
    }
    void SetFlag(uint64_t rowId, uint32_t Flag);

private:
    std::deque<InternalPid> ipids_ = {};
    std::deque<uint32_t> fds_ = {};
    std::deque<uint64_t> sizes_ = {};
    std::deque<uint32_t> inos_ = {};
    std::deque<uint32_t> expPids_ = {};
    std::deque<DataIndex> expTaskCommIds_ = {};
    std::deque<DataIndex> bufNameIds_ = {};
    std::deque<DataIndex> expNameIds_ = {};
    std::deque<uint32_t> flags_ = {};
    uint32_t rowCount_ = 0;
};

class GpuProcessMemData : public CacheBase {
public:
    void AppendNewData(uint64_t ts,
                       DataIndex gpuNameId,
                       uint64_t allGpuSize,
                       std::string addr,
                       InternalPid ipid,
                       InternalPid itid,
                       uint64_t usedGpuSize);
    const std::deque<DataIndex>& GpuNameIds() const;
    const std::deque<uint64_t>& AllGpuSizes() const;
    const std::deque<std::string>& Addrs() const;
    const std::deque<InternalPid>& Ipids() const;
    const std::deque<InternalPid>& Itids() const;
    const std::deque<uint64_t>& UsedGpuSizes() const;
    void Clear() override
    {
        CacheBase::Clear();
        gpuNameIds_.clear();
        allGpuSizes_.clear();
        addrs_.clear();
        ipids_.clear();
        itids_.clear();
        usedGpuSizes_.clear();
    }

private:
    std::deque<DataIndex> gpuNameIds_ = {};
    std::deque<uint64_t> allGpuSizes_ = {};
    std::deque<std::string> addrs_ = {};
    std::deque<InternalPid> ipids_ = {};
    std::deque<InternalPid> itids_ = {};
    std::deque<uint64_t> usedGpuSizes_ = {};
    uint32_t rowCount_ = 0;
};

class GpuWindowMemData : public CacheBase {
public:
    void AppendNewData(uint64_t ts,
                       DataIndex windowNameId,
                       uint64_t windowId,
                       DataIndex moduleNameId,
                       DataIndex categoryNameId,
                       uint64_t size,
                       uint32_t count,
                       uint64_t purgeableSize);
    const std::deque<DataIndex>& WindowNameIds() const;
    const std::deque<uint64_t>& WindowIds() const;
    const std::deque<DataIndex>& ModuleNameIds() const;
    const std::deque<DataIndex>& CategoryNameIds() const;
    const std::deque<uint64_t>& Sizes() const;
    const std::deque<uint32_t>& Counts() const;
    const std::deque<uint64_t>& PurgeableSizes() const;
    void Clear() override
    {
        CacheBase::Clear();
        windowNameIds_.clear();
        windowIds_.clear();
        moduleNameIds_.clear();
        categoryNameIds_.clear();
        sizes_.clear();
        counts_.clear();
        purgeableSizes_.clear();
    }

private:
    std::deque<DataIndex> windowNameIds_ = {};
    std::deque<uint64_t> windowIds_ = {};
    std::deque<DataIndex> moduleNameIds_ = {};
    std::deque<DataIndex> categoryNameIds_ = {};
    std::deque<uint64_t> sizes_ = {};
    std::deque<uint32_t> counts_ = {};
    std::deque<uint64_t> purgeableSizes_ = {};
    uint32_t rowCount_ = 0;
};

} // namespace TraceStdtype
} // namespace SysTuning

#endif // TRACE_STDTYPE_H
