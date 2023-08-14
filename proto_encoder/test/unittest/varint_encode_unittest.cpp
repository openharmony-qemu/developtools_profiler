/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdlib>
#include <limits>
#include <vector>
#include <gtest/gtest.h>

#include "varint_encode.h"

namespace OHOS {
namespace Developtools {
namespace Profiler {
using namespace testing::ext;
using namespace ProtoEncoder;

class VarintEncodeUnittest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {};
    void TearDown() {};

    const uint32_t two = 2;
    const uint32_t three = 3;
    const uint32_t four = 4;
};

HWTEST_F(VarintEncodeUnittest, GetPackedVarintLenSize, TestSize.Level1)
{
    uint32_t len = 0;
    uint32_t itemCount = 0;
    uint32_t itemSize = sizeof(char);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)1);
    EXPECT_EQ(len, (uint32_t)0);

    itemSize = sizeof(int32_t);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)1);
    EXPECT_EQ(len, itemCount * itemSize);

    itemSize = sizeof(int64_t);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)1);
    EXPECT_EQ(len, itemCount * itemSize);

    itemSize = sizeof(char);
    itemCount = VARINT_MAX_1BYTE;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)1);
    EXPECT_EQ(len, itemCount * itemSize);

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), two);
    EXPECT_EQ(len, itemCount * itemSize);

    itemCount = VARINT_MAX_2BYTE;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), two);
    EXPECT_EQ(len, itemCount * itemSize);

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), three);
    EXPECT_EQ(len, itemCount * itemSize);

    itemCount = VARINT_MAX_3BYTE;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), three);
    EXPECT_EQ(len, itemCount * itemSize);

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), four);
    EXPECT_EQ(len, itemCount * itemSize);

    itemCount = VARINT_MAX_4BYTE;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), four);
    EXPECT_EQ(len, itemCount * itemSize);

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)0);
    EXPECT_EQ(len, itemCount * itemSize);

    itemSize = sizeof(int32_t);
    itemCount = VARINT_MAX_1BYTE / (itemSize + 1);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)1);
    EXPECT_EQ(len, itemCount * (itemSize + 1));

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), two);
    EXPECT_EQ(len, itemCount * (itemSize + 1));

    itemCount = VARINT_MAX_2BYTE / (itemSize + 1);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), two);
    EXPECT_EQ(len, itemCount * (itemSize + 1));

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), three);
    EXPECT_EQ(len, itemCount * (itemSize + 1));

    itemCount = VARINT_MAX_3BYTE / (itemSize + 1);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), three);
    EXPECT_EQ(len, itemCount * (itemSize + 1));

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), four);
    EXPECT_EQ(len, itemCount * (itemSize + 1));

    itemCount = VARINT_MAX_4BYTE / (itemSize + 1);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), four);
    EXPECT_EQ(len, itemCount * (itemSize + 1));

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)0);
    EXPECT_EQ(len, itemCount * (itemSize + 1));

    itemSize = sizeof(int64_t);
    itemCount = VARINT_MAX_1BYTE / (itemSize + two);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)1);
    EXPECT_EQ(len, itemCount * (itemSize + two));

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), two);
    EXPECT_EQ(len, itemCount * (itemSize + two));

    itemCount = VARINT_MAX_2BYTE / (itemSize + two);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), two);
    EXPECT_EQ(len, itemCount * (itemSize + two));

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), three);
    EXPECT_EQ(len, itemCount * (itemSize + two));

    itemCount = VARINT_MAX_3BYTE / (itemSize + two);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), three);
    EXPECT_EQ(len, itemCount * (itemSize + two));

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), four);
    EXPECT_EQ(len, itemCount * (itemSize + two));

    itemCount = VARINT_MAX_4BYTE / (itemSize + two);
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), four);
    EXPECT_EQ(len, itemCount * (itemSize + two));

    itemCount++;
    EXPECT_EQ(GetPackedVarintLenSize(itemCount, itemSize, len), (uint32_t)0);
    EXPECT_EQ(len, itemCount * (itemSize + two));
}

