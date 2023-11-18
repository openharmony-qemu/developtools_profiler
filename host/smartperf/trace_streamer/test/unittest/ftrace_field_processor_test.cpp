/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "ftrace_field_processor.h"
#include "trace_streamer_selector.h"
#include "securec.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;

namespace SysTuning {
namespace TraceStreamer {
constexpr int CYCLE_MAX = 20;
class FtraceFieldProcessorTest : public ::testing::Test {
public:
    void SetUp() {}
    void TearDown() const {}

    void AppendIntFiled(int value)
    {
        size_t offset = fieldBuffer_.size();
        fieldBuffer_.resize(fieldBuffer_.size() + sizeof(value));
        if (memcpy_s(&fieldBuffer_[offset], fieldBuffer_.capacity() - offset, &value, sizeof(value))) {
            EXPECT_TRUE(false);
            return;
        }
        FieldFormat fieldFormat = {};
        fieldFormat.offset = offset;
        fieldFormat.size = sizeof(value);
        fieldFormat.isSigned = true;
        fieldFormat.filedType = FIELD_TYPE_INT32;
        fieldFormats_.push_back(fieldFormat);
    }

    void AppendString(const std::string& str)
    {
        size_t offset = fieldBuffer_.size();
        fieldBuffer_.resize(fieldBuffer_.size() + str.size());
        if (memcpy_s(&fieldBuffer_[offset], fieldBuffer_.capacity() - offset, &str[0], str.size())) {
            EXPECT_TRUE(false);
            return;
        }
        FieldFormat fieldFormat = {};
        fieldFormat.offset = offset;
        fieldFormat.size = str.size();
        fieldFormat.isSigned = true;
        fieldFormat.filedType = FIELD_TYPE_FIXEDCSTRING;
        fieldFormats_.push_back(fieldFormat);
    }

public:
    std::vector<uint8_t> fieldBuffer_;
    std::vector<FieldFormat> fieldFormats_;
};
/*
 * @tc.name: HandleIntField1
 * @tc.desc: test FtraceFieldProcessor::HandleIntField with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFieldProcessorTest, HandleIntField1, TestSize.Level1)
{
    for (int i = 0; i < CYCLE_MAX; i++) {
        int expectVal = i + 1;
        AppendIntFiled(expectVal);
        int actualVal =
            FtraceFieldProcessor::HandleIntField<int>(fieldFormats_[i], fieldBuffer_.data(), fieldBuffer_.size());
        EXPECT_EQ(actualVal, expectVal);
    }
}

/*
 * @tc.name: HandleIntField2
 * @tc.desc: test FtraceFieldProcessor::HandleIntField with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFieldProcessorTest, HandleIntField2, TestSize.Level1)
{
    for (int i = 0; i < CYCLE_MAX; i++) {
        int expectVal = i + 1;
        AppendIntFiled(expectVal);
        int actualVal =
            FtraceFieldProcessor::HandleIntField<int>(fieldFormats_, i, fieldBuffer_.data(), fieldBuffer_.size());
        EXPECT_EQ(actualVal, expectVal);
    }
}

/*
 * @tc.name: HandleStrField1
 * @tc.desc: test FtraceFieldProcessor::HandleIntField with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFieldProcessorTest, HandleStrField1, TestSize.Level1)
{
    for (int i = 0; i < CYCLE_MAX; i++) {
        std::string expectVal = std::to_string(i + 1);
        AppendString(expectVal);
        std::string actualVal =
            FtraceFieldProcessor::HandleStrField(fieldFormats_[i], fieldBuffer_.data(), fieldBuffer_.size());
        EXPECT_EQ(actualVal, expectVal);
    }
}

/*
 * @tc.name: HandleStrField2
 * @tc.desc: test FtraceFieldProcessor::HandleIntField with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceFieldProcessorTest, HandleStrField2, TestSize.Level1)
{
    for (int i = 0; i < CYCLE_MAX; i++) {
        std::string expectVal = std::to_string(i + 1);
        AppendString(expectVal);
        std::string actualVal =
            FtraceFieldProcessor::HandleStrField(fieldFormats_, i, fieldBuffer_.data(), fieldBuffer_.size());
        EXPECT_EQ(actualVal, expectVal);
    }
}
} // namespace TraceStreamer
} // namespace SysTuning