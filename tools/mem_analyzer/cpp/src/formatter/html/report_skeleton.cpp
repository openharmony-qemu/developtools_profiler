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
#include "formatter/html/report_skeleton.hpp"

#include "formatter/html/components/help_tooltip.hpp"
#include "formatter/html/components/navigation_bar.hpp"
#include "formatter/html/components/version.hpp"

#include <ostream>

namespace mem_analyzer {

std::string html_report_begin(const std::string& title) {
    return R"HTML(<!DOCTYPE html><html lang="zh-CN"><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>)HTML" +
           title + R"HTML(</title>
<style>
)HTML";
}

std::string html_report_css_end() {
    return "\n" + version_format() + "\n" + help_tooltip_css() + "\n" + nav_bar_format() +
           R"HTML(</style></head><body>)HTML";
}

std::string html_report_title(const std::string& text) {
    return R"HTML(<h1 class="report-title">)HTML" + text + "</h1>";
}

std::string html_report_meta(const std::vector<std::pair<std::string, std::string>>& items) {
    std::string html = R"HTML(<div class="meta-card"><div class="meta-grid">)HTML";
    for (const auto& [label, value] : items) {
        html += R"HTML(<div class="meta-item"><span class="meta-label">)HTML" + label +
                R"HTML(：</span><span class="meta-value">)HTML" + value + "</span></div>";
    }
    html += "</div></div>";
    return html;
}

std::string html_report_body_open() {
    return nav_bar_title() +
           R"HTML(<div id="navReport" class="nav-content active"><main class="report-wrap">)HTML";
}

std::string html_report_body_close() {
    return R"HTML(</main></div>
<div id="navDoc" class="nav-content">)HTML" +
           nav_bar_doc_content() +
           R"HTML(</div><div id="navHelp" class="nav-content">)HTML" +
           nav_bar_help_center_content() + "</div>";
}

std::string html_report_data(const std::string& var_name, const std::string& json_text) {
    return "<script>const " + var_name + "=" + json_text + ";\n</script>";
}

void html_report_data_streamed(std::ostream& out,
                               const std::string& var_name,
                               const std::function<void(std::ostream&)>& write_all) {
    out << "<script>const " << var_name << "=";
    write_all(out);
    out << ";\n</script>";
}

std::string html_report_script(const std::string& js) {
    return "<script>\n" + js + "\n</script>";
}

std::string html_report_core_script() {
    return html_report_script(nav_bar_switch_nav_func() + "\n" + help_tooltip_js());
}

std::string html_report_end() {
    return version_content() + "\n</body></html>";
}

std::string html_escape_script_json(const std::string& json_text) {
    std::string result = json_text;
    size_t pos = 0;
    while ((pos = result.find("</", pos)) != std::string::npos) {
        result.replace(pos, 2, "<\\/");
        pos += 3;
    }
    return result;
}

} // namespace mem_analyzer
