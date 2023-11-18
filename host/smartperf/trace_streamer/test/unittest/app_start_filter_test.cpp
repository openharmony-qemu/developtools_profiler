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

#include <hwext/gtest-ext.h>
#include <hwext/gtest-tag.h>

#include "app_start_filter.h"
#include "parser/bytrace_parser/bytrace_parser.h"
#include "slice_filter.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class AppStartFilterTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }
    void TearDown() {}

public:
    TraceStreamerSelector stream_ = {};
};

/**
 * @tc.name: ProcessAllTest
 * @tc.desc: Process All TEST, for ProcessTouchEvent、StartUIAbilityBySCB、LoadAbility etc.
 * @tc.type: FUNC
 */
HWTEST_F(AppStartFilterTest, ProcessAllTest, TestSize.Level1)
{
    TS_LOGI("test40-1");
    std::vector<std::string> processTouchEventVec = {
        "ohos.sceneboard-2037  ( 2037) [002] .... 233.207960: print: B|2037|H:client dispatch touchId:850",
        "ohos.sceneboard-2037  ( 2037) [004] .... 233.225036: print: B|2037|H:client dispatch touchId:851",
        "ohos.sceneboard-2037  ( 2037) [004] .... 233.225153: print: B|2037|H:client dispatch touchId:852",
        "ohos.sceneboard-2037  ( 2037) [004] .... 233.225203: print: B|2037|H:client dispatch touchId:853",
    };
    std::vector<std::string> startUIAbilityBySCBVec = {
        "SceneSessionMan-2396  ( 2037) [005] .... 233.273283: print: B|2037|H:OHOS::ErrCode "
        "OHOS::AAFwk::AbilityManagerClient::StartUIAbilityBySCB(sptr<OHOS::AAFwk::SessionInfo>) ",
    };
    std::vector<std::string> loadAbilityVec = {
        "ffrtwk/CPU-2-17-4540  ( 1093) [006] .... 233.279339: print: B|1093|H:virtual void "
        "OHOS::AppExecFwk::AppMgrServiceInner::LoadAbility(const sptr<OHOS::IRemoteObject> &, const "
        "sptr<OHOS::IRemoteObject> &, const std::shared_ptr<AbilityInfo> &, const std::shared_ptr<ApplicationInfo> &, "
        "const std::shared_ptr<AAFwk::Want> &)"};
    std::string appLaunchStr(
        "ffrtwk/CPU-2-16-4537  ( 1093) [007] .... 233.300562: print: B|1093|H:virtual void "
        "OHOS::AppExecFwk::AppMgrServiceInner::AttachApplication(const pid_t, const "
        "sptr<OHOS::AppExecFwk::IAppScheduler> &)##com.taobao.taobao");
    std::string uiLaunchStr(
        "m.taobao.taobao-4794  ( 4794) [007] .... 233.309669: print: B|4794|H:void "
        "OHOS::AppExecFwk::MainThread::HandleLaunchAbility(const std::shared_ptr<AbilityLocalRecord> "
        "&)##com.taobao.taobao");
    std::string uiOnForegroundStr(
        "m.taobao.taobao-4794  ( 4794) [007] .... 233.330670: print: B|4794|H:void "
        "OHOS::AbilityRuntime::FAAbilityThread::HandleAbilityTransaction(const OHOS::AbilityRuntime::Want &, const "
        "OHOS::AbilityRuntime::LifeCycleStateInfo &, sptr<AppExecFwk::SessionInfo>)##EntryAbility");
    BytraceParser bytraceParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    for (auto&& str : processTouchEventVec) {
        bytraceParser.ParseTraceDataItem(str);
    }
    for (auto&& str : startUIAbilityBySCBVec) {
        bytraceParser.ParseTraceDataItem(str);
    }
    for (auto&& str : loadAbilityVec) {
        bytraceParser.ParseTraceDataItem(str);
    }
    bytraceParser.ParseTraceDataItem(appLaunchStr);
    bytraceParser.ParseTraceDataItem(uiLaunchStr);
    bytraceParser.ParseTraceDataItem(uiOnForegroundStr);
    bytraceParser.WaitForParserEnd();
    stream_.streamFilters_->appStartupFilter_->FilterAllAPPStartupData();
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->procTouchItems_.size() == 0);
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->startUIAbilityBySCBItems_.size() == 0);
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->loadAbilityItems_.size() == 0);
}

/**
 * @tc.name: AppLunchTest
 * @tc.desc: App Lunch TEST
 * @tc.type: FUNC
 */
