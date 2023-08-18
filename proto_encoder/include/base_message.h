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
#ifndef BASE_MESSAGE_H
#define BASE_MESSAGE_H
#include <functional>
#include <vector>

#include "securec.h"
#include "varint_encode.h"
// for struct RandomWriteCtx
#include "plugin_module_api.h"

namespace OHOS {
namespace Developtools {
namespace Profiler {
namespace ProtoEncoder {
// https://developers.google.com/protocol-buffers/docs/encoding
// ID   Name    Used For
// 0    VARINT  int32, int64, uint32, uint64, sint32, sint64, bool, enum
// 1    I64     fixed64, sfixed64, double
// 2    LEN     string, bytes, embedded messages, packed repeated fields
// 3    SGROUP  group start (deprecated)
// 4    EGROUP  group end (deprecated)
// 5    I32     fixed32, sfixed32, float
enum ProtoMessageType : uint32_t {
    VARINT = 0,
    I64 = 1,
    LEN = 2,
    I32 = 5,
};

constexpr uint8_t FIELDID_SHIFT = 3;
constexpr uint8_t SIZE_FIXED32 = 4;
constexpr uint8_t SIZE_FIXED64 = 8;
constexpr uint8_t SIZE_RESERVED_LEN = 4;

class MessagePool;

class BaseMessage {
public:
    BaseMessage()
    {
        Reset(nullptr, nullptr);
    }

    explicit BaseMessage(RandomWriteCtx* writeCtx, MessagePool* messagePool = nullptr)
    {
        Reset(writeCtx, messagePool);
    }

    ~BaseMessage()
    {
        if (isWriting_) {
            Finish();
        }
    }

    void Reset(RandomWriteCtx* writeCtx = nullptr, MessagePool* messagePool = nullptr)
    {
        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }

        writeCtx_ = writeCtx;
        subMessageStack_ = messagePool;
        backfillOffset_ = 0;
        if (writeCtx == nullptr) {
            // can not write
            isWriting_ = false;
            size_ = -1;
            return;
        }
        size_ = 0;
        isWriting_ = true;
    }

    // return current size of message has wrote, < 0 if writing failed
    inline int32_t Size()
    {
        return size_;
    }

    // finish message and return size of message, < 0 if writing failed
    inline int32_t Finish()
    {
        if (!isWriting_) {
            return size_;
        }

        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }

        isWriting_ = false;
        return size_;
    }

    // ID Name Used For
    // 5  I32  fixed32, sfixed32, float
    template<typename T>
    void AddFixed32(uint32_t fieldId, T v)
    {
        static_assert(sizeof(T) == SIZE_FIXED32, "AddFixed32: T is not 32 bits");
        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }
        if (!isWriting_) {
            return;
        }

        uint8_t* fieldMemory = nullptr;
        uint32_t fieldOffset = 0;
        // max field size = varint(fieldId + type) + Fixed32(v)
        if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + SIZE_FIXED32,
                                  &fieldMemory, &fieldOffset)) {
            Drop();
            return;
        }

