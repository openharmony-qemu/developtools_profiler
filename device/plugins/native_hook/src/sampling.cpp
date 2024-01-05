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

#include "sampling.h"

void Sampling::Reset()
{
    sampleInterval_ = 0;
    sampleRate_ = 0;
    nextSampleInterval_ = 0;
    exponentialDist_.reset();
}

void Sampling::InitSampling(uint64_t sampleInterval)
{
    sampleInterval_ = sampleInterval;
    sampleRate_ = 1.0 / static_cast<double>(sampleInterval_);
    exponentialDist_ = std::exponential_distribution<double>(sampleRate_);
    nextSampleInterval_ = CalcNextSampleInterval();
}

size_t Sampling::CalcSamplings(size_t allocSize)
{
    size_t counts = 0;
    {
        Spinlock s;
        for (nextSampleInterval_ -= static_cast<int64_t>(allocSize); nextSampleInterval_ <= 0;
            nextSampleInterval_ += CalcNextSampleInterval()) {
            ++counts;
        }
    }
    return counts;
}