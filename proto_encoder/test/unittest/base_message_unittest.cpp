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
#include <limits>
#include <memory>
#include <vector>
#include <gtest/gtest.h>

#include "base_message.h"
#include "example.pb.h"
#include "example.pbencoder.h"

namespace OHOS {
namespace Developtools {
namespace Profiler {
using namespace testing::ext;

constexpr uint32_t SIZE_BUFFER = 1024 * 1024;
static uint8_t g_buf[SIZE_BUFFER] = {0};
static uint32_t g_writePos = 0;

bool GetMemory(RandomWriteCtx* ctx, uint32_t size, uint8_t** memory, uint32_t* offset)
{
    if (g_writePos + size > sizeof(g_buf)) {
        return false;
    }

    *memory = &g_buf[g_writePos];
    *offset = g_writePos;
    return true;
}

bool Seek(RandomWriteCtx* ctx, uint32_t offset)
{
    if (offset >= sizeof(g_buf)) {
        return false;
    }

    g_writePos = offset;
    return true;
}

static RandomWriteCtx g_writeCtx = {GetMemory, Seek};

class BaseMessageUnittest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp()
    {
        g_writePos = 0;
    }
    void TearDown() {};
};

HWTEST_F(BaseMessageUnittest, Size, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_int32(1);
    msgOpt.set_vint_int64(1);
    msgOpt.set_vint_uint32(1);
    msgOpt.set_vint_uint64(1);
    msgOpt.set_vint_sint32(1);
    msgOpt.set_vint_sint64(1);
    msgOpt.set_vint_bool(true);
    msgOpt.set_vint_enum(ProtoEncoder::NUM::ONE);
    ASSERT_EQ(g_writePos, (uint32_t)msgOpt.Size());

    const std::string str = "this is test.";
    msgOpt.set_len_string(str);
    msgOpt.set_len_bytes(str.data(), str.size());
    ASSERT_EQ(g_writePos, (uint32_t)msgOpt.Size());

    ProtoEncoder::SubMessage* msgSub = msgOpt.mutable_len_sub();
    ASSERT_TRUE(msgSub != nullptr);
    msgSub->set_vint_int32(1);
    msgSub->set_vint_int64(1);
    msgSub->set_vint_uint32(1);
    msgSub->set_vint_uint64(1);
    // has not backfill the length of subMessage
    ASSERT_GT(g_writePos, (uint32_t)msgOpt.Size());

    msgOpt.set_i64_fixed64(1);
    msgOpt.set_i64_sfixed64(1);
    msgOpt.set_i64_double(1);
    ASSERT_EQ(g_writePos, (uint32_t)msgOpt.Size());
}

HWTEST_F(BaseMessageUnittest, SetVarintInt32, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_int32(0);
    printf("set_vint_int32(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_int32(), 0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_int32(std::numeric_limits<int32_t>::min());
    printf("set_vint_int32(%" PRId32 "):\n", std::numeric_limits<int32_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_int32(), std::numeric_limits<int32_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_int32(std::numeric_limits<int32_t>::max());
    printf("set_vint_int32(%" PRId32 "):\n", std::numeric_limits<int32_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_int32(), std::numeric_limits<int32_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetVarintInt64, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_int64(0);
    printf("set_vint_int64(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_int64(), 0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_int64(std::numeric_limits<int64_t>::min());
    printf("set_vint_int64(%" PRId64 "):\n", std::numeric_limits<int64_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_int64(), std::numeric_limits<int64_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_int64(std::numeric_limits<int64_t>::max());
    printf("set_vint_int64(%" PRId64 "):\n", std::numeric_limits<int64_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_int64(), std::numeric_limits<int64_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetVarintUint32, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_uint32(0);
    printf("set_vint_uint32(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_uint32(), (uint32_t)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_uint32(std::numeric_limits<uint32_t>::min());
    printf("set_vint_uint32(%" PRIu32 "):\n", std::numeric_limits<uint32_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_uint32(), std::numeric_limits<uint32_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_uint32(std::numeric_limits<uint32_t>::max());
    printf("set_vint_uint32(%" PRIu32 "):\n", std::numeric_limits<uint32_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_uint32(), std::numeric_limits<uint32_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetVarintUint64, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_uint64(0);
    printf("set_vint_uint64(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_uint64(), (uint64_t)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_uint64(std::numeric_limits<uint64_t>::min());
    printf("set_vint_uint64(%" PRIu64 "):\n", std::numeric_limits<uint64_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_uint64(), std::numeric_limits<uint64_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_uint64(std::numeric_limits<uint64_t>::max());
    printf("set_vint_uint64(%" PRIu64 "):\n", std::numeric_limits<uint64_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_uint64(), std::numeric_limits<uint64_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetVarintSint32, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_sint32(0);
    printf("set_vint_sint32(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_sint32(), (int32_t)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_sint32(std::numeric_limits<int32_t>::min());
    printf("set_vint_sint32(%" PRId32 "):\n", std::numeric_limits<int32_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_sint32(), std::numeric_limits<int32_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_sint32(std::numeric_limits<int32_t>::max());
    printf("set_vint_sint32(%" PRId32 "):\n", std::numeric_limits<int32_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_sint32(), std::numeric_limits<int32_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetVarintSint64, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_sint64(0);
    printf("set_vint_sint64(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_sint64(), (int64_t)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_sint64(std::numeric_limits<int64_t>::min());
    printf("set_vint_sint64(%" PRId64 "):\n", std::numeric_limits<int64_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_sint64(), std::numeric_limits<int64_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_sint64(std::numeric_limits<int64_t>::max());
    printf("set_vint_sint64(%" PRId64 "):\n", std::numeric_limits<int64_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_sint64(), std::numeric_limits<int64_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetVarintBool, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_bool(std::numeric_limits<bool>::min());
    printf("set_vint_bool(%d):\n", std::numeric_limits<bool>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_bool(), std::numeric_limits<bool>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_bool(std::numeric_limits<int64_t>::max());
    printf("set_vint_bool(%d):\n", std::numeric_limits<bool>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_bool(), std::numeric_limits<bool>::max());
}

HWTEST_F(BaseMessageUnittest, SetVarintEnum, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_enum(ProtoEncoder::NUM::ZERO);
    printf("set_vint_enum(%d):\n", static_cast<int>(ProtoEncoder::NUM::ZERO));
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_enum(), static_cast<int>(ProtoEncoder::NUM::ZERO));

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_enum(ProtoEncoder::NUM::FOUR);
    printf("set_vint_enum(%d):\n", static_cast<int>(ProtoEncoder::NUM::FOUR));
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_enum(), static_cast<int>(ProtoEncoder::NUM::FOUR));
}

HWTEST_F(BaseMessageUnittest, SetFixed64, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_fixed64(0);
    printf("set_i64_fixed64(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_fixed64(), (uint64_t)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_fixed64(std::numeric_limits<uint64_t>::min());
    printf("set_i64_fixed64(%" PRIu64 "):\n", std::numeric_limits<uint64_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_fixed64(), std::numeric_limits<uint64_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_fixed64(std::numeric_limits<uint64_t>::max());
    printf("set_i64_fixed64(%" PRIu64 "):\n", std::numeric_limits<uint64_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_fixed64(), std::numeric_limits<uint64_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetSfixed64, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_sfixed64(0);
    printf("set_i64_sfixed64(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_sfixed64(), (int64_t)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_sfixed64(std::numeric_limits<int64_t>::min());
    printf("set_i64_sfixed64(%" PRId64 "):\n", std::numeric_limits<int64_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_sfixed64(), std::numeric_limits<int64_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_sfixed64(std::numeric_limits<int64_t>::max());
    printf("set_i64_sfixed64(%" PRId64 "):\n", std::numeric_limits<int64_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_sfixed64(), std::numeric_limits<int64_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetDouble, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_double(0);
    printf("set_i64_double(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_double(), (double)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_double(std::numeric_limits<double>::min());
    printf("set_i64_double(%e):\n", std::numeric_limits<double>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_double(), std::numeric_limits<double>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i64_double(std::numeric_limits<double>::max());
    printf("set_i64_double(%e):\n", std::numeric_limits<double>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i64_double(), std::numeric_limits<double>::max());
}

HWTEST_F(BaseMessageUnittest, SetFixed32, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_fixed32(0);
    printf("set_i32_fixed32(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_fixed32(), (uint32_t)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_fixed32(std::numeric_limits<uint32_t>::min());
    printf("set_i32_fixed32(%" PRIu32 "):\n", std::numeric_limits<uint32_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_fixed32(), std::numeric_limits<uint32_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_fixed32(std::numeric_limits<uint32_t>::max());
    printf("set_i32_fixed32(%" PRIu32 "):\n", std::numeric_limits<uint32_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_fixed32(), std::numeric_limits<uint32_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetSfixed32, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_sfixed32(0);
    printf("set_i32_sfixed32(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_sfixed32(), (int32_t)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_sfixed32(std::numeric_limits<int32_t>::min());
    printf("set_i32_sfixed32(%" PRId32 "):\n", std::numeric_limits<int32_t>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_sfixed32(), std::numeric_limits<int32_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_sfixed32(std::numeric_limits<int32_t>::max());
    printf("set_i32_sfixed32(%" PRId32 "):\n", std::numeric_limits<int32_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_sfixed32(), std::numeric_limits<int32_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetFloat, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_float(0);
    printf("set_i32_float(0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_float(), (float)0);

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_float(std::numeric_limits<float>::min());
    printf("set_i32_float(%e):\n", std::numeric_limits<float>::min());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_float(), std::numeric_limits<float>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_i32_float(std::numeric_limits<float>::max());
    printf("set_i32_float(%e):\n", std::numeric_limits<float>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.i32_float(), std::numeric_limits<float>::max());
}

HWTEST_F(BaseMessageUnittest, SetString, TestSize.Level1)
{
    const std::string emptyStr = "";
    const std::string str = "this is test.";
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_len_string(emptyStr);
    printf("set_len_string(empty string):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    // data of g_buf should is "AA 01 00"
    const int32_t emptyStrFieldLen = 3;
    ASSERT_EQ(msgOpt.Size(), emptyStrFieldLen);
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.len_string(), emptyStr);

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_len_string(str);
    printf("set_len_string(string: %s):\n", str.c_str());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.len_string(), str);

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_len_string(str.c_str());
    printf("set_len_string(char*: %s):\n", str.c_str());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.len_string(), str);
}

HWTEST_F(BaseMessageUnittest, SetSubMessage, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    // empty
    g_writeCtx.seek(&g_writeCtx, 0);
    ProtoEncoder::SubMessage* subMsgOpt = msgOpt.mutable_len_sub();
    ASSERT_TRUE(subMsgOpt != nullptr);
    msgOpt.set_vint_int32(std::numeric_limits<int32_t>::min());
    printf("mutable_len_sub: nothing:\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_TRUE(msgProtobuf.has_len_sub());

    // min
    g_writeCtx.seek(&g_writeCtx, 0);
    subMsgOpt = msgOpt.mutable_len_sub();
    ASSERT_TRUE(subMsgOpt != nullptr);
    subMsgOpt->set_vint_int32(std::numeric_limits<int32_t>::min());
    subMsgOpt->set_vint_int64(std::numeric_limits<int64_t>::min());
    subMsgOpt->set_vint_uint32(std::numeric_limits<uint32_t>::min());
    subMsgOpt->set_vint_uint64(std::numeric_limits<uint64_t>::min());

    msgOpt.set_vint_int32(std::numeric_limits<int32_t>::min());
    printf("mutable_len_sub: min:\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_TRUE(msgProtobuf.has_len_sub());
    SubMessage subMsgProtobuf = msgProtobuf.len_sub();
    ASSERT_EQ(subMsgProtobuf.vint_int32(), std::numeric_limits<int32_t>::min());
    ASSERT_EQ(subMsgProtobuf.vint_int64(), std::numeric_limits<int64_t>::min());
    ASSERT_EQ(subMsgProtobuf.vint_uint32(), std::numeric_limits<uint32_t>::min());
    ASSERT_EQ(subMsgProtobuf.vint_uint64(), std::numeric_limits<uint64_t>::min());

    // max
    g_writeCtx.seek(&g_writeCtx, 0);
    subMsgOpt = msgOpt.mutable_len_sub();
    ASSERT_TRUE(subMsgOpt != nullptr);
    subMsgOpt->set_vint_int32(std::numeric_limits<int32_t>::max());
    subMsgOpt->set_vint_int64(std::numeric_limits<int64_t>::max());
    subMsgOpt->set_vint_uint32(std::numeric_limits<uint32_t>::max());
    subMsgOpt->set_vint_uint64(std::numeric_limits<uint64_t>::max());

    msgOpt.set_vint_int32(std::numeric_limits<int32_t>::min());
    printf("mutable_len_sub: max:\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_TRUE(msgProtobuf.has_len_sub());
    subMsgProtobuf = msgProtobuf.len_sub();
    ASSERT_EQ(subMsgProtobuf.vint_int32(), std::numeric_limits<int32_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_int64(), std::numeric_limits<int64_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_uint32(), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_uint64(), std::numeric_limits<uint64_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetOneOf, TestSize.Level1)
{
    const std::string str = "this is test.";
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    // oneof_fixed64
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_oneof_fixed64(std::numeric_limits<uint64_t>::max());
    printf("set_oneof_fixed64(%" PRIu64"):\n", std::numeric_limits<uint64_t>::max());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.oneof_fixed64(), std::numeric_limits<uint64_t>::max());

    // oneof_sub
    ProtoEncoder::SubMessage* subMsgOpt = msgOpt.mutable_oneof_sub();
    ASSERT_TRUE(subMsgOpt != nullptr);
    subMsgOpt->set_vint_int32(std::numeric_limits<int32_t>::max());
    subMsgOpt->set_vint_int64(std::numeric_limits<int64_t>::max());
    subMsgOpt->set_vint_uint32(std::numeric_limits<uint32_t>::max());
    subMsgOpt->set_vint_uint64(std::numeric_limits<uint64_t>::max());
    msgOpt.set_i32_fixed32(0);
    printf("mutable_oneof_sub: max:\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_TRUE(msgProtobuf.has_oneof_sub());
    SubMessage subMsgProtobuf = msgProtobuf.oneof_sub();
    ASSERT_EQ(subMsgProtobuf.vint_int32(), std::numeric_limits<int32_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_int64(), std::numeric_limits<int64_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_uint32(), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_uint64(), std::numeric_limits<uint64_t>::max());

    // oneof_string
    msgOpt.set_oneof_string(str);
    printf("set_oneof_string(%s):\n", str.c_str());
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.oneof_string(), str);

    // last one wins
    ASSERT_NE(msgProtobuf.oneof_fixed64(), std::numeric_limits<uint64_t>::max());
    ASSERT_FALSE(msgProtobuf.has_oneof_sub());
}

HWTEST_F(BaseMessageUnittest, SetRepeatedPackedVarintSigned, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;
    const int32_t array[] = {0, 1,
                            std::numeric_limits<int32_t>::min(),
                            std::numeric_limits<int32_t>::max()};
    std::vector<int32_t> vec;
    vec.emplace_back(0);
    vec.emplace_back(1);
    vec.emplace_back(std::numeric_limits<int32_t>::min());
    vec.emplace_back(std::numeric_limits<int32_t>::max());

    // vector
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.add_len_repeated_packed_signed_vint(vec);
    printf("add_len_repeated_packed_signed_vint(vector):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    int count = msgProtobuf.len_repeated_packed_signed_vint_size();
    ASSERT_EQ(count, static_cast<int>(vec.size()));
    for (int i = 0; i < count; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_signed_vint(i), vec[i]);
    }

    // array
    msgOpt.add_len_repeated_packed_signed_vint(array, sizeof(array) / sizeof(array[0]));
    printf("add_len_repeated_packed_signed_vint(array):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    int countAll = msgProtobuf.len_repeated_packed_signed_vint_size();
    ASSERT_EQ(countAll, static_cast<int>(vec.size() + sizeof(array) / sizeof(array[0])));
    for (int i = 0; i < count; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_signed_vint(i), vec[i]);
    }
    for (int i = count; i < countAll; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_signed_vint(i), array[i - count]);
    }
}

HWTEST_F(BaseMessageUnittest, SetRepeatedPackedVarintUnsigned, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;
    const uint32_t array[] = {0, 1,
                            std::numeric_limits<uint32_t>::min(),
                            std::numeric_limits<uint32_t>::max()};
    std::vector<uint32_t> vec;
    vec.emplace_back(0);
    vec.emplace_back(1);
    vec.emplace_back(std::numeric_limits<uint32_t>::min());
    vec.emplace_back(std::numeric_limits<uint32_t>::max());

    // vector
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.add_len_repeated_packed_unsigned_vint(vec);
    printf("add_len_repeated_packed_unsigned_vint(vec):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    int count = msgProtobuf.len_repeated_packed_unsigned_vint_size();
    ASSERT_EQ(count, static_cast<int>(vec.size()));
    for (int i = 0; i < count; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_unsigned_vint(i), vec[i]);
    }

    // array
    msgOpt.add_len_repeated_packed_unsigned_vint(array, sizeof(array) / sizeof(array[0]));
    printf("add_len_repeated_packed_unsigned_vint(array):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    int countAll = msgProtobuf.len_repeated_packed_unsigned_vint_size();
    ASSERT_EQ(countAll, static_cast<int>(vec.size() + sizeof(array) / sizeof(array[0])));
    for (int i = 0; i < count; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_unsigned_vint(i), vec[i]);
    }
    for (int i = count; i < countAll; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_unsigned_vint(i), array[i - count]);
    }
}

HWTEST_F(BaseMessageUnittest, SetRepeatedPackedFixed64, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;
    const uint64_t array[] = {0, 1,
                            std::numeric_limits<uint64_t>::min(),
                            std::numeric_limits<uint64_t>::max()};
    std::vector<uint64_t> vec;
    vec.emplace_back(0);
    vec.emplace_back(1);
    vec.emplace_back(std::numeric_limits<uint64_t>::min());
    vec.emplace_back(std::numeric_limits<uint64_t>::max());

    // vector
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.add_len_repeated_packed_fixed(vec);
    printf("add_len_repeated_packed_fixed(vec):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    int count = msgProtobuf.len_repeated_packed_fixed_size();
    ASSERT_EQ(count, static_cast<int>(vec.size()));
    for (int i = 0; i < count; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_fixed(i), vec[i]);
    }

    // array
    msgOpt.add_len_repeated_packed_fixed(array, sizeof(array) / sizeof(array[0]));
    printf("add_len_repeated_packed_fixed(array):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    int countAll = msgProtobuf.len_repeated_packed_fixed_size();
    ASSERT_EQ(countAll, static_cast<int>(vec.size() + sizeof(array) / sizeof(array[0])));
    for (int i = 0; i < count; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_fixed(i), vec[i]);
    }
    for (int i = count; i < countAll; i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_fixed(i), array[i - count]);
    }
}

static const char BYTES_DATA[] = {0x00, 0x01, 0x02, 0xFD, 0xFE, 0xFF};

HWTEST_F(BaseMessageUnittest, SetBytes, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_len_bytes(BYTES_DATA, sizeof(BYTES_DATA));
    printf("set_len_bytes(void*, size_t):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.len_bytes(), std::string(&BYTES_DATA[0], sizeof(BYTES_DATA)));
}

HWTEST_F(BaseMessageUnittest, SetBytesCallback, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    // bytes stram
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_len_bytes(
        [](RandomWriteCtx* ctx) -> int32_t {
            uint8_t* memory = nullptr;
            uint32_t offset = 0;
            if (ctx->getMemory(ctx, sizeof(BYTES_DATA), &memory, &offset)) {
                if (memcpy_s(memory, sizeof(BYTES_DATA), BYTES_DATA, sizeof(BYTES_DATA)) == EOK) {
                    ctx->seek(ctx, offset + sizeof(BYTES_DATA));
                    return sizeof(BYTES_DATA);
                }
            }
            return 0;
        });
    printf("set_len_bytes(CallbackFunc(bytes)):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.len_bytes(), std::string(&BYTES_DATA[0], sizeof(BYTES_DATA)));

    // protobufOpt message
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_len_bytes(
        [](RandomWriteCtx* ctx) -> int32_t {
            ProtoEncoder::SubMessage subMsg(ctx);
            subMsg.set_vint_int32(std::numeric_limits<int32_t>::max());
            subMsg.set_vint_int64(std::numeric_limits<int64_t>::max());
            subMsg.set_vint_uint32(std::numeric_limits<uint32_t>::max());
            subMsg.set_vint_uint64(std::numeric_limits<uint64_t>::max());
            return subMsg.Finish();
        });
    printf("set_len_bytes(CallbackFunc(message)):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    auto& bytes = msgProtobuf.len_bytes();
    SubMessage subMsgProtobuf;
    ASSERT_TRUE(subMsgProtobuf.ParseFromArray(bytes.data(), bytes.size()));
    ASSERT_EQ(subMsgProtobuf.vint_int32(), std::numeric_limits<int32_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_int64(), std::numeric_limits<int64_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_uint32(), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_uint64(), std::numeric_limits<uint64_t>::max());
}

HWTEST_F(BaseMessageUnittest, AddBytes, TestSize.Level1)
{
    const char data[] = {0x00, 0x01, 0x02, 0xFD, 0xFE, 0xFF};
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    // bytes stram
    g_writeCtx.seek(&g_writeCtx, 0);
    RandomWriteCtx* writeCtx = msgOpt.startAdd_len_bytes();
    uint8_t* memory = nullptr;
    uint32_t offset = 0;
    if (writeCtx->getMemory(writeCtx, sizeof(data), &memory, &offset)) {
        if (memcpy_s(memory, sizeof(data), data, sizeof(data)) != EOK) {
            writeCtx->seek(writeCtx, offset + sizeof(data));
        }
    }
    msgOpt.finishAdd_len_bytes(sizeof(data));
    printf("Add_LEN_bytes(Customize(bytes)):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.len_bytes(), std::string(&data[0], sizeof(data)));

    // protobufOpt message
    g_writeCtx.seek(&g_writeCtx, 0);
    writeCtx = msgOpt.startAdd_len_bytes();
    ProtoEncoder::SubMessage subMsg(writeCtx);
    subMsg.set_vint_int32(std::numeric_limits<int32_t>::max());
    subMsg.set_vint_int64(std::numeric_limits<int64_t>::max());
    subMsg.set_vint_uint32(std::numeric_limits<uint32_t>::max());
    subMsg.set_vint_uint64(std::numeric_limits<uint64_t>::max());
    msgOpt.finishAdd_len_bytes(subMsg.Finish());
    printf("Add_LEN_bytes(Customize(message)):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    auto& bytes = msgProtobuf.len_bytes();
    SubMessage subMsgProtobuf;
    ASSERT_TRUE(subMsgProtobuf.ParseFromArray(bytes.data(), bytes.size()));
    ASSERT_EQ(subMsgProtobuf.vint_int32(), std::numeric_limits<int32_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_int64(), std::numeric_limits<int64_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_uint32(), std::numeric_limits<uint32_t>::max());
    ASSERT_EQ(subMsgProtobuf.vint_uint64(), std::numeric_limits<uint64_t>::max());
}

HWTEST_F(BaseMessageUnittest, SetAll, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_vint_int32(1);
    msgOpt.set_vint_int64(1);
    msgOpt.set_vint_uint32(1);
    msgOpt.set_vint_uint64(1);
    msgOpt.set_vint_sint32(1);
    msgOpt.set_vint_sint64(1);
    msgOpt.set_vint_bool(true);
    msgOpt.set_vint_enum(ProtoEncoder::NUM::ONE);
    msgOpt.set_i64_fixed64(1);
    msgOpt.set_i64_sfixed64(1);
    msgOpt.set_i64_double(1);

    const std::string str = "this is test.";
    msgOpt.set_len_string(str);
    msgOpt.set_len_bytes(BYTES_DATA, sizeof(BYTES_DATA));

    ProtoEncoder::SubMessage* msgSub = msgOpt.mutable_len_sub();
    ASSERT_TRUE(msgSub != nullptr);
    msgSub->set_vint_int32(1);
    msgSub->set_vint_int64(1);
    msgSub->set_vint_uint32(1);
    msgSub->set_vint_uint64(1);

    const int32_t arraySigned[] = {0, 1,
                            std::numeric_limits<int32_t>::min(),
                            std::numeric_limits<int32_t>::max()};
    std::vector<int32_t> vecSigned;
    vecSigned.emplace_back(0);
    vecSigned.emplace_back(1);
    vecSigned.emplace_back(std::numeric_limits<int32_t>::min());
    vecSigned.emplace_back(std::numeric_limits<int32_t>::max());
    msgOpt.add_len_repeated_packed_signed_vint(vecSigned);
    msgOpt.add_len_repeated_packed_signed_vint(arraySigned, sizeof(arraySigned) / sizeof(arraySigned[0]));

    const uint32_t arrayUnsigned[] = {0, 1,
                            std::numeric_limits<uint32_t>::min(),
                            std::numeric_limits<uint32_t>::max()};
    std::vector<uint32_t> vecUnsigned;
    vecUnsigned.emplace_back(0);
    vecUnsigned.emplace_back(1);
    vecUnsigned.emplace_back(std::numeric_limits<uint32_t>::min());
    vecUnsigned.emplace_back(std::numeric_limits<uint32_t>::max());
    msgOpt.add_len_repeated_packed_unsigned_vint(vecUnsigned);
    msgOpt.add_len_repeated_packed_unsigned_vint(arrayUnsigned, sizeof(arrayUnsigned) / sizeof(arrayUnsigned[0]));

    const uint64_t arrayFixed[] = {0, 1,
                            std::numeric_limits<uint64_t>::min(),
                            std::numeric_limits<uint64_t>::max()};
    std::vector<uint64_t> vecFixed;
    vecFixed.emplace_back(0);
    vecFixed.emplace_back(1);
    vecFixed.emplace_back(std::numeric_limits<uint64_t>::min());
    vecFixed.emplace_back(std::numeric_limits<uint64_t>::max());
    msgOpt.add_len_repeated_packed_fixed(vecFixed);
    msgOpt.add_len_repeated_packed_fixed(arrayFixed, sizeof(arrayFixed) / sizeof(arrayFixed[0]));

    msgOpt.set_i32_fixed32(1);
    msgOpt.set_i32_sfixed32(1);
    msgOpt.set_i32_float(1);

    msgOpt.set_oneof_fixed64(1);
    msgOpt.set_oneof_string(str);
    msgSub = msgOpt.mutable_oneof_sub();
    ASSERT_TRUE(msgSub != nullptr);
    msgSub->set_vint_int32(1);
    msgSub->set_vint_int64(1);
    msgSub->set_vint_uint32(1);
    msgSub->set_vint_uint64(1);

    ASSERT_EQ((uint32_t)msgOpt.Finish(), g_writePos);
    printf("SetAll size(%" PRIu32 "):\n", g_writePos);
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    // check result by protobuf
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));

    ASSERT_EQ(msgProtobuf.vint_int32(), (int32_t)1);
    ASSERT_EQ(msgProtobuf.vint_int64(), (int64_t)1);
    ASSERT_EQ(msgProtobuf.vint_uint32(), (uint32_t)1);
    ASSERT_EQ(msgProtobuf.vint_uint64(), (uint64_t)1);
    ASSERT_EQ(msgProtobuf.vint_sint32(), (int32_t)1);
    ASSERT_EQ(msgProtobuf.vint_sint64(), (int64_t)1);
    ASSERT_EQ(msgProtobuf.vint_sint32(), (int32_t)1);
    ASSERT_TRUE(msgProtobuf.vint_bool());
    ASSERT_EQ(msgProtobuf.vint_enum(), NUM::ONE);

    ASSERT_EQ(msgProtobuf.i64_fixed64(), (uint64_t)1);
    ASSERT_EQ(msgProtobuf.i64_sfixed64(), (int64_t)1);
    ASSERT_EQ(msgProtobuf.i64_double(), (double)1);

    ASSERT_EQ(msgProtobuf.len_string(), str);
    ASSERT_EQ(msgProtobuf.len_bytes(), std::string(&BYTES_DATA[0], sizeof(BYTES_DATA)));

    ASSERT_TRUE(msgProtobuf.has_len_sub());
    SubMessage subMsgProtobuf = msgProtobuf.len_sub();
    ASSERT_EQ(subMsgProtobuf.vint_int32(), (int32_t)1);
    ASSERT_EQ(subMsgProtobuf.vint_int64(), (int64_t)1);
    ASSERT_EQ(subMsgProtobuf.vint_uint32(), (uint32_t)1);
    ASSERT_EQ(subMsgProtobuf.vint_uint64(), (uint64_t)1);

    int countAll = msgProtobuf.len_repeated_packed_signed_vint_size();
    ASSERT_EQ(countAll, static_cast<int>(vecSigned.size() + sizeof(arraySigned) / sizeof(arraySigned[0])));
    for (size_t i = 0; i < vecSigned.size(); i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_signed_vint(i), vecSigned[i]);
    }
    for (size_t i = vecSigned.size(); i < static_cast<size_t>(countAll); i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_signed_vint(i), arraySigned[i - vecSigned.size()]);
    }

    countAll = msgProtobuf.len_repeated_packed_unsigned_vint_size();
    ASSERT_EQ(countAll, static_cast<int>(vecUnsigned.size() + sizeof(arrayUnsigned) / sizeof(arrayUnsigned[0])));
    for (size_t i = 0; i < vecUnsigned.size(); i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_unsigned_vint(i), vecUnsigned[i]);
    }
    for (size_t i = vecUnsigned.size(); i < static_cast<size_t>(countAll); i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_unsigned_vint(i), arrayUnsigned[i - vecUnsigned.size()]);
    }

    countAll = msgProtobuf.len_repeated_packed_fixed_size();
    ASSERT_EQ(countAll, static_cast<int>(vecFixed.size() + sizeof(arrayFixed) / sizeof(arrayFixed[0])));
    for (size_t i = 0; i < vecFixed.size(); i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_fixed(i), vecFixed[i]);
    }
    for (size_t i = vecFixed.size(); i < static_cast<size_t>(countAll); i++) {
        ASSERT_EQ(msgProtobuf.len_repeated_packed_fixed(i), arrayFixed[i - vecFixed.size()]);
    }

    ASSERT_EQ(msgProtobuf.i32_fixed32(), (uint32_t)1);
    ASSERT_EQ(msgProtobuf.i32_sfixed32(), (int32_t)1);
    ASSERT_EQ(msgProtobuf.i32_float(), (float)1);

    // last one wins
    ASSERT_NE(msgProtobuf.oneof_fixed64(), (uint64_t)1);
    ASSERT_NE(msgProtobuf.oneof_string(), str);
    ASSERT_TRUE(msgProtobuf.has_oneof_sub());
    subMsgProtobuf = msgProtobuf.oneof_sub();
    ASSERT_EQ(subMsgProtobuf.vint_int32(), (int32_t)1);
    ASSERT_EQ(subMsgProtobuf.vint_int64(), (int64_t)1);
    ASSERT_EQ(subMsgProtobuf.vint_uint32(), (uint32_t)1);
    ASSERT_EQ(subMsgProtobuf.vint_uint64(), (uint64_t)1);
}

HWTEST_F(BaseMessageUnittest, SubMessageLen0, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);
    ExampleMessage msgProtobuf;

    // empty
    g_writeCtx.seek(&g_writeCtx, 0);
    ProtoEncoder::SubMessage* subMsgOpt = msgOpt.mutable_len_sub();
    ASSERT_TRUE(subMsgOpt != nullptr);
    msgOpt.set_vint_int32(std::numeric_limits<int32_t>::min());
    printf("sub message length is 0:\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_TRUE(msgProtobuf.has_len_sub());

    g_writeCtx.seek(&g_writeCtx, 0);
    int subMsgOptRepeatedCount = 0;
    ProtoEncoder::SubMessage* subMsgOptRepeated = msgOpt.add_repeated_len_sub();
    ASSERT_TRUE(subMsgOptRepeated != nullptr);
    subMsgOptRepeatedCount++;
    msgOpt.set_vint_int32(std::numeric_limits<int32_t>::min());
    printf("repeated sub message length is 0:\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.repeated_len_sub_size(), subMsgOptRepeatedCount);

    subMsgOptRepeated = msgOpt.add_repeated_len_sub();
    ASSERT_TRUE(subMsgOptRepeated != nullptr);
    subMsgOptRepeatedCount++;
    subMsgOptRepeated->set_vint_int32(1);
    msgOpt.set_vint_int32(std::numeric_limits<int32_t>::min());
    printf("repeated sub message length > 0:\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.repeated_len_sub_size(), subMsgOptRepeatedCount);

    // set bytes by callback
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.set_len_bytes(
        [](RandomWriteCtx* randomWriteCtx) -> int32_t {
        return 0;
        });
    printf("set_len_bytes(CallbackFunc(len = 0)):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.len_bytes(), std::string(""));

    // add bytes
    g_writeCtx.seek(&g_writeCtx, 0);
    msgOpt.startAdd_len_bytes();
    msgOpt.finishAdd_len_bytes(0);
    printf("Add_LEN_bytes(len = 0):\n");
    for (uint32_t i = 0; i < g_writePos; i++) {
        printf("%02X ", g_buf[i]);
    }
    printf("\n");

    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.len_bytes(), std::string(""));
}

HWTEST_F(BaseMessageUnittest, AutoFinishSubMessage, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);

    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    msgOpt.set_i32_fixed32(1);
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    msgOpt.set_i64_fixed64(1);
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    msgOpt.set_vint_uint32(1);
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    msgOpt.set_vint_sint32(1);
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    msgOpt.set_len_string("\n");
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    ASSERT_TRUE(msgOpt.mutable_len_sub() != nullptr);
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    ASSERT_TRUE(msgOpt.startAdd_len_bytes() != nullptr);
    msgOpt.finishAdd_len_bytes(0);
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    msgOpt.set_len_bytes(
        [](RandomWriteCtx* randomWriteCtx) -> int32_t {
        return 0;
        });
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    std::vector<int32_t> arraySint32(1, -1);
    msgOpt.add_len_repeated_packed_signed_vint(arraySint32);
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    std::vector<uint32_t> arrayUint32(1, 1);
    msgOpt.add_len_repeated_packed_unsigned_vint(arrayUint32);
    ASSERT_TRUE(msgOpt.add_repeated_example() != nullptr);
    std::vector<uint64_t> arrayfint64(1, 1);
    msgOpt.add_len_repeated_packed_fixed(arrayfint64);

    ExampleMessage msgProtobuf;
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_GT(msgProtobuf.repeated_example_size(), 0);
}

HWTEST_F(BaseMessageUnittest, WriteFailedDrop, TestSize.Level1)
{
    RandomWriteCtx writeCtx = {
        [](RandomWriteCtx*, uint32_t, uint8_t**, uint32_t*) -> bool { return false; },
        [](RandomWriteCtx*, uint32_t) -> bool { return false; }
        };

    std::unique_ptr<ProtoEncoder::ExampleMessage> msgOpt =
        std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    msgOpt->set_i32_fixed32(1);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    msgOpt->set_i64_fixed64(1);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    msgOpt->set_vint_uint32(1);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    msgOpt->set_vint_sint32(1);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    msgOpt->set_len_string("\n");
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    ASSERT_TRUE(msgOpt->mutable_len_sub() != nullptr);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    ASSERT_TRUE(msgOpt->startAdd_len_bytes() == nullptr);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    msgOpt->set_len_bytes(
        [](RandomWriteCtx* randomWriteCtx) -> int32_t {
        return 0;
        });
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    std::vector<int32_t> arraySint32(1, -1);
    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    msgOpt->add_len_repeated_packed_signed_vint(arraySint32);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    std::vector<uint32_t> arrayUint32(1, 1);
    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    msgOpt->add_len_repeated_packed_unsigned_vint(arrayUint32);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);

    msgOpt = std::make_unique<ProtoEncoder::ExampleMessage>(&writeCtx);
    std::vector<uint64_t> arrayfint64(1, 1);
    msgOpt->add_len_repeated_packed_fixed(arrayfint64);
    ASSERT_EQ(msgOpt->Finish(), (int32_t)-1);
}

HWTEST_F(BaseMessageUnittest, NoWrite, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(nullptr);

    msgOpt.set_i32_fixed32(1);
    msgOpt.set_i64_fixed64(1);
    msgOpt.set_vint_uint32(1);
    msgOpt.set_vint_sint32(1);
    msgOpt.set_len_string("\n");
    ASSERT_TRUE(msgOpt.startAdd_len_bytes() == nullptr);
    msgOpt.set_len_bytes(nullptr);
    msgOpt.set_len_bytes(
        [](RandomWriteCtx* randomWriteCtx) -> int32_t {
        return 0;
        });

    // arrayCount is 0
    msgOpt.add_len_repeated_packed_signed_vint(nullptr, 0);
    msgOpt.add_len_repeated_packed_unsigned_vint(nullptr, 0);
    msgOpt.add_len_repeated_packed_fixed(nullptr, 0);
    std::vector<int32_t> arraySint32(1, -1);
    msgOpt.add_len_repeated_packed_signed_vint(arraySint32);
    std::vector<uint32_t> arrayUint32(1, 1);
    msgOpt.add_len_repeated_packed_unsigned_vint(arrayUint32);
    std::vector<uint64_t> arrayfint64(1, 1);
    msgOpt.add_len_repeated_packed_fixed(arrayfint64);
    ASSERT_EQ(msgOpt.Finish(), (int32_t)-1);
}

HWTEST_F(BaseMessageUnittest, SubMessageAlwaysAvailabe, TestSize.Level1)
{
    ProtoEncoder::ExampleMessage msgOpt(nullptr);

    ProtoEncoder::SubMessage* subMsgOpt = msgOpt.mutable_len_sub();
    ASSERT_TRUE(subMsgOpt != nullptr);
    subMsgOpt->set_vint_int32(1);
    subMsgOpt->set_vint_int64(1);
    subMsgOpt->set_vint_uint32(1);
    subMsgOpt->set_vint_uint64(1);
    ASSERT_EQ(subMsgOpt->Finish(), (int32_t)-1);

    ProtoEncoder::ExampleMessage* nested1 = msgOpt.add_repeated_example();
    ASSERT_TRUE(nested1 != nullptr);
    ProtoEncoder::ExampleMessage* nested2 = nested1->add_repeated_example();
    ASSERT_TRUE(nested2 != nullptr);
    ProtoEncoder::ExampleMessage* nested3 = nested2->add_repeated_example();
    ASSERT_TRUE(nested3 != nullptr);

    ASSERT_EQ(nested3->Finish(), (int32_t)-1);
    ASSERT_EQ(nested2->Finish(), (int32_t)-1);
    ASSERT_EQ(nested1->Finish(), (int32_t)-1);

    msgOpt.set_vint_int32(1);
    ASSERT_EQ(msgOpt.Finish(), (int32_t)-1);
}

HWTEST_F(BaseMessageUnittest, MessagePool, TestSize.Level1)
{
    ProtoEncoder::MessagePool  msgStack;
    ProtoEncoder::BaseMessage* pMsg = msgStack.Get();
    ASSERT_TRUE(pMsg != nullptr);

    msgStack.Reset(0);
    pMsg = msgStack.Get();
    ASSERT_TRUE(pMsg != nullptr);

    const uint32_t testRepeat = 1000;
    const uint32_t testDepth = 5;
    msgStack.Reset(testDepth);
    for (uint32_t i = 0; i < testRepeat; i++) {
        pMsg = msgStack.Get();
        ASSERT_TRUE(pMsg != nullptr);
        msgStack.Release();
    }

    for (uint32_t i = 0; i < testRepeat; i++) {
        for (uint32_t j = 0; j < testDepth; j++) {
            pMsg = msgStack.Get();
            ASSERT_TRUE(pMsg != nullptr);
        }
        for (uint32_t j = 0; j < testDepth; j++) {
            msgStack.Release();
        }
    }

    for (uint32_t i = 0; i < testRepeat; i++) {
        pMsg = msgStack.Get();
        ASSERT_TRUE(pMsg != nullptr);
    }
    for (uint32_t i = 0; i < testRepeat; i++) {
        msgStack.Release();
    }

    msgStack.Reset(1);
    for (uint32_t i = 0; i < testDepth; i++) {
        pMsg = msgStack.Get();
        ASSERT_TRUE(pMsg != nullptr);
    }
    ProtoEncoder::ExampleMessage* pExampleMsg = static_cast<ProtoEncoder::ExampleMessage*>(pMsg);
    g_writeCtx.seek(&g_writeCtx, 0);
    pExampleMsg->Reset(&g_writeCtx);
    pExampleMsg->set_vint_int32(1);
    ExampleMessage msgProtobuf;
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_EQ(msgProtobuf.vint_int32(), (int32_t)1);

    msgStack.Reset(1);
    ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx, &msgStack);
    g_writeCtx.seek(&g_writeCtx, 0);
    ProtoEncoder::SubMessage* subMsgOpt = msgOpt.mutable_len_sub();
    ASSERT_TRUE(subMsgOpt != nullptr);
    msgOpt.set_vint_int32(1);
    ASSERT_TRUE(msgProtobuf.ParseFromArray(g_buf, g_writePos));
    ASSERT_TRUE(msgProtobuf.has_len_sub());
    ASSERT_EQ(msgProtobuf.vint_int32(), (int32_t)1);
}

// reference from https://perfetto.dev/docs/design-docs/protozero
// For the full code of the benchmark see /src/protozero/test/protozero_benchmark.cc
const int TEST_TIMES = 1000 * 100;
uint64_t g_fakeInputSimple[] = {0x12345678,
                                0x90ABCDEF,
                                0x11111111,
                                0xFFFFFFFF,
                                0x6666666666666666ULL,
                                0x6666666666666666ULL,
                                0x6666666666666666ULL,
                                0x0066666666666666ULL};

HWTEST_F(BaseMessageUnittest, ProtobufferSimple, TestSize.Level1)
{
    for (int count = 0; count < TEST_TIMES; count++) {
        int index = 0;
        ExampleMessage msgProtobuf;

        msgProtobuf.set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        msgProtobuf.set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        msgProtobuf.set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        msgProtobuf.set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        msgProtobuf.set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));

        ASSERT_TRUE(msgProtobuf.SerializeToArray(&g_buf[0], SIZE_BUFFER) > 0);
    }
    printf("%d times\n", TEST_TIMES);
}

HWTEST_F(BaseMessageUnittest, ProtoEncoderSimple, TestSize.Level1)
{
    for (int count = 0; count < TEST_TIMES; count++) {
        int index = 0;
        g_writeCtx.seek(&g_writeCtx, 0);

        ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);

        msgOpt.set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        msgOpt.set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        msgOpt.set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        msgOpt.set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        msgOpt.set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));

        ASSERT_TRUE(msgOpt.Finish() > 0);
    }
    printf("%d times\n", TEST_TIMES);
}

HWTEST_F(BaseMessageUnittest, ProtobufferNested, TestSize.Level1)
{
    for (int count = 0; count < TEST_TIMES; count++) {
        int index = 0;
        ExampleMessage msgProtobuf;

        msgProtobuf.set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        msgProtobuf.set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        msgProtobuf.set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        msgProtobuf.set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        msgProtobuf.set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));
        // fisrt nested
        ExampleMessage* nested1 = msgProtobuf.add_repeated_example();
        ASSERT_TRUE(nested1 != nullptr);
        index = 0;
        nested1->set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        nested1->set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        nested1->set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        nested1->set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        nested1->set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));
        // second nested
        ExampleMessage* nested2 = nested1->add_repeated_example();
        ASSERT_TRUE(nested2 != nullptr);
        index = 0;
        nested2->set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        nested2->set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        nested2->set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        nested2->set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        nested2->set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));
        // third nested
        ExampleMessage* nested3 = nested2->add_repeated_example();
        ASSERT_TRUE(nested3 != nullptr);
        index = 0;
        nested3->set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        nested3->set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        nested3->set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        nested3->set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        nested3->set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));

        ASSERT_TRUE(msgProtobuf.SerializeToArray(&g_buf[0], SIZE_BUFFER) > 0);
    }
    printf("%d times\n", TEST_TIMES);
}

HWTEST_F(BaseMessageUnittest, ProtoEncoderNested, TestSize.Level1)
{
    for (int count = 0; count < TEST_TIMES; count++) {
        int index = 0;
        g_writeCtx.seek(&g_writeCtx, 0);

        ProtoEncoder::ExampleMessage msgOpt(&g_writeCtx);

        msgOpt.set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        msgOpt.set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        msgOpt.set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        msgOpt.set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        msgOpt.set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));
        // fisrt nested
        ProtoEncoder::ExampleMessage* nested1 = msgOpt.add_repeated_example();
        ASSERT_TRUE(nested1 != nullptr);
        index = 0;
        nested1->set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        nested1->set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        nested1->set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        nested1->set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        nested1->set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));
        // second nested
        ProtoEncoder::ExampleMessage* nested2 = nested1->add_repeated_example();
        ASSERT_TRUE(nested2 != nullptr);
        index = 0;
        nested2->set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        nested2->set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        nested2->set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        nested2->set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        nested2->set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));
        // third nested
        ProtoEncoder::ExampleMessage* nested3 = nested2->add_repeated_example();
        ASSERT_TRUE(nested3 != nullptr);
        index = 0;
        nested3->set_vint_int32(static_cast<int32_t>(g_fakeInputSimple[index++]));
        nested3->set_vint_uint32(static_cast<uint32_t>(g_fakeInputSimple[index++]));
        nested3->set_vint_int64(static_cast<int64_t>(g_fakeInputSimple[index++]));
        nested3->set_vint_uint64(static_cast<uint64_t>(g_fakeInputSimple[index++]));
        nested3->set_len_string(reinterpret_cast<const char*>(&g_fakeInputSimple[index++]));

        ASSERT_TRUE(msgOpt.Finish() > 0);
    }
    printf("%d times\n", TEST_TIMES);
}
} // namespace Profiler
} // namespace Developtools
} // namespace OHOS
