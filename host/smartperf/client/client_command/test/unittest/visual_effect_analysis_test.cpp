/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include "visual_effect_analysis.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace SmartPerf {
class VisualEffectAnalysisTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    void SetUp() {}
    void TearDown() {}
};

HWTEST_F(VisualEffectAnalysisTest, GetInstance_001, TestSize.Level1)
{
    VisualEffectAnalysis &instance1 = VisualEffectAnalysis::GetInstance();
    VisualEffectAnalysis &instance2 = VisualEffectAnalysis::GetInstance();
    
    EXPECT_EQ(&instance1, &instance2);
}

HWTEST_F(VisualEffectAnalysisTest, InitdynamicEffectRsData_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    
    analysis.InitdynamicEffectRsData();
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, InitdynamicEffectFeData_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    
    analysis.InitdynamicEffectFeData();
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetClusterNum_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "cpu_id=0 state=1000";
    
    int result = analysis.GetClusterNum(line);
    
    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectAnalysisTest, GetClusterNum_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "cpu_id=7 state=2000";
    
    int result = analysis.GetClusterNum(line);
    
    EXPECT_EQ(result, 7);
}

HWTEST_F(VisualEffectAnalysisTest, GetClusterNum_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "no_cpu_id_here";
    
    int result = analysis.GetClusterNum(line);
    
    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectAnalysisTest, GetClusterNum_004, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "";
    
    int result = analysis.GetClusterNum(line);
    
    EXPECT_EQ(result, 0);
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuCycle_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "part1|part2|part3|12345|part5";
    
    string result = analysis.GetCpuCycle(line);
    
    EXPECT_EQ(result, "12345");
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuCycle_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "a|b|c|cycle_value";
    
    string result = analysis.GetCpuCycle(line);
    
    EXPECT_EQ(result, "cycle_value");
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuCycle_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "not_enough_pipes";
    
    string result = analysis.GetCpuCycle(line);
    
    EXPECT_EQ(result, "");
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuCycle_004, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "";
    
    string result = analysis.GetCpuCycle(line);
    
    EXPECT_EQ(result, "");
}

