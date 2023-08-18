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
#ifndef OHOS_PROFILER_ABSTRACT_SEMAPHORE_H
#define OHOS_PROFILER_ABSTRACT_SEMAPHORE_H

#include <memory>
#include <string>

class ISemaphore {
public:
    virtual ~ISemaphore() {}
    virtual bool Wait() = 0;
    virtual bool TryWait() = 0;
    bool TimedWait(int seconds)
    {
        return TimedWait(seconds, 0);
    }
    virtual bool TimedWait(int seconds, int nanoSeconds) = 0;
    virtual bool Post() = 0;
    virtual unsigned int Value() const = 0;
};
using SemaphorePtr = std::shared_ptr<ISemaphore>;

enum SemaphoreFactoryType : int { STD_SEMAPHORE_FACTORY, POSIX_SEMAPHORE_FACTORY, PTHREAD_SEMAPHORE_FACTORY };

class ISemaphoreFactory {
public:
    virtual SemaphorePtr Create(unsigned int value) = 0;
};

ISemaphoreFactory& GetSemaphoreFactory(SemaphoreFactoryType type = STD_SEMAPHORE_FACTORY);

#endif // OHOS_PROFILER_ABSTRACT_SEMAPHORE_H