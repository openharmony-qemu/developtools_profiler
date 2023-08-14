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

#include "plugin_service.ipc.h"
#include "service_entry.h"

using namespace testing::ext;
namespace {
class ServiceEntryTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

namespace {
const int MS_PER_S = 1000;
const int US_PER_S = 1000000;
const int NS_PER_US = 1000;
const int NS_PER_S = 1000000000;
const int NS_PER_MS = 1000000;
const int SLEEP_TIME = 30000;
} // namespace

/**
 * @tc.name: Service
 * @tc.desc: Server process monitoring.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, AllCase, TestSize.Level1)
{
    ServiceEntry serviceEntry;
    IPluginServiceServer pluginService;
    serviceEntry.StartServer("test_unix_socket_service_entry");
    serviceEntry.RegisterService(pluginService);
    serviceEntry.FindServiceByName(pluginService.serviceName_);

    usleep(SLEEP_TIME);

    IPluginServiceClient pluginClient;
    ASSERT_TRUE(pluginClient.Connect("test_unix_socket_service_entry"));

    usleep(SLEEP_TIME);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the time in milliseconds.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetTimeMS, TestSize.Level1)
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    long t1 = ts.tv_sec * MS_PER_S + ts.tv_nsec / NS_PER_MS;
    long t2 = GetTimeMS();

    ASSERT_TRUE(t2 - t1 >= 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the time in microseconds.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetTimeUS, TestSize.Level1)
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    long t1= ts.tv_sec * US_PER_S + ts.tv_nsec / NS_PER_US;
    long t2 = GetTimeUS();

    ASSERT_TRUE(t2 - t1 >= 0);
}

/**
 * @tc.name: Service
 * @tc.desc: Gets the time in nanoseconds.
 * @tc.type: FUNC
 */
HWTEST_F(ServiceEntryTest, GetTimeNS, TestSize.Level1)
{
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    long t1 = ts.tv_sec * NS_PER_S + ts.tv_nsec;
    long t2 = GetTimeNS();
    ASSERT_TRUE(t2 - t1 >= 0);
}
} // namespace