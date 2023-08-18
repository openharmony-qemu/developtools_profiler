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

#include "profiler_capability_manager.h"

using namespace testing::ext;

namespace {
#if defined(__LP64__)
const std::string DEFAULT_TEST_PATH("/system/lib64/");
#else
const std::string DEFAULT_TEST_PATH("/system/lib/");
#endif
class ProfilerCapabilityManagerTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override
    {
        ProfilerCapabilityManager::GetInstance().pluginCapabilities_.clear();
    }
};

/**
 * @tc.name: memory plugin
 * @tc.desc: Add plugin capability.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCapabilityManagerTest, AddCapability, TestSize.Level1)
{
    ProfilerPluginCapability cap1;
    cap1.set_path(DEFAULT_TEST_PATH + "libcap1.so");
    cap1.set_name("cap1");

    EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(cap1));
}

/**
 * @tc.name: memory plugin
 * @tc.desc: get plugin capability.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCapabilityManagerTest, GetCapabilities, TestSize.Level1)
{
    EXPECT_EQ(ProfilerCapabilityManager::GetInstance().GetCapabilities().size(), 0);

    std::vector<ProfilerPluginCapability> caps = ProfilerCapabilityManager::GetInstance().GetCapabilities();
    EXPECT_EQ(caps.size(), 0);

    const int cnt = 10;
    for (int i = 0; i < cnt; i++) {
        ProfilerPluginCapability cap;
        cap.set_path(DEFAULT_TEST_PATH + "libcap_" + std::to_string(i) + ".so");
        cap.set_name("cap_" + std::to_string(i));
        EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(cap));
        caps = ProfilerCapabilityManager::GetInstance().GetCapabilities();
        EXPECT_EQ(caps.size(), i + 1);
    }
}

/**
 * @tc.name: memory plugin
 * @tc.desc: get plugin capability.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCapabilityManagerTest, GetCapability, TestSize.Level1)
{
    EXPECT_EQ(ProfilerCapabilityManager::GetInstance().GetCapability("xxx"), nullptr);

    const int cnt = 10;
    for (int i = 0; i < cnt; i++) {
        ProfilerPluginCapability cap;
        cap.set_path(DEFAULT_TEST_PATH + "libcap_" + std::to_string(i) + ".so");
        cap.set_name("cap_" + std::to_string(i));
        EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(cap));
    }

    for (int i = 0; i < cnt; i++) {
        ProfilerPluginCapability cap;
        cap.set_name("cap_" + std::to_string(i));
        auto capPtr = ProfilerCapabilityManager::GetInstance().GetCapability(cap.name());
        ASSERT_NE(capPtr, nullptr);
        EXPECT_EQ(capPtr->name(), cap.name());
    }
}

/**
 * @tc.name: memory plugin
 * @tc.desc: update plugin capability.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCapabilityManagerTest, UpdateCapability, TestSize.Level1)
{
    ProfilerPluginCapability cap1;
    cap1.set_path(DEFAULT_TEST_PATH + "libcap1.so");
    cap1.set_name("cap1");
    EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(cap1));

    ProfilerPluginCapability cap2(cap1);
    cap2.set_path(DEFAULT_TEST_PATH + "libcap2.so");
    EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().UpdateCapability("cap1", cap2));

    std::vector<ProfilerPluginCapability> caps = ProfilerCapabilityManager::GetInstance().GetCapabilities();
    ASSERT_EQ(caps.size(), 1);
    EXPECT_EQ(caps[0].path(), cap2.path());
}

/**
 * @tc.name: server
 * @tc.desc: remove plugin capability.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerCapabilityManagerTest, RemoveCapability, TestSize.Level1)
{
    const int cnt = 10;
    std::vector<ProfilerPluginCapability> caps;
    for (int i = 0; i < cnt; i++) {
        ProfilerPluginCapability cap;
        cap.set_path(DEFAULT_TEST_PATH + "libcap_" + std::to_string(i) + ".so");
        cap.set_name("cap_" + std::to_string(i));
        EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().AddCapability(cap));
        caps = ProfilerCapabilityManager::GetInstance().GetCapabilities();
        EXPECT_EQ(caps.size(), i + 1);
    }

    for (int i = 0; i < cnt; i++) {
        auto cap = caps[i];
        EXPECT_TRUE(ProfilerCapabilityManager::GetInstance().RemoveCapability(cap.name()));

        std::vector<ProfilerPluginCapability> tmpCaps;
        tmpCaps = ProfilerCapabilityManager::GetInstance().GetCapabilities();
        EXPECT_EQ(tmpCaps.size(), cnt - (i + 1));
    }
}
} // namespace