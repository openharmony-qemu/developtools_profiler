/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

using namespace OHOS::Developtools::NativeDaemon;

namespace {
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
const std::string VERSION = "1.01";
}  // namespace

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
    std::string processName = hookConfig_.process_name();
    bool isExist = COMMON::IsProcessExist(processName, pidValue);
    if (!isExist) {
        HILOG_INFO(LOG_CORE, "Process %s not exist, set param", hookConfig_.process_name().c_str());
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
        if (response.status() != ResponseStatus::OK) {
            HILOG_DEBUG(LOG_CORE, "UnregisterPlugin FAIL 1");
            return false;
        }
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

bool HookManager::CreatePluginSession(const std::vector<ProfilerPluginConfig>& config)
{
    HILOG_DEBUG(LOG_CORE, "CreatePluginSession");
    UNUSED_PARAMETER(config);
    smbName_ = "hooknativesmb";
    // save config
    std::string cfgData = config[0].config_data();
    if (hookConfig_.ParseFromArray(reinterpret_cast<const uint8_t*>(cfgData.c_str()), cfgData.size()) <= 0) {
        HILOG_ERROR(LOG_CORE, "%s: ParseFromArray failed", __func__);
        return false;
    }

    if (getuid() != 0 && !COMMON::CheckApplicationPermission(hookConfig_.pid(), hookConfig_.process_name())) {
        HILOG_ERROR(LOG_CORE, "Application debug permisson denied!");
        return false;
    }

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

    // hook config |F F            F F              F F F F       F F F F      F F F F|
    //              stack depth    malloctype       filtersize    sharememory  size

    if (hookConfig_.max_stack_depth() == 0) {
        // set default max depth
        hookConfig_.set_max_stack_depth(MAX_UNWIND_DEPTH);
    }
#if defined(__arm__)
    hookConfig_.set_fp_unwind(false); // if OS is 32-bit,set fp_unwind false.
#endif
    uint64_t hookConfig = (uint8_t)hookConfig_.max_stack_depth();
    hookConfig <<= MOVE_BIT_8;

    hookConfig |= hookConfig_.malloc_disable() ? MALLOCDISABLE : 0;
    hookConfig |= hookConfig_.mmap_disable() ? MMAPDISABLE : 0;
    hookConfig |= hookConfig_.free_stack_report() ? FREEMSGSTACK : 0;
    hookConfig |= hookConfig_.munmap_stack_report() ? MUNMAPMSGSTACK : 0;
    hookConfig |= hookConfig_.fp_unwind() ? FPUNWIND : 0;
    hookConfig |= hookConfig_.blocked() ? BLOCKED : 0;

    hookConfig <<= MOVE_BIT_16;
    hookConfig |= hookConfig_.filter_size();
    hookConfig <<= MOVE_BIT_32;
    hookConfig |= bufferSize;

    HILOG_INFO(LOG_CORE, "hookConfig filter size = %d, malloc disable = %d mmap disable = %d smb size = %u",
        hookConfig_.filter_size(), hookConfig_.malloc_disable(), hookConfig_.mmap_disable(), bufferSize);
    HILOG_INFO(LOG_CORE, "hookConfig fp unwind = %d, max stack depth = %d",
        hookConfig_.fp_unwind(), hookConfig_.max_stack_depth());

    hookService_ = std::make_shared<HookService>(shareMemoryBlock_->GetfileDescriptor(),
                                                eventNotifier_->GetFd(), pid_, hookConfig_.process_name(), hookConfig);
    CHECK_NOTNULL(hookService_, false, "HookService create failed!");

    stackData_ = std::make_shared<StackDataRepeater>(STACK_DATA_SIZE);
    CHECK_TRUE(stackData_ != nullptr, false, "Create StackDataRepeater FAIL");
    stackPreprocess_ = std::make_shared<StackPreprocess>(stackData_, hookConfig_);
    CHECK_TRUE(stackPreprocess_ != nullptr, false, "Create StackPreprocess FAIL");
    stackPreprocess_->SetWriter(g_buffWriter);
    return true;
}

void HookManager::ReadShareMemory()
{
    CHECK_NOTNULL(shareMemoryBlock_, NO_RETVAL, "smb is null!");
    uint64_t value = eventNotifier_->Take();

    while (true) {
        auto rawStack = std::make_shared<StackDataRepeater::RawStack>();

        bool ret = shareMemoryBlock_->TakeData([&](const int8_t data[], uint32_t size) -> bool {
            if (size < sizeof(rawStack->stackConext)) {
                HILOG_ERROR(LOG_CORE, "stack data invalid!");
                return false;
            }
            if (memcpy_s(reinterpret_cast<void*>(&(rawStack->stackConext)), sizeof(rawStack->stackConext), data,
                         sizeof(rawStack->stackConext)) != EOK) {
                HILOG_ERROR(LOG_CORE, "memcpy_s raw data failed!");
                return false;
            }
            rawStack->reportFlag = true;
            rawStack->reduceStackFlag = false;
            rawStack->stackSize = size - sizeof(rawStack->stackConext);
            if (rawStack->stackSize > 0) {
                rawStack->stackData = std::make_unique<uint8_t[]>(rawStack->stackSize);
                if (memcpy_s(rawStack->stackData.get(), rawStack->stackSize, data + sizeof(rawStack->stackConext),
                             rawStack->stackSize) != EOK) {
                    HILOG_ERROR(LOG_CORE, "memcpy_s stack data failed!");
                }
            }
            return true;
        });
        if (!ret) {
            break;
        }
        if (!stackData_->PutRawStack(rawStack)) {
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
        eventPoller_->RemoveFileDescriptor(eventNotifier_->GetFd());
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

    // make sure TakeResults thread exit
    if (stackData_) {
        stackData_->Close();
    }
    return true;
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

bool HookManager::SendProtobufPackage(uint8_t *cache, size_t length)
{
    if (g_buffWriter == nullptr) {
        HILOG_ERROR(LOG_CORE, "HookManager:: BufferWriter empty, should set writer first");
        return false;
    }
    ProfilerPluginData pluginData;
    pluginData.set_name("nativehook");
    pluginData.set_status(0);
    pluginData.set_data(cache, length);

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    pluginData.set_clock_id(ProfilerPluginData::CLOCKID_REALTIME);
    pluginData.set_tv_sec(ts.tv_sec);
    pluginData.set_tv_nsec(ts.tv_nsec);

    g_buffWriter->WriteMessage(pluginData, "nativehook");
    g_buffWriter->Flush();
    return true;
}
