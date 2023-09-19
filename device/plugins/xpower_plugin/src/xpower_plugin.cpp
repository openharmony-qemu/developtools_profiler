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
#include "xpower_plugin.h"
#include "securec.h"
#include "xpower_plugin_result.pb.h"
#include "common.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <dlfcn.h>

namespace {
constexpr size_t MAX_QUEUE_SIZE = 2000;
uint32_t g_waitDuration = 100;
}  // namespace
XpowerPlugin::XpowerPlugin()
{
    procMesTypeMapping.insert({XpowerMessageType::REAL_BATTERY, OptimizeMessageType::MESSAGE_REAL_BATTERY});
    procMesTypeMapping.insert({XpowerMessageType::APP_STATISTIC, OptimizeMessageType::MESSAGE_APP_STATISTIC});
    procMesTypeMapping.insert({XpowerMessageType::APP_DETAIL, OptimizeMessageType::MESSAGE_APP_DETAIL});
    procMesTypeMapping.insert({XpowerMessageType::COMPONENT_TOP, OptimizeMessageType::MESSAGE_COMPONENT_TOP});
    procMesTypeMapping.insert({XpowerMessageType::ABNORMAL_EVENTS, OptimizeMessageType::MESSAGE_ABNORMAL_EVENTS});
}

XpowerPlugin::~XpowerPlugin()
{}

