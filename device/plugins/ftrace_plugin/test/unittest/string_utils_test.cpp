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
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <random>
#include <string>
#include "string_utils.h"

namespace {
using testing::ext::TestSize;
constexpr int ROUNDS = 20;
constexpr int STR_MAX_SIZE = 10;
constexpr char RANDOM_CHAR_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr long RANDOM_CHAR_NUMBER = std::size(RANDOM_CHAR_TABLE) - 1;

class StringUtilsTest : public ::testing::Test {
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

    std::string RandomString(int len)
    {
        std::string str;
        str.reserve(len);
        for (int i = 0; i < len; i++) {
            str.push_back(RandomChar());
        }
        return str;
    }
};

/*
 * @tc.name: EndsWithNormal
 * @tc.desc: test StringUtils::EndsWith with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, EndsWithNormal, TestSize.Level1)
{
    std::string str = "abbcccdefgh.txt";
    std::string ext = ".txt";
    EXPECT_TRUE(StringUtils::EndsWith(str, ext));
    EXPECT_TRUE(StringUtils::EndsWith(str, str));
    EXPECT_TRUE(StringUtils::EndsWith(ext, ext));
}

/*
 * @tc.name: EndsWithFalse
 * @tc.desc: test StringUtils::EndsWith with false case.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, EndsWithFalse, TestSize.Level1)
{
    std::string str = "abbcccdefgh.txt";
    std::string ext = "txt";
    EXPECT_FALSE(StringUtils::EndsWith(str, +"."));
    EXPECT_FALSE(StringUtils::EndsWith(ext, str));
}

/*
 * @tc.name: StartsWithNormal
 * @tc.desc: test StringUtils::StartsWith with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, StartsWithNormal, TestSize.Level1)
{
    std::string str = "abbcccdefgh.txt";
    std::string pre = "abb";
    EXPECT_TRUE(StringUtils::StartsWith(str, pre));
    EXPECT_TRUE(StringUtils::StartsWith(str, "a"));
    EXPECT_TRUE(StringUtils::StartsWith(str, "ab"));
}

/*
 * @tc.name: StartsWithFalse
 * @tc.desc: test StringUtils::StartsWith with false case.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, StartsWithFalse, TestSize.Level1)
{
    std::string str = "abbcccdefgh.txt";
    std::string pre = "abb";
    EXPECT_FALSE(StringUtils::StartsWith(str, "b"));
    EXPECT_FALSE(StringUtils::StartsWith(str, "bb"));
    EXPECT_FALSE(StringUtils::StartsWith(str, "bbc"));
}

/*
 * @tc.name: StartsWithRandomized
 * @tc.desc: test StringUtils::StartsWith with random input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, StartsWithRandomize, TestSize.Level1)
{
    for (int i = 0; i < ROUNDS; i++) {
        std::string str = RandomString(STR_MAX_SIZE);
        std::string part = str.substr(0, RandomInt(1, static_cast<int>(str.size())));
        EXPECT_TRUE(StringUtils::StartsWith(str, part));

        std::string target = RandomString(RandomInt(1, STR_MAX_SIZE));
        bool isPrefix = (str.substr(0, target.size()) == target);
        EXPECT_EQ(StringUtils::StartsWith(str, target), isPrefix);
    }
}

/*
 * @tc.name: ContainsNormal
 * @tc.desc: test StringUtils::Contains with normal input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, ContainsNormal, TestSize.Level1)
{
    std::string str = "abbcccdefgh.txt";
    std::string target = "de";
    EXPECT_TRUE(StringUtils::Contains(str, target));
    EXPECT_TRUE(StringUtils::Contains(str, "a"));
    EXPECT_TRUE(StringUtils::Contains(str, "b"));
    EXPECT_TRUE(StringUtils::Contains(str, "c"));
    EXPECT_TRUE(StringUtils::Contains(str, "t"));
    EXPECT_TRUE(StringUtils::Contains(str, "txt"));
}

/*
 * @tc.name: ContainsNormal
 * @tc.desc: test StringUtils::Contains with false case.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, ContainsFalse, TestSize.Level1)
{
    std::string str = "abbcccdefgh.txt";
    std::string target = "de";
    EXPECT_FALSE(StringUtils::Contains(str, target + "."));
    EXPECT_FALSE(StringUtils::Contains(str, "a."));
    EXPECT_FALSE(StringUtils::Contains(str, "b."));
    EXPECT_FALSE(StringUtils::Contains(str, "c."));
    EXPECT_FALSE(StringUtils::Contains(str, "t."));
    EXPECT_FALSE(StringUtils::Contains(str, "txt."));
}

/*
 * @tc.name: ContainsRandomize
 * @tc.desc: test StringUtils::Contains with random input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, ContainsRandomize, TestSize.Level1)
{
    for (int i = 0; i < ROUNDS; i++) {
        std::string str = RandomString(STR_MAX_SIZE);
        std::string part = str.substr(RandomInt(1, static_cast<int>(str.size())));
        EXPECT_TRUE(StringUtils::Contains(str, part));

        std::string target = RandomString(RandomInt(1, static_cast<int>(str.size())));
        bool isContains = (str.find(target) != std::string::npos);
        EXPECT_EQ(StringUtils::Contains(str, target), isContains);
    }
}

/*
 * @tc.name: StripNormal
 * @tc.desc: test StringUtils::Strip with normal input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, StripNormal, TestSize.Level1)
{
    std::string str = "abbcccdefgh.txt";
    EXPECT_EQ(StringUtils::Strip(str + " "), str);
    EXPECT_EQ(StringUtils::Strip(" " + str), str);
    EXPECT_EQ(StringUtils::Strip(str + "\t"), str);
    EXPECT_EQ(StringUtils::Strip("\t" + str), str);
    EXPECT_EQ(StringUtils::Strip(str + "\n"), str);
    EXPECT_EQ(StringUtils::Strip("\n" + str), str);
}

/*
 * @tc.name: StripRandomize
 * @tc.desc: test StringUtils::Strip with random input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, StripRandomize, TestSize.Level1)
{
    for (int i = 0; i < ROUNDS; i++) {
        std::string str = RandomString(STR_MAX_SIZE);
        std::string original = str;
        int leftPadding = RandomInt(0, static_cast<int>(str.size()));
        if (leftPadding > 0) {
            str = std::string(leftPadding, ' ') + str;
        }

        int rightPadding = RandomInt(0, static_cast<int>(str.size()));
        if (rightPadding > 0) {
            str += std::string(rightPadding, ' ');
        }
        EXPECT_EQ(StringUtils::Strip(str), original);
    }
}

/*
 * @tc.name: JoinNormal
 * @tc.desc: test StringUtils::Join with normal input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, JoinNormal, TestSize.Level1)
{
    std::vector<std::string> parts = {"a", "bb", "ccc", "dddd"};
    std::string joined;
    for (auto& str : parts) {
        joined += str;
        joined += " ";
    }
    joined.pop_back();
    EXPECT_EQ(StringUtils::Join(parts, " "), joined);
}

/*
 * @tc.name: JoinRandomize
 * @tc.desc: test StringUtils::Join with random input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, JoinRandomize, TestSize.Level1)
{
    for (int i = 0; i < ROUNDS; i++) {
        std::vector<std::string> parts;
        for (int j = 0; j < STR_MAX_SIZE; j++) {
            parts.push_back(RandomString(RandomInt(1, STR_MAX_SIZE)));
        }
        std::string joined;
        for (auto& str : parts) {
            joined += str;
            joined += " ";
        }
        joined.pop_back();
        EXPECT_EQ(StringUtils::Join(parts, " "), joined);
    }
}

/*
 * @tc.name: SplitNormal
 * @tc.desc: test StringUtils::Split with normal input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, SplitNormal, TestSize.Level1)
{
    std::vector<std::string> parts = {"a", "bb", "ccc", "dddd"};
    std::string joined;
    for (auto& str : parts) {
        joined += str;
        joined += " ";
    }
    joined.pop_back();
    EXPECT_EQ(StringUtils::Split(joined, " "), parts);
}

/*
 * @tc.name: SplitRandomize
 * @tc.desc: test StringUtils::Split with random input.
 * @tc.type: FUNC
 */
HWTEST_F(StringUtilsTest, SplitRandomize, TestSize.Level1)
{
    for (int i = 0; i < ROUNDS; i++) {
        std::vector<std::string> parts;
        for (int j = 0; j < STR_MAX_SIZE; j++) {
            parts.push_back(RandomString(RandomInt(1, STR_MAX_SIZE)));
        }
        std::string joined;
        for (auto& str : parts) {
            joined += str;
            joined += " ";
        }
        EXPECT_EQ(StringUtils::Split(joined, " "), parts);
        joined.pop_back();
        EXPECT_EQ(StringUtils::Split(joined, " "), parts);
    }
}
} // namespace