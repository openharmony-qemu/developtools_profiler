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
#include "hook_common.h"
#include "common.h"

namespace OHOS::Developtools::NativeDaemon {
const int DEFAULT_EVENT_POLLING_INTERVAL = 5000;
const int PAGE_BYTES = 4096;
std::shared_ptr<BufferWriter> g_buffWriter;
constexpr uint32_t MAX_BUFFER_SIZE = 10 * 1024;
const std::string STARTUP = "startup:";
const std::string PARAM_NAME = "libc.hook_mode";
const int MOVE_BIT_8 = 8;
const int MOVE_BIT_16 = 16;
const int MOVE_BIT_32 = 32;
const int SIGNAL_START_HOOK = 36;
const int SIGNAL_STOP_HOOK = 37;
const std::string VERSION = "1.02";

bool HookManager::CheckProcess()
{
    if (pid_ != 0) {
        int ret = 0;
        std::string pid_path = std::string();
        struct stat stat_buf;
        pid_path = "/proc/" + std::to_string(pid_) + "/status";
        if (stat(pid_path.c_str(), &stat_buf) != 0) {
            pid_ = 0;
            HILOG_ERROR(LOG_CORE, "%s: hook process does not exist", __func__);
            return false;
        } else {
            return true;
        }
    } else if (hookConfig_.process_name() != "") {
        // check if the pid and process name is consistency
        CheckProcessName();
    }

    return true;
}

void HookManager::CheckProcessName()
{
    int pidValue = -1;
    const std::string processName = hookConfig_.process_name();
    bool isExist = COMMON::IsProcessExist(processName, pidValue);
    if (hookConfig_.startup_mode() || !isExist) {
        HILOG_INFO(LOG_CORE, "Wait process %s start or restart, set param", hookConfig_.process_name().c_str());
        std::string cmd = STARTUP + hookConfig_.process_name();
        int ret = SystemSetParameter(PARAM_NAME.c_str(), cmd.c_str());
        if (ret < 0) {
            HILOG_WARN(LOG_CORE, "set param failed, please manually set param and start process(%s)",
                       hookConfig_.process_name().c_str());
        } else {
            HILOG_INFO(LOG_CORE, "set param success, please start process(%s)", hookConfig_.process_name().c_str());
        }
    } else {
        pid_ = pidValue;
        HILOG_INFO(LOG_CORE, "Process %s exist, pid = %d", hookConfig_.process_name().c_str(), pid_);
    }
}

HookManager::HookManager() : buffer_(new (std::nothrow) uint8_t[MAX_BUFFER_SIZE]), pid_(0) { }

HookManager::HookManager(std::shared_ptr<ShareMemoryBlock>& shareMemoryBlock,
                         std::shared_ptr<EventNotifier>& eventNotifier,
                         StackDataRepeaterPtr& stackData) : shareMemoryBlock_(shareMemoryBlock),
                                                            eventNotifier_(eventNotifier),
                                                            stackData_(stackData)
{
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
            HILOG_DEBUG(LOG_CORE, "response.plugin_id() = %d", response.plugin_id());
            agentIndex_ = response.plugin_id();
            HILOG_DEBUG(LOG_CORE, "RegisterPlugin OK");
        } else {
            HILOG_DEBUG(LOG_CORE, "RegisterPlugin FAIL 1");
            return false;
        }
    } else {
        HILOG_DEBUG(LOG_CORE, "RegisterPlugin FAIL 2");
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
        HILOG_DEBUG(LOG_CORE, "UnregisterPlugin FAIL 2");
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

void HookManager::GetClientConfig(const NativeHookConfig& nativeHookConfig, ClientConfig& clientConfig)
{
    clientConfig.shareMemroySize = static_cast<uint32_t>(hookConfig_.smb_pages() * PAGE_BYTES);
    clientConfig.filterSize = static_cast<int32_t>(hookConfig_.filter_size());
    clientConfig.clockId = COMMON::GetClockId(hookConfig_.clock());
    clientConfig.maxStackDepth = hookConfig_.max_stack_depth();
    clientConfig.mallocDisable = hookConfig_.malloc_disable();
    clientConfig.mmapDisable = hookConfig_.mmap_disable();
    clientConfig.freeStackData = hookConfig_.free_stack_report();
    clientConfig.munmapStackData = hookConfig_.munmap_stack_report();
    clientConfig.fpunwind = hookConfig_.fp_unwind();
    clientConfig.isBlocked = hookConfig_.blocked();
    clientConfig.memtraceEnable = hookConfig_.memtrace_enable();
}

bool HookManager::CreatePluginSession(const std::vector<ProfilerPluginConfig>& config)
{
    HILOG_DEBUG(LOG_CORE, "CreatePluginSession");
    smbName_ = "hooknativesmb";
    // save config
    std::string cfgData = config[0].config_data();
    CHECK_TRUE(hookConfig_.ParseFromArray(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()) > 0,
               false, "%s: ParseFromArray failed", __func__);

    CHECK_TRUE(getuid() == 0 || COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name()),
               false, "Application debug permisson denied!");

    pid_ = hookConfig_.pid();

    int32_t uShortMax = (std::numeric_limits<unsigned short>::max)();
    if (hookConfig_.filter_size() > uShortMax) {
        HILOG_WARN(LOG_CORE, "%s: filter size invalid(size exceed 65535), reset to 65535!", __func__);
        hookConfig_.set_filter_size(uShortMax);
    }
    if (!CheckProcess()) {
        return false;
    }

    // create smb and eventNotifier
    uint32_t bufferSize = hookConfig_.smb_pages() * PAGE_BYTES; /* bufferConfig.pages() */
    shareMemoryBlock_ = ShareMemoryAllocator::GetInstance().CreateMemoryBlockLocal(smbName_, bufferSize);
    CHECK_TRUE(shareMemoryBlock_ != nullptr, false, "CreateMemoryBlockLocal FAIL %s", smbName_.c_str());

    eventNotifier_ = EventNotifier::Create(0, EventNotifier::NONBLOCK);
    CHECK_NOTNULL(eventNotifier_, false, "create EventNotifier for %s failed!", smbName_.c_str());

    // start event poller task
    eventPoller_ = std::make_unique<EpollEventPoller>(DEFAULT_EVENT_POLLING_INTERVAL);
    CHECK_NOTNULL(eventPoller_, false, "create event poller FAILED!");

    eventPoller_->Init();
    eventPoller_->Start();
    eventPoller_->AddFileDescriptor(eventNotifier_->GetFd(), std::bind(&HookManager::ReadShareMemory, this));
    HILOG_INFO(LOG_CORE, "hookservice smbFd = %d, eventFd = %d\n", shareMemoryBlock_->GetfileDescriptor(),
               eventNotifier_->GetFd());

    if (hookConfig_.max_stack_depth() < DLOPEN_MIN_UNWIND_DEPTH) {
        // set default max depth
        hookConfig_.set_max_stack_depth(DLOPEN_MIN_UNWIND_DEPTH);
    }
#if defined(__arm__)
    hookConfig_.set_fp_unwind(false); // if OS is 32-bit,set fp_unwind false.
#endif
    // offlinem symbolization, callframe must be compressed
    if (hookConfig_.offline_symbolization()) {
        hookConfig_.set_callframe_compress(true);
    }

    // statistical reporting must be callframe compressed and accurate.
    if (hookConfig_.statistics_interval() > 0) {
        hookConfig_.set_callframe_compress(true);
        hookConfig_.set_record_accurately(true);
    }

    // callframe compressed, string must be compressed.
    if (hookConfig_.callframe_compress()) {
        hookConfig_.set_string_compressed(true);
    }

    isRecordAccurately_ = hookConfig_.record_accurately();
    HILOG_INFO(LOG_CORE, "hookConfig filter size = %d, malloc disable = %d mmap disable = %d",
        hookConfig_.filter_size(), hookConfig_.malloc_disable(), hookConfig_.mmap_disable());
    HILOG_INFO(LOG_CORE, "hookConfig fp unwind = %d, max stack depth = %d, record_accurately=%d",
        hookConfig_.fp_unwind(), hookConfig_.max_stack_depth(), isRecordAccurately_);
    HILOG_INFO(LOG_CORE, "hookConfig  offline_symbolization = %d", hookConfig_.offline_symbolization());

    ClientConfig clientConfig;
    GetClientConfig(hookConfig_, clientConfig);
    std::string clientConfigStr = clientConfig.ToString();
    HILOG_INFO(LOG_CORE, "send hook client config:%s\n", clientConfigStr.c_str());
    hookService_ = std::make_shared<HookService>(shareMemoryBlock_->GetfileDescriptor(),
        eventNotifier_->GetFd(), pid_, hookConfig_.process_name(), clientConfig);
    CHECK_NOTNULL(hookService_, false, "HookService create failed!");

    stackData_ = std::make_shared<StackDataRepeater>(STACK_DATA_SIZE);
    CHECK_TRUE(stackData_ != nullptr, false, "Create StackDataRepeater FAIL");
    clockid_t pluginDataClockId = COMMON::GetClockId(config[0].clock());
    stackPreprocess_ = std::make_shared<StackPreprocess>(stackData_, hookConfig_, pluginDataClockId);
    CHECK_TRUE(stackPreprocess_ != nullptr, false, "Create StackPreprocess FAIL");
    stackPreprocess_->SetWriter(g_buffWriter);
    return true;
}

