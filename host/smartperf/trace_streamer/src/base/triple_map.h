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

#ifndef SRC_TRACE_BASE_TRIPLEMAP_H
#define SRC_TRACE_BASE_TRIPLEMAP_H

#include "double_map.h"

template <class T1, class T2, class T3, class T4>
class TripleMap {
public:
    TripleMap(T4 invalidValue)
    {
        invalidValue_ = invalidValue;
    }
    void SetInvalidRet(T4 invalidValue)
    {
        invalidValue_ = invalidValue;
    }
    void Insert(T1 t1, T2 t2, T3 t3, T4 t4)
    {
        auto streamIdHookidMap = internalMap_.find(t1);
        if (streamIdHookidMap != internalMap_.end()) {
            (*streamIdHookidMap).second.Insert(t2, t3, t4);
        } else {
            DoubleMap<T2, T3, T4> mm(invalidValue_);
            mm.Insert(t2, t3, t4);
            internalMap_.emplace(std::make_pair(t1, mm));
        }
    }
    T4 Find(T1 t1, T2 t2, T3 t3)
    {
        auto streamIdHookidMap = internalMap_.find(t1);
        if (streamIdHookidMap != internalMap_.end()) {
            return (*streamIdHookidMap).second.Find(t2, t3);
        } else {
            return invalidValue_;
        }
    }
    MAP_ERASE_WITH_SINGLE_PARME(T1)
    MAP_ERASE_WITH_DOUBLE_PARME(T1, T2)
    MAP_ERASE_WITH_TRIPLE_PARME(T1, T2, T3)
    void Clear()
    {
        internalMap_.clear();
    }

private:
    std::map<T1, DoubleMap<T2, T3, T4>> internalMap_;
    T4 invalidValue_;
};

#endif // SRC_TRACE_BASE_TRIPLEMAP_H
