/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "hiappevent_util.h"

#include <cinttypes>
#include <utility>

#include "app_event.h"
#include "app_event_processor_mgr.h"
#include "hidebug_util.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HIAPPEVENT_UTIL"

ApiInvokeRecorder::ApiInvokeRecorder(std::string apiName) : apiName_(std::move(apiName)),
    beginTime_(GetNanoSecondsTimestamp()) {}

ApiInvokeRecorder::~ApiInvokeRecorder()
{
    static int64_t processId = AddProcessor();
    if (processId < 0) {
        return;
    }
    HiAppEvent::Event event("api_diagnostic", "api_exec_end", HiAppEvent::BEHAVIOR);
    event.AddParam("trans_id", std::string("transId_") + std::to_string(beginTime_));
    event.AddParam("api_name", apiName_);
    event.AddParam("sdk_name", std::string("PerformanceAnalysisKit"));
    constexpr int milliSecondsToNanoseconds = 1000 * 1000;
    event.AddParam("begin_time", beginTime_ / milliSecondsToNanoseconds);
    event.AddParam("end_time", GetNanoSecondsTimestamp() / milliSecondsToNanoseconds);
    event.AddParam("result", static_cast<int>(errorCode_ == 0));
    event.AddParam("error_code", errorCode_);
    Write(event);
}

void ApiInvokeRecorder::SetErrorCode(int errorCode)
{
    errorCode_ = errorCode;
}

int64_t ApiInvokeRecorder::AddProcessor()
{
    using namespace HiAppEvent;
    ReportConfig config;
    config.name = "ha_app_event";
    config.appId = "com_huawei_hmos_sdk_ocg";
    config.routeInfo = "AUTO";
    constexpr int triggerTimeOut = 90;
    config.triggerCond.timeout = triggerTimeOut;
    constexpr int triggerRow = 30;
    config.triggerCond.row = triggerRow;
    config.eventConfigs.clear();
    config.eventConfigs.push_back({
        .domain = "api_diagnostic",
        .name = "api_exec_end",
        .isRealTime = false,
    });
    config.eventConfigs.push_back({
        .domain = "api_diagnostic",
        .name = "api_called_stat",
        .isRealTime = true,
    });
    config.eventConfigs.push_back({
        .domain = "api_diagnostic",
        .name = "api_called_stat_cnt",
        .isRealTime = true,
    });
    int64_t processId = AppEventProcessorMgr::AddProcessor(config);
    if (processId < 0) {
        HILOG_ERROR(LOG_CORE, "failed to init processor and ret: %{public}" PRId64, processId);
    }
    return processId;
}
}
}