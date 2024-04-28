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

#include "hook_common.h"
#include "hook_manager.h"
#include "logging.h"
#include "native_hook_config.pb.h"
#include "plugin_service_types.pb.h"
#include <cstdlib>


using namespace OHOS::Developtools::NativeDaemon;
namespace OHOS {
namespace Developtools {
namespace Profiler {
namespace Hook {
const int PAGE_BYTES = 4096;
std::shared_ptr<HookManager> g_hookManager;
NativeHookConfig g_nativeConfig;

void SetNativeHookConfig(const HookData& hookData)
{
    g_nativeConfig.set_fp_unwind(hookData.fpUnwind);
    g_nativeConfig.set_smb_pages(hookData.smbSize / PAGE_BYTES);
    g_nativeConfig.set_max_stack_depth(hookData.maxStackDepth);
    g_nativeConfig.set_filter_size(hookData.filterSize);
    g_nativeConfig.set_save_file(true);
    g_nativeConfig.set_file_name(hookData.fileName);
    g_nativeConfig.set_statistics_interval(hookData.statisticsInterval);
    g_nativeConfig.set_offline_symbolization(hookData.offlineSymbolization);
    g_nativeConfig.set_callframe_compress(hookData.callframeCompress);
    g_nativeConfig.set_string_compressed(hookData.stringCompressed);
    g_nativeConfig.set_clock("realtime");
    g_nativeConfig.set_record_accurately(true);
    g_nativeConfig.set_startup_mode(hookData.startupMode);
    g_nativeConfig.set_process_name(hookData.processName);
    g_nativeConfig.set_sample_interval(hookData.sampleInterval);
    g_nativeConfig.set_response_library_mode(hookData.responseLibraryMode);
    g_nativeConfig.set_js_stack_report(hookData.jsStackReport);
    g_nativeConfig.set_max_js_stack_depth(hookData.maxJsStackdepth);
    g_nativeConfig.set_filter_napi_name(hookData.filterNapiName);
    g_nativeConfig.set_malloc_free_matching_interval(hookData.mallocFreeMatchingInterval);
    for (const std::string& pid: hookData.pids) {
        g_nativeConfig.add_expand_pids(atoi(pid.data()));
    }
    // statistical reporting must be callframe compressed and accurate.
    if (g_nativeConfig.statistics_interval() > 0 ||
        g_nativeConfig.malloc_free_matching_interval() > 0) {
        g_nativeConfig.set_callframe_compress(true);
        g_nativeConfig.set_record_accurately(true);
    }
    // offlinem symbolization, callframe must be compressed
    if (g_nativeConfig.offline_symbolization()) {
        g_nativeConfig.set_callframe_compress(true);
    }

    // callframe compressed, string must be compressed.
    if (g_nativeConfig.callframe_compress()) {
        g_nativeConfig.set_string_compressed(true);
    }

    if (g_nativeConfig.string_compressed() || hookData.rawString ||
        g_nativeConfig.response_library_mode() || g_nativeConfig.js_stack_report() > 0) {
        g_hookManager->SethookStandalone(true);
    }
    PROFILER_LOG_INFO(LOG_CORE, "hookData config = %s", hookData.ToString().c_str());
}

bool StartHook(HookData& hookData)
{
    g_hookManager = std::make_shared<HookManager>();
    std::vector<ProfilerPluginConfig> config;
#if defined(__arm__)
    hookData.fpUnwind = false;
    hookData.responseLibraryMode = false;
#endif
    SetNativeHookConfig(hookData);
    g_hookManager->SetHookConfig(g_nativeConfig);
    CHECK_TRUE(g_hookManager->CreatePluginSession(config), false, "StartHook CreatePluginSession invalid");
    g_hookManager->StartPluginSession();
    return true;
}

void EndHook()
{
    std::vector<uint32_t> pluginIds;
    g_hookManager->StopPluginSession(pluginIds);
    g_hookManager->DestroyPluginSession(pluginIds);
}
} // namespace Hook
} // namespace Profiler
} // namespace Developtools
} // namespace OHOS