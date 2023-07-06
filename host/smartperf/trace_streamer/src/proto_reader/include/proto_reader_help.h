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

#ifndef PROTO_READER_HELP_H
#define PROTO_READER_HELP_H

#include <map>
#include <string>
#include <vector>

namespace SysTuning {
namespace ProtoReader {
class BytesView {
public:
    BytesView() : data_(nullptr), size_(0) {}
    BytesView(const uint8_t* data, size_t size) : data_(data), size_(size) {}
    BytesView(const BytesView& bytesView)
    {
        data_ = bytesView.data_;
        size_ = bytesView.size_;
    }
    BytesView& operator=(const BytesView& bytesView)
    {
        data_ = bytesView.data_;
        size_ = bytesView.size_;
        return *this;
    }
    std::string ToStdString() const
    {
        return std::string(reinterpret_cast<const char*>(data_), size_);
    }
    size_t Size() const
    {
        return size_;
    }
    const uint8_t* Data() const
    {
        return data_;
    }
    const uint8_t* data_;
    size_t size_;
};

struct CharsView {
    std::string ToStdString() const
    {
        return std::string(data, size);
    }

    const char* data;
    size_t size;
};

enum class ProtoWireType : uint8_t {
    kVarInt = 0,
    kFixed64 = 1,
    kLengthDelimited = 2,
    kFixed32 = 5,
};

inline uint32_t CreateTagVarInt(uint32_t DataAreaId)
{
    return (DataAreaId << 3) | static_cast<uint32_t>(ProtoWireType::kVarInt);
}

inline char Lowercase(char c)
{
    return ('A' <= c && c <= 'Z') ? static_cast<char>(c - ('A' - 'a')) : c;
}

inline char Uppercase(char c)
{
    return ('a' <= c && c <= 'z') ? static_cast<char>(c + ('A' - 'a')) : c;
}

inline std::string ToUppercase(const std::string& str)
{
    std::string string(str);
    auto end = string.end();
    for (auto c = string.begin(); c != end; ++c)
        *c = Uppercase(*c);
    return string;
}

inline std::string ToLowercase(const std::string& str)
{
    std::string string(str);
    auto end = string.end();
    for (auto c = string.begin(); c != end; ++c)
        *c = Lowercase(*c);
    return string;
}

constexpr size_t kMessageLengthFieldSize = 4;
constexpr size_t kMaxMessageLength = (1u << (kMessageLengthFieldSize * 7)) - 1;

constexpr size_t kMaxTagEncodedSize = 5;
constexpr size_t kMaxSimpleFieldEncodedSize = kMaxTagEncodedSize + 10;

template <typename T>
inline typename std::make_signed<T>::type ZigZagDecode(T value)
{
    using UnsignedType = typename std::make_unsigned<T>::type;
    using SignedType = typename std::make_signed<T>::type;
    auto unsignedValue = static_cast<UnsignedType>(value);
    auto unsignedmask = static_cast<UnsignedType>(-static_cast<SignedType>(unsignedValue & 1));
    return static_cast<SignedType>((unsignedValue >> 1) ^ unsignedmask);
}

constexpr uint8_t varIntValueBits = 7;
constexpr uint8_t varIntValueMask = 0x7f; // 111 1111b
constexpr uint8_t varIntValueDecodeMaxOffset = 64;
constexpr uint8_t byteHighestBitMark = 0x80; // 1000 0000b
inline const uint8_t* VarIntDecode(const uint8_t* start, const uint8_t* end, uint64_t* varIntValue)
{
    const uint8_t* cursor = start;
    uint64_t temp = 0;
    uint32_t shift = 0;
    do {
        uint8_t currentByte = *cursor++;
        temp |= static_cast<uint64_t>(currentByte & varIntValueMask) << shift;
        if (!(currentByte & byteHighestBitMark)) {
            *varIntValue = temp;
            return cursor;
        }
        shift += varIntValueBits;
    } while (cursor < end && shift < varIntValueDecodeMaxOffset);
    *varIntValue = 0;
    return start;
}
} // namespace ProtoReader
} // namespace SysTuning
#endif // PROTO_READER_HELP_H