void HookManager::ReadShareMemory()
{
    CHECK_NOTNULL(shareMemoryBlock_, NO_RETVAL, "smb is null!");
    uint64_t value = eventNotifier_->Take();
    int rawRealSize = 0;
    while (true) {
        auto rawStack = std::make_shared<StackDataRepeater::RawStack>();
        bool ret = shareMemoryBlock_->TakeData([&](const int8_t data[], uint32_t size) -> bool {
            CHECK_TRUE(size >= sizeof(BaseStackRawData), false, "stack data invalid!");

            rawStack->baseStackData = std::make_unique<uint8_t[]>(size);
            CHECK_TRUE(memcpy_s(rawStack->baseStackData.get(), size, data, size) == EOK, false,
                       "memcpy_s raw data failed!");

            rawStack->stackConext = reinterpret_cast<BaseStackRawData*>(rawStack->baseStackData.get());
            rawStack->data = rawStack->baseStackData.get() + sizeof(BaseStackRawData);
            rawStack->reportFlag = true;
            if (rawStack->stackConext->type == MEMORY_TAG || rawStack->stackConext->type == THREAD_NAME_MSG ||
                rawStack->stackConext->type == MMAP_FILE_TYPE || rawStack->stackConext->type == PR_SET_VMA_MSG) {
                return true;
            }
            rawStack->reduceStackFlag = false;
            if (hookConfig_.fp_unwind()) {
                rawStack->fpDepth = (size - sizeof(BaseStackRawData)) / sizeof(uint64_t);
                return true;
            } else {
                rawRealSize = sizeof(BaseStackRawData) + MAX_REG_SIZE * sizeof(char);
            }

            rawStack->stackSize = size - rawRealSize;
            if (rawStack->stackSize > 0) {
                rawStack->stackData = rawStack->baseStackData.get() + rawRealSize;
            }
            return true;
        });
        if (!ret) {
            break;
        }
        if (rawStack->stackConext->type == MEMORY_TAG) {
            std::string tagName = reinterpret_cast<char*>(rawStack->data);
            stackPreprocess_->SaveMemTag(rawStack->stackConext->tagId, tagName);
            continue;
        }
        if (!stackData_->PutRawStack(rawStack, isRecordAccurately_)) {
            break;
        }
    }
}

