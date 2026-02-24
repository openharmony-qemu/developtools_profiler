/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
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
#ifndef HIPERF_SMO_MAP_GLOBAL_H
#define HIPERF_SMO_MAP_GLOBAL_H
 
#include <unordered_map>
#include <shared_mutex>
#include "native_hook_result.pb.h"   // only this is needed
 
struct GlobalSmoEntry {
    SMOMapsInfo info;      // last send event
    bool initialized = false;
    bool sent = false;
};
 
extern std::unordered_map<std::string, GlobalSmoEntry> g_smoMaps;
 
extern std::shared_mutex g_smoMapsLock;
#endif