HWTEST_F(AppStartFilterTest, AppLunchTest, TestSize.Level1)
{
    TS_LOGI("test40-2");

    const std::string process_create_str =
        "H:virtual void OHOS::AppExecFwk::AppMgrServiceInner::AttachApplication("
        "const pid_t, const sptr<OHOS::AppExecFwk::IAppScheduler> &)##com.ohos.smartperf";
    uint64_t ts1 = 168758662957000;
    uint64_t ts2 = 168758663011000;
    uint32_t pid1 = 1655;
    uint32_t threadGroupId1 = 1127;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex(process_create_str.c_str());

    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts1, pid1, threadGroupId1, cat, splitStrIndex);
    stream_.streamFilters_->sliceFilter_->EndSlice(ts2, pid1, threadGroupId1);

    stream_.streamFilters_->appStartupFilter_->FilterAllAPPStartupData();

    auto dataIndex = stream_.traceDataCache_->GetDataIndex("com.ohos.smartperf");
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.size() == 1);
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.find(dataIndex) !=
                stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.end());
}

/**
 * @tc.name: LunchTest
 * @tc.desc: Lunch TEST
 * @tc.type: FUNC
 */
HWTEST_F(AppStartFilterTest, LunchTest, TestSize.Level1)
{
    TS_LOGI("test40-3");

    const std::string process_create_str =
        "H:void OHOS::AppExecFwk::MainThread::HandleLaunchAbility("
        "const std::shared_ptr<AbilityLocalRecord> &)##com.ohos.smartperf";
    uint64_t ts1 = 168758662957000;
    uint64_t ts2 = 168758663011000;
    uint32_t pid1 = 1655;
    uint32_t threadGroupId1 = 1127;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex(process_create_str.c_str());

    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts1, pid1, threadGroupId1, cat, splitStrIndex);
    stream_.streamFilters_->sliceFilter_->EndSlice(ts2, pid1, threadGroupId1);

    stream_.streamFilters_->appStartupFilter_->FilterAllAPPStartupData();

    auto dataIndex = stream_.traceDataCache_->GetDataIndex("com.ohos.smartperf");
    EXPECT_EQ(stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.size(), 0);
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.find(dataIndex) ==
                stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.end());
}

/**
 * @tc.name: OnforegroundTest
 * @tc.desc: Onforeground TEST
 * @tc.type: FUNC
 */
HWTEST_F(AppStartFilterTest, OnforegroundTest, TestSize.Level1)
{
    TS_LOGI("test40-4");

    const std::string process_create_str =
        "H:void OHOS::AppExecFwk::AbilityThread::HandleAbilityTransaction("
        "const OHOS::AppExecFwk::Want &, const OHOS::AppExecFwk::LifeCycleStateInfo &, "
        "sptr<OHOS::AAFwk::SessionInfo>)##";
    uint64_t ts1 = 168758662957000;
    uint64_t ts2 = 168758663011000;
    uint32_t pid1 = 1655;
    uint32_t threadGroupId1 = 1127;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex(process_create_str.c_str());

    stream_.streamFilters_->sliceFilter_->BeginSlice(".ohos.smartperf", ts1, pid1, threadGroupId1, cat, splitStrIndex);
    stream_.streamFilters_->sliceFilter_->EndSlice(ts2, pid1, threadGroupId1);

    stream_.streamFilters_->appStartupFilter_->FilterAllAPPStartupData();

    auto dataIndex = stream_.traceDataCache_->GetDataIndex(".ohos.smartperf");
    EXPECT_EQ(stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.size(), 0);
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.find(dataIndex) ==
                stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.end());
}

/**
 * @tc.name: SoInitalizationTest
 * @tc.desc: SoInitalization TEST
 * @tc.type: FUNC
 */
HWTEST_F(AppStartFilterTest, SoInitalizationTest, TestSize.Level1)
{
    TS_LOGI("test40-5");

    const std::string process_create_str = "dlopen:  /system/lib64/libcpudataplugin.z.so";
    uint64_t ts1 = 168758614489000;
    uint64_t ts2 = 168758663011000;
    uint32_t pid1 = 2235;
    uint32_t threadGroupId1 = 2221;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex(process_create_str.c_str());

    stream_.streamFilters_->sliceFilter_->BeginSlice("UnixSocketRecv", ts1, pid1, threadGroupId1, cat, splitStrIndex);
    stream_.streamFilters_->sliceFilter_->EndSlice(ts2, pid1, threadGroupId1);
    stream_.streamFilters_->appStartupFilter_->FilterAllAPPStartupData();

    EXPECT_TRUE(stream_.traceDataCache_->GetConstStaticInitalizationData().Size() == 1);
}

} // namespace TraceStreamer
} // namespace SysTuning
