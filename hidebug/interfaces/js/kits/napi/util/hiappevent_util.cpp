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
#include "ffrt.h"
#include "hidebug_util.h"
#include "hilog/log.h"

namespace OHOS {
namespace HiviewDFX {

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D0A
#undef LOG_TAG
#define LOG_TAG "HIAPPEVENT_UTIL"

int64_t ApiInvokeRecorder::processId_ = -1;

ApiInvokeRecorder::ApiInvokeRecorder(std::string apiName) : apiName_(std::move(apiName)),
    beginTime_(GetElapsedNanoSecondsSinceBoot()) {}

ApiInvokeRecorder::~ApiInvokeRecorder()
{
    if (processId_ < 0 || beginTime_ < 0) {
        return;
    }
    const int64_t costTime = GetElapsedNanoSecondsSinceBoot() - beginTime_;
    if (costTime < 0) {
        return;
    }
    int64_t realEndTime = GetRealNanoSecondsTimestamp();
    int64_t realBeginTime = realEndTime - costTime;
    if (realBeginTime < 0 || realEndTime < 0) {
        return;
    }
    std::string apiName(std::move(apiName_));
    int errorCode(errorCode_);
    auto task = [apiName, realEndTime, realBeginTime, errorCode] {
        HiAppEvent::Event event("api_diagnostic", "api_exec_end", HiAppEvent::BEHAVIOR);
        event.AddParam("trans_id", std::string("transId_") + std::to_string(realBeginTime));
        event.AddParam("api_name", apiName);
        event.AddParam("sdk_name", std::string("PerformanceAnalysisKit"));
        constexpr int milliSecondsToNanoseconds = 1000 * 1000;
        event.AddParam("begin_time", realBeginTime / milliSecondsToNanoseconds);
        event.AddParam("end_time", realEndTime / milliSecondsToNanoseconds);
        event.AddParam("result", static_cast<int>(errorCode == 0));
        event.AddParam("error_code", errorCode);
        Write(event);
    };
    ffrt::submit(task, {}, {});
}

void ApiInvokeRecorder::SetErrorCode(int errorCode)
{
    errorCode_ = errorCode;
}

void ApiInvokeRecorder::InitProcessor()
{
    ffrt::submit([] {
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
            processId_ = AppEventProcessorMgr::AddProcessor(config);
            if (processId_ < 0) {
                HILOG_ERROR(LOG_CORE, "failed to init processor and ret: %{public}" PRId64, processId_);
            }
        }, {}, {});
}
}
}