/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <array>
#include <gtest/gtest.h>

#define private public
#include "dwarf_encoding.h"
#undef private

using namespace testing::ext;

namespace OHOS::Developtools::NativeDaemon {
namespace {
class DwarfEncodingTest : public testing::Test {};

const unsigned char* ResetData(const std::array<unsigned char, sizeof(uint64_t)>& bytes)
{
    return bytes.data();
}

template <typename T>
std::array<unsigned char, sizeof(T)> ToBytes(T value)
{
    std::array<unsigned char, sizeof(T)> data = {};
    EXPECT_EQ(memcpy_s(data.data(), data.size(), &value, sizeof(T)), EOK);
    return data;
}

template <typename T>
void ExpectNullReadReturnsZero()
{
    const unsigned char* data = nullptr;
    EXPECT_EQ(dwReadAnyTypeData(data, T {}), 0u);
    EXPECT_EQ(data, nullptr);
}
}

HWTEST_F(DwarfEncodingTest, ReadValueHandlesUnsignedAndSignedFormats, TestSize.Level0)
{
    auto u16 = ToBytes<uint16_t>(0x1234);
    const unsigned char* data16 = u16.data();
    DwarfEncoding enc16(DW_EH_PE_udata2, data16);
    EXPECT_EQ(enc16.GetValue(), 0x1234u);
    EXPECT_EQ(enc16.GetSize(), sizeof(uint16_t));

    auto s16 = ToBytes<int16_t>(-2);
    const unsigned char* signedData = s16.data();
    DwarfEncoding signedEnc(DW_EH_PE_sdata2, signedData);
    EXPECT_EQ(signedEnc.GetValue(), static_cast<uint64_t>(static_cast<int16_t>(-2)));
}

HWTEST_F(DwarfEncodingTest, ReadValueHandlesWideAndDefaultFormats, TestSize.Level0)
{
    auto u32 = ToBytes<uint32_t>(0x12345678);
    const unsigned char* data32 = u32.data();
    DwarfEncoding enc32(DW_EH_PE_udata4, data32);
    EXPECT_EQ(enc32.GetValue(), 0x12345678u);

    auto s32 = ToBytes<int32_t>(-3);
    data32 = s32.data();
    DwarfEncoding encS32(DW_EH_PE_sdata4, data32);
    EXPECT_EQ(encS32.GetValue(), static_cast<uint64_t>(static_cast<int32_t>(-3)));

    auto s64 = ToBytes<int64_t>(-4);
    const unsigned char* signed64 = s64.data();
    DwarfEncoding encS64(DW_EH_PE_sdata8, signed64);
    EXPECT_EQ(encS64.GetValue(), static_cast<uint64_t>(static_cast<int64_t>(-4)));

    auto u64 = ToBytes<uint64_t>(0x1122334455667788ULL);
    const unsigned char* invalid = u64.data();
    DwarfEncoding enc64(DW_EH_PE_udata8, invalid);
    EXPECT_EQ(enc64.GetValue(), 0x1122334455667788ULL);
    invalid = u64.data();
    DwarfEncoding invalidEnc(DW_EH_PE_uleb128, invalid);
    EXPECT_EQ(invalidEnc.GetValue(), std::numeric_limits<uint64_t>::max());
}

HWTEST_F(DwarfEncodingTest, AppliedValueUsesApplicationBase, TestSize.Level0)
{
    auto bytes = ToBytes<uint32_t>(0x10);
    const unsigned char* data = bytes.data();
    DwarfEncoding dataRel(DW_EH_PE_datarel | DW_EH_PE_udata4, data, 0x1000, 0x2000, 0x3000);
    EXPECT_EQ(dataRel.GetAppliedValue(), 0x1010u);

    data = bytes.data();
    DwarfEncoding textRel(DW_EH_PE_textrel | DW_EH_PE_udata4, data, 0x1000, 0x2000, 0x3000);
    EXPECT_EQ(textRel.GetAppliedValue(), 0x3010u);

    data = bytes.data();
    DwarfEncoding pcRel(DW_EH_PE_pcrel | DW_EH_PE_udata4, data, 0x1000, 0x2000, 0x3000);
    EXPECT_EQ(pcRel.GetAppliedValue(), 0x2010u);

    data = bytes.data();
    DwarfEncoding noRel(DW_EH_PE_udata4, data, 0x1000, 0x2000, 0x3000);
    EXPECT_EQ(noRel.GetAppliedValue(), 0x10u);
}

HWTEST_F(DwarfEncodingTest, NamesAndStringCoverKnownFormats, TestSize.Level0)
{
    auto bytes = ToBytes<uint16_t>(0x22);
    const unsigned char* data = bytes.data();
    DwarfEncoding enc(DW_EH_PE_funcrel | DW_EH_PE_sdata2, data, 1, 2, 3);

    EXPECT_EQ(enc.FormatName(), "DW_EH_PE_data2");
    EXPECT_EQ(enc.ApplicationName(), "DW_EH_PE_funcrel");
    EXPECT_FALSE(enc.IsOmit());
    EXPECT_NE(enc.ToString().find("DW_EH_PE_funcrel"), std::string::npos);
}

HWTEST_F(DwarfEncodingTest, FormatNameCoversAllFormatBranches, TestSize.Level0)
{
    auto bytes = ToBytes<uint64_t>(0x22);
    const unsigned char* data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_absptr, data).FormatName(), "DW_EH_PE_absptr");

    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_uleb128, data).FormatName(), "DW_EH_PE_uleb128");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_udata2, data).FormatName(), "DW_EH_PE_udata2");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_udata4, data).FormatName(), "DW_EH_PE_udata4");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_udata8, data).FormatName(), "DW_EH_PE_udata8");

    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_sleb128, data).FormatName(), "DW_EH_PE_sleb128");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_sdata2, data).FormatName(), "DW_EH_PE_data2");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_sdata4, data).FormatName(), "DW_EH_PE_sdata4");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_sdata8, data).FormatName(), "DW_EH_PE_sdata8");
}

