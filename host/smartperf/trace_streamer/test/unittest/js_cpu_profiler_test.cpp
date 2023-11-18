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
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "file.h"
#include "htrace_js_memory_parser.h"
#include "htrace_js_cpu_profiler_parser.h"
#include "js_heap_config.pb.h"
#include "js_heap_config.pbreader.h"
#include "js_heap_result.pb.h"
#include "js_heap_result.pbreader.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class JsCpuProfilerTest : public ::testing::Test {
public:
    void SetUp()
    {
        stream_.InitFilter();
    }

    void TearDown() {}

public:
    SysTuning::TraceStreamer::TraceStreamerSelector stream_ = {};
};
/**
 * @tc.name: cpuProfilerParserNodesbyArkTs
 * @tc.desc: cpuProfiler parser nodes
 * @tc.type: FUNC
 */
HWTEST_F(JsCpuProfilerTest, cpuProfilerParserNodesbyArkTs, TestSize.Level1)
{
    TS_LOGI("test39-1");
    const int32_t pid = 1734;
    ArkTSConfig arkTSConfig;
    arkTSConfig.set_pid(pid);
    arkTSConfig.set_type(::ArkTSConfig_HeapType(ArkTSConfig_HeapType_INVALID));
    arkTSConfig.set_enable_cpu_profiler(0);
    arkTSConfig.set_cpu_profiler_interval(1000);
    std::string strConfig = "";
    arkTSConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);
    const char* result1 = "{\"id\":3,\"result\":{}}";
    const char* result2 =
        "{\"id\":3,\"result\":{\"profile\":{\"tid\":1394,\"startTime\":8013755173,\"endTime\":8034596087,\"gcTime\":0,"
        "\"cInterpreterTime\":0,\"asmInterpreterTime\":0,\"aotTime\":0,\"builtinTime\":0,\"napiTime\":0,"
        "\"arkuiEngineTime\":0,\"runtimeTime\":0,\"otherTime\":20840914,\"nodes\":[{\"id\":1,\"callFrame\":{"
        "\"functionName\":\"(root)\",\"scriptId\":\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":-1},"
        "\"hitCount\":0,\"children\":[2,3,4]},{\"id\":2,\"callFrame\":{\"functionName\":\"(program)\",\"scriptId\":"
        "\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":-1},\"hitCount\":113097,\"children\":[]},{\"id\":3,"
        "\"callFrame\":{\"functionName\":\"(idle)\",\"scriptId\":\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":"
        "-1},\"hitCount\":0,\"children\":[]},{\"id\":4,\"callFrame\":{\"functionName\":\"anonymous\",\"scriptId\":"
        "\"1\",\"url\":\"D:/workspace_ohos_system/Music/entry/build/default/intermediates/assets_jsbundle/default/ets/"
        "pages/"
        "index_.js\",\"lineNumber\":2922,\"columnNumber\":36},\"hitCount\":1,\"children\":[]}],\"samples\":[2,3,4,2,2,"
        "2,2,3,3,3,3,4,4,4],\"timeDeltas\":[99,72,215,637,96,288,36,89,94,54,26,598,784,522]}}}";

    ArkTSResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    ProfilerPluginDataHeader profilerPluginData;
    htraceJSMemoryParser.Parse(tracePacket1, 10000, 0, 0, profilerPluginData);

    ArkTSResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000, 0, 0, profilerPluginData);
    htraceJSMemoryParser.Finish();

    auto size = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().Size();
    EXPECT_EQ(4, size);
    auto scriptId = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().ScriptIds()[0];
    EXPECT_EQ("0", scriptId);
    auto url = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().Urls()[0];
    EXPECT_EQ(stream_.traceDataCache_->GetDataIndex(""), url);
    auto lineNumber = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().LineNumbers()[0];
    EXPECT_EQ(-1, lineNumber);
    auto columnNumber = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().ColumnNumbers()[0];
    EXPECT_EQ(-1, columnNumber);
    auto hitCount = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().HitCounts()[0];
    EXPECT_EQ(0, hitCount);

    auto size1 = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().Size();
    EXPECT_EQ(4, size1);
    auto scriptId1 = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().ScriptIds()[1];
    EXPECT_EQ("0", scriptId1);
    auto url1 = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().Urls()[1];
    EXPECT_EQ(stream_.traceDataCache_->GetDataIndex(""), url1);
    auto lineNumber1 = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().LineNumbers()[1];
    EXPECT_EQ(-1, lineNumber1);
    auto columnNumber1 = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().ColumnNumbers()[1];
    EXPECT_EQ(-1, columnNumber1);
    auto hitCount1 = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().HitCounts()[1];
    EXPECT_EQ(113097, hitCount1);
}

