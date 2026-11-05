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
#include <unordered_map>
#include <vector>

namespace mem_analyzer {

/**
 * 一条 sourceMap 条目：混淆 key（如 "entry|entry|1.0.0|src/main/ets/pages/a.ts"）对应的源码信息。
 */
struct SourceMapEntry {
    std::string file;                // 编译产物文件（source map "file" 字段）
    std::vector<std::string> sources;  // 真实源码路径列表（source map "sources" 字段，可多文件）
    std::string sourcePath;         // 便捷字段：sources[0]（单文件场景常用；多文件请用 sources[source_index]）
    std::string entryPackageInfo;  // "entry-package-info" 字段
    std::string packageInfo;        // "package-info" 字段
    std::string mappings;            // source map v3 "mappings" 原始串（用于行号还原）
};

struct ResolvedPosition {
    int& originalLine;
    int& originalColumn;
    int* outSourceIndex;
};

/**
 * sourceMap 索引：加载工程编译产物归档目录（sourceMaps.map 及 *.map）并建立
 * 混淆 key → 源码信息 的反查索引，供 heapsnapshot 反混淆使用。
 */
class SourceMapIndex {
public:
    // source map v3 "mappings" 解码结构：行 → 段 → 字段，段字段依次为
    // [generatedColumn, sourceIndex, originalLine, originalColumn, nameIndex?]
    using Segment = std::vector<int>;
    using LineSegments = std::vector<Segment>;
    using DecodedMappings = std::vector<LineSegments>;

    /**
     * 读取目录内 "sourceMaps.map" 及所有 "*.map" 文件合并索引
     * （递归扫描子目录，兼容多模块产物）。
     * 找不到文件或解析失败返回 false（由调用方决定告警策略）。
     */
    bool LoadFromDir(const std::string& dir);

    /**
     * 按混淆 key 精确查找，命中返回指向条目的指针（零拷贝），未命中返回 nullptr。
     */
    const SourceMapEntry* Lookup(const std::string& key) const;

    /**
     * 按 ArkTS 模块三元组（module / version / 相对路径）查找，适配
     * heapsnapshot 中形如 "[bundle/]module@version/src/main/ets/xxx.ts[#func...]"
     * 的字符串：内部将混淆 key 拆解为 "module|package|version|rel_path" 后
     * 以 (module, version, rel_path) 匹配。命中返回指向条目的指针，未命中返回 nullptr。
     */
    const SourceMapEntry* LookupByModule(const std::string& module, const std::string& version,
        const std::string& relPath) const;

    /**
     * 索引是否为空（未加载到任何条目）。
     */
    bool Empty() const;

    /**
     * Base64 VLQ 解码 source map v3 "mappings" 字段（无缓存，供独立解码场景使用）。
     */
    static DecodedMappings DecodeVlqMappings(const std::string& mappings);

    /**
     * 将编译产物行号直接映射为源码行号（均为 1-based，对应官方
     * "异常堆栈解析原理"中 sourceMap 的映射方式）：取目标行（含）之前
     * 最近一个已映射行的首个 segment 的 originalLine。命中返回 true 并填充
     * original_line；无映射或映射无效返回 false。
     * 注意：该函数与 resolve_original_position 同为 sourceMap 直接映射，
     * 区别仅在本函数只做行级映射（无列级细分）。内部按 mappings 内容缓存
     * 解码结果，重复映射同一 mappings 时避免重复解码。
     */
    bool ResolveOriginalLine(const std::string& mappings, int generatedLine, int& originalLine) const;

    /**
     * 将编译产物行列号精确映射为源码行列号（均为 1-based，对应官方
     * "异常堆栈解析原理"中 sourceMap 的列级映射）：
     * 解码 mappings 后取目标行内 generatedColumn <= 目标列的最末一个 segment
     * （该 segment 覆盖目标列），映射其 originalLine/originalColumn。
     * 目标行无覆盖段时，向前回退到最近一个有映射的行取其最末 segment。
     * 命中返回 true 并填充 original_line/original_column；若 out_source_index
     * 非空且命中段含 sourceIndex，同时填充其指向 sources 数组的索引。
     */
    bool ResolveOriginalPosition(const std::string& mappings, int generatedLine,
        int generatedColumn, ResolvedPosition& out) const;

private:
    /**
     * 返回 mappings 的解码结果（内部缓存，引用在缓存存续期间稳定）。
     */
    const DecodedMappings& GetDecodedMappings(const std::string& mappings) const;

    std::unordered_map<std::string, SourceMapEntry> index_;
    // 二级索引：module|version|rel_path → 条目，供 lookup_by_module 快速查找。
    std::unordered_map<std::string, const SourceMapEntry*> moduleIndex;
    // mappings 解码缓存：mappings 原始串 → 解码结果，避免反复解码同一份映射。
    mutable std::unordered_map<std::string, DecodedMappings> decodedCache;
};

} // namespace mem_analyzer
