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

#ifdef PERFORMANCE_DEBUG

class PerformanceTracker {
public:
    PerformanceTracker();
    ~PerformanceTracker();
    void RecordDataSize(size_t bytes);
    void RefreshStartTime();
    
    static void ResetPerfInfo();

private:
    struct timespec start_;

    static std::atomic<uint64_t> mallocTimes_ = 0;
    static std::atomic<uint64_t> timeCost_ = 0;
    static std::atomic<uint64_t> dataCounts_ = 0;
};

#define PERF_TRACK() PerformanceTracker __perf_tracker__
#define PERF_RECORD_DATA(bytes) __perf_tracker__.RecordDataSize(bytes)
#define PERF_REFRESH_START_TIME() __perf_tracker__.RefreshStartTime()
#define PERF_RESET() PerformanceTracker::ResetPerfInfo()

#else

#define PERF_TRACK() do {} while (0)
#define PERF_RECORD_DATA(bytes) do {} while (0)
#define PERF_REFRESH_START_TIME()  do {} while (0)
#define PERF_RESET() do {} while (0)

#endif  // PERFORMANCE_DEBUG

#endif  // PERFORMANCE_TRACKER_H
