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
#include <cinttypes>
#include <fstream>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <random>
#include <string>

#include "logging.h"
#include "printk_formats_parser.h"

using FTRACE_NS::PrintkFormatsParser;
using namespace testing::ext;
namespace {
constexpr int ROUNDS = 20;
constexpr int ADDR_MAX_SIZE = 8;
constexpr int SYMBOL_MAX_SIZE = 16;
constexpr int HEX_BASE = 16;

constexpr char RANDOM_HEX_TABLE[] = "0123456789ABCDEF";
constexpr long RANDOM_HEX_NUMBER = std::size(RANDOM_HEX_TABLE) - 1;
constexpr char RANDOM_CHAR_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
constexpr long RANDOM_CHAR_NUMBER = std::size(RANDOM_CHAR_TABLE) - 1;

constexpr uint64_t CPU_ON_ADDR = 0xc0b01947;
const std::string CPU_ON_STR = "CPU_ON";
const std::string NORMAL_PRINTK_FORMATS = R"(
0xc0aff2a9 : "CPU wakeup interrupts"
0xc0aff2bf : "Timer broadcast interrupts"
0xc0aff2da : "Rescheduling interrupts"
0xc0aff2f2 : "Function call interrupts"
0xc0aff30b : "CPU stop interrupts"
0xc0aff31f : "IRQ work interrupts"
0xc0aff333 : "completion interrupts"
0xc0b018e0 : "CPU_OFF"
0xc0b01947 : "CPU_ON"
0xc0b068a6 : "thaw_processes"
0xc0b068a6 : "thaw_processes"
0xc0b06a3a : "resume_console"
0xc0b06a3a : "resume_console"
0xc0b06b38 : "machine_suspend"
0xc0b06b38 : "machine_suspend"
0xc0b06b38 : "machine_suspend"
0xc0b06b38 : "machine_suspend"
0xc0b06b48 : "suspend_enter"
0xc0b06b56 : "sync_filesystems"
0xc0b06b56 : "sync_filesystems"
0xc0b06b48 : "suspend_enter"
0xc0b06b86 : "freeze_processes"
0xc0b06b86 : "freeze_processes"
0xc0e18840 : "rcu_sched"
0xc0e18b00 : "rcu_bh"
0xc0b08096 : "Start context switch"
0xc0b080ab : "End context switch"
0xc0b08144 : "Start scheduler-tick"
0xc0b08159 : "End scheduler-tick"
0xc0b0817e : "Startleaf"
0xc0b08188 : "Prestarted"
0xc0b08193 : "Startedleaf"
0xc0b0819f : "Startedleafroot"
0xc0b081af : "Startedroot"
0xc0b081bb : "NoGPkthread"
0xc0b6a70d : "Begin"
0xc0b084b7 : "EarlyExit"
0xc0b084c1 : "Inc1"
0xc0b084c6 : "OfflineNoCB"
0xc0b084d2 : "OnlineNoCB"
0xc0b084dd : "OnlineQ"
0xc0b084e5 : "OnlineNQ"
0xc0b084ee : "Inc2"
0xc0b0850c : "LastCB"
0xc0b084cf : "CB"
0xc0b01ebb : "IRQ"
0xc0b08513 : "IRQNQ"
0xc0b0868a : "CleanupMore"
0xc0b0868a : "CleanupMore"
0xc0b08696 : "Cleanup"
0xc0b0879f : "Start RCU core"
0xc0b087ae : "End RCU core"
)";

class PrintkFormatsParserTest : public ::testing::Test {
    std::mt19937 gen_;

protected:
    void SetUp() override
    {
        PrintkFormatsParser::GetInstance().printkFormats_.clear();
    }

    void TearDown() override
    {
        PrintkFormatsParser::GetInstance().printkFormats_.clear();
    }

    int RandomInt(int a, int b)
    {
        std::uniform_int_distribution<int> distrib(a, b);
        return distrib(gen_);
    }

