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
#include "js_heap_config.pb.h"
#include "js_heap_config.pb.cc"
#include "js_heap_config.pbreader.h"
#include "js_heap_result.pb.h"
#include "js_heap_result.pb.cc"
#include "js_heap_result.pbreader.h"
#include "trace_streamer_selector.h"

using namespace testing::ext;
using namespace SysTuning;
using namespace SysTuning::TraceStreamer;
namespace SysTuning {
namespace TraceStreamer {
class JsMemoryTest : public ::testing::Test {
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
 * @tc.name: snapshotParserNodesbyJsmemory
 * @tc.desc: snapshot parser nodes
 * @tc.type: FUNC
 */
HWTEST_F(JsMemoryTest, snapshotParserNodesByJsmemory, TestSize.Level1)
{
    TS_LOGI("test35-1");
    const int32_t pid = 1734;
    JsHeapConfig jsHeapConfig;
    jsHeapConfig.set_pid(pid);
    jsHeapConfig.set_type(::JsHeapConfig_HeapType(0));
    std::string strConfig = "";
    jsHeapConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);

    const char* result1 =
        "{\"method\":\"HeapProfiler.reportHeapSnapshotProgress\",\"params\":{\"done\":0,\"total\":21288}}";
    const char* result2 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"{\\\"snapshot\\\":\\n{\\\"meta\\\":"
        "\\n{\\\"node_fields\\\":[\\\"type\\\",\\\"name\\\",\\\"id\\\",\\\"self_size\\\",\\\"edge_count\\\",\\\"trace_"
        "node_id\\\",\\\"detachedness\\\"],\\n\\\"node_types\\\":[[\\\"hidden\\\",\\\"array\\\",\\\"string\\\","
        "\\\"object\\\",\\\"code\\\",\\\"closure\\\",\\\"regexp\\\",\\\"number\\\",\\\"native\\\",\\\"synthetic\\\","
        "\\\"concatenated "
        "string\\\",\\\"slicedstring\\\",\\\"symbol\\\",\\\"bigint\\\"],\\\"string\\\",\\\"number\\\",\\\"number\\\","
        "\\\"number\\\",\\\"number\\\",\\\"number\\\"],\\n\\\"edge_fields\\\":[\\\"type\\\",\\\"name_or_index\\\","
        "\\\"to_node\\\"],\\n\\\"edge_types\\\":[[\\\"context\\\",\\\"element\\\",\\\"property\\\",\\\"internal\\\","
        "\\\"hidden\\\",\\\"shortcut\\\",\\\"weak\\\"],\\\"string_or_number\\\",\\\"node\\\"],\\n\\\"trace_function_"
        "info_fields\\\":[\\\"function_id\\\",\\\"name\\\",\\\"script_name\\\",\\\"script_id\\\",\\\"line\\\","
        "\\\"column\\\"],\\n\\\"trace_node_fields\\\":[\\\"id\\\",\\\"function_info_index\\\",\\\"count\\\","
        "\\\"size\\\",\\\"children\\\"],\\n\\\"sample_fields\\\":[\\\"timestamp_us\\\",\\\"last_assigned_id\\\"],"
        "\\n\\\"location_fields\\\":[\\\"object_index\\\",\\\"script_id\\\",\\\"line\\\",\\\"column\\\"]},\\n\\\"node_"
        "count\\\":32837,\\n\\\"edge_count\\\":152856,\\n\\\"trace_function_count\\\":0\\n},\\n\\\"nodes\\\":[9,25571,"
        "1,0,3575,0,0\\n,1,3,3,432,52,0,0\\n,8,9,9949,40,\"}}";
    const char* result3 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"1,0,0],\\n\\\"edges\\\":[],"
        "\\n\\\"locations\\\":[],\\n\\\"samples\\\":[],\\n\\\"strings\\\":[],\\n\\\"trace_function_infos\\\":[],"
        "\\n\\\"trace_tree\\\":[]}\\n\"}}";
    const char* result4 = "{\"id\":1,\"result\":{}}";
    JsHeapResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    htraceJSMemoryParser.Parse(tracePacket1, 10000);

    JsHeapResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket2(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket2, 11000);

    JsHeapResult jsHeapResult3;
    jsHeapResult3.set_result(result3);
    std::string strResult3 = "";
    jsHeapResult3.SerializeToString(&strResult3);
    ProtoReader::BytesView tracePacket3(reinterpret_cast<const uint8_t*>(strResult3.data()), strResult3.size());
    htraceJSMemoryParser.Parse(tracePacket3, 12000);

    JsHeapResult jsHeapResult4;
    jsHeapResult4.set_result(result4);
    std::string strResult4 = "";
    jsHeapResult4.SerializeToString(&strResult4);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult4.data()), strResult4.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000);
    htraceJSMemoryParser.Finish();

    auto size = stream_.traceDataCache_->GetConstJsHeapFilesData().Size();
    EXPECT_EQ(1, size);
    auto filePath = stream_.traceDataCache_->GetConstJsHeapFilesData().FilePaths()[0];
    EXPECT_EQ("Snapshot0", filePath);
    auto startTime = stream_.traceDataCache_->GetConstJsHeapFilesData().StartTimes()[0];
    EXPECT_EQ(11000, startTime);
    auto endTime = stream_.traceDataCache_->GetConstJsHeapFilesData().EndTimes()[0];
    EXPECT_EQ(13000, endTime);
    auto pidFiles = stream_.traceDataCache_->GetConstJsHeapFilesData().Pids()[0];
    EXPECT_EQ(1734, pidFiles);
    auto type = stream_.traceDataCache_->GetConstJsHeapNodesData().Types()[0];
    EXPECT_EQ(9, type);
    auto name = stream_.traceDataCache_->GetConstJsHeapNodesData().Names()[0];
    EXPECT_EQ(25571, name);
    auto nodeId = stream_.traceDataCache_->GetConstJsHeapNodesData().NodeIds()[0];
    EXPECT_EQ(1, nodeId);
    auto selfSize = stream_.traceDataCache_->GetConstJsHeapNodesData().SelfSizes()[0];
    EXPECT_EQ(0, selfSize);
    auto edgeCount = stream_.traceDataCache_->GetConstJsHeapNodesData().EdgeCounts()[0];
    EXPECT_EQ(3575, edgeCount);
    auto traceNodeId = stream_.traceDataCache_->GetConstJsHeapNodesData().TraceNodeIds()[0];
    EXPECT_EQ(0, traceNodeId);
    auto detachedNess = stream_.traceDataCache_->GetConstJsHeapNodesData().DetachedNess()[0];
    EXPECT_EQ(0, detachedNess);
}

