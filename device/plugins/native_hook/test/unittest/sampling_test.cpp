/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "sampling.h"

using namespace testing::ext;

namespace {

class SamplerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/*
 * @tc.name: Sampler
 * @tc.desc: test Sampler::InitSampling with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(SamplerTest, InitSampling001, TestSize.Level0)
{
    Sampling sampler;
    sampler.InitSampling(512);
    EXPECT_EQ(sampler.StartSampling(1024), 1024u);
}

/*
 * @tc.name: Sampler
 * @tc.desc: test Sampler::InitSampling with abnormal case.
 * @tc.type: FUNC
 */
HWTEST_F(SamplerTest, InitSampling002, TestSize.Level0)
{
    Sampling sampler;
    sampler.SetRandomEngineSeed(1);
    sampler.InitSampling(512);
    EXPECT_EQ(sampler.StartSampling(511), 512u);
}

/*
 * @tc.name: Sampler
 * @tc.desc: test Sampler::InitSampling with boundary case.
 * @tc.type: FUNC
 */
HWTEST_F(SamplerTest, InitSampling003, TestSize.Level0)
{
    Sampling sampler;
    sampler.InitSampling(1);
    EXPECT_EQ(sampler.StartSampling(1), 1u);
    EXPECT_EQ(sampler.StartSampling(5), 5u);
    EXPECT_EQ(sampler.StartSampling(9), 9u);
}

/*
 * @tc.name: Sampler
 * @tc.desc: test Sampler::Reset with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(SamplerTest, Reset, TestSize.Level0)
{
    Sampling sampler;
    sampler.InitSampling(4096);
    sampler.Reset();
    EXPECT_EQ(sampler.GetSampleInterval(), 0);
}
} // namespace