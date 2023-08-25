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

#include <fcntl.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <memory>

#include "htrace_native_hook_parser.h"
#include "native_hook_result.pb.h"
#include "native_hook_result.pbreader.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "parser/common_types.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
const uint64_t TV_SEC_01 = 1632675525;
const uint64_t TV_SEC_02 = 1632675526;
const uint64_t TV_SEC_03 = 1632675527;
const uint64_t TV_SEC_04 = 1632675528;
const uint64_t TV_NSEC_01 = 996560701;
const uint64_t TV_NSEC_02 = 999560702;
const uint64_t TV_NSEC_03 = 996560703;
const uint64_t TV_NSEC_04 = 999560704;
const uint64_t TIMESTAMP_01 = TV_NSEC_01 + TV_SEC_01 * SEC_TO_NS;
const uint64_t TIMESTAMP_02 = TV_NSEC_02 + TV_SEC_02 * SEC_TO_NS;
const uint64_t TIMESTAMP_03 = TV_NSEC_03 + TV_SEC_03 * SEC_TO_NS;
const uint64_t TIMESTAMP_04 = TV_NSEC_04 + TV_SEC_04 * SEC_TO_NS;
const int32_t PID = 2716;
const int32_t TID_01 = 1532;
const int32_t TID_02 = 1532;
const uint64_t MEM_ADDR_01 = 10453088;
const uint64_t MEM_ADDR_02 = 10453089;
const uint64_t MEM_ADDR_03 = 10453090;
const int64_t MEM_SIZE_01 = 4096;
const int64_t MEM_SIZE_02 = 2048;
const uint64_t CALL_STACK_IP_01 = 4154215627;
const uint64_t CALL_STACK_IP_02 = 4154215630;
const uint64_t CALL_STACK_SP_01 = 4146449696;
const uint64_t CALL_STACK_SP_02 = 4146449698;
const std::string SYMBOL_NAME_01 = "__aeabi_read_tp";
const std::string SYMBOL_NAME_02 = "ThreadMmap";
const std::string FILE_PATH_01 = "/system/lib/ld-musl-arm.so.1";
const std::string FILE_PATH_02 = "/system/bin/nativetest_c";
const uint64_t OFFSET_01 = 359372;
const uint64_t OFFSET_02 = 17865;
const uint64_t SYMBOL_OFFSET_01 = 255;
const uint64_t SYMBOL_OFFSET_02 = 33;
const std::string ALLOCEVENT = "AllocEvent";
const std::string FREEEVENT = "FreeEvent";
const std::string MMAPEVENT = "MmapEvent";
const std::string MUNMAPEVENT = "MunmapEvent";
const std::string MMAP_SUB_TYPE_01 = "mmapType1";
const std::string MMAP_SUB_TYPE_02 = "mmapType2";
class NativeHookParserTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
};

class NativeHookCache {
public:
    NativeHookCache(const uint64_t callChainId,
                    const uint32_t ipid,
                    const uint32_t itid,
                    const std::string eventType,
                    const uint64_t subType,
                    const uint64_t startTimeStamp,
                    const uint64_t endTimeStamp,
                    const uint64_t duration,
                    const uint64_t address,
                    const int64_t memSize,
                    const int64_t allMemSize,
                    const uint64_t currentSizeDurations)
        : callChainId_(callChainId),
          ipid_(ipid),
          itid_(itid),
          eventType_(eventType),
          subType_(subType),
          startTimeStamp_(startTimeStamp),
          endTimeStamp_(endTimeStamp),
          duration_(duration),
          address_(address),
          memSize_(memSize),
          allMemSize_(allMemSize),
          currentSizeDurations_(currentSizeDurations)
    {
    }

    NativeHookCache(const NativeHook& nativeHook, uint64_t index)
    {
        if (nativeHook.Size() <= index) {
            TS_LOGE("index out of deque bounds! nativeHook.Size() = %lu, index = %lu", nativeHook.Size(), index);
            return;
        }
        callChainId_ = nativeHook.CallChainIds()[index];
        ipid_ = nativeHook.Ipids()[index];
        itid_ = nativeHook.Itids()[index];
        eventType_ = nativeHook.EventTypes()[index];
        subType_ = nativeHook.SubTypes()[index];
        startTimeStamp_ = nativeHook.TimeStampData()[index];
        endTimeStamp_ = nativeHook.EndTimeStamps()[index];
        duration_ = nativeHook.Durations()[index];
        address_ = nativeHook.Addrs()[index];
        memSize_ = nativeHook.MemSizes()[index];
        allMemSize_ = nativeHook.AllMemSizes()[index];
        currentSizeDurations_ = nativeHook.CurrentSizeDurs()[index];
    }
    ~NativeHookCache() = default;
    NativeHookCache(const NativeHookCache&) = delete;
    NativeHookCache& operator=(const NativeHookCache&) = delete;
    bool operator==(const NativeHookCache& nativeHookCache) const
    {
        if (nativeHookCache.GetCallChainId() != callChainId_) {
            TS_LOGE("callChainId_ = %lu, nativeHookCache.GetCallChainId() = %lu", callChainId_,
                    nativeHookCache.GetCallChainId());
            return false;
        }
        if (nativeHookCache.GetPid() != ipid_) {
            TS_LOGE("ipid_ = %u, nativeHookCache.GetPid() = %u", ipid_, nativeHookCache.GetPid());
            return false;
        }
        if (nativeHookCache.GetTid() != itid_) {
            TS_LOGE("itid_ = %u, nativeHookCache.GetTid() = %u", nativeHookCache.GetTid(), itid_);
            return false;
        }
        if (nativeHookCache.GetEventType() != eventType_) {
            TS_LOGE("eventType_ = %s, nativeHookCache.GetEventType() = %s", eventType_.c_str(),
                    nativeHookCache.GetEventType().c_str());
            return false;
        }
        if (nativeHookCache.GetSubType() != subType_) {
            TS_LOGE("subType_ = %lu, nativeHookCache.GetSubType() = %lu", subType_, nativeHookCache.GetSubType());
            return false;
        }
        if (nativeHookCache.GetStartTimeStamp() != startTimeStamp_) {
            TS_LOGE("startTimeStamp_ = %lu, nativeHookCache.GetStartTimeStamp() = %lu", startTimeStamp_,
                    nativeHookCache.GetStartTimeStamp());
            return false;
        }
        if (nativeHookCache.GetEndTimeStamp() != endTimeStamp_) {
            TS_LOGE(" endTimeStamp_ = %lu, nativeHookCache.GetEndTimeStamp() = %lu", endTimeStamp_,
                    nativeHookCache.GetEndTimeStamp());
            return false;
        }
        if (nativeHookCache.GetDuration() != duration_) {
            TS_LOGE("duration_ = %lu, nativeHookCache.GetDuration() = %lu", duration_, nativeHookCache.GetDuration());
            return false;
        }
        if (nativeHookCache.GetEndTimeStamp() != endTimeStamp_) {
            TS_LOGE("address_ = %lu, nativeHookCache.GetAddress() = %lu", address_, nativeHookCache.GetAddress());
            return false;
        }
        if (nativeHookCache.GetMemSize() != memSize_) {
            TS_LOGE("memSize_ = %ld, nativeHookCache.GetMemSize() = %ld", memSize_, nativeHookCache.GetMemSize());
            return false;
        }
        if (nativeHookCache.GetAllMemSize() != allMemSize_) {
            TS_LOGE("allMemSize_ = %lu, nativeHookCache.GetAllMemSize() = %lu", allMemSize_,
                    nativeHookCache.GetAllMemSize());
            return false;
        }
        if (nativeHookCache.GetCurrentSizeDuration() != currentSizeDurations_) {
            TS_LOGE("currentSizeDurations_ = %lu, nativeHookCache.GetCurrentSizeDuration() = %lu",
                    currentSizeDurations_, nativeHookCache.GetCurrentSizeDuration());
            return false;
        }
        return true;
    }
    inline uint64_t GetCallChainId() const
    {
        return callChainId_;
    }
    inline uint32_t GetPid() const
    {
        return ipid_;
    }
    inline uint32_t GetTid() const
    {
        return itid_;
    }
    inline std::string GetEventType() const
    {
        return eventType_;
    }
    inline uint64_t GetSubType() const
    {
        return subType_;
    }
    inline uint64_t GetStartTimeStamp() const
    {
        return startTimeStamp_;
    }
    inline uint64_t GetEndTimeStamp() const
    {
        return endTimeStamp_;
    }
    inline uint64_t GetDuration() const
    {
        return duration_;
    }
    inline uint64_t GetAddress() const
    {
        return address_;
    }
    inline int64_t GetMemSize() const
    {
        return memSize_;
    }
    inline int64_t GetAllMemSize() const
    {
        return allMemSize_;
    }
    inline uint64_t GetCurrentSizeDuration() const
    {
        return currentSizeDurations_;
    }

private:
    uint64_t callChainId_;
    uint32_t ipid_;
    uint32_t itid_;
    std::string eventType_;
    uint64_t subType_;
    uint64_t startTimeStamp_;
    uint64_t endTimeStamp_;
    uint64_t duration_;
    uint64_t address_;
    int64_t memSize_;
    int64_t allMemSize_;
    uint64_t currentSizeDurations_;
};

class NativeHookFrameCache {
public:
    NativeHookFrameCache(const uint64_t callChainId,
                         const uint64_t depth,
                         const uint64_t ip,
                         const uint64_t sp,
                         const uint64_t symbolName,
                         const uint64_t filePath,
                         const uint64_t offset,
                         const uint64_t symbolOffset)
        : callChainId_(callChainId),
          depth_(depth),
          ip_(ip),
          sp_(sp),
          symbolName_(symbolName),
          filePath_(filePath),
          offset_(offset),
          symbolOffset_(symbolOffset)
    {
    }

