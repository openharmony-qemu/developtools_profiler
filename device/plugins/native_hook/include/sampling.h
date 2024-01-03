/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef __SAMPLING_H__
#define __SAMPLING_H__

#include <stdint.h>
#include <cstddef>
#include <random>
#include <pthread.h>

class Sampling {
public:
    void InitSampling(uint64_t);
    void Reset();
    uint64_t GetSampleInterval() const
    {
        return sampleInterval_;
    }
    size_t StartSampling(size_t allocSize)
    {
        return (allocSize >= sampleInterval_) ? allocSize : (CalcSamplings(allocSize) * sampleInterval_) ;
    }
    int64_t CalcNextSampleInterval()
    {
        return static_cast<int64_t>(exponentialDist_(randomEngine_)) + 1;
    }
    Sampling()
    {
        pthread_spin_init(&spinlock_, PTHREAD_PROCESS_PRIVATE);
    }
    ~Sampling()
    {
        pthread_spin_destroy(&spinlock_);
    }

    // for test
    void SetRandomEngineSeed(uint64_t cnt)
    {
        randomEngine_.seed(cnt);
    }

private:
    class Spinlock {
    public:
        Spinlock(const Spinlock&) = delete;
        Spinlock& operator=(const Spinlock&) = delete;
        Spinlock()
        {
            pthread_spin_lock(&spinlock_);
        }
        ~Spinlock()
        {
            pthread_spin_unlock(&spinlock_);
        }
    };
    size_t CalcSamplings(size_t);

    uint64_t sampleInterval_ {0};
    double sampleRate_ {0.0};
    int64_t nextSampleInterval_ {0};
    std::exponential_distribution<double> exponentialDist_;
    static inline std::default_random_engine randomEngine_;
    static inline pthread_spinlock_t spinlock_;
};
#endif  // __SAMPLING_H__