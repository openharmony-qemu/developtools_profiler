/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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

#include "hook_manager.h"

#include <limits>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include "command_poller.h"
#include "common.h"
#include "epoll_event_poller.h"
#include "event_notifier.h"
#include "hook_common.h"
#include "hook_service.h"
#include "init_param.h"
#include "logging.h"
#include "plugin_service_types.pb.h"
#include "share_memory_allocator.h"
#include "utilities.h"
#include "virtual_runtime.h"
#include "native_memory_profiler_sa_service.h"
#include "hook_record.h"
#include "hook_record_factory.h"

namespace OHOS::Developtools::NativeDaemon {
namespace {
constexpr int DEFAULT_EVENT_POLLING_INTERVAL = 5000;
constexpr uint32_t PAGE_BYTES = 4096;
std::shared_ptr<Writer> g_buffWriter;
const std::string STARTUP = "startup:";
const std::string PARAM_NAME = "libc.hook_mode";
const std::string PRODUCT_CONFIG_PATH = "/sys_prod/etc/hiview/hiprofiler/hiprofiler_cfg.json";
constexpr int SIGNAL_START_HOOK = 36;
const std::string VERSION = "1.02";
constexpr int32_t RESPONSE_MAX_PID_COUNT = 8;
constexpr int32_t ONLY_NMD_TYPE = 2;
constexpr int32_t SIMP_NMD = 3;
constexpr int DOUBLE = 2;
static thread_local std::array<std::shared_ptr<HookRecord>, CACHE_ARRAY_SIZE> g_rawDataArray = {};
static thread_local uint32_t g_rawStackCount = 0;
}

HookManager::~HookManager()
{
    hookService_ = nullptr;
    for (const auto& item : hookCtx_) {
        for (size_t i = 0; i < item->shareMemoryBlockList.size(); ++i) {
            item->shareMemoryBlockList[i] = nullptr;
        }
        for (size_t i = 0; i < item->eventPollerList.size(); ++i) {
            item->eventPollerList[i] = nullptr;
        }
        if (item->stackPreprocess != nullptr) {
            item->stackPreprocess = nullptr;
        }
        if (item->stackData != nullptr) {
            item->stackData = nullptr;
        }
    }
}

bool HookManager::CheckProcess()
{
    if (hookConfig_.pid() > 0) {
        hookConfig_.add_expand_pids(hookConfig_.pid());
    }
    std::set<int32_t> pidCache;
    for (const auto& pid : hookConfig_.expand_pids()) {
        if (pid > 0) {
            struct stat statBuf;
            std::string pidPath = "/proc/" + std::to_string(pid) + "/status";
            if (stat(pidPath.c_str(), &statBuf) != 0) {
                PROFILER_LOG_ERROR(LOG_CORE, "%s: hook process does not exist", __func__);
                return false;
            } else {
                auto [iter, isExist] = pidCache.emplace(pid);
                if (isExist) {
                    hookCtx_.emplace_back(std::make_shared<HookManagerCtx>(pid));
                    PROFILER_LOG_INFO(LOG_CORE, "hook context: pid: %d", pid);
                }
                continue;
            }
        }
    }

    if (!hookConfig_.process_name().empty() && !CheckProcessName()) {
        return false;
    }

    if (hookConfig_.response_library_mode()) {
        if (hookCtx_.size() > RESPONSE_MAX_PID_COUNT) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: The maximum allowed is to set %d PIDs.",
                               __func__, RESPONSE_MAX_PID_COUNT);
            return false;
        }
    } else {
        auto maxPidCount = GetValueFromJsonFile(PRODUCT_CONFIG_PATH, "hiprofiler_hook_process_count");
        maxProcessCount_ = maxPidCount > 0 ? maxPidCount : maxProcessCount_;
        if (hookCtx_.size() > static_cast<size_t>(maxProcessCount_)) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: The maximum allowed is to set %d PIDs.", __func__, maxProcessCount_);
            return false;
        }
    }

    if (hookCtx_.size() > 1) {
        isProtobufSerialize_ = true;
    }
    return true;
}

bool HookManager::CheckProcessName()
{
    int pidValue = -1;
    const std::string processName = hookConfig_.process_name();
    bool isExist = COMMON::IsProcessExist(processName, pidValue);
    if (hookConfig_.startup_mode() || !isExist) {
        PROFILER_LOG_INFO(LOG_CORE, "Wait process %s start or restart, set param", hookConfig_.process_name().c_str());
        std::string cmd = STARTUP + hookConfig_.process_name();
        int ret = SystemSetParameter(PARAM_NAME.c_str(), cmd.c_str());
        if (ret < 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "set param failed, please manually set param and start process(%s)",
                               hookConfig_.process_name().c_str());
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "set param success, please start process(%s)",
                              hookConfig_.process_name().c_str());
            hookCtx_.emplace_back(std::make_shared<HookManagerCtx>(hookConfig_.process_name()));
            hookConfig_.set_startup_mode(true);
        }
    } else if (pidValue != -1) {
        PROFILER_LOG_INFO(LOG_CORE, "Process %s exist, pid = %d", hookConfig_.process_name().c_str(), pidValue);
        for (const auto& item : hookCtx_) {
            if (item->pid == pidValue) {
                return true;
            }
        }
        hookCtx_.emplace_back(std::make_shared<HookManagerCtx>(pidValue));
    } else {
        PROFILER_LOG_ERROR(LOG_CORE, "The startup mode parameter is not set, name: %s",
                           hookConfig_.process_name().c_str());
        return false;
    }
    return true;
}

