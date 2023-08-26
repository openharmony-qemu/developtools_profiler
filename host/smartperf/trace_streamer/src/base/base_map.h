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

#ifndef SRC_TRACE_BASE_MAP_H
#define SRC_TRACE_BASE_MAP_H

#define MAP_ERASE_COMMON(...)                           \
    void Erase(__VA_ARGS__)                             \
    {                                                   \
        auto streamIdHookidMap = internalMap_.find(t1); \
        if (streamIdHookidMap != internalMap_.end()) {

#define MAP_ERASE_WITH_SINGLE_PARME(T1)    \
    MAP_ERASE_COMMON(T1 t1)                \
    internalMap_.erase(streamIdHookidMap); \
    }                                      \
    }

#define MAP_ERASE_WITH_DOUBLE_PARME(T1, T2) \
    MAP_ERASE_COMMON(T1 t1, T2 t2)          \
    (*streamIdHookidMap).second.Erase(t2);  \
    }                                       \
    }

#define DOUBLE_MAP_ERASE_WITH_DOUBLE_PARME(T1, T2)      \
    MAP_ERASE_COMMON(T1 t1, T2 t2)                      \
    auto hookId = (*streamIdHookidMap).second.find(t2); \
    if (hookId != (*streamIdHookidMap).second.end()) {  \
        (*streamIdHookidMap).second.erase(hookId);      \
    }                                                   \
    }                                                   \
    }

#define MAP_ERASE_WITH_TRIPLE_PARME(T1, T2, T3) \
    MAP_ERASE_COMMON(T1 t1, T2 t2, T3 t3)       \
    (*streamIdHookidMap).second.Erase(t2, t3);  \
    }                                           \
    }

#define MAP_ERASE_WITH_QUATRA_PARME(T1, T2, T3, T4) \
    MAP_ERASE_COMMON(T1 t1, T2 t2, T3 t3, T4 t4)    \
    (*streamIdHookidMap).second.Erase(t2, t3, t4);  \
    }                                               \
    }

#endif // SRC_TRACE_BASE_MAP_H