HWTEST_F(DwarfEncodingTest, ApplicationNameCoversReachableBranches, TestSize.Level0)
{
    auto bytes = ToBytes<uint64_t>(0x22);
    const unsigned char* data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_nothing | DW_EH_PE_udata2, data).ApplicationName(), "DW_EH_PE_empty");

    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_pcrel | DW_EH_PE_udata2, data).ApplicationName(), "DW_EH_PE_pcrel");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_textrel | DW_EH_PE_udata2, data).ApplicationName(), "DW_EH_PE_textrel");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_datarel | DW_EH_PE_udata2, data).ApplicationName(), "DW_EH_PE_datarel");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_funcrel | DW_EH_PE_udata2, data).ApplicationName(), "DW_EH_PE_funcrel");
    data = ResetData(bytes);
    EXPECT_EQ(DwarfEncoding(DW_EH_PE_aligned | DW_EH_PE_udata2, data).ApplicationName(), "DW_EH_PE_aligned");

    data = ResetData(bytes);
    DwarfEncoding unknownApp(0x60 | DW_EH_PE_udata2, data);
    EXPECT_EQ(unknownApp.ApplicationName(), "unknow format");
}

HWTEST_F(DwarfEncodingTest, DefaultBranchesReturnFallbackValues, TestSize.Level0)
{
    auto bytes = ToBytes<uint16_t>(0x22);
    const unsigned char* data = bytes.data();
    DwarfEncoding enc(DW_EH_PE_omit, data, 1, 2, 3);

    EXPECT_EQ(enc.GetAppliedValue(), 0u);
    EXPECT_TRUE(enc.IsOmit());
    EXPECT_EQ(enc.FormatName(), "unknow format");
    EXPECT_EQ(enc.ApplicationName(), "unknow format");

    data = bytes.data();
    DwarfEncoding invalidFmt(0x0F, data);
    EXPECT_EQ(invalidFmt.GetValue(), std::numeric_limits<uint64_t>::max());
    EXPECT_EQ(invalidFmt.FormatName(), "unknow format");
    EXPECT_EQ(invalidFmt.ApplicationName(), "DW_EH_PE_empty");
}

HWTEST_F(DwarfEncodingTest, ReadAnyTypeDataRejectsNullBuffer, TestSize.Level0)
{
    ExpectNullReadReturnsZero<uint16_t>();
    ExpectNullReadReturnsZero<uint32_t>();
    ExpectNullReadReturnsZero<uint64_t>();
    ExpectNullReadReturnsZero<int16_t>();
    ExpectNullReadReturnsZero<int32_t>();
    ExpectNullReadReturnsZero<int64_t>();
}
}
