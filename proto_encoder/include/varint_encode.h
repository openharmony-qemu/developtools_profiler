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
#ifndef VARINT_ENCODE_H
#define VARINT_ENCODE_H
#include <cinttypes>
#include <cstddef>
#include <type_traits>
#include <cstdint>

namespace OHOS {
namespace Developtools {
namespace Profiler {
namespace ProtoEncoder {
constexpr uint32_t VARINT32_SIZE = 4;
constexpr uint32_t VARINT32_ENCODE_SIZE = 5;
constexpr uint32_t VARINT64_SIZE = 8;
constexpr uint32_t VARINT64_ENCODE_SIZE = 10;
constexpr uint32_t VARINT_ENCODE_MAX_SIZE = VARINT64_ENCODE_SIZE;

constexpr uint32_t VARINT_MAX_1BYTE = (1u << (7 * 1)) - 1;
constexpr uint32_t VARINT_MAX_2BYTE = (1u << (7 * 2)) - 1;
constexpr uint32_t VARINT_MAX_3BYTE = (1u << (7 * 3)) - 1;
constexpr uint32_t VARINT_MAX_4BYTE = (1u << (7 * 4)) - 1;

constexpr uint8_t VARINT_PAYLOAD_BITS = 7;
constexpr uint8_t VARINT_MASK_PAYLOAD = 0x7F;
constexpr uint8_t VARINT_MASK_MSB = 0x80;

inline uint32_t GetPackedVarintLenSize(uint32_t itemCount, uint32_t itemSize, uint32_t& len)
{
    len = itemCount;
    if (itemSize == VARINT32_SIZE) {
        len = itemCount * VARINT32_ENCODE_SIZE;
    } else if (itemSize == VARINT64_SIZE) {
        len = itemCount * VARINT64_ENCODE_SIZE;
    } // else bool is 1 byte

    const uint32_t one = 1;
    const uint32_t two = 2;
    const uint32_t three = 3;
    const uint32_t four = 4;
    if (len <= VARINT_MAX_1BYTE) {
        return one;
    } else if (len <= VARINT_MAX_2BYTE) {
        return two;
    } else if (len <= VARINT_MAX_3BYTE) {
        return three;
    } else if (len <= VARINT_MAX_4BYTE) {
        return four;
    }

    return 0; // illegal, too large
}

template<typename T>
inline typename std::make_unsigned<T>::type EncodeZigZag(T v)
{
    if (v >= 0) {
        return ((typename std::make_unsigned<T>::type)(v) << 1);
    }

    return ((typename std::make_unsigned<T>::type)(~v) << 1) + 1;
}

template<typename T>
inline uint32_t EncodeVarint(uint8_t* buf, T v)
{
    // https://developers.google.com/protocol-buffers/docs/encoding
    // Unsigned Integers and Signed Integers(intN)
    uint64_t value = static_cast<uint64_t>(v);
    uint32_t size = 0;
    while (value > static_cast<uint64_t>(VARINT_MAX_1BYTE)) {
        buf[size] = (VARINT_MASK_PAYLOAD & value) | VARINT_MASK_MSB;
        size++;
        value >>= VARINT_PAYLOAD_BITS;
    }
    buf[size] = (VARINT_MASK_PAYLOAD & value);
    size++;

    return size;
}

template<typename T>
inline uint32_t EncodeZigZagVarint(uint8_t* buf, T v)
{
    return EncodeVarint(buf, EncodeZigZag(v));
}

template<typename T>
inline void EncodeVarintPadding(uint8_t* buf, T v, uint32_t paddingSize)
{
    uint32_t size = 0;
    paddingSize--;
    while (size < paddingSize) {
        buf[size] = (VARINT_MASK_PAYLOAD & v) | VARINT_MASK_MSB;
        size++;
        v >>= VARINT_PAYLOAD_BITS;
    }
    buf[size] = (VARINT_MASK_PAYLOAD & v);
}
} // namespace ProtoEncoder
} // namespace Profiler
} // namespace Developtools
} // namespace OHOS
#endif // VARINT_ENCODE_H