HWTEST_F(JsMemoryTest, snapshotParserEdgesByJsmemory, TestSize.Level1)
{
    TS_LOGI("test35-2");
    const int32_t pid = 1734;
    JsHeapConfig jsHeapConfig;
    jsHeapConfig.set_pid(pid);
    jsHeapConfig.set_type(::JsHeapConfig_HeapType(0));
    std::string strConfig = "";
    jsHeapConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);

    const char* result1 =
        "{\"method\":\"HeapProfiler.reportHeapSnapshotProgress\",\"params\":{\"done\":0,\"total\":21288}}";
    const char* result2 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"{\\\"snapshot\\\":\\n{\\\"meta\\\":"
        "\\n{\\\"node_fields\\\":[\\\"type\\\",\\\"name\\\",\\\"id\\\",\\\"self_size\\\",\\\"edge_count\\\",\\\"trace_"
        "node_id\\\",\\\"detachedness\\\"],\\n\\\"node_types\\\":[[\\\"hidden\\\",\\\"array\\\",\\\"string\\\","
        "\\\"object\\\",\\\"code\\\",\\\"closure\\\",\\\"regexp\\\",\\\"number\\\",\\\"native\\\",\\\"synthetic\\\","
        "\\\"concatenated "
        "string\\\",\\\"slicedstring\\\",\\\"symbol\\\",\\\"bigint\\\"],\\\"string\\\",\\\"number\\\",\\\"number\\\","
        "\\\"number\\\",\\\"number\\\",\\\"number\\\"],\\n\\\"edge_fields\\\":[\\\"type\\\",\\\"name_or_index\\\","
        "\\\"to_node\\\"],\\n\\\"edge_types\\\":[[\\\"context\\\",\\\"element\\\",\\\"property\\\",\\\"internal\\\","
        "\\\"hidden\\\",\\\"shortcut\\\",\\\"weak\\\"],\\\"string_or_number\\\",\\\"node\\\"],\\n\\\"trace_function_"
        "info_fields\\\":[\\\"function_id\\\",\\\"name\\\",\\\"script_name\\\",\\\"script_id\\\",\\\"line\\\","
        "\\\"column\\\"],\\n\\\"trace_node_fields\\\":[\\\"id\\\",\\\"function_info_index\\\",\\\"count\\\","
        "\\\"size\\\",\\\"children\\\"],\\n\\\"sample_fields\\\":[\\\"timestamp_us\\\",\\\"last_assigned_id\\\"],"
        "\\n\\\"location_fields\\\":[\\\"object_index\\\",\\\"script_id\\\",\\\"line\\\",\\\"column\\\"]},\\n\\\"node_"
        "count\\\":3,\\n\\\"edge_count\\\":5,\\n\\\"trace_function_count\\\":0\\n},\\n\\\"nodes\\\":[9,25571,"
        "1,0,1,0,0\\n,1,3,3,432,3,0,0\\n,8,9,9949,40,\"}}";
    const char* result3 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"1,0,0],\\n\\\"edges\\\":[5,25572,"
        "1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n],"
        "\\n\\\"locations\\\":[],\\n\\\"samples\\\":[],\\n\\\"strings\\\":[],\\n\\\"trace_function_infos\\\":[],"
        "\\n\\\"trace_tree\\\":[]}\\n\"}}";
    const char* result4 = "{\"id\":1,\"result\":{}}";
    JsHeapResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    htraceJSMemoryParser.Parse(tracePacket1, 10000);

    JsHeapResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket2(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket2, 11000);

    JsHeapResult jsHeapResult3;
    jsHeapResult3.set_result(result3);
    std::string strResult3 = "";
    jsHeapResult3.SerializeToString(&strResult3);
    ProtoReader::BytesView tracePacket3(reinterpret_cast<const uint8_t*>(strResult3.data()), strResult3.size());
    htraceJSMemoryParser.Parse(tracePacket3, 12000);

    JsHeapResult jsHeapResult4;
    jsHeapResult4.set_result(result4);
    std::string strResult4 = "";
    jsHeapResult4.SerializeToString(&strResult4);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult4.data()), strResult4.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000);
    htraceJSMemoryParser.Finish();

    auto edgeType = stream_.traceDataCache_->GetConstJsHeapEdgesData().Types()[0];
    EXPECT_EQ(5, edgeType);
    auto nameOrIndex = stream_.traceDataCache_->GetConstJsHeapEdgesData().NameOrIndexs()[0];
    EXPECT_EQ(25572, nameOrIndex);
    auto toNodes = stream_.traceDataCache_->GetConstJsHeapEdgesData().ToNodes()[0];
    EXPECT_EQ(1, toNodes);
    auto fromNodeId = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[0];
    EXPECT_EQ(1, fromNodeId);
    auto fromNodeId1 = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[1];
    EXPECT_EQ(1, fromNodeId1);
    auto fromNodeId2 = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[2];
    EXPECT_EQ(1, fromNodeId2);
    auto fromNodeId3 = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[3];
    EXPECT_EQ(1, fromNodeId3);
    auto fromNodeId4 = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[4];
    EXPECT_EQ(1, fromNodeId4);
}