void HookManager::SetCommandPoller(const std::shared_ptr<CommandPoller>& p)
{
    commandPoller_ = p;
}

bool HookManager::RegisterAgentPlugin(const std::string& pluginPath)
{
    RegisterPluginRequest request;
    request.set_request_id(commandPoller_->GetRequestId());
    request.set_path(pluginPath);
    request.set_sha256("");
    request.set_name(pluginPath);
    request.set_buffer_size_hint(0);
    RegisterPluginResponse response;

    if (commandPoller_->RegisterPlugin(request, response)) {
        if (response.status() == ResponseStatus::OK) {
            PROFILER_LOG_DEBUG(LOG_CORE, "response.plugin_id() = %d", response.plugin_id());
            agentIndex_ = response.plugin_id();
            PROFILER_LOG_DEBUG(LOG_CORE, "RegisterPlugin OK");
        } else {
            PROFILER_LOG_DEBUG(LOG_CORE, "RegisterPlugin FAIL 1");
            return false;
        }
    } else {
        PROFILER_LOG_DEBUG(LOG_CORE, "RegisterPlugin FAIL 2");
        return false;
    }

    return true;
}

bool HookManager::UnregisterAgentPlugin(const std::string& pluginPath)
{
    UnregisterPluginRequest request;
    request.set_request_id(commandPoller_->GetRequestId());
    request.set_plugin_id(agentIndex_);
    UnregisterPluginResponse response;
    if (commandPoller_->UnregisterPlugin(request, response)) {
        CHECK_TRUE(response.status() == ResponseStatus::OK, false, "UnregisterPlugin FAIL 1");
    } else {
        PROFILER_LOG_DEBUG(LOG_CORE, "UnregisterPlugin FAIL 2");
        return false;
    }
    agentIndex_ = -1;

    return true;
}

bool HookManager::LoadPlugin(const std::string& pluginPath)
{
    return true;
}

bool HookManager::UnloadPlugin(const std::string& pluginPath)
{
    return true;
}

bool HookManager::UnloadPlugin(const uint32_t pluginId)
{
    return true;
}

void HookManager::GetClientConfig(ClientConfig& clientConfig)
{
    int sharedMemCount = (hookConfig_.offline_symbolization()) ? SHARED_MEMORY_NUM : 1;
    clientConfig.shareMemorySize = (static_cast<uint32_t>(hookConfig_.smb_pages()) / sharedMemCount) * PAGE_BYTES;
    clientConfig.filterSize = static_cast<int32_t>(hookConfig_.filter_size());
    clientConfig.clockId = COMMON::GetClockId(hookConfig_.clock());
    clientConfig.maxStackDepth = hookConfig_.max_stack_depth();
    clientConfig.arktsConfig.maxJsStackDepth = hookConfig_.max_js_stack_depth();
    clientConfig.mallocDisable = hookConfig_.malloc_disable();
    clientConfig.mmapDisable = hookConfig_.mmap_disable();
    clientConfig.freeStackData = hookConfig_.free_stack_report();
    clientConfig.munmapStackData = hookConfig_.munmap_stack_report();
    clientConfig.fpunwind = hookConfig_.fp_unwind();
    clientConfig.arktsConfig.jsFpunwind = hookConfig_.fp_unwind();
    clientConfig.isBlocked = hookConfig_.blocked();
    clientConfig.memtraceEnable = hookConfig_.memtrace_enable();
    clientConfig.statisticsInterval = hookConfig_.statistics_interval();
    clientConfig.sampleInterval = hookConfig_.sample_interval();
    clientConfig.offlineSymbolization = hookConfig_.offline_symbolization();
    clientConfig.responseLibraryMode = hookConfig_.response_library_mode();
    clientConfig.arktsConfig.jsStackReport = hookConfig_.js_stack_report();
    printMallocNmd_ = hookConfig_.dump_nmd() ? hookConfig_.dump_nmd() : printMallocNmd_;
    clientConfig.nmdType = (printMallocNmd_ && hookConfig_.dump_nmd()) ? ONLY_NMD_TYPE :
        static_cast<int>(nmdParamInfo_.type);
    const std::string& soName = hookConfig_.target_so_name();
    if (soName.length() < PATH_MAX && !COMMON::ContainsSpecialChars(soName)) {
        clientConfig.targetSoName = hookConfig_.target_so_name();
    }
    clientConfig.printNmd = printMallocNmd_;
    clientConfig.largestSize = largestSize_;
    clientConfig.secondLargestSize = secondLargestSize_;
    clientConfig.maxGrowthSize = maxGrowthSize_;
    // -1 is save '\0'
    int ret = memcpy_s(clientConfig.arktsConfig.filterNapiName, sizeof(clientConfig.arktsConfig.filterNapiName) - 1,
                       hookConfig_.filter_napi_name().c_str(), hookConfig_.filter_napi_name().size());
    if (ret != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s filter_napi_name fail");
    }
}

