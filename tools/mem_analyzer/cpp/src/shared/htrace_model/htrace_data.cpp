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
        char buf[32];
        snprintf(buf, sizeof(buf), "0x%llx", static_cast<unsigned long long>(vaddr));
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
    if (!compressed) {
        return;
    }// already resolved, no-op

    stacks = compressed->Resolve();
}

} // namespace mem_analyzer
