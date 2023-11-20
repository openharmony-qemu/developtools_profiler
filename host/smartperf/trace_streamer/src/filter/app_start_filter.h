/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef APP_START_FILTER_H
#define APP_START_FILTER_H

#include <cstdint>
#include <vector>
#include "filter_base.h"
#include "trace_data_cache.h"
#include "trace_streamer_filters.h"
#include "ts_common.h"

namespace SysTuning {
namespace TraceStreamer {
enum StartupApp {
    PROCESS_TOUCH = 0,
    START_UI_ABILITY_BY_SCB,
    LOAD_ABILITY,
    APPLICATION_LAUNCHING,
    UI_ABILITY_LAUNCHING,
    UI_ABILITY_ONFOREGROUND,
    FIRST_FRAME_APP_PHASE,
    FIRST_FRAME_RENDER_PHASE
};
class APPStartupData {
public:
    APPStartupData(uint32_t callid, uint32_t ipid, uint32_t tid, uint64_t startTime, uint64_t endTime)
        : callid_(callid), ipid_(ipid), tid_(tid), startTime_(startTime), endTime_(endTime)
    {
    }
    uint32_t callid_;
    uint32_t ipid_;
    uint32_t tid_;
    uint64_t startTime_;
    uint64_t endTime_;
};

class APPStartupFilter : private FilterBase {
public:
    APPStartupFilter(TraceDataCache* dataCache, const TraceStreamerFilters* filter);
    APPStartupFilter(const APPStartupFilter&) = delete;
    APPStartupFilter& operator=(const APPStartupFilter&) = delete;
    ~APPStartupFilter() override;
    void FilterAllAPPStartupData();

private:
    using appMap = std::unordered_map<DataIndex, std::map<uint32_t, std::unique_ptr<APPStartupData>>>;
    void ParserSoInitalization();
    void CalcDepthByTimeStamp(std::map<uint32_t, std::map<uint64_t, uint32_t>>::iterator it,
                              uint32_t& depth,
                              uint64_t endTime,
                              uint64_t startTime);
    void ParserAppStartup();
    void UpdatePidByNameIndex(const appMap& mAPPStartupData);
    bool CaclRsDataByPid(appMap& mAPPStartupData);
    void AppendData(const appMap& mAPPStartupData);
    bool UpdateAPPStartupData(uint32_t row, const std::string& nameString, uint32_t startIndex);
    bool ProcAbilityLaunchData(const std::string& nameString, uint64_t raw);
    void ProcForegroundData(uint64_t raw);

private:
    std::deque<std::unique_ptr<APPStartupData>> procTouchItems_;
    std::deque<std::unique_ptr<APPStartupData>> startUIAbilityBySCBItems_;
    std::deque<std::unique_ptr<APPStartupData>> loadAbilityItems_;
    appMap mAPPStartupData_;
    std::unordered_map<uint32_t, appMap> mAPPStartupDataWithPid_;
    const std::string procTouchCmd_ = "H:client dispatch touchId:";
    const std::string startUIAbilityBySCBCmd_ =
        "H:OHOS::ErrCode OHOS::AAFwk::AbilityManagerClient::StartUIAbilityBySCB";
    const std::string loadAbilityCmd_ = "H:virtual void OHOS::AppExecFwk::AppMgrServiceInner::LoadAbility";
    const std::string appLaunchCmd_ =
        "H:virtual void OHOS::AppExecFwk::AppMgrServiceInner::AttachApplication(const pid_t, const "
        "sptr<OHOS::AppExecFwk::IAppScheduler> &)##";
    const std::string uiLaunchCmd_ =
        "H:void OHOS::AppExecFwk::MainThread::HandleLaunchAbility(const std::shared_ptr<AbilityLocalRecord> &)##";
    const std::string uiOnForegroundCmd_ =
        "H:void OHOS::AbilityRuntime::FAAbilityThread::HandleAbilityTransaction(const OHOS::AbilityRuntime::Want &, "
        "const "
        "OHOS::AbilityRuntime::LifeCycleStateInfo &, sptr<AppExecFwk::SessionInfo>)##";
    const std::string dlopenCmd_ = "dlopen:";
};
} // namespace TraceStreamer
} // namespace SysTuning

#endif // APP_START_FILTER_H