bool HookManager::HandleHookContext(const std::shared_ptr<HookManagerCtx>& ctx)
{
    if (ctx == nullptr) {
        return false;
    }

    // create smb and eventNotifier
    int sharedMemCount = (hookConfig_.offline_symbolization()) ? SHARED_MEMORY_NUM : 1;
    uint32_t bufferSize = (static_cast<uint32_t>(hookConfig_.smb_pages()) / sharedMemCount) * PAGE_BYTES;

    for (int i = 0; i < sharedMemCount; ++i) {
        std::string smbName = "";
        if (ctx->pid > 0) {
            smbName = "hooknativesmb_" + std::to_string(ctx->pid) + ":" + std::to_string(i);
        } else if (!ctx->processName.empty()) {
            smbName = "hooknativesmb_" + ctx->processName + ":" + std::to_string(i);
        } else {
            PROFILER_LOG_ERROR(LOG_CORE, "HandleHookContext context error, pid: %d, process name: %s",
                ctx->pid, ctx->processName.c_str());
            return false;
        }
        ctx->smbNames.push_back(smbName);
        auto shareMemoryBlock = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(smbName, bufferSize);
        CHECK_TRUE(shareMemoryBlock != nullptr, false, "CreateMemoryBlockLocal FAIL %s", smbName.c_str());
        ctx->shareMemoryBlockList.push_back(shareMemoryBlock);
        auto eventNotifier = EventNotifier::Create(0, EventNotifier::NONBLOCK);
        CHECK_NOTNULL(eventNotifier, false, "create EventNotifier for %s failed!", smbName.c_str());
        ctx->eventNotifierList.push_back(eventNotifier);
        PROFILER_LOG_INFO(LOG_CORE, "hookservice smbFd = %d, eventFd = %d\n", shareMemoryBlock->GetfileDescriptor(),
                          eventNotifier->GetFd());
        // start event poller task
        auto eventPoller = std::make_shared<EpollEventPoller>(DEFAULT_EVENT_POLLING_INTERVAL);
        CHECK_NOTNULL(eventPoller, false, "create event poller FAILED!");

        eventPoller->Init();
        eventPoller->Start();
        ctx->eventPollerList.push_back(eventPoller);
    }

    ctx->isRecordAccurately = hookConfig_.record_accurately();
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig filter size = %d, malloc disable = %d mmap disable = %d",
        hookConfig_.filter_size(), hookConfig_.malloc_disable(), hookConfig_.mmap_disable());
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig fp unwind = %d, max stack depth = %d, record_accurately=%d",
        hookConfig_.fp_unwind(), hookConfig_.max_stack_depth(), ctx->isRecordAccurately);
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig  offline_symbolization = %d", hookConfig_.offline_symbolization());
    PROFILER_LOG_INFO(LOG_CORE, "hookConfig  js_stack_report = %d max_js_stack_depth = %u",
        hookConfig_.js_stack_report(), hookConfig_.max_js_stack_depth());

    clockid_t pluginDataClockId = COMMON::GetClockId(hookConfig_.clock());
    if (noDataQueue_) {
        ctx->stackPreprocess = std::make_shared<StackPreprocess>(nullptr, hookConfig_, pluginDataClockId,
            fpHookData_, isHookStandalone_, isSaService_, isProtobufSerialize_);
        ctx->stackPreprocess->SetFlushSize(shareMemorySize_);
        ctx->stackPreprocess->SetNmdFd(nmdParamInfo_.fd);
        for (int i = 0; i < sharedMemCount; ++i) {
            ctx->eventPollerList[i]->AddFileDescriptor(
                ctx->eventNotifierList[i]->GetFd(),
                std::bind(&StackPreprocess::TakeResultsFromShmem, ctx->stackPreprocess,
                ctx->eventNotifierList[i], ctx->shareMemoryBlockList[i]));
        }
    } else {
        ctx->stackData = std::make_shared<StackDataRepeater>(STACK_DATA_SIZE);
        CHECK_TRUE(ctx->stackData != nullptr, false, "Create StackDataRepeater FAIL");
        ctx->stackPreprocess = std::make_shared<StackPreprocess>(ctx->stackData, hookConfig_, pluginDataClockId,
            fpHookData_, isHookStandalone_, isSaService_, isProtobufSerialize_);
        ctx->stackPreprocess->SetFlushSize(shareMemorySize_);
        for (int i = 0; i < sharedMemCount; ++i) {
            ctx->eventPollerList[i]->AddFileDescriptor(
                ctx->eventNotifierList[i]->GetFd(),
                [this, &ctx, i] { this->ReadShareMemory(ctx, i); });
        }
    }
    if (isProtobufSerialize_ || isSaService_) {
        ctx->stackPreprocess->SetWriter(g_buffWriter);
    } else {
        ctx->stackPreprocess->SetWriter(const_cast<WriterStructPtr>(writerAdapter_->GetStruct()));
    }
    ctx->stackPreprocess->SetFactory(factory_);
    return true;
}

