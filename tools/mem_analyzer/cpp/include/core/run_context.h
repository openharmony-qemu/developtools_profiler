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
#include <unordered_set>

#include "config.h"

namespace mem_analyzer {

// ── 通用（管道层）────────────────────────────────────────────────
struct OutputOptions {
    std::string outDir = defaults::OUTPUT_DIR;  // 输出根目录（process_file 会再嵌一层 source stem）
    std::string format;                          // 空 = 用该解析器的默认格式
    bool parseOnly = false;
    bool autoOpen = false;
};

// ── 解析期需要的外部资源 ────────────────────────────────────────
struct ResourceOptions {
    std::string soDir;          // -S/--so-dir：native 符号表目录
    std::string sourcemapDir;   // --sourcemap：源码映射
    std::string nameCacheDir;  // --namecache：nameCache
};

// ── 各域的算法参数 ──────────────────────────────────────────────
struct HtraceOptions {
    Scope scope = defaults::SCOPE;
    int topN = defaults::TRACE_TOP_N;
    std::string category = defaults::HTRACE_TYPE;
    // -l: Native Leaks 不可达内存泄漏文件地址集合（TraceCommand 读 leak 文件后填入；空 = 非 -l 模式）
    std::unordered_set<uint64_t> leakAddrs;
};

struct TshandleOptions {
    int topN = defaults::TSHANDLE_TOP_N;
};

/**
 * 快照对比运行参数（jsleak 域的对比模式）。baseline_file 非空 = 对比模式：
 * req.file_path 为目标快照 B，本字段为 -b/--baseline 指定的基线快照 A。
 */
struct SnapshotCompareOptions {
    std::string baselineFile;
};

struct SnapshotOptions {
    std::string mode = defaults::JSLEAK_MODE;
    std::string leakListFile;
    int topN = defaults::JSLEAK_TOP_N;
    SnapshotCompareOptions compare;
};

/**
 * 一次运行的显式参数集合，构造后只读 —— 替代原先的 Config 单例。
 *
 * 纪律（P4 之后会进一步收紧）：
 *   1. RunContext 只出现在管道层（process_file / AnalysisModule::run）；
 *   2. 域算法只接收自己那**一个**子结构（HtraceOptions / TshandleOptions /
 *      SnapshotOptions），永远不接收整个 RunContext；
 *   3. 解析期资源（so_dir / sourcemap / namecache）属于「运行环境」，
 *      被多个域的解析器共用，因此放在上下文里而非某个域内。
 */
struct RunContext {
    OutputOptions output;
    ResourceOptions resources;
    std::string commandLine;  // 写入输出 meta.command

    HtraceOptions htrace;
    TshandleOptions tshandle;
    SnapshotOptions snapshot;
};

/**
 * 兜底默认上下文：仅用于 ctx 未被设置的异常路径，避免空指针解引用。
 * 默认值与 Config 的历史默认保持一致。
 */
inline const RunContext& DefaultRunContext()
{
    static const RunContext ctx{};
    return ctx;
}

} // namespace mem_analyzer
