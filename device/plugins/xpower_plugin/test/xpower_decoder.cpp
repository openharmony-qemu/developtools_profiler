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
#include "xpower_decoder.h"

#include <securec.h>

#include <climits>
#include <cstring>

#include "xpower_common.h"

XpowerDecoder::XpowerDecoder() {}

XpowerDecoder::~XpowerDecoder() {}

cJSON* XpowerDecoder::CreateAppDetail(const AppDetail& appDetail)
{
    cJSON* cjsonAppDetail = cJSON_CreateObject();
    cJSON_AddItemToObject(cjsonAppDetail, "cpu", CreateAppDetailCpu(appDetail.cpu()));
    cJSON_AddItemToObject(cjsonAppDetail, "gpu", CreateAppDetailGpu(appDetail.gpu()));
    return cjsonAppDetail;
}

cJSON* XpowerDecoder::CreateAppStatistic(const AppStatistic& appStatistic)
{
    cJSON* cjsonAppStatistic = cJSON_CreateObject();
    cJSON_AddItemToObject(cjsonAppStatistic, "audio", CreatAppStatisticCommon(appStatistic.audio()));
    cJSON_AddItemToObject(cjsonAppStatistic, "bluetooth", CreatAppStatisticCommon(appStatistic.audio()));
    cJSON_AddItemToObject(cjsonAppStatistic, "camera", CreatAppStatisticCommon(appStatistic.camera()));
    cJSON_AddItemToObject(cjsonAppStatistic, "cpu", CreatAppStatisticCommon(appStatistic.cpu()));
    cJSON_AddItemToObject(cjsonAppStatistic, "display", CreatAppStatisticCommon(appStatistic.display()));
    cJSON_AddItemToObject(cjsonAppStatistic, "flashlight", CreatAppStatisticCommon(appStatistic.flashlight()));
    cJSON_AddItemToObject(cjsonAppStatistic, "gpu", CreatAppStatisticCommon(appStatistic.gpu()));
    cJSON_AddItemToObject(cjsonAppStatistic, "location", CreatAppStatisticCommon(appStatistic.location()));
    cJSON_AddItemToObject(cjsonAppStatistic, "wifiscan", CreatAppStatisticCommon(appStatistic.wifiscan()));
    return cjsonAppStatistic;
}

cJSON* XpowerDecoder::CreateAppDetailGpu(const AppDetailGPU& appDeGpu)
{
    cJSON* cjsonGpu = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjsonGpu, "frequency_count", appDeGpu.frequency_count());
    cJSON* frequencyArr = cJSON_CreateArray();
    for (int i = 0; i < appDeGpu.frequency().size(); i++) {
        cJSON_AddItemToArray(frequencyArr, cJSON_CreateNumber(appDeGpu.frequency(i)));
    }
    cJSON_AddItemToObject(cjsonGpu, "frequency", frequencyArr);

    cJSON* idleTimeArr = cJSON_CreateArray();
    for (int i = 0; i < appDeGpu.idle_time().size(); i++) {
        cJSON_AddItemToArray(idleTimeArr, cJSON_CreateNumber(appDeGpu.idle_time(i)));
    }
    cJSON_AddItemToObject(cjsonGpu, "idle_time", idleTimeArr);

    cJSON* runTimeArr = cJSON_CreateArray();
    for (int i = 0; i < appDeGpu.run_time().size(); i++) {
        cJSON_AddItemToArray(runTimeArr, cJSON_CreateNumber(appDeGpu.run_time(i)));
    }
    cJSON_AddItemToObject(cjsonGpu, "run_time", runTimeArr);
    return cjsonGpu;
}