HWTEST_F(VisualEffectAnalysisTest, SetTraceTime_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    int time = 5000;
    
    analysis.SetTraceTime(time);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetTraceTime_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    int time = 0;
    
    analysis.SetTraceTime(time);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetTraceTime_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    int time = -100;
    
    analysis.SetTraceTime(time);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetTraceBegin_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string traceStartPoint = "begin_marker";
    
    analysis.SetTraceBegin(traceStartPoint);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetTraceBegin_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string emptyBegin = "";
    
    analysis.SetTraceBegin(emptyBegin);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetTraceEnd_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string traceEndPoint = "end_marker";
    
    analysis.SetTraceEnd(traceEndPoint);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetTraceEnd_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string emptyEnd = "";
    
    analysis.SetTraceEnd(emptyEnd);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetGpuFre_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "gpufreq-0 gpufreq state=800000";
    
    analysis.GetGpuFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetGpuFre_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "gpufreq-1 gpufreq state=600000";
    
    analysis.GetGpuFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetGpuFre_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "no_gpufreq_here";
    
    analysis.GetGpuFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetGpuFre_004, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "gpufreq-0 gpufreq state=";
    
    analysis.GetGpuFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GpuLoad_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "gpufreq-0 gpuload state=50";
    
    analysis.GpuLoad(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GpuLoad_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "gpufreq-1 gpuload state=75";
    
    analysis.GpuLoad(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GpuLoad_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "no_gpuload_here";
    
    analysis.GpuLoad(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetUtCpuCycle_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "RSUniRenderThre OnRenderFramePerfCount-Ins|data|12345|more";
    
    analysis.GetUtCpuCycle(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetUtCpuCycle_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "other_thread other_function|data|67890|more";
    
    analysis.GetUtCpuCycle(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetRefreshRate_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "pendingRefreshRate: 60";
    
    analysis.GetRefreshRate(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetRefreshRate_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "pendingRefreshRate: 90";
    
    analysis.GetRefreshRate(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetRefreshRate_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "no_refresh_rate";
    
    analysis.GetRefreshRate(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetRefreshRate_004, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "pendingRefreshRate: 120";
    analysis.GetRefreshRate(line);
    string line2 = "pendingRefreshRate: 60";
    
    analysis.GetRefreshRate(line2);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetGpuCycle_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "GPU Cycles data|12345|67890|99999";
    
    analysis.GetGpuCycle(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetGpuCycle_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "no_gpu_cycles";
    
    analysis.GetGpuCycle(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuIdFre_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.GetDeviceCpuNum();
    string line = "cpu_frequency: cpu_id=0 state=1800000";
    
    analysis.GetCpuIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuIdFre_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.GetDeviceCpuNum();
    string line = "cpu_frequency: cpu_id=3 state=2000000";
    
    analysis.GetCpuIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuIdFre_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.GetDeviceCpuNum();
    string line = "no_cpu_frequency";
    
    analysis.GetCpuIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetDdrIdFre_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "ddr_cluster cpu_id=0 state=1800000";
    
    analysis.GetDdrIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetDdrIdFre_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "ddr_cluster cpu_id=1 state=2000000";
    
    analysis.GetDdrIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetDdrIdFre_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "no_ddr_cluster";
    
    analysis.GetDdrIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuFre_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.GetDeviceCpuNum();
    analysis.InitdynamicEffectRsData();
    string line = "cpu_frequency: cpu_id=0 state=1800000";
    
    analysis.GetCpuIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetCpuFre_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.GetDeviceCpuNum();
    analysis.InitdynamicEffectRsData();
    string line = "cpu_frequency: cpu_id=1 state=2000000";
    string line2 = "cpu_frequency: cpu_id=1 state=2500000";
    
    analysis.GetCpuIdFre(line);
    analysis.GetCpuIdFre(line2);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetDdrFre_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.GetDeviceDdrNum();
    analysis.InitdynamicEffectRsData();
    string line = "ddr_cluster cpu_id=0 state=1800";
    
    analysis.GetDdrIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetDdrFre_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.GetDeviceDdrNum();
    analysis.InitdynamicEffectRsData();
    string line = "ddr_cluster cpu_id=1 state=2000";
    
    analysis.GetDdrIdFre(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetExternalMemoryRead_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "External Memory Read [KB] | data|1024";
    
    analysis.GetExternalMemoryRead(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetExternalMemoryRead_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "no_external_memory_read";
    
    analysis.GetExternalMemoryRead(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetexternalMemoryWrite_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "External Memory Write [KB] | data|2048";
    
    analysis.GetexternalMemoryWrite(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetexternalMemoryWrite_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "no_external_memory_write";
    
    analysis.GetexternalMemoryWrite(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetexternalMemoryTraffic_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "External Memory Traffic [KB] | data|4096";
    
    analysis.GetexternalMemoryTraffic(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetexternalMemoryTraffic_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "no_external_memory_traffic";
    
    analysis.GetexternalMemoryTraffic(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetCutTime_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "10:123.456 some data";
    
    string result = analysis.GetCutTime(line);
    
    EXPECT_FALSE(result.empty());
}

HWTEST_F(VisualEffectAnalysisTest, GetCutTime_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "no_timestamp_here";
    
    string result = analysis.GetCutTime(line);
    
    EXPECT_FALSE(result.empty());
}

HWTEST_F(VisualEffectAnalysisTest, GetBeginTraceTime_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "10:123.456 data";
    
    analysis.GetBeginTraceTime(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetBeginTraceTime_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "# comment line";
    
    analysis.GetBeginTraceTime(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetBeginTraceTime_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line = "data without timestamp";
    
    analysis.GetBeginTraceTime(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetRsLaneMap_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "render_service OnReadablePerfCount-Ins data";
    
    analysis.GetRsLaneMap(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetRsLaneMap_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "SceneAdapter::RenderFunction data";
    
    analysis.GetRsLaneMap(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetRsLaneMap_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectRsData();
    string line = "other service other_function data";
    
    analysis.GetRsLaneMap(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetFeLaneMap_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectFeData();
    string line = "H:Waiting for Acquire Fence data";
    
    analysis.GetFeLaneMap(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetFeLaneMap_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectFeData();
    string line = "GPU Cycles data|12345";
    
    analysis.GetFeLaneMap(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetFeLaneMap_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    analysis.InitdynamicEffectFeData();
    string line = "External Memory Read [KB] | data|1024";
    
    analysis.GetFeLaneMap(line);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, VisualEffectRanking_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string beginStr = "BEGIN";
    string overStr = "END";
    
    analysis.VisualEffectRanking(beginStr, overStr);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, VisualEffectRanking_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string emptyBegin = "";
    string emptyOver = "";
    
    analysis.VisualEffectRanking(emptyBegin, emptyOver);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, VisualEffectRanking_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string beginStr = "";
    string overStr = "END";
    
    analysis.VisualEffectRanking(beginStr, overStr);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, InitIndexInfo_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    
    analysis.InitIndexInfo();
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetDeviceCpuNum_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    
    analysis.GetDeviceCpuNum();
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GetDeviceDdrNum_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    
    analysis.GetDeviceDdrNum();
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, MultipleInitCalls_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    
    analysis.InitdynamicEffectRsData();
    analysis.InitdynamicEffectRsData();
    analysis.InitdynamicEffectRsData();
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, MultipleInitCalls_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    
    analysis.InitdynamicEffectFeData();
    analysis.InitdynamicEffectFeData();
    analysis.InitdynamicEffectFeData();
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetMultipleTimes_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    int time1 = 5000;
    int time2 = 10000;
    int time3 = 15000;
    
    analysis.SetTraceTime(time1);
    analysis.SetTraceTime(time2);
    analysis.SetTraceTime(time3);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetMultipleBeginPoints_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string begin1 = "marker1";
    string begin2 = "marker2";
    string begin3 = "marker3";
    
    analysis.SetTraceBegin(begin1);
    analysis.SetTraceBegin(begin2);
    analysis.SetTraceBegin(begin3);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, SetMultipleEndPoints_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string end1 = "end_marker1";
    string end2 = "end_marker2";
    string end3 = "end_marker3";
    
    analysis.SetTraceEnd(end1);
    analysis.SetTraceEnd(end2);
    analysis.SetTraceEnd(end3);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, EmptyLines_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string emptyLine = "";
    
    analysis.GetRsLaneMap(emptyLine);
    analysis.GetFeLaneMap(emptyLine);
    analysis.GetGpuFre(emptyLine);
    analysis.GpuLoad(emptyLine);
    analysis.GetUtCpuCycle(emptyLine);
    analysis.GetRefreshRate(emptyLine);
    analysis.GetGpuCycle(emptyLine);
    analysis.GetCpuIdFre(emptyLine);
    analysis.GetDdrIdFre(emptyLine);
    analysis.GetExternalMemoryRead(emptyLine);
    analysis.GetexternalMemoryWrite(emptyLine);
    analysis.GetexternalMemoryTraffic(emptyLine);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GpuLoadAccumulation_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line1 = "gpufreq-0 gpuload state=50";
    string line2 = "gpufreq-0 gpuload state=60";
    string line3 = "gpufreq-0 gpuload state=70";
    
    analysis.GpuLoad(line1);
    analysis.GpuLoad(line2);
    analysis.GpuLoad(line3);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, GpuCycleAccumulation_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line1 = "GPU Cycles data|1000|2000|3000";
    string line2 = "GPU Cycles data|4000|5000|6000";
    string line3 = "GPU Cycles data|7000|8000|9000";
    
    analysis.GetGpuCycle(line1);
    analysis.GetGpuCycle(line2);
    analysis.GetGpuCycle(line3);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, ExternalMemoryAccumulation_001, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line1 = "External Memory Read [KB] | data|1024";
    string line2 = "External Memory Read [KB] | data|2048";
    string line3 = "External Memory Read [KB] | data|4096";
    
    analysis.GetExternalMemoryRead(line1);
    analysis.GetExternalMemoryRead(line2);
    analysis.GetExternalMemoryRead(line3);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, ExternalMemoryAccumulation_002, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line1 = "External Memory Write [KB] | data|512";
    string line2 = "External Memory Write [KB] | data|1024";
    string line3 = "External Memory Write [KB] | data|2048";
    
    analysis.GetexternalMemoryWrite(line1);
    analysis.GetexternalMemoryWrite(line2);
    analysis.GetexternalMemoryWrite(line3);
    
    SUCCEED();
}

HWTEST_F(VisualEffectAnalysisTest, ExternalMemoryAccumulation_003, TestSize.Level1)
{
    VisualEffectAnalysis &analysis = VisualEffectAnalysis::GetInstance();
    string line1 = "External Memory Traffic [KB] | data|8192";
    string line2 = "External Memory Traffic [KB] | data|16384";
    string line3 = "External Memory Traffic [KB] | data|32768";
    
    analysis.GetexternalMemoryTraffic(line1);
    analysis.GetexternalMemoryTraffic(line2);
    analysis.GetexternalMemoryTraffic(line3);
    
    SUCCEED();
}

}
}
