/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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

#include "bpf_event_receiver.h"

#include <chrono>

using namespace std::chrono_literals;

static constexpr int DOUBLE = 2;

void BPFEventReceiver::DoWork()
{
    for (__u32 cnt = MAX_BUSY_LOOPS; cnt != 0; --cnt) {
        if (buf_->GetDataSize()) {
            break;
        }
    }
    if (buf_->GetDataSize() == 0) {
        std::unique_lock lk {mtx_};
        cond_.wait(lk, [this]()->bool {return this->stop_ || this->buf_->GetDataSize();});
        if (stop_) {
            return;
        }
    }

    while (buf_->GetDataSize() != 0) {
        __u32 tracer {0};
        if (buf_->Peek(&tracer) == 0) {
            switch (tracer) {
                case FSTRACE: {
                    ReceiveFSTraceEvent();
                    continue;
                }
                case PFTRACE: {
                    ReceivePFTraceEvent();
                    continue;
                }
                case BIOTRACE: {
                    ReceiveBIOTraceEvent();
                    continue;
                }
                case STRTRACE: {
                    ReceiveSTRTraceEvent();
                    continue;
                }
                case DLOPEN_TRACE: {
                    ReceiveDlopenTraceEvent();
                    continue;
                }
                default: {
                    DiscardEvent();
                    continue;
                }
            }
        } else {
            HHLOGE(true, "failed to peek the tracer type");
            return;
        }
    }
}

void BPFEventReceiver::ReceiveFSTraceEvent()
{
    struct fstrace_cmplt_event_t cmplt_event {};
    size_t dataSize = buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    if (dataSize != sizeof(cmplt_event)) {
        HHLOGE(true, "imcomplete fstrace event data receiveed");
        return;
    }
    WriteEventMaps(cmplt_event.tgid);
    auto file = file_.lock();
    if (file == nullptr) {
        HHLOGE(true, "failed to receive fstrace event: hiebpf data file closed");
        return;
    }
    __u32 tlvItemSize = GetFSTraceTLVItemSize(cmplt_event);
    void *dest = file->Reserve(tlvItemSize);
    if (dest == nullptr) {
        HHLOGE(true, "failed to reserve space for fstrace event tlv item");
        return;
    }
    if (EncodeFSTraceEvent(&cmplt_event, dest, tlvItemSize) != 0) {
        HHLOGE(true, "failed to encode fstrace event");
        file->Discard(dest);
        return;
    }
    file->Submit(dest);
}

void BPFEventReceiver::ReceivePFTraceEvent()
{
    struct pftrace_cmplt_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    WriteEventMaps(cmplt_event.tgid);
    auto file = file_.lock();
    if (file == nullptr) {
        HHLOGE(true, "failed to receive pftrace event: hiebpf data file closed");
        return;
    }
    __u32 tlvItemSize = GetPFTraceTLVItemSize(cmplt_event);
    void *dest = file->Reserve(tlvItemSize);
    if (dest == nullptr) {
        HHLOGE(true, "failed to reserve space for pftrace event tlv item");
        return;
    }
    if (EncodePFTraceEvent(&cmplt_event, dest, tlvItemSize) != 0) {
        HHLOGE(true, "failed to encode pftrace event");
        file->Discard(dest);
        return;
    }
    file->Submit(dest);
}

void BPFEventReceiver::ReceiveBIOTraceEvent()
{
    struct biotrace_cmplt_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    auto file = file_.lock();
    if (file == nullptr) {
        HHLOGE(true, "failed to receive biotrace event: hiebpf data file closed");
        return;
    }
    __u32 tlvItemSize = GetBIOTraceTLVItemSize(cmplt_event);
    void *dest = file->Reserve(tlvItemSize);
    if (dest == nullptr) {
        HHLOGE(true, "failed to reserve space for biotrace event tlv item");
        return;
    }
    if (EncodeBIOTraceEvent(&cmplt_event, dest, tlvItemSize) != 0) {
        HHLOGE(true, "failed to encode biotrace event");
        file->Discard(dest);
        return;
    }
    file->Submit(dest);
}

