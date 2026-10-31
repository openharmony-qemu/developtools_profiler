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
#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <variant>

namespace mem_analyzer {

/// Symbol resolution dictionary: int ID → string.
/// Used for lazy resolution of symbol/file/build IDs in stack frames.
using DataDict = std::unordered_map<int, std::string>;

// ============================================================================
//  HtraceEventType — core event type enum (used by htrace pipeline)
// ============================================================================

/// Enum for htrace event types.
/// Maps to the "type" field in native_hook_statistic and event_type pairs in native_hook.
enum class HtraceEventType : int {
    UNKNOWN = -1,
    MALLOC = 0,
    MMAP_ANON = 1,
    FILE_PAGE_MSG = 2,
    MEMORY_USING_MSG = 3,
    HP_FD = 4,
    HP_THREAD = 5,
    GPU_VK = 6,
    GPU_GLES = 7,
    GPU_CL = 8,
    HP_SO = 9,
    ARKTS_HEAP = 10,
    JS_HEAP = 11,
    KMP_HEAP = 12,
    RN_HERMES_HEAP = 13,
    ASHMEM = 14,
    HP_DMA = 15,
    OTHER = 16,
    ARK_GLOBAL_HANDLE = 17,
    ARK_LOCAL_HANDLE = 18,
    VMA_ARKWEB = 19,
    DART_HEAP = 20,
    ARKTS_STATIC_HEAP = 21,
};

/** Returns the human-readable name for an HtraceEventType. */
const char* HtraceEventTypeName(HtraceEventType type)
{
    switch (type) {
        case HtraceEventType::UNKNOWN: return "unknown";
        case HtraceEventType::MALLOC: return "malloc";
        case HtraceEventType::MMAP_ANON: return "mmap-anon";
        case HtraceEventType::FILE_PAGE_MSG: return "mmap-filepage";
        case HtraceEventType::MEMORY_USING_MSG: return "memory_using_msg";
        case HtraceEventType::HP_FD: return "fd";
        case HtraceEventType::HP_THREAD: return "thread";
        case HtraceEventType::GPU_VK: return "gpu_vk";
        case HtraceEventType::GPU_GLES: return "gpu_gles";
        case HtraceEventType::GPU_CL: return "gpu_cl";
        case HtraceEventType::HP_SO: return "so";
        case HtraceEventType::ARKTS_HEAP: return "arkts_heap";
        case HtraceEventType::JS_HEAP: return "js_heap";
        case HtraceEventType::KMP_HEAP: return "kmp_heap";
        case HtraceEventType::RN_HERMES_HEAP: return "rn_herm_heap";
        case HtraceEventType::ASHMEM: return "ashmem";
        case HtraceEventType::HP_DMA: return "dma";
        case HtraceEventType::OTHER: return "other";
        case HtraceEventType::ARK_GLOBAL_HANDLE: return "ark_global_handle";
        case HtraceEventType::ARK_LOCAL_HANDLE: return "ark_local_handle";
        case HtraceEventType::VMA_ARKWEB: return "vma_arkweb";
        case HtraceEventType::DART_HEAP: return "dart_heap";
        case HtraceEventType::ARKTS_STATIC_HEAP: return "arkts_static_heap";
        default: return "unknown";
    }
}

/** Returns the HtraceEventType value for a name string, or UNKNOWN if not recognized. */
HtraceEventType HtraceEventTypeFromName(const std::string& name)
{
    if (name == "unknown") {
        return HtraceEventType::UNKNOWN;
    }
    if (name == "malloc") {
        return HtraceEventType::MALLOC;
    }
    if (name == "ark_global_handle") {
        return HtraceEventType::ARK_GLOBAL_HANDLE;
    }
    if (name == "ark_local_handle") {
        return HtraceEventType::ARK_LOCAL_HANDLE;
    }
    if (name == "dma") {
        return HtraceEventType::HP_DMA;
    }
    if (name == "mmap-anon") {
        return HtraceEventType::MMAP_ANON;
    }
    if (name == "mmap-filepage") {
        return HtraceEventType::FILE_PAGE_MSG;
    }
    if (name == "ashmem") {
        return HtraceEventType::ASHMEM;
    }
    if (name == "fd") {
        return HtraceEventType::HP_FD;
    }
    if (name == "thread") {
        return HtraceEventType::HP_THREAD;
    }
    return HtraceEventType::UNKNOWN;
}

/** detail 模式下 data_dict 的 sub_type 值可能带 "FilePage:" 前缀，输出时剥离。 */
inline std::string NormalizeFilepageFilename(std::string name, const std::string& mode)
{
    if (mode == "detail") {
        const std::string prefix = "FilePage:";
        if (name.compare(0, prefix.size(), prefix) == 0) {
            name.erase(0, prefix.size());
        }
    }
    return name;
}

// ============================================================================
//  StackFrame hierarchy
// ============================================================================

/// Base class for stack frames.
/// Records depth and whether the frame is in resolved (string) or
/// compressed (int ID) form.
struct StackFrame {
    int depth = 0;
    bool resolved = false;

