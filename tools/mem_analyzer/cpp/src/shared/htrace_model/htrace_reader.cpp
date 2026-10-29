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

constexpr size_t OFFSET_LENGTH = 8;
constexpr size_t OFFSET_VERSION = 16;
constexpr size_t OFFSET_DATA_TYPE = 56;
constexpr size_t OFFSET_BOOTTIME_NS = 60;
constexpr size_t OFFSET_REALTIME_NS = 68;
constexpr size_t OFFSET_DURATION_NS = 244;
constexpr size_t LEN_PREFIX_SIZE = 4;
constexpr int VARINT_MAX_SHIFT = 64;

enum class HtraceDataType : uint32_t {
    PROTOBUF_BIN = 0,    // HIPROFILER_PROTOBUF_BIN，标准 HiProfiler protobuf 二进制
    HIPERF      = 1,    // hiperf 采样原始数据
    STANDALONE  = 1000, // 独立插件文件
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
        if (shift >= VARINT_MAX_SHIFT) {
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
    K_BOOL,
    K_INT,            // int32
    K_UINT,           // uint32
    K_STRING,
    K_REPEATED_INT,    // packed repeated int32
    K_REPEATED_ENUM,   // packed repeated enum
    K_REPEATED_STRING, // repeated string
};

struct SchemaEntry {
    uint32_t number;
    const char* name;
    Kind kind;
};

const SchemaEntry kConfigSchema[] = {
    {1,  "pid",                           Kind::K_INT},
    {2,  "save_file",                     Kind::K_BOOL},
    {3,  "file_name",                     Kind::K_STRING},
    {4,  "filter_size",                   Kind::K_INT},
    {5,  "smb_pages",                     Kind::K_INT},
    {6,  "max_stack_depth",               Kind::K_INT},
    {7,  "process_name",                  Kind::K_STRING},
    {8,  "malloc_disable",                Kind::K_BOOL},
    {9,  "mmap_disable",                  Kind::K_BOOL},
    {10, "free_stack_report",             Kind::K_BOOL},
    {11, "munmap_stack_report",           Kind::K_BOOL},
    {12, "malloc_free_matching_interval", Kind::K_UINT},
    {13, "malloc_free_matching_cnt",      Kind::K_UINT},
    {14, "string_compressed",             Kind::K_BOOL},
    {15, "fp_unwind",                     Kind::K_BOOL},
    {16, "blocked",                       Kind::K_BOOL},
    {17, "record_accurately",             Kind::K_BOOL},
    {18, "startup_mode",                  Kind::K_BOOL},
    {19, "memtrace_enable",               Kind::K_BOOL},
    {20, "offline_symbolization",         Kind::K_BOOL},
    {21, "callframe_compress",            Kind::K_BOOL},
    {22, "statistics_interval",           Kind::K_UINT},
    {23, "clock",                         Kind::K_STRING},
    {24, "sample_interval",               Kind::K_UINT},
    {25, "response_library_mode",         Kind::K_BOOL},
    {26, "expand_pids",                   Kind::K_REPEATED_INT},
    {27, "js_stack_report",               Kind::K_INT},
    {28, "max_js_stack_depth",            Kind::K_UINT},
    {29, "filter_napi_name",              Kind::K_STRING},
    {30, "dump_nmd",                      Kind::K_BOOL},
    {31, "target_so_name",                Kind::K_STRING},
    {32, "restrace_tag",                  Kind::K_REPEATED_STRING},
    {33, "async_stack_enable",            Kind::K_BOOL},
    {34, "async_type",                    Kind::K_REPEATED_ENUM},
    {35, "unique_stack_table_size",       Kind::K_UINT},
    {36, "use_file_cache_mode",           Kind::K_BOOL},
    {37, "file_cache_buffer_size",        Kind::K_UINT},
    {38, "allocator_sampling_enable",     Kind::K_BOOL},
    {39, "native_orphansnapshots_enable", Kind::K_BOOL},
    {40, "nested_async_stack_enable",     Kind::K_BOOL},
    {41, "filter_lib_list",               Kind::K_REPEATED_STRING},
    {42, "async_nesting_depth",           Kind::K_UINT},
    {43, "async_task_stack_depth",        Kind::K_UINT},
    {44, "arkui_mem_trace_enable",        Kind::K_BOOL},
    {45, "discard_destroyed_traces",      Kind::K_BOOL},
};

Kind SchemaKind(uint32_t number)
{
    for (const auto& e : kConfigSchema) {
        if (e.number == number) {
            return e.kind;
        }
    }
    return Kind::K_STRING; // 占位
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
        if (kind == Kind::K_REPEATED_STRING) {
            repeatedStrs[f.number].push_back(f.bytes);
        } else if (kind == Kind::K_REPEATED_INT || kind == Kind::K_REPEATED_ENUM) {
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

    uint32_t index = 0;
    out.pid = asInt(++index);
    out.saveFile = asBool(++index);
    out.fileName = asStr(++index);
    out.filterSize = asInt(++index);
    out.smbPages = asInt(++index);
    out.maxStackDepth = asInt(++index);
    out.processName = asStr(++index);
    out.mallocDisable = asBool(++index);
    out.mmapDisable = asBool(++index);
    out.freeStackReport = asBool(++index);
    out.munmapStackReport = asBool(++index);
    out.mallocFreeMatchingInterval = asUint(++index);
    out.mallocFreeMatchingCnt = asUint(++index);
    out.stringCompressed = asBool(++index);
    out.fpUnwind = asBool(++index);
    out.blocked = asBool(++index);
    out.recordAccurately = asBool(++index);
    out.startupMode = asBool(++index);
    out.memtraceEnable = asBool(++index);
    out.offlineSymbolization = asBool(++index);
    out.callframeCompress = asBool(++index);
    out.statisticsInterval = asUint(++index);
    out.clock = asStr(++index);
    out.sampleInterval = asUint(++index);
    out.responseLibraryMode = asBool(++index);
    out.expandPids = asRepeatedInt(++index);
    out.jsStackReport = asInt(++index);
    out.maxJsStackDepth = asUint(++index);
    out.filterNapiName = asStr(++index);
    out.dumpNmd = asBool(++index);
    out.targetSoName = asStr(++index);
    out.restraceTag = asRepeatedStr(++index);
    out.asyncStackEnable = asBool(++index);
    out.asyncType = asRepeatedEnum(++index);
    out.uniqueStackTableSize = asUint(++index);
    out.useFileCacheMode = asBool(++index);
    out.fileCacheBufferSize = asUint(++index);
    out.allocatorSamplingEnable = asBool(++index);
    out.nativeOrphansnapshotsEnable = asBool(++index);
    out.nestedAsyncStackEnable = asBool(++index);
    out.filterLibList = asRepeatedStr(++index);
    out.asyncNestingDepth = asUint(++index);
    out.asyncTaskStackDepth = asUint(++index);
    out.arkuiMemTraceEnable = asBool(++index);
    out.discardDestroyedTraces = asBool(++index);

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
    out.length = ReadU64(header + OFFSET_LENGTH);
    out.version = ReadU32(header + OFFSET_VERSION);
    out.dataType = ReadU32(header + OFFSET_DATA_TYPE);
    out.boottimeNs = ReadU64(header + OFFSET_BOOTTIME_NS);
    out.realtimeNs = ReadU64(header + OFFSET_REALTIME_NS);
    out.durationNs = ReadU64(header + OFFSET_DURATION_NS);
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
    const bool ok = h.valid && h.dataType == static_cast<uint32_t>(HtraceDataType::PROTOBUF_BIN);
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
        uint8_t lenBuf[LEN_PREFIX_SIZE];
        ifs.read(reinterpret_cast<char*>(lenBuf), static_cast<std::streamsize>(LEN_PREFIX_SIZE));
        if (ifs.gcount() != static_cast<std::streamsize>(LEN_PREFIX_SIZE)) {
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