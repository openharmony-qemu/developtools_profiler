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

#include "elf_symbol_info.h"

using namespace testing::ext;

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class ElfSymbolInfoTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}

    const std::string TEST_FILE_64 = "/data/test/resources/testdata/elf64.stripped";
    const std::string TEST_FILE_32 = "/data/test/resources/testdata/elf32.unstripped";
};

/**
 * @tc.name: GetSymbolTable
 * @tc.desc: Test framework
 * @tc.type: FUNC
 */
HWTEST_F(ElfSymbolInfoTest, GetSymbolTable32Unstripped, TestSize.Level1)
{
    ElfSymbolInfo elfSymbolInfo;
    ElfSymbolInfo::ElfSymbolTable elfSymbolTable;
    ASSERT_TRUE(elfSymbolInfo.GetSymbolTable(TEST_FILE_32, elfSymbolTable));
    ASSERT_GT(elfSymbolTable.textOffset_, 0);
    ASSERT_GT(elfSymbolTable.symTable_.size(), 0);
    ASSERT_GT(elfSymbolTable.strTable_.size(), 0);

    const uint32_t symEntSize32 = 16;
    ASSERT_EQ(elfSymbolTable.symEntSize_, symEntSize32);

    ASSERT_EQ(elfSymbolTable.fileName_, TEST_FILE_32);
}

HWTEST_F(ElfSymbolInfoTest, GetSymbolTable64Stripped, TestSize.Level1)
{
    ElfSymbolInfo elfSymbolInfo;
    ElfSymbolInfo::ElfSymbolTable elfSymbolTable;
    ASSERT_TRUE(elfSymbolInfo.GetSymbolTable(TEST_FILE_64, elfSymbolTable));
    ASSERT_GT(elfSymbolTable.textOffset_, 0);
    ASSERT_GT(elfSymbolTable.symTable_.size(), 0);
    ASSERT_GT(elfSymbolTable.strTable_.size(), 0);

    const uint32_t symEntSize64 = 24;
    ASSERT_EQ(elfSymbolTable.symEntSize_, symEntSize64);

    ASSERT_EQ(elfSymbolTable.fileName_, TEST_FILE_64);
}

HWTEST_F(ElfSymbolInfoTest, Cache, TestSize.Level1)
{
    ElfSymbolInfo elfSymbolInfo;
    ASSERT_FALSE(elfSymbolInfo.IsCached(TEST_FILE_64));
    elfSymbolInfo.CacheFileName(TEST_FILE_64);
    ASSERT_TRUE(elfSymbolInfo.IsCached(TEST_FILE_64));
}

HWTEST_F(ElfSymbolInfoTest, GetBinary, TestSize.Level1)
{
    ElfSymbolInfo elfSymbolInfo;
    ElfSymbolInfo::ElfSymbolTable elfSymbolTable;
    ASSERT_TRUE(elfSymbolInfo.GetSymbolTable(TEST_FILE_64, elfSymbolTable));
    std::vector<uint8_t> buf;
    uint32_t size = elfSymbolInfo.GetBinary(elfSymbolTable, buf);
    const uint32_t binLen = sizeof(elfSymbolTable.textVaddr_) +
                            sizeof(elfSymbolTable.textOffset_) +
                            sizeof(uint32_t) + sizeof(uint32_t) + // strTabLen + symTabLen
                            sizeof(uint32_t) + // fileNameLen
                            sizeof(elfSymbolTable.symEntSize_) +
                            elfSymbolTable.strTable_.size() +
                            elfSymbolTable.symTable_.size() +
                            elfSymbolTable.fileName_.size() + 1;
    ASSERT_EQ(size, binLen);

    uint8_t *p = buf.data() + binLen - elfSymbolTable.fileName_.size() - 1;
    std::string fileName = (char *)p;
    ASSERT_EQ(fileName, TEST_FILE_64);
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
