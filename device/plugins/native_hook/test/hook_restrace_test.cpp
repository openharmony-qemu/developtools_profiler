/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <unistd.h>
#include <securec.h>
#include <string>
#include <memory_trace.h>

#pragma clang optimize off

std::recursive_mutex mtx;

constexpr uint64_t S_TO_NS = 1000 * 1000 * 1000;
constexpr uint64_t SLEEP_TIME = 5;
constexpr uint64_t COUNT_INDEX = 3;
constexpr uint64_t SIZE_INDEX = 2;
constexpr int MAX_SIZE = 1024 * 1024 * 1024;
constexpr int DEFAULT_VAL = 10;

void AllocateMemoryFreeRegion(int depth, int size)
{
    if (size > MAX_SIZE || (size == 0)) {
        return;
    }
    if (depth == 0) {
        char* buffer = new (std::nothrow) char[size];
        if (buffer == nullptr) {
            std::cerr << "AllocateAndFreeRegion: allocation failed, size=" << size << std::endl;
            return;
        }
        restrace(RES_ARKTS_HEAP_MASK, buffer, DEFAULT_VAL, TAG_RES_ARKTS_HEAP_MASK, true);

        buffer[0] = 'x';
        buffer[size - 1] = 'y';
        resTraceFreeRegion(RES_ARKTS_HEAP_MASK, static_cast<void*>(buffer), static_cast<size_t>(size));

        restrace(RES_ARKTS_HEAP_MASK, buffer, DEFAULT_VAL, TAG_RES_ARKTS_HEAP_MASK, false);
        delete[] buffer;
        return;
    }
    AllocateMemoryFreeRegion(depth - 1, size);
}

void AllocateMemoryMove(int depth, int size)
{
    if (size > MAX_SIZE || (size == 0)) {
        return;
    }
    if (depth == 0) {
        char* oldMem = new (std::nothrow) char[size];
        if (oldMem == nullptr) {
            return;
        }
        oldMem[0] = 'a';
        restrace(RES_ARKTS_HEAP_MASK, oldMem, DEFAULT_VAL, TAG_RES_ARKTS_HEAP_MASK, true);

        void* newMem = malloc(static_cast<size_t>(size));
        if (!newMem) {
            delete[] oldMem;
            return;
        }

        errno_t ret = memcpy_s(newMem, static_cast<size_t>(size), oldMem, 1);
        if (ret != EOK) {
            delete[] oldMem;
            free(newMem);
            return;
        }

        resTraceMove(RES_ARKTS_HEAP_MASK, static_cast<void*>(oldMem), newMem, static_cast<size_t>(size));

        restrace(RES_ARKTS_HEAP_MASK, newMem, DEFAULT_VAL, TAG_RES_ARKTS_HEAP_MASK, false);
        delete[] oldMem;
        free(newMem);
        return;
    }
    AllocateMemoryMove(depth - 1, size);
}

void AllocateMemory(int depth, int size)
{
    if (size > MAX_SIZE || (size == 0)) {
        return;
    }
    if (depth == 0) {
        char* mem = new char[size];
        restrace(RES_GPU_VK, mem, DEFAULT_VAL, TAG_RES_GPU_VK, true);
        if (mem == nullptr) {
            return;
        }
        mem[0] = 'a';
        restrace(RES_GPU_VK, mem, DEFAULT_VAL, TAG_RES_GPU_VK, false);
        delete[] mem;
        return;
    }
    AllocateMemory(depth - 1, size);
}

void ThreadFunc(int depth, int count, int size)
{
    for (int i = 0; i < count; ++i) {
        AllocateMemory(depth, size);
        AllocateMemoryFreeRegion(depth, size);
        AllocateMemoryMove(depth, size);
    }
}

int main(int argc, char* argv[])
{
    int threadCount = DEFAULT_VAL;
    int depth = DEFAULT_VAL;
    int count = DEFAULT_VAL;
    int mallocSize = 1;
    if (argc < 4) { //4: number of expected args
        std::cout << "args are not enough!" << std::endl;
        return 0;
    }
    depth = atoi(argv[1]);
    mallocSize = atoi(argv[SIZE_INDEX]);
    count = atoi(argv[COUNT_INDEX]);
    if (depth <= 0) {
        std::cout << "invalid depth" << std::endl;
        return 0;
    }
    if (count <= 0) {
        std::cout << "invalid count" << std::endl;
        return 0;
    }
    if (mallocSize < 1 || mallocSize >= MAX_SIZE) {
        std::cout << "invalid size" << std::endl;
        return 0;
    }
    std::cout << "starting memory allocation..." << std::endl;
    sleep(SLEEP_TIME);
    std::cout << "starting hook..." << std::endl;
    void* ptr = malloc(1);
    free(ptr);
    sleep(SLEEP_TIME);
    std::thread threads[threadCount];
    std::cout << "Running..." << std::endl;
    struct timespec start = {};
    clock_gettime(CLOCK_REALTIME, &start);
    for (int i = 0; i < threadCount; ++i) {
        threads[i] = std::thread(ThreadFunc, depth, count, mallocSize);
    }

    for (int i = 0; i < threadCount; i++) {
        threads[i].join();
    }
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);

    std::cout << "Total cost time: " << (end.tv_sec - start.tv_sec) * S_TO_NS + (end.tv_nsec - start.tv_nsec)
              << " ns" << std::endl;

    sleep(SLEEP_TIME);
    return 0;
}