    char RandomChar()
    {
        return RANDOM_CHAR_TABLE[RandomInt(0, RANDOM_CHAR_NUMBER - 1)];
    }

    char RandomHex()
    {
        return RANDOM_HEX_TABLE[RandomInt(0, RANDOM_HEX_NUMBER - 1)];
    }

    std::string RandomAddr(int len)
    {
        std::string str = "0x0";
        str.reserve(str.size() + len);
        for (int i = 0; i < len; i++) {
            str.push_back(RandomHex());
        }
        return str;
    }

    std::string RandomString(int len)
    {
        std::string str;
        str.reserve(len);
        for (int i = 0; i < len; i++) {
            str.push_back(RandomChar());
        }
        return str;
    }

    std::string BuildPrintkLine(std::string randNumber, std::string randomString)
    {
        std::string str = randNumber + " : " + randomString;
        return str;
    }
};

/*
 * @tc.name: false PrintkFormatsParser size
 * @tc.desc: test PrintkFormatsParser::PrintkFormatsParser with false case
 * @tc.type: FUNC
 */
HWTEST_F(PrintkFormatsParserTest, PrintkFormatsParserFalse, TestSize.Level1)
{
    std::string input = "test";
    EXPECT_FALSE(PrintkFormatsParser::GetInstance().Parse(input));

    std::string symbol = PrintkFormatsParser::GetInstance().GetSymbol(0);
    EXPECT_EQ(symbol, "NULL");
}

/*
 * @tc.name: normal PrintkFormatsParser size
 * @tc.desc: test PrintkFormatsParser:: normal with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(PrintkFormatsParserTest, PrintkFormatsParserNormal, TestSize.Level1)
{
    EXPECT_TRUE(PrintkFormatsParser::GetInstance().Parse(NORMAL_PRINTK_FORMATS));

    for (auto& entry : PrintkFormatsParser::GetInstance().printkFormats_) {
        uint64_t addr = entry.first;
        std::string symbol = entry.second;
        HILOG_INFO(LOG_CORE, "%" PRIx64 " : %s", addr, symbol.c_str());
    }
    std::string symbol = PrintkFormatsParser::GetInstance().GetSymbol(CPU_ON_ADDR);
    EXPECT_EQ(symbol, CPU_ON_STR);
}

/*
 * @tc.name: normal PrintkFormatsParser size
 * @tc.desc: test PrintkFormatsParser:: normal with Symbols case.
 * @tc.type: FUNC
 */
HWTEST_F(PrintkFormatsParserTest, PrintkFormatsParserSymbolsNormal, TestSize.Level1)
{
    std::string input = "1 : \"test\"";
    EXPECT_NE(input, "");

    EXPECT_TRUE(PrintkFormatsParser::GetInstance().Parse(input));

    std::string symbol = PrintkFormatsParser::GetInstance().GetSymbol(1);
    EXPECT_EQ(symbol, "test");
}

/*
 * @tc.name: rand PrintkFormatsParser size
 * @tc.desc: test PrintkFormatsParser:: normal with rand case.
 * @tc.type: FUNC
 */
HWTEST_F(PrintkFormatsParserTest, PrintkFormatsParserNormalRand, TestSize.Level1)
{
    for (int i = 0; i < ROUNDS; i++) {
        // 组成
        std::string addr = RandomAddr(RandomInt(1, ADDR_MAX_SIZE));
        std::string symbol = RandomString(RandomInt(1, SYMBOL_MAX_SIZE));
        std::string line = BuildPrintkLine(addr, symbol);
        EXPECT_TRUE(PrintkFormatsParser::GetInstance().Parse(line));

        std::string symbolGot = PrintkFormatsParser::GetInstance().GetSymbol(strtoull(addr.c_str(), nullptr, HEX_BASE));
        EXPECT_EQ(symbolGot, symbol);
    }
}
} // namespace