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

#ifndef HIAPPEVENT_UTIL_H_
#define HIAPPEVENT_UTIL_H_
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "hidebug_util.h"

namespace OHOS {
namespace HiviewDFX {

class ApiRecordReporter {
public:
    virtual ~ApiRecordReporter() = default;
    virtual void ReportRecord(const std::string& apiName, int errorCode, int64_t beginTime, int64_t endTime) = 0;
    static void InitProcessor();
protected:
    static int64_t processId_;
};

class SingleRecordReporter final : public ApiRecordReporter {
public:
    static SingleRecordReporter& GetInstance();
    SingleRecordReporter(const SingleRecordReporter&) = delete;
    SingleRecordReporter& operator=(const SingleRecordReporter&) = delete;
    SingleRecordReporter(SingleRecordReporter&&) = delete;
    SingleRecordReporter& operator=(SingleRecordReporter&&) = delete;
    void ReportRecord(const std::string& apiName, int errorCode, int64_t beginTime, int64_t endTime) override;
private:
    SingleRecordReporter() = default;
    ~SingleRecordReporter() override = default;
};

class MultipleRecordReporter final : public ApiRecordReporter {
public:
    MultipleRecordReporter(uint32_t timeout, uint32_t limitValue);
    void ReportRecord(const std::string& apiName, int errorCode, int64_t beginTime, int64_t endTime) override;
private:
    void UploadRecordData(const std::string& apiName) const;
    std::mutex mutex_;
    const uint32_t timeout_;
    const uint32_t limitValue_;
    int64_t lastReportTime_{GetElapsedNanoSecondsSinceBoot()};
    int64_t totalCostTime_{0};
    int64_t maxCostTime_{std::numeric_limits<int64_t>::min()};
    int64_t minCostTime_{std::numeric_limits<int64_t>::max()};;
    std::vector<int32_t> records_{};
};

class ApiInvokeRecorder {
public:
    explicit ApiInvokeRecorder(std::string apiName, ApiRecordReporter& reporter = SingleRecordReporter::GetInstance());
    ~ApiInvokeRecorder();
    void SetErrorCode(int errorCode);

private:
    std::string apiName_;
    int errorCode_{0};
    int64_t beginTime_;
    ApiRecordReporter& reporter_;
};
}
}
#endif // HIAPPEVENT_UTIL_H_
