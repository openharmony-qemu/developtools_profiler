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
#include "xpower_plugin.h"

#include <dlfcn.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>
#include <sstream>

#include "common.h"
#include "securec.h"

namespace {
constexpr size_t MAX_QUEUE_SIZE = 2000;
constexpr uint32_t WAIT_DURATION = 100;
constexpr int32_t AID_HAP_START = 20010000;
constexpr int32_t AID_HAP_END = 21065535;
} // namespace
XpowerPlugin::XpowerPlugin()
{
    procMesTypeMapping_.insert({XpowerMessageType::REAL_BATTERY, OptimizeMessageType::MESSAGE_REAL_BATTERY});
    procMesTypeMapping_.insert({XpowerMessageType::APP_STATISTIC, OptimizeMessageType::MESSAGE_APP_STATISTIC});
    procMesTypeMapping_.insert({XpowerMessageType::APP_DETAIL, OptimizeMessageType::MESSAGE_APP_DETAIL});
    procMesTypeMapping_.insert({XpowerMessageType::COMPONENT_TOP, OptimizeMessageType::MESSAGE_COMPONENT_TOP});
    procMesTypeMapping_.insert({XpowerMessageType::ABNORMAL_EVENTS, OptimizeMessageType::MESSAGE_ABNORMAL_EVENTS});
}

XpowerPlugin::~XpowerPlugin()
{
    if (powerClientHandle_ != nullptr) {
        dlclose(powerClientHandle_);
        powerClientHandle_ = nullptr;
    }
}

int XpowerPlugin::Start(const uint8_t *configData, uint32_t configSize)
{
    PROFILER_LOG_INFO(LOG_CORE, "%s:config data -->configSize=%d", __func__, configSize);
    CHECK_TRUE(configData != nullptr, -1, "XpowerPlugin error: param invalid!!!");
    // 反序列化
    CHECK_TRUE(protoConfig_.ParseFromArray(configData, configSize) > 0, -1, "%s:parseFromArray failed!", __func__);
    uint32_t messageType = static_cast<uint32_t>(OptimizeMessageType::MESSAGE_OPTIMIZE_STOP);
    std::string bundleName = protoConfig_.bundle_name();
    if (bundleName.empty()) {
        PROFILER_LOG_ERROR(LOG_CORE, "XpowerPlugin error : bundle name is empty!");
        return -1;
    }
    // Get message type
    if (protoConfig_.message_type().size() > 0) {
        for (int i = 0; i < protoConfig_.message_type().size(); i++) {
            uint32_t mesType = procMesTypeMapping_[static_cast<XpowerMessageType>(protoConfig_.message_type(i))];
            messageType |= mesType;
        }
    }
    PROFILER_LOG_INFO(LOG_CORE, "bundleName is %s,messagetype is %d", bundleName.c_str(), messageType);
    if ((messageType & OptimizeMessageType::MESSAGE_APP_STATISTIC) != 0 ||
        (messageType & OptimizeMessageType::MESSAGE_APP_DETAIL) != 0 ||
        (messageType & OptimizeMessageType::MESSAGE_ABNORMAL_EVENTS) != 0) {
        // check bundleName
        int32_t uid = COMMON::GetPackageUid(bundleName);
        if (uid < AID_HAP_START || uid > AID_HAP_END) {
            PROFILER_LOG_ERROR(LOG_CORE, "the bundle name %s is not supported", bundleName.c_str());
            return -1;
        }
    }
    if ((messageType & OptimizeMessageType::MESSAGE_APP_DETAIL) != 0) {
        int processId = -1;
        bool isExsit = COMMON::IsProcessExist(bundleName, processId);
        if (!isExsit) {
            PROFILER_LOG_ERROR(LOG_CORE, "%s:the process %s does not exist.", __func__, bundleName.c_str());
            return -1;
        }
    }
    // 加载对应so 库文件
    if (!StartPowerManager(messageType, bundleName)) {
        PROFILER_LOG_ERROR(LOG_CORE, "start power manager failed!");
        return -1;
    }
    PROFILER_LOG_INFO(LOG_CORE, "finish register the callback function:%s", __func__);
    return 0;
}

