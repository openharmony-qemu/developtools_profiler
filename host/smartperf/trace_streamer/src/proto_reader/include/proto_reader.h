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

#ifndef TS_PROTO_READER_H_
#define TS_PROTO_READER_H_

#include <functional>
#include <limits>
#include <memory>
#include <stdint.h>
#include <string>

#include "data_area.h"
#include "log.h"
#include "proto_reader_help.h"

namespace SysTuning {
namespace ProtoReader {
constexpr uint8_t DATA_AREA_TYPE_BITS = 3;
constexpr uint8_t DATA_AREA_TYPE_VALUE = 7;
constexpr uint32_t INVALID_DATA_AREA_ID = 0;
constexpr uint8_t BYTE_HIGHEST_BIT_MARK = 0x80;
const std::map<ProtoWireType, uint32_t> fixedTypeToSizeMap_ = {{ProtoWireType::kFixed32, sizeof(uint32_t)},
                                                               {ProtoWireType::kFixed64, sizeof(uint64_t)}};
template <typename T>
class RepeatedDataAreaIterator {
public:
    RepeatedDataAreaIterator() = default;
    RepeatedDataAreaIterator(uint32_t dataAreaId,
                             const DataArea* currentAddr,
                             const DataArea* endAddr,
                             const DataArea* lastAddr)
        : dataAreaId_(dataAreaId), currentAddr_(currentAddr), endAddr_(endAddr), lastAddr_(lastAddr)
    {
        FindNextMatchingDataAreaId();
    }

    explicit operator bool() const
    {
        return currentAddr_ != endAddr_;
    }
    const DataArea& GetDataArea() const
    {
        return *currentAddr_;
    }

    T operator*() const
    {
        T value{};
        currentAddr_->GetValue(&value);
        return value;
    }
    const DataArea* operator->() const
    {
        return currentAddr_;
    }

    RepeatedDataAreaIterator& operator++()
    {
        if (currentAddr_ != lastAddr_) {
            currentAddr_++;
            FindNextMatchingDataAreaId();
            return *this;
        }
        currentAddr_ = endAddr_;
        return *this;
    }

    RepeatedDataAreaIterator operator++(int32_t)
    {
        RepeatedDataAreaIterator itor(*this);
        ++(*this);
        return itor;
    }

private:
    void FindNextMatchingDataAreaId()
    {
        while (currentAddr_ != endAddr_) {
            if (currentAddr_->DataAreaId() == dataAreaId_) {
                return;
            }
            ++currentAddr_;
        }
        currentAddr_ = lastAddr_->DataAreaValid() ? lastAddr_ : endAddr_;
    }

    uint32_t dataAreaId_ = INVALID_DATA_AREA_ID;
    const DataArea* currentAddr_ = nullptr;
    const DataArea* endAddr_ = nullptr;
    const DataArea* lastAddr_ = nullptr;
};

template <ProtoWireType wireType, typename cppType>
class PackedRepeatedDataAreaIterator {
public:
    PackedRepeatedDataAreaIterator(const uint8_t* startAddr, size_t length, bool* parseStatus)
        : endAddr_(startAddr ? startAddr + length : nullptr), currentReadAddr_(startAddr), parseStatus_(parseStatus)
    {
        static_assert(wireType != ProtoWireType::kLengthDelimited, "invalid type");

        if (length == 0) {
            currentValueValid_ = false;
            return;
        }
        auto itor = fixedTypeToSizeMap_.find(wireType);
        if (itor != fixedTypeToSizeMap_.end() && (length % itor->second)) {
            *parseStatus_ = false;
            currentValueValid_ = false;
            return;
        }
        ++(*this);
    }

    const cppType operator*() const
    {
        return currentValue_;
    }
    explicit operator bool() const
    {
        return currentValueValid_;
    }

    PackedRepeatedDataAreaIterator& operator++()
    {
        if (!currentValueValid_) {
            return *this;
        }

        if (currentReadAddr_ == endAddr_) {
            currentValueValid_ = false;
            return *this;
        }

        if (wireType == ProtoWireType::kVarInt) {
            uint64_t newValue = 0;
            const uint8_t* nextPos = VarIntDecode(currentReadAddr_, endAddr_, &newValue);

            if (nextPos != currentReadAddr_) {
                currentReadAddr_ = nextPos;
                currentValue_ = static_cast<cppType>(newValue);
            } else {
                *parseStatus_ = true;
                currentValueValid_ = false;
            }
        } else { // kFixed32 or kFixed64
            auto step = fixedTypeToSizeMap_.at(wireType);
            memcpy_s(&currentValue_, sizeof(cppType), currentReadAddr_, sizeof(cppType));
            currentReadAddr_ += step;
        }

        return *this;
    }