HWTEST_F(JsMemoryTest, timelineParserNodesByJsmemory, TestSize.Level1)
{
    TS_LOGI("test35-3");
    const int32_t pid = 1734;
    JsHeapConfig jsHeapConfig;
    jsHeapConfig.set_pid(pid);
    jsHeapConfig.set_type(::JsHeapConfig_HeapType(1));
    std::string strConfig = "";
    jsHeapConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);

    const char* result1 = "{\"id\":1,\"result\":{}}";
    const char* result2 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"{\\\"snapshot\\\":\\n{\\\"meta\\\":"
        "\\n{\\\"node_fields\\\":[\\\"type\\\",\\\"name\\\",\\\"id\\\",\\\"self_size\\\",\\\"edge_count\\\",\\\"trace_"
        "node_id\\\",\\\"detachedness\\\"],\\n\\\"node_types\\\":[[\\\"hidden\\\",\\\"array\\\",\\\"string\\\","
        "\\\"object\\\",\\\"code\\\",\\\"closure\\\",\\\"regexp\\\",\\\"number\\\",\\\"native\\\",\\\"synthetic\\\","
        "\\\"concatenated "
        "string\\\",\\\"slicedstring\\\",\\\"symbol\\\",\\\"bigint\\\"],\\\"string\\\",\\\"number\\\",\\\"number\\\","
        "\\\"number\\\",\\\"number\\\",\\\"number\\\"],\\n\\\"edge_fields\\\":[\\\"type\\\",\\\"name_or_index\\\","
        "\\\"to_node\\\"],\\n\\\"edge_types\\\":[[\\\"context\\\",\\\"element\\\",\\\"property\\\",\\\"internal\\\","
        "\\\"hidden\\\",\\\"shortcut\\\",\\\"weak\\\"],\\\"string_or_number\\\",\\\"node\\\"],\\n\\\"trace_function_"
        "info_fields\\\":[\\\"function_id\\\",\\\"name\\\",\\\"script_name\\\",\\\"script_id\\\",\\\"line\\\","
        "\\\"column\\\"],\\n\\\"trace_node_fields\\\":[\\\"id\\\",\\\"function_info_index\\\",\\\"count\\\","
        "\\\"size\\\",\\\"children\\\"],\\n\\\"sample_fields\\\":[\\\"timestamp_us\\\",\\\"last_assigned_id\\\"],"
        "\\n\\\"location_fields\\\":[\\\"object_index\\\",\\\"script_id\\\",\\\"line\\\",\\\"column\\\"]},\\n\\\"node_"
        "count\\\":32837,\\n\\\"edge_count\\\":152856,\\n\\\"trace_function_count\\\":0\\n},\\n\\\"nodes\\\":[9,25571,"
        "1,0,3575,0,0\\n,1,3,3,432,52,0,0\\n,8,9,9949,40,\"}}";
    const char* result3 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"1,0,0],\\n\\\"edges\\\":[],"
        "\\n\\\"locations\\\":[],\\n\\\"samples\\\":[],\\n\\\"strings\\\":[],\\n\\\"trace_function_infos\\\":[],"
        "\\n\\\"trace_tree\\\":[]}\\n\"}}";
    const char* result4 = "{\"id\":2,\"result\":{}}";
    JsHeapResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    htraceJSMemoryParser.Parse(tracePacket1, 10000);

    JsHeapResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket2(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket2, 11000);

    JsHeapResult jsHeapResult3;
    jsHeapResult3.set_result(result3);
    std::string strResult3 = "";
    jsHeapResult3.SerializeToString(&strResult3);
    ProtoReader::BytesView tracePacket3(reinterpret_cast<const uint8_t*>(strResult3.data()), strResult3.size());
    htraceJSMemoryParser.Parse(tracePacket3, 12000);

    JsHeapResult jsHeapResult4;
    jsHeapResult4.set_result(result4);
    std::string strResult4 = "";
    jsHeapResult4.SerializeToString(&strResult4);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult4.data()), strResult4.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000);
    htraceJSMemoryParser.Finish();

    auto size = stream_.traceDataCache_->GetConstJsHeapFilesData().Size();
    EXPECT_EQ(1, size);
    auto filePath = stream_.traceDataCache_->GetConstJsHeapFilesData().FilePaths()[0];
    EXPECT_EQ("Timeline", filePath);
    auto startTime = stream_.traceDataCache_->GetConstJsHeapFilesData().StartTimes()[0];
    EXPECT_EQ(10000, startTime);
    auto endTime = stream_.traceDataCache_->GetConstJsHeapFilesData().EndTimes()[0];
    EXPECT_EQ(13000, endTime);
    auto pidFiles = stream_.traceDataCache_->GetConstJsHeapFilesData().Pids()[0];
    EXPECT_EQ(1734, pidFiles);
    auto type = stream_.traceDataCache_->GetConstJsHeapNodesData().Types()[0];
    EXPECT_EQ(9, type);
    auto name = stream_.traceDataCache_->GetConstJsHeapNodesData().Names()[0];
    EXPECT_EQ(25571, name);
    auto nodeId = stream_.traceDataCache_->GetConstJsHeapNodesData().NodeIds()[0];
    EXPECT_EQ(1, nodeId);
    auto selfSize = stream_.traceDataCache_->GetConstJsHeapNodesData().SelfSizes()[0];
    EXPECT_EQ(0, selfSize);
    auto edgeCount = stream_.traceDataCache_->GetConstJsHeapNodesData().EdgeCounts()[0];
    EXPECT_EQ(3575, edgeCount);
    auto traceNodeId = stream_.traceDataCache_->GetConstJsHeapNodesData().TraceNodeIds()[0];
    EXPECT_EQ(0, traceNodeId);
    auto detachedNess = stream_.traceDataCache_->GetConstJsHeapNodesData().DetachedNess()[0];
    EXPECT_EQ(0, detachedNess);
}

