#pragma once
#include "formatter/json/json_writer.hpp"
#include <string>

namespace mem_analyzer {

/// 快照对比报告的 HTML 输出（自包含交互式报告：泄漏对象 / 最短引用链两个 Tab）。
/// 入参为对比报告 json（schemaVersion 6）；writer 只做纯重组，不改分析层。
/// 返回完整 HTML 文档内容。
std::string jsleak_render_compare_html(const json& report);

/// 写出对比报告 HTML（out_dir/snapshot-compare.html），返回文件路径；失败返回空串。
std::string jsleak_write_compare_html(const json& report, const std::string& out_dir);

/// 写出对比报告 JSON（out_dir/snapshot-compare.json，schemaVersion 6），返回文件路径；
/// 失败返回空串。
std::string jsleak_write_compare_json(const json& report, const std::string& out_dir);

} // namespace mem_analyzer
