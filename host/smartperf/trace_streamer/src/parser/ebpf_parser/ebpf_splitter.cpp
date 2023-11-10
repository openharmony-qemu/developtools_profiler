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

#include "ebpf_splitter.h"
#include <cinttypes>

namespace SysTuning {
namespace TraceStreamer {
// using namespace SysTuning::base;
using namespace SysTuning::EbpfStdtype;
void EbpfSplitter::SetEbpfDataOffset(uint64_t offset)
{
    // 记录ebpf的1024头相对于文件的位置。
    offsetOfEbpfDataInFile_ = offset;
}

void EbpfSplitter::SetSpliteTimeRange(uint64_t splitFileMinTs, uint64_t splitFileMaxTs)
{
    splitFileMinTs_ = splitFileMinTs;
    splitFileMaxTs_ = splitFileMaxTs;
    TS_LOGE("splitFileMinTs_ = %llu, splitFileMaxTs_ = %llu", splitFileMinTs_, splitFileMaxTs_);
}
bool EbpfSplitter::SplitEbpfHeader()
{
    splitEbpfHeader_ = std::make_unique<EbpfDataHeader>();
    std::copy_n(ebpfBuffer_.begin(), EbpfDataHeader::EBPF_DATA_HEADER_SIZE,
                reinterpret_cast<char*>(splitEbpfHeader_.get()));
    if (splitEbpfHeader_->header.magic != EbpfDataHeader::HEADER_MAGIC) {
        TS_LOGE("Get EBPF file header failed! magic = %" PRIx64 "", splitEbpfHeader_->header.magic);
        return false;
    }
    if (splitEbpfHeader_->header.headSize != EbpfDataHeader::EBPF_DATA_HEADER_SIZE) {
        TS_LOGE("Get ebpf file header failed! headSize = %u", splitEbpfHeader_->header.headSize);
        return false;
    }
    TS_LOGI("EBPF data header : magic = %" PRIu64 ", headSize = %u, clock = %u, cmdline = %s",
            splitEbpfHeader_->header.magic, splitEbpfHeader_->header.headSize, splitEbpfHeader_->header.clock,
            splitEbpfHeader_->cmdline);
    splittedLen_ += EbpfDataHeader::EBPF_DATA_HEADER_SIZE;
    TS_LOGE("splittedLen_ = %llu", splittedLen_);
    ebpfBuffer_.erase(ebpfBuffer_.begin(), ebpfBuffer_.begin() + EbpfDataHeader::EBPF_DATA_HEADER_SIZE);
    return true;
}

bool EbpfSplitter::AddAndSplitEbpfData(const std::deque<uint8_t>& dequeBuffer)
{
    ebpfBuffer_.insert(ebpfBuffer_.end(), dequeBuffer.begin(), dequeBuffer.end());
    if (!splitEbpfHeader_) {
        HtraceSplitResult ebpfHtraceHead = {.type = (int32_t)SplitDataDataType::SPLIT_FILE_DATA,
                                            .buffer = {.address = reinterpret_cast<uint8_t*>(&profilerHeader_),
                                                       .size = sizeof(ProfilerTraceFileHeader)}};
        ebpfSplitResult_.emplace_back(ebpfHtraceHead);
        if (ebpfBuffer_.size() >= EbpfDataHeader::EBPF_DATA_HEADER_SIZE) {
            auto ret = SplitEbpfHeader();
            HtraceSplitResult ebpfHead = {.type = (int32_t)SplitDataDataType::SPLIT_FILE_DATA,
                                          .buffer = {.address = reinterpret_cast<uint8_t*>(splitEbpfHeader_.get()),
                                                     .size = EbpfDataHeader::EBPF_DATA_HEADER_SIZE}};
            ebpfSplitResult_.emplace_back(ebpfHead);
            TS_ASSERT(!ret);
        } else {
            return false;
        }
    }

    SplitEbpfBodyData();
    if (profilerHeader_.data.length - sizeof(ProfilerTraceFileHeader) - splittedLen_ < EBPF_TITLE_SIZE) {
        profilerHeader_.data.length = usefulDataLen_ + sizeof(ProfilerTraceFileHeader) + sizeof(EbpfDataHeader);
    }
    return false;
}
void EbpfSplitter::SplitEbpfBodyData()
{
    while (profilerHeader_.data.length - sizeof(ProfilerTraceFileHeader) - splittedLen_ > EBPF_TITLE_SIZE &&
           ebpfBuffer_.size() > EBPF_TITLE_SIZE) {
        EbpfTypeAndLength dataTitle;
        std::copy_n(ebpfBuffer_.begin(), EBPF_TITLE_SIZE, reinterpret_cast<char*>(&dataTitle));
        if (dataTitle.length + EBPF_TITLE_SIZE > ebpfBuffer_.size()) {
            return;
        }
        auto segLen = EBPF_TITLE_SIZE + dataTitle.length;
        switch (dataTitle.type) {
            case ITEM_EVENT_MAPS:
            case ITEM_SYMBOL_INFO:
            case ITEM_EVENT_STR:
            case ITEM_EVENT_KENEL_SYMBOL_INFO: {
                HtraceSplitResult publicDataOffset = {
                    .type = (int32_t)SplitDataDataType::SPLIT_FILE_JSON,
                    .json = {.offset = offsetOfEbpfDataInFile_ + splittedLen_, .size = segLen}};
                ebpfSplitResult_.emplace_back(publicDataOffset);
                usefulDataLen_ += segLen;
                break;
            }
            case ITEM_EVENT_FS: {
                FsFixedHeader fsFixedHeader;
                std::copy_n(ebpfBuffer_.begin() + EBPF_TITLE_SIZE, sizeof(FsFixedHeader),
                            reinterpret_cast<char*>(&fsFixedHeader));
                if (fsFixedHeader.endTime <= splitFileMaxTs_ && fsFixedHeader.startTime >= splitFileMinTs_) {
                    HtraceSplitResult fsDataOffset = {
                        .type = (int32_t)SplitDataDataType::SPLIT_FILE_JSON,
                        .json = {.offset = offsetOfEbpfDataInFile_ + splittedLen_, .size = segLen}};
                    ebpfSplitResult_.emplace_back(fsDataOffset);
                    usefulDataLen_ += segLen;
                }
                break;
            }
            case ITEM_EVENT_VM: {
                PagedMemoryFixedHeader pagedMemoryFixedHeader;
                std::copy_n(ebpfBuffer_.begin() + EBPF_TITLE_SIZE, sizeof(pagedMemoryFixedHeader),
                            reinterpret_cast<char*>(&pagedMemoryFixedHeader));
                if (pagedMemoryFixedHeader.endTime <= splitFileMaxTs_ &&
                    pagedMemoryFixedHeader.startTime >= splitFileMinTs_) {
                    HtraceSplitResult pagedMemoryOffset = {
                        .type = (int32_t)SplitDataDataType::SPLIT_FILE_JSON,
                        .json = {.offset = offsetOfEbpfDataInFile_ + splittedLen_, .size = segLen}};
                    ebpfSplitResult_.emplace_back(pagedMemoryOffset);
                    usefulDataLen_ += segLen;
                }
                break;
            }
            case ITEM_EVENT_BIO: {
                BIOFixedHeader bioFixedHeader;
                std::copy_n(ebpfBuffer_.begin() + EBPF_TITLE_SIZE, sizeof(bioFixedHeader),
                            reinterpret_cast<char*>(&bioFixedHeader));
                if (bioFixedHeader.endTime <= splitFileMaxTs_ && bioFixedHeader.startTime >= splitFileMinTs_) {
                    HtraceSplitResult bioDataOffset = {.type = (int32_t)SplitDataDataType::SPLIT_FILE_JSON,
                                                       .json = {.offset = offsetOfEbpfDataInFile_ + splittedLen_,
                                                                .size = dataTitle.length + EBPF_TITLE_SIZE}};
                    ebpfSplitResult_.emplace_back(bioDataOffset);
                    usefulDataLen_ += segLen;
                }
                break;
            }
            default:
                TS_LOGI("Do not support EBPF type: %d, length: %d", dataTitle.type, dataTitle.length);
        }
        ebpfBuffer_.erase(ebpfBuffer_.begin(), ebpfBuffer_.begin() + segLen);
        splittedLen_ += segLen;
    }
}
} // namespace TraceStreamer
} // namespace SysTuning