void BPFEventReceiver::ReceiveSTRTraceEvent()
{
    struct strtrace_cmplt_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    if (cmplt_event.len == 0) {
        return;
    }
    auto file = file_.lock();
    if (file == nullptr) {
        HHLOGE(true, "failed to receive strtrace event: hiebpf data file closed");
        return;
    }
    __u32 tlvItemSize = GetSTRTraceTLVItemSize(cmplt_event);
    void *dest = file->Reserve(tlvItemSize);
    if (dest == nullptr) {
        HHLOGE(true, "failed to reserve space for strtrace event tlv item");
        return;
    }
    if (EncodeSTRTraceEvent(&cmplt_event, dest, tlvItemSize) != 0) {
        HHLOGE(true, "failed to encode strtrace event");
        file->Discard(dest);
        return;
    }
    file->Submit(dest);
}

void BPFEventReceiver::ReceiveDlopenTraceEvent()
{
    struct dlopen_trace_start_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    mapsInfo_.RemovePid(cmplt_event.tgid);
}

void BPFEventReceiver::WriteEventMaps(uint32_t pid)
{
    if (mapsInfo_.IsCached(pid)) {
        return;
    }
    using OHOS::Developtools::Hiebpf::MapsInfo;
    std::vector<MapsInfo::MapsItem> mapsItems;
    mapsInfo_.GetMaps(pid, mapsItems);
    auto file = file_.lock();
    for (auto &item : mapsItems) {
        size_t size = sizeof(FixedMapTLVItem) + item.fileName_.size() + 1;
        void *dest = file->Reserve(size);
        if (dest == nullptr) {
            HHLOGE(true, "failed to reserve space for strtrace event tlv item");
            continue;
        }
        FixedMapTLVItem *mapItem = static_cast<FixedMapTLVItem *>(dest);
        mapItem->type = MAPSTRACE;
        mapItem->len = size - sizeof(uint32_t) * DOUBLE;
        mapItem->start = item.start_;
        mapItem->end = item.end_;
        mapItem->offset = item.offset_;
        mapItem->pid = item.pid_;
        mapItem->fileNameLen = item.fileName_.size() + 1;
        char* tmp = static_cast<char*>(dest);
        char* fileName = tmp + sizeof(FixedMapTLVItem);
        (void)strncpy_s(fileName, mapItem->fileNameLen, item.fileName_.c_str(), item.fileName_.size());
        file->Submit(dest);
        WriteSymbolInfo(item.fileName_);
    }
    mapsInfo_.CachePid(pid);
}

void BPFEventReceiver::WriteSymbolInfo(const std::string &fileName)
{
    if (elfSymbolInfo_.IsCached(fileName)) {
        return;
    }
    using OHOS::Developtools::Hiebpf::ElfSymbolInfo;
    ElfSymbolInfo::ElfSymbolTable symbolInfo;
    if (elfSymbolInfo_.GetSymbolTable(fileName, symbolInfo)) {
        auto file = file_.lock();
        size_t size = sizeof(FixedSymbolTLVItem) + symbolInfo.strTable_.size() + symbolInfo.symTable_.size()
            + symbolInfo.fileName_.size() + 1;
        void *dest = file->Reserve(size);
        if (dest == nullptr) {
            HHLOGE(true, "file reserve failed");
            return;
        }
        FixedSymbolTLVItem *sym = static_cast<FixedSymbolTLVItem *>(dest);
        sym->type = SYMBOLTRACE;
        sym->len = size - sizeof(uint32_t) * DOUBLE;
        sym->textVaddr = symbolInfo.textVaddr_;
        sym->textOffset = symbolInfo.textOffset_;
        sym->strTabLen = symbolInfo.strTable_.size();
        sym->symTabLen = symbolInfo.symTable_.size();
        sym->fileNameLen = symbolInfo.fileName_.size() + 1;
        sym->symEntLen = symbolInfo.symEntSize_;
        char* tmp = static_cast<char*>(dest);
        size_t pos = 0;
        pos += sizeof(FixedSymbolTLVItem);
        if (memcpy_s(tmp + pos, size - pos, symbolInfo.strTable_.data(), sym->strTabLen) != EOK) {
            HHLOGE(true, "memcpy_s failed");
            return;
        }
        pos += sym->strTabLen;
        if (memcpy_s(tmp + pos, size - pos, symbolInfo.symTable_.data(), sym->symTabLen) != EOK) {
            HHLOGE(true, "memcpy_s failed");
            return;
        }
        pos += sym->symTabLen;
        if (memcpy_s(tmp + pos, size - pos, symbolInfo.fileName_.c_str(), symbolInfo.fileName_.size()) != EOK) {
            HHLOGE(true, "memcpy_s failed");
            return;
        }
        file->Submit(dest);
    }
    elfSymbolInfo_.CacheFileName(fileName);
}