void HookManager::CheckHapEncryped()
{
    for (const auto& pid : hookConfig_.expand_pids()) {
        if (pid > 0 && COMMON::CheckApplicationEncryped(pid, "")) {
            hookConfig_.set_js_stack_report(0);
            hookConfig_.set_max_js_stack_depth(0);
            break;
        }
    }
    const std::string processName = hookConfig_.process_name();
    if (!processName.empty() && COMMON::CheckApplicationEncryped(0, processName)) {
        PROFILER_LOG_INFO(LOG_CORE, "Encryped Application don't unwind js stack:%s", processName.c_str());
        hookConfig_.set_js_stack_report(0);
        hookConfig_.set_max_js_stack_depth(0);
    }
}

bool HookManager::CreatePluginSession(const std::vector<ProfilerPluginConfig>& config)
{
    PROFILER_LOG_DEBUG(LOG_CORE, "CreatePluginSession");
    // save config
    if (!config.empty()) {
        std::string cfgData = config[0].config_data();
        if (hookConfig_.ParseFromArray(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()) <= 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s: ParseFromArray failed", __func__);
            return false;
        }
    }
    if ((!saMode_) && (COMMON::IsUserMode())) {
        if (!COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
            return false;
        }
    }
    int32_t uShortMax = (std::numeric_limits<unsigned short>::max)();
    if (hookConfig_.filter_size() > uShortMax) {
        PROFILER_LOG_WARN(LOG_CORE, "%s: filter size invalid(size exceed 65535), reset to 65535!", __func__);
        hookConfig_.set_filter_size(uShortMax);
    }
    if (!CheckProcess()) { // Check and initialize the context for the target process.
        return false;
    }
    (void)CheckHapEncryped();
    if (hookConfig_.max_stack_depth() < DLOPEN_MIN_UNWIND_DEPTH) {
        // set default max depth
        hookConfig_.set_max_stack_depth(DLOPEN_MIN_UNWIND_DEPTH);
    }
#if defined(__arm__)
    hookConfig_.set_fp_unwind(false); // if OS is 32-bit,set fp_unwind false.
    hookConfig_.set_response_library_mode(false);
#endif
    if (hookConfig_.response_library_mode()) {
        hookConfig_.set_fp_unwind(true);
        hookConfig_.set_offline_symbolization(true);
        hookConfig_.set_js_stack_report(0);
    }
    // offlinem symbolization, callframe must be compressed
    if (hookConfig_.offline_symbolization()) {
        hookConfig_.set_callframe_compress(true);
    }

    // statistical reporting must be callframe compressed and accurate.
    if (hookConfig_.statistics_interval() > 0) {
        hookConfig_.set_callframe_compress(true);
        hookConfig_.set_record_accurately(true);
    }

    // malloc and free matching interval reporting must be callframe compressed and accurate.
    if (hookConfig_.malloc_free_matching_interval() > 0) {
        hookConfig_.set_callframe_compress(true);
        hookConfig_.set_record_accurately(true);
        hookConfig_.set_statistics_interval(0);
    }

    // callframe compressed, string must be compressed.
    if (hookConfig_.callframe_compress()) {
        hookConfig_.set_string_compressed(true);
    }

    if (hookConfig_.js_stack_report() > 0 && hookConfig_.max_js_stack_depth() == 0 && hookConfig_.fp_unwind()) {
        hookConfig_.set_max_js_stack_depth(DEFAULT_MAX_JS_STACK_DEPTH);
    }

    if (hookCtx_.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "HookManager no task");
        return false;
    }
    if ((hookConfig_.save_file() || isHookStandalone_) && !hookConfig_.file_name().empty()) {
        auto retFile = COMMON::CheckNotExistsFilePath(hookConfig_.file_name());
        if (!retFile.first) {
            PROFILER_LOG_INFO(LOG_CORE, "check file path %s fail", hookConfig_.file_name().c_str());
            return false;
        }
        fpHookData_ = fopen(retFile.second.c_str(), "wb+");
        if (fpHookData_ == nullptr) {
            PROFILER_LOG_INFO(LOG_CORE, "fopen file %s fail", hookConfig_.file_name().c_str());
            return false;
        }
    }
    if (hookConfig_.fp_unwind() && hookConfig_.offline_symbolization()
        && hookConfig_.statistics_interval() > 0) {
        noDataQueue_ = true;
    }

    if (!isSaService_) {
        CreateWriter();
    }
    factory_ = std::make_shared<HookRecordFactory>(hookConfig_);
    for (const auto& item : hookCtx_) {
        CHECK_TRUE(HandleHookContext(item), false, "handle hook context failed"); // Create the required resources.
    }

    if (!isSaService_) { // SA mode will start HookService in the service.
        ClientConfig clientConfig;
        GetClientConfig(clientConfig);
        if (noDataQueue_) {
            clientConfig.freeEventOnlyAddrEnable = true;
        }
        std::string clientConfigStr = clientConfig.ToString();
        PROFILER_LOG_INFO(LOG_CORE, "send hook client config:%s\n", clientConfigStr.c_str());
        hookService_ = std::make_shared<HookService>(clientConfig, shared_from_this(), (hookCtx_.size() > 1));
        CHECK_NOTNULL(hookService_, false, "HookService create failed!");
    }

    return true;
}

