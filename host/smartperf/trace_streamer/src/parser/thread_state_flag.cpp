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

#include "thread_state_flag.h"

namespace SysTuning {
namespace TraceStreamer {
Direction ThreadStateFlag::SetStatByChar(char ch)
{
    if (ch == 'R') {
        if (state_ == 0) {
            return NEED_CONTINUE;
        }
    }

    if (statMap_.find(ch) == statMap_.end()) {
        return NEED_BREAK;
    }

    state_ |= statMap_[ch];
    return NEED_GO;
}

void ThreadStateFlag::ProcessSate(const std::string& stateStr)
{
    for (size_t i = 0; i < stateStr.size(); i++) {
        if (stateStr[i] == '+') {
            invalid_ = true;
            SetStat(TASKNEW);
            continue;
        }

        Direction ret = SetStatByChar(stateStr[i]);
        if (ret == NEED_CONTINUE) {
            invalid_ = true;
            continue;
        } else if (ret == NEED_BREAK) {
            break;
        }
    }
}

ThreadStateFlag::ThreadStateFlag(const std::string& stateStr)
{
    ProcessSate(stateStr);
}
} // namespace TraceStreamer
} // namespace SysTuning
