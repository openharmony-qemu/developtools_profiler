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
#include <vector>
#include <functional>
#include <iosfwd>

namespace mem_analyzer {

/// HTML 报告页面骨架接口：封装所有报告共用的文档结构，
/// 各域 formatter 只提供标题、meta 项、数据、脚本和主体内容，骨架负责拼接完整页面。
///
/// 用法示例（非流式）：
///   out << html_report_begin("报告标题");
///   out << domain_css;                  // ① CSS 注入点（域专属 + 可选组件）
///   out << html_report_css_end();       // ② 自动注入 version/help_tooltip/nav_bar CSS
///   out << html_report_title("报告标题");
///   out << html_report_body_open();
///   out << html_report_meta({{"日志名", "xxx"}, {"类型", "yyy"}});
///   out << body_content;                // ③ 域正文
///   out << html_report_body_close();
///   out << html_report_data("D", json); // ④ 数据段
///   out << html_report_core_script();   // ⑤ 脚本段（核心交互 + 可选组件 + 域）
///   out << html_report_end();           // ⑥ </body></html>
///
/// 骨架自动注入的「必选核心」：navigation_bar / help_tooltip / version。
/// 「可选组件」（chart_module 图 / tab_bar 分类 tab / tree_table 数据树表）由域按需注入。

/// 生成 HTML 文档头：DOCTYPE + <html> + <head> + <meta> + <title> + <style> 开标签。
std::string HtmlReportBegin(const std::string& title);

/// 生成 CSS 区域收尾 + <body> 开标签 + 导航栏标题。
/// 调用方需在 begin 与本函数之间注入专属 CSS 与可选组件 CSS。
/// 内部自动追加 version_format / help_tooltip_css / nav_bar_format。
std::string HtmlReportCssEnd();

/// 生成报告标题 <h1>。
std::string HtmlReportTitle(const std::string& text);

/// 生成 meta 卡片（key-value 信息栏）。
/// items 为 {label, value} 对；label 后自动补全角冒号，value 可含内联 HTML。
std::string HtmlReportMeta(const std::vector<std::pair<std::string, std::string>>& items);

/// 生成导航栏主体开壳：<div id="navReport"> + <main class="report-wrap">。
std::string HtmlReportBodyOpen();

/// 关闭报告主体 + 导航栏文档面板 + 帮助面板。
std::string HtmlReportBodyClose();

/// 生成数据段（非流式便捷版）：<script>const {var}={json};\n</script>。
/// json_text 为已序列化并做 XSS 转义后的 JSON 文本。
std::string HtmlReportData(const std::string& varName, const std::string& jsonText);

/// 生成数据段（流式版）：write_all 负责写任意 JSON 值（对象或数组），
/// 骨架只包 <script>const {var}= 前缀与 ;\n</script> 后缀。
void HtmlReportDataStreamed(std::ostream& out,
                               const std::string& varName,
                               const std::function<void(std::ostream&)>& writeAll);

/// 生成通用脚本段：<script>\n{js}\n</script>（可多次调用，自由组织多段脚本）。
std::string HtmlReportScript(const std::string& js);

/// 生成核心交互脚本段：nav_switch + help_tooltip_js。
std::string HtmlReportCoreScript();

/// 生成文档尾：</body></html>（纯收尾，不含 </script>）。
std::string HtmlReportEnd();

/// XSS 转义：将 JSON 字符串中的 "</" 替换为 "<\/"，防止 </script> 提前闭合。
std::string HtmlEscapeScriptJson(const std::string& jsonText);

} // namespace mem_analyzer
