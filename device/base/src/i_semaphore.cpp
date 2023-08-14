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
#include "i_semaphore.h"
#include "posix_semaphore.h"
#include "std_semaphore.h"

ISemaphoreFactory& GetSemaphoreFactory(SemaphoreFactoryType type)
{
    static StdSemaphoreFactory stdFactory;
    if (type == STD_SEMAPHORE_FACTORY) {
        return stdFactory;
    } else if (type == POSIX_SEMAPHORE_FACTORY) {
        static PosixSemaphoreFactory posxiFactory;
        return posxiFactory;
    } else if (type == PTHREAD_SEMAPHORE_FACTORY) {
        // static PthreadSemaphoreFactory pthreadFactory;
        return stdFactory;
    }
    return stdFactory;
}