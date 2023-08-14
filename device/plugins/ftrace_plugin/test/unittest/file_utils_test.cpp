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
#include <fcntl.h>
#include <fstream>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "file_utils.h"

using namespace testing::ext;

namespace {
constexpr int FILE_MODE = 0644;

class FileUtilsTest : public ::testing::Test {
protected:
    std::string data_;
    std::string path_;

    void SetUp() override
    {
        data_ = "ABCDEFGH";
        path_ = "file_utils_test.txt";
        std::ofstream fout(path_);
        fout << data_;
    }

    void TearDown() override
    {
        unlink(path_.c_str());
    }
};

/*
 * @tc.name: ReadFileFromFd
 * @tc.desc: test FileUtils::ReadFile with fd.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, ReadFileFromFd, TestSize.Level1)
{
    int fd = open(path_.c_str(), O_RDONLY);
    ASSERT_GT(fd, 0);

    std::string text = FileUtils::ReadFile(fd);
    EXPECT_EQ(text, data_);
    EXPECT_EQ(close(fd), 0);
}

/*
 * @tc.name: ReadFileFromInvalidFd
 * @tc.desc: test FileUtils::ReadFile with invalid fd.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, ReadFileFromInvalidFd, TestSize.Level1)
{
    int fd = -1;
    std::string text = FileUtils::ReadFile(fd);
    EXPECT_EQ(text, "");
}

/*
 * @tc.name: ReadFileFromPath
 * @tc.desc: test FileUtils::ReadFile with path.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, ReadFileFromPath, TestSize.Level1)
{
    std::string text = FileUtils::ReadFile(path_);
    EXPECT_EQ(text, data_);
}

/*
 * @tc.name: ReadFileFromInvalidPath
 * @tc.desc: test FileUtils::ReadFile with invalid path.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, ReadFileFromInvalidPath, TestSize.Level1)
{
    std::string path = "file-name-that-not-exists.txt";
    std::string text = FileUtils::ReadFile(path);
    EXPECT_EQ(text, "");
}

/*
 * @tc.name: WriteFileWithPath
 * @tc.desc: test FileUtils::ReadFile with path.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, WriteFileWithPath, TestSize.Level1)
{
    std::string path = "temp.txt";

    int nbytes = data_.size();
    int retval = FileUtils::WriteFile(path, data_, O_CREAT | O_RDWR, FILE_MODE);
    EXPECT_EQ(retval, nbytes); // expect write success!

    std::string text = FileUtils::ReadFile(path);
    EXPECT_EQ(text, data_); // expect same content read back!

    EXPECT_EQ(unlink(path.c_str()), 0);
}

/*
 * @tc.name: WriteFileWithInvalidPath
 * @tc.desc: test FileUtils::ReadFile with path.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, WriteFileWithInvalidPath, TestSize.Level1)
{
    std::string path = "dir_not_exists/temp.txt";

    int retval = FileUtils::WriteFile(path, data_);
    EXPECT_LT(retval, 0);
}

/*
 * @tc.name: WriteFileWithPathOptions
 * @tc.desc: test FileUtils::ReadFile with path.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, WriteFileWithPathOptions, TestSize.Level1)
{
    std::string path = "temp.txt";

    int nbytes = data_.size();
    int retval = FileUtils::WriteFile(path, data_, O_CREAT | O_RDWR, FILE_MODE);
    EXPECT_EQ(retval, nbytes);

    std::string text = FileUtils::ReadFile(path);
    EXPECT_EQ(text, data_); // expect same content read back!

    EXPECT_EQ(unlink(path.c_str()), 0);
}

/*
 * @tc.name: WriteFileWithInvalidPath
 * @tc.desc: test FileUtils::ReadFile with path.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, WriteFileWithInvalidPathOptions, TestSize.Level1)
{
    std::string path = "dir_not_exists/temp.txt";

    int retval = FileUtils::WriteFile(path, data_);
    EXPECT_LT(retval, 0);
}

/*
 * @tc.name: ListDirNormal
 * @tc.desc: test FileUtils::ListDir with path.
 * @tc.type: FUNC
 */
HWTEST_F(FileUtilsTest, ListDirNormal, TestSize.Level1)
{
    std::string path = "/";

    auto items = FileUtils::ListDir(path);
    EXPECT_GT(items.size(), static_cast<size_t>(0));
}
} // namespace