void HookManager::HookManagerCtx::FlushStackArray()
{
    if (g_rawDataArray.size() > 0 && stackData != nullptr) {
        if (!stackData->PutRawStackArray(g_rawDataArray, g_rawStackCount)) {
            PROFILER_LOG_INFO(LOG_CORE, "PutRawStackArray error");
        }
        g_rawStackCount = 0;
        g_rawDataArray = {};
    }
}

void HookManager::FlushRawStackArray(const std::shared_ptr<HookManagerCtx>& hookCtx,
                                     std::shared_ptr<HookRecord>& hookRecord)
{
    if (hookCtx == nullptr || hookRecord == nullptr) {
        return;
    }
    g_rawDataArray[g_rawStackCount] = hookRecord;
    ++g_rawStackCount;
    if (g_rawStackCount == CACHE_ARRAY_SIZE) {
        hookCtx->FlushStackArray();
    }
}

void HookManager::DumpNmdInfo(const char* nmdResult)
{
    std::string fileName = hookConfig_.process_name().empty() ? std::to_string(hookConfig_.pid()) :
                                                                hookConfig_.process_name();
    const std::string filePath = "/data/local/tmp/nmd_" + fileName + ".txt";
    std::ofstream outFile(filePath);
    if (!outFile.is_open()) {
        PROFILER_LOG_ERROR(LOG_CORE, "unable to write into nmd.txt");
        return;
    }
    outFile << nmdResult;
    if (!outFile) {
        PROFILER_LOG_ERROR(LOG_CORE, "Error: Failed to write data to file %s", filePath.c_str());
        outFile.close();
        return;
    }
    outFile.close();
    PROFILER_LOG_INFO(LOG_CORE, "Data successfully written to nmd file");
}

void HookManager::ReadShareMemory(const std::shared_ptr<HookManagerCtx>& hookCtx, int sharedMemoryIndex)
{
    std::shared_ptr<ShareMemoryBlock> shareMemoryBlock = hookCtx->shareMemoryBlockList[sharedMemoryIndex];
    CHECK_NOTNULL(shareMemoryBlock, NO_RETVAL, "smb is null!");
    hookCtx->eventNotifierList[sharedMemoryIndex]->Take();
    while (true) {
        std::shared_ptr<HookRecord> hookRecord = nullptr;
        std::shared_ptr<RawStack> rawStack = nullptr;
        bool ret = shareMemoryBlock->TakeData([&](const int8_t data[], uint32_t size) -> bool {
            hookRecord = factory_->GetHookRecord(data, size);
            CHECK_NOTNULL(hookRecord, false, "ReadShareMemory hookRecord invalid!");
            rawStack = hookRecord->GetRawStack();
            uint16_t type = hookRecord->GetType();
            if (type == FREE_MSG_SIMP) {
                return true;
            }

            if (type == NMD_MSG && printMallocNmd_) {
                const char* nmdResult = reinterpret_cast<const char*>(rawStack->data);
                if (!saMode_) {
                    DumpNmdInfo(nmdResult);
                }
                if (nmdParamInfo_.type == SIMP_NMD) {
                    simplifiedNmd_ = std::string(nmdResult);
                    nmdComplete_ = true;
                    PROFILER_LOG_INFO(LOG_CORE, "receive simplified nmd info, target pid :%d, processName:%s.\n",
                                      hookCtx->pid, hookCtx->processName.c_str());
                } else {
                    lseek(nmdParamInfo_.fd, 0, SEEK_END);
                    (void)write(nmdParamInfo_.fd, nmdResult, strlen(nmdResult));
                }
                return true;
            } else if (type == END_MSG) {
                return true;
            }
            rawStack->reportFlag = true;
            if (type == MEMORY_TAG || type == THREAD_NAME_MSG ||
                type == MMAP_FILE_TYPE || type == PR_SET_VMA_MSG ||
                type == JS_STACK_MSG) {
                return true;
            }
            rawStack->reduceStackFlag = false;
            if (hookConfig_.fp_unwind()) {
                if (rawStack->stackContext->jsChainId > 0) {
                    rawStack->jsStackData = hookCtx->stackPreprocess->GetJsRawStack(rawStack->stackContext->jsChainId);
                }
                return true;
            }
            return true;
        });
        if (!ret) {
            break;
        }
        uint16_t type = hookRecord->GetType();
        if (type == MEMORY_TAG) {
            std::string tagName = reinterpret_cast<char*>(rawStack->data);
            hookCtx->stackPreprocess->SaveMemTag(rawStack->stackContext->tagId, tagName);
            continue;
        } else if (type == JS_STACK_MSG) {
            hookCtx->stackPreprocess->SaveJsRawStack(rawStack->stackContext->jsChainId,
                                                     reinterpret_cast<char*>(rawStack->data));
            continue;
        } else if (type == END_MSG) {
            hookCtx->FlushStackArray();
            if (!hookCtx->stackData->PutRawStack(hookRecord, hookCtx->isRecordAccurately)) {
                break;
            }
            if (!hookCtx->stackData->PutRawStack(nullptr, false)) {
                break;
            }
            continue;
        } else if (type == NMD_MSG) {
            continue;
        }
        FlushRawStackArray(hookCtx, hookRecord);
    }
}