bool HookManager::DestroyPluginSession(const std::vector<uint32_t>& pluginIds)
{
    // release hook service
    hookService_ = nullptr;

    // stop event poller
    if (eventPoller_) {
        HILOG_ERROR(LOG_CORE, "eventPoller_ unset!");
        if (eventNotifier_ != nullptr) {
            eventPoller_->RemoveFileDescriptor(eventNotifier_->GetFd());
        }
        eventPoller_->Stop();
        eventPoller_->Finalize();
        eventPoller_ = nullptr;
    }

    // release smb and eventNotifier
    if (shareMemoryBlock_) {
        ShareMemoryAllocator::GetInstance().ReleaseMemoryBlockLocal(smbName_);
        shareMemoryBlock_ = nullptr;
    }

    if (eventNotifier_) {
        eventNotifier_ = nullptr;
    }

    stackPreprocess_ = nullptr;
    stackData_ = nullptr;
    return true;
}

bool HookManager::StartPluginSession(const std::vector<uint32_t>& pluginIds,
                                     const std::vector<ProfilerPluginConfig>& config, PluginResult& result)
{
    UNUSED_PARAMETER(config);
    CHECK_TRUE(stackPreprocess_ != nullptr, false, "start StackPreprocess FAIL");
    stackPreprocess_->StartTakeResults();

    if (pid_ > 0) {
        HILOG_INFO(LOG_CORE, "start command : send 36 signal to process  %d", pid_);
        if (kill(pid_, SIGNAL_START_HOOK) == -1) {
            const int bufSize = 256;
            char buf[bufSize] = {0};
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "send 36 signal error = %s", buf);
        }
    } else {
        HILOG_INFO(LOG_CORE, "StartPluginSession: pid_(%d) is less or equal zero.", pid_);
    }

    return true;
}