    NativeHookFrameCache(const NativeHookFrame& nativeHookFrame, const uint64_t index)
    {
        if (nativeHookFrame.Size() <= index) {
            TS_LOGE("index out of deque bounds! nativeHookFrame.Size() = %lu, index = %lu", nativeHookFrame.Size(),
                    index);
            return;
        }
        callChainId_ = nativeHookFrame.CallChainIds()[index];
        depth_ = nativeHookFrame.Depths()[index];
        ip_ = nativeHookFrame.Ips()[index];
        sp_ = nativeHookFrame.Sps()[index];
        symbolName_ = nativeHookFrame.SymbolNames()[index];
        filePath_ = nativeHookFrame.FilePaths()[index];
        offset_ = nativeHookFrame.Offsets()[index];
        symbolOffset_ = nativeHookFrame.SymbolOffsets()[index];
    }

    ~NativeHookFrameCache() = default;
    NativeHookFrameCache(const NativeHookFrameCache&) = delete;
    NativeHookFrameCache& operator=(const NativeHookFrameCache&) = delete;
    bool operator==(const NativeHookFrameCache& frameCache) const
    {
        if (frameCache.GetCallChainId() != callChainId_) {
            TS_LOGE("callChainId_ = %lu, frameCache.GetCallChainId() = %lu", callChainId_, frameCache.GetCallChainId());
            return false;
        }
        if (frameCache.GetDepth() != depth_) {
            TS_LOGE("depth_ = %lu, frameCache.GetDepth() = %lu", depth_, frameCache.GetDepth());
            return false;
        }
        if (frameCache.GetIp() != ip_) {
            TS_LOGE("ip_ = %lu, frameCache.GetIp() = %lu", ip_, frameCache.GetIp());
            return false;
        }
        if (frameCache.GetSp() != sp_) {
            TS_LOGE("sp_ = %lu, frameCache.GetSp() = %lu", sp_, frameCache.GetSp());
            return false;
        }
        if (frameCache.GetSymbolName() != symbolName_) {
            TS_LOGE("symbolName_ = %lu, frameCache.GetSymbolName() = %lu", symbolName_, frameCache.GetSymbolName());
            return false;
        }
        if (frameCache.GetFilePath() != filePath_) {
            TS_LOGE("filePath_ = %lu, frameCache.GetFilePath() = %lu", filePath_, frameCache.GetFilePath());
            return false;
        }
        if (frameCache.GetOffset() != offset_) {
            TS_LOGE("offset_ = %lu, frameCache.GetOffset() = %lu", offset_, frameCache.GetOffset());
            return false;
        }
        if (frameCache.GetSymbolOffset() != symbolOffset_) {
            TS_LOGE("symbolOffset_ = %lu, frameCache.GetSymbolName() = %lu", symbolOffset_, frameCache.GetSymbolName());
            return false;
        }
        return true;
    }
    inline uint64_t GetCallChainId() const
    {
        return callChainId_;
    }
    inline uint64_t GetDepth() const
    {
        return depth_;
    }
    inline uint64_t GetIp() const
    {
        return ip_;
    }
    inline uint64_t GetSp() const
    {
        return sp_;
    }
    inline uint64_t GetSymbolName() const
    {
        return symbolName_;
    }
    inline uint64_t GetFilePath() const
    {
        return filePath_;
    }
    inline uint64_t GetOffset() const
    {
        return offset_;
    }
    inline uint64_t GetSymbolOffset() const
    {
        return symbolOffset_;
    }

private:
    uint64_t callChainId_;
    uint64_t depth_;
    uint64_t ip_;
    uint64_t sp_;
    uint64_t symbolName_;
    uint64_t filePath_;
    uint64_t offset_;
    uint64_t symbolOffset_;
};

