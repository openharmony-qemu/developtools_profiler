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
    bool InitSampling(uint64_t);
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
    Sampling() {}
    ~Sampling() {}

    // for test
    void SetRandomEngineSeed(uint64_t cnt)
    {
        randomEngine_.seed(cnt);
    }

private:
    size_t CalcSamplings(size_t);
    uint64_t sampleInterval_ {0};
    double sampleRate_ {0.0};
    int64_t nextSampleInterval_ {0};
    bool isInit_{false};
    std::exponential_distribution<double> exponentialDist_;
    static std::default_random_engine randomEngine_;
};
#endif  // __SAMPLING_H__