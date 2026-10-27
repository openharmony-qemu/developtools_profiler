#pragma once
#include "formatter/json/json_writer.hpp"
#include <string>

namespace mem_analyzer {

json build_ref_chain_icicle_tree(const json& chain_items, const std::string& root_name);

// 引用链总览图（冰柱图）骨架：工具栏/面包屑/画布 + 数据树表 HTML。
// 数据树表本身由 tree_table 组件生成（jsleak/formatters/tree_table.hpp）。
std::string build_icicle_chart_body(const std::string& canvasId,
                                    const std::string& treeTableHtml);

} // namespace mem_analyzer
