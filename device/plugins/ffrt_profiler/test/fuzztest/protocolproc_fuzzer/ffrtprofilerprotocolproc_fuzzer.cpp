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

#include "ffrtprofilerprotocolproc_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "socket_context.h"
#include "ffrt_profiler_manager.h"
#include "ffrt_profiler_socket_service.h"

using namespace OHOS::Developtools::Profiler;

namespace OHOS {
template<typename T>
T GenerateRandomNumber(const uint8_t* data, size_t size, size_t& offset)
{
    if (offset + sizeof(T) > size) {
        return 0;
    }
    T arg;
    if (memcpy_s(&arg, sizeof(arg), data + offset, sizeof(arg)) != EOK) {
        return 0;
    }
    offset = (offset + sizeof(T)) % size;
    return arg;
}

bool FfrtProtocolProcFuzzTest(const uint8_t* data, size_t size)
{
    auto ffrtProfilerMgr = std::make_shared<FfrtProfilerManager>();
    auto socketService = std::make_shared<FfrtProfilerSocketService>(ffrtProfilerMgr);
    SocketContext socketContext;
    size_t offset = 0;
    int8_t config = GenerateRandomNumber<int8_t>(data, size, offset);
    uint32_t pnum = GenerateRandomNumber<uint32_t>(data, size, offset);
    auto ptr = reinterpret_cast<const int8_t*>(&config);
    uint32_t sizeTemp = GenerateRandomNumber<uint32_t>(data, size, offset);
    bool ret2 = socketService->ProtocolProc(socketContext, pnum, ptr, sizeTemp);
    return ret2;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::FfrtProtocolProcFuzzTest(data, size);
    return 0;
}