HWTEST_F(JsMemoryTest, timelineParserEdgesByJsmemory, TestSize.Level1)
{
    TS_LOGI("test35-4");
    const int32_t pid = 1734;
    JsHeapConfig jsHeapConfig;
    jsHeapConfig.set_pid(pid);
    jsHeapConfig.set_type(::JsHeapConfig_HeapType(1));
    std::string strConfig = "";
    jsHeapConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);

    const char* result1 = "{\"id\":1,\"result\":{}}";
    const char* result2 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"{\\\"snapshot\\\":\\n{\\\"meta\\\":"
        "\\n{\\\"node_fields\\\":[\\\"type\\\",\\\"name\\\",\\\"id\\\",\\\"self_size\\\",\\\"edge_count\\\",\\\"trace_"
        "node_id\\\",\\\"detachedness\\\"],\\n\\\"node_types\\\":[[\\\"hidden\\\",\\\"array\\\",\\\"string\\\","
        "\\\"object\\\",\\\"code\\\",\\\"closure\\\",\\\"regexp\\\",\\\"number\\\",\\\"native\\\",\\\"synthetic\\\","
        "\\\"concatenated "
        "string\\\",\\\"slicedstring\\\",\\\"symbol\\\",\\\"bigint\\\"],\\\"string\\\",\\\"number\\\",\\\"number\\\","
        "\\\"number\\\",\\\"number\\\",\\\"number\\\"],\\n\\\"edge_fields\\\":[\\\"type\\\",\\\"name_or_index\\\","
        "\\\"to_node\\\"],\\n\\\"edge_types\\\":[[\\\"context\\\",\\\"element\\\",\\\"property\\\",\\\"internal\\\","
        "\\\"hidden\\\",\\\"shortcut\\\",\\\"weak\\\"],\\\"string_or_number\\\",\\\"node\\\"],\\n\\\"trace_function_"
        "info_fields\\\":[\\\"function_id\\\",\\\"name\\\",\\\"script_name\\\",\\\"script_id\\\",\\\"line\\\","
        "\\\"column\\\"],\\n\\\"trace_node_fields\\\":[\\\"id\\\",\\\"function_info_index\\\",\\\"count\\\","
        "\\\"size\\\",\\\"children\\\"],\\n\\\"sample_fields\\\":[\\\"timestamp_us\\\",\\\"last_assigned_id\\\"],"
        "\\n\\\"location_fields\\\":[\\\"object_index\\\",\\\"script_id\\\",\\\"line\\\",\\\"column\\\"]},\\n\\\"node_"
        "count\\\":32837,\\n\\\"edge_count\\\":152856,\\n\\\"trace_function_count\\\":0\\n},\\n\\\"nodes\\\":[9,25571,"
        "1,0,1,0,0\\n,1,3,3,432,3,0,0\\n,8,9,9949,40,\"}}";
    const char* result3 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"1,0,0],\\n\\\"edges\\\":[5,25572,"
        "1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n],"
        "\\n\\\"locations\\\":[],\\n\\\"samples\\\":[],\\n\\\"strings\\\":[],\\n\\\"trace_function_infos\\\":[],"
        "\\n\\\"trace_tree\\\":[]}\\n\"}}";
    const char* result4 = "{\"id\":2,\"result\":{}}";
    JsHeapResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    htraceJSMemoryParser.Parse(tracePacket1, 10000);

    JsHeapResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket2(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket2, 11000);

    JsHeapResult jsHeapResult3;
    jsHeapResult3.set_result(result3);
    std::string strResult3 = "";
    jsHeapResult3.SerializeToString(&strResult3);
    ProtoReader::BytesView tracePacket3(reinterpret_cast<const uint8_t*>(strResult3.data()), strResult3.size());
    htraceJSMemoryParser.Parse(tracePacket3, 12000);

    JsHeapResult jsHeapResult4;
    jsHeapResult4.set_result(result4);
    std::string strResult4 = "";
    jsHeapResult4.SerializeToString(&strResult4);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult4.data()), strResult4.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000);
    htraceJSMemoryParser.Finish();

    auto edgeType = stream_.traceDataCache_->GetConstJsHeapEdgesData().Types()[0];
    EXPECT_EQ(5, edgeType);
    auto nameOrIndex = stream_.traceDataCache_->GetConstJsHeapEdgesData().NameOrIndexs()[0];
    EXPECT_EQ(25572, nameOrIndex);
    auto toNodes = stream_.traceDataCache_->GetConstJsHeapEdgesData().ToNodes()[0];
    EXPECT_EQ(1, toNodes);
    auto fromNodeId = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[0];
    EXPECT_EQ(1, fromNodeId);
    auto fromNodeId1 = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[1];
    EXPECT_EQ(1, fromNodeId1);
    auto fromNodeId2 = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[2];
    EXPECT_EQ(1, fromNodeId2);
    auto fromNodeId3 = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[3];
    EXPECT_EQ(1, fromNodeId3);
    auto fromNodeId4 = stream_.traceDataCache_->GetConstJsHeapEdgesData().FromNodeIds()[4];
    EXPECT_EQ(1, fromNodeId4);
}

