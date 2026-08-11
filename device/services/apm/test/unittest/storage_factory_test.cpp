/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "storage_factory.h"
#include "i_data_storage.h"
#include "zip_storage.h"

using namespace testing::ext;

namespace {
const std::string ZIP_FILE_SUFFIX = ".zip";
}

class StorageFactoryTest : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: CreateWithZipDestination
 * @tc.desc: Test Create returns ZipStorage for .zip destination
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, CreateWithZipDestination, TestSize.Level1)
{
    std::string dest = "/data/local/tmp/storage_factory_test.zip";
    auto storage = StorageFactory::Create(dest);
    EXPECT_NE(storage, nullptr);
}

/**
 * @tc.name: CreateWithEmptyDestination
 * @tc.desc: Test Create returns nullptr for empty destination
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, CreateWithEmptyDestination, TestSize.Level1)
{
    std::string dest = "";
    auto storage = StorageFactory::Create(dest);
    EXPECT_EQ(storage, nullptr);
}

/**
 * @tc.name: CreateWithNonZipDestination
 * @tc.desc: Test Create returns nullptr for non-zip destination
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, CreateWithNonZipDestination, TestSize.Level1)
{
    std::string dest = "/data/local/tmp/output.txt";
    auto storage = StorageFactory::Create(dest);
    EXPECT_EQ(storage, nullptr);
}

/**
 * @tc.name: CreateWithNoSuffix
 * @tc.desc: Test Create returns nullptr for destination with no file suffix
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, CreateWithNoSuffix, TestSize.Level1)
{
    std::string dest = "/data/local/tmp/output";
    auto storage = StorageFactory::Create(dest);
    EXPECT_EQ(storage, nullptr);
}

/**
 * @tc.name: CreateWithZipInMiddle
 * @tc.desc: Test Create returns nullptr when .zip is not at the end
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, CreateWithZipInMiddle, TestSize.Level1)
{
    std::string dest = "/data/local/tmp/output.zip.bak";
    auto storage = StorageFactory::Create(dest);
    EXPECT_EQ(storage, nullptr);
}

/**
 * @tc.name: StringEndsWithTrue
 * @tc.desc: Test StringEndsWith returns true for matching suffix
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, StringEndsWithTrue, TestSize.Level1)
{
    EXPECT_TRUE(StorageFactory::StringEndsWith("test.zip", ZIP_FILE_SUFFIX));
}

/**
 * @tc.name: StringEndsWithFalse
 * @tc.desc: Test StringEndsWith returns false for non-matching suffix
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, StringEndsWithFalse, TestSize.Level1)
{
    EXPECT_FALSE(StorageFactory::StringEndsWith("test.txt", ZIP_FILE_SUFFIX));
}

/**
 * @tc.name: StringEndsWithEmptyString
 * @tc.desc: Test StringEndsWith returns false for empty string
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, StringEndsWithEmptyString, TestSize.Level1)
{
    EXPECT_FALSE(StorageFactory::StringEndsWith("", ZIP_FILE_SUFFIX));
}

/**
 * @tc.name: StringEndsWithEmptySuffix
 * @tc.desc: Test StringEndsWith returns false for empty suffix
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, StringEndsWithEmptySuffix, TestSize.Level1)
{
    EXPECT_FALSE(StorageFactory::StringEndsWith("test.zip", ""));
}

/**
 * @tc.name: StringEndsWithStringShorterThanSuffix
 * @tc.desc: Test StringEndsWith returns false when string is shorter than suffix
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, StringEndsWithStringShorterThanSuffix, TestSize.Level1)
{
    EXPECT_FALSE(StorageFactory::StringEndsWith(".z", ZIP_FILE_SUFFIX));
}

/**
 * @tc.name: StringEndsWithExactMatch
 * @tc.desc: Test StringEndsWith returns true when string equals suffix
 * @tc.type: FUNC
 */
HWTEST_F(StorageFactoryTest, StringEndsWithExactMatch, TestSize.Level1)
{
    EXPECT_TRUE(StorageFactory::StringEndsWith(ZIP_FILE_SUFFIX, ZIP_FILE_SUFFIX));
}
