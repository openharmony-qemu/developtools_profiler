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

#ifndef BINDER_FILTER_H
#define BINDER_FILTER_H

#include <unordered_set>
#include "args_set.h"
#include "filter_base.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"
namespace SysTuning {
namespace TraceStreamer {
class BinderFilter : private FilterBase {
public:
    BinderFilter(TraceDataCache*, const TraceStreamerFilters*);
    BinderFilter(const BinderFilter&) = delete;
    BinderFilter& operator=(const BinderFilter&) = delete;
    ~BinderFilter() override;

public:
    void SendTraction(int64_t ts,
                      uint32_t tid,
                      uint64_t transactionId,
                      int32_t destNode,
                      int32_t destTgid,
                      int32_t destTid,
                      bool isReply,
                      int32_t flags,
                      int32_t code);
    void ReceiveTraction(int64_t ts, uint32_t pid, uint64_t transactionId);
    void TransactionAllocBuf(int64_t ts, uint32_t pid, uint64_t dataSize, uint64_t offsetsSize);
    void TractionLock(int64_t ts, uint32_t pid, const std::string& tag);
    void TractionLocked(int64_t ts, uint32_t pid, const std::string& tag);
    void TractionUnlock(int64_t ts, uint32_t pid, const std::string& tag);
    bool IsAsync(int32_t flags) const;
    void Clear();

private:
    std::string GetBinderFlagsDesc(uint32_t flag);
    static bool IsValidUint32(uint32_t value)
    {
        return (value != INVALID_UINT32);
    }
    uint32_t noReturnMsgFlag_ = 0x01;
    uint32_t rootObjectMsgFlag_ = 0x04;
    uint32_t statusCodeMsgFlag_ = 0x08;
    uint32_t acceptFdsMsgFlag_ = 0x10;
    uint32_t noFlagsMsgFlag_ = 0;
    const DataIndex binderCatalogId_ = traceDataCache_->GetDataIndex("binder");
    const DataIndex replyId_ = traceDataCache_->GetDataIndex("binder reply");
    const DataIndex isReplayId_ = traceDataCache_->GetDataIndex("reply transaction?");
    const DataIndex flagsId_ = traceDataCache_->GetDataIndex("flags");
    const DataIndex transSliceId_ = traceDataCache_->GetDataIndex("binder transaction");
    const DataIndex transId_ = traceDataCache_->GetDataIndex("transaction id");
    const DataIndex asyncRcvId_ = traceDataCache_->GetDataIndex("binder async rcv");
    const DataIndex codeId_ = traceDataCache_->GetDataIndex("code");
    const DataIndex callingTid_ = traceDataCache_->GetDataIndex("calling tid");
    const DataIndex destNodeId_ = traceDataCache_->GetDataIndex("destination node");
    const DataIndex destThreadId_ = traceDataCache_->GetDataIndex("destination thread");
    const DataIndex destThreadNameId_ = traceDataCache_->GetDataIndex("destination name");
    const DataIndex destSliceId_ = traceDataCache_->GetDataIndex("destination slice id");
    const DataIndex destProcessId_ = traceDataCache_->GetDataIndex("destination process");
    const DataIndex transAsyncId_ = traceDataCache_->GetDataIndex("binder transaction async");
    const DataIndex lockTryId_ = traceDataCache_->GetDataIndex("binder lock waiting");
    const DataIndex lockHoldId_ = traceDataCache_->GetDataIndex("binder lock held");
    const DataIndex dataSizeId_ = traceDataCache_->GetDataIndex("data size");
    const DataIndex dataOffsetSizeId_ = traceDataCache_->GetDataIndex("offsets size");
    const DataIndex nullStringId_ = traceDataCache_->GetDataIndex("null");
    std::unordered_map<uint64_t, int64_t> lastEventTs_ = {};
    std::unordered_set<uint64_t> transReplyWaitingReply_ = {};
    std::unordered_map<uint64_t, FilterId> transNeedReply_ = {};
    std::unordered_map<uint64_t, FilterId> transReplyFilter_ = {};
    std::unordered_map<uint64_t, ArgsSet> asyncBinderEvents_ = {};
    std::unordered_map<int32_t, std::string> binderFlagDescs_ = {};
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // BINDER_FILTER_H
