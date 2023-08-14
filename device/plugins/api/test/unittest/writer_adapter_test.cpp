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
#include "writer_adapter.h"

using namespace testing::ext;

namespace {
class WriterAdapterTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

/**
 * @tc.name: plugin
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(WriterAdapterTest, Writer, TestSize.Level1)
{
    WriterAdapter writerAdapter;
    EXPECT_EQ(writerAdapter.GetWriter(), nullptr);
    EXPECT_EQ(writerAdapter.GetStruct(), &writerAdapter.writerStruct_);
}

/**
 * @tc.name: plugin
 * @tc.desc: Write data to shared memory through writer.
 * @tc.type: FUNC
 */
HWTEST_F(WriterAdapterTest, Func, TestSize.Level1)
{
    WriterAdapter writerAdapter;
    EXPECT_EQ(writerAdapter.WriteFunc(nullptr, nullptr, 0), 0);
    EXPECT_FALSE(writerAdapter.FlushFunc(nullptr));
}
} // namespace