int XpowerPlugin::Start(const uint8_t *configData, uint32_t configSize)
{
    HILOG_INFO(LOG_CORE, "%s:config data -->configSize=%d", __func__, configSize);
    CHECK_TRUE(configData != nullptr, -1, "XpowerPlugin error: param invalid!!!");
    // 反序列化
    CHECK_TRUE(protoConfig.ParseFromArray(configData, configSize) > 0, -1, "%s:parseFromArray failed!", __func__);
    uint32_t messageType = static_cast<uint32_t>(OptimizeMessageType::MESSAGE_OPTIMIZE_STOP);
    std::string bundleName = protoConfig.bundle_name();
    if (bundleName.empty()) {
        HILOG_ERROR(LOG_CORE, "XpowerPlugin error : bundle name is empty!");
        return -1;
    }
    // 判断进程是否存在
    int processId = -1;
    bool isExsit = COMMON::IsProcessExist(bundleName, processId);
    if (!isExsit) {
        HILOG_ERROR(LOG_CORE, "%s:the process %s does not exist.", __func__, bundleName.c_str());
        return -1;
    }
    //check bundleName 
    int32_t uid = COMMON::GetPackageUid(bundleName);
    if (uid < 10000) {
        HILOG_ERROR(LOG_CORE, "the bundle name %s is not supported", bundleName.c_str());
        return -1;
    }
    if (protoConfig.message_type().size() > 0) {
        for (int i = 0; i < protoConfig.message_type().size(); i++) {
            uint32_t mesType = procMesTypeMapping[static_cast<XpowerMessageType>(protoConfig.message_type(i))];
            messageType |= mesType;
        }
    }
    HILOG_INFO(LOG_CORE, "bundleName is %s,messagetype is %d", bundleName.c_str(), messageType);
    // 加载对应so 库文件
    if (powerClientHandle == nullptr) {
        powerClientHandle = dlopen("/system/lib64/libxpower_manager_client.z.so", RTLD_LAZY);
    }
    if (powerClientHandle == nullptr) {
        HILOG_ERROR(LOG_CORE, "%s :fail to open libxpower_manager_client.z.so", __func__);
        return -1;
    }
    // 注册回调
    StartOptimizeMode startOptimizeMode = (StartOptimizeMode)dlsym(powerClientHandle, "StartOptimizeC");
    if (startOptimizeMode == nullptr) {
        HILOG_ERROR(LOG_CORE, "Failed to dlsy startOptimizeMode");
        return -1;
    }
    config.messageType = messageType;
    config.packageName = bundleName;
    config.callback = std::bind(
        &XpowerPlugin::OptimizeCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    listenerHandle = startOptimizeMode(config);
    if (listenerHandle == nullptr) {
        HILOG_ERROR(LOG_CORE, "Failed to startOptimizeMode");
        return -1;
    }
    dataQueuePtr = std::make_shared<PowerMessageQueue>(MAX_QUEUE_SIZE);
    HILOG_INFO(LOG_CORE, "finish register the callback function:%s", __func__);
    return 0;
}

void XpowerPlugin::OptimizeCallback(const std::uint32_t messageType, const uint8_t *protoData, size_t protoSize)
{
    if (protoData == nullptr) {
        HILOG_ERROR(LOG_CORE, "the power message is invalid !");
        return;
    }
    // 拷贝
    auto rawData = std::make_shared<PowerOptimizeData>();
    rawData->baseData = std::make_unique<uint8_t[]>(protoSize);
    if (memcpy_s(rawData->baseData.get(), protoSize, protoData, protoSize) != EOK) {
        HILOG_ERROR(LOG_CORE, "memcpy_s raw powerdata failed!");
        return ;
    }
    rawData->length = protoSize;
    rawData->messageType = messageType;
    dataQueuePtr->PushBack(rawData);
    if (resultWriter != nullptr) {
        resultWriter->write(resultWriter, protoData, protoSize);
        resultWriter->flush(resultWriter);
    }
}

void XpowerPlugin::DisplayPowerMessage(std::shared_ptr<PowerOptimizeData>& result)
{
    OptimizeReport dataProto;
    dataProto.ParseFromArray(result->baseData.get(), result->length);
    HILOG_INFO(LOG_CORE,
        "power report :recive optimize start_time = %ld, end_time=%ld",
        dataProto.start_time(),
        dataProto.end_time());
    HILOG_INFO(LOG_CORE, "power report :bundle_name is %s", dataProto.bundle_name().c_str());
    if ((result->messageType & OptimizeMessageType::MESSAGE_REAL_BATTERY) != 0) {
        HILOG_INFO(LOG_CORE,
            "power report: real_battery:capacity = %ld,charge=%ld,current=%ld,gas_gauge=%ld,level=%ld,screen=%ld",
            dataProto.real_battery().capacity(),
            dataProto.real_battery().charge(),
            dataProto.real_battery().current(),
            dataProto.real_battery().gas_gauge(),
            dataProto.real_battery().level(),
            dataProto.real_battery().screen());
    }
    if ((result->messageType & OptimizeMessageType::MESSAGE_APP_STATISTIC) != 0) {
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:auto enegy=%ld,time=%ld",
            dataProto.app_statistic().audio().energy(),
            dataProto.app_statistic().audio().time());
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:bluetooth enegy=%ld,time=%ld",
            dataProto.app_statistic().bluetooth().energy(),
            dataProto.app_statistic().bluetooth().time());
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:camera enegy=%ld,time=%ld",
            dataProto.app_statistic().camera().energy(),
            dataProto.app_statistic().camera().time());
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:cpu enegy=%ld,time=%ld",
            dataProto.app_statistic().cpu().energy(),
            dataProto.app_statistic().cpu().time());
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:cpu display=%ld,time=%ld",
            dataProto.app_statistic().display().energy(),
            dataProto.app_statistic().display().time());
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:cpu flashlight=%ld,time=%ld",
            dataProto.app_statistic().flashlight().energy(),
            dataProto.app_statistic().flashlight().time());
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:cpu gpu=%ld,time=%ld",
            dataProto.app_statistic().gpu().energy(),
            dataProto.app_statistic().gpu().time());
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:cpu location=%ld,time=%ld",
            dataProto.app_statistic().location().energy(),
            dataProto.app_statistic().location().time());
        HILOG_INFO(LOG_CORE,
            "power report: app_statistic:cpu wifiscan=%ld,time=%ld",
            dataProto.app_statistic().wifiscan().energy(),
            dataProto.app_statistic().wifiscan().time());
    }
    if ((result->messageType & OptimizeMessageType::MESSAGE_APP_DETAIL) != 0) {
        HILOG_INFO(LOG_CORE, "power report: app_detail:cpu thread_count=%d", dataProto.app_detail().cpu().thread_count());
        std::string loadStr;
        for (int i = 0; i < dataProto.app_detail().cpu().thread_load().size(); i++) {
            loadStr += std::to_string(static_cast<long long>(dataProto.app_detail().cpu().thread_load(i))) + ":";
        }
        loadStr.pop_back();
        HILOG_INFO(LOG_CORE, "power report: app_detail:cpu tread_load=%s", loadStr.c_str());
        std::string threadName;
        for (int i = 0; i < dataProto.app_detail().cpu().thread_name().size(); i++) {
            threadName += dataProto.app_detail().cpu().thread_name(i) + ":";
        }
        threadName.pop_back();
        HILOG_INFO(LOG_CORE, "power report: app_detail:cpu thread_name=%s", threadName.c_str());
        HILOG_INFO(LOG_CORE, "power report: app_detail:gpu frequency_count=%ld", dataProto.app_detail().gpu().frequency_count());
        std::string freqStr;
        for (int i = 0; i < dataProto.app_detail().gpu().frequency().size(); i++) {
            freqStr += std::to_string(dataProto.app_detail().gpu().frequency(i)) + ":";
        }
        freqStr.pop_back();
        HILOG_INFO(LOG_CORE, "power report: app_detail:gpu frequency=%s", freqStr.c_str());
        std::string idleStr;
        for (int i = 0; i < dataProto.app_detail().gpu().idle_time().size(); i++) {
            idleStr += std::to_string(dataProto.app_detail().gpu().idle_time(i)) + ":";
        }
        idleStr.pop_back();
        HILOG_INFO(LOG_CORE, "power report: app_detail:gpu idle_time=%s", idleStr.c_str());
        std::string runStr;
        for (int i = 0; i < dataProto.app_detail().gpu().run_time().size(); i++) {
            runStr += std::to_string(dataProto.app_detail().gpu().run_time(i)) + ":";
        }
        runStr.pop_back();
        HILOG_INFO(LOG_CORE, "power report: app_detail:gpu run_time=%s", runStr.c_str());
    }
}
int XpowerPlugin::Report(uint8_t *data, uint32_t dataSize)
{
    HILOG_INFO(LOG_CORE, "power: report the data to caller");
    if (dataQueuePtr->Empty()) {
        return 0;
    }
    std::shared_ptr<PowerOptimizeData> result = nullptr;
    if (!dataQueuePtr->WaitAndPop(result, std::chrono::milliseconds(g_waitDuration))) {
        HILOG_ERROR(LOG_CORE, "fetch data error!");
        return 0;
    }
    if (result == nullptr) {
        HILOG_ERROR(LOG_CORE, "fetch data error!");
        return 0;
    }
    if (result->length > dataSize) {
        return -result->length;
    }
    DisplayPowerMessage(result);
    CHECK_TRUE(memcpy_s(data, dataSize, result->baseData.get(), result->length) == EOK, 0, "memcpy_s raw data failed!");
    return result->length;
}

void XpowerPlugin::SetWriter(WriterStruct* writer)
{
    resultWriter = writer;
}

int XpowerPlugin::Stop()
{
    HILOG_INFO(LOG_CORE, "%s:begin to stop xpower plugin", __func__);
    StopOptimizeMode stopOptimizeMode = (StopOptimizeMode)dlsym(powerClientHandle, "StopOptimizeC");
    if (stopOptimizeMode == nullptr) {
        HILOG_ERROR(LOG_CORE, "Faile to dlsym StopOptimizeC");
        return -1;
    }
    if (listenerHandle != nullptr) {
        stopOptimizeMode(listenerHandle);
        HILOG_INFO(LOG_CORE, "stop xpower plugin callback");
    }
    if (powerClientHandle != nullptr) {
        dlclose(powerClientHandle);
    }
    return 0;
}
