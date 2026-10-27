#pragma once
#include <string>

namespace mem_analyzer {

/// 单快照报告专属 JS（主 tab 切换 / 详情展开 / 节点 ID 与 hash 值展开）。
/// 二级页签与冰柱图相关 JS 见 report_common。
std::string snapshot_html_js();

}