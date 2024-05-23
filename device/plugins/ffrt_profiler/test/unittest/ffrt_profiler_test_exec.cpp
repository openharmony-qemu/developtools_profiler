/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#include <iostream>
#include <cstdio>
#include <ctime>
#include <unistd.h>

#include "ffrt.h"

#define NSEC_PER_SEC 1000000000L
namespace {
constexpr int TWO_NUM = 2;
constexpr int VALUE_NUM = 100;

static long GetNanos()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * NSEC_PER_SEC + ts.tv_nsec;
}
} // namespace

int main(int argc, char *argv[])
{
    if (argc != TWO_NUM) {
        return 0;
    }
    int i = 0;
    for (; i < TWO_NUM; i++) {
        std::cout << "hook" << std::endl;
        sleep(TWO_NUM);
        ffrt::submit([i] { std::cout << "num: " << i << std::endl; });
    }
    ffrt::wait();

    long startTime = 0;
    long endTime = 0;
    long elapsedTime = 0;
    std::cout << "start" << std::endl;
    startTime = GetNanos();
    for (int j = 0; j < std::atoi(argv[1]); ++j) {
        int x = 1;
        ffrt::submit([&] { x = VALUE_NUM; }, {}, {&x});
    }
    ffrt::wait();
    endTime = GetNanos();
    elapsedTime = endTime - startTime;
    printf("Elapsed time: %ld nanoseconds, mean time: %ld\n", elapsedTime, elapsedTime / std::atoi(argv[1]));
    return 0;
}