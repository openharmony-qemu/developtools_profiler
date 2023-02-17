/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
struct PerfData {
    long long malloc_total_time;
    long long free_total_time;
    long long times;
};

static int duration;
static int mem_size;
static std::vector<PerfData> thread_data;
static std::atomic<long long> times;
static std::atomic<long long> malloc_total_time;
static std::atomic<long long> free_total_time;
static constexpr int nInstallMallocHookSignal = 36;
static constexpr int g_duration = 30 * 60;
static constexpr int g_memSize = 19;

void* user_thread(void* param)
{
    int idx = *static_cast<int*>(param);
    struct timespec begin_time;
    struct timespec end_time;
    struct timespec begin_time_malloc;
    struct timespec end_time_malloc;
    struct timespec begin_time_free;
    struct timespec end_time_free;

    clock_gettime(CLOCK_REALTIME, &begin_time);
    int display_size = mem_size > 5? 5 : mem_size;
    do {
        clock_gettime(CLOCK_REALTIME, &end_time);
        if (end_time.tv_sec - begin_time.tv_sec >= duration) {
            break;
        }
        clock_gettime(CLOCK_REALTIME, &begin_time_malloc);
        char* mem = (char *)malloc(mem_size);
        clock_gettime(CLOCK_REALTIME, &end_time_malloc);
        std::atomic_fetch_add_explicit(
            &malloc_total_time,
            static_cast<long long>((end_time_malloc.tv_sec - begin_time_malloc.tv_sec) * 1000000000L +
                                   (end_time_malloc.tv_nsec - begin_time_malloc.tv_nsec)),
            std::memory_order_relaxed);
        long long current_times = ++times;
        long long timeBase = 10000;
        if (current_times % timeBase == 0) {
            printf("Thread %d, %lld: ", idx + 1, current_times);
            for (int i = 0 ; i < display_size ; i++) {
                printf("%d ", mem[i]);
            }
            printf("\n");
        }
        clock_gettime(CLOCK_REALTIME, &begin_time_free);
        free(mem);
        clock_gettime(CLOCK_REALTIME, &end_time_free);
        std::atomic_fetch_add_explicit(
            &free_total_time,
            static_cast<long long>((end_time_free.tv_sec - begin_time_free.tv_sec) * 1000000000L +
                                   (end_time_free.tv_nsec - begin_time_free.tv_nsec)),
            std::memory_order_relaxed);
    }
    while (1);
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

void usage()
{
    printf("Usage: perf_test_data <-o output_file_name> [-t thread_num[] [-d duration] [-s mem_size]\n");
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
    int thread_num = 1;
    duration = g_duration;
    mem_size = g_memSize;
    std::unique_ptr<FILE, void (*)(FILE*)> out_fp(nullptr, nullptr);
    for (int idx = 1; idx < argc; ++idx) {
        if (strcmp(argv[idx], "-o") == 0) {
            if (idx + 1 >= argc) {
                usage();
                return 1;
            } else {
                ++idx;
                out_fp = std::unique_ptr<FILE, void (*)(FILE*)>(fopen(argv[idx], "w"), FileClose);
                if (out_fp == nullptr) {
                    printf("File '%s' can't be opened.\n", argv[idx]);
                    return 1;
                }
            }
        } else if (strcmp(argv[idx], "-t") == 0) {
            if (idx + 1 >= argc) {
                usage();
                return 1;
            } else {
                ++idx;
                thread_num = atoi(argv[idx]);
                if (thread_num <= 0) {
                    printf("Thread number can't be less or equal zero.\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[idx], "-d") == 0) {
            if (idx + 1 >= argc) {
                usage();
                return 1;
            } else {
                ++idx;
                duration = atoi(argv[idx]);
                if (duration <= 0) {
                    printf("duration can't be less or equal zero.\n");
                    return 1;
                }
            }
        } else if (strcmp(argv[idx], "-s") == 0) {
            if (idx + 1 >= argc) {
                usage();
                return 1;
            } else {
                ++idx;
                mem_size = atoi(argv[idx]);
                if (mem_size <= 0) {
                    printf("memory size can't be less or equal zero.\n");
                    return 1;
                }
            }
        }
    }
    if (out_fp == nullptr) {
        usage();
        return 1;
    }

    thread_data.resize(thread_num);
    pthread_t* thr_array = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);
    int idxSituation;
    int idxSituationMax = 2;
    int pid = static_cast<int>(getpid());
    PRINTF_DATA(out_fp.get(), "PID: %d, file: %d.nativehook\n", pid, pid);
    PRINTF_DATA(out_fp.get(), "Thread number: %d, duration: %d seconds, memory size: %d bytes\n", thread_num, duration,
                mem_size);
    for (idxSituation = 0; idxSituation < idxSituationMax; ++idxSituation) {
        if (idxSituation == 0) {
            PRINTF_DATA(out_fp.get(), "No hook situation\n");
        } else {
            PRINTF_DATA(out_fp.get(), "\nWith hook situation\n");
            raise(nInstallMallocHookSignal);
        }
        int idx;

        std::atomic_store_explicit(&times, static_cast<long long>(0), std::memory_order_seq_cst);
        std::atomic_store_explicit(&malloc_total_time, static_cast<long long>(0), std::memory_order_seq_cst);
        std::atomic_store_explicit(&free_total_time, static_cast<long long>(0), std::memory_order_seq_cst);

        for (idx = 0; idx < thread_num; ++idx) {
            if (pthread_create(thr_array + idx, nullptr, user_thread, reinterpret_cast<void*>(&idx)) != 0) {
                printf("Creating thread failed.\n");
            }
        }

        for (idx = 0; idx < thread_num; ++idx) {
            pthread_join(thr_array[idx], nullptr);
        }
        long long total_times = times.load(std::memory_order_relaxed);
        PRINTF_DATA(out_fp.get(), "The total times(malloc/free): %lld\n", total_times);
    }
    free(thr_array);
    printf("Exit\n");
}
