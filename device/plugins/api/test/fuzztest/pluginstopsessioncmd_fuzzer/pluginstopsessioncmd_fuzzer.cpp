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

#include "pluginstopsessioncmd_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <thread>

#include "command_poller.h"
#include "plugin_manager.h"
#include "plugin_service.ipc.h"
#include "socket_context.h"

namespace OHOS {
bool StopSessionTest(const uint8_t* data, size_t size)
{
#ifdef ASAN_MODE
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
#endif
    auto pluginManage = std::make_shared<PluginManager>();
    auto commandPoller = std::make_shared<CommandPoller>(pluginManage);
    pluginManage->SetCommandPoller(commandPoller);

    StopSessionCmd successCmd;

    if (!successCmd.ParseFromArray(data, size)) {
        return true;
    }
    commandPoller->OnStopSessionCmd(successCmd);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::StopSessionTest(data, size);
    return 0;
}