/**
 * @tc.name: cpuProfilerParserSamplesbyArkTs
 * @tc.desc: cpuProfiler parser samples
 * @tc.type: FUNC
 */
HWTEST_F(JsCpuProfilerTest, cpuProfilerParserSamplesbyArkTs, TestSize.Level1)
{
    TS_LOGI("test39-2");
    const int32_t pid = 1734;
    ArkTSConfig arkTSConfig;
    arkTSConfig.set_pid(pid);
    arkTSConfig.set_type(::ArkTSConfig_HeapType(ArkTSConfig_HeapType_INVALID));
    arkTSConfig.set_enable_cpu_profiler(0);
    arkTSConfig.set_cpu_profiler_interval(1000);
    std::string strConfig = "";
    arkTSConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);
    const char* result1 = "{\"id\":3,\"result\":{}}";
    const char* result2 =
        "{\"id\":3,\"result\":{\"profile\":{\"tid\":1394,\"startTime\":8013755173,\"endTime\":8034596087,\"gcTime\":0,"
        "\"cInterpreterTime\":0,\"asmInterpreterTime\":0,\"aotTime\":0,\"builtinTime\":0,\"napiTime\":0,"
        "\"arkuiEngineTime\":0,\"runtimeTime\":0,\"otherTime\":20840914,\"nodes\":[{\"id\":1,\"callFrame\":{"
        "\"functionName\":\"(root)\",\"scriptId\":\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":-1},"
        "\"hitCount\":0,\"children\":[2,3,4]},{\"id\":2,\"callFrame\":{\"functionName\":\"(program)\",\"scriptId\":"
        "\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":-1},\"hitCount\":113097,\"children\":[]},{\"id\":3,"
        "\"callFrame\":{\"functionName\":\"(idle)\",\"scriptId\":\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":"
        "-1},\"hitCount\":0,\"children\":[]},{\"id\":4,\"callFrame\":{\"functionName\":\"anonymous\",\"scriptId\":"
        "\"1\",\"url\":\"D:/workspace_ohos_system/Music/entry/build/default/intermediates/assets_jsbundle/default/ets/"
        "pages/"
        "index_.js\",\"lineNumber\":2922,\"columnNumber\":36},\"hitCount\":1,\"children\":[]}],\"samples\":[2,3,4,2,2,"
        "2,2,3,3,3,3,4,4,4],\"timeDeltas\":[99,72,215,637,96,288,36,89,94,54,26,598,784,522]}}}";

    ArkTSResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    ProfilerPluginDataHeader profilerPluginData;
    htraceJSMemoryParser.Parse(tracePacket1, 10000, 0, 0, profilerPluginData);

    ArkTSResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000, 0, 0, profilerPluginData);
    htraceJSMemoryParser.Finish();

    auto sampleFunctionId1 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().FunctionIds()[0];
    EXPECT_EQ(2, sampleFunctionId1);
    auto startTime1 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().StartTimes()[0];
    EXPECT_EQ(8013755173000, startTime1);
    auto endTime1 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().EndTimes()[0];
    EXPECT_EQ(8013755245000, endTime1);
    auto dur1 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().Durs()[0];
    EXPECT_EQ(72000, dur1);
    auto sampleFunctionId2 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().FunctionIds()[1];
    EXPECT_EQ(3, sampleFunctionId2);
    auto startTime2 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().StartTimes()[1];
    EXPECT_EQ(8013755245000, startTime2);
    auto endTime2 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().EndTimes()[1];
    EXPECT_EQ(8013755460000, endTime2);
    auto dur2 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().Durs()[1];
    EXPECT_EQ(215000, dur2);
    auto sampleFunctionId3 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().FunctionIds()[2];
    EXPECT_EQ(4, sampleFunctionId3);
    auto startTime3 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().StartTimes()[2];
    EXPECT_EQ(8013755460000, startTime3);
    auto endTime3 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().EndTimes()[2];
    EXPECT_EQ(8013756097000, endTime3);
    auto dur3 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().Durs()[2];
    EXPECT_EQ(637000, dur3);
    auto sampleFunctionId4 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().FunctionIds()[3];
    EXPECT_EQ(2, sampleFunctionId4);
    auto startTime4 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().StartTimes()[3];
    EXPECT_EQ(8013756097000, startTime4);
    auto endTime4 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().EndTimes()[3];
    EXPECT_EQ(8013756606000, endTime4);
    auto dur4 = stream_.traceDataCache_->GetConstJsCpuProfilerSampleData().Durs()[3];
    EXPECT_EQ(509000, dur4);
}

