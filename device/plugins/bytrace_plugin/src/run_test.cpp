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

#include "bytrace_module.h"
#include "bytrace_plugin_config.pb.h"
#include "logging.h"

namespace {
constexpr uint32_t TRACE_TIME = 8;
constexpr uint32_t WAIT_TIME = 9;
constexpr uint32_t BUFFE_SIZE = 1024;
}

int main()
{
    BytracePluginConfig config;
    config.set_outfile_name("/data/local/tmp/bytrace.txt");
    config.set_clock("boot");
    config.set_time(TRACE_TIME);
    config.set_buffe_size(BUFFE_SIZE);
    config.add_categories("sched");
    config.add_categories("freq");
    config.add_categories("idle");
    config.add_categories("workq");

    std::vector<char> buffer(config.ByteSizeLong());
    CHECK_TRUE(config.SerializeToArray(buffer.data(), buffer.size()), 0, "Serialize config FAILED!");
    CHECK_TRUE(BytracePluginSessionStart(reinterpret_cast<uint8_t*>(buffer.data()), buffer.size()) == 0, 0,
               "call start callback FAILED!");
    sleep(WAIT_TIME);
    BytracePluginSessionStop();
    return 0;
}
