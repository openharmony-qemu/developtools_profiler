/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include "networkprotocolproc_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "socket_context.h"
#include "network_profiler_manager.h"
#include "network_profiler_socket_service.h"
#include "fuzzer/FuzzedDataProvider.h"


using namespace OHOS::Developtools::Profiler;

namespace OHOS {
bool NetworkProtocolProcFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return false;
    }
    FuzzedDataProvider provider(data, size);
    auto networkProfilerMgr = std::make_shared<NetworkProfilerManager>();
    auto socketService = std::make_shared<NetworkProfilerSocketService>(networkProfilerMgr);
    SocketContext socketContext;
    int config = provider.ConsumeIntegral<int>();
    uint32_t pnum = provider.ConsumeIntegral<uint32_t>();
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    bool ret1 = socketService->ProtocolProc(socketContext, pnum, ptr, sizeof(int));
    uint32_t sizeTemp = provider.ConsumeIntegral<uint32_t>();
    bool ret2 = socketService->ProtocolProc(socketContext, pnum, ptr, sizeTemp);
    return ret1 || ret2;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::NetworkProtocolProcFuzzTest(data, size);
    return 0;
}