/**
 * @tc.name: ParseBatchNativeHookWithOutNativeHookData
 * @tc.desc: Parse a BatchNativeHookData that does not contain any NativeHookData
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithOutNativeHookData, TestSize.Level1)
{
    TS_LOGI("test24-1");
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    auto size = stream_.traceDataCache_->GetConstHilogData().Size();
    EXPECT_FALSE(size);
}

/**
 * @tc.name: ParseBatchNativeHookWithOneMalloc
 * @tc.desc: Parse a BatchNativeHookData with only one Malloc
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithOneMalloc, TestSize.Level1)
{
    TS_LOGI("test24-2");
    // construct AllocEvent
    AllocEvent* allocEvent = new AllocEvent();
    allocEvent->set_pid(PID);
    allocEvent->set_tid(TID_01);
    allocEvent->set_addr(MEM_ADDR_01);
    allocEvent->set_size(MEM_SIZE_01);
    // construct AllocEvent's Frame
    auto frame = allocEvent->add_frame_info();
    frame->set_ip(CALL_STACK_IP_01);
    frame->set_sp(CALL_STACK_SP_01);
    frame->set_symbol_name(SYMBOL_NAME_01);
    frame->set_file_path(FILE_PATH_01);
    frame->set_offset(OFFSET_01);
    frame->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookData = batchNativeHookData->add_events();
    nativeHookData->set_tv_sec(TV_SEC_01);
    nativeHookData->set_tv_nsec(TV_NSEC_01);
    nativeHookData->set_allocated_alloc_event(allocEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;

    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    NativeHookCache expectNativeHookCache(1, expect_ipid, expect_itid, ALLOCEVENT.c_str(), INVALID_UINT64, TIMESTAMP_01,
                                          0, 0, MEM_ADDR_01, MEM_SIZE_01, MEM_SIZE_01, 0);
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    NativeHookCache resultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(expectNativeHookCache == resultNativeHookCache);

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(1, size);

    // Verification parse NativeHook Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    auto expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_01);
    auto expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_01);
    NativeHookFrameCache expectFrameCache(1, 0, CALL_STACK_IP_01, CALL_STACK_SP_01, expectSymbolData,
                                          expectFilePathData, OFFSET_01, SYMBOL_OFFSET_01);
    NativeHookFrameCache resultFrameCache(nativeHookFrame, 0);
    EXPECT_TRUE(expectFrameCache == resultFrameCache);

    size = nativeHookFrame.Size();
    EXPECT_EQ(1, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MALLOC, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithMultipleMalloc
 * @tc.desc: Parse a NativeHook with multiple Malloc and Frame
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithMultipleMalloc, TestSize.Level1)
{
    TS_LOGI("test24-3");
    // constructs first AllocEvent with two Frame
    AllocEvent* firstAllocEvent = new AllocEvent();
    firstAllocEvent->set_pid(PID);
    firstAllocEvent->set_tid(TID_01);
    firstAllocEvent->set_addr(MEM_ADDR_01);
    firstAllocEvent->set_size(MEM_SIZE_01);
    // construct first AllocEvent's first Frame
    auto firstAllocFirstFrame = firstAllocEvent->add_frame_info();
    firstAllocFirstFrame->set_ip(CALL_STACK_IP_01);
    firstAllocFirstFrame->set_sp(CALL_STACK_SP_01);
    firstAllocFirstFrame->set_symbol_name(SYMBOL_NAME_01);
    firstAllocFirstFrame->set_file_path(FILE_PATH_01);
    firstAllocFirstFrame->set_offset(OFFSET_01);
    firstAllocFirstFrame->set_symbol_offset(SYMBOL_OFFSET_01);
    // construct first AllocEvent's second Frame
    auto firstAllocSecondFrame = firstAllocEvent->add_frame_info();
    firstAllocSecondFrame->set_ip(CALL_STACK_IP_02);
    firstAllocSecondFrame->set_sp(CALL_STACK_SP_02);
    firstAllocSecondFrame->set_symbol_name(SYMBOL_NAME_02);
    firstAllocSecondFrame->set_file_path(FILE_PATH_02);
    firstAllocSecondFrame->set_offset(OFFSET_02);
    firstAllocSecondFrame->set_symbol_offset(SYMBOL_OFFSET_02);

    // constructs second AllocEvent with two Frame
    AllocEvent* secondAllocEvent = new AllocEvent();
    secondAllocEvent->set_pid(PID);
    secondAllocEvent->set_tid(TID_02);
    secondAllocEvent->set_addr(MEM_ADDR_02);
    secondAllocEvent->set_size(MEM_SIZE_02);
    // construct second AllocEvent's first Frame
    auto secondAllocFirstFrame = secondAllocEvent->add_frame_info();
    secondAllocFirstFrame->set_ip(CALL_STACK_IP_01);
    secondAllocFirstFrame->set_sp(CALL_STACK_SP_01);
    secondAllocFirstFrame->set_symbol_name(SYMBOL_NAME_01);
    secondAllocFirstFrame->set_file_path(FILE_PATH_01);
    secondAllocFirstFrame->set_offset(OFFSET_01);
    secondAllocFirstFrame->set_symbol_offset(SYMBOL_OFFSET_01);
    // construct second AllocEvent's second Frame
    auto secondAllocSecondFrame = secondAllocEvent->add_frame_info();
    secondAllocSecondFrame->set_ip(CALL_STACK_IP_02);
    secondAllocSecondFrame->set_sp(CALL_STACK_SP_02);
    secondAllocSecondFrame->set_symbol_name(SYMBOL_NAME_02);
    secondAllocSecondFrame->set_file_path(FILE_PATH_02);
    secondAllocSecondFrame->set_offset(OFFSET_02);
    secondAllocSecondFrame->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add first NativeHookData
    auto firstNativeHookData = batchNativeHookData->add_events();
    firstNativeHookData->set_tv_sec(TV_SEC_01);
    firstNativeHookData->set_tv_nsec(TV_NSEC_01);
    firstNativeHookData->set_allocated_alloc_event(firstAllocEvent);
    // add second NativeHookData
    auto secondNativeHookData = batchNativeHookData->add_events();
    secondNativeHookData->set_tv_sec(TV_SEC_02);
    secondNativeHookData->set_tv_nsec(TV_NSEC_02);
    secondNativeHookData->set_allocated_alloc_event(secondAllocEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;

    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    // Calculate partial expectations
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    EXPECT_TRUE(MEM_SIZE_01 == nativeHook.AllMemSizes()[0]);
    EXPECT_TRUE(nativeHook.CurrentSizeDurs()[0] == TIMESTAMP_02 - TIMESTAMP_01);

    // Construct the nativehookcache object using the element with subscript 0 in nativehook and compare it with the
    // expected value
    NativeHookCache firstExpectNativeHookCache(1, expect_ipid, expect_itid, ALLOCEVENT.c_str(), INVALID_UINT64,
                                               TIMESTAMP_01, 0, 0, MEM_ADDR_01, MEM_SIZE_01, MEM_SIZE_01,
                                               TIMESTAMP_02 - TIMESTAMP_01);
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    // construct first Malloc event's first frame expect value.
    // Note: the nativehookframe data is parsed in reverse order
    auto firstExpectSymbol = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_02);
    auto firstExpectFilePath = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_02);
    auto secondExpectSymbol = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_01);
    auto secondExpectFilePath = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_01);
    NativeHookFrameCache firstMallocExpectFirstFrame(1, 0, CALL_STACK_IP_02, CALL_STACK_SP_02, firstExpectSymbol,
                                                     firstExpectFilePath, OFFSET_02, SYMBOL_OFFSET_02);
    // Construct the NativeHookFrameCache object using the element with subscript 0 in NativeHookFrame and compare it
    // with the expected value
    NativeHookFrameCache firstMallocResultFirstFrame(nativeHookFrame, 0);
    EXPECT_TRUE(firstMallocExpectFirstFrame == firstMallocResultFirstFrame);

    // construct first Malloc event's second frame expect value.
    NativeHookFrameCache firstMallocExpectSecondFrame(1, 1, CALL_STACK_IP_01, CALL_STACK_SP_01, secondExpectSymbol,
                                                      secondExpectFilePath, OFFSET_01, SYMBOL_OFFSET_01);
    NativeHookFrameCache firstMallocResultSecondFrame(nativeHookFrame, 1);
    EXPECT_TRUE(firstMallocExpectSecondFrame == firstMallocResultSecondFrame);

    // Construct the nativehookcache object using the element with subscript 1 in nativehook and compare it with the
    // expected value
    expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_02);
    NativeHookCache secondExpectNativeHookCache(1, expect_ipid, expect_itid, ALLOCEVENT.c_str(), INVALID_UINT64,
                                                TIMESTAMP_02, 0, 0, MEM_ADDR_02, MEM_SIZE_02, MEM_SIZE_01 + MEM_SIZE_02,
                                                0);
    NativeHookCache secondResultNativeHookCache(nativeHook, 1);
    EXPECT_TRUE(secondExpectNativeHookCache == secondResultNativeHookCache);

    // construct second Malloc event's first frame expect value.
    // Note: the nativehookframe data is parsed in reverse order
    NativeHookFrameCache secondMallocExpectFirstFrame(1, 0, CALL_STACK_IP_02, CALL_STACK_SP_02, firstExpectSymbol,
                                                      firstExpectFilePath, OFFSET_02, SYMBOL_OFFSET_02);
    // Construct the NativeHookFrameCache object using the element with subscript 2 in NativeHookFrame and compare it
    // Verify the compression algorithm here=
    EXPECT_EQ(nativeHookFrame.CallChainIds()[1], 1);
    EXPECT_EQ(nativeHookFrame.Depths()[1], 1);
    EXPECT_EQ(nativeHookFrame.Ips()[1], CALL_STACK_IP_01);
    EXPECT_EQ(nativeHookFrame.Sps()[1], CALL_STACK_SP_01);
    EXPECT_EQ(nativeHookFrame.SymbolNames()[1], secondExpectSymbol);
    EXPECT_EQ(nativeHookFrame.FilePaths()[1], secondExpectFilePath);
    EXPECT_EQ(nativeHookFrame.Offsets()[1], OFFSET_01);
    EXPECT_EQ(nativeHookFrame.SymbolOffsets()[1], SYMBOL_OFFSET_01);
}

/**
 * @tc.name: ParseBatchNativeHookWithOneFree
 * @tc.desc: Parse a BatchNativeHookData with only one Free
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithOneFree, TestSize.Level1)
{
    TS_LOGI("test24-4");
    // construct FreeEvent
    FreeEvent* freeEvent = new FreeEvent();
    freeEvent->set_pid(PID);
    freeEvent->set_tid(TID_01);
    freeEvent->set_addr(MEM_ADDR_01);
    // construct FreeEvent's Frame
    auto frame = freeEvent->add_frame_info();
    frame->set_ip(CALL_STACK_IP_01);
    frame->set_sp(CALL_STACK_SP_01);
    frame->set_symbol_name(SYMBOL_NAME_01);
    frame->set_file_path(FILE_PATH_01);
    frame->set_offset(OFFSET_01);
    frame->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookData = batchNativeHookData->add_events();
    nativeHookData->set_tv_sec(TV_SEC_01);
    nativeHookData->set_tv_nsec(TV_NSEC_01);
    nativeHookData->set_allocated_free_event(freeEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;

    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(0, size);
    size = stream_.traceDataCache_->GetConstNativeHookFrameData().Size();
    EXPECT_EQ(0, size);
    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithMultipleFree
 * @tc.desc: Parse a NativeHook with multiple Free and Frame
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithMultipleFree, TestSize.Level1)
{
    TS_LOGI("test24-5");
    // constructs first FreeEvent with two Frame
    FreeEvent* firstFreeEvent = new FreeEvent();
    firstFreeEvent->set_pid(PID);
    firstFreeEvent->set_tid(TID_01);
    firstFreeEvent->set_addr(MEM_ADDR_01);
    // construct first FreeEvent's first Frame
    auto firstFreeFirstFrame = firstFreeEvent->add_frame_info();
    firstFreeFirstFrame->set_ip(CALL_STACK_IP_01);
    firstFreeFirstFrame->set_sp(CALL_STACK_SP_01);
    firstFreeFirstFrame->set_symbol_name(SYMBOL_NAME_01);
    firstFreeFirstFrame->set_file_path(FILE_PATH_01);
    firstFreeFirstFrame->set_offset(OFFSET_01);
    firstFreeFirstFrame->set_symbol_offset(SYMBOL_OFFSET_01);
    // construct first FreeEvent's second Frame
    auto firstFreeSecondFrame = firstFreeEvent->add_frame_info();
    firstFreeSecondFrame->set_ip(CALL_STACK_IP_02);
    firstFreeSecondFrame->set_sp(CALL_STACK_SP_02);
    firstFreeSecondFrame->set_symbol_name(SYMBOL_NAME_02);
    firstFreeSecondFrame->set_file_path(FILE_PATH_02);
    firstFreeSecondFrame->set_offset(OFFSET_02);
    firstFreeSecondFrame->set_symbol_offset(SYMBOL_OFFSET_02);

    // constructs second FreeEvent with two Frame
    FreeEvent* secondFreeEvent = new FreeEvent();
    secondFreeEvent->set_pid(PID);
    secondFreeEvent->set_tid(TID_02);
    secondFreeEvent->set_addr(MEM_ADDR_02);
    // construct second FreeEvent's first Frame
    auto secondFreeFirstFrame = secondFreeEvent->add_frame_info();
    secondFreeFirstFrame->set_ip(CALL_STACK_IP_01);
    secondFreeFirstFrame->set_sp(CALL_STACK_SP_01);
    secondFreeFirstFrame->set_symbol_name(SYMBOL_NAME_01);
    secondFreeFirstFrame->set_file_path(FILE_PATH_01);
    secondFreeFirstFrame->set_offset(OFFSET_01);
    secondFreeFirstFrame->set_symbol_offset(SYMBOL_OFFSET_01);
    // construct second FreeEvent's second Frame
    auto secondFreeSecondFrame = secondFreeEvent->add_frame_info();
    secondFreeSecondFrame->set_ip(CALL_STACK_IP_02);
    secondFreeSecondFrame->set_sp(CALL_STACK_SP_02);
    secondFreeSecondFrame->set_symbol_name(SYMBOL_NAME_02);
    secondFreeSecondFrame->set_file_path(FILE_PATH_02);
    secondFreeSecondFrame->set_offset(OFFSET_02);
    secondFreeSecondFrame->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add first NativeHookData
    auto firstNativeHookData = batchNativeHookData->add_events();
    firstNativeHookData->set_tv_sec(TV_SEC_01);
    firstNativeHookData->set_tv_nsec(TV_NSEC_01);
    firstNativeHookData->set_allocated_free_event(firstFreeEvent);
    // add second NativeHookData
    auto secondNativeHookData = batchNativeHookData->add_events();
    secondNativeHookData->set_tv_sec(TV_SEC_02);
    secondNativeHookData->set_tv_nsec(TV_NSEC_02);
    secondNativeHookData->set_allocated_free_event(secondFreeEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;

    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    // Calculate partial expectations
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    EXPECT_TRUE(0 == nativeHook.Size());
    EXPECT_TRUE(0 == nativeHookFrame.Size());

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithOnePairsMallocAndFree
 * @tc.desc: Parse a BatchNativeHookData with a pair of matching Malloc and Free Event
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithOnePairsMallocAndFree, TestSize.Level1)
{
    TS_LOGI("test24-6");
    // construct AllocEvent
    AllocEvent* allocEvent = new AllocEvent();
    allocEvent->set_pid(PID);
    allocEvent->set_tid(TID_01);
    allocEvent->set_addr(MEM_ADDR_01);
    allocEvent->set_size(MEM_SIZE_01);
    // construct AllocEvent's Frame
    auto allocframe = allocEvent->add_frame_info();
    allocframe->set_ip(CALL_STACK_IP_01);
    allocframe->set_sp(CALL_STACK_SP_01);
    allocframe->set_symbol_name(SYMBOL_NAME_01);
    allocframe->set_file_path(FILE_PATH_01);
    allocframe->set_offset(OFFSET_01);
    allocframe->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct FreeEvent
    FreeEvent* freeEvent = new FreeEvent();
    freeEvent->set_pid(PID);
    freeEvent->set_tid(TID_02);
    freeEvent->set_addr(MEM_ADDR_01);
    // construct FreeEvent's Frame
    auto freeframe = freeEvent->add_frame_info();
    freeframe->set_ip(CALL_STACK_IP_02);
    freeframe->set_sp(CALL_STACK_SP_02);
    freeframe->set_symbol_name(SYMBOL_NAME_02);
    freeframe->set_file_path(FILE_PATH_02);
    freeframe->set_offset(OFFSET_02);
    freeframe->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookMallocData = batchNativeHookData->add_events();
    nativeHookMallocData->set_tv_sec(TV_SEC_01);
    nativeHookMallocData->set_tv_nsec(TV_NSEC_01);
    nativeHookMallocData->set_allocated_alloc_event(allocEvent);
    auto nativeHookFreeData = batchNativeHookData->add_events();
    nativeHookFreeData->set_tv_sec(TV_SEC_02);
    nativeHookFreeData->set_tv_nsec(TV_NSEC_02);
    nativeHookFreeData->set_allocated_free_event(freeEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;

    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse Malloc event results
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    NativeHookCache firstExpectNativeHookCache(1, expect_ipid, expect_itid, ALLOCEVENT.c_str(), INVALID_UINT64,
                                               TIMESTAMP_01, TIMESTAMP_02, TIMESTAMP_02 - TIMESTAMP_01, MEM_ADDR_01,
                                               MEM_SIZE_01, MEM_SIZE_01, TIMESTAMP_02 - TIMESTAMP_01);
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    // Verification parse Malloc Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    auto expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_01);
    auto expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_01);
    NativeHookFrameCache secondExpectFrameCache(1, 0, CALL_STACK_IP_01, CALL_STACK_SP_01, expectSymbolData,
                                                expectFilePathData, OFFSET_01, SYMBOL_OFFSET_01);
    NativeHookFrameCache secondResultFrameCache(nativeHookFrame, 0);
    EXPECT_TRUE(secondExpectFrameCache == secondResultFrameCache);

    // Verification parse Free event results
    expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_02);
    NativeHookCache expectNativeHookCache(2, expect_ipid, expect_itid, FREEEVENT.c_str(), INVALID_UINT64, TIMESTAMP_02,
                                          0, 0, MEM_ADDR_01, MEM_SIZE_01, 0, 0);
    NativeHookCache resultNativeHookCache(nativeHook, 1);
    EXPECT_TRUE(expectNativeHookCache == resultNativeHookCache);

    // Verification parse Free Event Frame results
    expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_02);
    expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_02);
    NativeHookFrameCache expectFrameCache(2, 0, CALL_STACK_IP_02, CALL_STACK_SP_02, expectSymbolData,
                                          expectFilePathData, OFFSET_02, SYMBOL_OFFSET_02);
    NativeHookFrameCache resultFrameCache(nativeHookFrame, 1);
    EXPECT_TRUE(expectFrameCache == resultFrameCache);

    auto size = nativeHookFrame.Size();
    EXPECT_EQ(2, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MALLOC, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithNotMatchMallocAndFree
 * @tc.desc: Parse a BatchNativeHookData with Not Match Malloc and Free Event
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithNotMatchMallocAndFree, TestSize.Level1)
{
    TS_LOGI("test24-7");
    // construct AllocEvent
    AllocEvent* allocEvent = new AllocEvent();
    allocEvent->set_pid(PID);
    allocEvent->set_tid(TID_01);
    allocEvent->set_addr(MEM_ADDR_01);
    allocEvent->set_size(MEM_SIZE_01);
    // construct AllocEvent's Frame
    auto allocframe = allocEvent->add_frame_info();
    allocframe->set_ip(CALL_STACK_IP_01);
    allocframe->set_sp(CALL_STACK_SP_01);
    allocframe->set_symbol_name(SYMBOL_NAME_01);
    allocframe->set_file_path(FILE_PATH_01);
    allocframe->set_offset(OFFSET_01);
    allocframe->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct FreeEvent
    FreeEvent* freeEvent = new FreeEvent();
    freeEvent->set_pid(PID);
    freeEvent->set_tid(TID_02);
    freeEvent->set_addr(MEM_ADDR_02);
    auto freeframe = freeEvent->add_frame_info();
    // construct FreeEvent's Frame
    freeframe->set_ip(CALL_STACK_IP_02);
    freeframe->set_sp(CALL_STACK_SP_02);
    freeframe->set_symbol_name(SYMBOL_NAME_02);
    freeframe->set_file_path(FILE_PATH_02);
    freeframe->set_offset(OFFSET_02);
    freeframe->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookMallocData = batchNativeHookData->add_events();
    nativeHookMallocData->set_tv_sec(TV_SEC_01);
    nativeHookMallocData->set_tv_nsec(TV_NSEC_01);
    nativeHookMallocData->set_allocated_alloc_event(allocEvent);
    auto nativeHookFreeData = batchNativeHookData->add_events();
    nativeHookFreeData->set_tv_sec(TV_SEC_02);
    nativeHookFreeData->set_tv_nsec(TV_NSEC_02);
    nativeHookFreeData->set_allocated_free_event(freeEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;

    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse Malloc event results
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    NativeHookCache firstExpectNativeHookCache(1, expect_ipid, expect_itid, ALLOCEVENT.c_str(), INVALID_UINT64,
                                               TIMESTAMP_01, 0, 0, MEM_ADDR_01, MEM_SIZE_01, MEM_SIZE_01, 0);
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    // Verification parse Malloc Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    auto expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_01);
    auto expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_01);
    NativeHookFrameCache firstExpectFrameCache(1, 0, CALL_STACK_IP_01, CALL_STACK_SP_01, expectSymbolData,
                                               expectFilePathData, OFFSET_01, SYMBOL_OFFSET_01);
    NativeHookFrameCache firstResultFrameCache(nativeHookFrame, 0);
    EXPECT_TRUE(firstExpectFrameCache == firstResultFrameCache);

    auto size = nativeHookFrame.Size();
    EXPECT_EQ(1, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MALLOC, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseTwoMallocAndFreeEventMatched
 * @tc.desc: Parse a BatchNativeHookData with two Malloc and two Free Event, that Malloc and Free was matched.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseTwoMallocAndFreeEventMatched, TestSize.Level1)
{
    TS_LOGI("test24-8");
    // construct  first AllocEvent
    AllocEvent* firstAllocEvent = new AllocEvent();
    firstAllocEvent->set_pid(PID);
    firstAllocEvent->set_tid(TID_01);
    firstAllocEvent->set_addr(MEM_ADDR_01);
    firstAllocEvent->set_size(MEM_SIZE_01);

    // construct  second AllocEvent
    AllocEvent* secondAllocEvent = new AllocEvent();
    secondAllocEvent->set_pid(PID);
    secondAllocEvent->set_tid(TID_02);
    secondAllocEvent->set_addr(MEM_ADDR_02);
    secondAllocEvent->set_size(MEM_SIZE_02);

    // construct first FreeEvent
    FreeEvent* firstFreeEvent = new FreeEvent();
    firstFreeEvent->set_pid(PID);
    firstFreeEvent->set_tid(TID_01);
    firstFreeEvent->set_addr(MEM_ADDR_01);

    // construct second FreeEvent
    FreeEvent* secondFreeEvent = new FreeEvent();
    secondFreeEvent->set_pid(PID);
    secondFreeEvent->set_tid(TID_02);
    secondFreeEvent->set_addr(MEM_ADDR_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto firstMallocData = batchNativeHookData->add_events();
    firstMallocData->set_tv_sec(TV_SEC_01);
    firstMallocData->set_tv_nsec(TV_NSEC_01);
    firstMallocData->set_allocated_alloc_event(firstAllocEvent);
    auto firstFreeData = batchNativeHookData->add_events();
    firstFreeData->set_tv_sec(TV_SEC_02);
    firstFreeData->set_tv_nsec(TV_NSEC_02);
    firstFreeData->set_allocated_free_event(firstFreeEvent);
    auto secondMallocData = batchNativeHookData->add_events();
    secondMallocData->set_tv_sec(TV_SEC_03);
    secondMallocData->set_tv_nsec(TV_NSEC_03);
    secondMallocData->set_allocated_alloc_event(secondAllocEvent);
    auto secondFreeData = batchNativeHookData->add_events();
    secondFreeData->set_tv_sec(TV_SEC_04);
    secondFreeData->set_tv_nsec(TV_NSEC_04);
    secondFreeData->set_allocated_free_event(secondFreeEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());

    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;

    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse first Malloc event results
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    NativeHookCache firstExpectMallocCache(INVALID_UINT32, expect_ipid, expect_itid, ALLOCEVENT.c_str(), INVALID_UINT64,
                                           TIMESTAMP_01, TIMESTAMP_02, TIMESTAMP_02 - TIMESTAMP_01, MEM_ADDR_01,
                                           MEM_SIZE_01, MEM_SIZE_01, TIMESTAMP_02 - TIMESTAMP_01);
    NativeHookCache firstResultMallocCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectMallocCache == firstResultMallocCache);

    // Verification parse first Free event results
    NativeHookCache firstExpectFreeCache(INVALID_UINT32, expect_ipid, expect_itid, FREEEVENT.c_str(), INVALID_UINT64,
                                         TIMESTAMP_02, 0, 0, MEM_ADDR_01, MEM_SIZE_01, 0, TIMESTAMP_03 - TIMESTAMP_02);
    NativeHookCache firstResultFreeCache(nativeHook, 1);
    EXPECT_TRUE(firstExpectFreeCache == firstResultFreeCache);

    expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_02);
    NativeHookCache secondExpectMallocCache(INVALID_UINT32, expect_ipid, expect_itid, ALLOCEVENT.c_str(),
                                            INVALID_UINT64, TIMESTAMP_03, TIMESTAMP_04, TIMESTAMP_04 - TIMESTAMP_03,
                                            MEM_ADDR_02, MEM_SIZE_02, MEM_SIZE_02, TIMESTAMP_04 - TIMESTAMP_03);
    NativeHookCache secondResultMallocCache(nativeHook, 2);
    EXPECT_TRUE(secondExpectMallocCache == secondResultMallocCache);

    // Verification parse first Free event results
    NativeHookCache secondExpectFreeCache(INVALID_UINT32, expect_ipid, expect_itid, FREEEVENT.c_str(), INVALID_UINT64,
                                          TIMESTAMP_04, 0, 0, MEM_ADDR_02, MEM_SIZE_02, 0, 0);
    NativeHookCache secondResultFreeCache(nativeHook, 3);
    EXPECT_TRUE(secondExpectFreeCache == secondResultFreeCache);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MALLOC, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
}

/**
 * @tc.name: ParseTwoMallocAndFreeEventPartialMatched
 * @tc.desc: Parse a BatchNativeHookData with two Malloc and two Free Event, that Malloc and Free was partial
 matched.
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseTwoMallocAndFreeEventPartialMatched, TestSize.Level1)
{
    TS_LOGI("test24-9");
    // construct  first AllocEvent
    AllocEvent* firstAllocEvent = new AllocEvent();
    firstAllocEvent->set_pid(PID);
    firstAllocEvent->set_tid(TID_01);
    firstAllocEvent->set_addr(MEM_ADDR_01);
    firstAllocEvent->set_size(MEM_SIZE_01);

    // construct  second AllocEvent
    AllocEvent* secondAllocEvent = new AllocEvent();
    secondAllocEvent->set_pid(PID);
    secondAllocEvent->set_tid(TID_02);
    secondAllocEvent->set_addr(MEM_ADDR_02);
    secondAllocEvent->set_size(MEM_SIZE_02);

    // construct first FreeEvent
    FreeEvent* firstFreeEvent = new FreeEvent();
    firstFreeEvent->set_pid(PID);
    firstFreeEvent->set_tid(TID_01);
    firstFreeEvent->set_addr(MEM_ADDR_01);

    // construct second FreeEvent
    FreeEvent* secondFreeEvent = new FreeEvent();
    secondFreeEvent->set_pid(PID);
    secondFreeEvent->set_tid(TID_02);
    secondFreeEvent->set_addr(MEM_ADDR_03);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto firstMallocData = batchNativeHookData->add_events();
    firstMallocData->set_tv_sec(TV_SEC_01);
    firstMallocData->set_tv_nsec(TV_NSEC_01);
    firstMallocData->set_allocated_alloc_event(firstAllocEvent);
    auto firstFreeData = batchNativeHookData->add_events();
    firstFreeData->set_tv_sec(TV_SEC_02);
    firstFreeData->set_tv_nsec(TV_NSEC_02);
    firstFreeData->set_allocated_free_event(firstFreeEvent);
    auto secondMallocData = batchNativeHookData->add_events();
    secondMallocData->set_tv_sec(TV_SEC_03);
    secondMallocData->set_tv_nsec(TV_NSEC_03);
    secondMallocData->set_allocated_alloc_event(secondAllocEvent);
    auto secondFreeData = batchNativeHookData->add_events();
    secondFreeData->set_tv_sec(TV_SEC_04);
    secondFreeData->set_tv_nsec(TV_NSEC_04);
    secondFreeData->set_allocated_free_event(secondFreeEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse first Malloc event results
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    NativeHookCache firstExpectMallocCache(INVALID_UINT32, expect_ipid, expect_itid, ALLOCEVENT.c_str(), INVALID_UINT64,
                                           TIMESTAMP_01, TIMESTAMP_02, TIMESTAMP_02 - TIMESTAMP_01, MEM_ADDR_01,
                                           MEM_SIZE_01, MEM_SIZE_01, TIMESTAMP_02 - TIMESTAMP_01);
    NativeHookCache firstResultMallocCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectMallocCache == firstResultMallocCache);

    // Verification parse first Free event results
    NativeHookCache firstExpectFreeCache(INVALID_UINT32, expect_ipid, expect_itid, FREEEVENT.c_str(), INVALID_UINT64,
                                         TIMESTAMP_02, 0, 0, MEM_ADDR_01, MEM_SIZE_01, 0, TIMESTAMP_03 - TIMESTAMP_02);
    NativeHookCache firstResultFreeCache(nativeHook, 1);
    EXPECT_TRUE(firstExpectFreeCache == firstResultFreeCache);

    // Verification parse second Malloc event results
    expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_02);
    NativeHookCache secondExpectMallocCache(INVALID_UINT32, expect_ipid, expect_itid, ALLOCEVENT.c_str(),
                                            INVALID_UINT64, TIMESTAMP_03, 0, 0, MEM_ADDR_02, MEM_SIZE_02, MEM_SIZE_02,
                                            0);
    NativeHookCache secondResultMallocCache(nativeHook, 2);
    EXPECT_TRUE(secondExpectMallocCache == secondResultMallocCache);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MALLOC, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithOneMmap
 * @tc.desc: Parse a BatchNativeHookData with only one MMAP
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithOneMmap, TestSize.Level1)
{
    TS_LOGI("test24-10");
    // construct MmapEvent
    MmapEvent* mmapEvent = new MmapEvent();
    mmapEvent->set_pid(PID);
    mmapEvent->set_tid(TID_01);
    mmapEvent->set_addr(MEM_ADDR_01);
    mmapEvent->set_size(MEM_SIZE_01);
    mmapEvent->set_type(MMAP_SUB_TYPE_01);
    // construct MmapEvent's Frame
    auto frame = mmapEvent->add_frame_info();
    frame->set_ip(CALL_STACK_IP_01);
    frame->set_sp(CALL_STACK_SP_01);
    frame->set_symbol_name(SYMBOL_NAME_01);
    frame->set_file_path(FILE_PATH_01);
    frame->set_offset(OFFSET_01);
    frame->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookData = batchNativeHookData->add_events();
    nativeHookData->set_tv_sec(TV_SEC_01);
    nativeHookData->set_tv_nsec(TV_NSEC_01);
    nativeHookData->set_allocated_mmap_event(mmapEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    auto mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_01);
    NativeHookCache expectNativeHookCache(1, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType, TIMESTAMP_01, 0,
                                          0, MEM_ADDR_01, MEM_SIZE_01, MEM_SIZE_01, 0);
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    NativeHookCache resultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(expectNativeHookCache == resultNativeHookCache);

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(1, size);

    // Verification parse NativeHook Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    auto expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_01);
    auto expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_01);
    NativeHookFrameCache expectFrameCache(1, 0, CALL_STACK_IP_01, CALL_STACK_SP_01, expectSymbolData,
                                          expectFilePathData, OFFSET_01, SYMBOL_OFFSET_01);
    NativeHookFrameCache resultFrameCache(nativeHookFrame, 0);
    EXPECT_TRUE(expectFrameCache == resultFrameCache);

    size = nativeHookFrame.Size();
    EXPECT_EQ(1, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithOneMunmap
 * @tc.desc: Parse a BatchNativeHookData with only one MMAP
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithOneMunmap, TestSize.Level1)
{
    TS_LOGI("test24-11");
    // construct MunmapEvent
    MunmapEvent* munmapEvent = new MunmapEvent();
    munmapEvent->set_pid(PID);
    munmapEvent->set_tid(TID_01);
    munmapEvent->set_addr(MEM_ADDR_01);
    munmapEvent->set_size(MEM_SIZE_01);
    // construct MunmapEvent's Frame
    auto frame = munmapEvent->add_frame_info();
    frame->set_ip(CALL_STACK_IP_01);
    frame->set_sp(CALL_STACK_SP_01);
    frame->set_symbol_name(SYMBOL_NAME_01);
    frame->set_file_path(FILE_PATH_01);
    frame->set_offset(OFFSET_01);
    frame->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookData = batchNativeHookData->add_events();
    nativeHookData->set_tv_sec(TV_SEC_01);
    nativeHookData->set_tv_nsec(TV_NSEC_01);
    nativeHookData->set_allocated_munmap_event(munmapEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(0, size);

    // Verification parse NativeHook Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();

    size = nativeHookFrame.Size();
    EXPECT_EQ(0, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithMultipleMmap
 * @tc.desc: Parse a BatchNativeHookData with multiple MMAP
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithMultipleMmap, TestSize.Level1)
{
    TS_LOGI("test24-12");
    // construct first MmapEvent
    MmapEvent* firstMmapEvent = new MmapEvent();
    firstMmapEvent->set_pid(PID);
    firstMmapEvent->set_tid(TID_01);
    firstMmapEvent->set_addr(MEM_ADDR_01);
    firstMmapEvent->set_size(MEM_SIZE_01);
    firstMmapEvent->set_type(MMAP_SUB_TYPE_01);
    // construct first MmapEvent's Frame
    auto firstFrame = firstMmapEvent->add_frame_info();
    firstFrame->set_ip(CALL_STACK_IP_01);
    firstFrame->set_sp(CALL_STACK_SP_01);
    firstFrame->set_symbol_name(SYMBOL_NAME_01);
    firstFrame->set_file_path(FILE_PATH_01);
    firstFrame->set_offset(OFFSET_01);
    firstFrame->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct second MmapEvent
    MmapEvent* secondMmapEvent = new MmapEvent();
    secondMmapEvent->set_pid(PID);
    secondMmapEvent->set_tid(TID_02);
    secondMmapEvent->set_addr(MEM_ADDR_02);
    secondMmapEvent->set_size(MEM_SIZE_02);
    secondMmapEvent->set_type(MMAP_SUB_TYPE_02);
    // construct second MmapEvent's Frame
    auto secondFrame = secondMmapEvent->add_frame_info();
    secondFrame->set_ip(CALL_STACK_IP_02);
    secondFrame->set_sp(CALL_STACK_SP_02);
    secondFrame->set_symbol_name(SYMBOL_NAME_02);
    secondFrame->set_file_path(FILE_PATH_02);
    secondFrame->set_offset(OFFSET_02);
    secondFrame->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto firstNativeHookData = batchNativeHookData->add_events();
    firstNativeHookData->set_tv_sec(TV_SEC_01);
    firstNativeHookData->set_tv_nsec(TV_NSEC_01);
    firstNativeHookData->set_allocated_mmap_event(firstMmapEvent);
    auto secondNativeHookData = batchNativeHookData->add_events();
    secondNativeHookData->set_tv_sec(TV_SEC_02);
    secondNativeHookData->set_tv_nsec(TV_NSEC_02);
    secondNativeHookData->set_allocated_mmap_event(secondMmapEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    auto mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_01);

    NativeHookCache firstExpectNativeHookCache(1, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                               TIMESTAMP_01, 0, 0, MEM_ADDR_01, MEM_SIZE_01, MEM_SIZE_01,
                                               TIMESTAMP_02 - TIMESTAMP_01);
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_02);
    mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_02);
    NativeHookCache secondExpectNativeHookCache(2, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                                TIMESTAMP_02, 0, 0, MEM_ADDR_02, MEM_SIZE_02, MEM_SIZE_01 + MEM_SIZE_02,
                                                0);
    NativeHookCache secondResultNativeHookCache(nativeHook, 1);
    EXPECT_TRUE(secondExpectNativeHookCache == secondResultNativeHookCache);
    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(2, size);

    // Verification parse NativeHook Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    auto expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_01);
    auto expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_01);
    NativeHookFrameCache firstExpectFrameCache(1, 0, CALL_STACK_IP_01, CALL_STACK_SP_01, expectSymbolData,
                                               expectFilePathData, OFFSET_01, SYMBOL_OFFSET_01);
    NativeHookFrameCache firstResultFrameCache(nativeHookFrame, 0);
    EXPECT_TRUE(firstExpectFrameCache == firstResultFrameCache);

    expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_02);
    expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_02);
    NativeHookFrameCache expectFrameCache(2, 0, CALL_STACK_IP_02, CALL_STACK_SP_02, expectSymbolData,
                                          expectFilePathData, OFFSET_02, SYMBOL_OFFSET_02);
    NativeHookFrameCache resultFrameCache(nativeHookFrame, 1);
    EXPECT_TRUE(expectFrameCache == resultFrameCache);

    size = nativeHookFrame.Size();
    EXPECT_EQ(2, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithMultipleMunmap
 * @tc.desc: Parse a BatchNativeHookData with multiple munmap
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithMultipleMunmap, TestSize.Level1)
{
    TS_LOGI("test24-13");
    // construct first MunmapEvent
    MunmapEvent* firstMunmapEvent = new MunmapEvent();
    firstMunmapEvent->set_pid(PID);
    firstMunmapEvent->set_tid(TID_01);
    firstMunmapEvent->set_addr(MEM_ADDR_01);
    firstMunmapEvent->set_size(MEM_SIZE_01);
    // construct MunmapEvent's Frame
    auto firstFrame = firstMunmapEvent->add_frame_info();
    firstFrame->set_ip(CALL_STACK_IP_01);
    firstFrame->set_sp(CALL_STACK_SP_01);
    firstFrame->set_symbol_name(SYMBOL_NAME_01);
    firstFrame->set_file_path(FILE_PATH_01);
    firstFrame->set_offset(OFFSET_01);
    firstFrame->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct second MunmapEvent
    MunmapEvent* secondMunmapEvent = new MunmapEvent();
    secondMunmapEvent->set_pid(PID);
    secondMunmapEvent->set_tid(TID_02);
    secondMunmapEvent->set_addr(MEM_ADDR_02);
    secondMunmapEvent->set_size(MEM_SIZE_02);
    // construct MunmapEvent's Frame
    auto secondFrame = secondMunmapEvent->add_frame_info();
    secondFrame->set_ip(CALL_STACK_IP_02);
    secondFrame->set_sp(CALL_STACK_SP_02);
    secondFrame->set_symbol_name(SYMBOL_NAME_02);
    secondFrame->set_file_path(FILE_PATH_02);
    secondFrame->set_offset(OFFSET_02);
    secondFrame->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto firstNativeHookData = batchNativeHookData->add_events();
    firstNativeHookData->set_tv_sec(TV_SEC_01);
    firstNativeHookData->set_tv_nsec(TV_NSEC_01);
    firstNativeHookData->set_allocated_munmap_event(firstMunmapEvent);
    auto secondNativeHookData = batchNativeHookData->add_events();
    secondNativeHookData->set_tv_sec(TV_SEC_02);
    secondNativeHookData->set_tv_nsec(TV_NSEC_02);
    secondNativeHookData->set_allocated_munmap_event(secondMunmapEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(0, size);

    // Verification parse NativeHook Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();

    size = nativeHookFrame.Size();
    EXPECT_EQ(0, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
}

/**
 * @tc.name: ParseOnePairsMmapAndMunmapEvent
 * @tc.desc: Parse a BatchNativeHookData with one pairs Mmap and MunmapEvent
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseOnePairsMmapAndMunmapEvent, TestSize.Level1)
{
    TS_LOGI("test24-14");
    // construct MmapEvent
    MmapEvent* mmapEvent = new MmapEvent();
    mmapEvent->set_pid(PID);
    mmapEvent->set_tid(TID_01);
    mmapEvent->set_addr(MEM_ADDR_01);
    mmapEvent->set_size(MEM_SIZE_01);
    mmapEvent->set_type(MMAP_SUB_TYPE_01);
    // construct MmapEvent's Frame
    auto firstFrame = mmapEvent->add_frame_info();
    firstFrame->set_ip(CALL_STACK_IP_01);
    firstFrame->set_sp(CALL_STACK_SP_01);
    firstFrame->set_symbol_name(SYMBOL_NAME_01);
    firstFrame->set_file_path(FILE_PATH_01);
    firstFrame->set_offset(OFFSET_01);
    firstFrame->set_symbol_offset(SYMBOL_OFFSET_01);
    auto secondFrame = mmapEvent->add_frame_info();
    secondFrame->set_ip(CALL_STACK_IP_02);
    secondFrame->set_sp(CALL_STACK_SP_02);
    secondFrame->set_symbol_name(SYMBOL_NAME_02);
    secondFrame->set_file_path(FILE_PATH_02);
    secondFrame->set_offset(OFFSET_02);
    secondFrame->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct first MunmapEvent
    MunmapEvent* munmapEvent = new MunmapEvent();
    munmapEvent->set_pid(PID);
    munmapEvent->set_tid(TID_01);
    munmapEvent->set_addr(MEM_ADDR_01);
    munmapEvent->set_size(MEM_SIZE_01);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookMmapData = batchNativeHookData->add_events();
    nativeHookMmapData->set_tv_sec(TV_SEC_01);
    nativeHookMmapData->set_tv_nsec(TV_NSEC_01);
    nativeHookMmapData->set_allocated_mmap_event(mmapEvent);
    auto nativeHookMunmapData = batchNativeHookData->add_events();
    nativeHookMunmapData->set_tv_sec(TV_SEC_02);
    nativeHookMunmapData->set_tv_nsec(TV_NSEC_02);
    nativeHookMunmapData->set_allocated_munmap_event(munmapEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    auto mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_01);
    NativeHookCache firstExpectNativeHookCache(1, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                               TIMESTAMP_01, TIMESTAMP_02, TIMESTAMP_02 - TIMESTAMP_01, MEM_ADDR_01,
                                               MEM_SIZE_01, MEM_SIZE_01, TIMESTAMP_02 - TIMESTAMP_01);
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    NativeHookCache secondExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MUNMAPEVENT.c_str(),
                                                mmapSubType, TIMESTAMP_02, 0, 0, MEM_ADDR_01, MEM_SIZE_01, 0, 0);
    NativeHookCache secondResultNativeHookCache(nativeHook, 1);
    EXPECT_TRUE(secondExpectNativeHookCache == secondResultNativeHookCache);

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(2, size);

    // Verification parse NativeHook Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    auto expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_02);
    auto expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_02);
    NativeHookFrameCache firstExpectFrameCache(1, 0, CALL_STACK_IP_02, CALL_STACK_SP_02, expectSymbolData,
                                               expectFilePathData, OFFSET_02, SYMBOL_OFFSET_02);
    NativeHookFrameCache firstResultFrameCache(nativeHookFrame, 0);
    EXPECT_TRUE(firstExpectFrameCache == firstResultFrameCache);

    expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_01);
    expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_01);
    NativeHookFrameCache secondExpectFrameCache(1, 1, CALL_STACK_IP_01, CALL_STACK_SP_01, expectSymbolData,
                                                expectFilePathData, OFFSET_01, SYMBOL_OFFSET_01);
    NativeHookFrameCache secondResultFrameCache(nativeHookFrame, 1);
    EXPECT_TRUE(secondExpectFrameCache == secondResultFrameCache);

    size = nativeHookFrame.Size();
    EXPECT_EQ(2, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseNotMatchMmapAndMunmapEvent
 * @tc.desc: Parse a BatchNativeHookData with not match Mmap and MunmapEvent
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseNotMatchMmapAndMunmapEvent, TestSize.Level1)
{
    TS_LOGI("test24-15");
    // construct MmapEvent
    MmapEvent* mmapEvent = new MmapEvent();
    mmapEvent->set_pid(PID);
    mmapEvent->set_tid(TID_01);
    mmapEvent->set_addr(MEM_ADDR_01);
    mmapEvent->set_size(MEM_SIZE_01);
    mmapEvent->set_type(MMAP_SUB_TYPE_01);
    // construct MmapEvent's Frame
    auto firstFrame = mmapEvent->add_frame_info();
    firstFrame->set_ip(CALL_STACK_IP_01);
    firstFrame->set_sp(CALL_STACK_SP_01);
    firstFrame->set_symbol_name(SYMBOL_NAME_01);
    firstFrame->set_file_path(FILE_PATH_01);
    firstFrame->set_offset(OFFSET_01);
    firstFrame->set_symbol_offset(SYMBOL_OFFSET_01);
    auto secondFrame = mmapEvent->add_frame_info();
    secondFrame->set_ip(CALL_STACK_IP_02);
    secondFrame->set_sp(CALL_STACK_SP_02);
    secondFrame->set_symbol_name(SYMBOL_NAME_02);
    secondFrame->set_file_path(FILE_PATH_02);
    secondFrame->set_offset(OFFSET_02);
    secondFrame->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct MunmapEvent
    MunmapEvent* munmapEvent = new MunmapEvent();
    munmapEvent->set_pid(PID);
    munmapEvent->set_tid(TID_01);
    munmapEvent->set_addr(MEM_ADDR_02);
    munmapEvent->set_size(MEM_SIZE_01);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookMmapData = batchNativeHookData->add_events();
    nativeHookMmapData->set_tv_sec(TV_SEC_01);
    nativeHookMmapData->set_tv_nsec(TV_NSEC_01);
    nativeHookMmapData->set_allocated_mmap_event(mmapEvent);
    auto nativeHookMunmapData = batchNativeHookData->add_events();
    nativeHookMunmapData->set_tv_sec(TV_SEC_02);
    nativeHookMunmapData->set_tv_nsec(TV_NSEC_02);
    nativeHookMunmapData->set_allocated_munmap_event(munmapEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    auto mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_01);
    NativeHookCache firstExpectNativeHookCache(1, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                               TIMESTAMP_01, 0, 0, MEM_ADDR_01, MEM_SIZE_01, MEM_SIZE_01, 0);
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(1, size);

    // Verification parse NativeHook Frame results
    const NativeHookFrame& nativeHookFrame = stream_.traceDataCache_->GetConstNativeHookFrameData();
    auto expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_02);
    auto expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_02);
    NativeHookFrameCache firstExpectFrameCache(1, 0, CALL_STACK_IP_02, CALL_STACK_SP_02, expectSymbolData,
                                               expectFilePathData, OFFSET_02, SYMBOL_OFFSET_02);
    NativeHookFrameCache firstResultFrameCache(nativeHookFrame, 0);
    EXPECT_TRUE(firstExpectFrameCache == firstResultFrameCache);

    expectSymbolData = stream_.traceDataCache_->dataDict_.GetStringIndex(SYMBOL_NAME_01);
    expectFilePathData = stream_.traceDataCache_->dataDict_.GetStringIndex(FILE_PATH_01);
    NativeHookFrameCache secondExpectFrameCache(1, 1, CALL_STACK_IP_01, CALL_STACK_SP_01, expectSymbolData,
                                                expectFilePathData, OFFSET_01, SYMBOL_OFFSET_01);
    NativeHookFrameCache secondResultFrameCache(nativeHookFrame, 1);
    EXPECT_TRUE(secondExpectFrameCache == secondResultFrameCache);

    size = nativeHookFrame.Size();
    EXPECT_EQ(2, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}

/**
 * @tc.name: ParseTwoPairsMatchedMmapAndMunmapEvent
 * @tc.desc: Parse a BatchNativeHookData with two pairs matched Mmap and MunmapEvent
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseTwoPairsMatchedMmapAndMunmapEvent, TestSize.Level1)
{
    TS_LOGI("test24-16");
    // construct first MmapEvent
    MmapEvent* firstMmapEvent = new MmapEvent();
    firstMmapEvent->set_pid(PID);
    firstMmapEvent->set_tid(TID_01);
    firstMmapEvent->set_addr(MEM_ADDR_01);
    firstMmapEvent->set_size(MEM_SIZE_01);
    firstMmapEvent->set_type(MMAP_SUB_TYPE_01);
    // construct second MmapEvent
    MmapEvent* secondMmapEvent = new MmapEvent();
    secondMmapEvent->set_pid(PID);
    secondMmapEvent->set_tid(TID_02);
    secondMmapEvent->set_addr(MEM_ADDR_02);
    secondMmapEvent->set_size(MEM_SIZE_02);
    secondMmapEvent->set_type(MMAP_SUB_TYPE_02);

    // construct first MunmapEvent
    MunmapEvent* firstMunmapEvent = new MunmapEvent();
    firstMunmapEvent->set_pid(PID);
    firstMunmapEvent->set_tid(TID_01);
    firstMunmapEvent->set_addr(MEM_ADDR_01);
    firstMunmapEvent->set_size(MEM_SIZE_01);
    // construct second MunmapEvent
    MunmapEvent* secondMunmapEvent = new MunmapEvent();
    secondMunmapEvent->set_pid(PID);
    secondMunmapEvent->set_tid(TID_02);
    secondMunmapEvent->set_addr(MEM_ADDR_02);
    secondMunmapEvent->set_size(MEM_SIZE_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto firstNativeHookMmapData = batchNativeHookData->add_events();
    firstNativeHookMmapData->set_tv_sec(TV_SEC_01);
    firstNativeHookMmapData->set_tv_nsec(TV_NSEC_01);
    firstNativeHookMmapData->set_allocated_mmap_event(firstMmapEvent);
    auto firstNativeHookMunmapData = batchNativeHookData->add_events();
    firstNativeHookMunmapData->set_tv_sec(TV_SEC_02);
    firstNativeHookMunmapData->set_tv_nsec(TV_NSEC_02);
    firstNativeHookMunmapData->set_allocated_munmap_event(firstMunmapEvent);
    auto secondNativeHookMmapData = batchNativeHookData->add_events();
    secondNativeHookMmapData->set_tv_sec(TV_SEC_03);
    secondNativeHookMmapData->set_tv_nsec(TV_NSEC_03);
    secondNativeHookMmapData->set_allocated_mmap_event(secondMmapEvent);
    auto secondNativeHookMunmapData = batchNativeHookData->add_events();
    secondNativeHookMunmapData->set_tv_sec(TV_SEC_04);
    secondNativeHookMunmapData->set_tv_nsec(TV_NSEC_04);
    secondNativeHookMunmapData->set_allocated_munmap_event(secondMunmapEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    auto mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_01);
    NativeHookCache firstExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                               TIMESTAMP_01, TIMESTAMP_02, TIMESTAMP_02 - TIMESTAMP_01, MEM_ADDR_01,
                                               MEM_SIZE_01, MEM_SIZE_01, TIMESTAMP_02 - TIMESTAMP_01);
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    NativeHookCache secondExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MUNMAPEVENT.c_str(),
                                                mmapSubType, TIMESTAMP_02, 0, 0, MEM_ADDR_01, MEM_SIZE_01, 0,
                                                TIMESTAMP_03 - TIMESTAMP_02);
    NativeHookCache secondResultNativeHookCache(nativeHook, 1);
    EXPECT_TRUE(secondExpectNativeHookCache == secondResultNativeHookCache);

    expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_02);
    mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_02);
    NativeHookCache thirdExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                               TIMESTAMP_03, TIMESTAMP_04, TIMESTAMP_04 - TIMESTAMP_03, MEM_ADDR_02,
                                               MEM_SIZE_02, MEM_SIZE_02, TIMESTAMP_04 - TIMESTAMP_03);
    NativeHookCache thirdResultNativeHookCache(nativeHook, 2);
    EXPECT_TRUE(thirdExpectNativeHookCache == thirdResultNativeHookCache);

    NativeHookCache fourthExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MUNMAPEVENT.c_str(),
                                                mmapSubType, TIMESTAMP_04, 0, 0, MEM_ADDR_02, MEM_SIZE_02, 0, 0);
    NativeHookCache fourthResultNativeHookCache(nativeHook, 3);
    EXPECT_TRUE(secondExpectNativeHookCache == secondResultNativeHookCache);

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(4, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
}

/**
 * @tc.name: ParsePartialMatchedMmapAndMunmapEvent
 * @tc.desc: Parse a BatchNativeHookData with partial matched Mmap and MunmapEvent
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParsePartialMatchedMmapAndMunmapEvent, TestSize.Level1)
{
    TS_LOGI("test24-17");
    // construct AllocEvent
    MmapEvent* firstMmapEvent = new MmapEvent();
    firstMmapEvent->set_pid(PID);
    firstMmapEvent->set_tid(TID_01);
    firstMmapEvent->set_addr(MEM_ADDR_01);
    firstMmapEvent->set_size(MEM_SIZE_01);
    firstMmapEvent->set_type(MMAP_SUB_TYPE_01);
    MmapEvent* secondMmapEvent = new MmapEvent();
    secondMmapEvent->set_pid(PID);
    secondMmapEvent->set_tid(TID_02);
    secondMmapEvent->set_addr(MEM_ADDR_02);
    secondMmapEvent->set_size(MEM_SIZE_02);
    secondMmapEvent->set_type(MMAP_SUB_TYPE_02);

    // construct first MunmapEvent
    MunmapEvent* firstMunmapEvent = new MunmapEvent();
    firstMunmapEvent->set_pid(PID);
    firstMunmapEvent->set_tid(TID_01);
    firstMunmapEvent->set_addr(MEM_ADDR_01);
    firstMunmapEvent->set_size(MEM_SIZE_01);
    MunmapEvent* secondMunmapEvent = new MunmapEvent();
    secondMunmapEvent->set_pid(PID);
    secondMunmapEvent->set_tid(TID_02);
    secondMunmapEvent->set_addr(MEM_ADDR_03);
    secondMunmapEvent->set_size(MEM_SIZE_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto firstNativeHookMmapData = batchNativeHookData->add_events();
    firstNativeHookMmapData->set_tv_sec(TV_SEC_01);
    firstNativeHookMmapData->set_tv_nsec(TV_NSEC_01);
    firstNativeHookMmapData->set_allocated_mmap_event(firstMmapEvent);
    auto firstNativeHookMunmapData = batchNativeHookData->add_events();
    firstNativeHookMunmapData->set_tv_sec(TV_SEC_02);
    firstNativeHookMunmapData->set_tv_nsec(TV_NSEC_02);
    firstNativeHookMunmapData->set_allocated_munmap_event(firstMunmapEvent);
    auto secondNativeHookMmapData = batchNativeHookData->add_events();
    secondNativeHookMmapData->set_tv_sec(TV_SEC_03);
    secondNativeHookMmapData->set_tv_nsec(TV_NSEC_03);
    secondNativeHookMmapData->set_allocated_mmap_event(secondMmapEvent);
    auto secondNativeHookMunmapData = batchNativeHookData->add_events();
    secondNativeHookMunmapData->set_tv_sec(TV_SEC_04);
    secondNativeHookMunmapData->set_tv_nsec(TV_NSEC_04);
    secondNativeHookMunmapData->set_allocated_munmap_event(secondMunmapEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    auto mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_01);
    NativeHookCache firstExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                               TIMESTAMP_01, TIMESTAMP_02, TIMESTAMP_02 - TIMESTAMP_01, MEM_ADDR_01,
                                               MEM_SIZE_01, MEM_SIZE_01, TIMESTAMP_02 - TIMESTAMP_01);
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    NativeHookCache secondExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MUNMAPEVENT.c_str(),
                                                mmapSubType, TIMESTAMP_02, 0, 0, MEM_ADDR_01, MEM_SIZE_01, 0,
                                                TIMESTAMP_03 - TIMESTAMP_02);
    NativeHookCache secondResultNativeHookCache(nativeHook, 1);
    EXPECT_TRUE(secondExpectNativeHookCache == secondResultNativeHookCache);

    expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_02);
    mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_02);
    NativeHookCache thirdExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                               TIMESTAMP_03, 0, 0, MEM_ADDR_02, MEM_SIZE_02, MEM_SIZE_02, 0);
    NativeHookCache thirdResultNativeHookCache(nativeHook, 2);
    EXPECT_TRUE(thirdExpectNativeHookCache == thirdResultNativeHookCache);

    NativeHookCache fourthExpectNativeHookCache(INVALID_UINT32, expect_ipid, expect_itid, MUNMAPEVENT.c_str(),
                                                mmapSubType, TIMESTAMP_04, 0, 0, MEM_ADDR_03, MEM_SIZE_02, MEM_SIZE_02,
                                                0);
    NativeHookCache fourthResultNativeHookCache(nativeHook, 3);
    EXPECT_TRUE(secondExpectNativeHookCache == secondResultNativeHookCache);

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(3, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(2 == eventCount);
}

/**
 * @tc.name: ParseBatchNativeHookWithAllTypesEvents
 * @tc.desc: Parse a BatchNativeHookData with one pairs Mmap and MunmapEvent and one pairs Malloc and Free
 * @tc.type: FUNC
 */