bool HookManager::DestroyPluginSession(const std::vector<uint32_t>& pluginIds)
{
    if ((!saMode_) && (COMMON::IsUserMode())) {
        if (!COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
            return false;
        }
    }
    for (const auto& item : hookCtx_) {
        for (size_t i = 0; i < item->eventPollerList.size(); ++i) {
            if (item->eventPollerList[i] && item->eventNotifierList[i]) {
                PROFILER_LOG_ERROR(LOG_CORE, "eventPoller unset! num: %zu", i);
                item->eventPollerList[i]->RemoveFileDescriptor(item->eventNotifierList[i]->GetFd());
                item->eventPollerList[i]->Stop();
                item->eventPollerList[i]->Finalize();
            }
            if (item->shareMemoryBlockList[i]) {
                ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(item->smbNames[i]);
            }
        }
        if (item->stackData != nullptr) {
            item->stackData->ClearCache();
        }
    }
    if (fpHookData_) {
        fclose(fpHookData_);
        fpHookData_ = nullptr;
    }
    return true;
}

bool HookManager::StartPluginSession(const std::vector<uint32_t>& pluginIds,
                                     const std::vector<ProfilerPluginConfig>& config, PluginResult& result)
{
    UNUSED_PARAMETER(config);
    if (hookCtx_.empty()) {
        return false;
    }
    if ((!saMode_) && (COMMON::IsUserMode())) {
        if (!COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
            return false;
        }
    }
    StartPluginSession();
    return true;
}

bool HookManager::StopPluginSession(const std::vector<uint32_t>& pluginIds)
{
    if (hookCtx_.empty()) {
        return false;
    }
    if ((!saMode_) && (COMMON::IsUserMode())) {
        if (!COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
            return false;
        }
    }
    for (const auto& item : hookCtx_) {
        if ((item->pid > 0) && hookService_) {
            hookService_->CloseSocketFd(static_cast<pid_t>(item->pid));
            hookService_->RemovePidInfo(static_cast<pid_t>(item->pid));
        } else if (item->pid <= 0) {
            PROFILER_LOG_INFO(LOG_CORE, "StopPluginSession: pid(%d) is less or equal zero.", item->pid);
        }
        CHECK_TRUE(item->stackPreprocess != nullptr, false, "stop StackPreprocess FAIL");
        item->stackPreprocess->StopTakeResults();
        PROFILER_LOG_INFO(LOG_CORE, "StopTakeResults success");
        if (hookConfig_.statistics_interval() > 0) {
            item->stackPreprocess->FlushRecordStatistics();
        }
        if (hookConfig_.malloc_free_matching_interval() > 0) {
            item->stackPreprocess->FlushRecordApplyAndReleaseMatchData();
        }
        if (item->stackData != nullptr) {
            item->stackData->Close();
        }
        item->stackPreprocess->FinishTraceFile();
    }
    return true;
}

void HookManager::ResetStartupParam()
{
    const std::string resetParam = "startup:disabled";
    if (hookConfig_.startup_mode()) {
        int ret = SystemSetParameter(PARAM_NAME.c_str(), resetParam.c_str());
        if (ret < 0) {
            PROFILER_LOG_WARN(LOG_CORE, "set param failed, please reset param(%s)", PARAM_NAME.c_str());
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "reset param success");
        }
    }
}

bool HookManager::ReportPluginBasicData(const std::vector<uint32_t>& pluginIds)
{
    return true;
}

bool HookManager::CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd,
                               bool isProtobufSerialize)
{
    PROFILER_LOG_DEBUG(LOG_CORE, "agentIndex_ %d", agentIndex_);
    writer_ = std::make_shared<BufferWriter>(pluginName, VERSION, bufferSize, smbFd, eventFd, agentIndex_);
    isProtobufSerialize_ = isProtobufSerialize;
    shareMemorySize_ = bufferSize;
    return true;
}