/**
 * @tc.name: cpuProfilerParserNoProfilebyArkTs
 * @tc.desc: cpuProfiler parser no profile
 * @tc.type: FUNC
 */
HWTEST_F(JsCpuProfilerTest, cpuProfilerParserNoProfilebyArkTs, TestSize.Level1)
{
    TS_LOGI("test39-3");
    const int32_t pid = 1734;
    ArkTSConfig arkTSConfig;
    arkTSConfig.set_pid(pid);
    arkTSConfig.set_type(::ArkTSConfig_HeapType(ArkTSConfig_HeapType_INVALID));
    arkTSConfig.set_enable_cpu_profiler(0);
    arkTSConfig.set_cpu_profiler_interval(1000);
    std::string strConfig = "";
    arkTSConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);
    const char* result1 = "{\"id\":3,\"result\":{}}";
    const char* result2 =
        "{\"id\":3,\"result\":{\"data\":{\"tid\":1394,\"startTime\":8013755173,\"endTime\":8034596087,\"gcTime\":0,"
        "\"cInterpreterTime\":0,\"asmInterpreterTime\":0,\"aotTime\":0,\"builtinTime\":0,\"napiTime\":0,"
        "\"arkuiEngineTime\":0,\"runtimeTime\":0,\"otherTime\":20840914,\"nodes\":[{\"id\":1,\"callFrame\":{"
        "\"functionName\":\"(root)\",\"scriptId\":\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":-1},"
        "\"hitCount\":0,\"children\":[2,3,4]},{\"id\":2,\"callFrame\":{\"functionName\":\"(program)\",\"scriptId\":"
        "\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":-1},\"hitCount\":113097,\"children\":[]},{\"id\":3,"
        "\"callFrame\":{\"functionName\":\"(idle)\",\"scriptId\":\"0\",\"url\":\"\",\"lineNumber\":-1,\"columnNumber\":"
        "-1},\"hitCount\":0,\"children\":[]},{\"id\":4,\"callFrame\":{\"functionName\":\"anonymous\",\"scriptId\":"
        "\"1\",\"url\":\"D:/workspace_ohos_system/Music/entry/build/default/intermediates/assets_jsbundle/default/ets/"
        "pages/"
        "index_.js\",\"lineNumber\":2922,\"columnNumber\":36},\"hitCount\":1,\"children\":[]}],\"samples\":[2,3,4,2,2,"
        "2,2,3,3,3,3,4,4,4],\"timeDeltas\":[99,72,215,637,96,288,36,89,94,54,26,598,784,522]}}}";

    ArkTSResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    ProfilerPluginDataHeader profilerPluginData;
    htraceJSMemoryParser.Parse(tracePacket1, 10000, 0, 0, profilerPluginData);

    ArkTSResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000, 0, 0, profilerPluginData);
    htraceJSMemoryParser.Finish();

    auto size = stream_.traceDataCache_->GetConstJsCpuProfilerNodeData().Size();
    EXPECT_EQ(0, size);
}
} // namespace TraceStreamer
} // namespace SysTuning
