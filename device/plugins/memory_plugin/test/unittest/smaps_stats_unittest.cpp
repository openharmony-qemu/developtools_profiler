 /*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#include <dlfcn.h>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "memory_data_plugin.h"
#include "plugin_module_api.h"
#include "smaps_stats.h"

using namespace testing::ext;

namespace {
const std::string DEFAULT_TEST_PATH("/data/local/tmp/utresources/");
const int PERCENT = 100;

class SmapsStatsTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase()
    {
        if (access(DEFAULT_TEST_PATH.c_str(), F_OK) == 0) {
            std::string str = "rm -rf " + DEFAULT_TEST_PATH;
            system(str.c_str());
        }
    }

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMapHead
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestParseMapHead1, TestSize.Level1)
{
    std::string line = "00400000-00409000 \n";
    SmapsStats plugin;
    MapPiecesInfo expectMappic = {0x00400000, 0x00409000, ""};
    MapPiecesInfo targetMappic = {0};
    SmapsHeadInfo expectSmapsHeadInfo = {"00400000", "00409000", "", "", -1};
    SmapsHeadInfo smapsHeadInfo = {};
    ASSERT_FALSE(plugin.ParseMapHead(line, targetMappic, smapsHeadInfo));
    // test result
    EXPECT_EQ(expectMappic.startAddr, targetMappic.startAddr);
    EXPECT_EQ(expectMappic.endAddr, targetMappic.endAddr);
    EXPECT_STREQ(expectMappic.name.c_str(), targetMappic.name.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.permission.c_str(), smapsHeadInfo.permission.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.path.c_str(), smapsHeadInfo.path.c_str());
    EXPECT_EQ(expectSmapsHeadInfo.iNode, smapsHeadInfo.iNode);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMapHead
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestParseMapHead2, TestSize.Level1)
{
    std::string line = "00400000-00409000 r-xp\n";
    SmapsStats plugin;
    MapPiecesInfo expectMappic = {0x00400000, 0x00409000, ""};
    MapPiecesInfo targetMappic = {0};
    SmapsHeadInfo expectSmapsHeadInfo = {"00400000", "00409000", "", "", -1};
    SmapsHeadInfo smapsHeadInfo = {};
    ASSERT_FALSE(plugin.ParseMapHead(line, targetMappic, smapsHeadInfo));
    // test result
    EXPECT_EQ(expectMappic.startAddr, targetMappic.startAddr);
    EXPECT_EQ(expectMappic.endAddr, targetMappic.endAddr);
    EXPECT_STREQ(expectMappic.name.c_str(), targetMappic.name.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.permission.c_str(), smapsHeadInfo.permission.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.path.c_str(), smapsHeadInfo.path.c_str());
    EXPECT_EQ(expectSmapsHeadInfo.iNode, smapsHeadInfo.iNode);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMapHead
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestParseMapHead3, TestSize.Level1)
{
    std::string line = "00400000-00409000 r-xp 00000000\n";
    SmapsStats plugin;
    MapPiecesInfo expectMappic = {0x00400000, 0x00409000, ""};
    MapPiecesInfo targetMappic = {0};
    SmapsHeadInfo expectSmapsHeadInfo = {"00400000", "00409000", "r-x", "", -1};
    SmapsHeadInfo smapsHeadInfo = {};
    ASSERT_FALSE(plugin.ParseMapHead(line, targetMappic, smapsHeadInfo));
    // test result
    EXPECT_EQ(expectMappic.startAddr, targetMappic.startAddr);
    EXPECT_EQ(expectMappic.endAddr, targetMappic.endAddr);
    EXPECT_STREQ(expectMappic.name.c_str(), targetMappic.name.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.permission.c_str(), smapsHeadInfo.permission.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.path.c_str(), smapsHeadInfo.path.c_str());
    EXPECT_EQ(expectSmapsHeadInfo.iNode, smapsHeadInfo.iNode);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMapHead
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestParseMapHead4, TestSize.Level1)
{
    std::string line = "00400000-00409000 r-xp 00000000 fc:00\n";
    SmapsStats plugin;
    MapPiecesInfo expectMappic = {0x00400000, 0x00409000, ""};
    MapPiecesInfo targetMappic = {0};
    SmapsHeadInfo expectSmapsHeadInfo = {"00400000", "00409000", "r-x", "", -1};
    SmapsHeadInfo smapsHeadInfo = {};
    ASSERT_FALSE(plugin.ParseMapHead(line, targetMappic, smapsHeadInfo));
    // test result
    EXPECT_EQ(expectMappic.startAddr, targetMappic.startAddr);
    EXPECT_EQ(expectMappic.endAddr, targetMappic.endAddr);
    EXPECT_STREQ(expectMappic.name.c_str(), targetMappic.name.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.permission.c_str(), smapsHeadInfo.permission.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.path.c_str(), smapsHeadInfo.path.c_str());
    EXPECT_EQ(expectSmapsHeadInfo.iNode, smapsHeadInfo.iNode);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMapHead
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestParseMapHead5, TestSize.Level1)
{
    std::string line = "00400000-00409000 r-xp 00000000 fc:00 426998\n";
    SmapsStats plugin;
    MapPiecesInfo expectMappic = {0x00400000, 0x00409000, ""};
    MapPiecesInfo targetMappic = {0};
    SmapsHeadInfo expectSmapsHeadInfo = {"00400000", "00409000", "r-x", "", -1};
    SmapsHeadInfo smapsHeadInfo = {};
    ASSERT_FALSE(plugin.ParseMapHead(line, targetMappic, smapsHeadInfo));
    // test result
    EXPECT_EQ(expectMappic.startAddr, targetMappic.startAddr);
    EXPECT_EQ(expectMappic.endAddr, targetMappic.endAddr);
    EXPECT_STREQ(expectMappic.name.c_str(), targetMappic.name.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.permission.c_str(), smapsHeadInfo.permission.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.path.c_str(), smapsHeadInfo.path.c_str());
    EXPECT_EQ(expectSmapsHeadInfo.iNode, smapsHeadInfo.iNode);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMapHead
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestParseMapHead6, TestSize.Level1)
{
    std::string line = "00400000-00409000 r-xp 00000000 fc:00 426998  /usr/lib/gvfs/gvfsd-http\n";
    SmapsStats plugin;
    MapPiecesInfo expectMappic = {0x00400000, 0x00409000, "/usr/lib/gvfs/gvfsd-http"};
    MapPiecesInfo targetMappic = {0};
    SmapsHeadInfo expectSmapsHeadInfo = {"00400000", "00409000", "r-x", "/usr/lib/gvfs/gvfsd-http", 426998};
    SmapsHeadInfo smapsHeadInfo = {};
    ASSERT_TRUE(plugin.ParseMapHead(line, targetMappic, smapsHeadInfo));
    // test result
    EXPECT_EQ(expectMappic.startAddr, targetMappic.startAddr);
    EXPECT_EQ(expectMappic.endAddr, targetMappic.endAddr);
    EXPECT_STREQ(expectMappic.name.c_str(), targetMappic.name.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.permission.c_str(), smapsHeadInfo.permission.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.path.c_str(), smapsHeadInfo.path.c_str());
    EXPECT_EQ(expectSmapsHeadInfo.iNode, smapsHeadInfo.iNode);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMapHead
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestParseMapHead7, TestSize.Level1)
{
    std::string line = "00400000-00409000 r-xp 00000000 fc:00 426998  /usr/lib/gvfs/gvfsd-http";
    SmapsStats plugin;
    MapPiecesInfo expectMappic = {0x00400000, 0x00409000, "/usr/lib/gvfs/gvfsd-htt"};
    MapPiecesInfo targetMappic = {0};
    SmapsHeadInfo expectSmapsHeadInfo = {"00400000", "00409000", "r-x", "/usr/lib/gvfs/gvfsd-htt", 426998};
    SmapsHeadInfo smapsHeadInfo = {};
    ASSERT_TRUE(plugin.ParseMapHead(line, targetMappic, smapsHeadInfo));
    // test result
    EXPECT_EQ(expectMappic.startAddr, targetMappic.startAddr);
    EXPECT_EQ(expectMappic.endAddr, targetMappic.endAddr);
    EXPECT_STREQ(expectMappic.name.c_str(), targetMappic.name.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.permission.c_str(), smapsHeadInfo.permission.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.path.c_str(), smapsHeadInfo.path.c_str());
    EXPECT_EQ(expectSmapsHeadInfo.iNode, smapsHeadInfo.iNode);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMapHead
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestParseMapHead8, TestSize.Level1)
{
    std::string line = "564045dbe000-564045ddf000 rw-p 00000000 00:00 0                          [heap]";
    SmapsStats plugin;
    MapPiecesInfo expectMappic = {0x564045dbe000, 0x564045ddf000, "[heap"};
    MapPiecesInfo targetMappic = {0};
    SmapsHeadInfo expectSmapsHeadInfo = {"564045dbe000", "564045ddf000", "rw-", "[heap", 0};
    SmapsHeadInfo smapsHeadInfo = {};
    ASSERT_TRUE(plugin.ParseMapHead(line, targetMappic, smapsHeadInfo));
    // test result
    EXPECT_EQ(expectMappic.startAddr, targetMappic.startAddr);
    EXPECT_EQ(expectMappic.endAddr, targetMappic.endAddr);
    EXPECT_STREQ(expectMappic.name.c_str(), targetMappic.name.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.startAddrStr.c_str(), smapsHeadInfo.startAddrStr.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.permission.c_str(), smapsHeadInfo.permission.c_str());
    EXPECT_STREQ(expectSmapsHeadInfo.path.c_str(), smapsHeadInfo.path.c_str());
    EXPECT_EQ(expectSmapsHeadInfo.iNode, smapsHeadInfo.iNode);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField1, TestSize.Level1)
{
    std::string line = "Pss:                   4 kB";
    SmapsStats plugin;
    MemUsageInfo expect = {0, 0, 4};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField2, TestSize.Level1)
{
    std::string line = "Private_Clean:         0 kB";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField3, TestSize.Level1)
{
    std::string line = "Private_Dirty:         4 kB";
    SmapsStats plugin;
    uint64_t num = 4;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    expect.privateDirty = num;
    expect.uss += num;
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField4, TestSize.Level1)
{
    std::string line = "Size:                  4 kB";
    SmapsStats plugin;
    uint64_t num = 4;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    expect.vss = num;
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField5, TestSize.Level1)
{
    std::string line = "Shared_Clean:          0 kB";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField6, TestSize.Level1)
{
    std::string line = "Shared_Dirty:          0 kB";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField7, TestSize.Level1)
{
    std::string line = "Swap:                  0 kB";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField8, TestSize.Level1)
{
    std::string line = "SwapPss:               0 kB";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField9, TestSize.Level1)
{
    std::string line = "Rss:                   4 kB";
    SmapsStats plugin;
    uint64_t num = 4;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    expect.rss = num;
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField10, TestSize.Level1)
{
    std::string line = "VmFlags: rd ex mr mw me dw sd";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestGetMemUsageField11, TestSize.Level1)
{
    std::string line = "Anonymous:             0 kB";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestAbnormalGetMemUsageField1, TestSize.Level1)
{
    std::string line = "Pss:";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_TRUE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test GetMemUsageField
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestAbnormalGetMemUsageField2, TestSize.Level1)
{
    std::string line = "Pss";
    SmapsStats plugin;
    MemUsageInfo expect = {0};
    MemUsageInfo target = {0};

    ASSERT_FALSE(plugin.GetMemUsageField(line, target));
    // test result
    EXPECT_EQ(expect.vss, target.vss);
    EXPECT_EQ(expect.rss, target.rss);
    EXPECT_EQ(expect.pss, target.pss);
    EXPECT_EQ(expect.uss, target.uss);
    EXPECT_EQ(expect.swap, target.swap);
    EXPECT_EQ(expect.swapPss, target.swapPss);
    EXPECT_EQ(expect.privateClean, target.privateClean);
    EXPECT_EQ(expect.privateDirty, target.privateDirty);
    EXPECT_EQ(expect.sharedClean, target.sharedClean);
    EXPECT_EQ(expect.sharedDirty, target.sharedDirty);
}

namespace {
const char* SMAPS_FORMAT = R"(5637ebe15000-5637ebe16000 r-xp 00000000 08:01 4587541                    test/run
Size:                  4 kB
Rss:                   4 kB
Pss:                   4 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         4 kB
Private_Dirty:         0 kB
Anonymous:             0 kB
Swap:                  0 kB
SwapPss:               0 kB
LazyFree:              1 kB
AnonHugePages:         1 kB
ShmemPmdMapped:        1 kB
FilePmdMapped:        1 kB
Shared_Hugetlb:        1 kB
Private_Hugetlb:       1 kB
Locked:                1 kB
THPeligible:           1
VmFlags: rd ex mr mw me dw sd
5637ec015000-5637ec016000 r--p 00000000 08:01 4587541                    test/run
Size:                  4 kB
Rss:                   4 kB
Pss:                   4 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         4 kB
Anonymous:             4 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd mr mw me dw ac sd
5637ec016000-5637ec017000 rw-p 00001000 08:01 4587541                    test/run
Size:                  4 kB
Rss:                   4 kB
Pss:                   4 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         4 kB
Anonymous:             4 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd wr mr mw me dw ac sd
5637ec60f000-5637ec630000 rw-p 00000000 00:00 0                          [heap]
Size:                132 kB
Rss:                   4 kB
Pss:                   4 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         4 kB
Anonymous:             4 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd wr mr mw me ac sd
7fa384a9e000-7fa384c85000 r-xp 00000000 08:01 929310                     /lib/x86_64-linux-gnu/libc-2.27.so
Size:               1948 kB
Rss:                1200 kB
Pss:                  20 kB
Shared_Clean:       1200 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         0 kB
Anonymous:             0 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd ex mr mw me sd
7fa384c85000-7fa384e85000 ---p 001e7000 08:01 929310                     /lib/x86_64-linux-gnu/libc-2.27.so
Size:               2048 kB
Rss:                   0 kB
Pss:                   0 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         0 kB
Anonymous:             0 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: mr mw me sd
7fa384e85000-7fa384e89000 r--p 001e7000 08:01 929310                     /lib/x86_64-linux-gnu/libc-2.27.so
Size:                 16 kB
Rss:                  16 kB
Pss:                  16 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:        16 kB
Anonymous:            16 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd mr mw me ac sd
7fa384e89000-7fa384e8b000 rw-p 001eb000 08:01 929310                     /lib/x86_64-linux-gnu/libc-2.27.so
Size:                  8 kB
Rss:                   8 kB
Pss:                   8 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         8 kB
Anonymous:             8 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd wr mr mw me ac sd
7fa384e8b000-7fa384e8f000 rw-p 00000000 00:00 0 
Size:                 16 kB
Rss:                  12 kB
Pss:                  12 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:        12 kB
Anonymous:            12 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd wr mr mw me ac sd
7fa384e8f000-7fa384eb8000 r-xp 00000000 08:01 927476                     /lib/x86_64-linux-gnu/ld-2.27.so
Size:                164 kB
Rss:                 164 kB
Pss:                   3 kB
Shared_Clean:        164 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         0 kB
Anonymous:             0 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd ex mr mw me dw sd
7fa3850a1000-7fa3850a3000 rw-p 00000000 00:00 0 
Size:                  8 kB
Rss:                   8 kB
Pss:                   8 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         8 kB
Anonymous:             8 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd wr mr mw me ac sd
7fa3850b8000-7fa3850b9000 r--p 00029000 08:01 927476                     /lib/x86_64-linux-gnu/ld-2.27.so
Size:                  4 kB
Rss:                   4 kB
Pss:                   4 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         4 kB
Anonymous:             4 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd mr mw me dw ac sd
7fa3850b9000-7fa3850ba000 rw-p 0002a000 08:01 927476                     /lib/x86_64-linux-gnu/ld-2.27.so
Size:                  4 kB
Rss:                   4 kB
Pss:                   4 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         4 kB
Anonymous:             4 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd wr mr mw me dw ac sd
7fa3850ba000-7fa3850bb000 rw-p 00000000 00:00 0 
Size:                  4 kB
Rss:                   4 kB
Pss:                   4 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         4 kB
Anonymous:             4 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd wr mr mw me ac sd
7ffc54ec0000-7ffc54ee1000 rw-p 00000000 00:00 0                          [stack]
Size:                132 kB
Rss:                  12 kB
Pss:                  12 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:        12 kB
Anonymous:            12 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd wr mr mw me gd ac
7ffc54f1c000-7ffc54f1f000 r--p 00000000 00:00 0                          [vvar]
Size:                 12 kB
Rss:                   0 kB
Pss:                   0 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         0 kB
Anonymous:             0 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd mr pf io de dd sd
7ffc54f1f000-7ffc54f20000 r-xp 00000000 00:00 0                          [vdso]
Size:                  4 kB
Rss:                   4 kB
Pss:                   0 kB
Shared_Clean:          4 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         0 kB
Anonymous:             0 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: rd ex mr mw me de sd
ffffffffff600000-ffffffffff601000 --xp 00000000 00:00 0                  [vsyscall]
Size:                  4 kB
MMUPageSize:           4 kB
Rss:                   0 kB
Pss:                   0 kB
Shared_Clean:          0 kB
Shared_Dirty:          0 kB
Private_Clean:         0 kB
Private_Dirty:         0 kB
Anonymous:             0 kB
Swap:                  0 kB
SwapPss:               0 kB
VmFlags: ex)";
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ReadVmemareasFile
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestFramework, TestSize.Level1)
{
    bool findMapHead = false;
    MapPiecesInfo mappic = {0};
    MemUsageInfo memusage = {0};
    SmapsHeadInfo smapsHeadInfo = {};
    // 3036, 4256, 288, 748, 0, 1388
    uint64_t prevEnd = 0;
    int prevHeap = 0;
    uint64_t java_heap = 0;
    uint64_t native_heap = 4;
    uint64_t code = 48;
    uint64_t stack = 12;
    uint64_t graphics = 0;
    uint64_t private_other = 20;
    SmapsStats plugin;
    std::istringstream ss(SMAPS_FORMAT);
    std::string line;

    while (std::getline(ss, line)) {
        line += '\n';
        if (!findMapHead) {
            ASSERT_TRUE(plugin.ParseMapHead(line, mappic, smapsHeadInfo));
            findMapHead = true;
            continue;
        }
        if (findMapHead) {
            if (line.substr(0, strlen("VmFlags:")) != std::string("VmFlags:")) {
                ASSERT_TRUE(plugin.GetMemUsageField(line, memusage));
                continue;
            }
        }
        plugin.CollectVmemAreasData(mappic, memusage, prevEnd, prevHeap);
        findMapHead = false;
    }
    plugin.ReviseStatsData();

    // test result
    EXPECT_EQ(java_heap, (uint64_t)(plugin.GetProcessJavaHeap()));
    EXPECT_EQ(native_heap, (uint64_t)(plugin.GetProcessNativeHeap()));
    EXPECT_EQ(code, (uint64_t)(plugin.GetProcessCode()));
    EXPECT_EQ(stack, (uint64_t)(plugin.GetProcessStack()));
    EXPECT_EQ(graphics, (uint64_t)(plugin.GetProcessGraphics()));
    EXPECT_EQ(private_other, (uint64_t)(plugin.GetProcessPrivateOther()));
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test ParseMaps
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestFrameworkForPath, TestSize.Level1)
{
    // 3036, 4256, 288, 748, 0, 1388
    int pid = 2;
    uint64_t java_heap = 0;
    uint64_t native_heap = 4;
    uint64_t code = 48;
    uint64_t stack = 12;
    uint64_t graphics = 0;
    uint64_t private_other = 20;

    uint64_t size = 4;
    uint64_t rss = 4;
    uint64_t pss = 4;
    uint64_t dirty = 0;
    uint64_t swapper = 0;
    double reside = static_cast<double>(rss) / size * PERCENT;
    uint64_t dirty2 = 4;
    uint64_t privateClean = 4;
    uint64_t privateDirty = 0;
    uint64_t sharedClean = 0;
    uint64_t sharedDirty = 0;
    uint64_t swap = 0;
    uint64_t swapPss = 0;
    uint64_t privateClean2 = 0;
    uint64_t privateDirty2 = 4;
    uint64_t sharedClean2 = 0;
    uint64_t sharedDirty2 = 0;
    uint64_t swap2 = 0;
    uint64_t swapPss2 = 0;

    ASSERT_EQ(access(DEFAULT_TEST_PATH.c_str(), F_OK), 0);
    SmapsStats plugin(DEFAULT_TEST_PATH + std::string("proc/"));

    // test result
    ProcessMemoryInfo processMemoryInfo;
    ASSERT_TRUE(plugin.ParseMaps(pid, processMemoryInfo, true, true));
    EXPECT_EQ(java_heap, (uint64_t)(plugin.GetProcessJavaHeap()));
    EXPECT_EQ(native_heap, (uint64_t)(plugin.GetProcessNativeHeap()));
    EXPECT_EQ(code, (uint64_t)(plugin.GetProcessCode()));
    EXPECT_EQ(stack, (uint64_t)(plugin.GetProcessStack()));
    EXPECT_EQ(graphics, (uint64_t)(plugin.GetProcessGraphics()));
    EXPECT_EQ(private_other, (uint64_t)(plugin.GetProcessPrivateOther()));

    EXPECT_EQ(size, processMemoryInfo.smapinfo(0).size());
    EXPECT_EQ(rss, processMemoryInfo.smapinfo(0).rss());
    EXPECT_EQ(pss, processMemoryInfo.smapinfo(0).pss());
    EXPECT_EQ(dirty, processMemoryInfo.smapinfo(0).dirty());
    EXPECT_EQ(swapper, processMemoryInfo.smapinfo(0).swapper());
    EXPECT_EQ(privateClean, processMemoryInfo.smapinfo(0).private_clean());
    EXPECT_EQ(privateDirty, processMemoryInfo.smapinfo(0).private_dirty());
    EXPECT_EQ(sharedClean, processMemoryInfo.smapinfo(0).shared_clean());
    EXPECT_EQ(sharedDirty, processMemoryInfo.smapinfo(0).shared_dirty());
    EXPECT_EQ(swap, processMemoryInfo.smapinfo(0).swap());
    EXPECT_EQ(swapPss, processMemoryInfo.smapinfo(0).swap_pss());
    EXPECT_EQ(reside, processMemoryInfo.smapinfo(0).reside());
    EXPECT_EQ(size, processMemoryInfo.smapinfo(1).size());
    EXPECT_EQ(rss, processMemoryInfo.smapinfo(1).rss());
    EXPECT_EQ(pss, processMemoryInfo.smapinfo(1).pss());
    EXPECT_EQ(dirty2, processMemoryInfo.smapinfo(1).dirty());
    EXPECT_EQ(swapper, processMemoryInfo.smapinfo(1).swapper());
    EXPECT_EQ(privateClean2, processMemoryInfo.smapinfo(1).private_clean());
    EXPECT_EQ(privateDirty2, processMemoryInfo.smapinfo(1).private_dirty());
    EXPECT_EQ(sharedClean2, processMemoryInfo.smapinfo(1).shared_clean());
    EXPECT_EQ(sharedDirty2, processMemoryInfo.smapinfo(1).shared_dirty());
    EXPECT_EQ(swap2, processMemoryInfo.smapinfo(1).swap());
    EXPECT_EQ(swapPss2, processMemoryInfo.smapinfo(1).swap_pss());
    EXPECT_EQ(reside, processMemoryInfo.smapinfo(1).reside());
}

/**
 * @tc.name: smaps stats
 * @tc.desc: test check framework
 * @tc.type: FUNC
 */
