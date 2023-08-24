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
 * @tc.name: ProcessCreateTest
 * @tc.desc: Process Create TEST
 * @tc.type: FUNC
 */
HWTEST_F(AppStartFilterTest, ProcessCreateTest, TestSize.Level1)
{
    TS_LOGI("test40-1");

    const std::string parent_str =
        "H:virtual int OHOS::AAFwk::AbilityManagerService::StartAbility("
        "const OHOS::AAFwk::Want &, const sptr<OHOS::IRemoteObject> &, int32_t, int)";
    const std::string process_create_str =
        "H:int OHOS::AAFwk::MissionListManager::StartAbilityLocked("
        "const std::shared_ptr<AbilityRecord> &, const std::shared_ptr<AbilityRecord> &, "
        "const OHOS::AAFwk::AbilityRequest &)##com.ohos.smartperf##MainAbility";
    uint64_t ts1 = 168758662957000;
    uint64_t ts2 = 168758663011000;
    uint64_t ts3 = 168758663057000;
    uint64_t ts4 = 168758663111000;
    uint32_t pid1 = 1655;
    uint32_t threadGroupId1 = 1127;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("Catalog");
    DataIndex parent_splitStrIndex = stream_.traceDataCache_->GetDataIndex(parent_str.c_str());
    DataIndex splitStrIndex = stream_.traceDataCache_->GetDataIndex(process_create_str.c_str());

    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts1, pid1, threadGroupId1, cat, parent_splitStrIndex);
    stream_.streamFilters_->sliceFilter_->BeginSlice("comm", ts2, pid1, threadGroupId1, cat, splitStrIndex);
    stream_.streamFilters_->sliceFilter_->EndSlice(ts3, pid1, threadGroupId1);
    stream_.streamFilters_->sliceFilter_->EndSlice(ts4, pid1, threadGroupId1);

    stream_.streamFilters_->appStartupFilter_->FilterAllAPPStartupData();

    auto dataIndex = stream_.traceDataCache_->GetDataIndex("com.ohos.smartperf");
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.size() == 1);
    EXPECT_TRUE(stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.find(dataIndex) !=
                stream_.streamFilters_->appStartupFilter_->mAPPStartupData_.end());
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
