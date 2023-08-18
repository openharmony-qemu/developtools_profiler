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
#include "posix_semaphore.h"
#include <memory>
#include <ctime>

namespace {
constexpr int NS_PER_SEC = 1000 * 1000 * 1000;
}

PosixSemaphore::PosixSemaphore(unsigned int value)
{
    sem_init(&sem_, 0, value);
}

PosixSemaphore::~PosixSemaphore()
{
    sem_destroy(&sem_);
}

bool PosixSemaphore::Wait()
{
    return sem_wait(&sem_) == 0;
}

bool PosixSemaphore::TryWait()
{
    return sem_trywait(&sem_) == 0;
}

bool PosixSemaphore::TimedWait(int seconds, int nanoSeconds)
{
    struct timespec ts = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += seconds;
    ts.tv_nsec += nanoSeconds;
    ts.tv_sec += ts.tv_nsec / NS_PER_SEC;
    ts.tv_nsec %= NS_PER_SEC;
    return sem_timedwait(&sem_, &ts) == 0;
}

bool PosixSemaphore::Post()
{
    return sem_post(&sem_) == 0;
}

unsigned PosixSemaphore::Value() const
{
    int value = 0;
    sem_getvalue(&sem_, &value);
    return value;
}

SemaphorePtr PosixSemaphoreFactory::Create(unsigned int value)
{
    return std::make_shared<PosixSemaphore>(value);
}