cJSON* XpowerDecoder::CreateAppDetailCpu(const AppDetailCPU& appDeCpu)
{
    cJSON* cjsonCpu = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjsonCpu, "thread_count", appDeCpu.thread_count());
    cJSON* threadLoadArr = cJSON_CreateArray();
    for (int i = 0; i < appDeCpu.thread_load().size(); i++) {
        cJSON_AddItemToArray(threadLoadArr, cJSON_CreateNumber(appDeCpu.thread_load(i)));
    }
    cJSON_AddItemToObject(cjsonCpu, "thread_load", threadLoadArr);

    cJSON* threadNameArr = cJSON_CreateArray();
    for (int i = 0; i < appDeCpu.thread_name().size(); i++) {
        cJSON_AddItemToArray(threadNameArr, cJSON_CreateString(appDeCpu.thread_name(i).c_str()));
    }
    cJSON_AddItemToObject(cjsonCpu, "thread_name", threadNameArr);

    cJSON* threadTimeArr = cJSON_CreateArray();
    for (int i = 0; i < appDeCpu.thread_time().size(); i++) {
        cJSON_AddItemToArray(threadTimeArr, cJSON_CreateNumber(appDeCpu.thread_time(i)));
    }
    cJSON_AddItemToObject(cjsonCpu, "thread_time", threadTimeArr);

    cJSON* threadEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < appDeCpu.thread_energy().size(); i++) {
        cJSON_AddItemToArray(threadEnergyArr, cJSON_CreateNumber(appDeCpu.thread_energy(i)));
    }
    cJSON_AddItemToObject(cjsonCpu, "thread_energy", threadEnergyArr);
    return cjsonCpu;
}

cJSON* XpowerDecoder::CreatAppStatisticCommon(const AppStatisticCommon& appCom)
{
    cJSON* cjsonAppCom = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjsonAppCom, "energy", appCom.energy());
    cJSON_AddNumberToObject(cjsonAppCom, "time", appCom.time());
    return cjsonAppCom;
}

cJSON* XpowerDecoder::CreateRealBattery(const RealBattery& realBattery)
{
    cJSON* cjsonBattery = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjsonBattery, "capacity", realBattery.capacity());
    cJSON_AddNumberToObject(cjsonBattery, "charge", realBattery.charge());
    cJSON_AddNumberToObject(cjsonBattery, "current", realBattery.current());
    cJSON_AddNumberToObject(cjsonBattery, "gas_gauge", realBattery.gas_gauge());
    cJSON_AddNumberToObject(cjsonBattery, "level", realBattery.level());
    cJSON_AddNumberToObject(cjsonBattery, "screen", realBattery.screen());
    return cjsonBattery;
}

