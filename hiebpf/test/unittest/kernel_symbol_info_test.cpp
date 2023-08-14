/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
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
#include <cstring>

#include <gtest/gtest.h>

#include "kernel_symbol_info.h"

using namespace testing::ext;

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class KernelSymbolInfoTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: GetSymbolData
 * @tc.desc: Test framework
 * @tc.type: FUNC
 */
HWTEST_F(KernelSymbolInfoTest, GetSymbolData, TestSize.Level1)
{
    std::vector<uint8_t> buf;
    uint32_t headSize = sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t);
    uint32_t size = KernelSymbolInfo::GetSymbolData(buf);
    ASSERT_GE(size, headSize);
    uint8_t *p = buf.data();
    uint64_t vaddrStart =  *(reinterpret_cast<uint64_t *>(p));
    p += sizeof(uint64_t);
    uint64_t vaddrEnd =  *(reinterpret_cast<uint64_t *>(p));
    p += sizeof(uint64_t);
    uint32_t symTabLen =  *(reinterpret_cast<uint32_t *>(p));
    p += sizeof(uint32_t);
    uint32_t strTabLen =  *(reinterpret_cast<uint32_t *>(p));
    p += sizeof(uint32_t);
    ASSERT_GT(vaddrStart, 0);
    ASSERT_GE(vaddrEnd, vaddrStart);
    vaddrStart--;
    ASSERT_EQ(size, headSize + symTabLen + strTabLen);

    char *pStrTab = reinterpret_cast<char *>(p) + symTabLen;
    const uint32_t symItemSize = sizeof(uint64_t) + sizeof(uint32_t) + sizeof(uint32_t);
    while (symTabLen > symItemSize) {
        uint64_t value =  *(reinterpret_cast<uint64_t *>(p));
        p += sizeof(uint64_t);
        ASSERT_GT(value, vaddrStart);
        vaddrStart = value;

        uint32_t symbolSize =  *(reinterpret_cast<uint32_t *>(p));
        p += sizeof(uint32_t);
        ASSERT_GE(vaddrEnd, value + symbolSize);

        uint32_t symbolNameOffset =  *(reinterpret_cast<uint32_t *>(p));
        p += sizeof(uint32_t);
        std::string name(pStrTab + symbolNameOffset);
        ASSERT_GE(strTabLen, symbolNameOffset + name.size() + 1);

        symTabLen -= symItemSize;
    }
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
