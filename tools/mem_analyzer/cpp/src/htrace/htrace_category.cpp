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
#include "htrace/htrace_category.hpp"

#include <stdexcept>

namespace mem_analyzer {

const std::vector<HtraceCategorySpec>& HtraceCategories()
{
    // ─────────────────────────────────────────────────────────────────────────
    // 描述符表：全仓唯一一处枚举「有哪些 category、各自长什么样」的地方。
    // 一行 = 一个 HtraceCategorySpec，字段顺序（与头文件一致）：
    //   name, event_types(升序), summary_total_key, html_type_class, count_mode,
    //   tracks, summary_entries
    // 各列注意：
    // - name：category 关键词；**行为契约**，改名会改输出/报错文案，回归会挂。
    // - event_types：allowed_event_types(name, true) 的结果，必须升序（SQL 的 type IN(...) 与过滤遍历都依赖）。
    // - summary_total_key / html_type_class：JSON 汇总键 / HTML 类型 CSS 类。
    // - count_mode：fd / thread 置 true（泄漏单元是「个数」而非字节，见头文件字段注释）。
    // - tracks：N 轨声明。单轨用 {{"<key>", "", TrackRender::SYMBOL}}（key 是 JSON 嵌套键；
    //   无 label，因为单轨无 HTML 二级 tab）。多轨（仅 mmap）按渲染顺序列出，label 是 HTML 二级 tab 名的唯一权威来源。
    // - summary_entries：build_summary 的输出条目，顺序即 JSON 键顺序。
    // 表内顺序是有意设计的：它即 CLI 报错「Valid values:」的展示顺序（保持与历史一致：
    //   nativeheap, mmap, dma, ashmem, fd, thread）。
    //   注意 parse summary 的输出顺序与此不同（它在 parse formatter 里单独定序，见
    //   htrace_parse_json_formatter.cpp 的 PARSE_SUMMARY_ORDER），不要把这处顺序当「输出顺序」改。
    // 扩展：新增 category = 在下面加一行；新增轨道 = 改该行 tracks；新增行种 = 改 TrackRender +
    //       writer 分派（详见头文件三个正交维度 + docs/htrace_refactor_design.md）。
    // ─────────────────────────────────────────────────────────────────────────
    static const std::vector<HtraceCategorySpec> specs = {
        // nativeheap：默认类别。主键是「未释放堆块」，按字节计。
        {"nativeheap",
         {HtraceEventType::MALLOC, HtraceEventType::ARK_GLOBAL_HANDLE, HtraceEventType::ARK_LOCAL_HANDLE},
         "existingNativeHeapSize", "htrace-nativeheap", false,
         {{"nativeHeaps", "", TrackRender::SYMBOL}},
         {{"existingNativeHeapSize", {"malloc", "ark_global_handle", "ark_local_handle"}}}},

        // mmap：双轨（匿名映射=符号，文件映射=文件）。count_mode=false（按字节）。
        // summary 有三条：Anon / Filepage 各自一条，Total 一条把前两条的类型名都收进来，
        // 让 writer 复用「两轨各自求和」，不对 Total 单独特判。
        {"mmap",
         {HtraceEventType::MMAP_ANON, HtraceEventType::FILE_PAGE_MSG},
         "existingMmapTotalSize", "htrace-mmap", false,
         {{"mmapAnons", "mmap-anon（匿名映射）", TrackRender::SYMBOL},
          {"mmapFilepages", "mmap-filepage（文件映射）", TrackRender::FILEPAGE}},
         {{"existingMmapAnonSize", {"mmap-anon"}},
          {"existingMmapFilepageSize", {"mmap-filepage"}},
          {"existingMmapTotalSize", {"mmap-anon", "mmap-filepage"}}}},

        // dma / ashmem：单轨符号，按字节。各只有一条汇总。
        {"dma",
         {HtraceEventType::HP_DMA}, "existingDMASize", "htrace-DMA", false,
         {{"dmas", "", TrackRender::SYMBOL}},
         {{"existingDMASize", {"dma"}}}},
        {"ashmem",
         {HtraceEventType::ASHMEM}, "existingAshmemSize", "htrace-ASHMEM", false,
         {{"ashmems", "", TrackRender::SYMBOL}},
         {{"existingAshmemSize", {"ashmem"}}}},

        // fd / thread：count_mode=true——单元是「个数」而非字节，low_threshold 会归零、HTML 按个数显示。
        {"fd",
         {HtraceEventType::HP_FD}, "existingFdCount", "htrace-FD", true,
         {{"fds", "", TrackRender::SYMBOL}},
         {{"existingFdCount", {"fd"}}}},
        {"thread",
         {HtraceEventType::HP_THREAD}, "existingThreadCount", "htrace-THREAD", true,
         {{"threads", "", TrackRender::SYMBOL}},
         {{"existingThreadCount", {"thread"}}}},
    };
    return specs;
}

const HtraceCategorySpec* HtraceFindCategory(const std::string& name) noexcept
{
    for (const auto& spec : HtraceCategories()) {
        if (spec.name == name) {
            return &spec;
        }
    }
    return nullptr;
}

const HtraceCategorySpec& HtraceCategory(const std::string& name)
{
    if (const auto* spec = HtraceFindCategory(name)) {
        return *spec;
    }
    throw std::invalid_argument("unknown htrace category: " + name);
}

} // namespace mem_analyzer