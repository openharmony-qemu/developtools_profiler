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

int64_t ApiRecordReporter::processId_ = -1;

void ApiRecordReporter::InitProcessor()
{
    ffrt::submit([] {
            using namespace HiAppEvent;
            ReportConfig config;
            config.name = "ha_app_event";
            config.configName = "SDK_OCG";
            processId_ = AppEventProcessorMgr::AddProcessor(config);
            if (processId_ < 0) {
                HILOG_ERROR(LOG_CORE, "failed to init processor and ret: %{public}" PRId64, processId_);
            }
        }, {}, {});
}

SingleRecordReporter& SingleRecordReporter::GetInstance()
{
    static SingleRecordReporter singleRecordReporter;
    return singleRecordReporter;
}

void SingleRecordReporter::ReportRecord(const std::string& apiName, int errorCode, int64_t beginTime, int64_t endTime)
{
    if (processId_ < 0) {
        return;
    }
    auto task = [apiName, errorCode, beginTime, endTime] {
        HiAppEvent::Event event("api_diagnostic", "api_exec_end", HiAppEvent::BEHAVIOR);
        event.AddParam("trans_id", std::string("transId_") + std::to_string(beginTime));
        event.AddParam("api_name", apiName);
        event.AddParam("sdk_name", std::string("PerformanceAnalysisKit"));
        event.AddParam("begin_time", beginTime);
        event.AddParam("end_time", endTime);
        event.AddParam("result", errorCode ? 1 : 0);
        event.AddParam("error_code", errorCode);
        Write(event);
    };
    ffrt::submit(task, {}, {});
}

MultipleRecordReporter::MultipleRecordReporter(uint32_t timeout, uint32_t limitValue) : timeout_(timeout),
    limitValue_(limitValue) {}

void MultipleRecordReporter::ReportRecord(const std::string& apiName, int errorCode, int64_t beginTime, int64_t endTime)
{
    if (processId_ < 0) {
        return;
    }
    std::unique_lock<std::mutex> lock(mutex_);
    records_.emplace_back(errorCode);
    const int64_t costTime = endTime - beginTime;
    maxCostTime_ = std::max(maxCostTime_, costTime);
    minCostTime_ = std::min(minCostTime_, costTime);
    totalCostTime_ += costTime;
    constexpr int64_t secondToNanosecond = 1 * 1000 * 1000 * 1000;
    if (records_.size() >= limitValue_ ||
        lastReportTime_ + timeout_ * secondToNanosecond < GetElapsedNanoSecondsSinceBoot()) {
        UploadRecordData(apiName);
        lastReportTime_ = GetElapsedNanoSecondsSinceBoot();
        records_.assign({});
        minCostTime_ = std::numeric_limits<int64_t>::max();
        maxCostTime_ = std::numeric_limits<int64_t>::min();
        totalCostTime_ = 0;
    }
}

void MultipleRecordReporter::UploadRecordData(const std::string& apiName) const
{
    auto maxCostTime = maxCostTime_;
    auto minCostTime = minCostTime_;
    auto totalCostTime = totalCostTime_;
    auto task = [apiName, records = std::move(records_), maxCostTime, minCostTime, totalCostTime] {
        HiAppEvent::Event event("api_diagnostic", "api_called_stat_cnt", HiAppEvent::BEHAVIOR);
        event.AddParam("api_name", apiName);
        event.AddParam("sdk_name", std::string("PerformanceAnalysisKit"));
        event.AddParam("call_times", static_cast<int32_t>(records.size()));
        event.AddParam("error_code_num", records);
        int32_t successTime = 0;
        for (const auto& errCode : records) {
            if (errCode == 0) {
                successTime++;
            }
        }
        event.AddParam("success_times", successTime);
        event.AddParam("max_cost_time", maxCostTime);
        event.AddParam("min_cost_time", minCostTime);
        event.AddParam("total_cost_time", totalCostTime);
        Write(event);
    };
    ffrt::submit(task, {}, {});
}

ApiInvokeRecorder::ApiInvokeRecorder(std::string apiName, ApiRecordReporter& reporter) : apiName_(std::move(apiName)),
    beginTime_(GetElapsedNanoSecondsSinceBoot()), reporter_(reporter) {}

ApiInvokeRecorder::~ApiInvokeRecorder()
{
    if (beginTime_ < 0) {
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
    constexpr int milliSecondsToNanoseconds = 1000 * 1000;
    reporter_.ReportRecord(apiName_, errorCode_,
        realBeginTime / milliSecondsToNanoseconds, realEndTime / milliSecondsToNanoseconds);
}

void ApiInvokeRecorder::SetErrorCode(int errorCode)
{
    errorCode_ = errorCode;
}
}
}