HWTEST_F(VarintEncodeUnittest, EncodeZigZag, TestSize.Level1)
{
    int8_t signed8 = 0;
    EXPECT_EQ(EncodeZigZag(signed8), (uint8_t)0);
    signed8 = -1;
    EXPECT_EQ(EncodeZigZag(signed8), (uint8_t)1);
    signed8 = 1;
    EXPECT_EQ(EncodeZigZag(signed8), (uint8_t)(signed8 * two));
    std::srand(std::time(nullptr));
    int rand = std::rand();
    signed8 = static_cast<int8_t>(rand);
    if (signed8 >= 0) {
        // n are encoded as 2 * n
        EXPECT_EQ(EncodeZigZag(signed8), (uint8_t)(signed8 * two));
    } else {
        // -n are encoded as 2 * n + 1
        EXPECT_EQ(EncodeZigZag(signed8), (uint8_t)(-signed8 - 1 + -signed8));
    }
    signed8 = std::numeric_limits<int8_t>::min();
    EXPECT_EQ(EncodeZigZag(signed8), std::numeric_limits<uint8_t>::max());
    signed8 = std::numeric_limits<int8_t>::max();
    EXPECT_EQ(EncodeZigZag(signed8), std::numeric_limits<uint8_t>::max() - 1);

    int32_t signed32 = 0;
    EXPECT_EQ(EncodeZigZag(signed32), (uint32_t)0);
    signed32 = -1;
    EXPECT_EQ(EncodeZigZag(signed32), (uint32_t)1);
    signed32 = 1;
    EXPECT_EQ(EncodeZigZag(signed32), (uint32_t)(signed32 * two));
    std::srand(rand);
    rand = std::rand();
    signed32 = static_cast<int32_t>(rand);
    if (signed32 >= 0) {
        // n are encoded as 2 * n
        EXPECT_EQ(EncodeZigZag(signed32), (uint32_t)(signed32 * two));
    } else {
        // -n are encoded as 2 * n + 1
        EXPECT_EQ(EncodeZigZag(signed32), (uint32_t)(-signed32 - 1 + -signed32));
    }
    signed32 = std::numeric_limits<int32_t>::min();
    EXPECT_EQ(EncodeZigZag(signed32), std::numeric_limits<uint32_t>::max());
    signed32 = std::numeric_limits<int32_t>::max();
    EXPECT_EQ(EncodeZigZag(signed32), std::numeric_limits<uint32_t>::max() - 1);

    int64_t signed64 = 0;
    EXPECT_EQ(EncodeZigZag(signed64), (uint64_t)0);
    signed64 = -1;
    EXPECT_EQ(EncodeZigZag(signed64), (uint64_t)1);
    signed64 = 1;
    EXPECT_EQ(EncodeZigZag(signed64), (uint64_t)(signed64 * two));
    std::srand(rand);
    rand = std::rand();
    signed64 = static_cast<int64_t>(rand);
    if (signed64 >= 0) {
        // n are encoded as 2 * n
        EXPECT_EQ(EncodeZigZag(signed64), (uint64_t)(signed64 * two));
    } else {
        // -n are encoded as 2 * n + 1
        EXPECT_EQ(EncodeZigZag(signed64), (uint64_t)(-signed64 - 1 + -signed64));
    }
    signed64 = std::numeric_limits<int64_t>::min();
    EXPECT_EQ(EncodeZigZag(signed64), std::numeric_limits<uint64_t>::max());
    signed64 = std::numeric_limits<int64_t>::max();
    EXPECT_EQ(EncodeZigZag(signed64), std::numeric_limits<uint64_t>::max() - 1);
}