cJSON* XpowerDecoder::CreateComponentTopCommon(const ComponentTopCommon& topCommon)
{
    cJSON* cjsonTopCommon = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjsonTopCommon, "count", topCommon.count());
    cJSON* appnameArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.appname().size(); i++) {
        cJSON_AddItemToArray(appnameArr, cJSON_CreateString(topCommon.appname(i).c_str()));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "appname", appnameArr);

    cJSON* durationArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.background_duration().size(); i++) {
        cJSON_AddItemToArray(durationArr, cJSON_CreateNumber(topCommon.background_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "background_duration", durationArr);

    cJSON* energyArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.background_energy().size(); i++) {
        cJSON_AddItemToArray(energyArr, cJSON_CreateNumber(topCommon.background_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "background_energy", energyArr);

    cJSON* foreDurationArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.foreground_duration().size(); i++) {
        cJSON_AddItemToArray(foreDurationArr, cJSON_CreateNumber(topCommon.foreground_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "foreground_duration", foreDurationArr);

    cJSON* foreEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.foreground_energy().size(); i++) {
        cJSON_AddItemToArray(foreEnergyArr, cJSON_CreateNumber(topCommon.foreground_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "foreground_energy", foreEnergyArr);

    cJSON* screenDuraArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.screen_off_duration().size(); i++) {
        cJSON_AddItemToArray(screenDuraArr, cJSON_CreateNumber(topCommon.screen_off_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_off_duration", screenDuraArr);

    cJSON* screenEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.screen_off_energy().size(); i++) {
        cJSON_AddItemToArray(screenEnergyArr, cJSON_CreateNumber(topCommon.screen_off_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_off_energy", screenEnergyArr);

    cJSON* screenOnDuraArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.screen_on_duration().size(); i++) {
        cJSON_AddItemToArray(screenOnDuraArr, cJSON_CreateNumber(topCommon.screen_on_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_on_duration", screenOnDuraArr);

    cJSON* screenOnEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCommon.screen_on_energy().size(); i++) {
        cJSON_AddItemToArray(screenOnEnergyArr, cJSON_CreateNumber(topCommon.screen_on_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_on_energy", screenOnEnergyArr);
    return cjsonTopCommon;
}
cJSON* XpowerDecoder::CreateComponentTopCamera(const ComponentTopCamera& topCamera)
{
    cJSON* cjsonTopCommon = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjsonTopCommon, "count", topCamera.count());
    cJSON* appnameArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.appname().size(); i++) {
        cJSON_AddItemToArray(appnameArr, cJSON_CreateString(topCamera.appname(i).c_str()));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "appname", appnameArr);

    cJSON* cameryIdArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.camera_id().size(); i++) {
        cJSON_AddItemToArray(cameryIdArr, cJSON_CreateNumber(topCamera.camera_id(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "camera_id", cameryIdArr);

    cJSON* durationArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.background_duration().size(); i++) {
        cJSON_AddItemToArray(durationArr, cJSON_CreateNumber(topCamera.background_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "background_duration", durationArr);

    cJSON* energyArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.background_energy().size(); i++) {
        cJSON_AddItemToArray(energyArr, cJSON_CreateNumber(topCamera.background_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "background_energy", energyArr);

    cJSON* foreDurationArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.foreground_duration().size(); i++) {
        cJSON_AddItemToArray(foreDurationArr, cJSON_CreateNumber(topCamera.foreground_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "foreground_duration", foreDurationArr);

    cJSON* foreEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.foreground_energy().size(); i++) {
        cJSON_AddItemToArray(foreEnergyArr, cJSON_CreateNumber(topCamera.foreground_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "foreground_energy", foreEnergyArr);

    cJSON* screenDuraArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.screen_off_duration().size(); i++) {
        cJSON_AddItemToArray(screenDuraArr, cJSON_CreateNumber(topCamera.screen_off_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_off_duration", screenDuraArr);

    cJSON* screenEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.screen_off_energy().size(); i++) {
        cJSON_AddItemToArray(screenEnergyArr, cJSON_CreateNumber(topCamera.screen_off_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_off_energy", screenEnergyArr);

    cJSON* screenOnDuraArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.screen_on_duration().size(); i++) {
        cJSON_AddItemToArray(screenOnDuraArr, cJSON_CreateNumber(topCamera.screen_on_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_on_duration", screenOnDuraArr);

    cJSON* screenOnEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCamera.screen_on_energy().size(); i++) {
        cJSON_AddItemToArray(screenOnEnergyArr, cJSON_CreateNumber(topCamera.screen_on_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_on_energy", screenOnEnergyArr);
    return cjsonTopCommon;
}

cJSON* XpowerDecoder::CreateComponentTopCpu(const ComponentTopCpu& topCpu)
{
    cJSON* cjsonTopCommon = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjsonTopCommon, "count", topCpu.count());
    cJSON* appnameArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.appname().size(); i++) {
        cJSON_AddItemToArray(appnameArr, cJSON_CreateString(topCpu.appname(i).c_str()));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "appname", appnameArr);

    cJSON* userIdArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.uid().size(); i++) {
        cJSON_AddItemToArray(userIdArr, cJSON_CreateNumber(topCpu.uid(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "uid", userIdArr);

    cJSON* durationArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.background_duration().size(); i++) {
        cJSON_AddItemToArray(durationArr, cJSON_CreateNumber(topCpu.background_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "background_duration", durationArr);

    cJSON* energyArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.background_energy().size(); i++) {
        cJSON_AddItemToArray(energyArr, cJSON_CreateNumber(topCpu.background_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "background_energy", energyArr);

    cJSON* foreDurationArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.foreground_duration().size(); i++) {
        cJSON_AddItemToArray(foreDurationArr, cJSON_CreateNumber(topCpu.foreground_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "foreground_duration", foreDurationArr);

    cJSON* foreEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.foreground_energy().size(); i++) {
        cJSON_AddItemToArray(foreEnergyArr, cJSON_CreateNumber(topCpu.foreground_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "foreground_energy", foreEnergyArr);

    cJSON* screenDuraArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.screen_off_duration().size(); i++) {
        cJSON_AddItemToArray(screenDuraArr, cJSON_CreateNumber(topCpu.screen_off_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_off_duration", screenDuraArr);

    cJSON* screenEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.screen_off_energy().size(); i++) {
        cJSON_AddItemToArray(screenEnergyArr, cJSON_CreateNumber(topCpu.screen_off_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_off_energy", screenEnergyArr);

    cJSON* screenOnDuraArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.screen_on_duration().size(); i++) {
        cJSON_AddItemToArray(screenOnDuraArr, cJSON_CreateNumber(topCpu.screen_on_duration(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_on_duration", screenOnDuraArr);

    cJSON* screenOnEnergyArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.screen_on_energy().size(); i++) {
        cJSON_AddItemToArray(screenOnEnergyArr, cJSON_CreateNumber(topCpu.screen_on_energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "screen_on_energy", screenOnEnergyArr);

    cJSON* loadArr = cJSON_CreateArray();
    for (int i = 0; i < topCpu.load().size(); i++) {
        cJSON_AddItemToArray(loadArr, cJSON_CreateNumber(topCpu.load(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "load", loadArr);

    return cjsonTopCommon;
}

cJSON* XpowerDecoder::CreateComponentTopDisplay(const ComponentTopDisplay& topDisplay)
{
    cJSON* cjsonTopCommon = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjsonTopCommon, "count", topDisplay.count());
    cJSON* appnameArr = cJSON_CreateArray();
    for (int i = 0; i < topDisplay.appname().size(); i++) {
        cJSON_AddItemToArray(appnameArr, cJSON_CreateString(topDisplay.appname(i).c_str()));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "appname", appnameArr);

    cJSON* timeArr = cJSON_CreateArray();
    for (int i = 0; i < topDisplay.time().size(); i++) {
        cJSON_AddItemToArray(timeArr, cJSON_CreateNumber(topDisplay.time(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "time", timeArr);

    cJSON* energyArr = cJSON_CreateArray();
    for (int i = 0; i < topDisplay.energy().size(); i++) {
        cJSON_AddItemToArray(energyArr, cJSON_CreateNumber(topDisplay.energy(i)));
    }
    cJSON_AddItemToObject(cjsonTopCommon, "energy", energyArr);
    return cjsonTopCommon;
}

cJSON* XpowerDecoder::CreateComponentTop(const ComponentTop& componentTop)
{
    cJSON* jsonComTop = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonComTop, "audio", CreateComponentTopCommon(componentTop.audio()));
    cJSON_AddItemToObject(jsonComTop, "bluetooth", CreateComponentTopCommon(componentTop.bluetooth()));
    cJSON_AddItemToObject(jsonComTop, "camera", CreateComponentTopCamera(componentTop.camera()));
    cJSON_AddItemToObject(jsonComTop, "cpu", CreateComponentTopCpu(componentTop.cpu()));
    cJSON_AddItemToObject(jsonComTop, "display", CreateComponentTopDisplay(componentTop.display()));
    cJSON_AddItemToObject(jsonComTop, "flashlight", CreateComponentTopCommon(componentTop.flashlight()));
    cJSON_AddItemToObject(jsonComTop, "gpu", CreateComponentTopDisplay(componentTop.gpu()));
    cJSON_AddItemToObject(jsonComTop, "location", CreateComponentTopCommon(componentTop.location()));
    cJSON_AddItemToObject(jsonComTop, "wifiscan", CreateComponentTopCommon(componentTop.wifiscan()));
    return jsonComTop;
}

std::string XpowerDecoder::DecodeXpowerMessage(OptimizeReport& dataProto)
{
    std::string retStr;
    cJSON* optReport = cJSON_CreateObject();
    cJSON_AddNumberToObject(optReport, "start_time", dataProto.start_time());
    cJSON_AddNumberToObject(optReport, "end_time", dataProto.end_time());
    cJSON_AddStringToObject(optReport, "bundle_name", dataProto.bundle_name().c_str());
    cJSON_AddNumberToObject(optReport, "message_type", dataProto.message_type());
    uint32_t mesType = dataProto.message_type();
    if ((mesType & OptimizeMessageType::MESSAGE_REAL_BATTERY) != 0) {
        cJSON_AddItemToObject(optReport, "real_battery", CreateRealBattery(dataProto.real_battery()));
    }
    if ((mesType & OptimizeMessageType::MESSAGE_APP_STATISTIC) != 0) {
        cJSON_AddItemToObject(optReport, "app_statistic", CreateAppStatistic(dataProto.app_statistic()));
    }
    if ((mesType & OptimizeMessageType::MESSAGE_APP_DETAIL) != 0) {
        cJSON_AddItemToObject(optReport, "app_detail", CreateAppDetail(dataProto.app_detail()));
    }
    if ((mesType & OptimizeMessageType::MESSAGE_COMPONENT_TOP) != 0) {
        cJSON_AddItemToObject(optReport, "component_top", CreateComponentTop(dataProto.component_top()));
    }
    retStr = cJSON_Print(optReport);
    cJSON_Delete(optReport);
    return retStr;
}
