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
#include "trace_ops.h"

namespace {
using FTRACE_NS::HitraceOps;
using FTRACE_NS::TraceOps;
using testing::ext::TestSize;

class TraceOpsTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/*
 * @tc.name: GetTraceType
 * @tc.desc: test HitraceOps::GetTraceType with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, GetTraceType, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<HitraceOps>();
    EXPECT_TRUE(traceOps->GetTraceType() == TraceOps::TraceType::HITRACE);
}

/*
 * @tc.name: GetCommand
 * @tc.desc: test HitraceOps::GetCommand with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, GetCommand, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<HitraceOps>();
    EXPECT_STREQ(traceOps->GetCommand().c_str(), "/system/bin/hitrace");
}

/*
 * @tc.name: HasCategoryNormal
 * @tc.desc: test HitraceOps::HasCategory with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, HasCategoryNormal, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<HitraceOps>();
    EXPECT_TRUE(traceOps->HasCategory("ability"));
    EXPECT_TRUE(traceOps->HasCategory("idle"));
}

/*
 * @tc.name: HasCategoryFalse
 * @tc.desc: test HitraceOps::HasCategory with false case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, HasCategoryFalse, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<HitraceOps>();
    EXPECT_FALSE(traceOps->HasCategory("12345"));

    traceOps = std::make_unique<TraceOps>("/system/bin/hitrace", "4567", TraceOps::TraceType::HITRACE);
    EXPECT_FALSE(traceOps->HasCategory("sched"));

    traceOps = std::make_unique<TraceOps>("1234", "hitrace", TraceOps::TraceType::HITRACE);
    EXPECT_FALSE(traceOps->HasCategory("ability"));

    traceOps = std::make_unique<TraceOps>("/system/bin/hitrace", "4567", TraceOps::TraceType::UNKNOW);
    EXPECT_FALSE(traceOps->HasCategory("idle"));

    traceOps = std::make_unique<TraceOps>("/system/bin/hitrace", "hitrace", TraceOps::TraceType::UNKNOW);
    EXPECT_FALSE(traceOps->HasCategory("idle"));
}

/*
 * @tc.name: EnableCategoriesNormal
 * @tc.desc: test HitraceOps::EnableCategories with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, EnableCategoriesNormal, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<HitraceOps>();
    std::vector<std::string> categories;
    categories.push_back("sched");
    EXPECT_TRUE(traceOps->EnableCategories(categories));
    categories.push_back("ability");
    EXPECT_TRUE(traceOps->EnableCategories(categories));
    categories.push_back("idle");
    EXPECT_TRUE(traceOps->EnableCategories(categories));
}

/*
 * @tc.name: EnableCategoriesFalse
 * @tc.desc: test HitraceOps::EnableCategories with false case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, EnableCategoriesFalse, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<HitraceOps>();
    std::vector<std::string> categories;
    EXPECT_FALSE(traceOps->EnableCategories(categories));

    categories.push_back("sched");
    traceOps = std::make_unique<TraceOps>("/system/bin/hitrace", "4567", TraceOps::TraceType::HITRACE);
    EXPECT_FALSE(traceOps->EnableCategories(categories));

    categories.push_back("ability");
    traceOps = std::make_unique<TraceOps>("1234", "hitrace", TraceOps::TraceType::HITRACE);
    EXPECT_FALSE(traceOps->EnableCategories(categories));

    categories.push_back("idle");
    traceOps = std::make_unique<TraceOps>("/system/bin/hitrace", "4567", TraceOps::TraceType::UNKNOW);
    EXPECT_FALSE(traceOps->EnableCategories(categories));

    categories.push_back("1234");
    traceOps = std::make_unique<TraceOps>("/system/bin/hitrace", "hitrace", TraceOps::TraceType::UNKNOW);
    EXPECT_FALSE(traceOps->EnableCategories(categories));
}

/*
 * @tc.name: DisableCategoriesNormal
 * @tc.desc: test HitraceOps::DisableCategories with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, DisableCategoriesNormal, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<HitraceOps>();
    EXPECT_TRUE(traceOps->DisableCategories());
}

/*
 * @tc.name: DisableCategoriesFalse
 * @tc.desc: test HitraceOps::DisableCategories with false case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, DisableCategoriesFalse, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<TraceOps>("1234", "hitrace", TraceOps::TraceType::HITRACE);
    EXPECT_FALSE(traceOps->DisableCategories());

    traceOps = std::make_unique<TraceOps>("/system/bin/hitrace", "4567", TraceOps::TraceType::UNKNOW);
    EXPECT_FALSE(traceOps->DisableCategories());
}

/*
 * @tc.name: IsSupportedNormal
 * @tc.desc: test HitraceOps::IsSupported with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, IsSupportedNormal, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<HitraceOps>();
    EXPECT_TRUE(traceOps->IsSupported());
}

/*
 * @tc.name: IsSupportedFalse
 * @tc.desc: test HitraceOps::IsSupported with false case.
 * @tc.type: FUNC
 */
HWTEST_F(TraceOpsTest, IsSupportedFalse, TestSize.Level1)
{
    std::unique_ptr<TraceOps> traceOps = std::make_unique<TraceOps>("1234", "hitrace", TraceOps::TraceType::HITRACE);
    EXPECT_FALSE(traceOps->IsSupported());

    traceOps = std::make_unique<TraceOps>("/system/bin/hitrace", "4567", TraceOps::TraceType::UNKNOW);
    EXPECT_TRUE(traceOps->IsSupported());

    traceOps = std::make_unique<TraceOps>("1234", "4567", TraceOps::TraceType::HITRACE);
    EXPECT_FALSE(traceOps->IsSupported());

    traceOps = std::make_unique<TraceOps>("1234", "4567", TraceOps::TraceType::UNKNOW);
    EXPECT_FALSE(traceOps->IsSupported());
}
} // namespace