HWTEST_F(JsMemoryTest, timelineParserSamplesByJsmemory, TestSize.Level1)
{
    TS_LOGI("test35-5");
    const int32_t pid = 1734;
    JsHeapConfig jsHeapConfig;
    jsHeapConfig.set_pid(pid);
    jsHeapConfig.set_type(::JsHeapConfig_HeapType(1));
    std::string strConfig = "";
    jsHeapConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);

    const char* result1 = "{\"id\":1,\"result\":{}}";
    const char* result2 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"{\\\"snapshot\\\":\\n{\\\"meta\\\":"
        "\\n{\\\"node_fields\\\":[\\\"type\\\",\\\"name\\\",\\\"id\\\",\\\"self_size\\\",\\\"edge_count\\\",\\\"trace_"
        "node_id\\\",\\\"detachedness\\\"],\\n\\\"node_types\\\":[[\\\"hidden\\\",\\\"array\\\",\\\"string\\\","
        "\\\"object\\\",\\\"code\\\",\\\"closure\\\",\\\"regexp\\\",\\\"number\\\",\\\"native\\\",\\\"synthetic\\\","
        "\\\"concatenated "
        "string\\\",\\\"slicedstring\\\",\\\"symbol\\\",\\\"bigint\\\"],\\\"string\\\",\\\"number\\\",\\\"number\\\","
        "\\\"number\\\",\\\"number\\\",\\\"number\\\"],\\n\\\"edge_fields\\\":[\\\"type\\\",\\\"name_or_index\\\","
        "\\\"to_node\\\"],\\n\\\"edge_types\\\":[[\\\"context\\\",\\\"element\\\",\\\"property\\\",\\\"internal\\\","
        "\\\"hidden\\\",\\\"shortcut\\\",\\\"weak\\\"],\\\"string_or_number\\\",\\\"node\\\"],\\n\\\"trace_function_"
        "info_fields\\\":[\\\"function_id\\\",\\\"name\\\",\\\"script_name\\\",\\\"script_id\\\",\\\"line\\\","
        "\\\"column\\\"],\\n\\\"trace_node_fields\\\":[\\\"id\\\",\\\"function_info_index\\\",\\\"count\\\","
        "\\\"size\\\",\\\"children\\\"],\\n\\\"sample_fields\\\":[\\\"timestamp_us\\\",\\\"last_assigned_id\\\"],"
        "\\n\\\"location_fields\\\":[\\\"object_index\\\",\\\"script_id\\\",\\\"line\\\",\\\"column\\\"]},\\n\\\"node_"
        "count\\\":32837,\\n\\\"edge_count\\\":152856,\\n\\\"trace_function_count\\\":0\\n},\\n\\\"nodes\\\":[9,25571,"
        "1,0,1,0,0\\n,1,3,3,432,3,0,0\\n,8,9,9949,40,\"}}";
    const char* result3 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"1,0,0],\\n\\\"edges\\\":[5,25572,"
        "1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n],"
        "\\n\\\"locations\\\":[],\\n\\\"samples\\\":[0, 42570\\n, 200631, 42571\\n, 401040, 42572\\n, 601899, "
        "42573\\n, 804764, 42574\\n, 1006866, 42575\\n, 1207797, "
        "42576\\n],\\n\\\"strings\\\":[],\\n\\\"trace_function_infos\\\":[],"
        "\\n\\\"trace_tree\\\":[]}\\n\"}}";
    const char* result4 = "{\"id\":2,\"result\":{}}";
    JsHeapResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    htraceJSMemoryParser.Parse(tracePacket1, 10000);

    JsHeapResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket2(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket2, 11000);

    JsHeapResult jsHeapResult3;
    jsHeapResult3.set_result(result3);
    std::string strResult3 = "";
    jsHeapResult3.SerializeToString(&strResult3);
    ProtoReader::BytesView tracePacket3(reinterpret_cast<const uint8_t*>(strResult3.data()), strResult3.size());
    htraceJSMemoryParser.Parse(tracePacket3, 12000);

    JsHeapResult jsHeapResult4;
    jsHeapResult4.set_result(result4);
    std::string strResult4 = "";
    jsHeapResult4.SerializeToString(&strResult4);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult4.data()), strResult4.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000);
    htraceJSMemoryParser.Finish();

    auto timeStampUs = stream_.traceDataCache_->GetConstJsHeapSampleData().TimeStampUs()[0];
    EXPECT_EQ(0, timeStampUs);
    auto lastAssignedIds = stream_.traceDataCache_->GetConstJsHeapSampleData().LastAssignedIds()[0];
    EXPECT_EQ(42570, lastAssignedIds);
    auto timeStampUs1 = stream_.traceDataCache_->GetConstJsHeapSampleData().TimeStampUs()[1];
    EXPECT_EQ(200631, timeStampUs1);
    auto lastAssignedIds1 = stream_.traceDataCache_->GetConstJsHeapSampleData().LastAssignedIds()[1];
    EXPECT_EQ(42571, lastAssignedIds1);
    auto timeStampUs2 = stream_.traceDataCache_->GetConstJsHeapSampleData().TimeStampUs()[2];
    EXPECT_EQ(401040, timeStampUs2);
    auto lastAssignedIds2 = stream_.traceDataCache_->GetConstJsHeapSampleData().LastAssignedIds()[2];
    EXPECT_EQ(42572, lastAssignedIds2);
    auto timeStampUs3 = stream_.traceDataCache_->GetConstJsHeapSampleData().TimeStampUs()[3];
    EXPECT_EQ(601899, timeStampUs3);
    auto lastAssignedIds3 = stream_.traceDataCache_->GetConstJsHeapSampleData().LastAssignedIds()[3];
    EXPECT_EQ(42573, lastAssignedIds3);
}

