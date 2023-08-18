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

#include "hitrace_ops.h"

namespace {
using FTRACE_NS::HitraceOps;
using testing::ext::TestSize;

class HitraceOpsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/*
 * @tc.name: PrepareListCategoriesCmd
 * @tc.desc: test HitraceOps::PrepareListCategoriesCmd with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HitraceOpsTest, PrepareListCategoriesCmd, TestSize.Level1)
{
    std::unique_ptr<HitraceOps> hitraceOps = std::make_unique<HitraceOps>();
    EXPECT_TRUE(hitraceOps->PrepareListCategoriesCmd());
}

/*
 * @tc.name: PrepareEnableCategoriesCmd
 * @tc.desc: test HitraceOps::PrepareEnableCategoriesCmd with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HitraceOpsTest, PrepareEnableCategoriesCmd, TestSize.Level1)
{
    std::unique_ptr<HitraceOps> hitraceOps = std::make_unique<HitraceOps>();
    EXPECT_TRUE(hitraceOps->PrepareEnableCategoriesCmd());
}

/*
 * @tc.name: PrepareDisableCategoriesCmd
 * @tc.desc: test HitraceOps::PrepareDisableCategoriesCmd with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(HitraceOpsTest, PrepareDisableCategoriesCmd, TestSize.Level1)
{
    std::unique_ptr<HitraceOps> hitraceOps = std::make_unique<HitraceOps>();
    EXPECT_TRUE(hitraceOps->PrepareDisableCategoriesCmd());
}
} // namespace