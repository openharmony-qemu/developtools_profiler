/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <dlfcn.h>
#include <fcntl.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "memory_trace.h"
#include "securec.h"

#pragma clang optimize off

namespace {
typedef char* (*DepthMallocSo)(int depth, int mallocSize);
typedef void (*DepthFreeSo)(int depth, char *p);
constexpr int MALLOC_SIZE = 1000;
constexpr int ARGC_NUM_MAX = 3;
constexpr int DOUBLE = 2;

constexpr int USLEEP_TIME = 1000;
// liba.z.so and libb.z.so for same so.
#ifdef __arm__
const std::vector<std::string> VEC_SO_PATH { "/system/lib/liba.z.so", "/system/lib/libb.z.so" };
#else
const std::vector<std::string> VEC_SO_PATH { "/system/lib64/liba.z.so", "/system/lib64/libb.z.so" };
#endif
unsigned int g_stickDepth = 1;
}

static void CallocFun()
{
    static int i = 0;
    char* ptr = static_cast<char*>(calloc(1, MALLOC_SIZE / 100));
    if (ptr == nullptr) {
        fprintf(stderr, "calloc err.\n");
        return;
    }
    fprintf(stderr, "calloc i=%d\n", i);
    free(ptr);
    i++;
}

static void ReallocFun()
{
    static int i = 0;
    char* ptr = static_cast<char*>(calloc(1, MALLOC_SIZE / 10)); // 10: multiple num
    if (ptr == nullptr) {
        fprintf(stderr, "calloc err.\n");
        return;
    }
    ptr = static_cast<char*>(realloc(ptr, MALLOC_SIZE * 10)); // 10: multiple num
    if (ptr == nullptr) {
        fprintf(stderr, "realloc err.\n");
        return;
    }
    fprintf(stderr, "realloc i=%d\n", i);
    free(ptr);
    i++;
}

static bool DepthMallocFree(int depth = 0, int mallocSize = 100)
{
    if (depth < 0 || mallocSize <= 0) {
        return false;
    }
    if (depth == 0) {
        char* ptr = static_cast<char*>(malloc(mallocSize));
        if (ptr == nullptr) {
            fprintf(stderr, "malloc err.\n");
            return false;
        }
        fprintf(stderr, "%s\n", __func__);
        *ptr = 'a';
        free(ptr);
        return true;
    }
    return (DepthMallocFree(depth - 1, mallocSize));
}

static void DlopenAndCloseSo(const std::string& filePath, int size)
{
    void* handle = nullptr;
    DepthMallocSo mallocFunc = nullptr;
    DepthFreeSo freeFunc = nullptr;

    fprintf(stderr, "dlopen %s %d!!!\n", filePath.data(), size);
    usleep(USLEEP_TIME * 300); // 300 ms
    handle = dlopen(filePath.data(), RTLD_LAZY);
    if (handle == nullptr) {
        fprintf(stderr, "library not exist!\n");
        exit(0);
    }
    mallocFunc = (DepthMallocSo)dlsym(handle, "DepthMallocSo");
    freeFunc = (DepthFreeSo)dlsym(handle, "DepthFreeSo");
    if (mallocFunc == nullptr || freeFunc == nullptr) {
        fprintf(stderr, "function not exist!\n");
        exit(0);
    }
    char* ptr = nullptr;
    for (size_t i = 0; i < 20; i++) { // 20: loop count
        ptr = mallocFunc(g_stickDepth, size);
        *ptr = 'a';
        freeFunc(g_stickDepth, ptr);
    }
    if (handle != nullptr) {
        usleep(USLEEP_TIME * 300); // 300 ms
        dlclose(handle);
    }
}

static int MmapAndMunMap()
{
    size_t size = (1024);

    char* ptr = static_cast<char*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "Mmap err.\n");
        ptr = nullptr;
        return 1;
    }

    (void)memset_s(ptr, size, 0, size);
    munmap(ptr, size);
    return 0;
}

static void Fun1()
{
    static int i = 0;
    char* ptr = static_cast<char*>(malloc(MALLOC_SIZE));
    if (ptr == nullptr) {
        fprintf(stderr, "malloc err.\n");
        return;
    }
    fprintf(stderr, "i=%d\n", i);
    *ptr = 'a';
    free(ptr);
    i++;
    MmapAndMunMap();
    CallocFun();
    ReallocFun();
}

static void Fun2()
{
    Fun1();
    static int i = 0;
    char *ptr = static_cast<char*>(malloc(MALLOC_SIZE));
    if (ptr == nullptr) {
        fprintf(stderr, "malloc err.\n");
        return;
    }
    fprintf(stderr, "i=%d\n", i);
    *ptr = 'a';
    if (i % DOUBLE == 0) {
        free(ptr);
    }
    i++;
}

