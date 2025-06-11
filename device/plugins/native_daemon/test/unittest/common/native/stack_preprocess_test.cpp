
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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
#include "stack_preprocess.h"
#include "native_hook_config.pb.h"
#include "plugin_module_api.h"

using namespace std;
using namespace testing::ext;
using namespace OHOS::Developtools::NativeDaemon;

namespace {
constexpr uint32_t MAX_MATCH_INTERVAL = 3600;
constexpr uint32_t MAX_MATCH_CNT = 1000;

class StackPreprocessTest : public testing::Test {
public:
static void SetUpTestCase(void);
static void TearDownTestCase(void);
void SetUp();
void TearDown();
};
void StackPreprocessTest::SetUpTestCase(void)
{
}

void StackPreprocessTest::TearDownTestCase(void)
{
}

void StackPreprocessTest::SetUp(void)
{
}

void StackPreprocessTest::TearDown(void)
{
}

/*
@tc.name: StackPreprocessTest001
@tc.desc: test StackPreprocess with overcceeding max matching interval.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StackPreprocessTest001, TestSize.Level1)
{
    NativeHookConfig hookConfig;
    hookConfig.set_malloc_free_matching_interval(MAX_MATCH_INTERVAL + 1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    ASSERT_TRUE(preprocess.hookConfig_.malloc_free_matching_interval() == MAX_MATCH_INTERVAL);
}

/*
@tc.name: StackPreprocessTest002
@tc.desc: test StackPreprocess with overcceeding max matching cnt.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StackPreprocessTest002, TestSize.Level1)
{
    NativeHookConfig hookConfig;
    hookConfig.set_malloc_free_matching_cnt(MAX_MATCH_CNT + 1);
    StackPreprocess preprocess(nullptr, hookConfig, 0);
    ASSERT_TRUE(preprocess.hookConfig_.malloc_free_matching_cnt() == MAX_MATCH_CNT);
}

/*
@tc.name: StackPreprocessTest003
@tc.desc: test StackPreprocess with save_file set to true but no file pointer provided.
@tc.type: FUNC
*/
HWTEST_F(StackPreprocessTest, StackPreprocessTest003, TestSize.Level1)
{
    NativeHookConfig hookConfig;
    hookConfig.set_save_file(true);
    FILE* fpHookData = nullptr;
    StackPreprocess preprocess(nullptr, hookConfig, 0, fpHookData);
    ASSERT_TRUE(hookConfig.save_file() && fpHookData == nullptr);
}
}