/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_H
#define NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_H

#include <gtest/gtest.h>
#include <memory>

#include "init_param.h"
#include "native_memory_profiler_sa_service.h"
#include "utilities.h"

using namespace testing::ext;

namespace {
const std::string TEST_HAP_NAME = "cn.openharmony.rebound_project";
const std::string NATIVE_PARAM = "hiviewdfx.hiprofiler.native_memoryd.start";
constexpr uint32_t TEST_DURATION = 10;
constexpr uint32_t TEST_SHARE_MEMORY_SIZE = 4096;
}

class NativeMemoryProfilerSaServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void)
    {
        SystemSetParameter(NATIVE_PARAM.c_str(), "0");
    }
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: NativeMemoryProfilerSaServiceTest001
 * @tc.desc: Test the service interface
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NativeMemoryProfilerSaServiceTest001, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    int32_t pid = GetProcessPid(TEST_HAP_NAME);
    EXPECT_NE(pid, 0);
    config->pid_ = pid;
    config->duration_ = TEST_DURATION;
    config->filePath_ = "/data/local/tmp/native_hook_test.htrace";
    config->offlineSymbolization_ = true;
    config->shareMemorySize_ = TEST_SHARE_MEMORY_SIZE;

    NativeMemoryProfilerSaService service;
    EXPECT_EQ(service.Start(config), RET_OK);
    sleep(3);
    EXPECT_EQ(service.Stop(config->pid_), RET_OK);

    uint32_t fd = open("/data/local/tmp/test_dump_data.htrace",
        O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    EXPECT_GT(fd, 0);
    EXPECT_EQ(service.DumpData(fd, config), RET_OK);
    sleep(2);
    EXPECT_EQ(service.Stop(config->pid_), RET_OK);
}

/**
 * @tc.name: NativeMemoryProfilerSaServiceTest
 * @tc.desc: Test the service interface with false case.
 * @tc.type: FUNC
 */
HWTEST_F(NativeMemoryProfilerSaServiceTest, NativeMemoryProfilerSaServiceTest002, TestSize.Level3)
{
    using namespace OHOS::Developtools::NativeDaemon;
    std::shared_ptr<NativeMemoryProfilerSaConfig> config = std::make_shared<NativeMemoryProfilerSaConfig>();
    EXPECT_NE(config, nullptr);

    int32_t pid = GetProcessPid(TEST_HAP_NAME);
    EXPECT_NE(pid, 0);
    config->pid_ = pid;
    config->duration_ = TEST_DURATION;
    config->filePath_ = "/data/local/tmp/native_hook_test.htrace";
    config->offlineSymbolization_ = true;
    config->shareMemorySize_ = 0;

    NativeMemoryProfilerSaService service;
    EXPECT_EQ(service.Start(config), RET_ERR);

    config->pid_ = 0;
    config->processName_ = "";
    config->shareMemorySize_ = TEST_SHARE_MEMORY_SIZE;
    EXPECT_EQ(service.Start(config), RET_ERR);
}

#endif // NATIVE_MEMORY_PROFILER_SA_SERVICE_TEST_H