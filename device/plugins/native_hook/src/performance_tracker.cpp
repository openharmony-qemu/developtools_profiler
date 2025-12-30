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

#include "performance_tracker.h"

#ifdef PERFORMANCE_DEBUG

#include <atomic>
#include <cinttypes>
#include "logging.h"
#include "hook_client.h"

constexpr int PRINT_INTERVAL = 5000;
constexpr uint64_t S_TO_NS = 1000 * 1000 * 1000;

std::atomic<uint64_t> PerformanceTracker::mallocTimes_ = 0;
std::atomic<uint64_t> PerformanceTracker::mallocRecordTimes_ = 0;
std::atomic<uint64_t> PerformanceTracker::timeCost_ = 0;
std::atomic<uint64_t> PerformanceTracker::dataCounts_ = 0;
std::atomic<uint64_t> PerformanceTracker::mallocSize_ = 0;
std::map<OperatType, uint64_t> PerformanceTracker::mallocCountMap_;
std::mutex g_mutex;

PerformanceTracker::PerformanceTracker(OperatType type)
{
    RefreshStartTime();
    std::unique_lock<std::mutex> lock(g_mutex);
    mallocCountMap_[type]++;
}

PerformanceTracker::~PerformanceTracker()
{
    struct timespec end = {};
    clock_gettime(CLOCK_REALTIME, &end);

    uint64_t elapsed = (end.tv_sec - start_.tv_sec) * S_TO_NS + (end.tv_nsec - start_.tv_nsec);
    timeCost_.fetch_add(elapsed);
    mallocTimes_++;
}

void PerformanceTracker::RecordDataSize(size_t stackSize, size_t mallocSize)
{
    dataCounts_.fetch_add(stackSize);
    mallocSize_.fetch_add(mallocSize);
    mallocRecordTimes_++;
}

void PerformanceTracker::RefreshStartTime()
{
    clock_gettime(CLOCK_REALTIME, &start_);
}

void PerformanceTracker::ResetPerfInfo()
{
    mallocTimes_ = 0;
    mallocRecordTimes_ = 0;
    mallocSize_ = 0;
    timeCost_ = 0;
    dataCounts_ = 0;
    std::unique_lock<std::mutex> lock(g_mutex);
    mallocCountMap_.clear();
}

void PerformanceTracker::PrintResult()
{
    if (mallocTimes_.load() == 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "no malloc operation!");
    }
    PROFILER_LOG_ERROR(LOG_CORE,
        "mallocTimes: %" PRIu64" mallocRecordTimes: %" PRIu64" mallocSize: %" PRIu64""
        " cost time: %" PRIu64" copy data bytes: %" PRIu64" mean cost: %" PRIu64"\n",
        mallocTimes_.load(), mallocRecordTimes_.load(), mallocSize_.load(),
        timeCost_.load(), dataCounts_.load(), timeCost_.load() / mallocTimes_.load());
    std::unique_lock<std::mutex> lock(g_mutex);
    for (const auto& [key, value] : PerformanceTracker::mallocCountMap_) {
        PROFILER_LOG_ERROR(LOG_CORE, "%hhu: %" PRIu64 "\n", key, value);
    }
}

#endif  // PERFORMANCE_DEBUG
