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
#include <gmock/gmock.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "plugin_manager.h"
#include "plugin_watcher.h"

#include "logging.h"

using namespace testing::ext;

namespace {
constexpr int DEAFULT_FILE_MODE = 0777;

#if defined(__i386__) || defined(__x86_64__)
const std::string DEFAULT_TEST_PATH = "./";
#else
const std::string DEFAULT_TEST_PATH_1 = "/data/local/tmp/watchertest/1/";
const std::string DEFAULT_TEST_PATH_2 = "/data/local/tmp/watchertest/2/";
const std::string DEFAULT_TEST_PATH_3 = "/data/local/tmp/watchertest/3/";
const std::string NO_EXIST_TEST_PATH = "/data/local/tmp/noexist/";
#endif

class PluginWatchTest : public ::testing::Test {
protected:
    static constexpr int TEMP_DELAY = 10 * 1000;
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override
    {
        std::string cmd = "mkdir -p " + DEFAULT_TEST_PATH_1;
        system(cmd.c_str());
        cmd = "mkdir -p " + DEFAULT_TEST_PATH_2;
        system(cmd.c_str());
        cmd = "mkdir -p " + DEFAULT_TEST_PATH_3;
        system(cmd.c_str());
        sort(expectFileList.begin(), expectFileList.end());
    }

    void TearDown() override
    {
        cmpFileList.clear();
    }

    void OnPluginAddedStub(const std::string& path)
    {
        cmpFileList.push_back(path);
        sort(cmpFileList.begin(), cmpFileList.end());
    }

    void OnPluginRemovedStub(const std::string& path)
    {
        for (auto iter = cmpFileList.cbegin(); iter != cmpFileList.cend(); iter++) {
            if (*iter == path) {
                cmpFileList.erase(iter);
                break;
            }
        }
    }

    void CreateFile(std::string dirPath) const
    {
        for (auto it : createFileList) {
            int fd = creat((dirPath + it).c_str(), DEAFULT_FILE_MODE);
            if (fd > 0) {
                close(fd);
            }
        }
    }

    void AddFile(std::string dirPath) const
    {
        for (auto it : addFileList) {
            int fd = creat((dirPath + it).c_str(), DEAFULT_FILE_MODE);
            if (fd < 0) {
                return;
            }
            write(fd, "testcase", 1);
            close(fd);
        }
    }

    void DeleteFile(std::string dirPath) const
    {
        for (auto it : createFileList) {
            if (remove((dirPath + it).c_str()) != 0) {
                return;
            }
        }
        for (auto it : addFileList) {
            if (remove((dirPath + it).c_str()) != 0) {
                return;
            }
        }
    }

    bool CheckFileList(std::string dirPath) const
    {
        if (cmpFileList.size() != expectFileList.size()) {
            return false;
        }

        for (size_t i = 0; i < cmpFileList.size(); i++) {
            if (cmpFileList.at(i) != (dirPath + expectFileList.at(i))) {
                return false;
            }
        }

        return true;
    }

private:
    std::vector<std::string> cmpFileList;

    const std::vector<std::string> createFileList = {
        "lib_6.so", "lib_5.so", "lib_8.so", "lib_4.so", "test1.txt"
    };

    const std::vector<std::string> addFileList = {
        "libadd_6.so", "libadd_5.so", "libadd_8.so", "libadd_4.so", "test2.txt"
    };