        uint32_t tagSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::I32);
        // T must be little-endian
        if (memcpy_s(fieldMemory + tagSize, SIZE_FIXED32, &v, SIZE_FIXED32) != EOK) {
            Drop();
            return;
        }

        writeCtx_->seek(writeCtx_, fieldOffset + tagSize + SIZE_FIXED32);
        size_ += tagSize + SIZE_FIXED32;
    }

    // ID Name Used For
    // 1  I64  fixed64, sfixed64, double
    template<typename T>
    void AddFixed64(uint32_t fieldId, T v)
    {
        static_assert(sizeof(T) == SIZE_FIXED64, "AddFixed64: T is not 64 bits");
        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }
        if (!isWriting_) {
            return;
        }

        uint8_t* fieldMemory = nullptr;
        uint32_t fieldOffset = 0;
        // max field size = varint(fieldId + type) + Fixed64(v)
        if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + SIZE_FIXED64,
                                  &fieldMemory, &fieldOffset)) {
            Drop();
            return;
        }

        uint32_t tagSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::I64);
        // T must be little-endian
        if (memcpy_s(fieldMemory + tagSize, SIZE_FIXED64, &v, SIZE_FIXED64) != EOK) {
            Drop();
            return;
        }

        writeCtx_->seek(writeCtx_, fieldOffset + tagSize + SIZE_FIXED64);
        size_ += tagSize + SIZE_FIXED64;
    }

    // ID Name   Used For(unsigned and 'intN' varint)
    // 0  VARINT int32, int64, uint32, uint64, bool, enum
    template<typename T>
    void AddVarint(uint32_t fieldId, T v)
    {
        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }
        if (!isWriting_) {
            return;
        }

        uint8_t* fieldMemory = nullptr;
        uint32_t fieldOffset = 0;
        // max field size = varint(fieldId + type) + varint(v)
        if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + VARINT_ENCODE_MAX_SIZE,
                                  &fieldMemory, &fieldOffset)) {
            Drop();
            return;
        }

        uint32_t fieldSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::VARINT);
        fieldSize += EncodeVarint(fieldMemory + fieldSize, v);
        writeCtx_->seek(writeCtx_, fieldOffset + fieldSize);
        size_ += fieldSize;
    }

    // ID Name   Used For('sintN' varint, ZigZag encode)
    // 0  VARINT sint32, sint64
    template<typename T>
    void AddZigZagVarint(uint32_t fieldId, T v)
    {
        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }
        if (!isWriting_) {
            return;
        }

        uint8_t* fieldMemory = nullptr;
        uint32_t fieldOffset = 0;
        // max field size = varint(fieldId + type) + varint(v)
        if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + VARINT_ENCODE_MAX_SIZE,
                                  &fieldMemory, &fieldOffset)) {
            Drop();
            return;
        }

        uint32_t fieldSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::VARINT);
        fieldSize += EncodeZigZagVarint(fieldMemory + fieldSize, v);
        writeCtx_->seek(writeCtx_, fieldOffset + fieldSize);
        size_ += fieldSize;
    }

    // ID Name Used For
    // 2  LEN  bytes, string
    void AddBytes(uint32_t fieldId, const void* data, uint32_t dataSize);

    // add customize data, return RandomWriteCtx pointer, caller implement:
    // 1, write data by writeCtx->write() directly;
    // 2, constructor Message object by writeCtx, and fill data by method of Message
    RandomWriteCtx* StartAddBytes(uint32_t fieldId);
    void FinishAddBytes(int32_t size);

    // get data by getData(), implement in getData() function:
    // 1, write data by randomWriteCtx->write() directly;
    // 2, constructor Message object by randomWriteCtx, and fill data by method of Message
    using GetDataCallback = std::function<int32_t(RandomWriteCtx* randomWriteCtx)>;
    void AddBytesByCallBack(uint32_t fieldId, GetDataCallback getData);

    // ID Name Used For
    // 2  LEN  embedded messages
    template<typename T>
    T* AddSubMessage(uint32_t fieldId)
    {
        static_assert(std::is_base_of<BaseMessage, T>::value,
                      "SubMessage must be a derived class of BaseMessage");
        static_assert(sizeof(T) == sizeof(BaseMessage),
                      "Size of SubMessage class must be equal to BaseMessage");
        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }
        if (!isWriting_) {
            // return message self pointer for business,
            // business can call message->set_XXX() without checking message is nullptr
            return static_cast<T*>(this);
        }

        uint8_t* fieldMemory = nullptr;
        // varint(fieldId) + varint(len)
        if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + SIZE_RESERVED_LEN,
                                  &fieldMemory, &backfillOffset_)) {
            Drop();
            return static_cast<T*>(this);
        }

        uint32_t tagSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::LEN);
        backfillOffset_ += tagSize;
        size_ += tagSize;
        // reserve length space
        writeCtx_->seek(writeCtx_, backfillOffset_ + SIZE_RESERVED_LEN);
        if (!AllocateSubMessage()) {
            Drop();
            return static_cast<T*>(this);
        }
        return static_cast<T*>(subMessage_);
    }

    // ID Name Used For
    // 2  LEN  packed repeated fields(unsigned and 'intN' varint)
    template<typename T>
    inline void AddPackedVarint(uint32_t fieldId, const T* array, uint32_t arrayCount)
    {
        if (arrayCount == 0) {
            return;
        }
        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }
        if (!isWriting_) {
            return;
        }

        uint32_t maxLen = 0;
        uint32_t lenSize = GetPackedVarintLenSize(arrayCount, sizeof(T), maxLen);
        if (lenSize == 0) {
            Drop();
            return;
        }

        uint8_t* fieldMemory = nullptr;
        uint32_t fieldOffset = 0;
        // varint(fieldId) + lenSize + maxLen
        if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + lenSize + maxLen,
                                  &fieldMemory, &fieldOffset)) {
            Drop();
            return;
        }

        uint32_t tagSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::LEN);
        // encode array of varint first
        uint32_t dataSize = 0;
        uint8_t* pData = fieldMemory + tagSize + lenSize;
        for (uint32_t i = 0; i < arrayCount; i++) {
            dataSize += EncodeVarint(pData + dataSize, *array);
            array++;
        }
        // varint(Length)
        EncodeVarintPadding(fieldMemory + tagSize, dataSize, lenSize);
        size_ += tagSize + lenSize + dataSize;
        // seek to tail
        writeCtx_->seek(writeCtx_, fieldOffset + tagSize + lenSize + dataSize);
    }

    // ID Name Used For
    // 2  LEN  packed repeated fields(I32 and I64)
    template<typename T>
    inline void AddPackedFixed(uint32_t fieldId, const T* array, uint32_t arrayCount)
    {
        static_assert(sizeof(T) == SIZE_FIXED32 || sizeof(T) == SIZE_FIXED64,
                      "AddPackedFixed: T is not 32 or 64 bits");
        if (arrayCount == 0) {
            return;
        }
        if (subMessage_ != nullptr) {
            FinishSubMessage();
        }
        if (!isWriting_) {
            return;
        }

        uint32_t dataSize = arrayCount * sizeof(T);
        uint8_t* fieldMemory = nullptr;
        uint32_t fieldOffset = 0;
        // varint(fieldId) + varint(dataSize) + dataSize
        if (!writeCtx_->getMemory(writeCtx_, VARINT_ENCODE_MAX_SIZE + SIZE_RESERVED_LEN + dataSize,
                                  &fieldMemory, &fieldOffset)) {
            Drop();
            return;
        }

        uint32_t tagSize = EncodeVarint(fieldMemory, (fieldId << FIELDID_SHIFT) | ProtoMessageType::LEN);
        tagSize += EncodeVarint(fieldMemory + tagSize, dataSize);
        if (memcpy_s(fieldMemory + tagSize, dataSize, array, dataSize) != EOK) {
            Drop();
            return;
        }

        size_ += tagSize + dataSize;
        // seek to tail
        writeCtx_->seek(writeCtx_, fieldOffset + tagSize + dataSize);
    }

