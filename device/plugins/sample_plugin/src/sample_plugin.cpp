/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include "sample_plugin.h"

#include <ctime>

#include "sample_plugin_result.pbencoder.h"
#include "securec.h"

namespace {
using namespace OHOS::Developtools::Profiler;
constexpr int MAX_INT = 10;
constexpr int MAX_DOUBLE = 100;
constexpr int CONST_NUMBER = 1024;
} // namespace

SamplePlugin::SamplePlugin() {}

SamplePlugin::~SamplePlugin() {}

uint64_t SamplePlugin::GetTimeMS()
{
    const int MS_PER_S = 1000;
    const int NS_PER_MS = 1000000;
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    return ts.tv_sec * MS_PER_S + ts.tv_nsec / NS_PER_MS;
}

int SamplePlugin::Start(const uint8_t* configData, uint32_t configSize)
{
    PROFILER_LOG_INFO(LOG_CORE, "%s:config data -->configSize=%d", __func__, configSize);
    CHECK_TRUE(configData != nullptr, -1, "SamplePlugin: param invalid!!!");
    for (uint32_t i = 0; i < configSize; i++) {
        PROFILER_LOG_INFO(LOG_CORE, "%s:configData[%d] = 0x%02x", __func__, i, configData[i]);
    }

    // 反序列化
    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, -1, "%s:parseFromArray failed!", __func__);
    PROFILER_LOG_INFO(LOG_CORE, "%s:pid = %d", __func__, protoConfig_.pid());
    // 插件准备工作

    return 0;
}

int SamplePlugin::ReportOptimize(RandomWriteCtx* randomWrite)
{
    ProtoEncoder::SampleData dataProto(randomWrite);

    // 回填数据
    int intData = CONST_NUMBER % MAX_INT;
    double doubleData = CONST_NUMBER % MAX_DOUBLE;
    dataProto.set_time_ms(GetTimeMS());
    dataProto.set_int_data(intData);
    dataProto.set_double_data(doubleData);

    int msgSize = dataProto.Finish();
    return msgSize;
}

int SamplePlugin::Report(uint8_t* data, uint32_t dataSize)
{
    SampleData dataProto;

    // 回填数据
    int intData = CONST_NUMBER % MAX_INT;
    double doubleData = CONST_NUMBER % MAX_DOUBLE;
    dataProto.set_time_ms(GetTimeMS());
    dataProto.set_int_data(intData);
    dataProto.set_double_data(doubleData);

    uint32_t length = dataProto.ByteSizeLong();
    CHECK_TRUE(length <= dataSize, -length, "%s:Report failed!", __func__);
    // 序列化
    if (dataProto.SerializeToArray(data, length) > 0) {
        return length;
    }
    return 0;
}

int SamplePlugin::Stop()
{
    PROFILER_LOG_INFO(LOG_CORE, "%s:stop success!", __func__);
    return 0;
}