/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef BYTRACE_THREAD_STATES_H
#define BYTRACE_THREAD_STATES_H

#include <map>
#include "log.h"

namespace SysTuning {
namespace TraceStreamer {
enum Direction { NEED_GO = 0, NEED_CONTINUE, NEED_BREAK };
enum Stat : uint32_t {
    RUNNABLE = 0,
    INTERRUPTABLESLEEP = 1,
    UNINTERRUPTIBLESLEEP = 2,
    STOPPED = 4,
    TRACED = 8, // the process is being debug
    EXITDEAD = 16,
    EXITZOMBIE = 32,
    TASKDEAD = 64,
    WAKEKILL = 128,
    WAKING = 256,
    PARKED = 512,
    NOLOAD = 1024,
    TASKNEW = 2048,
    VALID = 0X8000,
};
class ThreadStateFlag {
public:
    explicit ThreadStateFlag(const std::string& stateStr);
    ~ThreadStateFlag() {}

    uint32_t State() const
    {
        return state_ & ~VALID;
    }
    bool IsValid() const
    {
        return invalid_;
    }

private:
    void SetStat(Stat value)
    {
        state_ |= value;
    }

    void ProcessSate(const std::string& stateStr);
    Direction SetStatByChar(char ch);

private:
    uint32_t state_ = 0;
    std::map<char, Stat> statMap_ = {
        {'R', RUNNABLE},
        {'S', INTERRUPTABLESLEEP},
        {'D', UNINTERRUPTIBLESLEEP},
        {'T', STOPPED},
        {'t', TRACED},
        {'X', EXITDEAD},
        {'Z', EXITZOMBIE},
        {'x', TASKDEAD},
        {'I', TASKDEAD},
        {'K', WAKEKILL},
        {'P', PARKED},
        {'N', NOLOAD},
        {'|', VALID},
    };
    bool invalid_ = false;
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // _BYTRACE_THREAD_STATES_H_