    virtual ~StackFrame() = default;

    bool IsResolved() const { return resolved; }
};

/// Frame with fully resolved string fields.
/// Used by: tshandle parser (pre-resolved), or after resolve() from CompressedStackFrame.
struct ResolvedStackFrame : StackFrame {
    std::string symbol;       // function / symbol name (with offset)
    std::string lib;          // library / file path
    std::string libOffset;   // offset within library (hex string, e.g. "0x1234")
    std::string buildId;     // library build ID

    ResolvedStackFrame() { resolved = true; }
};

/// Frame with compressed int-ID representation for lazy resolution.
/// Used by: htrace parser (avoids string allocation for large datasets).
/// Flame/icicle graph construction uses int IDs for memory-efficient O(1) lookup.
struct CompressedStackFrame : StackFrame {
    int symbolId = 0;
    int fileId = 0;
    bool fileIdNull = false;
    uint64_t vaddr = 0;
    bool vaddrNull = false;
    int buildId = 0;
    bool buildIdNull = true;

    CompressedStackFrame() { resolved = false; }

    /// Convert this frame to resolved form using a DataDict.
    ResolvedStackFrame Resolve(const DataDict& dict) const;
};

// ============================================================================
//  CallStack hierarchy — homogeneous frame type per instance
// ============================================================================

/// Common fields shared by all call stack types.
struct CallStack {
    long long id = 0;
    HtraceEventType type = HtraceEventType::UNKNOWN;
    long long existingSize = 0;       // original allocation size (bytes)
    long long existingCount = 0;
    double relatedSize = 0.0;         // associated retained/shared size (tshandle: ArkTS retained_size)
    int subTypeId = -1;              // data_dict id → 文件名（mmap-filepage 非 NULL，其余 -1）
    std::shared_ptr<std::vector<std::string>> existingAddrs;  // tshandle: address list for ArkTS node mapping
};

/// Call stack with fully resolved string frames.
/// Used by: tshandle pipeline, or after resolve() from CompressedCallStack.
struct ResolvedCallStack : CallStack {
    std::vector<ResolvedStackFrame> frames;
};

/// Call stack with compressed int-ID frames for lazy resolution.
/// Used by: htrace pipeline (avoids string allocation before analysis).
struct CompressedCallStack : CallStack {
    std::vector<CompressedStackFrame> frames;

    /// Convert all frames to resolved form using a DataDict.
    ResolvedCallStack Resolve(const DataDict& dict) const;
};

// ============================================================================
//  TraceMeta — trace-level metadata (common to htrace and tshandle)
// ============================================================================

/// Trace-level metadata common to all pipeline types.
/// Replaces the ad-hoc TraceRange / TshandleTraceMeta structures.
struct TraceMeta {
    // ── Time range ──
    long long startTimeMs = 0;   // trace start, millisecond timestamp
    long long endTimeMs = 0;     // trace end, millisecond timestamp
    long long durationNs = 0;     // trace duration in nanoseconds

    // ── Process ──
    std::string processName;

    // ── htrace-specific ──
    std::string mode;              // "statistic" or "detail" (htrace only)
    std::unordered_map<HtraceEventType, long long> typeSizes;  // HtraceEventType → accumulated size
};

// ============================================================================
//  TraceDB — parsed trace database abstraction
// ============================================================================

/// Compressed call stacks with their resolution dictionary.
/// DataDict is bundled here — only the compressed form needs it.
struct CompressedCallStackList {
    DataDict dict;
    std::vector<CompressedCallStack> stacks;

    /// Resolve all stacks to resolved form using the embedded DataDict.
    struct ResolvedCallStackList Resolve() const;
};

/// Resolved call stacks — strings are already populated, no DataDict needed.
struct ResolvedCallStackList {
    std::vector<ResolvedCallStack> stacks;
};

/// List of call stacks, either compressed (+DataDict) or resolved (no DataDict).
using CallStackList = std::variant<CompressedCallStackList, ResolvedCallStackList>;

/// Abstract representation of a parsed trace database.
/// Contains:
///   - stacks : call stacks — compressed with DataDict (htrace) or resolved (tshandle)
///   - meta   : trace-level metadata (time range, process name, mode)
/// This is the output of the parsing stage — agnostic to htrace vs tshandle.
/// Downstream analyzers and formatters consume TraceDB uniformly.
struct TraceDB {
    CallStackList stacks;
    TraceMeta meta;

    // ── Queries ──

    /// True if this DB contains compressed (int-ID) call stacks.
    bool IsCompressed() const;

    /// Total number of call stacks.
    size_t StackCount() const;

    // ── Resolution ──

    /// If compressed, resolve all stacks in-place and replace with resolved form.
    /// No-op if already resolved.
    void ResolveStacks();
};

} // namespace mem_analyzer
