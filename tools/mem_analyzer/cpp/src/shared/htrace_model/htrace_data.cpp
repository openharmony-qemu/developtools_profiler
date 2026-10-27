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
#include "shared/htrace_model/htrace_data.hpp"

namespace mem_analyzer {

// ============================================================================
//  CompressedStackFrame → ResolvedStackFrame
// ============================================================================

ResolvedStackFrame CompressedStackFrame::resolve(const DataDict& dict) const {
    ResolvedStackFrame r;
    r.depth = depth;

    // Resolve symbol name
    auto sym_it = dict.find(symbol_id);
    r.symbol = (sym_it != dict.end()) ? sym_it->second : "unknown";

    // Resolve library name
    if (file_id_null) {
        r.lib = "unknown";
    } else {
        auto lib_it = dict.find(file_id);
        r.lib = (lib_it != dict.end()) ? lib_it->second : "unknown";
    }

    // Build library offset string
    if (!vaddr_null) {
        char buf[32];
        snprintf(buf, sizeof(buf), "0x%llx", static_cast<unsigned long long>(vaddr));
        r.lib_offset = buf;
    }

    // Resolve build ID
    if (!build_id_null) {
        auto bid_it = dict.find(build_id);
        if (bid_it != dict.end()) {
            r.build_id = bid_it->second;
        }
    }

    return r;
}

// ============================================================================
//  CompressedCallStack → ResolvedCallStack
// ============================================================================

ResolvedCallStack CompressedCallStack::resolve(const DataDict& dict) const {
    ResolvedCallStack r;
    r.id = id;
    r.type = type;
    r.existing_size = existing_size;
    r.existing_count = existing_count;
    r.sub_type_id = sub_type_id;
    r.frames.reserve(frames.size());
    for (const auto& f : frames) {
        r.frames.push_back(f.resolve(dict));
    }
    return r;
}

// ============================================================================
//  CompressedCallStackList → ResolvedCallStackList
// ============================================================================

ResolvedCallStackList CompressedCallStackList::resolve() const {
    ResolvedCallStackList result;
    result.stacks.reserve(stacks.size());
    for (const auto& cs : stacks) {
        result.stacks.push_back(cs.resolve(dict));
    }
    return result;
}

// ============================================================================
//  TraceDB
// ============================================================================

bool TraceDB::is_compressed() const {
    return std::holds_alternative<CompressedCallStackList>(stacks);
}

size_t TraceDB::stack_count() const {
    if (auto* v = std::get_if<CompressedCallStackList>(&stacks)) {
        return v->stacks.size();
    }
    if (auto* v = std::get_if<ResolvedCallStackList>(&stacks)) {
        return v->stacks.size();
    }
    return 0;
}

void TraceDB::resolve_stacks() {
    auto* compressed = std::get_if<CompressedCallStackList>(&stacks);
    if (!compressed) return;  // already resolved, no-op

    stacks = compressed->resolve();
}

} // namespace mem_analyzer
