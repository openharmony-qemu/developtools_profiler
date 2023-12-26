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

#include <thread>
#include <unistd.h>

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>
#include "logging.h"
#include "profiler_data_repeater.h"

using namespace testing::ext;

namespace {
class ProfilerDataRepeaterTest : public ::testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: server
 * @tc.desc: put plugin data.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerDataRepeaterTest, PutPluginData, TestSize.Level1)
{
    const int bufferSize = 10;
    auto dataRepeater = std::make_shared<ProfilerDataRepeater>(bufferSize);
    ASSERT_NE(dataRepeater, nullptr);

    for (int i = 0; i < bufferSize; i++) {
        auto data = std::make_shared<ProfilerPluginData>();
        dataRepeater->PutPluginData(data);
        EXPECT_EQ(dataRepeater->Size(), i + 1);
    }
}

/**
 * @tc.name: server
 * @tc.desc: take plugin data.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerDataRepeaterTest, TakePluginData, TestSize.Level1)
{
    const int bufferSize = 10;
    const int itemCounts = 10000;
    auto inDataRepeater = std::make_shared<ProfilerDataRepeater>(bufferSize);
    ASSERT_NE(inDataRepeater, nullptr);

    auto outDataRepeater = std::make_shared<ProfilerDataRepeater>(bufferSize);
    ASSERT_NE(outDataRepeater, nullptr);

    auto f = [](int x) { return 2 * x + 1; };

    std::thread worker([&]() {
        for (int i = 0; i < itemCounts; i++) {
            auto itemX = inDataRepeater->TakePluginData();

            // compute in worker thread
            int x = itemX ? std::stoi(itemX->data()) : 0;
            int y = f(x);

            auto itemY = std::make_shared<ProfilerPluginData>();
            itemY->set_data(std::to_string(y));
            outDataRepeater->PutPluginData(itemY);
        }
    });

    for (int i = 0; i < itemCounts; i++) {
        int x0 = i;
        auto itemX = std::make_shared<ProfilerPluginData>();
        itemX->set_data(std::to_string(x0));
        inDataRepeater->PutPluginData(itemX);

        auto itemY = outDataRepeater->TakePluginData();
        int y = itemY ? std::stoi(itemY->data()) : 0;

        // redo compute in main thread
        int y0 = f(x0);

        // check results
        EXPECT_EQ(y, y0);
    }
    worker.join();
}

/**
 * @tc.name: server
 * @tc.desc: take plugin profiler data.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerDataRepeaterTest, TakePluginDataVec, TestSize.Level1)
{
    const int itemCounts = 10000;
    const int bufferSize = itemCounts;
    auto inDataRepeater = std::make_shared<ProfilerDataRepeater>(bufferSize);
    ASSERT_NE(inDataRepeater, nullptr);

    auto outDataRepeater = std::make_shared<ProfilerDataRepeater>(bufferSize);
    ASSERT_NE(outDataRepeater, nullptr);

    auto f = [](int x) { return 2 * x + 1; };
    std::thread worker([&]() {
        for (int i = 0; i < itemCounts; i++) {
            auto xData = inDataRepeater->TakePluginData();

            // compute in worker thread
            int x = xData ? std::stoi(xData->data()) : 0;
            int y = f(x);

            auto yData = std::make_shared<ProfilerPluginData>();
            yData->set_data(std::to_string(y));
            outDataRepeater->PutPluginData(yData);
        }
    });

    std::vector<int> yVec;
    for (int i = 0; i < itemCounts; i++) {
        int x0 = i;
        auto xData = std::make_shared<ProfilerPluginData>();
        xData->set_data(std::to_string(x0));
        inDataRepeater->PutPluginData(xData);

        int y0 = f(x0);
        yVec.push_back(y0);
    }
    worker.join();

    std::vector<ProfilerPluginDataPtr> pluginDataVec;
    auto count = outDataRepeater->TakePluginData(pluginDataVec);
    EXPECT_EQ(count, yVec.size());

    for (size_t i = 0; i < pluginDataVec.size(); i++) {
        auto yData = pluginDataVec[i];
        int y = yData ? std::stoi(yData->data()) : 0;
        EXPECT_EQ(y, yVec[i]);
    }
}

/**
 * @tc.name: server
 * @tc.desc: close plugin profiler data.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerDataRepeaterTest, Close, TestSize.Level1)
{
    const int bufferSize = 10;
    auto dataRepeater = std::make_shared<ProfilerDataRepeater>(bufferSize);
    ASSERT_NE(dataRepeater, nullptr);

    dataRepeater->Close();
}

/**
 * @tc.name: server
 * @tc.desc: reset plugin profiler data.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerDataRepeaterTest, Reset, TestSize.Level1)
{
    const int bufferSize = 10;
    auto dataRepeater = std::make_shared<ProfilerDataRepeater>(bufferSize);
    ASSERT_NE(dataRepeater, nullptr);

    dataRepeater->Reset();
}

/**
 * @tc.name: server
 * @tc.desc: close plugin profiler data.
 * @tc.type: FUNC
 */
HWTEST_F(ProfilerDataRepeaterTest, CloseWithFastProducer, TestSize.Level1)
{
    const int bufferSize = 10;
    const int numProducts = 20;
    auto dataRepeater = std::make_shared<ProfilerDataRepeater>(bufferSize);
    ASSERT_NE(dataRepeater, nullptr);

    std::thread producer([=]() {
        for (int i = 0; i < numProducts; i++) {
            auto data = std::make_shared<ProfilerPluginData>();
            if (!dataRepeater->PutPluginData(data)) {
                HILOG_DEBUG(LOG_CORE, "put data %d FAILED!", i);
                break;
            }
        }
    });

    const int consumeDelayUs = 50 * 1000;
    std::thread consumer([=]() {
        for (int i = 0; i < numProducts; i++) {
            auto data = dataRepeater->TakePluginData();
            if (!data) {
                HILOG_DEBUG(LOG_CORE, "get data %d FAILED!", i);
                break;
            }
            usleep(consumeDelayUs);
        }
    });

    usleep(consumeDelayUs * 3);
    dataRepeater->Close();
    producer.join();
    consumer.join();
}
} // namespace