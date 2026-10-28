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
inline const std::unordered_set<HtraceEventType>& HtraceEnabledTypes()
{
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
inline bool HtraceIsEnabledType(HtraceEventType type)
{
    return HtraceEnabledTypes().count(type) > 0;
}

/** Only types in this set have their size counted toward totalExistingSize; others treat size as 0. */
inline const std::unordered_set<HtraceEventType>& HtraceCountSizeTypes()
{
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
inline bool HtraceIsCountSizeType(HtraceEventType type)
{
    return HtraceCountSizeTypes().count(type) > 0;
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
    std::string allocEvent;
    HtraceEventType htraceType;
    SubTypeCondition subTypeCond = SubTypeCondition::Any;
};

/**
 * Returns the default event type configurations.
 * Extend this vector to support additional event types.
 */
inline std::vector<EventTypeMapping> DefaultEventMappings()
{
    return {
        {"AllocEvent", HtraceEventType::MALLOC},
        {"MmapEvent", HtraceEventType::MMAP_ANON, SubTypeCondition::IsNull},      // MAP_ANONYMOUS: sub_type_id IS NULL
        // file/page mapping: sub_type_id IS NOT NULL
        {"MmapEvent", HtraceEventType::FILE_PAGE_MSG, SubTypeCondition::IsNotNull},
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
TraceDB ProcessHtraceDb(const std::string& dbPath, const std::string& category,
                          bool filterCategory, bool needAddrs = false);

} // namespace mem_analyzer
