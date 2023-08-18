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

#include <signal.h>
#include <sys/file.h>
#include <unistd.h>

#include "common.h"
#include "command_poller.h"
#include "logging.h"
#include "plugin_manager.h"
#include "plugin_service_types.pb.h"
#include "plugin_watcher.h"
#include "schedule_task_manager.h"
#include "writer_adapter.h"

namespace {
#if defined(__i386__) || defined(__x86_64__)
const char DEFAULT_PLUGIN_PATH[] = "./";
#else
const char DEFAULT_PLUGIN_PATH[] = "/data/local/tmp/";
#endif

const int SLEEP_ONE_SECOND = 1000;
#if defined(__LP64__)
const char DEFAULT_LIB_PATH[] = "/system/lib64/";
#else
const char DEFAULT_LIB_PATH[] = "/system/lib/";
#endif

std::vector<std::string> presetPluginVec = {
    "libcpudataplugin.z.so",
    "libdiskiodataplugin.z.so",
    "libftrace_plugin.z.so",
    "libhidumpplugin.z.so",
    "libhiebpfplugin.z.so",
    "libhilogplugin.z.so",
    "libhiperfplugin.z.so",
    "libhisyseventplugin.z.so",
    "libmemdataplugin.z.so",
    "libnetworkplugin.z.so",
    "libprocessplugin.z.so",
    "libarktsplugin.z.so",
};

volatile sig_atomic_t g_isRunning = true;
void SignalSigtermHandler(int sig)
{
    HILOG_INFO(LOG_CORE, "hiprofiler plugin receive sigterm signal!");
    g_isRunning = false;
}
} // namespace

int main(int argc, char* argv[])
{
    int lockFileFd = -1;
    if (COMMON::IsProcessRunning(lockFileFd)) { // process is running
        return 0;
    }
    signal(SIGTERM, SignalSigtermHandler);
    const int connectRetrySeconds = 3;
    std::string pluginDir(DEFAULT_PLUGIN_PATH);
    if (argv[1] != nullptr) {
        HILOG_DEBUG(LOG_CORE, "%s:pluginDir = %s", __func__, argv[1]);
        pluginDir = argv[1];
    }

    auto pluginManager = std::make_shared<PluginManager>();
    CHECK_NOTNULL(pluginManager, 1, "create PluginManager FAILED!");

    auto commandPoller = std::make_shared<CommandPoller>(pluginManager);
    CHECK_NOTNULL(commandPoller, 1, "create CommandPoller FAILED!");

    while (true) {
        if (commandPoller->OnConnect()) {
            break;
        }
        HILOG_DEBUG(LOG_CORE, "%s:connect failed, try again", __func__);
        sleep(connectRetrySeconds);
    }
    pluginManager->SetCommandPoller(commandPoller);

    // add preset plugin
    for (size_t i = 0; i < presetPluginVec.size(); i++) {
        const std::string pluginPath = DEFAULT_LIB_PATH + presetPluginVec[i];
        if (pluginManager->AddPlugin(pluginPath)) {
            HILOG_INFO(LOG_CORE, "add preset plugin %s success!", pluginPath.c_str());
        } else {
            HILOG_INFO(LOG_CORE, "add preset plugin %s failed!", pluginPath.c_str());
        }
    }

    while (g_isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_ONE_SECOND));
    }
    if (flock(lockFileFd, LOCK_UN) == -1) {
        HILOG_INFO(LOG_CORE, "release lockfile failed!");
    }
    close(lockFileFd);
    pluginManager->StopAllPluginSession();
    return 0;
}
