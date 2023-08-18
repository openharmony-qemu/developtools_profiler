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
#ifndef OHOS_PROFILER_POSIX_SEMAPHORE_H
#define OHOS_PROFILER_POSIX_SEMAPHORE_H

#include <semaphore.h>
#include "i_semaphore.h"

class PosixSemaphore : public ISemaphore {
public:
    explicit PosixSemaphore(unsigned int value);
    ~PosixSemaphore();

    bool Wait() override;
    bool TryWait() override;
    bool TimedWait(int seconds, int nanoSeconds) override;
    bool Post() override;
    unsigned Value() const override;

private:
    mutable sem_t sem_ = {};
};

class PosixSemaphoreFactory : public ISemaphoreFactory {
public:
    SemaphorePtr Create(unsigned int value) override;
};

#endif // OHOS_PROFILER_POSIX_SEMAPHORE_H