bool CompareBytes(const uint8_t* a, const uint8_t* b, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

const uint8_t ENCODE_BYTES_MIN_BOOL[] = {0x00};
const uint8_t ENCODE_BYTES_MAX_BOOL[] = {0x01};
const uint8_t ENCODE_BYTES_MIN_S8[] = {0x80, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};
const uint8_t ENCODE_BYTES_MAX_S8[] = {0x7F};
const uint8_t ENCODE_BYTES_MIN_U8[] = {0x00};
const uint8_t ENCODE_BYTES_MAX_U8[] = {0xFF, 0x01};
const uint8_t ENCODE_BYTES_MIN_S32[] = {0x80, 0x80, 0x80, 0x80, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};
const uint8_t ENCODE_BYTES_MAX_S32[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x07};
const uint8_t ENCODE_BYTES_MIN_U32[] = {0x00};
const uint8_t ENCODE_BYTES_MAX_U32[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x0F};
const uint8_t ENCODE_BYTES_MIN_S64[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01};
const uint8_t ENCODE_BYTES_MAX_S64[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F};
const uint8_t ENCODE_BYTES_MIN_U64[] = {0x00};
const uint8_t ENCODE_BYTES_MAX_U64[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};

HWTEST_F(VarintEncodeUnittest, EncodeVarint, TestSize.Level1)
{
    uint8_t buf[VARINT_ENCODE_MAX_SIZE] = {0};
    bool b = std::numeric_limits<bool>::min();
    EXPECT_EQ(EncodeVarint(buf, b), (uint32_t)sizeof(ENCODE_BYTES_MIN_BOOL));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MIN_BOOL, sizeof(ENCODE_BYTES_MIN_BOOL)));
    b = std::numeric_limits<bool>::max();
    EXPECT_EQ(EncodeVarint(buf, b), (uint32_t)sizeof(ENCODE_BYTES_MAX_BOOL));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MAX_BOOL, sizeof(ENCODE_BYTES_MAX_BOOL)));

    int8_t s8 = std::numeric_limits<int8_t>::min();
    EXPECT_EQ(EncodeVarint(buf, s8), (uint32_t)sizeof(ENCODE_BYTES_MIN_S8));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MIN_S8, sizeof(ENCODE_BYTES_MIN_S8)));
    s8 = std::numeric_limits<int8_t>::max();
    EXPECT_EQ(EncodeVarint(buf, s8), (uint32_t)sizeof(ENCODE_BYTES_MAX_S8));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MAX_S8, sizeof(ENCODE_BYTES_MAX_S8)));

    uint8_t u8 = std::numeric_limits<uint8_t>::min();
    EXPECT_EQ(EncodeVarint(buf, u8), (uint32_t)sizeof(ENCODE_BYTES_MIN_U8));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MIN_U8, sizeof(ENCODE_BYTES_MIN_U8)));
    u8 = std::numeric_limits<uint8_t>::max();
    EXPECT_EQ(EncodeVarint(buf, u8), (uint32_t)sizeof(ENCODE_BYTES_MAX_U8));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MAX_U8, sizeof(ENCODE_BYTES_MAX_U8)));

    int32_t s32 = std::numeric_limits<int32_t>::min();
    EXPECT_EQ(EncodeVarint(buf, s32), (uint32_t)sizeof(ENCODE_BYTES_MIN_S32));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MIN_S32, sizeof(ENCODE_BYTES_MIN_S32)));
    s32 = std::numeric_limits<int32_t>::max();
    EXPECT_EQ(EncodeVarint(buf, s32), (uint32_t)sizeof(ENCODE_BYTES_MAX_S32));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MAX_S32, sizeof(ENCODE_BYTES_MAX_S32)));

    uint32_t u32 = std::numeric_limits<uint32_t>::min();
    EXPECT_EQ(EncodeVarint(buf, u32), (uint32_t)sizeof(ENCODE_BYTES_MIN_U32));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MIN_U32, sizeof(ENCODE_BYTES_MIN_U32)));
    u32 = std::numeric_limits<uint32_t>::max();
    EXPECT_EQ(EncodeVarint(buf, u32), (uint32_t)sizeof(ENCODE_BYTES_MAX_U32));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MAX_U32, sizeof(ENCODE_BYTES_MAX_U32)));

    int64_t s64 = std::numeric_limits<int64_t>::min();
    EXPECT_EQ(EncodeVarint(buf, s64), (uint32_t)sizeof(ENCODE_BYTES_MIN_S64));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MIN_S64, sizeof(ENCODE_BYTES_MIN_S64)));
    s64 = std::numeric_limits<int64_t>::max();
    EXPECT_EQ(EncodeVarint(buf, s64), (uint32_t)sizeof(ENCODE_BYTES_MAX_S64));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MAX_S64, sizeof(ENCODE_BYTES_MAX_S64)));

    uint64_t u64 = std::numeric_limits<uint64_t>::min();
    EXPECT_EQ(EncodeVarint(buf, u64), (uint32_t)sizeof(ENCODE_BYTES_MIN_U64));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MIN_U64, sizeof(ENCODE_BYTES_MIN_U64)));
    u64 = std::numeric_limits<uint64_t>::max();
    EXPECT_EQ(EncodeVarint(buf, u64), (uint32_t)sizeof(ENCODE_BYTES_MAX_U64));
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MAX_U64, sizeof(ENCODE_BYTES_MAX_U64)));
}

HWTEST_F(VarintEncodeUnittest, EncodeVarintPadding, TestSize.Level1)
{
    const uint8_t paddingByte = 0x80;
    uint8_t buf[VARINT_ENCODE_MAX_SIZE] = {0};
    uint64_t u64 = 1;
    EncodeVarintPadding(buf, u64, VARINT_ENCODE_MAX_SIZE);
    uint32_t i = 1;
    while (i < VARINT_ENCODE_MAX_SIZE - 1) {
        EXPECT_EQ(buf[i++], paddingByte);
    }
    EXPECT_EQ(buf[i], (uint8_t)0);

    u64 = std::numeric_limits<uint64_t>::max();
    EncodeVarintPadding(buf, u64, VARINT_ENCODE_MAX_SIZE);
    EXPECT_TRUE(CompareBytes(buf, ENCODE_BYTES_MAX_U64, sizeof(ENCODE_BYTES_MAX_U64)));
}
} // namespace Profiler
} // namespace Developtools
} // namespace OHOS
