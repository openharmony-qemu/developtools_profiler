/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include "native_daemon_sa_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <unistd.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "init_param.h"
#include "native_memory_profiler_sa_client_manager.h"
#include "utilities.h"
#include "token_setproc.h"
#include "accesstoken_kit.h"

namespace {
const std::string NATIVE_PARAM = "hiviewdfx.hiprofiler.native_memoryd.start";
const std::string TEST_PROC_NAME = "hiview";
}

namespace OHOS {
bool FuzzNativeDaemonSa(const uint8_t* data, size_t size)
{
    using namespace OHOS::Developtools::NativeDaemon;
    using namespace OHOS::Security::AccessToken;
    AccessTokenID tokenID = AccessTokenKit::GetNativeTokenId(TEST_PROC_NAME);
    SetSelfTokenID(tokenID);
    sleep(1);
    uint32_t pid = 0;
    uint32_t duration = 0;
    uint32_t sampleInterval = 0;
    uint32_t stopPid= 0;
    if (size >= 1) {
        pid = static_cast<uint32_t>(data[0]);
    }
    if (size >= 2) {
        duration = static_cast<uint32_t>(data[1]);
    }
    if (size >= 3) {
        sampleInterval = static_cast<uint32_t>(data[2]);
    }
    if (size >= 4) {
        stopPid = static_cast<uint32_t>(data[3]);
    }
    NativeMemoryProfilerSaClientManager::Start(
        NativeMemoryProfilerSaClientManager::NativeMemProfilerType::MEM_PROFILER_CALL_STACK, pid, duration, sampleInterval);
    sleep(10);
    NativeMemoryProfilerSaClientManager::Stop(stopPid);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzNativeDaemonSa(data, size);
    return 0;
}
