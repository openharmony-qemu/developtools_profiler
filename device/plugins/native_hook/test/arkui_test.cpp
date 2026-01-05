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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdint>
#include <unistd.h>
#include <chrono>
#include <unistd.h>
#include <securec.h>
#include <string>

#ifdef HOOK_ENABLE
#include <dlfcn.h>
#include <memory_trace.h>
#endif

#pragma clang optimize off

namespace {
constexpr int DEFAULT_DEPTH = 5;
constexpr int DEFAULT_COUNT = 10;
constexpr size_t DEFAULT_SIZE = 64;
constexpr uint64_t SLEEP_TIME_SEC = 5;

constexpr uint32_t NODE_TYPE_A = 1;
constexpr uint64_t NODE_ID_A = 100;
constexpr uint32_t NODE_TYPE_B = 2;
constexpr uint64_t NODE_ID_B = 200;

static void DepthArkUiMalloc(int depth, size_t size, uint32_t nodeType, uint64_t nodeId)
{
    if (size == 0) {
        return;
    }
    if (depth > 0) {
        DepthArkUiMalloc(depth - 1, size, nodeType, nodeId);
        return;
    }
    char* buffer = new (std::nothrow) char[size];
    if (buffer == nullptr) {
        printf("DMA allocateAndFreeRegion: allocation failed, size=%zu", size);
        return;
    }
    uint32_t oldNodeType = 0;
    uint64_t oldNodeId = 0;
    setResTraceId(nodeType, nodeId, &oldNodeType, &oldNodeId);
    printf("setResTraceId: newNodeType=%u newNodeId=%lu oldNodeType=%u oldNodeId=%lu\n",
        nodeType, nodeId, oldNodeType, oldNodeId);
    restrace(RES_GPU_GLES_IMAGE, buffer, static_cast<int>(size), TAG_RES_GPU_GLES_IMAGE, true);
    buffer[0] = 'x';
    buffer[size - 1] = 'y';
    restrace(RES_GPU_GLES_IMAGE, buffer, static_cast<int>(size), TAG_RES_GPU_GLES_IMAGE, false);
    char* gpuBuffer = new (std::nothrow) char[size];
    if (gpuBuffer == nullptr) {
        printf("GPU allocateAndFreeRegion: allocation failed, size=%zu", size);
        delete[] buffer;
        return;
    }
    restrace(RES_GPU_VK, gpuBuffer, static_cast<int>(size), TAG_RES_GPU_VK, true);
    gpuBuffer[0] = 'x';
    gpuBuffer[size - 1] = 'y';
    restrace(RES_GPU_VK, gpuBuffer, static_cast<int>(size), TAG_RES_GPU_VK, false);
    delete[] buffer;
    delete[] gpuBuffer;
    return;
}

} // namespace

int main()
{
    const int depth = DEFAULT_DEPTH;
    const size_t size = DEFAULT_SIZE;
    const int count = DEFAULT_COUNT;

    printf("arkui_test start: depth=%d size=%zu count=%d\n", depth, size, count);
    printf("This test triggers same call stack, different tagName and node_type/node_id via malloc.\n");
    sleep(SLEEP_TIME_SEC);

    for (int i = 0; i < count; ++i) {
        DepthArkUiMalloc(depth, size, NODE_TYPE_A, NODE_ID_A);
        DepthArkUiMalloc(depth, size, NODE_TYPE_B, NODE_ID_B);
    }

    printf("arkui_test finished.\n");
    sleep(SLEEP_TIME_SEC);
    _exit(0);
}

#pragma clang optimize on