    PackedRepeatedDataAreaIterator operator++(int32_t)
    {
        PackedRepeatedDataAreaIterator itor(*this);
        ++(*this);
        return itor;
    }

private:
    const uint8_t* const endAddr_;
    const uint8_t* currentReadAddr_;
    cppType currentValue_ = 0;
    bool currentValueValid_ = true;
    bool* const parseStatus_;
};

enum ParseProtoStatus { ABORT, SKIP, OK };
struct ParseDataAreaResult {
    ParseProtoStatus status;
    const uint8_t* next;
    DataArea dataArea;
};

class ProtoReaderBase {
public:
    ProtoReaderBase() : startAddr_(0), endAddr_(0), dataAreas_(nullptr), dataAreasCount_(0), size_(0), volume_(0) {}
    ProtoReaderBase(DataArea* storage, uint32_t dataAreasCount, const uint8_t* buffer, size_t length);
    const uint8_t* GetStartAddr() const
    {
        return startAddr_;
    }
    const uint8_t* GetEndAddr() const
    {
        return endAddr_;
    }
    void ResetCurrentAddr()
    {
        currentReadAddr_ = startAddr_;
    }

    void ResetCurrentAddr(const uint8_t* pos)
    {
        currentReadAddr_ = pos;
    }

    size_t GetCurrentOffset() const
    {
        return static_cast<size_t>(currentReadAddr_ - startAddr_);
    }

    size_t GetCurrentLeft() const
    {
        return static_cast<size_t>(endAddr_ - currentReadAddr_);
    }

    DataArea ReadNextDataArea();
    DataArea FindDataArea(uint32_t dataAreaId);
    const DataArea& Get(uint32_t id) const
    {
        if (id < dataAreasCount_) {
            return dataAreas_[id];
        }
        return dataAreas_[0];
    }

    template <typename T>
    RepeatedDataAreaIterator<T> GetRepeated(uint32_t dataAreaId) const
    {
        return RepeatedDataAreaIterator<T>(dataAreaId, &dataAreas_[dataAreasCount_], &dataAreas_[size_],
                                           &dataAreas_[dataAreaId]);
    }

    template <ProtoWireType wireType, typename cppType>
    PackedRepeatedDataAreaIterator<wireType, cppType> GetPackedRepeated(uint32_t dataAreaId, bool* parseErrorAddr) const
    {
        const DataArea& dataArea = Get(dataAreaId);
        if (dataArea.DataAreaValid()) {
            return PackedRepeatedDataAreaIterator<wireType, cppType>(dataArea.Data(), dataArea.Size(), parseErrorAddr);
        } else {
            return PackedRepeatedDataAreaIterator<wireType, cppType>(nullptr, 0, parseErrorAddr);
        }
    }

protected:
    ParseDataAreaResult ParseOneDataArea(const uint8_t* const startAddr, const uint8_t* const endAddr);
    void ParseAllDataAreas();
    void MoveToLargerHeapStorage();

    const uint8_t* const startAddr_;
    const uint8_t* const endAddr_;
    const uint8_t* currentReadAddr_ = nullptr;
    std::unique_ptr<DataArea[]> lagerHeapStorage_;
    DataArea* dataAreas_;
    uint32_t dataAreasCount_;
    uint32_t size_;
    uint32_t volume_;

private:
    const uint8_t* GetNextProtoTag(const uint8_t* const startAddr, const uint8_t* const endAddr, uint64_t* dataAreaTag);
    bool ParseVarIntValue(ParseDataAreaResult& result, const uint8_t* startAddr, const uint8_t* const endAddr);
    bool ParseLengthDelimitedValue(ParseDataAreaResult& result, const uint8_t* startAddr, const uint8_t* const endAddr);
    bool ParseFixed64Value(ParseDataAreaResult& result, const uint8_t* startAddr, const uint8_t* const endAddr);
    bool ParseFixed32Value(ParseDataAreaResult& result, const uint8_t* startAddr, const uint8_t* const endAddr);
    using ParseDataAreaValueByType = std::function<bool(ParseDataAreaResult&, const uint8_t*, const uint8_t* const)>;
    std::map<ProtoWireType, ParseDataAreaValueByType> dataAreaTypeToParseFuncMap_ = {};
};

template <int32_t MAX_DATA_AREA_ID>
class TypedProtoReader : public ProtoReaderBase {
public:
    TypedProtoReader(const uint8_t* buffer, size_t length)
        : ProtoReaderBase(defaultStorage_, MAX_DATA_AREA_ID + 1, buffer, length)
    {
        ProtoReaderBase::ParseAllDataAreas();
    }
    TypedProtoReader(TypedProtoReader&& other) noexcept : ProtoReaderBase(std::move(other))
    {
        if (dataAreas_ == other.defaultStorage_) {
            dataAreas_ = defaultStorage_;
            memcpy_s(defaultStorage_, sizeof(defaultStorage_), other.defaultStorage_, sizeof(defaultStorage_));
        }
    }
    template <uint32_t DATA_AREA_ID>
    const DataArea& at() const
    {
        return dataAreas_[DATA_AREA_ID];
    }

private:
    DataArea defaultStorage_[MAX_DATA_AREA_ID + 1];
};
} // namespace ProtoReader
} // namespace SysTuning
#endif // TS_PROTO_READER_H_
