/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
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

#ifndef POWER_PLUGIN_H
#define POWER_PLUGIN_H

#include "logging.h"
#include "xpower_common.h"
#include "plugin_module_api.h"
#include "power_message_queue.h"

using MesTypeMap = std::map<XpowerMessageType, OptimizeMessageType>;


class XpowerPlugin {
public:
    XpowerPlugin();
    ~XpowerPlugin();
    int Start(const uint8_t *configData, uint32_t configSize);
    int Report(uint8_t *configData, uint32_t configSize);
    int Stop();
    void OptimizeCallback(const std::uint32_t messageType, const uint8_t *protoData, size_t protoSize);
    void SetWriter(WriterStruct* writer);
    void DisplayPowerMessage(std::shared_ptr<PowerOptimizeData>& result);

private:
    void *powerClientHandle = nullptr;
    XpowerConfig protoConfig;
    MesTypeMap procMesTypeMapping;
    std::shared_ptr<PowerMessageQueue> dataQueuePtr;
    WriterStruct* resultWriter{nullptr};
    // xpower callback config
    OptimizeConfig config;
    void* listenerHandle = nullptr;
};

#endif  // POWER_PLUGIN_H