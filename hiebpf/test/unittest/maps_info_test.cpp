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

#include "maps_info.h"

using namespace testing::ext;

namespace OHOS {
namespace Developtools {
namespace Hiebpf {
class MapsInofTest : public ::testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};

    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: GetMaps
 * @tc.desc: Test framework
 * @tc.type: FUNC
 */
HWTEST_F(MapsInofTest, GetMaps, TestSize.Level1)
{
    MapsInfo mapsInfo;
    std::vector<MapsInfo::MapsItem> mapsItems;
    mapsInfo.GetMaps(0, mapsItems);
    ASSERT_EQ(mapsItems.size(), 0);
    mapsInfo.GetMaps(-1, mapsItems);
    ASSERT_EQ(mapsItems.size(), 0);

    mapsInfo.GetMaps(1, mapsItems);
    ASSERT_GE(mapsItems.size(), 1);
    ASSERT_EQ(mapsItems[0].pid_, 1);
    ASSERT_FALSE(mapsItems[0].fileName_.find("init") == std::string::npos);
}

HWTEST_F(MapsInofTest, Cache, TestSize.Level1)
{
    MapsInfo mapsInfo;
    ASSERT_FALSE(mapsInfo.IsCached(1));
    mapsInfo.CachePid(1);
    ASSERT_TRUE(mapsInfo.IsCached(1));
    mapsInfo.RemovePid(1);
    ASSERT_FALSE(mapsInfo.IsCached(1));
}

HWTEST_F(MapsInofTest, GetBinary, TestSize.Level1)
{
    MapsInfo mapsInfo;
    MapsInfo::MapsItem mapsItem;
    const int testNum = 1000;
    mapsItem.start_ = 0;
    mapsItem.end_ = testNum;
    mapsItem.offset_ = testNum;
    mapsItem.pid_ = testNum;
    mapsItem.fileName_ = "testGetBinary";

    std::vector<uint8_t> buf;
    uint32_t size = mapsInfo.GetBinary(mapsItem, buf);
    const uint32_t fixLen = sizeof(mapsItem.start_) + sizeof(mapsItem.end_) +
                      sizeof(mapsItem.offset_) + sizeof(mapsItem.pid_) + sizeof(uint32_t);
    ASSERT_EQ(size, fixLen + mapsItem.fileName_.size() + 1);

    uint8_t *p = buf.data();
    uint64_t *start = (uint64_t *)p;
    ASSERT_EQ(mapsItem.start_, *start);
    p += sizeof(mapsItem.start_);

    uint64_t *end = (uint64_t *)p;
    ASSERT_EQ(mapsItem.end_, *end);
    p += sizeof(mapsItem.end_);

    uint32_t *offset = (uint32_t *)p;
    ASSERT_EQ(mapsItem.offset_, *offset);
    p += sizeof(mapsItem.offset_);

    uint32_t *pid = (uint32_t *)p;
    ASSERT_EQ(mapsItem.pid_, *pid);
    p += sizeof(mapsItem.pid_);

    uint32_t *fileNameLen = (uint32_t *)p;
    ASSERT_EQ((uint32_t)(mapsItem.fileName_.size() + 1), *fileNameLen);
    p += sizeof(uint32_t);

    std::string fileName = (char *)p;
    ASSERT_EQ(fileName, mapsItem.fileName_);
}
} // namespace Hiebpf
} // namespace Developtools
} // namespace OHOS
