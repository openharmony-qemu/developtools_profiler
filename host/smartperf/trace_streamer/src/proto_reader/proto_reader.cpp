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

#include "proto_reader.h"
#include "optimize.h"

namespace SysTuning {
namespace ProtoReader {
ProtoReaderBase::ProtoReaderBase(DataArea* storage, uint32_t dataAreasCount, const uint8_t* buffer, size_t length)
    : startAddr_(buffer),
      endAddr_(startAddr_ + length),
      currentReadAddr_(startAddr_),
      dataAreas_(storage),
      dataAreasCount_(dataAreasCount),
      size_(dataAreasCount),
      volume_(dataAreasCount)
{
    auto dataAreasSize = sizeof(DataArea) * dataAreasCount_;
    (void)memset_s(dataAreas_, dataAreasSize, 0, dataAreasSize);
    dataAreaTypeToParseFuncMap_ = {
        {ProtoWireType::kVarInt, std::bind(&ProtoReaderBase::ParseVarIntValue, this, std::placeholders::_1,
                                           std::placeholders::_2, std::placeholders::_3)},
        {ProtoWireType::kLengthDelimited,
         std::bind(&ProtoReaderBase::ParseLengthDelimitedValue, this, std::placeholders::_1, std::placeholders::_2,
                   std::placeholders::_3)},
        {ProtoWireType::kFixed64, std::bind(&ProtoReaderBase::ParseFixed64Value, this, std::placeholders::_1,
                                            std::placeholders::_2, std::placeholders::_3)},
        {ProtoWireType::kFixed32, std::bind(&ProtoReaderBase::ParseFixed32Value, this, std::placeholders::_1,
                                            std::placeholders::_2, std::placeholders::_3)}};
}

// return next parse addr and dataAreaTag. if failed returns nullptr
const uint8_t* ProtoReaderBase::GetNextProtoTag(const uint8_t* const startAddr,
                                                const uint8_t* const endAddr,
                                                uint64_t* dataAreaTag)
{
    const uint8_t* cursor = startAddr;
    if (*cursor & BYTE_HIGHEST_BIT_MARK) {
        const uint8_t* nextAddr = VarIntDecode(cursor, endAddr, dataAreaTag);
        if (cursor == nextAddr) {
            return nullptr;
        }
        cursor = nextAddr;
    } else {
        *dataAreaTag = *(cursor++);
    }
    if (cursor >= endAddr) {
        return nullptr;
    }
    return cursor;
}

bool ProtoReaderBase::ParseVarIntValue(ParseDataAreaResult& result,
                                       const uint8_t* startAddr,
                                       const uint8_t* const endAddr)
{
    uint64_t intValue = 0;
    auto cursor = VarIntDecode(startAddr, endAddr, &intValue);
    if (cursor == startAddr) {
        return false;
    }
    result.dataArea.SetDataAreaIntValue(intValue);
    result.next = cursor;
    result.status = OK;
    return true;
}

bool ProtoReaderBase::ParseLengthDelimitedValue(ParseDataAreaResult& result,
                                                const uint8_t* startAddr,
                                                const uint8_t* const endAddr)
{
    uint64_t length = 0;
    auto cursor = VarIntDecode(startAddr, endAddr, &length);
    if (cursor == startAddr || length > static_cast<uint64_t>(endAddr - cursor)) {
        return false;
    }
    const uintptr_t dataStartAddr = reinterpret_cast<uintptr_t>(cursor);
    result.dataArea.SetDataAreaIntValue(dataStartAddr);
    result.dataArea.SetDataAreaSize(length);
    result.next = cursor + length;
    if (length > kMaxMessageLength) {
        TS_LOGD("Skip this data, because it is too large. length: %d", length);
        result.status = SKIP;
        return true;
    }
    result.status = OK;
    return true;
}

bool ProtoReaderBase::ParseFixed64Value(ParseDataAreaResult& result,
                                        const uint8_t* startAddr,
                                        const uint8_t* const endAddr)
{
    uint64_t intValue = 0;
    auto cursor = startAddr + sizeof(uint64_t);
    if (cursor > endAddr) {
        return false;
    }

    (void)memcpy_s(&intValue, sizeof(uint64_t), startAddr, sizeof(uint64_t));
    result.dataArea.SetDataAreaIntValue(intValue);
    result.next = cursor;
    result.status = OK;
    return true;
}

bool ProtoReaderBase::ParseFixed32Value(ParseDataAreaResult& result,
                                        const uint8_t* startAddr,
                                        const uint8_t* const endAddr)
{
    uint64_t intValue = 0;
    auto cursor = startAddr + sizeof(uint32_t);
    if (cursor > endAddr) {
        return false;
    }
    (void)memcpy_s(&intValue, sizeof(uint64_t), startAddr, sizeof(uint32_t));
    result.dataArea.SetDataAreaIntValue(intValue);
    result.next = cursor;
    result.status = OK;
    return true;
}

ParseDataAreaResult ProtoReaderBase::ParseOneDataArea(const uint8_t* const startAddr, const uint8_t* const endAddr)
{
    ParseDataAreaResult result = {ABORT, startAddr, DataArea{}};
    if (!startAddr || startAddr >= endAddr) {
        return result;
    }
    uint64_t dataAreaTag = 0;
    auto cursor = GetNextProtoTag(startAddr, endAddr, &dataAreaTag);
    if (!cursor) {
        return result;
    }
    uint32_t dataAreaId = static_cast<uint32_t>(dataAreaTag >> DATA_AREA_TYPE_BITS);
    if (dataAreaId == 0) {
        return result;
    }
    if (TS_UNLIKELY(dataAreaId > std::numeric_limits<uint16_t>::max())) {
        TS_LOGD("Skip dataArea %d because its too big", dataAreaId);
        result.status = ParseProtoStatus::SKIP;
        result.next = cursor;
        return result;
    }
    result.dataArea.SetDataAreaId(dataAreaId);

    auto dataAreaType = static_cast<uint8_t>(dataAreaTag) & DATA_AREA_TYPE_VALUE;
    result.dataArea.SetDataAreaType(dataAreaType);

    auto itor = dataAreaTypeToParseFuncMap_.find(static_cast<ProtoWireType>(dataAreaType));
    if (itor == dataAreaTypeToParseFuncMap_.end()) {
        return result;
    }
    itor->second(result, cursor, endAddr_);
    return result;
}

DataArea ProtoReaderBase::FindDataArea(uint32_t dataAreaId)
{
    DataArea dataArea{};
    auto temp = currentReadAddr_;
    currentReadAddr_ = startAddr_;
    for (auto nextDataArea = ReadNextDataArea(); nextDataArea.DataAreaValid(); nextDataArea = ReadNextDataArea()) {
        if (nextDataArea.DataAreaId() == dataAreaId) {
            dataArea = nextDataArea;
            break;
        }
    }
    currentReadAddr_ = temp;
    return dataArea;
}
TS_INLINE
DataArea ProtoReaderBase::ReadNextDataArea()
{
    ParseDataAreaResult result = {ABORT, currentReadAddr_, DataArea{}};
    do {
        result = ParseOneDataArea(currentReadAddr_, endAddr_);
        currentReadAddr_ = result.next;
    } while (result.status == ParseProtoStatus::SKIP);
    return result.dataArea;
}

void ProtoReaderBase::ParseAllDataAreas()
{
    const uint8_t* cur = startAddr_;
    ParseDataAreaResult result = {ABORT, startAddr_, DataArea{}};
    while (true) {
        result = ParseOneDataArea(cur, endAddr_);
        cur = result.next;
        if (result.status == ParseProtoStatus::SKIP) {
            continue;
        } else if (result.status == ParseProtoStatus::ABORT) {
            break;
        }
        auto dataAreaId = result.dataArea.DataAreaId();
        if (dataAreaId >= dataAreasCount_) {
            continue;
        }

        DataArea* dataArea = &dataAreas_[dataAreaId];
        if (!dataArea->DataAreaValid()) {
            *dataArea = std::move(result.dataArea);
        } else {
            if (size_ >= volume_) {
                MoveToLargerHeapStorage();
                dataArea = &dataAreas_[dataAreaId];
            }
            dataAreas_[size_++] = *dataArea;
            *dataArea = std::move(result.dataArea);
        }
    }
    currentReadAddr_ = result.next;
}

void ProtoReaderBase::MoveToLargerHeapStorage()
{
    uint32_t largerVolume = volume_ << 1;
    std::unique_ptr<DataArea[]> largerVolumeStorage(new DataArea[largerVolume]);
    (void)memcpy_s(&largerVolumeStorage[0], sizeof(DataArea) * largerVolume, dataAreas_, sizeof(DataArea) * size_);
    lagerHeapStorage_ = std::move(largerVolumeStorage);
    dataAreas_ = &lagerHeapStorage_[0];
    volume_ = largerVolume;
}
} // namespace ProtoReader
} // namespace SysTuning
