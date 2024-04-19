/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <exception>
#include <iostream>
#include <string>
#include <gtest/gtest.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <functional>
#include "sp_utils.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class SPdaemonUtilsTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: SPUtils::IntegerValueVerification
 * @tc.desc: Test IntegerValueVerification
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonUtilsTest, IntegerValueVerificationTest001, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    std::set<std::string> keys;

    keys.insert("N");
    keys.insert("fl");
    keys.insert("ftl");

    mapInfo["N"] = "";
    mapInfo["fl"] = "";
    mapInfo["ftl"] = "";

    bool ret = SPUtils::IntegerValueVerification(keys, mapInfo, errorInfo);
    EXPECT_EQ(ret, false);
}

HWTEST_F(SPdaemonUtilsTest, IntegerValueVerificationTest002, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    std::set<std::string> keys;

    keys.insert("N");
    keys.insert("fl");
    keys.insert("ftl");

    mapInfo["N"] = "A";
    mapInfo["fl"] = "B";
    mapInfo["ftl"] = "C";

    bool ret = SPUtils::IntegerValueVerification(keys, mapInfo, errorInfo);
    EXPECT_EQ(ret, false);
}

HWTEST_F(SPdaemonUtilsTest, IntegerValueVerificationTest003, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    std::set<std::string> keys;

    keys.insert("N");
    keys.insert("fl");
    keys.insert("ftl");

    mapInfo["N"] = "1";
    mapInfo["fl"] = "2";
    mapInfo["ftl"] = "3";

    bool ret = SPUtils::IntegerValueVerification(keys, mapInfo, errorInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SPUtils::VerifyValueStr
 * @tc.desc: Test VerifyValueStr
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonUtilsTest, VerifyValueStrTest001, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    mapInfo["VIEW"] = "";
    bool ret = SPUtils::VerifyValueStr(mapInfo, errorInfo);
    EXPECT_EQ(ret, false);
}

HWTEST_F(SPdaemonUtilsTest, VerifyValueStrTest002, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    mapInfo["VIEW"] = "TestVIEW";
    mapInfo["PKG"] = "";
    bool ret = SPUtils::VerifyValueStr(mapInfo, errorInfo);
    EXPECT_EQ(ret, false);
}

HWTEST_F(SPdaemonUtilsTest, VerifyValueStrTest003, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    mapInfo["VIEW"] = "TestVIEW";
    mapInfo["PKG"] = "TestPKG";
    mapInfo["OUT"] = "";
    bool ret = SPUtils::VerifyValueStr(mapInfo, errorInfo);
    EXPECT_EQ(ret, false);
}

HWTEST_F(SPdaemonUtilsTest, VerifyValueStrTest004, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    mapInfo["VIEW"] = "TestVIEW";
    mapInfo["PKG"] = "TestPKG";
    mapInfo["OUT"] = "Test/sp_utils_test/";
    bool ret = SPUtils::VerifyValueStr(mapInfo, errorInfo);
    EXPECT_EQ(ret, false);
}

HWTEST_F(SPdaemonUtilsTest, VerifyValueStrTest005, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    mapInfo["VIEW"] = "TestVIEW";
    mapInfo["PKG"] = "TestPKG";
    mapInfo["OUT"] = "/sp_utils_test";
    bool ret = SPUtils::VerifyValueStr(mapInfo, errorInfo);
    EXPECT_EQ(ret, false);
}

HWTEST_F(SPdaemonUtilsTest, VerifyValueStrTest006, TestSize.Level1)
{
    std::string errorInfo;
    std::map<std::string, std::string> mapInfo;
    mapInfo["VIEW"] = "TestVIEW";
    mapInfo["PKG"] = "TestPKG";
    bool ret = SPUtils::VerifyValueStr(mapInfo, errorInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SPUtils::VeriyKey
 * @tc.desc: Test VeriyKey
 * @tc.type: FUNC
 */
HWTEST_F(SPdaemonUtilsTest, VeriyKey001, TestSize.Level1)
{
    std::set<std::string> keys;
    std::map<std::string, std::string> mapInfo;
    std::string errorInfo;

    keys.insert("apple");
    keys.insert("banana");
    keys.insert("cherry");
    keys.insert("orange");
    keys.insert("pineapple");

    mapInfo["A"] = "";
    mapInfo["B"] = "";
    mapInfo["C"] = "";

    bool ret = SPUtils::VeriyKey(keys, mapInfo, errorInfo);
    EXPECT_EQ(ret, false);
}

HWTEST_F(SPdaemonUtilsTest, VeriyKey002, TestSize.Level1)
{
    std::set<std::string> keys;
    std::map<std::string, std::string> mapInfo;
    std::string errorInfo;

    keys.insert("apple");
    keys.insert("banana");
    keys.insert("cherry");
    keys.insert("orange");
    keys.insert("pineapple");

    mapInfo["apple"] = "";
    mapInfo["cherry"] = "";
    mapInfo["pineapple"] = "";

    bool ret = SPUtils::VeriyKey(keys, mapInfo, errorInfo);
    EXPECT_EQ(ret, true);
}

}
}