HWTEST_F(JsMemoryTest, timelineParserStringsByJsmemory, TestSize.Level1)
{
    TS_LOGI("test35-6");
    const int32_t pid = 1734;
    JsHeapConfig jsHeapConfig;
    jsHeapConfig.set_pid(pid);
    jsHeapConfig.set_type(::JsHeapConfig_HeapType(1));
    std::string strConfig = "";
    jsHeapConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);

    const char* result1 = "{\"id\":1,\"result\":{}}";
    const char* result2 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"{\\\"snapshot\\\":\\n{\\\"meta\\\":"
        "\\n{\\\"node_fields\\\":[\\\"type\\\",\\\"name\\\",\\\"id\\\",\\\"self_size\\\",\\\"edge_count\\\",\\\"trace_"
        "node_id\\\",\\\"detachedness\\\"],\\n\\\"node_types\\\":[[\\\"hidden\\\",\\\"array\\\",\\\"string\\\","
        "\\\"object\\\",\\\"code\\\",\\\"closure\\\",\\\"regexp\\\",\\\"number\\\",\\\"native\\\",\\\"synthetic\\\","
        "\\\"concatenated "
        "string\\\",\\\"slicedstring\\\",\\\"symbol\\\",\\\"bigint\\\"],\\\"string\\\",\\\"number\\\",\\\"number\\\","
        "\\\"number\\\",\\\"number\\\",\\\"number\\\"],\\n\\\"edge_fields\\\":[\\\"type\\\",\\\"name_or_index\\\","
        "\\\"to_node\\\"],\\n\\\"edge_types\\\":[[\\\"context\\\",\\\"element\\\",\\\"property\\\",\\\"internal\\\","
        "\\\"hidden\\\",\\\"shortcut\\\",\\\"weak\\\"],\\\"string_or_number\\\",\\\"node\\\"],\\n\\\"trace_function_"
        "info_fields\\\":[\\\"function_id\\\",\\\"name\\\",\\\"script_name\\\",\\\"script_id\\\",\\\"line\\\","
        "\\\"column\\\"],\\n\\\"trace_node_fields\\\":[\\\"id\\\",\\\"function_info_index\\\",\\\"count\\\","
        "\\\"size\\\",\\\"children\\\"],\\n\\\"sample_fields\\\":[\\\"timestamp_us\\\",\\\"last_assigned_id\\\"],"
        "\\n\\\"location_fields\\\":[\\\"object_index\\\",\\\"script_id\\\",\\\"line\\\",\\\"column\\\"]},\\n\\\"node_"
        "count\\\":32837,\\n\\\"edge_count\\\":152856,\\n\\\"trace_function_count\\\":0\\n},\\n\\\"nodes\\\":[9,25571,"
        "1,0,1,0,0\\n,1,3,3,432,3,0,0\\n,8,9,9949,40,\"}}";
    const char* result3 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"1,0,0],\\n\\\"edges\\\":[5,25572,"
        "1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n],"
        "\\n\\\"locations\\\":[],\\n\\\"samples\\\":[0, 42570\\n, 200631, 42571\\n, 401040, 42572\\n, 601899, "
        "42573\\n, 804764, 42574\\n, 1006866, 42575\\n, 1207797, "
        "42576\\n],\\n\\\"strings\\\":[\\\"<dummy>\\\",\\n\\\"\\\",\\n\\\"GC "
        "roots\\\",\\n\\\"TaggedDict[52]\\\",\\n\\\"JSFunction\\\"],\\n\\\"trace_function_infos\\\":[],"
        "\\n\\\"trace_tree\\\":[]}\\n\"}}";
    const char* result4 = "{\"id\":2,\"result\":{}}";
    JsHeapResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    htraceJSMemoryParser.Parse(tracePacket1, 10000);

    JsHeapResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket2(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket2, 11000);

    JsHeapResult jsHeapResult3;
    jsHeapResult3.set_result(result3);
    std::string strResult3 = "";
    jsHeapResult3.SerializeToString(&strResult3);
    ProtoReader::BytesView tracePacket3(reinterpret_cast<const uint8_t*>(strResult3.data()), strResult3.size());
    htraceJSMemoryParser.Parse(tracePacket3, 12000);

    JsHeapResult jsHeapResult4;
    jsHeapResult4.set_result(result4);
    std::string strResult4 = "";
    jsHeapResult4.SerializeToString(&strResult4);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult4.data()), strResult4.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000);
    htraceJSMemoryParser.Finish();

    auto string = stream_.traceDataCache_->GetConstJsHeapStringData().Strings()[0];
    EXPECT_EQ("<dummy>", string);
    auto string1 = stream_.traceDataCache_->GetConstJsHeapStringData().Strings()[1];
    EXPECT_EQ("", string1);
    auto string2 = stream_.traceDataCache_->GetConstJsHeapStringData().Strings()[2];
    EXPECT_EQ("GC roots", string2);
    auto string3 = stream_.traceDataCache_->GetConstJsHeapStringData().Strings()[3];
    EXPECT_EQ("TaggedDict[52]", string3);
    auto string4 = stream_.traceDataCache_->GetConstJsHeapStringData().Strings()[4];
    EXPECT_EQ("JSFunction", string4);
}

