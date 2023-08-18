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

#include "ftrace_field_parser.h"
#include "securec.h"

using FTRACE_NS::FIELD_TYPE_FIXEDCSTRING;
using FTRACE_NS::FIELD_TYPE_INT32;
using FTRACE_NS::FieldFormat;
using FTRACE_NS::FtraceFieldParser;
using testing::ext::TestSize;

namespace {
constexpr int NROUNDS = 20;

class FtraceFieldParserTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buffer_;
    std::vector<FieldFormat> formats_;

    void AppendInt(int value)
    {
        size_t offset = buffer_.size();
        buffer_.resize(buffer_.size() + sizeof(value));
        if (memcpy_s(&buffer_[offset], buffer_.capacity() - offset, &value, sizeof(value))) {
            EXPECT_TRUE(false);
            return;
        }
        FieldFormat format = {};
        format.offset = offset;
        format.size = sizeof(value);
        format.isSigned = true;
        format.filedType = FIELD_TYPE_INT32;
        formats_.push_back(format);
    }

    void AppendString(const std::string& str)
    {
        size_t offset = buffer_.size();
        buffer_.resize(buffer_.size() + str.size());
        if (memcpy_s(&buffer_[offset], buffer_.capacity() - offset, &str[0], str.size())) {
            EXPECT_TRUE(false);
            return;
        }
        FieldFormat format = {};
        format.offset = offset;
        format.size = str.size();
        format.isSigned = true;
        format.filedType = FIELD_TYPE_FIXEDCSTRING;
        formats_.push_back(format);
    }

    void SetUp() override {}

    void TearDown() override {}
};

/*
 * @tc.name: ParseIntField1
 * @tc.desc: test FtraceFieldParser::ParseIntField with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFieldParserTest, ParseIntField1, TestSize.Level1)
{
    for (int i = 0; i < NROUNDS; i++) {
        int expect = i + 1;
        AppendInt(expect);
        int actual = FtraceFieldParser::ParseIntField<int>(formats_[i], buffer_.data(), buffer_.size());
        EXPECT_EQ(actual, expect);
    }
}

/*
 * @tc.name: ParseIntField2
 * @tc.desc: test FtraceFieldParser::ParseIntField with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFieldParserTest, ParseIntField2, TestSize.Level1)
{
    for (int i = 0; i < NROUNDS; i++) {
        int expect = i + 1;
        AppendInt(expect);
        int actual = FtraceFieldParser::ParseIntField<int>(formats_, i, buffer_.data(), buffer_.size());
        EXPECT_EQ(actual, expect);
    }
}

/*
 * @tc.name: ParseStrField1
 * @tc.desc: test FtraceFieldParser::ParseIntField with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFieldParserTest, ParseStrField1, TestSize.Level1)
{
    for (int i = 0; i < NROUNDS; i++) {
        std::string expect = std::to_string(i + 1);
        AppendString(expect);
        std::string actual = FtraceFieldParser::ParseStrField(formats_[i], buffer_.data(), buffer_.size());
        EXPECT_EQ(actual, expect);
    }
}

/*
 * @tc.name: ParseStrField2
 * @tc.desc: test FtraceFieldParser::ParseIntField with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFieldParserTest, ParseStrField2, TestSize.Level1)
{
    for (int i = 0; i < NROUNDS; i++) {
        std::string expect = std::to_string(i + 1);
        AppendString(expect);
        std::string actual = FtraceFieldParser::ParseStrField(formats_, i, buffer_.data(), buffer_.size());
        EXPECT_EQ(actual, expect);
    }
}
} // namespace
