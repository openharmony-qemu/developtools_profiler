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
#include <fstream>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <random>
#include <string>

#include "kernel_symbols_parser.h"

using FTRACE_NS::KernelSymbol;
using FTRACE_NS::KernelSymbolsParser;
using namespace testing::ext;
namespace {
constexpr int ROUNDS = 20;
constexpr int STR_MAX_SIZE = 10;
constexpr int NUM_BEGIN_PLACE = 10000000;
constexpr char RANDOM_CHAR_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr long RANDOM_CHAR_NUMBER = std::size(RANDOM_CHAR_TABLE) - 1; // ignore '\0'

class KernelSymbolsParserTest : public ::testing::Test {
    std::mt19937 gen_;

protected:
    void SetUp() override {}

    void TearDown() override {}

    int RandomInt(int a, int b)
    {
        std::uniform_int_distribution<int> distrib(a, b);
        return distrib(gen_);
    }

    char RandomChar()
    {
        return RANDOM_CHAR_TABLE[RandomInt(0, RANDOM_CHAR_NUMBER - 1)];
    }
    int RandomNumber()
    {
        return RandomInt(1, NUM_BEGIN_PLACE);
    }

    std::string RandAddr()
    {
        std::string str = std::to_string(RandomNumber());
        return str;
    }

    std::string RandomName(int len, bool suffix)
    {
        std::string str;
        str.reserve(len + 1);
        for (int i = 0; i < (len + 1); i++) {
            str.push_back(RandomChar());
        }
        if (suffix) {
            str + ".cfi";
        }
        return str;
    }

    char RandomType()
    {
        char str = 't';
        if (RandomInt(0, 1) == 0) {
            str = 'T';
        }
        return str;
    }

    std::string RandomSymbols(std::string randAddr, char randomType, std::string RandomName)
    {
        std::string str = randAddr + " " + randomType + " " + RandomName;
        return str;
    }
};

void visitor(const KernelSymbol& kernelSymbol)
{
    uint64_t addr = kernelSymbol.addr;
    char type = kernelSymbol.type;
    std::string name = kernelSymbol.name;
    if (addr > 0 && type > 0) {
    }
}

/*
 * @tc.name: normal KernelSymbolParser size
 * @tc.desc: test KernelSymbolParser:: normal with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(KernelSymbolsParserTest, KernelSymbolsParserSingle, TestSize.Level1)
{
    std::unique_ptr<KernelSymbolsParser> kernelSymbols = std::make_unique<KernelSymbolsParser>();
    EXPECT_NE(kernelSymbols, nullptr);

    kernelSymbols->Accept(visitor);

    std::string symbols = "1 T name.cfi";
    EXPECT_TRUE(kernelSymbols->Parse(symbols));
}

/*
 * @tc.name: normal KernelSymbolsParser size
 * @tc.desc: test KernelSymbolsParser:: normal with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(KernelSymbolsParserTest, KernelSymbolsParserNormalMulti, TestSize.Level1)
{
    std::unique_ptr<KernelSymbolsParser> kernelSymbols = std::make_unique<KernelSymbolsParser>();
    EXPECT_NE(kernelSymbols, nullptr);

    kernelSymbols->Accept(visitor);

    std::string symbols = "1 T name1.cfi\n2 t name2.cfi\n3 T name3.cfi\n4 t name4.cfi";
    EXPECT_TRUE(kernelSymbols->Parse(symbols));
}

/*
 * @tc.name: normal KernelSymbolsParser size
 * @tc.desc: test KernelSymbolsParser::KernelSymbolsParser with false case.
 * @tc.type: FUNC
 */
HWTEST_F(KernelSymbolsParserTest, KernelSymbolsParserNormalFalse, TestSize.Level1)
{
    std::unique_ptr<KernelSymbolsParser> kernelSymbols = std::make_unique<KernelSymbolsParser>();
    EXPECT_NE(kernelSymbols, nullptr);

    kernelSymbols->Accept(visitor);

    std::string symbols = "0 T name1.cfi\n2 a name2.cfi\n3 T $name3.cfi\n0 t name4.cfi\n4 t .cfi";
    EXPECT_FALSE(kernelSymbols->Parse(symbols));
}

/*
 * @tc.name: normal KernelSymbolsParser
 * @tc.desc: test KernelSymbolsParser::KernelSymbolsParser with other case.
 * @tc.type: FUNC
 */
HWTEST_F(KernelSymbolsParserTest, KernelSymbolsParserNormalOther, TestSize.Level1)
{
    std::unique_ptr<KernelSymbolsParser> kernelSymbols = std::make_unique<KernelSymbolsParser>();
    EXPECT_NE(kernelSymbols, nullptr);

    kernelSymbols->Accept(visitor);

    std::string symbolsName = "1 T nameA\n1 T nameB\n1 T nameC\n1 T nameD\n";
    EXPECT_TRUE(kernelSymbols->Parse(symbolsName));

    std::string symbolsNameType = "2 t name\n2 T name\n2 t name\n2 T name\n";
    EXPECT_TRUE(kernelSymbols->Parse(symbolsNameType));
}

/*
 * @tc.name: rand KernelSymbolsParser size
 * @tc.desc: test KernelSymbolsParser:: normal with rand case.
 * @tc.type: FUNC
 */
HWTEST_F(KernelSymbolsParserTest, KernelSymbolsParserNormalRand, TestSize.Level1)
{
    for (int i = 0; i < ROUNDS; i++) {
        // 组成
        std::string randAddr = RandAddr();
        char randomType = RandomType();
        std::string randomName = RandomName(RandomInt(1, STR_MAX_SIZE), true);
        std::string randomSymbols = RandomSymbols(randAddr, randomType, randomName);

        std::unique_ptr<KernelSymbolsParser> kernelSymbols = std::make_unique<KernelSymbolsParser>();
        EXPECT_NE(kernelSymbols, nullptr);

        kernelSymbols->Accept(visitor);
        EXPECT_TRUE(kernelSymbols->Parse(randomSymbols));
    }
}
} // namespace