HWTEST_F(JsMemoryTest, timelineParserTraceFuncInfoByJsmemory, TestSize.Level1)
{
    TS_LOGI("test35-7");
    const int32_t pid = 1734;
    JsHeapConfig jsHeapConfig;
    jsHeapConfig.set_pid(pid);
    jsHeapConfig.set_type(::JsHeapConfig_HeapType(1));
    std::string strConfig = "";
    jsHeapConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);

    const char* result1 = "{\"id\":1,\"result\":{}}";
    const char* result2 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"{\\\"snapshot\\\":\\n{\\\"meta\\\":"
        "\\n{\\\"node_fields\\\":[\\\"type\\\",\\\"name\\\",\\\"id\\\",\\\"self_size\\\",\\\"edge_count\\\",\\\"trace_"
        "node_id\\\",\\\"detachedness\\\"],\\n\\\"node_types\\\":[[\\\"hidden\\\",\\\"array\\\",\\\"string\\\","
        "\\\"object\\\",\\\"code\\\",\\\"closure\\\",\\\"regexp\\\",\\\"number\\\",\\\"native\\\",\\\"synthetic\\\","
        "\\\"concatenated "
        "string\\\",\\\"slicedstring\\\",\\\"symbol\\\",\\\"bigint\\\"],\\\"string\\\",\\\"number\\\",\\\"number\\\","
        "\\\"number\\\",\\\"number\\\",\\\"number\\\"],\\n\\\"edge_fields\\\":[\\\"type\\\",\\\"name_or_index\\\","
        "\\\"to_node\\\"],\\n\\\"edge_types\\\":[[\\\"context\\\",\\\"element\\\",\\\"property\\\",\\\"internal\\\","
        "\\\"hidden\\\",\\\"shortcut\\\",\\\"weak\\\"],\\\"string_or_number\\\",\\\"node\\\"],\\n\\\"trace_function_"
        "info_fields\\\":[\\\"function_id\\\",\\\"name\\\",\\\"script_name\\\",\\\"script_id\\\",\\\"line\\\","
        "\\\"column\\\"],\\n\\\"trace_node_fields\\\":[\\\"id\\\",\\\"function_info_index\\\",\\\"count\\\","
        "\\\"size\\\",\\\"children\\\"],\\n\\\"sample_fields\\\":[\\\"timestamp_us\\\",\\\"last_assigned_id\\\"],"
        "\\n\\\"location_fields\\\":[\\\"object_index\\\",\\\"script_id\\\",\\\"line\\\",\\\"column\\\"]},\\n\\\"node_"
        "count\\\":32837,\\n\\\"edge_count\\\":152856,\\n\\\"trace_function_count\\\":0\\n},\\n\\\"nodes\\\":[9,25571,"
        "1,0,1,0,0\\n,1,3,3,432,3,0,0\\n,8,9,9949,40,\"}}";
    const char* result3 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"1,0,0],\\n\\\"edges\\\":[5,25572,"
        "1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n],"
        "\\n\\\"locations\\\":[],\\n\\\"samples\\\":[],\\n\\\"strings\\\":[],\\n\\\"trace_function_infos\\\":[0,181,"
        "1601,0,0,"
        "0\\n],"
        "\\n\\\"trace_tree\\\":[]}\\n\"}}";
    const char* result4 = "{\"id\":2,\"result\":{}}";
    JsHeapResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    htraceJSMemoryParser.Parse(tracePacket1, 10000);

    JsHeapResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket2(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket2, 11000);

    JsHeapResult jsHeapResult3;
    jsHeapResult3.set_result(result3);
    std::string strResult3 = "";
    jsHeapResult3.SerializeToString(&strResult3);
    ProtoReader::BytesView tracePacket3(reinterpret_cast<const uint8_t*>(strResult3.data()), strResult3.size());
    htraceJSMemoryParser.Parse(tracePacket3, 12000);

    JsHeapResult jsHeapResult4;
    jsHeapResult4.set_result(result4);
    std::string strResult4 = "";
    jsHeapResult4.SerializeToString(&strResult4);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult4.data()), strResult4.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000);
    htraceJSMemoryParser.Finish();

    auto functionId = stream_.traceDataCache_->GetConstJsHeapTraceFuncInfoData().FunctionIds()[0];
    EXPECT_EQ(0, functionId);
    auto name = stream_.traceDataCache_->GetConstJsHeapTraceFuncInfoData().Names()[0];
    EXPECT_EQ(181, name);
    auto scriptName = stream_.traceDataCache_->GetConstJsHeapTraceFuncInfoData().ScriptNames()[0];
    EXPECT_EQ(1601, scriptName);
    auto scriptId = stream_.traceDataCache_->GetConstJsHeapTraceFuncInfoData().ScriptIds()[0];
    EXPECT_EQ(0, scriptId);
    auto line = stream_.traceDataCache_->GetConstJsHeapTraceFuncInfoData().Lines()[0];
    EXPECT_EQ(0, line);
    auto column = stream_.traceDataCache_->GetConstJsHeapTraceFuncInfoData().Columns()[0];
    EXPECT_EQ(0, column);
}