void BPFEventReceiver::ReverseStr(char* left, char* right)
{
    while (left < right) {
        char tmp = 0;
        tmp = *left;
        *left = *right;
        *right = tmp;
        left++;
        right--;
    }
}

void BPFEventReceiver::DiscardEvent()
{
    struct strtrace_cmplt_event_t cmplt_event {};
    buf_->Get((char*)&cmplt_event, sizeof(cmplt_event));
    HHLOGE(true, "unkown tracer type = %u, event data will be discarded", cmplt_event.tracer);
}

int BPFEventReceiver::EncodeFSTraceEvent(
    const struct fstrace_cmplt_event_t *cmplt_event,
    void* tlvItem,
    const size_t itemLen)
{
    struct FixedFSTraceTLVItem *item = static_cast<struct FixedFSTraceTLVItem *>(tlvItem);
    item->tracer_ = FSTRACE;
    item->itemLen_ = itemLen - sizeof(uint32_t) * DOUBLE;
    item->pid_ = cmplt_event->tgid;
    item->tid_ = cmplt_event->pid;
    CHECK_TRUE(strncpy_s(item->tracerName_, MAX_TRACER_NAME_LEN,
                         gTracerTable[FSTRACE].c_str(), gTracerTable[FSTRACE].size()) == EOK,
               -1, "failed to copy fstrace tracer name");
    item->stime_ = cmplt_event->start_event.stime;
    item->ctime_ = cmplt_event->ctime;
    item->retval_ = cmplt_event->retval;
    item->nips_ = static_cast<uint16_t>(cmplt_event->nips);
    item->type_ = static_cast<uint16_t>(cmplt_event->start_event.type);
    CHECK_TRUE(strncpy_s(item->typeName_, MAX_TYPE_NAME_LEN,
                         gFSTraceTypeTable[item->type_].c_str(),
                         gFSTraceTypeTable[item->type_].size()) == EOK,
               -1, "failed to copy fstrace type name");
    CHECK_TRUE(ConvertFSTraceArgsToArray(item->args_, &cmplt_event->start_event) == 0, -1,
               "failed to convert fstrace event args");
    (void)memcpy_s(item->comm_, MAX_COMM_LEN, cmplt_event->comm, MAX_COMM_LEN);
    if (item->nips_ != 0) {
        int ret = memcpy_s(reinterpret_cast<__u64*>((reinterpret_cast<char *>(item) +
                           sizeof(struct FixedFSTraceTLVItem))), item->nips_ * sizeof(__u64),
                           cmplt_event->ips, item->nips_ * sizeof(__u64));
        if (ret != EOK) {
            return -1;
        }
    }
    return 0;
}

int BPFEventReceiver::EncodePFTraceEvent(
    const struct pftrace_cmplt_event_t *cmplt_event,
    void* tlvItem,
    const size_t itemLen)
{
    struct FixedPFTraceTLVItem *item = static_cast<struct FixedPFTraceTLVItem *>(tlvItem);
    item->tracer_ = PFTRACE;
    item->itemLen_ = itemLen - sizeof(uint32_t) * DOUBLE;
    item->pid_ = cmplt_event->tgid;
    item->tid_ = cmplt_event->pid;
    CHECK_TRUE(strncpy_s(item->tracerName_, MAX_TRACER_NAME_LEN,
                         gTracerTable[PFTRACE].c_str(), gTracerTable[PFTRACE].size()) == EOK,
               -1, "failed to copy pftrace tracer name");
    item->stime_ = cmplt_event->start_event.stime;
    item->ctime_ = cmplt_event->ctime;
    item->addr_ = cmplt_event->start_event.addr;
    item->size_ = cmplt_event->size;
    item->nips_ = static_cast<uint16_t>(cmplt_event->nips);
    item->type_ = static_cast<uint16_t>(cmplt_event->start_event.type);
    CHECK_TRUE(strncpy_s(item->typeName_,
                         MAX_TYPE_NAME_LEN,
                         gPFTraceTypeTable[item->type_].c_str(),
                         gPFTraceTypeTable[item->type_].size()) == EOK,
               -1, "failed to copy pftrace type name");
    (void)memcpy_s(item->comm_, MAX_COMM_LEN, cmplt_event->comm, MAX_COMM_LEN);
    if (item->nips_ != 0) {
        int ret = memcpy_s(reinterpret_cast<__u64*>((reinterpret_cast<char *>(item) +
                           sizeof(struct FixedPFTraceTLVItem))), item->nips_ * sizeof(__u64),
                           cmplt_event->ips, item->nips_ * sizeof(__u64));
        if (ret != EOK) {
            return -1;
        }
    }
    return 0;
}

