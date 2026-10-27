#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <utility>

#include "nlohmann/json.hpp"
#include "shared/htrace_model/htrace_data.hpp"
using json = nlohmann::ordered_json;

#include <unordered_set>

/**
 * Data structures matching the htrace SQLite schema.
 */
namespace mem_analyzer {

/** Only types in this set are processed; all others are skipped entirely. */
inline const std::unordered_set<HtraceEventType>& htrace_enabled_types() {
    static const std::unordered_set<HtraceEventType> enabled = {
        HtraceEventType::MALLOC,
        HtraceEventType::MMAP_ANON,
        HtraceEventType::FILE_PAGE_MSG,
        HtraceEventType::HP_FD,
        HtraceEventType::HP_THREAD,
        // HtraceEventType::GPU_VK,
        // HtraceEventType::GPU_GLES,
        // HtraceEventType::GPU_CL,
        HtraceEventType::ASHMEM,
        HtraceEventType::HP_DMA,
        HtraceEventType::ARK_GLOBAL_HANDLE,
        HtraceEventType::ARK_LOCAL_HANDLE,
    };
    return enabled;
}

/** Returns true if the type should be processed. */
inline bool htrace_is_enabled_type(HtraceEventType type) {
    return htrace_enabled_types().count(type) > 0;
}

/** Only types in this set have their size counted toward totalExistingSize; others treat size as 0. */
inline const std::unordered_set<HtraceEventType>& htrace_count_size_types() {
    static const std::unordered_set<HtraceEventType> count = {
        HtraceEventType::MALLOC,
        HtraceEventType::MMAP_ANON,
        HtraceEventType::FILE_PAGE_MSG,
        HtraceEventType::HP_FD,
        HtraceEventType::HP_THREAD,
        // HtraceEventType::GPU_VK,
        // HtraceEventType::GPU_GLES,
        // HtraceEventType::GPU_CL,
        HtraceEventType::ASHMEM,
        HtraceEventType::HP_DMA,
        HtraceEventType::ARK_GLOBAL_HANDLE,
        HtraceEventType::ARK_LOCAL_HANDLE,
    };
    return count;
}

/** Returns true if the type's size should be counted toward total. */
inline bool htrace_is_count_size_type(HtraceEventType type) {
    return htrace_count_size_types().count(type) > 0;
}

/**
 * Configurable event_type mapping for detail mode.
 * Maps an alloc event_type string (e.g. "AllocEvent") to an HtraceEventType.
 * Default: "AllocEvent" → MALLOC.
 * Extend to support additional event types (e.g., "MmapEvent" → MMAP_ANON).
 */
enum class SubTypeCondition {
    Any,      // match regardless of sub_type_id
    IsNull,   // match only records where sub_type_id IS NULL (mmap-anon)
    IsNotNull // match only records where sub_type_id IS NOT NULL (mmap-filepage)
};

struct EventTypeMapping {
    std::string alloc_event;
    HtraceEventType htrace_type;
    SubTypeCondition sub_type_cond = SubTypeCondition::Any;
};

/**
 * Returns the default event type configurations.
 * Extend this vector to support additional event types.
 */
inline std::vector<EventTypeMapping> default_event_mappings() {
    return {
        {"AllocEvent", HtraceEventType::MALLOC},
        {"MmapEvent", HtraceEventType::MMAP_ANON, SubTypeCondition::IsNull},      // MAP_ANONYMOUS: sub_type_id IS NULL
        {"MmapEvent", HtraceEventType::FILE_PAGE_MSG, SubTypeCondition::IsNotNull}, // file/page mapping: sub_type_id IS NOT NULL
        {"FD_Open_Event", HtraceEventType::HP_FD},
        {"THREAD_Create_Event", HtraceEventType::HP_THREAD},
        {"ASHMEM_Alloc_Event", HtraceEventType::ASHMEM},
        {"ION_Alloc_Event", HtraceEventType::HP_DMA},
        {"ARK_GLOBAL_HANDLE_Alloc_Event", HtraceEventType::ARK_GLOBAL_HANDLE},
        {"ARK_LOCAL_HANDLE_Alloc_Event", HtraceEventType::ARK_LOCAL_HANDLE},
        // Future: {"MapAllocEvent", HtraceEventType::MAP_ALLOC}, etc.
    };
}

/**
 * Process a trace_streamer-generated SQLite DB file.
 * `category` 决定纳入哪些资源类型（如 "nativeheap"/"fd"）；
 * `filter_category` 为 false 时（parse-only 模式）不做类型过滤，全部启用的类型都纳入。
 */
TraceDB process_htrace_db(const std::string& db_path, const std::string& category,
                          bool filter_category, bool need_addrs = false);

} // namespace mem_analyzer