HWTEST_F(SmapsStatsTest, TestCheckFramework, TestSize.Level1)
{
    bool findMapHead = false;
    MapPiecesInfo mappic = {0};
    MemUsageInfo memusage = {0};
    SmapsHeadInfo smapsHeadInfo = {};
    uint64_t prevEnd = 0;
    int prevHeap = 0;
    SmapsStats testPlugin;
    int pid = 2;
    std::istringstream ss(SMAPS_FORMAT);
    std::string line;

    while (std::getline(ss, line)) {
        line += '\n';
        if (!findMapHead) {
            ASSERT_TRUE(testPlugin.ParseMapHead(line, mappic, smapsHeadInfo));
            findMapHead = true;
            continue;
        }
        if (findMapHead) {
            if (line.substr(0, strlen("VmFlags:")) != std::string("VmFlags:")) {
                ASSERT_TRUE(testPlugin.GetMemUsageField(line, memusage));
                continue;
            }
        }
        testPlugin.CollectVmemAreasData(mappic, memusage, prevEnd, prevHeap);
        findMapHead = false;
    }
    testPlugin.ReviseStatsData();

    ASSERT_EQ(access(DEFAULT_TEST_PATH.c_str(), F_OK), 0);
    SmapsStats targetPlugin(DEFAULT_TEST_PATH + std::string("proc/"));
    ProcessMemoryInfo processMemoryInfo;
    ASSERT_TRUE(targetPlugin.ParseMaps(pid, processMemoryInfo, true, false));

    // test result
    EXPECT_EQ(testPlugin.GetProcessJavaHeap(), targetPlugin.GetProcessJavaHeap());
    EXPECT_EQ(testPlugin.GetProcessNativeHeap(), targetPlugin.GetProcessNativeHeap());
    EXPECT_EQ(testPlugin.GetProcessCode(), targetPlugin.GetProcessCode());
    EXPECT_EQ(testPlugin.GetProcessStack(), targetPlugin.GetProcessStack());
    EXPECT_EQ(testPlugin.GetProcessGraphics(), targetPlugin.GetProcessGraphics());
    EXPECT_EQ(testPlugin.GetProcessPrivateOther(), targetPlugin.GetProcessPrivateOther());
}
} // namespace