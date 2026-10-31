/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
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
#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace mem_analyzer {

// HiProfiler NativeHookConfig（native_hook_config.proto）
// 未出现在文件中的字段保持默认值。
struct HtraceNativeHookConfig {
    bool present = false;                     // 是否解析到 nativehook_config 段
    int32_t pid = 0;                          // 1
    bool saveFile = false;                    // 2
    std::string fileName;                     // 3
    int32_t filterSize = 0;                   // 4
    int32_t smbPages = 0;                     // 5
    int32_t maxStackDepth = 0;                // 6
    std::string processName;                  // 7
    bool mallocDisable = false;               // 8
    bool mmapDisable = false;                 // 9
    bool freeStackReport = false;             // 10
    bool munmapStackReport = false;           // 11
    uint32_t mallocFreeMatchingInterval = 0;  // 12
    uint32_t mallocFreeMatchingCnt = 0;       // 13
    bool stringCompressed = false;            // 14
    bool fpUnwind = false;                    // 15
    bool blocked = false;                     // 16
    bool recordAccurately = false;            // 17
    bool startupMode = false;                 // 18
    bool memtraceEnable = false;              // 19
    bool offlineSymbolization = false;        // 20
    bool callframeCompress = false;           // 21
    uint32_t statisticsInterval = 0;          // 22
    std::string clock;                        // 23
    uint32_t sampleInterval = 0;              // 24
    bool responseLibraryMode = false;         // 25
    std::vector<int32_t> expandPids;          // 26 (repeated int32)
    int32_t  jsStackReport = 0;               // 27
    uint32_t maxJsStackDepth = 0;             // 28
    std::string filterNapiName;               // 29
    bool dumpNmd = false;                     // 30
    std::string targetSoName;                 // 31
    std::vector<std::string> restraceTag;     // 32 (repeated string)
    bool asyncStackEnable = false;            // 33
    std::vector<uint32_t> asyncType;          // 34 (repeated enum)
    uint32_t uniqueStackTableSize = 0;        // 35
    bool useFileCacheMode = false;            // 36
    uint32_t fileCacheBufferSize = 0;         // 37
    bool allocatorSamplingEnable = false;     // 38
    bool nativeOrphansnapshotsEnable = false; // 39
    bool nestedAsyncStackEnable = false;      // 40
    std::vector<std::string> filterLibList;   // 41 (repeated string)
    uint32_t asyncNestingDepth = 0;           // 42
    uint32_t asyncTaskStackDepth = 0;         // 43
    bool arkuiMemTraceEnable = false;         // 44
    bool discardDestroyedTraces = false;      // 45
};

// 直接读取原始 .htrace 文件信息（零依赖，手工解析 protobuf wire format）。
// 用于在 trace_streamer 转换结果缺失时兜底读取 process_name，以及文件类型判定。
class HtraceReader {
public:
    // profiler 文件判断：魔数合法 && dataType == HIPROFILER_PROTOBUF_BIN(0)。
    // 判断型函数：文件打不开/格式异常统一返回 false，不抛异常。
    static bool IsProfilerFile(const std::string& path);

    // 解析第一个 nativehook_config 段并填充全部字段。
    static bool ReadNativehookConfig(const std::string& path, HtraceNativeHookConfig& out);

    // 便捷入口：只取 process_name（内部复用 read_nativehook_config）。
    static std::string ReadProcessName(const std::string& path);
};

} // namespace mem_analyzer