HWTEST_F(JsMemoryTest, timelineParserTraceTreeByJsmemory, TestSize.Level1)
{
    TS_LOGI("test35-8");
    const int32_t pid = 1734;
    JsHeapConfig jsHeapConfig;
    jsHeapConfig.set_pid(pid);
    jsHeapConfig.set_type(::JsHeapConfig_HeapType(1));
    std::string strConfig = "";
    jsHeapConfig.SerializeToString(&strConfig);

    HtraceJSMemoryParser htraceJSMemoryParser(stream_.traceDataCache_.get(), stream_.streamFilters_.get());
    ProtoReader::BytesView tracePacket(reinterpret_cast<const uint8_t*>(strConfig.data()), strConfig.size());
    htraceJSMemoryParser.ParseJSMemoryConfig(tracePacket);

    const char* result1 = "{\"id\":1,\"result\":{}}";
    const char* result2 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"{\\\"snapshot\\\":\\n{\\\"meta\\\":"
        "\\n{\\\"node_fields\\\":[\\\"type\\\",\\\"name\\\",\\\"id\\\",\\\"self_size\\\",\\\"edge_count\\\",\\\"trace_"
        "node_id\\\",\\\"detachedness\\\"],\\n\\\"node_types\\\":[[\\\"hidden\\\",\\\"array\\\",\\\"string\\\","
        "\\\"object\\\",\\\"code\\\",\\\"closure\\\",\\\"regexp\\\",\\\"number\\\",\\\"native\\\",\\\"synthetic\\\","
        "\\\"concatenated "
        "string\\\",\\\"slicedstring\\\",\\\"symbol\\\",\\\"bigint\\\"],\\\"string\\\",\\\"number\\\",\\\"number\\\","
        "\\\"number\\\",\\\"number\\\",\\\"number\\\"],\\n\\\"edge_fields\\\":[\\\"type\\\",\\\"name_or_index\\\","
        "\\\"to_node\\\"],\\n\\\"edge_types\\\":[[\\\"context\\\",\\\"element\\\",\\\"property\\\",\\\"internal\\\","
        "\\\"hidden\\\",\\\"shortcut\\\",\\\"weak\\\"],\\\"string_or_number\\\",\\\"node\\\"],\\n\\\"trace_function_"
        "info_fields\\\":[\\\"function_id\\\",\\\"name\\\",\\\"script_name\\\",\\\"script_id\\\",\\\"line\\\","
        "\\\"column\\\"],\\n\\\"trace_node_fields\\\":[\\\"id\\\",\\\"function_info_index\\\",\\\"count\\\","
        "\\\"size\\\",\\\"children\\\"],\\n\\\"sample_fields\\\":[\\\"timestamp_us\\\",\\\"last_assigned_id\\\"],"
        "\\n\\\"location_fields\\\":[\\\"object_index\\\",\\\"script_id\\\",\\\"line\\\",\\\"column\\\"]},\\n\\\"node_"
        "count\\\":32837,\\n\\\"edge_count\\\":152856,\\n\\\"trace_function_count\\\":0\\n},\\n\\\"nodes\\\":[9,25571,"
        "1,0,1,0,0\\n,1,3,3,432,3,0,0\\n,8,9,9949,40,\"}}";
    const char* result3 =
        "{\"method\":\"HeapProfiler.addHeapSnapshotChunk\",\"params\":{\"chunk\":\"1,0,0],\\n\\\"edges\\\":[5,25572,"
        "1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n,5,25572,1\\n],"
        "\\n\\\"locations\\\":[],\\n\\\"samples\\\":[],\\n\\\"strings\\\":[],\\n\\\"trace_function_infos\\\":[0,181,"
        "1601,0,0,"
        "0\\n],"
        "\\n\\\"trace_tree\\\":[1,0,53,996,[2,1,571,26580,[],3,2,5,248,[4,3,27,2684,[5,4,1352,68984,[6,5,20,920,[]],7,"
        "6,5,144,[],8,7,18,636,[9,8,5,120,[10,9,1,28,[]]]]],11,10,10,348,[12,11,11,576,[13,12,21,632,[14,14,0,0,[15,13,"
        "2,28,[]],16,15,5,188,[17,16,7,196,[]]]]],18,17,10,560,[19,9,2,28,[],20,18,172,7896,[21,19,4,3196,[]]]]]}\\n\"}"
        "}";
    const char* result4 = "{\"id\":2,\"result\":{}}";
    JsHeapResult jsHeapResult1;
    jsHeapResult1.set_result(result1);
    std::string strResult1 = "";
    jsHeapResult1.SerializeToString(&strResult1);
    ProtoReader::BytesView tracePacket1(reinterpret_cast<const uint8_t*>(strResult1.data()), strResult1.size());
    htraceJSMemoryParser.Parse(tracePacket1, 10000);

    JsHeapResult jsHeapResult2;
    jsHeapResult2.set_result(result2);
    std::string strResult2 = "";
    jsHeapResult2.SerializeToString(&strResult2);
    ProtoReader::BytesView tracePacket2(reinterpret_cast<const uint8_t*>(strResult2.data()), strResult2.size());
    htraceJSMemoryParser.Parse(tracePacket2, 11000);

    JsHeapResult jsHeapResult3;
    jsHeapResult3.set_result(result3);
    std::string strResult3 = "";
    jsHeapResult3.SerializeToString(&strResult3);
    ProtoReader::BytesView tracePacket3(reinterpret_cast<const uint8_t*>(strResult3.data()), strResult3.size());
    htraceJSMemoryParser.Parse(tracePacket3, 12000);

    JsHeapResult jsHeapResult4;
    jsHeapResult4.set_result(result4);
    std::string strResult4 = "";
    jsHeapResult4.SerializeToString(&strResult4);
    ProtoReader::BytesView tracePacket4(reinterpret_cast<const uint8_t*>(strResult4.data()), strResult4.size());
    htraceJSMemoryParser.Parse(tracePacket4, 13000);
    htraceJSMemoryParser.Finish();

    auto functionInfoIndex = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().FunctionInfoIndexs()[0];
    EXPECT_EQ(0, functionInfoIndex);
    auto count = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().Counts()[0];
    EXPECT_EQ(53, count);
    auto nodeSize = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().NodeSizes()[0];
    EXPECT_EQ(996, nodeSize);
    auto parentId = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().ParentIds()[0];
    EXPECT_EQ(-1, parentId);
    auto functionInfoIndex1 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().FunctionInfoIndexs()[1];
    EXPECT_EQ(1, functionInfoIndex1);
    auto count1 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().Counts()[1];
    EXPECT_EQ(571, count1);
    auto nodeSize1 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().NodeSizes()[1];
    EXPECT_EQ(26580, nodeSize1);
    auto parentId1 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().ParentIds()[1];
    EXPECT_EQ(1, parentId1);
    auto functionInfoIndex2 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().FunctionInfoIndexs()[2];
    EXPECT_EQ(2, functionInfoIndex2);
    auto count2 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().Counts()[2];
    EXPECT_EQ(5, count2);
    auto nodeSize2 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().NodeSizes()[2];
    EXPECT_EQ(248, nodeSize2);
    auto parentId2 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().ParentIds()[2];
    EXPECT_EQ(1, parentId2);
    auto parentId3 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().ParentIds()[3];
    EXPECT_EQ(3, parentId3);
    auto parentId4 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().ParentIds()[4];
    EXPECT_EQ(4, parentId4);
    auto functionInfoIndex10 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().FunctionInfoIndexs()[10];
    EXPECT_EQ(10, functionInfoIndex10);
    auto count10 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().Counts()[10];
    EXPECT_EQ(10, count10);
    auto nodeSize10 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().NodeSizes()[10];
    EXPECT_EQ(348, nodeSize10);
    auto parentId10 = stream_.traceDataCache_->GetConstJsHeapTraceNodeData().ParentIds()[10];
    EXPECT_EQ(1, parentId10);
}
} // namespace TraceStreamer
} // namespace SysTuning