void HookManager::CreateWriter()
{
    PROFILER_LOG_INFO(LOG_CORE, "CreateWriter isProtobufSerialize: %d, noDataQueue_: %d",
                      isProtobufSerialize_, noDataQueue_);
    if (isProtobufSerialize_) {
        RegisterWriter(writer_);
    } else {
        writerAdapter_ = std::make_shared<WriterAdapter>(isProtobufSerialize_);
        writerAdapter_->SetWriter(writer_);
    }
}

bool HookManager::ResetWriter(uint32_t pluginId)
{
    RegisterWriter(nullptr);
    return true;
}

void HookManager::RegisterWriter(const std::shared_ptr<Writer> writer)
{
    g_buffWriter = writer;
    return;
}

void HookManager::SetHookConfig(const NativeHookConfig& hookConfig)
{
    hookConfig_ = hookConfig;
}

void HookManager::SethookStandalone(bool HookStandalone)
{
    isHookStandalone_ = HookStandalone;
}

void HookManager::SetHookConfig(const std::shared_ptr<NativeMemoryProfilerSaConfig>& config)
{
    hookConfig_.set_pid(config->pid_);
    if (!config->processName_.empty()) {
        hookConfig_.set_process_name(config->processName_);
    }
    hookConfig_.set_filter_size(config->filterSize_);
    hookConfig_.set_smb_pages(config->shareMemorySize_);
    hookConfig_.set_max_stack_depth(config->maxStackDepth_);
    hookConfig_.set_malloc_disable(config->mallocDisable_);
    hookConfig_.set_mmap_disable(config->mmapDisable_);
    hookConfig_.set_free_stack_report(config->freeStackData_);
    hookConfig_.set_munmap_stack_report(config->munmapStackData_);
    hookConfig_.set_malloc_free_matching_interval(config->mallocFreeMatchingInterval_);
    hookConfig_.set_malloc_free_matching_cnt(config->mallocFreeMatchingCnt_);
    hookConfig_.set_string_compressed(config->stringCompressed_);
    hookConfig_.set_fp_unwind(config->fpUnwind_);
    hookConfig_.set_blocked(config->blocked_);
    hookConfig_.set_record_accurately(config->recordAccurately_);
    hookConfig_.set_startup_mode(config->startupMode_);
    hookConfig_.set_memtrace_enable(config->memtraceEnable_);
    hookConfig_.set_offline_symbolization(config->offlineSymbolization_);
    hookConfig_.set_callframe_compress(config->callframeCompress_);
    hookConfig_.set_statistics_interval(config->statisticsInterval_);
    hookConfig_.set_clock(COMMON::GetClockStr(config->clockId_));
    hookConfig_.set_sample_interval(config->sampleInterval_);
    hookConfig_.set_response_library_mode(config->responseLibraryMode_);
    hookConfig_.set_js_stack_report(config->jsStackReport_);
    hookConfig_.set_max_js_stack_depth(config->maxJsStackDepth_);
    hookConfig_.set_filter_napi_name(config->filterNapiName_);
    hookConfig_.set_target_so_name(config->targetSoName_);
    printMallocNmd_ = config->printNmd_;
    if (config->hookstandalone_) {
        isHookStandalone_ = true;
    }
    if (config->saveFile_ || config->hookstandalone_) {
        hookConfig_.set_save_file(config->saveFile_);
        hookConfig_.set_file_name(config->fileName_);
        isProtobufSerialize_ = true;
    }
    largestSize_ = config->largestSize_;
    secondLargestSize_ = config->secondLargestSize_;
    maxGrowthSize_ = config->maxGrowthSize_;
}

int32_t HookManager::CreatePluginSession()
{
    if (CreatePluginSession({})) {
        return RET_OK;
    }
    return RET_ERR;
}

void HookManager::StartPluginSession(std::unordered_map<pid_t, std::pair<uid_t, gid_t>>* pidInfo)
{
    for (const auto& item : hookCtx_) {
        if (item->stackPreprocess == nullptr) {
            continue;
        }
        PROFILER_LOG_ERROR(LOG_CORE, "StartPluginSession name: %s", item->processName.c_str());
        if (!noDataQueue_) {
            item->stackPreprocess->StartTakeResults();
        }
        item->stackPreprocess->InitStatisticsTime();
        if (item->pid > 0) {
            PROFILER_LOG_INFO(LOG_CORE, "start command : send 36 signal to process  %d", item->pid);
            uid_t uid = 0;
            gid_t gid = 0;
            if (!COMMON::GetUidGidFromPid(static_cast<pid_t>(item->pid), uid, gid)) {
                PROFILER_LOG_INFO(LOG_CORE, "get target process uid gid failed");
                continue;
            }
            if (kill(item->pid, SIGNAL_START_HOOK) == -1) {
                const int bufSize = 256;
                char buf[bufSize] = {0};
                strerror_r(errno, buf, bufSize);
                PROFILER_LOG_ERROR(LOG_CORE, "send 36 signal error = %s", buf);
            }
            if (!saMode_) {
                hookService_->AddPidInfo(static_cast<pid_t>(item->pid), uid, gid);
            } else if (pidInfo != nullptr) {
                (*pidInfo)[static_cast<pid_t>(item->pid)] = std::make_pair(uid, gid);
            }
        } else {
            PROFILER_LOG_INFO(LOG_CORE, "StartPluginSession: pid(%d) is less or equal zero.", item->pid);
        }
    }
    if (!saMode_) {
        int ret = COMMON::PluginWriteToHisysevent("native_hook_plugin", "sh", GetCmdArgs(hookConfig_),
            COMMON::ErrorType::RET_SUCC, "success");
        PROFILER_LOG_INFO(LOG_CORE, "hisysevent report native_hook_plugin result:%d", ret);
    }
}

