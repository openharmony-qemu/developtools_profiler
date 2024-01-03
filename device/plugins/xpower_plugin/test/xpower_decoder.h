/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
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
#ifndef XPOWER_DECODER_H
#define XPOWER_DECODER_H

#include "cJSON.h"
#include "xpower_plugin_result.pb.h"

class XpowerDecoder {
public:
    XpowerDecoder();
    ~XpowerDecoder();
    std::string DecodeXpowerMessage(OptimizeReport& dataProto);

private:
    // 应用各器件的调优信息
    cJSON* CreateAppDetail(const AppDetail& appDetail);
    cJSON* CreateAppDetailGpu(const AppDetailGPU& appDeGpu);
    cJSON* CreateAppDetailCpu(const AppDetailCPU& appDeCpu);
    cJSON* CreateAppDetailWifi(const AppDetailWifi& appWifi);
    cJSON* CreateAppDetailDisplay(const AppDetailDisplay& appDisplay);
    // 应用的各器件信息的统计
    cJSON* CreateAppStatistic(const AppStatistic& appStatistic);
    cJSON* CreatAppStatisticCommon(const AppStatisticCommon& appCom);
    // 电源信息统计
    cJSON* CreateRealBattery(const RealBattery& realBattery);
    // 整机各器件的top应用信息
    cJSON* CreateComponentTop(const ComponentTop& componentTop);
    cJSON* CreateComponentTopCommon(const ComponentTopCommon& topCommon);
    cJSON* CreateComponentTopCamera(const ComponentTopCamera& topCamera);
    cJSON* CreateComponentTopCpu(const ComponentTopCpu& topCpu);
    cJSON* CreateComponentTopDisplay(const ComponentTopDisplay& topDisplay);
    // 异常事件上报
    cJSON* CreateAbnormalEventInfo(const AbnormalEventInfo& eventInfo);
    cJSON* CreateAbnormalEvents(const AbnormalEvents& abnormEvent);
};

#endif // XPOWER_DECODER_H