static void Fun3()
{
    Fun2();
    static int i = 0;
    char *ptr = static_cast<char*>(malloc(MALLOC_SIZE));
    if (ptr == nullptr) {
        fprintf(stderr, "malloc err.\n");
        return;
    }
    fprintf(stderr, "i=%d\n", i);
    *ptr = 'a';
    if (i % DOUBLE == 0) {
        free(ptr);
    }
    i++;
    DlopenAndCloseSo(VEC_SO_PATH[0], MALLOC_SIZE * DOUBLE);
}

static void Fun4()
{
    Fun3();
    static int i = 0;
    char *ptr = static_cast<char*>(malloc(MALLOC_SIZE));
    if (ptr == nullptr) {
        fprintf(stderr, "malloc err.\n");
        return;
    }
    fprintf(stderr, "i=%d\n", i);
    *ptr = 'a';
    if (i % DOUBLE == 0) {
        free(ptr);
    }
    i++;
}

static void Fun5()
{
    Fun4();
    static int i = 0;
    char *ptr = static_cast<char*>(malloc(MALLOC_SIZE));
    if (ptr == nullptr) {
        fprintf(stderr, "malloc err.\n");
        return;
    }
    fprintf(stderr, "i=%d\n", i);
    *ptr = 'a';
    if (i % DOUBLE == 0) {
        free(ptr);
    }
    i++;
    DepthMallocFree(g_stickDepth * 30); // 30: depth count
    DlopenAndCloseSo(VEC_SO_PATH[1], MALLOC_SIZE * 3); // 3: multiple num
}

static void* HhreadFuncCpp(void* param)
{
    std::string name = "thread";
    name = name + std::to_string(gettid());
    prctl(PR_SET_NAME, name.c_str());
    int forNum = *static_cast<int*>(param);
    for (int num = 0; num < forNum; num++) {
        fprintf(stderr, "thread %d:num=%d\n", gettid(), num);
        Fun5();
    }
    return nullptr;
}

static void TestMemoryMap()
{
    constexpr int smallSize = 4096;
    constexpr int bigSize = 8192;
    int fd = open("/bin/hiebpf", O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        printf("open %s failed\n", "/bin/hiebpf");
        return;
    }

    void* mapAddr1 = mmap(nullptr, smallSize, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_POPULATE, fd, 0);
    if (mapAddr1 == MAP_FAILED) {
        printf("named mmap failed\n");
        close(fd);
        return;
    }
    printf("named mmap size: 4096, fd: %d\n", fd);

    void* mapAddr2 = mmap(nullptr, bigSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mapAddr2 == MAP_FAILED) {
        printf("anonymous mmap failed\n");
        close(fd);
        munmap(mapAddr1, smallSize);
        return;
    }
    printf("anonymous mmap size: 8192\n");

    memtrace(reinterpret_cast<void*>(0x123456), 3333, "memtrace_test", true); // 3333 num
    printf("memtrace(0x123456, 3333, \"memtrace_test\", true)\n");

    memtrace(reinterpret_cast<void*>(0x123456), 3333, "memtrace_test", false); // 3333 num
    printf("memtrace(0x123456, 3333, \"memtrace_test\", false)\n");
    close(fd);
    munmap(mapAddr1, smallSize);
    munmap(mapAddr2, bigSize);
}

int main(int argc, char *argv[])
{
    int threadNum = 1;
    int forNum = 10;
    if  (argc == ARGC_NUM_MAX) {
        if (atoi(argv[1]) > 0 && atoi(argv[1]) <= 10) { // 10: max threads
            threadNum = atoi(argv[1]);
        }
        if (atoi(argv[2]) > 0 && atoi(argv[2]) <= 100) { // 2: args num. 100: max value
            forNum = atoi(argv[2]); // 2: args num
        }
    } else if (argc > ARGC_NUM_MAX) {
        printf("command error, argc must <= %d\n", ARGC_NUM_MAX);
        return 0;
    }
    fprintf(stderr, "start.Enter or send signal for next.\n");
    getchar();
    TestMemoryMap();

    fprintf(stderr, "forNum = %d, threadNum = %d\n", forNum, threadNum);
    fprintf(stderr, "Notice: need copy libnativetest_so.z.so for %s, %s\n",
            VEC_SO_PATH[0].data(), VEC_SO_PATH[1].data());
    pthread_t* thrArray = new (std::nothrow) pthread_t[threadNum];
    if (!thrArray) {
        printf("new thread array failed.\n");
        return 1;
    }
    int idx;
    for (idx = 0; idx < threadNum; ++idx) {
        if (pthread_create(thrArray + idx, nullptr, HhreadFuncCpp, static_cast<void*>(&forNum)) != 0) {
            printf("Creating thread failed.\n");
        }
    }
    for (idx = 0; idx < threadNum; ++idx) {
        pthread_join(thrArray[idx], nullptr);
    }
    delete []thrArray;

    fprintf(stderr, "end.\n");
    return 0;
}

#pragma clang optimize on