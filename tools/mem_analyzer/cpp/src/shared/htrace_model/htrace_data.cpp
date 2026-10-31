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
#include "shared/htrace_model/htrace_data.h"

namespace mem_analyzer {

namespace {
constexpr int HEX_BUF_SIZE = 32;
}

// ============================================================================
//  HtraceEventType name/enum conversion
// ============================================================================

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

// ============================================================================
//  CompressedStackFrame → ResolvedStackFrame
// ============================================================================

ResolvedStackFrame CompressedStackFrame::Resolve(const DataDict& dict) const
{
    ResolvedStackFrame r;
    r.depth = depth;

    // Resolve symbol name
    auto symIt = dict.find(symbolId);
    r.symbol = (symIt != dict.end()) ? symIt->second : "unknown";

    // Resolve library name
    if (fileIdNull) {
        r.lib = "unknown";
    } else {
        auto libIt = dict.find(fileId);
        r.lib = (libIt != dict.end()) ? libIt->second : "unknown";
    }

    // Build library offset string
    if (!vaddrNull) {
        char buf[HEX_BUF_SIZE];
        int ret = snprintf(buf, sizeof(buf), "0x%llx",
                           static_cast<unsigned long long>(vaddr));
        if (ret < 0) buf[0] = '\0';
        r.libOffset = buf;
    }

    // Resolve build ID
    if (!buildIdNull) {
        auto bidIt = dict.find(buildId);
        if (bidIt != dict.end()) {
            r.buildId = bidIt->second;
        }
    }

    return r;
}

// ============================================================================
//  CompressedCallStack → ResolvedCallStack
// ============================================================================

ResolvedCallStack CompressedCallStack::Resolve(const DataDict& dict) const
{
    ResolvedCallStack r;
    r.id = id;
    r.type = type;
    r.existingSize = existingSize;
    r.existingCount = existingCount;
    r.subTypeId = subTypeId;
    r.frames.reserve(frames.size());
    for (const auto& f : frames) {
        r.frames.push_back(f.Resolve(dict));
    }
    return r;
}

// ============================================================================
//  CompressedCallStackList → ResolvedCallStackList
// ============================================================================

ResolvedCallStackList CompressedCallStackList::Resolve() const
{
    ResolvedCallStackList result;
    result.stacks.reserve(stacks.size());
    for (const auto& cs : stacks) {
        result.stacks.push_back(cs.Resolve(dict));
    }
    return result;
}

// ============================================================================
//  TraceDB
// ============================================================================

bool TraceDB::IsCompressed() const
{
    return std::holds_alternative<CompressedCallStackList>(stacks);
}

size_t TraceDB::StackCount() const
{
    if (auto* v = std::get_if<CompressedCallStackList>(&stacks)) {
        return v->stacks.size();
    }
    if (auto* v = std::get_if<ResolvedCallStackList>(&stacks)) {
        return v->stacks.size();
    }
    return 0;
}

void TraceDB::ResolveStacks()
{
    auto* compressed = std::get_if<CompressedCallStackList>(&stacks);
    if (!compressed) { // already resolved, no-op
        return;
    }

    stacks = compressed->Resolve();
}

} // namespace mem_analyzer
