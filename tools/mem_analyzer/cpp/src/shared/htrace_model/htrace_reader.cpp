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
#include "shared/htrace_model/htrace_reader.hpp"
#include "utils/file_utils.hpp"
#include "utils/logger.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace mem_analyzer {

namespace {

constexpr uint32_t kHeaderSize = 1024;
constexpr uint64_t kHeaderMagic = 0x464F5250534F484FULL; // 小端 == "OHOSPROF"

enum class HtraceDataType : uint32_t {
    ProtobufBin = 0,    // HIPROFILER_PROTOBUF_BIN，标准 HiProfiler protobuf 二进制
    Hiperf      = 1,    // hiperf 采样原始数据
    Standalone  = 1000, // 独立插件文件
};

struct HtraceHeaderInfo {
    bool     valid = false;
    uint64_t magic = 0;
    uint64_t length = 0;
    uint32_t version = 0;
    uint32_t dataType = 0;
    uint64_t boottimeNs = 0;
    uint64_t realtimeNs = 0;
    uint64_t durationNs = 0;
};

// ---------------------------------------------------------------------------
// 小端整数读取（避免未对齐/别名问题）
// ---------------------------------------------------------------------------
uint32_t ReadU32(const uint8_t* p)
{
    uint32_t v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}
uint64_t ReadU64(const uint8_t* p)
{
    uint64_t v;
    std::memcpy(&v, p, sizeof(v));
    return v;
}

int32_t AsInt32(uint64_t v)
{
    return static_cast<int32_t>(static_cast<uint32_t>(v & 0xFFFFFFFFULL));
}

// ---------------------------------------------------------------------------
// protobuf wire format（零依赖手工解析）
// ---------------------------------------------------------------------------
uint64_t ReadVarint(const uint8_t* buf, size_t size, size_t& pos)
{
    uint64_t result = 0;
    int shift = 0;
    while (pos < size) {
        if (shift >= 64) {
            throw std::runtime_error("varint too long");
        }
        uint8_t b = buf[pos++];
        result |= static_cast<uint64_t>(b & 0x7F) << shift;
        if (!(b & 0x80)) {
            return result;
        }
        shift += 7;
    }
    throw std::runtime_error("varint truncated or too long");
}

struct Field {
    uint32_t number = 0;
    uint32_t wireType = 0;  // 0 varint, 1 fixed64, 2 len, 5 fixed32
    uint64_t varint = 0;
    std::string bytes;
};

std::vector<Field> ParseFields(const uint8_t* buf, size_t size)
{
    std::vector<Field> fields;
    size_t pos = 0;
    while (pos < size) {
        uint64_t tag = ReadVarint(buf, size, pos);
        Field f;
        f.number = static_cast<uint32_t>(tag >> 3);
        f.wireType = tag & 0x07;
        switch (f.wireType) {
            case 0:
                f.varint = ReadVarint(buf, size, pos);
                break;
            case 1:
                if (pos + 8 > size) {
                    throw std::runtime_error("fixed64 out of range");
                }
                f.varint = ReadU64(buf + pos);
                pos += 8;
                break;
            case 2: {
                uint64_t len = ReadVarint(buf, size, pos);
                if (len > size - pos) {
                    throw std::runtime_error("length-delimited out of range");
                }
                f.bytes.assign(reinterpret_cast<const char*>(buf + pos), static_cast<size_t>(len));
                pos += static_cast<size_t>(len);
                break;
            }
            case 5:
                if (pos + 4 > size) {
                    throw std::runtime_error("fixed32 out of range");
                }
                f.varint = ReadU32(buf + pos);
                pos += 4;
                break;
            default:
                throw std::runtime_error("unsupported wire type");
        }
        fields.push_back(std::move(f));
    }
    return fields;
}

std::vector<uint64_t> UnpackPackedVarints(const std::string& raw)
{
    std::vector<uint64_t> vals;
    size_t pos = 0;
    while (pos < raw.size()) {
        vals.push_back(ReadVarint(reinterpret_cast<const uint8_t*>(raw.data()), raw.size(), pos));
    }
    return vals;
}

// ---------------------------------------------------------------------------
// NativeHookConfig（native_hook_config.proto）schema
// ---------------------------------------------------------------------------
enum class Kind {
    kBool,
    kInt,            // int32
    kUint,           // uint32
    kString,
    kRepeatedInt,    // packed repeated int32
    kRepeatedEnum,   // packed repeated enum
    kRepeatedString, // repeated string
};

struct SchemaEntry {
    uint32_t number;
    const char* name;
    Kind kind;
};

const SchemaEntry kConfigSchema[] = {
    {1,  "pid",                           Kind::kInt},
    {2,  "save_file",                     Kind::kBool},
    {3,  "file_name",                     Kind::kString},
    {4,  "filter_size",                   Kind::kInt},
    {5,  "smb_pages",                     Kind::kInt},
    {6,  "max_stack_depth",               Kind::kInt},
    {7,  "process_name",                  Kind::kString},
    {8,  "malloc_disable",                Kind::kBool},
    {9,  "mmap_disable",                  Kind::kBool},
    {10, "free_stack_report",             Kind::kBool},
    {11, "munmap_stack_report",           Kind::kBool},
    {12, "malloc_free_matching_interval", Kind::kUint},
    {13, "malloc_free_matching_cnt",      Kind::kUint},
    {14, "string_compressed",             Kind::kBool},
    {15, "fp_unwind",                     Kind::kBool},
    {16, "blocked",                       Kind::kBool},
    {17, "record_accurately",             Kind::kBool},
    {18, "startup_mode",                  Kind::kBool},
    {19, "memtrace_enable",               Kind::kBool},
    {20, "offline_symbolization",         Kind::kBool},
    {21, "callframe_compress",            Kind::kBool},
    {22, "statistics_interval",           Kind::kUint},
    {23, "clock",                         Kind::kString},
    {24, "sample_interval",               Kind::kUint},
    {25, "response_library_mode",         Kind::kBool},
    {26, "expand_pids",                   Kind::kRepeatedInt},
    {27, "js_stack_report",               Kind::kInt},
    {28, "max_js_stack_depth",            Kind::kUint},
    {29, "filter_napi_name",              Kind::kString},
    {30, "dump_nmd",                      Kind::kBool},
    {31, "target_so_name",                Kind::kString},
    {32, "restrace_tag",                  Kind::kRepeatedString},
    {33, "async_stack_enable",            Kind::kBool},
    {34, "async_type",                    Kind::kRepeatedEnum},
    {35, "unique_stack_table_size",       Kind::kUint},
    {36, "use_file_cache_mode",           Kind::kBool},
    {37, "file_cache_buffer_size",        Kind::kUint},
    {38, "allocator_sampling_enable",     Kind::kBool},
    {39, "native_orphansnapshots_enable", Kind::kBool},
    {40, "nested_async_stack_enable",     Kind::kBool},
    {41, "filter_lib_list",               Kind::kRepeatedString},
    {42, "async_nesting_depth",           Kind::kUint},
    {43, "async_task_stack_depth",        Kind::kUint},
    {44, "arkui_mem_trace_enable",        Kind::kBool},
    {45, "discard_destroyed_traces",      Kind::kBool},
};

Kind SchemaKind(uint32_t number)
{
    for (const auto& e : kConfigSchema) {
        if (e.number == number) {
            return e.kind;
        }
    }
    return Kind::kString; // 占位
}

bool InSchema(uint32_t number)
{
    for (const auto& e : kConfigSchema) {
        if (e.number == number) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// ProfilerPluginData 信封（common_types.proto）
// name=1(string) data=3(bytes) 其余字段跳过
// ---------------------------------------------------------------------------
struct PluginData {
    std::string name;
    std::string data;
};

bool DecodePlugin(const uint8_t* buf, size_t size, PluginData& out)
{
    std::vector<Field> fields;
    try {
        fields = ParseFields(buf, size);
    } catch (...) {
        return false;
    }
    for (const auto& f : fields) {
        if (f.number == 1) {
            out.name = f.bytes;
        } else if (f.number == 3) {
            out.data = f.bytes;
        }
    }
    return true;
}

// 把 NativeHookConfig 字节解析进完整结构体
bool DecodeConfig(const std::string& data, HtraceNativeHookConfig& out)
{
    std::vector<Field> fields;
    try {
        fields = ParseFields(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    } catch (...) {
        return false;
    }

    std::map<uint32_t, const Field*> single;
    std::map<uint32_t, std::vector<std::string>> repeatedStrs;
    std::map<uint32_t, std::vector<std::string>> repeatedPacked;

    for (const auto& f : fields) {
        if (!InSchema(f.number)) {
            continue;
        }
        Kind kind = SchemaKind(f.number);
        if (kind == Kind::kRepeatedString) {
            repeatedStrs[f.number].push_back(f.bytes);
        } else if (kind == Kind::kRepeatedInt || kind == Kind::kRepeatedEnum) {
            repeatedPacked[f.number].push_back(f.bytes);
        } else {
            single[f.number] = &f;
        }
    }

    auto asBool = [&](uint32_t n) {
        auto it = single.find(n);
        return it != single.end() && it->second->varint != 0;
    };
    auto asInt = [&](uint32_t n) {
        auto it = single.find(n);
        return it != single.end() ? AsInt32(it->second->varint) : 0;
    };
    auto asUint = [&](uint32_t n) {
        auto it = single.find(n);
        return it != single.end() ? static_cast<uint32_t>(it->second->varint) : 0u;
    };
    auto asStr = [&](uint32_t n) {
        auto it = single.find(n);
        return it != single.end() ? it->second->bytes : std::string();
    };
    auto asRepeatedInt = [&](uint32_t n) {
        std::vector<int32_t> r;
        auto it = repeatedPacked.find(n);
        if (it != repeatedPacked.end())
            for (const auto& blob : it->second)
                for (uint64_t v : UnpackPackedVarints(blob)) {
                    r.push_back(AsInt32(v));
                }
        return r;
    };
    auto asRepeatedEnum = [&](uint32_t n) {
        std::vector<uint32_t> r;
        auto it = repeatedPacked.find(n);
        if (it != repeatedPacked.end())
            for (const auto& blob : it->second)
                for (uint64_t v : UnpackPackedVarints(blob)) {
                    r.push_back(static_cast<uint32_t>(v));
                }
        return r;
    };
    auto asRepeatedStr = [&](uint32_t n) {
        auto it = repeatedStrs.find(n);
        return it != repeatedStrs.end() ? it->second : std::vector<std::string>();
    };

    out.pid = asInt(1);
    out.saveFile = asBool(2);
    out.fileName = asStr(3);
    out.filterSize = asInt(4);
    out.smbPages = asInt(5);
    out.maxStackDepth = asInt(6);
    out.processName = asStr(7);
    out.mallocDisable = asBool(8);
    out.mmapDisable = asBool(9);
    out.freeStackReport = asBool(10);
    out.munmapStackReport = asBool(11);
    out.mallocFreeMatchingInterval = asUint(12);
    out.mallocFreeMatchingCnt = asUint(13);
    out.stringCompressed = asBool(14);
    out.fpUnwind = asBool(15);
    out.blocked = asBool(16);
    out.recordAccurately = asBool(17);
    out.startupMode = asBool(18);
    out.memtraceEnable = asBool(19);
    out.offlineSymbolization = asBool(20);
    out.callframeCompress = asBool(21);
    out.statisticsInterval = asUint(22);
    out.clock = asStr(23);
    out.sampleInterval = asUint(24);
    out.responseLibraryMode = asBool(25);
    out.expandPids = asRepeatedInt(26);
    out.jsStackReport = asInt(27);
    out.maxJsStackDepth = asUint(28);
    out.filterNapiName = asStr(29);
    out.dumpNmd = asBool(30);
    out.targetSoName = asStr(31);
    out.restraceTag = asRepeatedStr(32);
    out.asyncStackEnable = asBool(33);
    out.asyncType = asRepeatedEnum(34);
    out.uniqueStackTableSize = asUint(35);
    out.useFileCacheMode = asBool(36);
    out.fileCacheBufferSize = asUint(37);
    out.allocatorSamplingEnable = asBool(38);
    out.nativeOrphansnapshotsEnable = asBool(39);
    out.nestedAsyncStackEnable = asBool(40);
    out.filterLibList = asRepeatedStr(41);
    out.asyncNestingDepth = asUint(42);
    out.asyncTaskStackDepth = asUint(43);
    out.arkuiMemTraceEnable = asBool(44);
    out.discardDestroyedTraces = asBool(45);

    return true;
}

// 读 1024 字节文件头；成功返回 true，valid 表示魔数是否合法。
bool ReadHeader(const std::string& path, HtraceHeaderInfo& out)
{
    out = HtraceHeaderInfo{};

    std::ifstream ifs;
    try {
        ifs = OpenIfstream(path, std::ios::binary);
    } catch (...) {
        return false;
    }
    if (!ifs) {
        return false;
    }

    uint8_t header[kHeaderSize];
    ifs.read(reinterpret_cast<char*>(header), static_cast<std::streamsize>(kHeaderSize));
    if (ifs.gcount() != static_cast<std::streamsize>(kHeaderSize)) {
        return false;
    }

    out.magic = ReadU64(header + 0);
    out.valid = (out.magic == kHeaderMagic);
    out.length = ReadU64(header + 8);
    out.version = ReadU32(header + 16);
    out.dataType = ReadU32(header + 56);
    out.boottimeNs = ReadU64(header + 60);
    out.realtimeNs = ReadU64(header + 68);
    out.durationNs = ReadU64(header + 244);
    return true;
}

} // namespace

// ============================================================================
//  HtraceReader 公共 API
// ============================================================================

bool HtraceReader::IsProfilerFile(const std::string& path)
{
    HtraceHeaderInfo h;
    if (!ReadHeader(path, h)) {
        log::Logd("[htrace] header read failed: ", path);
        return false;
    }
    const bool ok = h.valid && h.dataType == static_cast<uint32_t>(HtraceDataType::ProtobufBin);
    log::Logd("[htrace] header: version=", h.version, " data_type=", h.dataType,
        " -> ", (ok ? "profiler" : "non-profiler"));
    return ok;
}

bool HtraceReader::ReadNativehookConfig(const std::string& path, HtraceNativeHookConfig& out)
{
    out = HtraceNativeHookConfig{};

    HtraceHeaderInfo h;
    if (!ReadHeader(path, h) || !h.valid) {
        return false;
    }

    std::ifstream ifs;
    try {
        ifs = OpenIfstream(path, std::ios::binary);
    } catch (...) {
        return false;
    }
    if (!ifs) {
        return false;
    }

    ifs.seekg(static_cast<std::streamoff>(kHeaderSize));

    // 记录文件末尾位置，用于校验每个 length-delimited 字段的长度上界。
    const std::streampos fileEnd = ifs.seekg(0, std::ios::end).tellg();
    ifs.seekg(static_cast<std::streamoff>(kHeaderSize));

    while (true) {
        uint8_t lenBuf[4];
        ifs.read(reinterpret_cast<char*>(lenBuf), 4);
        if (ifs.gcount() != 4) {
            break;
        }
        uint32_t len = ReadU32(lenBuf);
        if (len == 0) {
            break;
        }

        // 边界校验：len（不可信 uint32，可达 4GB）不能超过文件剩余字节数，
        // 否则为损坏/恶意数据，避免 std::vector<char> payload(len) 超大分配崩溃。
        const std::streamoff remaining = fileEnd - ifs.tellg();
        if (remaining < 0 ||
            static_cast<std::uint64_t>(len) > static_cast<std::uint64_t>(remaining)) {
            break;
        }

        std::vector<char> payload(len);
        ifs.read(payload.data(), static_cast<std::streamsize>(len));
        if (ifs.gcount() != static_cast<std::streamsize>(len)) {
            break;
        }

        PluginData pd;
        if (!DecodePlugin(reinterpret_cast<const uint8_t*>(payload.data()), payload.size(), pd)) {
            break; // 解析失败，后续数据无法对齐，停止扫描
        }
        if (pd.name == "nativehook_config") {
            out.present = true;
            DecodeConfig(pd.data, out);
            return true;
        }
    }
    return false;
}

std::string HtraceReader::ReadProcessName(const std::string& path)
{
    HtraceNativeHookConfig cfg;
    if (!ReadNativehookConfig(path, cfg)) {
        return "";
    }
    return cfg.processName;
}

} // namespace mem_analyzer