bool XpowerPlugin::StartPowerManager(std::uint32_t messageType, std::string &bundleName)
{
    if (powerClientHandle_ == nullptr) {
        powerClientHandle_ = dlopen("/system/lib64/libxpower_manager_client.z.so", RTLD_LAZY);
    }
    if (powerClientHandle_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "%s :fail to open libxpower_manager_client.z.so", __func__);
        return false;
    }
    // 注册回调
    StartOptimizeMode startOptimizeMode = (StartOptimizeMode)dlsym(powerClientHandle_, "StartOptimizeC");
    if (startOptimizeMode == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "Failed to dlsy startOptimizeMode");
        return false;
    }
    config_.messageType = messageType;
    config_.packageName = bundleName;
    config_.callback = std::bind(&XpowerPlugin::OptimizeCallback, this, std::placeholders::_1, std::placeholders::_2,
                                 std::placeholders::_3);
    listenerHandle_ = startOptimizeMode(config_);
    if (listenerHandle_ == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "Failed to startOptimizeMode");
        return false;
    }
    dataQueuePtr_ = std::make_shared<PowerMessageQueue>(MAX_QUEUE_SIZE);
    return true;
}

void XpowerPlugin::OptimizeCallback(const std::uint32_t messageType, const uint8_t *protoData, size_t protoSize)
{
    if (protoData == nullptr || protoSize <= 0) {
        PROFILER_LOG_ERROR(LOG_CORE, "the power message is invalid !");
        return;
    }
    // write protoData to file
    if (resultWriter_ != nullptr && resultWriter_->isProtobufSerialize) {
        resultWriter_->write(resultWriter_, protoData, protoSize);
        resultWriter_->flush(resultWriter_);
    }
    if (dataQueuePtr_->Size() >= MAX_QUEUE_SIZE) {
        PROFILER_LOG_ERROR(LOG_CORE, "The buffer queue is full,discard message");
        return;
    }
    // 拷贝
    auto rawData = std::make_shared<PowerOptimizeData>();
    rawData->baseData = std::make_unique<uint8_t[]>(protoSize);
    if (memcpy_s(rawData->baseData.get(), protoSize, protoData, protoSize) != EOK) {
        PROFILER_LOG_ERROR(LOG_CORE, "memcpy_s raw powerdata failed!");
        return;
    }
    rawData->length = protoSize;
    rawData->messageType = messageType;
    dataQueuePtr_->PushBack(rawData);
}

int XpowerPlugin::Report(uint8_t *data, uint32_t dataSize)
{
    if (dataQueuePtr_->Empty()) {
        return 0;
    }
    std::shared_ptr<PowerOptimizeData> result = nullptr;
    if (!dataQueuePtr_->WaitAndPop(result, std::chrono::milliseconds(WAIT_DURATION))) {
        PROFILER_LOG_ERROR(LOG_CORE, "fetch data error!");
        return 0;
    }
    if (result == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "fetch data error!");
        return 0;
    }
    if (result->length > dataSize) {
        return -result->length;
    }
    CHECK_TRUE(memcpy_s(data, dataSize, result->baseData.get(), result->length) == EOK, 0, "memcpy_s raw data failed!");
    return result->length;
}

void XpowerPlugin::SetWriter(WriterStruct *writer)
{
    resultWriter_ = writer;
}

int XpowerPlugin::Stop()
{
    PROFILER_LOG_INFO(LOG_CORE, "%s:begin to stop xpower plugin", __func__);
    StopOptimizeMode stopOptimizeMode = (StopOptimizeMode)dlsym(powerClientHandle_, "StopOptimizeC");
    if (stopOptimizeMode == nullptr) {
        PROFILER_LOG_ERROR(LOG_CORE, "Faile to dlsym StopOptimizeC");
        return -1;
    }
    if (listenerHandle_ != nullptr) {
        stopOptimizeMode(listenerHandle_);
        PROFILER_LOG_INFO(LOG_CORE, "stop xpower plugin callback");
    }
    dataQueuePtr_->ShutDown();
    return 0;
}
