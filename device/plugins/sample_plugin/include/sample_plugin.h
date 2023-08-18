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

#ifndef SAMPLE_PLUGIN_H
#define SAMPLE_PLUGIN_H

#include "logging.h"
#include "sample_plugin_config.pb.h"
#include "sample_plugin_result.pb.h"

class SamplePlugin {
public:
    SamplePlugin();
    ~SamplePlugin();
    int Start(const uint8_t* configData, uint32_t configSize);
    int Report(uint8_t* configData, uint32_t configSize);
    int Stop();
    uint64_t GetTimeMS();

private:
    SampleConfig protoConfig_;
};

#endif // SAMPLE_PLUGIN_H