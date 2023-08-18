/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <thread>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#pragma clang optimize off

namespace {
typedef char* (*DepthMallocSo)(int depth, int mallocSize);
typedef void (*DepthFreeSo)(int depth, char *p);
constexpr int MALLOC_SIZE = 1000;
constexpr int DATA_SIZE = 200;
constexpr int SLEEP_TIME = 200;
constexpr int ARGC_NUM_MAX = 4;
constexpr int ARGC_NUM_MUST = 3;
constexpr int ARGC_MALLOC_TIMES = 2;
constexpr int ARGC_STICK_DEPTH = 3;
unsigned int g_stickDepth = 100;

const static char SO_PATH[] = "libnativetest_so.z.so";
using StaticSpace = struct {
    int data[DATA_SIZE];
};

class Timer {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    Timer() : startTime_(Now()) {}

    ~Timer() {}

    long ElapsedUs()
    {
        auto currentTime = Now();
        return std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime_).count();
    }

    void Reset()
    {
        startTime_ = Now();
    }

protected:
    TimePoint Now()
    {
        return Clock::now();
    }

private:
    TimePoint startTime_;
};

char *DepthMalloc(int depth, int mallocSize)
{
    if (mallocSize <= 0) {
        return nullptr;
    }
    StaticSpace staticeData;
    if (depth == 0) {
        staticeData.data[0] = 1;
        return new char[mallocSize];
    }
    return (DepthMalloc(depth - 1, mallocSize));
}

void DepthFree(int depth, char *p)
{
    StaticSpace staticeData;
    if (depth == 0) {
        staticeData.data[0] = 1;
        delete []p;
        return;
    }
    return (DepthFree(depth - 1, p));
}

void* ThreadFuncCpp(void* param)
{
    char *p = nullptr;
    long tid = syscall(SYS_gettid);
    printf("start thread %ld\n", tid);
    int times = *static_cast<int*>(param);
    int idx = 0;
    std::string name = "thread";
    name = name + std::to_string(times);
    prctl(PR_SET_NAME, name.c_str());

    constexpr int timeBase = 100;
    while (idx < times) {
        p = DepthMalloc(g_stickDepth, MALLOC_SIZE);
        if (idx % timeBase == 0) {
            printf("thread %ld malloc %d times\n", tid, idx);
        }
        if (p) {
            DepthFree(g_stickDepth, p);
        }
        idx++;
    }
    return nullptr;
}

void* ThreadFuncCppHook(void* param)
{
    char *p = nullptr;
    long tid = syscall(SYS_gettid);
    printf("start thread %ld\n", tid);
    int times = *static_cast<int*>(param);
    int idx = 0;
    std::string name = "thread";
    name = name + std::to_string(times);
    prctl(PR_SET_NAME, name.c_str());
    constexpr int timeBase = 1000;
    void* handle = nullptr;
    DepthMallocSo mallocFunc = DepthMalloc;
    DepthFreeSo freeFunc = DepthFree;

    constexpr unsigned int dlopenTrigger = 30000;
    while (idx < times) {
        if (idx == static_cast<int>(dlopenTrigger)) {
            printf("dlopen!!!\n");
            handle = dlopen(SO_PATH, RTLD_LAZY);
            if (handle == nullptr) {
                printf("library not exist!\n");
                exit(0);
            }
            mallocFunc = (DepthMallocSo)dlsym(handle, "DepthMallocSo");
            freeFunc = (DepthFreeSo)dlsym(handle, "DepthFreeSo");
            if (mallocFunc == nullptr || freeFunc == nullptr) {
                printf("function not exist!\n");
                exit(0);
            }
        }
        p = mallocFunc(g_stickDepth, MALLOC_SIZE);
        if (idx % timeBase == 0) {
            printf("thread %ld malloc %d times\n", tid, idx);
        }
        if (p) {
            freeFunc(g_stickDepth, p);
        }
        idx++;
    }
    if (handle != nullptr) {
        dlclose(handle);
    }
    return nullptr;
}

int ThreadTimeCost(int threadNum, int mallocTimes)
{
    Timer timer = {};
    if (threadNum <= 0) {
        printf("threadNum less than or equal to 0.\n");
        return 1;
    }
    pthread_t* thrArray = new (std::nothrow) pthread_t[threadNum];
    if (!thrArray) {
        printf("new thread array failed.\n");
        return 1;
    }
    int idx;
    for (idx = 0; idx < threadNum; ++idx) {
        if (pthread_create(thrArray + idx, nullptr, ThreadFuncCpp, static_cast<void*>(&mallocTimes)) != 0) {
            printf("Creating thread failed.\n");
        }
    }
    for (idx = 0; idx < threadNum; ++idx) {
        pthread_join(thrArray[idx], nullptr);
    }
    delete []thrArray;
    auto timeCost = timer.ElapsedUs();
    printf("Before hook, time cost %ldus.\nAfter hook test sleeping 200 ......., please send signal\n", timeCost);
    sleep(SLEEP_TIME);
    printf("Hook test start\n");
    Timer hooktimer = {};
    pthread_t* thrArrayHook = new (std::nothrow) pthread_t[threadNum];
    if (!thrArrayHook) {
        printf("new thread lock array failed.\n");
        return 1;
    }
    for (idx = 0; idx < threadNum; ++idx) {
        if (pthread_create(thrArrayHook + idx, nullptr, ThreadFuncCppHook, static_cast<void*>(&mallocTimes)) !=
            0) {
            printf("Creating thread failed.\n");
        }
    }
    for (idx = 0; idx < threadNum; ++idx) {
        pthread_join(thrArrayHook[idx], nullptr);
    }
    delete []thrArrayHook;
    auto hookCost = hooktimer.ElapsedUs();
    printf("After hook, time cost %ldus.\nPerformance test finish!", hookCost);
    return 0;
}
} // namespace

int main(int argc, char *argv[])
{
    int threadNum = 1;
    int mallocTimes = 0;
    if  (argc >= ARGC_NUM_MUST) {
        if (atoi(argv[1]) > 0) {
            threadNum = atoi(argv[1]);
        }
        mallocTimes = atoi(argv[ARGC_MALLOC_TIMES]);
        if (argc == ARGC_NUM_MAX) {
            g_stickDepth = static_cast<unsigned int>(atoi(argv[ARGC_STICK_DEPTH]));
        }
    } else {
        printf("command error\n");
        return 0;
    }
    printf("Test start %d thread, malloc %d times\n", threadNum, mallocTimes);
    if (!ThreadTimeCost(threadNum, mallocTimes)) {
        printf("Test success end!\n");
    } else {
        printf("Test failure end!\n");
    }
}

#pragma clang optimize on