    std::vector<std::string> expectFileList = {
        "libadd_6.so", "libadd_5.so", "libadd_8.so", "libadd_4.so",
        "lib_6.so",    "lib_5.so",    "lib_8.so",    "lib_4.so"
    };
};

class MockPluginWatcher : public PluginWatcher {
public:
    explicit MockPluginWatcher(const PluginManagerPtr& pluginManager) : PluginWatcher(pluginManager) {}
    ~MockPluginWatcher() = default;
    MOCK_METHOD1(OnPluginAdded, void(const std::string&));
    MOCK_METHOD1(OnPluginRemoved, void(const std::string&));
};

/**
 * @tc.name: plugin
 * @tc.desc: Monitor single plugin dir.
 * @tc.type: FUNC
 */
HWTEST_F(PluginWatchTest, SingleWatchDirTest, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManager>();
    MockPluginWatcher watcher(pluginManage);

    EXPECT_CALL(watcher, OnPluginAdded(testing::_)).WillRepeatedly(
        testing::Invoke(this, &PluginWatchTest::OnPluginAddedStub));
    EXPECT_CALL(watcher, OnPluginRemoved(testing::_)).WillRepeatedly(
        testing::Invoke(this, &PluginWatchTest::OnPluginRemovedStub));
    CreateFile(DEFAULT_TEST_PATH_1);

    EXPECT_TRUE(watcher.ScanPlugins(DEFAULT_TEST_PATH_1));
    EXPECT_TRUE(watcher.WatchPlugins(DEFAULT_TEST_PATH_1));
    usleep(TEMP_DELAY);
    AddFile(DEFAULT_TEST_PATH_1);
    usleep(TEMP_DELAY);
    EXPECT_EQ(CheckFileList(DEFAULT_TEST_PATH_1), true);
    DeleteFile(DEFAULT_TEST_PATH_1);
    usleep(TEMP_DELAY);
    EXPECT_EQ(cmpFileList.size(), 0);
}

/**
 * @tc.name: plugin
 * @tc.desc: Monitor multi plugin dir.
 * @tc.type: FUNC
 */
HWTEST_F(PluginWatchTest, MultiWatchDirTest, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManager>();
    MockPluginWatcher watcher(pluginManage);

    EXPECT_CALL(watcher, OnPluginAdded(testing::_)).WillRepeatedly(
        testing::Invoke(this, &PluginWatchTest::OnPluginAddedStub));
    EXPECT_CALL(watcher, OnPluginRemoved(testing::_)).WillRepeatedly(
        testing::Invoke(this, &PluginWatchTest::OnPluginRemovedStub));
    CreateFile(DEFAULT_TEST_PATH_1);
    EXPECT_TRUE(watcher.ScanPlugins(DEFAULT_TEST_PATH_1));
    EXPECT_TRUE(watcher.WatchPlugins(DEFAULT_TEST_PATH_1));
    usleep(TEMP_DELAY);
    AddFile(DEFAULT_TEST_PATH_1);
    usleep(TEMP_DELAY);
    EXPECT_EQ(CheckFileList(DEFAULT_TEST_PATH_1), true);
    DeleteFile(DEFAULT_TEST_PATH_1);
    usleep(TEMP_DELAY);
    EXPECT_EQ(cmpFileList.size(), 0);

    CreateFile(DEFAULT_TEST_PATH_2);
    EXPECT_TRUE(watcher.ScanPlugins(DEFAULT_TEST_PATH_2));
    EXPECT_TRUE(watcher.WatchPlugins(DEFAULT_TEST_PATH_2));
    usleep(TEMP_DELAY);
    AddFile(DEFAULT_TEST_PATH_2);
    usleep(TEMP_DELAY);
    EXPECT_EQ(CheckFileList(DEFAULT_TEST_PATH_2), true);
    DeleteFile(DEFAULT_TEST_PATH_2);
    usleep(TEMP_DELAY);
    EXPECT_EQ(cmpFileList.size(), 0);

    CreateFile(DEFAULT_TEST_PATH_3);
    EXPECT_TRUE(watcher.ScanPlugins(DEFAULT_TEST_PATH_3));
    EXPECT_TRUE(watcher.WatchPlugins(DEFAULT_TEST_PATH_3));
    usleep(TEMP_DELAY);
    AddFile(DEFAULT_TEST_PATH_3);
    usleep(TEMP_DELAY);
    EXPECT_EQ(CheckFileList(DEFAULT_TEST_PATH_3), true);
    DeleteFile(DEFAULT_TEST_PATH_3);
    usleep(TEMP_DELAY);
    EXPECT_EQ(cmpFileList.size(), 0);
}

/**
 * @tc.name: plugin
 * @tc.desc: Exception test.
 * @tc.type: FUNC
 */
HWTEST_F(PluginWatchTest, ExceptionTest, TestSize.Level1)
{
    auto pluginManage = std::make_shared<PluginManager>();
    MockPluginWatcher watcher(pluginManage);
    EXPECT_FALSE(watcher.ScanPlugins(NO_EXIST_TEST_PATH));
    EXPECT_FALSE(watcher.WatchPlugins(NO_EXIST_TEST_PATH));
}
} // namespace
