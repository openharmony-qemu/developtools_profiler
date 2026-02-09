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

#ifndef PERFORMANCE_TRACKER_H
#define PERFORMANCE_TRACKER_H

#include <sys/time.h>
#include <cstdint>
#include <atomic>

enum class OperatType : uint8_t {
    malloc = 0,
    aligned_alloc,
    calloc,
    realloc,
    free,
    mmap,
    munmap,
    prctl,
    memtrace,
    resTraceMove,
    resTraceFreeRegion,
    restrace,
    other,
};

#ifdef PERFORMANCE_DEBUG

class PerformanceTracker {
public:
    PerformanceTracker(OperatType type);
    ~PerformanceTracker();
    void RecordDataSize(size_t stackSize, size_t mallocSize);
    void RefreshStartTime();
    
    static void ResetPerfInfo();
    static void PrintResult();

private:
    struct timespec start_;

    static std::atomic<uint64_t> mallocTimes_;
    static std::atomic<uint64_t> mallocRecordTimes_;
    static std::atomic<uint64_t> timeCost_;
    static std::atomic<uint64_t> dataCounts_;
    static std::atomic<uint64_t> mallocSize_;
};

#define PERF_TRACK(type) PerformanceTracker __perf_tracker__(type)
#define PERF_RECORD_DATA(stackSize, mallocSize) __perf_tracker__.RecordDataSize(stackSize, mallocSize)
#define PERF_REFRESH_START_TIME() __perf_tracker__.RefreshStartTime()
#define PERF_RESET() PerformanceTracker::ResetPerfInfo()
#define PERF_PRINT_RESULTS() PerformanceTracker::PrintResult()

#else

#define PERF_TRACK(type) do {} while (0)
#define PERF_RECORD_DATA(stackSize, mallocSize) do {} while (0)
#define PERF_REFRESH_START_TIME()  do {} while (0)
#define PERF_RESET() do {} while (0)
#define PERF_PRINT_RESULTS() do {} while (0)

#endif  // PERFORMANCE_DEBUG

#endif  // PERFORMANCE_TRACKER_H
