/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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

#include "native_daemon_service.h"

#include <thread>

#include "command_poller.h"
#include "hook_manager.h"
#include "logging.h"

namespace {
constexpr int SLEEP_ONE_SECOND = 1000;
}

namespace OHOS::Developtools::NativeDaemon {
int RunDaemonServiceLoop()
{
    auto hookManager = std::make_shared<HookManager>();
    auto commandPoller = std::make_shared<CommandPoller>(hookManager);
    if (!commandPoller->OnConnect()) {
        PROFILER_LOG_INFO(LOG_CORE, "connect FAILED");
        return 1;
    }
    hookManager->SetCommandPoller(commandPoller);
    hookManager->RegisterAgentPlugin("nativehook");
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_ONE_SECOND));
    }
}
} // namespace OHOS::Developtools::NativeDaemon
