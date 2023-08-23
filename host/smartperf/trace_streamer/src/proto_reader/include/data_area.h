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

#ifndef PROTO_READER_FIELD_H
#define PROTO_READER_FIELD_H

#include <stdint.h>
#include <string.h>
#include <vector>
#include "proto_reader_help.h"
#include "string_help.h"
namespace SysTuning {
namespace ProtoReader {
class DataArea {
public:
    bool DataAreaValid() const
    {
        return dataAreaID_ != 0;
    }
    uint16_t DataAreaId() const
    {
        return dataAreaID_;
    }
    explicit operator bool() const
    {
        return DataAreaValid();
    }

    ProtoWireType Type() const
    {
        auto res = static_cast<ProtoWireType>(type_);
        return res;
    }

    bool ToBool() const
    {
        return static_cast<bool>(intValue_);
    }

    uint32_t ToUint32() const
    {
        return static_cast<uint32_t>(intValue_);
    }

    int32_t ToInt32() const
    {
        return static_cast<int32_t>(intValue_);
    }

    int32_t ToSint32() const
    {
        return ZigZagDecode(static_cast<uint32_t>(intValue_));
    }

    uint64_t ToUint64() const
    {
        return intValue_;
    }

    int64_t ToInt64() const
    {
        return static_cast<int64_t>(intValue_);
    }

    int64_t ToSint64() const
    {
        return ZigZagDecode(static_cast<uint64_t>(intValue_));
    }

    float ToFloat() const
    {
        float res;
        uint32_t value32 = static_cast<uint32_t>(intValue_);
        (void)memcpy_s(&res, sizeof(res), &value32, sizeof(value32));
        return res;
    }

    double ToDouble() const
    {
        double res;
        (void)memcpy_s(&res, sizeof(res), &intValue_, sizeof(intValue_));
        return res;
    }

    CharsView ToString() const
    {
        return CharsView{reinterpret_cast<const char*>(Data()), size_};
    }

    std::string ToStdString() const
    {
        return ToString().ToStdString();
    }

    BytesView ToBytes() const
    {
        return BytesView(Data(), size_);
    }

    const uint8_t* Data() const
    {
        return reinterpret_cast<const uint8_t*>(intValue_);
    }

    size_t Size() const
    {
        return size_;
    }

    uint64_t RawIntValue() const
    {
        return intValue_;
    }

    void SetDataAreaId(uint16_t id)
    {
        dataAreaID_ = id;
    }
    void SetDataAreaType(uint8_t type)
    {
        type_ = type;
    }
    void SetDataAreaIntValue(uint64_t intValue)
    {
        intValue_ = intValue;
    }
    void SetDataAreaSize(uint32_t size)
    {
        size_ = size;
    }
    void Initialize(uint16_t id, uint8_t type, uint64_t intValue, uint32_t size)
    {
        dataAreaID_ = id;
        type_ = type;
        intValue_ = intValue;
        size_ = size;
    }

    void GetValue(bool* val) const
    {
        *val = ToBool();
    }
    void GetValue(uint32_t* val) const
    {
        *val = ToUint32();
    }
    void GetValue(int32_t* val) const
    {
        *val = ToInt32();
    }
    void GetValue(uint64_t* val) const
    {
        *val = ToUint64();
    }
    void GetValue(int64_t* val) const
    {
        *val = ToInt64();
    }
    void GetValue(float* val) const
    {
        *val = ToFloat();
    }
    void GetValue(double* val) const
    {
        *val = ToDouble();
    }
    void GetValue(std::string* val) const
    {
        *val = ToStdString();
    }
    void GetValue(CharsView* val) const
    {
        *val = ToString();
    }
    void GetValue(BytesView* val) const
    {
        *val = ToBytes();
    }
    void GetSignedValue(int32_t* val) const
    {
        *val = ToSint32();
    }
    void GetSignedValue(int64_t* val) const
    {
        *val = ToSint64();
    }

    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
    void getT(T* val) const
    {
        *val = static_cast<T>(ToInt32());
    }

private:
    uint64_t intValue_;
    uint32_t size_;
    uint16_t dataAreaID_;
    uint8_t type_;
};
} // namespace ProtoReader
} // namespace SysTuning
#endif // PROTO_READER_FIELD_H
