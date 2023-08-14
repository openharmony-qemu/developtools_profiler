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
#include <fstream>
#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include <random>
#include <string>

#include "ftrace_data_reader.h"

using FTRACE_NS::FtraceDataReader;
using testing::ext::TestSize;

namespace {
constexpr char RANDOM_CHAR_TABLE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
constexpr long RANDOM_CHAR_NUMBER = std::size(RANDOM_CHAR_TABLE) - 1;
constexpr int RAW_DATA_SIZE = 1024 * 1024;
#ifndef PAGE_SIZE
constexpr uint32_t PAGE_SIZE = 4096;
#endif

class FtraceDataReaderTest : public ::testing::Test {
protected:
    std::mt19937 gen_;
    std::string path_ = "raw_data.bin";
    long dataSize_ = RAW_DATA_SIZE;

    void SetUp() override
    {
        std::ofstream fout(path_);
        std::string data = RandomString(dataSize_);
        fout.write(data.data(), data.size());
    }

    void TearDown() override
    {
        unlink(path_.c_str());
    }

    int RandomInt(int a, int b)
    {
        std::uniform_int_distribution<int> distrib(a, b);
        return distrib(gen_);
    }

    char RandomChar()
    {
        return RANDOM_CHAR_TABLE[RandomInt(0, RANDOM_CHAR_NUMBER - 1)];
    }

    std::string RandomString(int len)
    {
        std::string str;
        str.reserve(len);
        for (int i = 0; i < len; i++) {
            str.push_back(RandomChar());
        }
        return str;
    }
};

/*
 * @tc.name: ReadNormal
 * @tc.desc: test FtraceDataReader::Read with normal case.
 * @tc.type: FUNC
 */
HWTEST_F(FtraceDataReaderTest, ReadNormal, TestSize.Level1)
{
    FtraceDataReader reader(path_);
    std::vector<uint8_t> zeros(PAGE_SIZE, 0);
    std::vector<uint8_t> buffer(PAGE_SIZE, 0);

    for (long i = 0; i < dataSize_; i += buffer.size()) {
        long bufferSize = static_cast<long>(buffer.size());
        EXPECT_EQ(reader.Read(buffer.data(), buffer.size()), bufferSize);
        EXPECT_NE(buffer, zeros);
    }
}
} // namespace