HWTEST_F(NativeHookParserTest, ParseBatchNativeHookWithAllTypesEvents, TestSize.Level1)
{
    TS_LOGI("test24-18");
    // construct MmapEvent
    MmapEvent* mmapEvent = new MmapEvent();
    mmapEvent->set_pid(PID);
    mmapEvent->set_tid(TID_01);
    mmapEvent->set_addr(MEM_ADDR_01);
    mmapEvent->set_size(MEM_SIZE_01);
    mmapEvent->set_type(MMAP_SUB_TYPE_01);
    // construct MmapEvent's Frame
    auto mmapframe = mmapEvent->add_frame_info();
    mmapframe->set_ip(CALL_STACK_IP_01);
    mmapframe->set_sp(CALL_STACK_SP_01);
    mmapframe->set_symbol_name(SYMBOL_NAME_01);
    mmapframe->set_file_path(FILE_PATH_01);
    mmapframe->set_offset(OFFSET_01);
    mmapframe->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct MunmapEvent
    MunmapEvent* munmapEvent = new MunmapEvent();
    munmapEvent->set_pid(PID);
    munmapEvent->set_tid(TID_01);
    munmapEvent->set_addr(MEM_ADDR_01);
    munmapEvent->set_size(MEM_SIZE_01);
    // construct MunmapEvent's Frame
    auto munmapframe = munmapEvent->add_frame_info();
    munmapframe->set_ip(CALL_STACK_IP_01);
    munmapframe->set_sp(CALL_STACK_SP_01);
    munmapframe->set_symbol_name(SYMBOL_NAME_01);
    munmapframe->set_file_path(FILE_PATH_01);
    munmapframe->set_offset(OFFSET_01);
    munmapframe->set_symbol_offset(SYMBOL_OFFSET_01);

    // construct AllocEvent
    AllocEvent* allocEvent = new AllocEvent();
    allocEvent->set_pid(PID);
    allocEvent->set_tid(TID_02);
    allocEvent->set_addr(MEM_ADDR_02);
    allocEvent->set_size(MEM_SIZE_02);
    // construct AllocEvent's Frame
    auto allocframe = allocEvent->add_frame_info();
    allocframe->set_ip(CALL_STACK_IP_02);
    allocframe->set_sp(CALL_STACK_SP_02);
    allocframe->set_symbol_name(SYMBOL_NAME_02);
    allocframe->set_file_path(FILE_PATH_02);
    allocframe->set_offset(OFFSET_02);
    allocframe->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct FreeEvent
    FreeEvent* freeEvent = new FreeEvent();
    freeEvent->set_pid(PID);
    freeEvent->set_tid(TID_02);
    freeEvent->set_addr(MEM_ADDR_02);
    // construct FreeEvent's Frame
    auto freeframe = freeEvent->add_frame_info();
    freeframe->set_ip(CALL_STACK_IP_02);
    freeframe->set_sp(CALL_STACK_SP_02);
    freeframe->set_symbol_name(SYMBOL_NAME_02);
    freeframe->set_file_path(FILE_PATH_02);
    freeframe->set_offset(OFFSET_02);
    freeframe->set_symbol_offset(SYMBOL_OFFSET_02);

    // construct BatchNativeHookData
    BatchNativeHookData* batchNativeHookData = new BatchNativeHookData();

    // add NativeHookData
    auto nativeHookMmapData = batchNativeHookData->add_events();
    nativeHookMmapData->set_tv_sec(TV_SEC_01);
    nativeHookMmapData->set_tv_nsec(TV_NSEC_01);
    nativeHookMmapData->set_allocated_mmap_event(mmapEvent);
    auto nativeHookMunmapData = batchNativeHookData->add_events();
    nativeHookMunmapData->set_tv_sec(TV_SEC_02);
    nativeHookMunmapData->set_tv_nsec(TV_NSEC_02);
    nativeHookMunmapData->set_allocated_munmap_event(munmapEvent);
    auto nativeHookAllocData = batchNativeHookData->add_events();
    nativeHookAllocData->set_tv_sec(TV_SEC_03);
    nativeHookAllocData->set_tv_nsec(TV_NSEC_03);
    nativeHookAllocData->set_allocated_alloc_event(allocEvent);
    auto nativeHookFreeData = batchNativeHookData->add_events();
    nativeHookFreeData->set_tv_sec(TV_SEC_04);
    nativeHookFreeData->set_tv_nsec(TV_NSEC_04);
    nativeHookFreeData->set_allocated_free_event(freeEvent);

    // start parse
    HtraceNativeHookParser htraceNativeHookParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    std::string hookStrMsg = "";
    batchNativeHookData->SerializeToString(&hookStrMsg);
    HtraceDataSegment dataSeg;
    dataSeg.seg = std::make_shared<std::string>(hookStrMsg);
    ProtoReader::BytesView hookBytesView(reinterpret_cast<const uint8_t*>(hookStrMsg.data()), hookStrMsg.size());
    dataSeg.protoData = hookBytesView;
    htraceNativeHookParser.Parse(dataSeg);
    htraceNativeHookParser.FinishParseNativeHookData();

    // Verification parse NativeHook results
    const NativeHook& nativeHook = stream_.traceDataCache_->GetConstNativeHookData();
    auto expect_ipid = stream_.streamFilters_->processFilter_->GetInternalPid(PID);
    auto expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_01);
    auto mmapSubType = stream_.traceDataCache_->dataDict_.GetStringIndex(MMAP_SUB_TYPE_01);
    NativeHookCache firstExpectNativeHookCache(1, expect_ipid, expect_itid, MMAPEVENT.c_str(), mmapSubType,
                                               TIMESTAMP_01, TIMESTAMP_02, TIMESTAMP_02 - TIMESTAMP_01, MEM_ADDR_01,
                                               MEM_SIZE_01, MEM_SIZE_01, TIMESTAMP_02 - TIMESTAMP_01);
    NativeHookCache firstResultNativeHookCache(nativeHook, 0);
    EXPECT_TRUE(firstExpectNativeHookCache == firstResultNativeHookCache);

    NativeHookCache secondExpectNativeHookCache(1, expect_ipid, expect_itid, MUNMAPEVENT.c_str(), mmapSubType,
                                                TIMESTAMP_02, 0, 0, MEM_ADDR_01, MEM_SIZE_01, 0, 0);
    NativeHookCache secondResultNativeHookCache(nativeHook, 1);
    EXPECT_TRUE(secondExpectNativeHookCache == secondResultNativeHookCache);

    expect_itid = stream_.streamFilters_->processFilter_->GetInternalTid(TID_02);
    NativeHookCache thirdExpectNativeHookCache(2, expect_ipid, expect_itid, ALLOCEVENT.c_str(), INVALID_UINT64,
                                               TIMESTAMP_03, TIMESTAMP_04, TIMESTAMP_04 - TIMESTAMP_03, MEM_ADDR_02,
                                               MEM_SIZE_02, MEM_SIZE_02, TIMESTAMP_04 - TIMESTAMP_03);
    NativeHookCache thirdResultNativeHookCache(nativeHook, 2);
    EXPECT_TRUE(thirdExpectNativeHookCache == thirdResultNativeHookCache);

    NativeHookCache fourthExpectNativeHookCache(2, expect_ipid, expect_itid, FREEEVENT.c_str(), INVALID_UINT64,
                                                TIMESTAMP_04, 0, 0, MEM_ADDR_02, MEM_SIZE_02, 0, 0);
    NativeHookCache fourthResultNativeHookCache(nativeHook, 3);
    EXPECT_TRUE(fourthExpectNativeHookCache == fourthResultNativeHookCache);

    auto size = stream_.traceDataCache_->GetConstNativeHookData().Size();
    EXPECT_EQ(4, size);

    auto eventCount =
        stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MUNMAP, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_MALLOC, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
    eventCount = stream_.traceDataCache_->GetConstStatAndInfo().GetValue(TRACE_NATIVE_HOOK_FREE, STAT_EVENT_RECEIVED);
    EXPECT_TRUE(1 == eventCount);
}
} // namespace TraceStreamer
} // namespace SysTuning