bool HookManager::StopPluginSession(const std::vector<uint32_t>& pluginIds)
{
    // send signal
    if (pid_ == 0 && hookService_ != nullptr) {
        pid_ = hookService_->GetPid();
    }
    if (pid_ > 0) {
        HILOG_INFO(LOG_CORE, "stop command : send 37 signal to process  %d", pid_);
        if (kill(pid_, SIGNAL_STOP_HOOK) == -1) {
            const int bufSize = 256;
            char buf[bufSize] = {0};
            strerror_r(errno, buf, bufSize);
            HILOG_ERROR(LOG_CORE, "send 37 signal to process %d , error = %s", pid_, buf);
        }
    } else {
        HILOG_INFO(LOG_CORE, "StopPluginSession: pid_(%d) is less or equal zero.", pid_);
    }

    CHECK_TRUE(stackPreprocess_ != nullptr, false, "stop StackPreprocess FAIL");
    HILOG_INFO(LOG_CORE, "start StopTakeResults");
    stackPreprocess_->StopTakeResults();

    HILOG_INFO(LOG_CORE, "StopTakeResults success");
    if (hookConfig_.statistics_interval() > 0) {
        stackPreprocess_->FlushRecordStatistics();
    }

    // make sure TakeResults thread exit
    if (stackData_) {
        stackData_->Close();
    }
    ResetStartupParam();
    return true;
}

void HookManager::ResetStartupParam()
{
    const std::string resetParam = "startup:disabled";
    if (hookConfig_.startup_mode()) {
        int ret = SystemSetParameter(PARAM_NAME.c_str(), resetParam.c_str());
        if (ret < 0) {
            HILOG_WARN(LOG_CORE, "set param failed, please reset param(%s)", PARAM_NAME.c_str());
        } else {
            HILOG_INFO(LOG_CORE, "reset param success");
        }
    }
}

bool HookManager::ReportPluginBasicData(const std::vector<uint32_t>& pluginIds)
{
    return true;
}

bool HookManager::CreateWriter(std::string pluginName, uint32_t bufferSize, int smbFd, int eventFd)
{
    HILOG_DEBUG(LOG_CORE, "agentIndex_ %d", agentIndex_);
    RegisterWriter(std::make_shared<BufferWriter>(pluginName, VERSION, bufferSize, smbFd, eventFd, agentIndex_));
    return true;
}

bool HookManager::ResetWriter(uint32_t pluginId)
{
    RegisterWriter(nullptr);
    return true;
}

void HookManager::RegisterWriter(const BufferWriterPtr& writer)
{
    g_buffWriter = writer;
    return;
}

void HookManager::SetHookConfig(const NativeHookConfig& hookConfig)
{
    hookConfig_ = hookConfig;
}
}