int BPFEventReceiver::EncodeBIOTraceEvent(
    const struct biotrace_cmplt_event_t *cmplt_event,
    void* tlvItem,
    const size_t itemLen)
{
    struct FixedBIOTraceTLVItem *item = static_cast<struct FixedBIOTraceTLVItem *>(tlvItem);
    item->tracer_ = BIOTRACE;
    item->itemLen_ = itemLen - sizeof(uint32_t) * DOUBLE;
    item->pid_ = cmplt_event->start_event.tgid;
    item->tid_ = cmplt_event->start_event.pid;
    (void)memcpy_s(item->comm_, MAX_COMM_LEN, cmplt_event->start_event.comm, MAX_COMM_LEN);
    item->stime_ = cmplt_event->start_event.stime;
    item->ctime_ = cmplt_event->ctime;
    item->prio_ = cmplt_event->prio;
    item->size_ = cmplt_event->start_event.size;
    item->blkcnt_ = cmplt_event->blkcnt;
    item->nips_ = cmplt_event->nips;
    item->type_ = cmplt_event->start_event.type;
    CHECK_TRUE(strncpy_s(item->typeName_,
                         MAX_TYPE_NAME_LEN,
                         gBIOTraceTypeTable[item->type_].c_str(),
                         gBIOTraceTypeTable[item->type_].size()) == EOK,
               -1, "failed to copy BIOstrace type name");
    if (item->nips_ != 0) {
        int ret = memcpy_s(reinterpret_cast<__u64*>((reinterpret_cast<char *>(item) +
                           sizeof(struct FixedBIOTraceTLVItem))), item->nips_ * sizeof(__u64),
                           cmplt_event->ips, item->nips_ * sizeof(__u64));
        if (ret != EOK) {
            return -1;
        }
    }
    return 0;
}

int BPFEventReceiver::EncodeSTRTraceEvent(
    const struct strtrace_cmplt_event_t *cmplt_event,
    void* tlvItem,
    const size_t itemLen)
{
    struct FixedSTRTraceTLVItem *item = static_cast<struct FixedSTRTraceTLVItem *>(tlvItem);
    item->tracer_ = STRTRACE;
    item->itemLen_ = itemLen - sizeof(uint32_t) * DOUBLE;
    item->pid_ = cmplt_event->tgid;
    item->tid_ = cmplt_event->pid;
    item->stime_ = cmplt_event->start_event.stime;
    item->srcTracer_ = cmplt_event->start_event.stracer;
    item->srcType_ = cmplt_event->start_event.type;
    item->strLen_ = cmplt_event->len;
    if (item->strLen_ and item->strLen_ <= MAX_FILENAME_LEN) {
        char *filename = static_cast<char*>(tlvItem);
        filename += sizeof(struct FixedSTRTraceTLVItem);
        CHECK_TRUE(strncpy_s(filename, MAX_FILENAME_LEN, cmplt_event->filename, item->strLen_) == EOK, -1,
                   "failed to copy cmplt_event file name");
        if (item->srcTracer_ == BIOTRACE || (item->srcTracer_ == FSTRACE && item->srcType_ == SYS_CLOSE)) {
            ReverseStr(filename, filename + item->strLen_ - 2); // 2: last 2 chars
            char* start = filename;
            while (*start != '\0') {
                char* end = start;
                while (*end != '/' && *end != '\0') {
                    end++;
                }
                ReverseStr(start, end - 1);
                if (*end != '\0') {
                    start = end + 1;
                } else {
                    start = end;
                }
            }
        }
    }
    return 0;
}
