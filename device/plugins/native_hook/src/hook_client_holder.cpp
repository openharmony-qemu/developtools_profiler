/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2024. All rights reserved.
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

#include "hook_client_holder.h"
#include "hook_socket_client.h"
#include "hook_common.h"
#include "hook_client.h"
#include "get_thread_id.h"
#include "logging.h"
#include "runtime_stack_range.h"
#include <sys/prctl.h>
#include <pthread.h>
#include <cstring>

constexpr int UPDATE_THREAD_NAME = 1000;

HookClientHolder::HookClientHolder(std::shared_ptr<HookSocketClient>& sharedClient)
    : holder_(sharedClient)
{
}

HookSocketClient* HookClientHolder::Get()
{
    std::shared_ptr<HookSocketClient> client = holder_.lock();
    return client ? client.get() : nullptr;
}

bool HookClientHolder::IsValid() const
{
    return holder_.lock() != nullptr;
}

bool HookClientHolder::SendStackWithPayload(const void* data, size_t size, const void* payload,
    size_t payloadSize, int smbIndex)
{
    std::shared_ptr<HookSocketClient> client = holder_.lock();
    if (client == nullptr) {
        return false;
    }
    return client->SendStackWithPayload(data, size, payload, payloadSize, smbIndex);
}

bool HookClientHolder::UpdateThreadName()
{
    std::shared_ptr<HookSocketClient> client = holder_.lock();
    if (client == nullptr) {
        return false;
    }

    long updateCount = reinterpret_cast<long>(pthread_getspecific(g_updateThreadNameCount));
    bool ret = true;

    if (updateCount == 0) {
        NameData tnameData = {{{{0}}}};
        tnameData.tid = static_cast<uint32_t>(GetCurThreadId());
        tnameData.type = THREAD_NAME_MSG;
        if (prctl(PR_GET_NAME, tnameData.name) != 0) {
            PROFILER_LOG_ERROR(LOG_CORE, "prctl tnameData.name error");
            return false;
        }
        ret = client->SendStackWithPayload(&tnameData,
                                           sizeof(BaseStackRawData) + strlen(tnameData.name) + 1,
                                           nullptr, 0);
        if (!ret) {
            return ret;
        }
    }
    pthread_setspecific(g_updateThreadNameCount,
                        reinterpret_cast<void *>(updateCount == UPDATE_THREAD_NAME ? 0 : updateCount + 1));
    return ret;
}

void HookClientHolder::SendMmapFileRawData(int prot, int flags, off_t offset, const char* filePath,
                                           const StackRawData& rawdata)
{
    std::shared_ptr<HookSocketClient> client = holder_.lock();
    if (client == nullptr) {
        return;
    }
    NameData curRawdata = {{{{0}}}};
    curRawdata.addr = rawdata.addr;
    curRawdata.pid = static_cast<uint32_t>(g_hookPid.load());
    curRawdata.mallocSize = rawdata.mallocSize;
    curRawdata.mmapArgs.offset = offset;
    curRawdata.type = MMAP_FILE_TYPE;
    if (static_cast<uint32_t>(prot) & PROT_EXEC) {
        curRawdata.mmapArgs.flags |= PROT_EXEC;
    }
    size_t len = strlen(filePath) + 1;
    if ((static_cast<uint32_t>(flags) & MAP_FIXED) && (g_clientConfig.responseLibraryMode) &&
        (IsLegalSoName(filePath)) && (strstr(filePath, "ld-musl") != NULL || strstr(filePath, "libc++") != NULL)) {
        std::lock_guard<std::recursive_timed_mutex> guard(g_FilterMapMutex);
        ParseEvent(filePath, g_filterStaLibRange, curRawdata);
    }
    if ((static_cast<uint32_t>(flags) & MAP_FIXED) && (IsLegalSoName(filePath)) &&
        (!g_clientConfig.targetSoName.empty()) && strstr(filePath, g_clientConfig.targetSoName.c_str()) != NULL) {
        uint64_t soStart = reinterpret_cast<uint64_t>(curRawdata.addr);
        uint64_t soEnd = soStart + static_cast<uint64_t>(curRawdata.mallocSize);
        auto range = g_targetedRange.load();
        if (range.start == 0 && range.end == 0) {
            range.start = soStart;
            range.end = soEnd;
        }
        if (range.start > soStart) {
            range.start = soStart;
        }
        if (range.end < soEnd) {
            range.end = soEnd;
        }
        g_targetedRange.store(range);
    }
    if (strncpy_s(curRawdata.name, MAX_HOOK_PATH + 1, filePath, len) != EOK) {
        return;
    }
    curRawdata.name[len - 1] = '\0';
    if (static_cast<uint32_t>(flags) & MAP_FIXED) {
        curRawdata.mmapArgs.flags |= MAP_FIXED;
    }
    client->SendStackWithPayload(&curRawdata, sizeof(BaseStackRawData) + len, nullptr, 0);
}