private:
    RandomWriteCtx* writeCtx_ = nullptr;
    MessagePool* subMessageStack_ = nullptr;
    BaseMessage* subMessage_ = nullptr;
    int32_t size_ = 0;
    uint32_t backfillOffset_ = 0;
    bool isWriting_ = true; // false when Finish() or Drop()

    bool AllocateSubMessage();
    void FinishSubMessage();

    inline void Drop()
    {
        isWriting_ = false;
        size_ = -1;
        return;
    }
};

constexpr uint32_t DEFAULT_SUBMESSAGE_DEPTH = 10;
// MessagePool is cache of the BaseMessage's submessage, avoid new and delete multiple times
// ONE BaseMessage corresponds to ONE MessagePool
class MessagePool {
public:
    explicit MessagePool(uint32_t depth = DEFAULT_SUBMESSAGE_DEPTH)
    {
        Reset(depth);
    }

    inline void Reset(uint32_t depth = DEFAULT_SUBMESSAGE_DEPTH)
    {
        messageCache_.resize(depth);
        cursor_ = 0;
    }

    BaseMessage* Get()
    {
        if (cursor_ < messageCache_.size()) {
            return &messageCache_[cursor_++];
        } else {
            auto& msg = messageCache_.emplace_back();
            cursor_++;
            return &msg;
        }
    }

    inline void Release()
    {
        if (cursor_ > 0) {
            cursor_--;
        }
    }
private:
    std::vector<BaseMessage> messageCache_;
    uint32_t cursor_ = 0;
};
} // namespace ProtoEncoder
} // namespace Profiler
} // namespace Developtools
} // namespace OHOS
#endif // BASE_MESSAGE_H
