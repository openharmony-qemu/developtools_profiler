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
#include "htrace/leak_filter_analyzer.hpp"
#include "shared/htrace_model/htrace_data.hpp"
#include "htrace/htrace_analyzer.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

namespace mem_analyzer {

namespace {

// hex 字符串 "0x5c843f8b00" -> uint64（addr 是绝对地址）
uint64_t ParseHexAddr(const std::string& s)
{
    return static_cast<uint64_t>(std::strtoull(s.c_str(), nullptr, 16));
}

// L1: 调用栈关联的分配地址（existing_addrs，hex 串）与 leaks 集合求交集，非空才留
bool HasLeakAddr(const CompressedCallStack& cs, const std::unordered_set<uint64_t>& leaks)
{
    if (leaks.empty()) {
        return false;
    }
    if (!cs.existingAddrs || cs.existingAddrs->empty()) {
        return false;
    }
    for (const auto& a : *cs.existingAddrs) {
        if (leaks.count(ParseHexAddr(a)) > 0) {
            return true;
        }
    }
    return false;
}

// L3: 至少一帧 lib 含 data/storage/el 且 .so，且不含 libarkweb_engine.so
// （arkweb 例外：arkweb 虽在沙箱目录但归 system，不算应用 so 帧）
bool HasAppSoFrame(const CompressedCallStack& cs, const std::unordered_map<int, std::string>& dict)
{
    for (const auto& f : cs.frames) {
        if (f.fileIdNull) {
            continue;
        }
        auto it = dict.find(f.fileId);
        if (it == dict.end()) {
            continue;
        }
        const std::string& lib = it->second;
        if (lib.find("data/storage/el") != std::string::npos &&
            lib.find("libarkweb_engine.so") == std::string::npos &&
            lib.size() >= 3 &&
            lib.compare(lib.size() - 3, 3, ".so") == 0) {
            return true;
        }
    }
    return false;
}

// L2: 任意帧 symbol 含 "__emutls_get_address" → 该栈排除
bool HasEmutlsFrame(const CompressedCallStack& cs, const std::unordered_map<int, std::string>& dict) {
    for (const auto& f : cs.frames) {
        auto it = dict.find(f.symbolId);
        if (it == dict.end()) {
            continue;
        }
        if (it->second.find("__emutls_get_address") != std::string::npos) {
            return true;
        }
    }
    return false;
}

} // namespace

// L1: 按 Native Leaks 不可达内存泄漏文件地址过滤（addr 匹配）。parse 后调用，解析/分析模式均执行。
void ApplyLeakAddrFilter(TraceDB& db, const std::unordered_set<uint64_t>& leaks)
{
    if (leaks.empty()) {
        return;
    }
    auto* clist = std::get_if<CompressedCallStackList>(&db.stacks);
    if (!clist) {
        return;
    }

    size_t total = clist->stacks.size();
    std::vector<CompressedCallStack> kept;
    kept.reserve(total);
    for (auto& cs : clist->stacks) {
        if (HasLeakAddr(cs, leaks)) {
            kept.push_back(std::move(cs));
        }
    }
    size_t removed = total - kept.size();
    clist->stacks = std::move(kept);

    log::Logok("  Leak L1: kept ", clist->stacks.size(),
              " / ", total, " stacks (", removed, " filtered by addr).");
}

// -l 分析模式（不带 -p）：
//   L2 __emutls_get_address 排除 → L3 栈分 app/system（arkweb 例外）→
//   按 scope 走 analyze_call_stacks(leak_mode)：
//     app-only/system-only 各跑一遍；app-and-system 跑两遍（app+system）；mixed 不分栈跑一遍。
//   每组 top N + other（非 top N 覆盖的栈合并火焰图）。
//   组装 HtraceAnalysisOutput，from=htrace-analysis。L1 已由 apply_leak_addr_filter 完成。
HtraceAnalysisOutput HtraceLeakAnalyze(TraceDB& db, const RunContext& ctx)
{
    auto* clist = std::get_if<CompressedCallStackList>(&db.stacks);
    if (!clist) {
        throw std::runtime_error("htrace_leak_analyze: expected CompressedCallStackList in TraceDB");
    }

    Scope scope = ctx.htrace.scope;
    std::string scopeStr = ScopeToString(scope);

    AnalysisConfig acfg;
    acfg.leakMode = true;
    acfg.useForceScope = true;
    acfg.topN = ctx.htrace.topN;
    acfg.scope = scope;
    acfg.category = ctx.htrace.category;

    // L2: __emutls_get_address 帧排除
    size_t total = clist->stacks.size();
    std::vector<CompressedCallStack> kept;
    kept.reserve(total);
    for (auto& cs : clist->stacks) {
        if (HasEmutlsFrame(cs, clist->dict)) {
            continue;
        }
        kept.push_back(std::move(cs));
    }
    size_t l2Removed = total - kept.size();
    clist->stacks = std::move(kept);
    log::Logok("  Leak L2: kept ", clist->stacks.size(),
              " / ", total, " stacks (", l2Removed, " filtered by emutls).");

    // L3: 栈分 app/system（mixed 不分，直接用全部栈）
    std::vector<CompressedCallStack> appStacks, systemStacks;
    if (scope != Scope::Mixed) {
        for (auto& cs : clist->stacks) {
            if (HasAppSoFrame(cs, clist->dict)) {
                appStacks.push_back(std::move(cs));
            } else {
                systemStacks.push_back(std::move(cs));
            }
        }
        log::Logok("  Leak L3: app=", appStacks.size(), ", system=", systemStacks.size());
    }

    HtraceAnalysisOutput output;
    if (scopeStr == "app-only") {
        acfg.forceScope = Scope::AppOnly;
        output = AnalyzeCallStacks(appStacks, clist->dict, acfg);
    } else if (scopeStr == "system-only") {
        acfg.forceScope = Scope::SystemOnly;
        output = AnalyzeCallStacks(systemStacks, clist->dict, acfg);
    } else if (scopeStr == "mixed") {
        acfg.forceScope = Scope::Mixed;
        output = AnalyzeCallStacks(clist->stacks, clist->dict, acfg);
    } else {  // app-and-system（默认）：走两遍
        acfg.forceScope = Scope::AppOnly;
        auto appOut = AnalyzeCallStacks(appStacks, clist->dict, acfg);
        acfg.forceScope = Scope::SystemOnly;
        auto sysOut = AnalyzeCallStacks(systemStacks, clist->dict, acfg);
        output.topSymbols.mode = "app-and-system";
        output.topSymbols.app = std::move(appOut.topSymbols.app);
        output.topSymbols.system = std::move(sysOut.topSymbols.system);
        output.totalLeakSize = appOut.totalLeakSize + sysOut.totalLeakSize;
        output.stats.orgSetSize = appOut.stats.orgSetSize + sysOut.stats.orgSetSize;
        output.stats.filteredSetSize = appOut.stats.filteredSetSize + sysOut.stats.filteredSetSize;
        output.stats.finalSetSize = appOut.stats.finalSetSize + sysOut.stats.finalSetSize;
        output.stats.equivalentGroups = appOut.stats.equivalentGroups + sysOut.stats.equivalentGroups;
        output.stats.upApproxRemoved = appOut.stats.upApproxRemoved + sysOut.stats.upApproxRemoved;
        output.stats.downApproxRemoved = appOut.stats.downApproxRemoved + sysOut.stats.downApproxRemoved;
        output.stats.representationAdded = appOut.stats.representationAdded + sysOut.stats.representationAdded;
    }

    // 填 metadata（照 htrace_analyze）
    output.dataDict = std::move(clist->dict);
    output.mode = std::move(db.meta.mode);
    output.processName = std::move(db.meta.processName);
    output.typeSizes = std::move(db.meta.typeSizes);
    output.traceMeta = std::move(db.meta);

    int totalTop = (int)(output.topSymbols.app.SymbolCount() +
                        output.topSymbols.system.SymbolCount() +
                        output.topSymbols.mixed.SymbolCount());
    log::Logok("  Leak analysis: ", totalTop, " top symbols (incl other).");

    return output;
}

} // namespace mem_analyzer
