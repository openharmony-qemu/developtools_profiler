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

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <ctime>
#include <csignal>
#include <atomic>
#include <memory>

namespace {
static int g_duration;
static int g_memSize;
static std::atomic<long long> g_times;
static std::atomic<long long> g_mallocTotalTime;
static std::atomic<long long> g_freeTotalTime;
static constexpr int N_INSTALL_MALLOC_HOOK_SIGNAL = 36;
static constexpr int DURATION_TIME = 30 * 60;
static constexpr int MEMORY_BUFFER_SIZE = 19;

void* UserThread(void* param)
{
    int idx = *static_cast<int*>(param);
    struct timespec beginTime;
    struct timespec endTime;
    struct timespec beginTimeMalloc;
    struct timespec endTimeMalloc;
    struct timespec beginTimeFree;
    struct timespec endTimeFree;

    clock_gettime(CLOCK_REALTIME, &beginTime);
    int displaySize = g_memSize > 5 ? 5 : g_memSize;
    do {
        clock_gettime(CLOCK_REALTIME, &endTime);
        if (endTime.tv_sec - beginTime.tv_sec >= g_duration) {
            break;
        }
        clock_gettime(CLOCK_REALTIME, &beginTimeMalloc);
        char* mem = (char *)malloc(g_memSize);
        if (mem == nullptr) {
            printf("Error: malloc mem memory failed.\n");
            return nullptr;
        }
        clock_gettime(CLOCK_REALTIME, &endTimeMalloc);
        std::atomic_fetch_add_explicit(
            &g_mallocTotalTime,
            static_cast<long long>((endTimeMalloc.tv_sec - beginTimeMalloc.tv_sec) * 1000000000L +
                                   (endTimeMalloc.tv_nsec - beginTimeMalloc.tv_nsec)),
            std::memory_order_relaxed);
        long long currentTimes = ++g_times;
        long long timeBase = 10000;
        if (currentTimes % timeBase == 0) {
            printf("Thread %d, %lld: ", idx + 1, currentTimes);
            for (int i = 0 ; i < displaySize ; i++) {
                printf("%d ", mem[i]);
            }
            printf("\n");
        }
        clock_gettime(CLOCK_REALTIME, &beginTimeFree);
        if (mem != nullptr) {
            free(mem);
        }
        clock_gettime(CLOCK_REALTIME, &endTimeFree);
        std::atomic_fetch_add_explicit(
            &g_freeTotalTime,
            static_cast<long long>((endTimeFree.tv_sec - beginTimeFree.tv_sec) * 1000000000L +
                                   (endTimeFree.tv_nsec - beginTimeFree.tv_nsec)),
            std::memory_order_relaxed);
    }
    while (true);
    return nullptr;
}
#define PRINTF_DATA(fileptr, fmt, ...) \
{                              \
    do { \
        fprintf(stderr, fmt, ## __VA_ARGS__); \
        fprintf(fileptr, fmt, ## __VA_ARGS__); \
        fflush(fileptr); \
    } while (0); \
}

void Usage()
{
    printf("Usage: perf_test_data <-o output_file_name> [-t threadNum[] [-d g_duration] [-s g_memSize]\n");
}

void FileClose(FILE* fp)
{
    if (fp != nullptr) {
        fclose(fp);
    }
}

} // namespace

int main(int argc, char *argv[])
{
    int threadNum = 1;
    g_duration = DURATION_TIME;
    g_memSize = MEMORY_BUFFER_SIZE;
    std::unique_ptr<FILE, void (*)(FILE*)> outFp(nullptr, nullptr);
    for (int idx = 1; idx < argc; ++idx) {
        if (strcmp(argv[idx], "-o") == 0) {
            if (idx + 1 >= argc) {
                Usage();
                return 1;
            } else {
                ++idx;
                outFp = std::unique_ptr<FILE, void (*)(FILE*)>(fopen(argv[idx], "w"), FileClose);
                if (outFp == nullptr) {
                    printf("File '%s' can't be opened.\n", argv[idx]);
                    return 1;
                }
            }
        } else if (strcmp(argv[idx], "-t") == 0) {
            if (idx + 1 >= argc) {
                Usage();
                return 1;
            } else {
                ++idx;
                threadNum = atoi(argv[idx]);
                if (threadNum <= 0) {
                    printf("Thread number can't be less or equal zero.\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[idx], "-d") == 0) {
            if (idx + 1 >= argc) {
                Usage();
                return 1;
            } else {
                ++idx;
                g_duration = atoi(argv[idx]);
                if (g_duration <= 0) {
                    printf("g_duration can't be less or equal zero.\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[idx], "-s") == 0) {
            if (idx + 1 >= argc) {
                Usage();
                return 1;
            } else {
                ++idx;
                g_memSize = atoi(argv[idx]);
                if (g_memSize <= 0) {
                    printf("memory size can't be less or equal zero.\n");
                    return 1;
                }
            }
        }
    }
    if (outFp == nullptr) {
        Usage();
        return 1;
    }
    pthread_t* thrArray = (pthread_t*)malloc(sizeof(pthread_t) * threadNum);
    if (thrArray == nullptr) {
        printf("malloc thrArray memory failed.\n");
        return 1;
    }
    int idxSituation;
    int idxSituationMax = 2;
    int pid = static_cast<int>(getpid());
    PRINTF_DATA(outFp.get(), "PID: %d, file: %d.nativehook\n", pid, pid);
    PRINTF_DATA(outFp.get(),
        "Thread number: %d, duration: %d seconds, memory size: %d bytes\n",
        threadNum,
        g_duration,
        g_memSize);
    for (idxSituation = 0; idxSituation < idxSituationMax; ++idxSituation) {
        if (idxSituation == 0) {
            PRINTF_DATA(outFp.get(), "No hook situation\n");
        } else {
            PRINTF_DATA(outFp.get(), "\nWith hook situation\n");
            raise(N_INSTALL_MALLOC_HOOK_SIGNAL);
        }
        int idx;

        std::atomic_store_explicit(&g_times, static_cast<long long>(0), std::memory_order_seq_cst);
        std::atomic_store_explicit(&g_mallocTotalTime, static_cast<long long>(0), std::memory_order_seq_cst);
        std::atomic_store_explicit(&g_freeTotalTime, static_cast<long long>(0), std::memory_order_seq_cst);

        for (idx = 0; idx < threadNum; ++idx) {
            if (pthread_create(thrArray + idx, nullptr, UserThread, reinterpret_cast<void*>(&idx)) != 0) {
                printf("Creating thread failed.\n");
            }
        }

        for (idx = 0; idx < threadNum; ++idx) {
            pthread_join(thrArray[idx], nullptr);
        }
        long long totalTimes = g_times.load(std::memory_order_relaxed);
        PRINTF_DATA(outFp.get(), "The total g_times(malloc/free): %lld\n", totalTimes);
    }
    if (thrArray != nullptr) {
        free(thrArray);
    }
    printf("Exit\n");
}
