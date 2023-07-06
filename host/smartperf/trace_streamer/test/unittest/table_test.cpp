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

#include "htrace_mem_parser.h"
#include "rpc_server.h"
#include "trace_data_cache.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class TableTest : public ::testing::Test {
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
 * @tc.name: AppnameTableTest
 * @tc.desc: Appname Table Test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, AppnameTableTest, TestSize.Level1)
{
    TS_LOGI("test31-1");
    std::string sqlSelect = "select * from app_name";
    uint8_t flags = 0;
    DataIndex eventSource = stream_.traceDataCache_->GetDataIndex("eventSource");
    DataIndex appName = stream_.traceDataCache_->GetDataIndex("app1");
    stream_.traceDataCache_->GetAppNamesData()->AppendAppName(flags, eventSource, appName);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: ArgsTableTest
 * @tc.desc: Args Table Test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ArgsTableTest, TestSize.Level1)
{
    TS_LOGI("test31-2");
    std::string sqlSelect = "select * from args";
    std::string sqlSelect1 = "select * from args where id = 1";
    std::string sqlSelect2 = "select * from args where key > 1";
    std::string sqlSelect3 = "select * from args where id < 1";
    std::string sqlSelect4 = "select * from args where id >= 1";
    std::string sqlSelect5 = "select * from args where id <= 1";
    DataIndex nameId0 = stream_.traceDataCache_->GetDataIndex("args0");
    DataIndex nameId1 = stream_.traceDataCache_->GetDataIndex("args1");
    BaseDataType dataType0 = BASE_DATA_TYPE_INT;
    BaseDataType dataType1 = BASE_DATA_TYPE_STRING;
    int64_t value0 = 123;
    int64_t value1 = 456;
    size_t argSet0 = 321;
    size_t argSet1 = 654;
    stream_.traceDataCache_->InitDB();
    stream_.traceDataCache_->GetArgSetData()->AppendNewArg(nameId0, dataType0, value0, argSet0);
    stream_.traceDataCache_->GetArgSetData()->AppendNewArg(nameId1, dataType1, value1, argSet1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: CallstackTableTest
 * @tc.desc: Callstack Table Test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, CallstackTableTest, TestSize.Level1)
{
    TS_LOGI("test31-3");
    std::string sqlSelect = "select * from callstack";
    std::string sqlSelect1 = "select * from callstack where id = 1";
    std::string sqlSelect2 = "select * from callstack where ts > 1";
    std::string sqlSelect3 = "select * from callstack where callid < 1";
    std::string sqlSelect4 = "select * from callstack where cookie >= 1";
    std::string sqlSelect5 = "select * from callstack where cookie <= 1";
    uint64_t startT = 1;
    uint64_t durationNs = 1;
    InternalTid internalTid = 1;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("callstack");
    uint16_t nameIdentify = 1;
    DataIndex name = stream_.traceDataCache_->GetDataIndex("name");
    uint8_t depth = 1;
    uint64_t cookid = stream_.traceDataCache_->GetDataIndex("cook");
    const std::optional<uint64_t>& parentId = 1;

    uint64_t startT1 = 1;
    uint64_t durationNs1 = 1;
    InternalTid internalTid1 = 1;
    DataIndex cat1 = stream_.traceDataCache_->GetDataIndex("callstack1");
    uint16_t nameIdentify1 = 1;
    DataIndex name1 = stream_.traceDataCache_->GetDataIndex("name1");
    uint8_t depth1 = 1;
    uint64_t cookid1 = stream_.traceDataCache_->GetDataIndex("cook1");
    const std::optional<uint64_t>& parentId1 = 1;

    stream_.traceDataCache_->InitDB();
    stream_.traceDataCache_->GetInternalSlicesData()->AppendInternalAsyncSlice(
        startT, durationNs, internalTid, cat, nameIdentify, name, depth, cookid, parentId);
    stream_.traceDataCache_->GetInternalSlicesData()->AppendInternalAsyncSlice(
        startT1, durationNs1, internalTid1, cat1, nameIdentify1, name1, depth1, cookid1, parentId1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: ClkEventFilterTableTest
 * @tc.desc: ClkEvent filter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ClkEventFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-4");
    std::string sqlSelect = "select * from clk_event_filter";
    std::string sqlSelect1 = "select * from clk_event_filter where id = 1";
    std::string sqlSelect2 = "select * from clk_event_filter where name < 0";
    std::string sqlSelect3 = "select * from clk_event_filter where id > 0";
    std::string sqlSelect4 = "select * from clk_event_filter where id >= 0";
    std::string sqlSelect5 = "select * from clk_event_filter where id <= 0";
    uint64_t id = 1;
    uint64_t rate = 1;
    DataIndex name = stream_.traceDataCache_->GetDataIndex("name");
    uint64_t cpu = 1;

    uint64_t id1 = 1;
    uint64_t rate1 = 0;
    DataIndex name1 = stream_.traceDataCache_->GetDataIndex("name1");
    uint64_t cpu1 = 1;
    stream_.traceDataCache_->GetClkEventFilterData()->AppendNewFilter(id, rate, name, cpu);
    stream_.traceDataCache_->GetClkEventFilterData()->AppendNewFilter(id1, rate1, name1, cpu1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: ClockEventFilterTableTest
 * @tc.desc: ClockEvent filter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ClockEventFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-5");
    std::string sqlSelect = "select * from clock_event_filter";
    std::string sqlSelect1 = "select * from clock_event_filter where id = 1";
    std::string sqlSelect2 = "select * from clock_event_filter where type > 1";
    std::string sqlSelect3 = "select * from clock_event_filter where name < 1";
    std::string sqlSelect4 = "select * from clock_event_filter where cpu >= 1";
    std::string sqlSelect5 = "select * from clock_event_filter where id <= 1";
    uint64_t id = 1;
    uint64_t type = 1;
    DataIndex name = stream_.traceDataCache_->GetDataIndex("name");
    uint64_t cpu = 1;

    uint64_t id1 = 1;
    uint64_t type1 = 0;
    DataIndex name1 = stream_.traceDataCache_->GetDataIndex("name1");
    uint64_t cpu1 = 1;
    stream_.traceDataCache_->GetClockEventFilterData()->AppendNewFilter(id, type, name, cpu);
    stream_.traceDataCache_->GetClockEventFilterData()->AppendNewFilter(id1, type1, name1, cpu1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: CpuMeasureFilterTableTest
 * @tc.desc: Cpu measure filter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, CpuMeasureFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-6");
    std::string sqlSelect = "select * from cpu_measure_filter";
    std::string sqlSelect1 = "select * from cpu_measure_filter where id = 1";
    std::string sqlSelect2 = "select * from cpu_measure_filter where id > 1";
    std::string sqlSelect3 = "select * from cpu_measure_filter where type < 1";
    std::string sqlSelect4 = "select * from cpu_measure_filter where name >= 1";
    std::string sqlSelect5 = "select * from cpu_measure_filter where cpu <= 1";
    uint64_t filterId = 2;
    DataIndex name = stream_.traceDataCache_->GetDataIndex("name");
    uint32_t cpu = 1;

    uint64_t filterId1 = 1;
    DataIndex name1 = stream_.traceDataCache_->GetDataIndex("name1");
    uint32_t cpu1 = 2;

    stream_.traceDataCache_->GetCpuMeasuresData()->AppendNewFilter(filterId, name, cpu);
    stream_.traceDataCache_->GetCpuMeasuresData()->AppendNewFilter(filterId1, name1, cpu1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: CpuUsageFilterTableTest
 * @tc.desc: Cpu usage filter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, CpuUsageFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-7");
    std::string sqlSelect = "select * from cpu_usage";
    uint64_t newTimeStamp = 1663869124160;
    uint64_t dur = 560;
    double totalLoad = 2;
    double userLoad = 2;
    double systemLoad = 2;
    int64_t thread = 2;

    uint64_t newTimeStamp1 = 1663869224160;
    uint64_t dur1 = 550;
    double totalLoad1 = 1;
    double userLoad1 = 1;
    double systemLoad1 = 1;
    int64_t thread1 = 1;

    stream_.traceDataCache_->GetCpuUsageInfoData()->AppendNewData(newTimeStamp, dur, totalLoad, userLoad, userLoad,
                                                                  thread);
    stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    stream_.traceDataCache_->GetCpuUsageInfoData()->AppendNewData(newTimeStamp1, dur1, totalLoad1, userLoad1, userLoad1,
                                                                  thread1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: DataDictTableTest
 * @tc.desc: Data dict table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, DataDictTableTest, TestSize.Level1)
{
    TS_LOGI("test31-8");
    std::string sqlSelect = "select * from data_dict";
    std::string sqlSelect1 = "select * from data_dict where id = 1";
    std::string sqlSelect2 = "select * from data_dict where id > 1";
    std::string sqlSelect3 = "select * from data_dict where id < 1";
    std::string sqlSelect4 = "select * from data_dict where id >= 1";
    std::string sqlSelect5 = "select * from data_dict where data <= 1";
    stream_.traceDataCache_->GetDataFromDict(1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 59);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 57);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 58);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: DataTypeTableTest
 * @tc.desc: Data type table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, DataTypeTableTest, TestSize.Level1)
{
    TS_LOGI("test31-9");
    std::string sqlSelect = "select * from data_type";
    std::string sqlSelect1 = "select * from data_type where id = 1";
    std::string sqlSelect2 = "select * from data_type where id > 1";
    std::string sqlSelect3 = "select * from data_type where id < 1";
    std::string sqlSelect4 = "select * from data_type where typeId >= 1";
    std::string sqlSelect5 = "select * from data_type where id <= 1";
    BaseDataType dataType = BASE_DATA_TYPE_INT;
    DataIndex dataDescIndex = stream_.traceDataCache_->GetDataIndex("dataDescIndex");
    BaseDataType dataType1 = BASE_DATA_TYPE_STRING;
    DataIndex dataDescIndex1 = stream_.traceDataCache_->GetDataIndex("dataDescIndex1");

    stream_.traceDataCache_->GetDataTypeData()->AppendNewDataType(dataType, dataDescIndex);
    stream_.traceDataCache_->GetDataTypeData()->AppendNewDataType(dataType1, dataDescIndex1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 6);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 4);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 4);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: DiskIoTableTest
 * @tc.desc: Disk io table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, DiskIoTableTest, TestSize.Level1)
{
    TS_LOGI("test31-10");
    std::string sqlSelect = "select * from diskio";
    uint64_t ts = 1663869124160;
    uint64_t dur = 540;
    uint64_t rd = 5;
    uint64_t wr = 5;
    uint64_t rdPerSec = 6;
    uint64_t wrPerSec = 6;
    double rdCountPerSec = 2;
    double wrCountPerSec = 2;
    uint64_t rdCount = 2;
    uint64_t wrCount = 2;

    stream_.traceDataCache_->GetDiskIOData()->AppendNewData(ts, dur, rd, wr, rdPerSec, wrPerSec, rdCountPerSec,
                                                            wrCountPerSec, rdCount, wrCount);
    stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
}
/**
 * @tc.name: EbpfCallstackTableTest
 * @tc.desc: Ebpf callstack table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, EbpfCallstackTableTest, TestSize.Level1)
{
    TS_LOGI("test31-11");
    std::string sqlSelect = "select * from ebpf_callstack";
    std::string sqlSelect1 = "select * from ebpf_callstack where id = 1";
    std::string sqlSelect2 = "select * from ebpf_callstack where id > 1";
    std::string sqlSelect3 = "select * from ebpf_callstack where id < 1";
    std::string sqlSelect4 = "select * from ebpf_callstack where id >= 1";
    std::string sqlSelect5 = "select * from ebpf_callstack where id <= 1";
    std::string sqlSelect6 = "select * from hidump";
    uint64_t callChainId = 1;
    uint32_t depth = 1;
    uint64_t ip = 1;
    uint64_t symbolId = 1;
    uint64_t filePathId = 1;

    uint64_t callChainId1 = 2;
    uint32_t depth1 = 2;
    uint64_t ip1 = 2;
    uint64_t symbolId1 = 2;
    uint64_t filePathId1 = 2;

    uint64_t timeStamp = 1663869124160;
    uint32_t fps = 1;

    uint64_t timestamp1 = 1663869224160;
    uint32_t fps1 = 2;

    stream_.traceDataCache_->GetHidumpData()->AppendNewHidumpInfo(timeStamp, fps);
    stream_.traceDataCache_->GetHidumpData()->AppendNewHidumpInfo(timestamp1, fps1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect6.c_str(), false);
    EXPECT_EQ(row, 2);

    stream_.traceDataCache_->GetEbpfCallStack()->AppendNewData(callChainId, depth, ip, symbolId, filePathId);
    stream_.traceDataCache_->GetEbpfCallStack()->AppendNewData(callChainId1, depth1, ip1, symbolId1, filePathId1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: FileSystemSampleTableTest
 * @tc.desc: File system sample table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, FileSystemSampleTableTest, TestSize.Level1)
{
    TS_LOGI("test31-12");
    std::string sqlSelect = "select * from file_system_sample";
    std::string sqlSelect1 = "select * from file_system_sample where id = 1";
    std::string sqlSelect2 = "select * from file_system_sample where type > 1";
    std::string sqlSelect3 = "select * from hidump";
    std::string sqlSelect4 = "select * from hidump where id = 1";
    std::string sqlSelect5 = "select * from hidump where ts < 1663869124160";
    // std::string sqlSelect6 = "select * from hidump";
    uint64_t callChainId = 1;
    uint16_t type = 1;
    uint32_t ipid = 1;
    uint32_t itid = 1;
    uint64_t startTs = 1663869124160;
    uint64_t endTs = 1663869124260;
    uint64_t dur = 100;
    DataIndex returnValue = stream_.traceDataCache_->GetDataIndex("returnValue");
    DataIndex errorCode = stream_.traceDataCache_->GetDataIndex("errorCode");
    size_t size = 1;
    int32_t fd = 0;
    DataIndex fileId = stream_.traceDataCache_->GetDataIndex("fileId");
    DataIndex firstArgument = stream_.traceDataCache_->GetDataIndex("firstArgument");
    DataIndex secondArgument = stream_.traceDataCache_->GetDataIndex("secondArgument");
    DataIndex thirdArgument = stream_.traceDataCache_->GetDataIndex("thirdArgument");
    DataIndex fourthArgument = stream_.traceDataCache_->GetDataIndex("fourthArgument");

    uint64_t callChainId1 = 2;
    uint16_t type1 = 2;
    uint32_t ipid1 = 2;
    uint32_t itid1 = 2;
    uint64_t startTs1 = 1663869124161;
    uint64_t endTs1 = 1663869124261;
    uint64_t dur1 = 200;
    DataIndex returnValue1 = stream_.traceDataCache_->GetDataIndex("returnValue1");
    DataIndex errorCode1 = stream_.traceDataCache_->GetDataIndex("errorCode1");
    size_t size1 = 2;
    int32_t fd1 = 1;
    DataIndex fileId1 = stream_.traceDataCache_->GetDataIndex("fileId1");
    DataIndex firstArgument1 = stream_.traceDataCache_->GetDataIndex("firstArgument1");
    DataIndex secondArgument1 = stream_.traceDataCache_->GetDataIndex("secondArgument1");
    DataIndex thirdArgument1 = stream_.traceDataCache_->GetDataIndex("thirdArgument1");
    DataIndex fourthArgument1 = stream_.traceDataCache_->GetDataIndex("fourthArgument1");

    uint64_t timeStamp = 1663869124160;
    uint32_t fps = 1;

    uint64_t timestamp1 = 1663869224160;
    uint32_t fps1 = 2;

    stream_.traceDataCache_->GetHidumpData()->AppendNewHidumpInfo(timeStamp, fps);
    stream_.traceDataCache_->GetHidumpData()->AppendNewHidumpInfo(timestamp1, fps1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 0);

    stream_.traceDataCache_->GetFileSystemSample()->AppendNewData(
        callChainId, type, ipid, itid, startTs, endTs, dur, returnValue, errorCode, size, fd, fileId, firstArgument,
        secondArgument, thirdArgument, fourthArgument);
    stream_.traceDataCache_->GetFileSystemSample()->AppendNewData(
        callChainId1, type1, ipid1, itid1, startTs1, endTs1, dur1, returnValue1, errorCode1, size1, fd1, fileId1,
        firstArgument1, secondArgument1, thirdArgument1, fourthArgument1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: HidumpTableTest
 * @tc.desc: Hidump table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, HidumpTableTest, TestSize.Level1)
{
    TS_LOGI("test31-13");
    std::string sqlSelect = "select * from hidump";
    uint64_t timeStamp = 1663869124160;
    uint32_t fps = 1;

    stream_.traceDataCache_->GetHidumpData()->AppendNewHidumpInfo(timeStamp, fps);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: HisysEventMeasureTableTest
 * @tc.desc: Hisys event measure table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, HisysEventMeasureTableTest, TestSize.Level1)
{
    TS_LOGI("test31-14");
    std::string sqlSelect = "select * from hisys_event_measure";
    uint64_t serial = 1;
    uint64_t ts = 1663869124160;
    uint32_t nameId = stream_.traceDataCache_->GetDataIndex("event");
    uint32_t keyId = stream_.traceDataCache_->GetDataIndex("data");
    int32_t type = 1;
    double numericValue = 0;
    DataIndex stringValue = stream_.traceDataCache_->GetDataIndex("stringValue");

    stream_.traceDataCache_->GetSyseventMeasureData()->AppendData(ts, nameId, keyId, type, numericValue, stringValue,
                                                                  serial);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: InstantTableTest
 * @tc.desc: Instant table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, InstantTableTest, TestSize.Level1)
{
    TS_LOGI("test31-15");
    std::string sqlSelect = "select * from instant";
    std::string sqlSelect2 = "select * from instant where name = \"name\"";
    std::string sqlSelect3 = "select * from instant where ts < 1663869124160";
    std::string sqlSelect4 = "select * from instant where ref > 1";
    std::string sqlSelect5 = "select * from instant where wakeup_from >= 1";
    uint64_t timeStamp = 1663869124160;
    DataIndex nameIndex = stream_.traceDataCache_->GetDataIndex("name");
    int64_t internalTid = 1;
    int64_t wakeupFromInternalPid = 1;

    DataIndex nameIndex1 = stream_.traceDataCache_->GetDataIndex("name1");
    int64_t internalTid1 = 2;
    int64_t wakeupFromInternalPid1 = 2;

    std::string sqlSelect1 = "select * from measure";
    uint32_t type = 1;
    int64_t value = 1;
    uint32_t filterId = 1;

    uint32_t type1 = 2;
    uint64_t timestamp1 = 1663869124160;
    int64_t value1 = 2;
    uint32_t filterId1 = 2;

    stream_.traceDataCache_->GetMeasureData()->AppendMeasureData(type, timeStamp, value, filterId);
    stream_.traceDataCache_->GetMeasureData()->AppendMeasureData(type1, timestamp1, value1, filterId1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 2);

    stream_.traceDataCache_->GetInstantsData()->AppendInstantEventData(timeStamp, nameIndex, internalTid,
                                                                       wakeupFromInternalPid);
    stream_.traceDataCache_->GetInstantsData()->AppendInstantEventData(timestamp1, nameIndex1, internalTid1,
                                                                       wakeupFromInternalPid1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: IoLatencySampleTableTest
 * @tc.desc: Io latency sample table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, IoLatencySampleTableTest, TestSize.Level1)
{
    TS_LOGI("test31-16");
    std::string sqlSelect = "select * from bio_latency_sample";
    std::string sqlSelect1 = "select * from hidump";
    std::string sqlSelect2 = "select * from bio_latency_sample where id = 1";
    uint64_t callChainId = 1;
    uint64_t type = 1;
    uint64_t ipid = 1;
    uint64_t itid = 1;
    uint64_t startTs = 1663869124160;
    uint64_t endTs = 1663869224160;
    uint64_t latencyDur = 200;
    uint32_t tier = 1;
    uint64_t size = 1;
    uint64_t blockNumber = 1;
    uint64_t filePathId = stream_.traceDataCache_->GetDataIndex("filePathId");
    uint64_t durPer4k = 1;

    uint64_t callChainId1 = 2;
    uint64_t type1 = 2;
    uint64_t ipid1 = 2;
    uint64_t itid1 = 2;
    uint64_t startTs1 = 1663869224160;
    uint64_t endTs1 = 1663869424160;
    uint64_t latencyDur1 = 200;
    uint32_t tier1 = 2;
    uint64_t size1 = 2;
    uint64_t blockNumber1 = 2;
    uint64_t filePathId1 = stream_.traceDataCache_->GetDataIndex("filePathId1");
    uint64_t durPer4k1 = 2;

    uint64_t timeStamp = 1663869124160;
    uint32_t fps = 1;

    uint64_t timestamp1 = 1663869224160;
    uint32_t fps1 = 2;

    stream_.traceDataCache_->GetHidumpData()->AppendNewHidumpInfo(timeStamp, fps);
    stream_.traceDataCache_->GetHidumpData()->AppendNewHidumpInfo(timestamp1, fps1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 2);

    stream_.traceDataCache_->GetBioLatencySampleData()->AppendNewData(
        callChainId, type, ipid, itid, startTs, endTs, latencyDur, tier, size, blockNumber, filePathId, durPer4k);
    stream_.traceDataCache_->GetBioLatencySampleData()->AppendNewData(callChainId1, type1, ipid1, itid1, startTs1,
                                                                      endTs1, latencyDur1, tier1, size1, blockNumber1,
                                                                      filePathId1, durPer4k1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: IrqTableTest
 * @tc.desc: Irq table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, IrqTableTest, TestSize.Level1)
{
    TS_LOGI("test31-17");
    std::string sqlSelect = "select * from irq";
    std::string sqlSelect1 = "select * from irq where id < 2";
    uint64_t startT = 1663869124160;
    uint64_t durationNs = 200;
    InternalTid internalTid = 1;
    DataIndex cat = stream_.traceDataCache_->GetDataIndex("cat");
    uint16_t nameIdentify = 1;
    DataIndex name = stream_.traceDataCache_->GetDataIndex("name");
    uint8_t depth = 1;
    const std::optional<uint64_t>& parentId = 1;

    uint64_t startT1 = 1663869224160;
    uint64_t durationNs1 = 200;
    InternalTid internalTid1 = 2;
    DataIndex cat1 = stream_.traceDataCache_->GetDataIndex("cat1");
    uint16_t nameIdentify1 = 2;
    DataIndex name1 = stream_.traceDataCache_->GetDataIndex("name1");
    uint8_t depth1 = 2;
    const std::optional<uint64_t>& parentId1 = 2;

    stream_.traceDataCache_->GetIrqData()->AppendInternalSlice(startT, durationNs, internalTid, cat, nameIdentify, name,
                                                               depth, parentId);
    stream_.traceDataCache_->GetIrqData()->AppendInternalSlice(startT1, durationNs1, internalTid1, cat1, nameIdentify1,
                                                               name1, depth1, parentId1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: LiveProcessTableTest
 * @tc.desc: Live Process table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, LiveProcessTableTest, TestSize.Level1)
{
    TS_LOGI("test31-18");
    std::string sqlSelect = "select * from live_process";
    uint64_t newTimeStamp = 1663869124160;
    uint64_t dur = 200;
    int32_t processID = 1;
    std::string processName = "processName";
    int32_t parentProcessID = 1;
    int32_t uid = 1;
    std::string userName = "userName";
    double cpuUsage = 1;
    int32_t pssInfo = 1;
    uint64_t cpuTime = 1663888624160;
    int32_t threads = 1;
    int64_t diskWrites = 1;
    int64_t diskReads = 1;

    stream_.traceDataCache_->GetLiveProcessData()->AppendNewData(newTimeStamp, dur, processID, processName,
                                                                 parentProcessID, uid, userName, cpuUsage, pssInfo,
                                                                 cpuTime, threads, diskWrites, diskReads);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: LogTableTest
 * @tc.desc: Log table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, LogTableTest, TestSize.Level1)
{
    TS_LOGI("test31-19");
    std::string sqlSelect = "select * from log";
    uint64_t seq = 1;
    uint64_t timeStamp = 1663869124160;
    uint32_t pid = 1;
    uint32_t tid = 1;
    DataIndex level = stream_.traceDataCache_->GetDataIndex("leve");
    DataIndex tag = stream_.traceDataCache_->GetDataIndex("tag");
    DataIndex context = stream_.traceDataCache_->GetDataIndex("context");
    uint64_t originTs = 1;

    stream_.traceDataCache_->GetHilogData()->AppendNewLogInfo(seq, timeStamp, pid, tid, level, tag, context, originTs);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: MeasureTableTest
 * @tc.desc: Measure table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, MeasureTableTest, TestSize.Level1)
{
    TS_LOGI("test31-20");
    std::string sqlSelect = "select * from measure";
    std::string sqlSelect1 = "select * from measure where ts = 1663869124160";
    uint32_t type = 1;
    uint64_t timeStamp = 1663869124160;
    int64_t value = 1;
    uint32_t filterId = 1;

    uint32_t type1 = 2;
    uint64_t timestamp1 = 1663869224160;
    int64_t value1 = 2;
    uint32_t filterId1 = 2;

    stream_.traceDataCache_->GetMeasureData()->AppendMeasureData(type, timeStamp, value, filterId);
    stream_.traceDataCache_->GetMeasureData()->AppendMeasureData(type1, timestamp1, value1, filterId1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: MeasureFilterTableTest
 * @tc.desc: Measure Filter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, MeasureFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-21");
    std::string sqlSelect = "select * from measure_filter";
    uint64_t filterId = stream_.traceDataCache_->GetDataIndex("filter");
    ;
    uint32_t nameIndex = stream_.traceDataCache_->GetDataIndex("name");
    uint64_t internalTid = 1;

    stream_.traceDataCache_->GetThreadMeasureFilterData()->AppendNewFilter(filterId, nameIndex, internalTid);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: MetaTableTest
 * @tc.desc: Meta table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, MetaTableTest, TestSize.Level1)
{
    TS_LOGI("test31-22");
    std::string sqlSelect = "select * from meta";

    stream_.traceDataCache_->GetMetaData();
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 9);
}
/**
 * @tc.name: NativeHookTableTest
 * @tc.desc: Native hook table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, NativeHookTableTest, TestSize.Level1)
{
    TS_LOGI("test31-23");
    std::string sqlSelect = "select * from native_hook";
    std::string sqlSelect1 = "select * from native_hook where id = 1";
    std::string sqlSelect2 = "select * from native_hook where ipid > 1";
    std::string sqlSelect3 = "select * from native_hook where itid >= 1";
    std::string sqlSelect4 = "select * from native_hook where callchain_id < 1";
    uint64_t callChainId = 1;
    uint32_t ipid = 1;
    uint32_t itid = 1;
    std::string eventType = "eventType";
    DataIndex subType = stream_.traceDataCache_->GetDataIndex("subType");
    uint64_t timeStamp = 1663869124160;
    uint64_t endTimestamp = 1663869124360;
    uint64_t duration = 200;
    uint64_t addr = 1;
    int64_t memSize = 1;
    int64_t curMemSize = 1;

    uint64_t callChainId1 = 2;
    uint32_t ipid1 = 2;
    uint32_t itid1 = 2;
    std::string eventType1 = "eventType1";
    DataIndex subType1 = stream_.traceDataCache_->GetDataIndex("subType1");
    uint64_t timestamp1 = 1663869224160;
    uint64_t endTimestamp1 = 1663869224360;
    uint64_t duration1 = 200;
    uint64_t addr1 = 2;
    int64_t memSize1 = 2;
    int64_t curMemSize1 = 2;

    stream_.traceDataCache_->GetNativeHookData()->AppendNewNativeHookData(
        callChainId, ipid, itid, eventType, subType, timeStamp, endTimestamp, duration, addr, memSize);
    stream_.traceDataCache_->GetNativeHookData()->AppendNewNativeHookData(
        callChainId1, ipid1, itid1, eventType1, subType1, timestamp1, endTimestamp1, duration1, addr1, memSize1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: NativeHookFrameTableTest
 * @tc.desc: Native hook Frame table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, NativeHookFrameTableTest, TestSize.Level1)
{
    TS_LOGI("test31-24");
    std::string sqlSelect = "select * from native_hook_frame";
    std::string sqlSelect1 = "select * from native_hook_frame where id = 1";
    std::string sqlSelect2 = "select * from native_hook_frame where callchain_id > 1";
    std::string sqlSelect3 = "select * from native_hook_frame where symbol_id >= 1";
    std::string sqlSelect4 = "select * from native_hook_frame where file_id < 2";
    uint64_t callChainId = 1;
    uint64_t depth = 1;
    uint64_t ip = 1;
    uint64_t sp = 1;
    DataIndex symbolName = stream_.traceDataCache_->GetDataIndex("symbolName");
    DataIndex filePath = stream_.traceDataCache_->GetDataIndex("filePath");
    uint64_t offset = 1;
    uint64_t symbolOffset = 1;
    const std::string vaddr = "addr";

    uint64_t callChainId1 = 2;
    uint64_t depth1 = 2;
    uint64_t ip1 = 2;
    uint64_t sp1 = 2;
    DataIndex symbolName1 = stream_.traceDataCache_->GetDataIndex("symbolName1");
    DataIndex filePath1 = stream_.traceDataCache_->GetDataIndex("filePath1");
    uint64_t offset1 = 2;
    uint64_t symbolOffset1 = 2;
    const std::string vaddr1 = "addr1";

    stream_.traceDataCache_->GetNativeHookFrameData()->AppendNewNativeHookFrame(callChainId, depth, ip, sp, symbolName,
                                                                                filePath, offset, symbolOffset, vaddr);
    stream_.traceDataCache_->GetNativeHookFrameData()->AppendNewNativeHookFrame(
        callChainId1, depth1, ip1, sp1, symbolName1, filePath1, offset1, symbolOffset1, vaddr1);

    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: NetworkTableTest
 * @tc.desc: Network table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, NetworkTableTest, TestSize.Level1)
{
    TS_LOGI("test31-25");
    std::string sqlSelect = "select * from network";
    uint64_t newTimeStamp = 1663869124160;
    uint64_t tx = 1;
    uint64_t rx = 1;
    uint64_t dur = 200;
    double rxSpeed = 1;
    double txSpeed = 1;
    uint64_t packetIn = 1;
    double packetInSec = 1;
    uint64_t packetOut = 1;
    double packetOutSec = 1;
    const std::string& netType = "nettype";

    stream_.traceDataCache_->GetNetworkData()->AppendNewNetData(newTimeStamp, tx, rx, dur, rxSpeed, txSpeed, packetIn,
                                                                packetInSec, packetOut, packetOutSec, netType);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: PerfCallchainTableTest
 * @tc.desc: Perf callchain table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, PerfCallchainTableTest, TestSize.Level1)
{
    TS_LOGI("test31-26");
    std::string sqlSelect = "select * from perf_callchain";
    std::string sqlSelect1 = "select * from perf_callchain where id = 1";
    std::string sqlSelect2 = "select * from perf_callchain where callchain_id > 1";
    std::string sqlSelect3 = "select * from perf_callchain where file_id < 1";
    std::string sqlSelect4 = "select * from perf_callchain where symbol_id >= 1";
    uint64_t sampleId = stream_.traceDataCache_->GetDataIndex("sample");
    uint64_t callChainId = stream_.traceDataCache_->GetDataIndex("callChain");
    uint64_t vaddrInFile = 1;
    uint64_t fileId = stream_.traceDataCache_->GetDataIndex("file");
    uint64_t symbolId = stream_.traceDataCache_->GetDataIndex("symbolId");

    uint64_t sampleId1 = stream_.traceDataCache_->GetDataIndex("sample1");
    uint64_t callChainId1 = stream_.traceDataCache_->GetDataIndex("callChain1");
    uint64_t vaddrInFile1 = 2;
    uint64_t fileId1 = stream_.traceDataCache_->GetDataIndex("file1");
    uint64_t symbolId1 = stream_.traceDataCache_->GetDataIndex("symbolId1");

    stream_.traceDataCache_->GetPerfCallChainData()->AppendNewPerfCallChain(sampleId, callChainId, vaddrInFile, fileId,
                                                                            symbolId);

    stream_.traceDataCache_->GetPerfCallChainData()->AppendNewPerfCallChain(sampleId1, callChainId1, vaddrInFile1,
                                                                            fileId1, symbolId1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: PerfFilesTableTest
 * @tc.desc: Perf files table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, PerfFilesTableTest, TestSize.Level1)
{
    TS_LOGI("test31-27");
    std::string sqlSelect = "select * from perf_files";
    std::string sqlSelect1 = "select * from perf_files where id = 1";
    std::string sqlSelect2 = "select * from perf_files where file_id < 1";
    uint64_t fileIds = stream_.traceDataCache_->GetDataIndex("file");
    uint32_t serial = 1;
    DataIndex symbols = stream_.traceDataCache_->GetDataIndex("symbol");
    DataIndex filePath = stream_.traceDataCache_->GetDataIndex("filePath");

    uint64_t fileIds1 = stream_.traceDataCache_->GetDataIndex("file1");
    uint32_t serial1 = 1;
    DataIndex symbols1 = stream_.traceDataCache_->GetDataIndex("symbol1");
    DataIndex filePath1 = stream_.traceDataCache_->GetDataIndex("filePath1");

    stream_.traceDataCache_->GetPerfFilesData()->AppendNewPerfFiles(fileIds, serial, symbols, filePath);
    stream_.traceDataCache_->GetPerfFilesData()->AppendNewPerfFiles(fileIds1, serial1, symbols1, filePath1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: PerfReportTableTest
 * @tc.desc: Perf report table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, PerfReportTableTest, TestSize.Level1)
{
    TS_LOGI("test31-28");
    std::string sqlSelect = "select * from perf_report";
    DataIndex type = stream_.traceDataCache_->GetDataIndex("type");
    DataIndex value = stream_.traceDataCache_->GetDataIndex("value");

    stream_.traceDataCache_->GetPerfReportData()->AppendNewPerfReport(type, value);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: PerfSampleTableTest
 * @tc.desc: Perf sample table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, PerfSampleTableTest, TestSize.Level1)
{
    TS_LOGI("test31-29");
    std::string sqlSelect = "select * from perf_sample";
    std::string sqlSelect1 = "select * from perf_sample where id = 1";
    std::string sqlSelect2 = "select * from perf_sample where callchain_id > 1";
    std::string sqlSelect3 = "select * from perf_sample where thread_id < 1";
    std::string sqlSelect4 = "select * from perf_sample where event_type_id >= 1";
    std::string sqlSelect5 = "select * from perf_sample where cpu_id <= 1";
    uint64_t sampleId = stream_.traceDataCache_->GetDataIndex("type");
    uint64_t timeStamp = 1663869124160;
    uint64_t tid = 1;
    uint64_t eventCount = 2;
    uint64_t eventTypeId = 1;
    uint64_t timestampTrace = 1;
    uint64_t cpuId = 1;
    uint64_t threadState = stream_.traceDataCache_->GetDataIndex("threadState");

    uint64_t sampleId1 = stream_.traceDataCache_->GetDataIndex("type1");
    uint64_t timestamp1 = 1663869124160;
    uint64_t tid1 = 2;
    uint64_t eventCount1 = 3;
    uint64_t eventTypeId1 = 2;
    uint64_t timestampTrace1 = 2;
    uint64_t cpuId1 = 2;
    uint64_t threadState1 = stream_.traceDataCache_->GetDataIndex("threadState1");

    stream_.traceDataCache_->GetPerfSampleData()->AppendNewPerfSample(sampleId, timeStamp, tid, eventCount, eventTypeId,
                                                                      timestampTrace, cpuId, threadState);
    stream_.traceDataCache_->GetPerfSampleData()->AppendNewPerfSample(
        sampleId1, timestamp1, tid1, eventCount1, eventTypeId1, timestampTrace1, cpuId1, threadState1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: PerfThreadTableTest
 * @tc.desc: Perf Thread table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, PerfThreadTableTest, TestSize.Level1)
{
    TS_LOGI("test31-30");
    std::string sqlSelect = "select * from perf_thread";
    std::string sqlSelect1 = "select * from perf_thread where id = 1";
    std::string sqlSelect2 = "select * from perf_thread where thread_id > 1";
    std::string sqlSelect3 = "select * from perf_thread where process_id < 1";
    uint64_t pid = 1;
    uint64_t tid = 1;
    DataIndex threadName = stream_.traceDataCache_->GetDataIndex("threadState");

    uint64_t pid1 = 2;
    uint64_t tid1 = 2;
    DataIndex threadName1 = stream_.traceDataCache_->GetDataIndex("threadState1");

    stream_.traceDataCache_->GetPerfThreadData()->AppendNewPerfThread(pid, tid, threadName);
    stream_.traceDataCache_->GetPerfThreadData()->AppendNewPerfThread(pid1, tid1, threadName1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: ProcessTableTest
 * @tc.desc: Process table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ProcessTableTest, TestSize.Level1)
{
    TS_LOGI("test31-31");
    std::string sqlSelect = "select * from process";
    std::string sqlSelect1 = "select * from process where id = 1";

    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: ProcessFilterTableTest
 * @tc.desc: Process filter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ProcessFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-32");
    std::string sqlSelect = "select * from process_filter";
    uint64_t id = 1;
    DataIndex name = stream_.traceDataCache_->GetDataIndex("name");
    uint32_t internalPid = 1;

    stream_.traceDataCache_->GetProcessFilterData()->AppendNewFilter(id, name, internalPid);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: ProcessMeasureTableTest
 * @tc.desc: Process Measure table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ProcessMeasureTableTest, TestSize.Level1)
{
    TS_LOGI("test31-33");
    std::string sqlSelect = "select * from process_measure";
    uint32_t type = 1;
    uint64_t timeStamp = 1663869124160;
    int64_t value = 1;
    uint32_t filterId = 1;

    stream_.traceDataCache_->GetProcessMeasureData()->AppendMeasureData(type, timeStamp, value, filterId);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: ProcessMeasureFilterTableTest
 * @tc.desc: Process Measure filter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ProcessMeasureFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-34");
    std::string sqlSelect = "select * from process_measure_filter";
    std::string sqlSelect1 = "select * from process_measure_filter where id = 1";
    std::string sqlSelect2 = "select * from process_measure_filter where ipid < 1";
    std::string sqlSelect3 = "select * from process_measure_filter where name = \"name\"";
    uint64_t id = 1;
    DataIndex name = stream_.traceDataCache_->GetDataIndex("name");
    uint32_t internalPid = 1;

    uint64_t id1 = 1;
    DataIndex name1 = stream_.traceDataCache_->GetDataIndex("name1");
    uint32_t internalPid1 = 1;

    stream_.traceDataCache_->GetProcessMeasureFilterData()->AppendNewFilter(id, name, internalPid);
    stream_.traceDataCache_->GetProcessMeasureFilterData()->AppendNewFilter(id1, name1, internalPid1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: RawTableTest
 * @tc.desc: Raw table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, RawTableTest, TestSize.Level1)
{
    TS_LOGI("test31-35");
    std::string sqlSelect = "select * from raw";
    std::string sqlSelect1 = "select * from raw where id = 1";
    std::string sqlSelect2 = "select * from raw where name = \"sched_waking\"";
    std::string sqlSelect3 = "select * from raw where ts = 1663869124160";
    std::string sqlSelect4 = "select * from raw where itid < 2";
    uint32_t id = 1;
    uint64_t timeStamp = 1663869124160;
    uint32_t name = stream_.traceDataCache_->GetDataIndex("cpu_idle");
    uint32_t cpu = 1;
    uint32_t internalTid = 1;

    uint32_t id1 = 2;
    uint64_t timestamp1 = 1663869224160;
    uint32_t name1 = stream_.traceDataCache_->GetDataIndex("sched_waking");
    uint32_t cpu1 = 2;
    uint32_t internalTid1 = 2;

    stream_.traceDataCache_->GetRawData()->AppendRawData(id, timeStamp, name, cpu, internalTid);
    stream_.traceDataCache_->GetRawData()->AppendRawData(id1, timestamp1, name1, cpu1, internalTid1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: SchedSliceTest
 * @tc.desc: Sched slice table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, SchedSliceTest, TestSize.Level1)
{
    TS_LOGI("test31-36");
    std::string sqlSelect = "select * from sched_slice";
    std::string sqlSelect1 = "select * from sched_slice where id = 1";
    std::string sqlSelect2 = "select * from sched_slice where ts > 1";
    std::string sqlSelect3 = "select * from sched_slice where cpu < 1";
    std::string sqlSelect4 = "select * from sched_slice where itid >= 1";
    std::string sqlSelect5 = "select * from sched_slice where ipid <= 1";
    std::string sqlSelect6 = "select * from sched_slice where dur >= 200";
    uint64_t ts = 1663869124160;
    uint64_t dur = 200;
    uint64_t cpu = 1;
    uint64_t internalTid = 1;
    uint64_t endState = 1;
    uint64_t priority = 1;

    uint64_t ts1 = 1663869224160;
    uint64_t dur1 = 200;
    uint64_t cpu1 = 2;
    uint64_t internalTid1 = 2;
    uint64_t endState1 = 2;
    uint64_t priority1 = 2;

    stream_.traceDataCache_->GetSchedSliceData()->AppendSchedSlice(ts, dur, cpu, internalTid, endState, priority);
    stream_.traceDataCache_->GetSchedSliceData()->AppendSchedSlice(ts1, dur1, cpu1, internalTid1, endState1, priority1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect6.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: SmapsTest
 * @tc.desc: Smaps table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, SmapsTest, TestSize.Level1)
{
    TS_LOGI("test31-37");
    std::string sqlSelect = "select * from smaps";
    uint64_t timeStamp = 1663869124160;
    std::string startAddr = "startAddr";
    std::string endAddr = "endAddr";
    uint64_t dirty = 1;
    uint64_t swapper = 1;
    uint64_t rss = 1;
    uint64_t pss = 1;
    uint64_t size = 2;
    double reside = 1;
    DataIndex protectionId = stream_.traceDataCache_->GetDataIndex("protection");
    DataIndex pathId = stream_.traceDataCache_->GetDataIndex("path");

    stream_.traceDataCache_->GetSmapsData()->AppendNewData(timeStamp, startAddr, endAddr, dirty, swapper, rss, pss,
                                                           size, reside, protectionId, pathId);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: StatTableTest
 * @tc.desc: Stat table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, StatTableTest, TestSize.Level1)
{
    TS_LOGI("test31-38");
    std::string sqlSelect = "select * from stat";
    stream_.traceDataCache_->GetStatAndInfo();
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 430);
}
/**
 * @tc.name: SymbolsTableTest
 * @tc.desc: Symbols table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, SymbolsTableTest, TestSize.Level1)
{
    TS_LOGI("test31-39");
    std::string sqlSelect = "select * from symbols";
    std::string sqlSelect1 = "select * from symbols where id = 1";
    std::string sqlSelect2 = "select * from symbols where id < 1";
    const DataIndex& name = stream_.traceDataCache_->GetDataIndex("name");
    const uint64_t& addr = 1;

    const DataIndex& name1 = stream_.traceDataCache_->GetDataIndex("name1");
    const uint64_t& addr1 = 2;

    stream_.traceDataCache_->GetSymbolsData()->InsertSymbol(name, addr);
    stream_.traceDataCache_->GetSymbolsData()->InsertSymbol(name1, addr1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: SyscallTableTest
 * @tc.desc: Syscall table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, SyscallTableTest, TestSize.Level1)
{
    TS_LOGI("test31-40");
    std::string sqlSelect = "select * from syscall";
    int64_t sysCallNum = 1;
    DataIndex type = stream_.traceDataCache_->GetDataIndex("type");
    uint64_t ipid = 1;
    uint64_t timeStamp = 1663869124160;
    int64_t ret = 1;

    stream_.traceDataCache_->GetSysCallData()->AppendSysCallData(sysCallNum, type, ipid, timeStamp, ret);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: SysEventFilterTableTest
 * @tc.desc: SysEventFilter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, SysEventFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-41");
    std::string sqlSelect = "select * from sys_event_filter";
    std::string sqlSelect1 = "select * from sys_event_filter where id = 1";
    std::string sqlSelect2 = "select * from sys_event_filter where id > 1";
    std::string sqlSelect3 = "select * from sys_event_filter where id < 1";
    std::string sqlSelect4 = "select * from sys_event_filter where id >= 1";
    std::string sqlSelect5 = "select * from sys_event_filter where id <= 1";
    uint64_t filterId = 1;
    DataIndex type = stream_.traceDataCache_->GetDataIndex("type");
    DataIndex nameId = stream_.traceDataCache_->GetDataIndex("name");

    uint64_t filterId1 = 2;
    DataIndex type1 = stream_.traceDataCache_->GetDataIndex("type1");
    DataIndex nameId1 = stream_.traceDataCache_->GetDataIndex("name1");

    stream_.traceDataCache_->GetSysMeasureFilterData()->AppendNewFilter(filterId, type, nameId);
    stream_.traceDataCache_->GetSysMeasureFilterData()->AppendNewFilter(filterId1, type1, nameId1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 2);
}
/**
 * @tc.name: SysMemMeasureTableTest
 * @tc.desc: SysMemMeasure table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, SysMemMeasureTableTest, TestSize.Level1)
{
    TS_LOGI("test31-42");
    std::string sqlSelect = "select * from sys_mem_measure";
    uint32_t type = 1;
    uint64_t timeStamp = 1663869124160;
    int64_t value = 1;
    uint32_t filterId = stream_.traceDataCache_->GetDataIndex("filter");

    stream_.traceDataCache_->GetSysMemMeasureData()->AppendMeasureData(type, timeStamp, value, filterId);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: ThreadTableTest
 * @tc.desc: Thread table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ThreadTableTest, TestSize.Level1)
{
    TS_LOGI("test31-43");
    std::string sqlSelect = "select * from thread";

    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: ThreadFilterTableTest
 * @tc.desc: ThreadFilter table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ThreadFilterTableTest, TestSize.Level1)
{
    TS_LOGI("test31-44");
    std::string sqlSelect = "select * from thread_filter";
    uint64_t filterId = stream_.traceDataCache_->GetDataIndex("ilter");
    uint32_t nameIndex = stream_.traceDataCache_->GetDataIndex("name");
    uint64_t internalTid = 1;

    stream_.traceDataCache_->GetThreadFilterData()->AppendNewFilter(filterId, nameIndex, internalTid);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
/**
 * @tc.name: ThreadStateTableTest
 * @tc.desc: ThreadState table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, ThreadStateTableTest, TestSize.Level1)
{
    TS_LOGI("test31-45");
    std::string sqlSelect = "select * from thread_state";
    std::string sqlSelect1 = "select * from thread_state where id = 1";
    std::string sqlSelect2 = "select * from thread_state where tid > 1";
    std::string sqlSelect3 = "select * from thread_state where pid < 1";
    std::string sqlSelect4 = "select * from thread_state where itid >= 1";
    std::string sqlSelect5 = "select * from thread_state where cpu <= 1";
    std::string sqlSelect6 = "select * from thread_state where ts = 1663869124160";
    std::string sqlSelect7 = "select * from thread_state where dur = 1";
    std::string sqlSelect8 = "select * from thread_state where state = \"idState\"";
    InternalTime ts = 1663869124160;
    InternalTime dur = 200;
    InternalCpu cpu = 1;
    InternalTid itid = 1;
    TableRowId idState = 1;

    InternalTime ts1 = 1663869224160;
    InternalTime dur1 = 200;
    InternalCpu cpu1 = 2;
    InternalTid itid1 = 2;
    TableRowId idState1 = 2;

    stream_.traceDataCache_->GetThreadStateData()->AppendThreadState(ts, dur, cpu, itid, idState);
    stream_.traceDataCache_->GetThreadStateData()->AppendThreadState(ts1, dur1, cpu1, itid1, idState1);
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect1.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect2.c_str(), false);
    EXPECT_EQ(row, 2);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect3.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect4.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect5.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect6.c_str(), false);
    EXPECT_EQ(row, 1);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect7.c_str(), false);
    EXPECT_EQ(row, 0);
    row = stream_.traceDataCache_->SearchDatabase(sqlSelect8.c_str(), false);
    EXPECT_EQ(row, 0);
}
/**
 * @tc.name: TraceRangeTableTest
 * @tc.desc: TraceRange table test
 * @tc.type: FUNC
 */
HWTEST_F(TableTest, TraceRangeTableTest, TestSize.Level1)
{
    TS_LOGI("test31-46");
    std::string sqlSelect = "select * from trace_range";

    stream_.traceDataCache_->UpdateTraceRange();
    auto row = stream_.traceDataCache_->SearchDatabase(sqlSelect.c_str(), false);
    EXPECT_EQ(row, 1);
}
} // namespace TraceStreamer
} // namespace SysTuning
