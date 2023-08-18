/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#include "base_message.h"

#include "securec.h"
#include "varint_encode.h"

namespace OHOS {
namespace Developtools {
namespace Profiler {
namespace ProtoEncoder {
bool BaseMessage::AllocateSubMessage()
{
    if (subMessageStack_ == nullptr) {
        subMessage_ = new (std::nothrow) BaseMessage(writeCtx_);
    } else {
        subMessage_ = subMessageStack_->Get();
    }

    if (subMessage_ == nullptr) {
        return false;
    }
    subMessage_->Reset(writeCtx_, subMessageStack_);
    return true;
}

void BaseMessage::FinishSubMessage()
{
    int32_t subSize = subMessage_->Finish();
    if (subMessageStack_ == nullptr) {
        delete subMessage_;
    } else {
        subMessageStack_->Release();
    }
    subMessage_ = nullptr;
    if (subSize < 0) {
        Drop();
        return;
    }

    uint8_t* fieldMemory = nullptr;
    // backfill length
    writeCtx_->seek(writeCtx_, backfillOffset_);
    if (!writeCtx_->getMemory(writeCtx_, SIZE_RESERVED_LEN,
                              &fieldMemory, &backfillOffset_)) {
        Drop();
        return;
    }

    if (subSize == 0) {
        // reduce the size
        *fieldMemory = 0;
        writeCtx_->seek(writeCtx_, backfillOffset_ + 1);
        size_++;
        return;
    }

    // varint(Length)
    EncodeVarintPadding(fieldMemory, subSize, SIZE_RESERVED_LEN);
    size_ += SIZE_RESERVED_LEN + subSize;
    // seek to tail
    writeCtx_->seek(writeCtx_, backfillOffset_ + SIZE_RESERVED_LEN + subSize);
}

void BaseMessage::AddBytes(uint32_t fieldId, const void* data, uint32_t dataSize)
{
    if (subMessage_ != nullptr) {
        FinishSubMessage();
    }
    if (!isWriting_) {
        return;
    }

    uint8_t* fieldMemory = nullptr;
    uint32_t fieldOffset = 0;
    // max field size = varint(fieldId + type) + varint(dataSize) + dataSize
    if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + SIZE_RESERVED_LEN + dataSize,
                              &fieldMemory, &fieldOffset)) {
        Drop();
        return;
    }

    uint32_t fieldSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::LEN);
    fieldSize += EncodeVarint(fieldMemory + fieldSize, dataSize);
    if (dataSize != 0) {
        if (memcpy_s(fieldMemory + fieldSize, dataSize, data, dataSize) != EOK) {
            Drop();
            return;
        }
    }
    fieldSize += dataSize;
    size_ += static_cast<int32_t>(fieldSize);
    // seek to tail
    writeCtx_->seek(writeCtx_, fieldOffset + fieldSize);
}

RandomWriteCtx* BaseMessage::StartAddBytes(uint32_t fieldId)
{
    if (subMessage_ != nullptr) {
        FinishSubMessage();
    }
    if (!isWriting_) {
        // finished or dropped
        return nullptr;
    }

    uint8_t* fieldMemory = nullptr;
    // max field size = varint(fieldId + type) + varint(len)
    if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + SIZE_RESERVED_LEN,
                              &fieldMemory, &backfillOffset_)) {
        Drop();
        return nullptr;
    }

    uint32_t tagSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::LEN);
    backfillOffset_ += tagSize;
    size_ += static_cast<int32_t>(tagSize);
    // reserve length space
    writeCtx_->seek(writeCtx_, backfillOffset_ + SIZE_RESERVED_LEN);
    return writeCtx_;
}

void BaseMessage::FinishAddBytes(int32_t size)
{
    if (size < 0) {
        Drop();
        return;
    }

    uint8_t* fieldMemory = nullptr;
    // backfill length
    writeCtx_->seek(writeCtx_, backfillOffset_);
    if (!writeCtx_->getMemory(writeCtx_, SIZE_RESERVED_LEN, &fieldMemory, &backfillOffset_)) {
        Drop();
        return;
    }

    if (size == 0) {
        // reduce the size
        *fieldMemory = 0;
        writeCtx_->seek(writeCtx_, backfillOffset_ + 1);
        size_++;
        return;
    }

    // varint(Length)
    EncodeVarintPadding(fieldMemory, size, SIZE_RESERVED_LEN);
    size_ += SIZE_RESERVED_LEN + size;
    // seek to tail
    writeCtx_->seek(writeCtx_, backfillOffset_ + SIZE_RESERVED_LEN + size);
}

void BaseMessage::AddBytesByCallBack(uint32_t fieldId, GetDataCallback getData)
{
    if (!getData) {
        return;
    }

    if (subMessage_ != nullptr) {
        FinishSubMessage();
    }
    if (!isWriting_) {
        return;
    }

    uint8_t* fieldMemory = nullptr;
    uint32_t fieldOffset = 0;
    // max field size = varint(fieldId + type) + varint(len)
    if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + SIZE_RESERVED_LEN,
                              &fieldMemory, &fieldOffset)) {
        Drop();
        return;
    }

    uint32_t tagSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::LEN);
    fieldOffset += tagSize;
    size_ += static_cast<int32_t>(tagSize);
    // reserve length space
    writeCtx_->seek(writeCtx_, fieldOffset + SIZE_RESERVED_LEN);

    int32_t dataSize = getData(writeCtx_);
    if (dataSize < 0) {
        Drop();
        return;
    }

    // backfill length
    writeCtx_->seek(writeCtx_, fieldOffset);
    if (!writeCtx_->getMemory(writeCtx_, SIZE_RESERVED_LEN, &fieldMemory, &fieldOffset)) {
        Drop();
        return;
    }

    if (dataSize == 0) {
        // reduce the size
        *fieldMemory = 0;
        writeCtx_->seek(writeCtx_, fieldOffset + 1);
        size_++;
        return;
    }

    EncodeVarintPadding(fieldMemory, dataSize, SIZE_RESERVED_LEN);
    size_ += SIZE_RESERVED_LEN + dataSize;
    // seek to tail
    writeCtx_->seek(writeCtx_, fieldOffset + SIZE_RESERVED_LEN + dataSize);
}
} // namespace ProtoEncoder
} // namespace Profiler
} // namespace Developtools
} // namespace OHOS
