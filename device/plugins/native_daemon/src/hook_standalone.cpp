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
#include "hook_standalone.h"

#include <csignal>
#include <functional>
#include <linux/types.h>
#include <sys/mman.h>

#include "common.h"
#include "epoll_event_poller.h"
#include "event_notifier.h"
#include "hook_common.h"
#include "hook_manager.h"
#include "hook_service.h"
#include "logging.h"
#include "native_hook_config.pb.h"
#include "share_memory_allocator.h"
#include "stack_data_repeater.h"
#include "stack_preprocess.h"
#include "virtual_runtime.h"
#include "hook_manager.h"
#include "utilities.h"

using namespace OHOS::Developtools::NativeDaemon;
namespace OHOS {
namespace Developtools {
namespace Profiler {
namespace Hook {
const int DEFAULT_EVENT_POLLING_INTERVAL = 5000;
const int MOVE_BIT_16 = 16;
const int MOVE_BIT_32 = 32;
std::string g_smbName = "hooknativesmb";
std::unique_ptr<EpollEventPoller> g_eventPoller_;
std::shared_ptr<ShareMemoryBlock> g_shareMemoryBlock;
std::shared_ptr<EventNotifier> g_eventNotifier;
std::shared_ptr<HookService> g_hookService;
std::shared_ptr<HookManager> g_hookManager;
std::shared_ptr<StackDataRepeater> g_stackDataRepeater;
std::shared_ptr<StackPreprocess> g_stackpreprocess;
uint32_t g_maxStackDepth;
bool g_unwindErrorFlag = false;
bool g_fpUnwind = false;
uint32_t g_statisticsinterval;
std::unique_ptr<FILE, decltype(&fclose)> g_fpHookFile(nullptr, nullptr);

static void SignalHandl(int signo)
{
    HILOG_ERROR(LOG_CORE, "recv signal %d", signo);
    g_stackpreprocess->StopTakeResults();
    if (g_statisticsinterval > 0) {
        g_stackpreprocess->FlushRecordStatistics();
    }
    exit(-1);
}

void GetClientConfig(const HookData& hookData, ClientConfig& clientConfig)
{
    clientConfig.shareMemroySize = hookData.smbSize;
    clientConfig.filterSize = hookData.filterSize;
    clientConfig.maxStackDepth = hookData.maxStackDepth;
    clientConfig.mallocDisable = hookData.mallocDisable;
    clientConfig.mmapDisable = hookData.mmapDisable;
    clientConfig.freeStackData = hookData.freemsgstack;
    clientConfig.munmapStackData = hookData.munmapmsgstack;
    clientConfig.fpunwind = hookData.fpUnwind;
    clientConfig.isBlocked = true;
}

bool StartHook(HookData& hookData)
{
    // create smb and eventNotifier
    g_shareMemoryBlock = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(g_smbName, hookData.smbSize);
    g_eventNotifier = EventNotifier::Create(0, EventNotifier::NONBLOCK);
    g_stackDataRepeater = std::make_shared<StackDataRepeater>(STACK_DATA_SIZE);
    CHECK_TRUE(g_stackDataRepeater != nullptr, false, "Create StackDataRepeater FAIL");
    g_hookManager = std::make_shared<HookManager>(g_shareMemoryBlock, g_eventNotifier, g_stackDataRepeater);
    // start event poller task
    g_eventPoller_ = std::make_unique<EpollEventPoller>(DEFAULT_EVENT_POLLING_INTERVAL);
    g_eventPoller_->Init();
    g_eventPoller_->Start();
    g_eventPoller_->AddFileDescriptor(g_eventNotifier->GetFd(),
                                      std::bind(&HookManager::ReadShareMemory, g_hookManager.get()));

    HILOG_INFO(LOG_CORE, "hookservice smbFd = %d, eventFd = %d\n", g_shareMemoryBlock->GetfileDescriptor(),
               g_eventNotifier->GetFd());

#if defined(__arm__)
    hookData.fpUnwind = false;
#endif
    if (hookData.maxStackDepth < DLOPEN_MIN_UNWIND_DEPTH) {
        // set default max depth
        hookData.maxStackDepth = DLOPEN_MIN_UNWIND_DEPTH;
    }

    HILOG_INFO(LOG_CORE, "hookservice smbFd = %d, eventFd = %d\n",
        g_shareMemoryBlock->GetfileDescriptor(), g_eventNotifier->GetFd());

    ClientConfig clientConfig;
    GetClientConfig(hookData, clientConfig);
    std::string clientConfigStr = clientConfig.ToString();
    HILOG_INFO(LOG_CORE, "send hook client config:%s\n", clientConfigStr.c_str());
    g_hookService = std::make_shared<HookService>(g_shareMemoryBlock->GetfileDescriptor(),
        g_eventNotifier->GetFd(), hookData.pid, hookData.processName, clientConfig);

    g_maxStackDepth = hookData.maxStackDepth;
    g_fpUnwind = hookData.fpUnwind;
    g_statisticsinterval = hookData.statisticsInterval;

    NativeHookConfig nativeConfig;
    nativeConfig.set_fp_unwind(hookData.fpUnwind);
    nativeConfig.set_max_stack_depth(hookData.maxStackDepth);
    nativeConfig.set_filter_size(hookData.filterSize);
    nativeConfig.set_save_file(true);
    nativeConfig.set_file_name(hookData.fileName);
    nativeConfig.set_statistics_interval(hookData.statisticsInterval);
    nativeConfig.set_offline_symbolization(hookData.offlineSymbolization);
    nativeConfig.set_callframe_compress(hookData.callframeCompress);
    nativeConfig.set_string_compressed(hookData.stringCompressed);
#if defined(__arm__)
    nativeConfig.set_fp_unwind(false); // if OS is 32-bit,set fp_unwind false.
#endif
    bool isHookStandaloneSerialize = false;
    // statistical reporting must be callframe compressed and accurate.
    if (nativeConfig.statistics_interval() > 0) {
        nativeConfig.set_callframe_compress(true);
        nativeConfig.set_record_accurately(true);
    }
    // offlinem symbolization, callframe must be compressed
    if (nativeConfig.offline_symbolization()) {
        nativeConfig.set_callframe_compress(true);
    }

    // callframe compressed, string must be compressed.
    if (nativeConfig.callframe_compress()) {
        nativeConfig.set_string_compressed(true);
    }

    if (nativeConfig.string_compressed() || hookData.rawString) {
        isHookStandaloneSerialize = true;
    }
    HILOG_INFO(LOG_CORE, "fp_unwind = %d", nativeConfig.fp_unwind());
    HILOG_INFO(LOG_CORE, "offline_symbolization = %d", nativeConfig.offline_symbolization());
    HILOG_INFO(LOG_CORE, "callframe_compress; = %d", nativeConfig.callframe_compress());
    HILOG_INFO(LOG_CORE, "string_compressed; = %d", nativeConfig.string_compressed());
    HILOG_INFO(LOG_CORE, "statistics_interval = %u", nativeConfig.statistics_interval());
    HILOG_INFO(LOG_CORE, "isHookStandaloneSerialize = %d", isHookStandaloneSerialize);
    HILOG_INFO(LOG_CORE, "rawString = %d", hookData.rawString);
    clockid_t pluginDataClockId = COMMON::GetClockId("realtime");
    g_stackpreprocess = std::make_shared<StackPreprocess>(g_stackDataRepeater, nativeConfig,
                                                          pluginDataClockId, isHookStandaloneSerialize);
    CHECK_TRUE(g_stackpreprocess != nullptr, false, "Create StackPreprocess FAIL");
    g_hookManager->SetHookConfig(nativeConfig);
    g_stackpreprocess->StartTakeResults();
    signal(SIGINT, SignalHandl);
    return true;
}
} // namespace Hook
} // namespace Profiler
} // namespace Developtools
} // namespace OHOS