std::string HookManager::GetCmdArgs(const NativeHookConfig& traceConfig)
{
    std::stringstream args;
    args << "pid: " << COMMON::GetProcessNameByPid(traceConfig.pid()) << ", ";
    args << "save_file: " << (traceConfig.save_file() ? "true" : "false") << ", ";
    args << "filter_size: " << std::to_string(traceConfig.filter_size()) << ", ";
    args << "smb_pages: " << std::to_string(traceConfig.smb_pages()) << ", ";
    args << "max_stack_depth: " << std::to_string(traceConfig.max_stack_depth()) << ", ";
    args << "process_name: " << traceConfig.process_name() << ", ";
    args << "malloc_disable: " << (traceConfig.malloc_disable() ? "true" : "false") << ", ";
    args << "mmap_disable: " << (traceConfig.mmap_disable() ? "true" : "false") << ", ";
    args << "free_stack_report: " << (traceConfig.free_stack_report() ? "true" : "false") << ", ";
    args << "munmap_stack_report: " << (traceConfig.munmap_stack_report() ? "true" : "false") << ", ";
    args << "malloc_free_matching_interval: " << std::to_string(traceConfig.malloc_free_matching_interval()) << ", ";
    args << "malloc_free_matching_cnt: " << std::to_string(traceConfig.malloc_free_matching_cnt()) << ", ";
    args << "string_compressed: " << (traceConfig.string_compressed() ? "true" : "false") << ", ";
    args << "fp_unwind: " << (traceConfig.fp_unwind() ? "true" : "false") << ", ";
    args << "blocked: " << (traceConfig.blocked() ? "true" : "false") << ", ";
    args << "record_accurately: " << (traceConfig.record_accurately() ? "true" : "false") << ", ";
    args << "startup_mode: " << (traceConfig.startup_mode() ? "true" : "false") << ", ";
    args << "memtrace_enable: " << (traceConfig.memtrace_enable() ? "true" : "false") << ", ";
    args << "offline_symbolization: " << (traceConfig.offline_symbolization() ? "true" : "false") << ", ";
    args << "callframe_compress: " << (traceConfig.callframe_compress() ? "true" : "false") << ", ";
    args << "statistics_interval: " << std::to_string(traceConfig.statistics_interval()) << ", ";
    args << "clock: " << traceConfig.clock() << ", ";
    args << "sample_interval: " << std::to_string(traceConfig.sample_interval()) << ", ";
    args << "response_library_mode: " << (traceConfig.response_library_mode() ? "true" : "false") << ", ";
    args << "js_stack_report: " << std::to_string(traceConfig.js_stack_report()) << ", ";
    args << "max_js_stack_depth: " << std::to_string(traceConfig.max_js_stack_depth()) << ", ";
    args << "filter_napi_name: " << traceConfig.filter_napi_name() << ", ";
    args << "target_so_name: " << traceConfig.target_so_name() << ", ";
    for (const auto& pid : traceConfig.expand_pids()) {
        args << "expand_pids: " << std::to_string(pid) << ", ";
    }
    return args.str();
}

void HookManager::WriteHookConfig()
{
    for (const auto& item : hookCtx_) {
        if (item == nullptr) {
            PROFILER_LOG_ERROR(LOG_CORE, "HookManager WriteHookConfig failed");
            return;
        }
        item->stackPreprocess->WriteHookConfig();
    }
}

std::vector<int> HookManager::GetFds(int32_t pid, const std::string& name, int sharedMemCount)
{
    for (const auto& item : hookCtx_) {
        if (item->pid == pid || item->processName == name) {
            if (item->pid == -1) {
                item->pid = pid;
            }
            item->stackPreprocess->SetPid(pid);
            std::vector<int> fds;
            for (int i = 0; i < sharedMemCount; ++i) {
                fds.push_back(item->eventNotifierList[i]->GetFd());
                fds.push_back(item->shareMemoryBlockList[i]->GetfileDescriptor());
            }
            return fds;
        }
    }
    return std::vector<int>(sharedMemCount * DOUBLE, -1);
}

void HookManager::SetNmdInfo(std::pair<uint32_t, uint32_t> info)
{
    printMallocNmd_ = true;
    nmdParamInfo_.fd = info.first;
    nmdParamInfo_.type = info.second;
}
}