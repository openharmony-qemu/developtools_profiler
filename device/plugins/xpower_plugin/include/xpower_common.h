/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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

#ifndef X_POWER_COMMONT_H
#define X_POWER_COMMONT_H

#include <functional>
#include <map>
#include <string>

#include "xpower_plugin_config.pb.h"


struct OptimizeConfig {
    std::uint32_t messageType;
    std::string packageName;
    std::function<void(const std::uint32_t, const std::uint8_t *, size_t)> callback;
};

enum OptimizeMessageType : uint32_t {
    MESSAGE_OPTIMIZE_STOP = 0,
    MESSAGE_REAL_BATTERY = 1 << 0,
    MESSAGE_APP_STATISTIC = 1 << 1,
    MESSAGE_APP_DETAIL = 1 << 2,
    MESSAGE_COMPONENT_TOP = 1 << 3,
    MESSAGE_ABNORMAL_EVENTS = 1 << 4,
    MESSAGE_OPTIMIZE_MAX = (MESSAGE_ABNORMAL_EVENTS << 1) - 1,
};
struct PowerOptimizeData {
    std::unique_ptr<uint8_t[]> baseData; // save the powere message data
    size_t length;
    std::uint32_t messageType;
};
using StartOptimizeMode = void *(*)(struct OptimizeConfig &);
using StopOptimizeMode = bool (*)(void *);

#endif