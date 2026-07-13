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
#include <fstream>
#include <filesystem>
#include <string>

#include "zip_storage.h"

using namespace testing::ext;

namespace {
const std::string TEST_DIR = "/data/local/tmp/";
const std::string TEST_ZIP_FILE = TEST_DIR + "zip_storage_test.zip";
const std::string TEST_INPUT_FILE = TEST_DIR + "zip_storage_test_input.txt";
const std::string TEST_INPUT_FILE_CONTENT = "hello zip storage test";
}

class ZipStorageTest : public testing::Test {
protected:
    static void SetUpTestCase()
    {
        std::ofstream ofs(TEST_INPUT_FILE, std::ios::trunc);
        ofs << TEST_INPUT_FILE_CONTENT;
        ofs.close();
    }
    static void TearDownTestCase()
    {
        std::filesystem::remove(TEST_INPUT_FILE);
        std::filesystem::remove(TEST_ZIP_FILE);
    }
    void SetUp() override {}
    void TearDown() override
    {
        std::filesystem::remove(TEST_ZIP_FILE);
    }
};

/**
 * @tc.name: ConstructorAndDestructor
 * @tc.desc: Test ZipStorage can be constructed and destructed normally
 * @tc.type: FUNC
 */
HWTEST_F(ZipStorageTest, ConstructorAndDestructor, TestSize.Level1)
{
    auto storage = std::make_unique<ZipStorage>(TEST_ZIP_FILE);
    EXPECT_NE(storage, nullptr);
    storage.reset();
    SUCCEED();
}

/**
 * @tc.name: AddFileSuccess
 * @tc.desc: Test AddFile successfully adds a regular file to zip
 * @tc.type: FUNC
 */
HWTEST_F(ZipStorageTest, AddFileSuccess, TestSize.Level1)
{
    ZipStorage storage(TEST_ZIP_FILE);
    bool result = storage.AddFile(TEST_INPUT_FILE);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: AddFileNonExistent
 * @tc.desc: Test AddFile returns false for non-existent file
 * @tc.type: FUNC
 */
HWTEST_F(ZipStorageTest, AddFileNonExistent, TestSize.Level1)
{
    ZipStorage storage(TEST_ZIP_FILE);
    bool result = storage.AddFile("/data/local/tmp/non_existent_file_for_test.txt");
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AddMultipleFiles
 * @tc.desc: Test AddFile can add multiple files to the same zip
 * @tc.type: FUNC
 */
HWTEST_F(ZipStorageTest, AddMultipleFiles, TestSize.Level1)
{
    std::string secondFile = TEST_DIR + "zip_storage_test_input2.txt";
    {
        std::ofstream ofs(secondFile, std::ios::trunc);
        ofs << "second file content";
        ofs.close();
    }
    ZipStorage storage(TEST_ZIP_FILE);
    EXPECT_TRUE(storage.AddFile(TEST_INPUT_FILE));
    EXPECT_TRUE(storage.AddFile(secondFile));
    std::filesystem::remove(secondFile);
}

/**
 * @tc.name: AddEmptyFile
 * @tc.desc: Test AddFile handles empty file correctly
 * @tc.type: FUNC
 */
HWTEST_F(ZipStorageTest, AddEmptyFile, TestSize.Level1)
{
    std::string emptyFile = TEST_DIR + "zip_storage_test_empty.txt";
    {
        std::ofstream ofs(emptyFile, std::ios::trunc);
        ofs.close();
    }
    ZipStorage storage(TEST_ZIP_FILE);
    bool result = storage.AddFile(emptyFile);
    EXPECT_TRUE(result);
    std::filesystem::remove(emptyFile);
}

/**
 * @tc.name: AddDirectoryFails
 * @tc.desc: Test AddFile returns false when given a directory path
 * @tc.type: FUNC
 */
HWTEST_F(ZipStorageTest, AddDirectoryFails, TestSize.Level1)
{
    ZipStorage storage(TEST_ZIP_FILE);
    bool result = storage.AddFile(TEST_DIR);
    EXPECT_FALSE(result);
}
