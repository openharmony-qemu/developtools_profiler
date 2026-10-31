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
#include "formatter/html/report_skeleton.h"

#include "formatter/html/components/help_tooltip.h"
#include "formatter/html/components/navigation_bar.h"
#include "formatter/html/components/version.h"

#include <ostream>

namespace mem_analyzer {

constexpr int CLOSE_TAG_LEN = 2;
constexpr int ESCAPED_CLOSE_TAG_LEN = 3;

std::string HtmlReportBegin(const std::string& title)
{
    return R"HTML(<!DOCTYPE html><html lang="zh-CN"><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>)HTML" +
           title +
           R"HTML(</title>
<style>
)HTML";
}

std::string HtmlReportCssEnd()
{
    return "\n" + VersionFormat() + "\n" + HelpTooltipCss() + "\n" + NavBarFormat() +
           R"HTML(</style></head><body>)HTML";
}

std::string HtmlReportTitle(const std::string& text)
{
    return R"HTML(<h1 class="report-title">)HTML" + text + "</h1>";
}

std::string HtmlReportMeta(const std::vector<std::pair<std::string, std::string>>& items)
{
    std::string html = R"HTML(<div class="meta-card"><div class="meta-grid">)HTML";
    for (const auto& [label, value] : items) {
        html += R"HTML(<div class="meta-item"><span class="meta-label">)HTML" + label +
                R"HTML(：</span><span class="meta-value">)HTML" + value + "</span></div>";
    }
    html += "</div></div>";
    return html;
}

std::string HtmlReportBodyOpen()
{
    return NavBarTitle() +
           R"HTML(<div id="navReport" class="nav-content active"><main class="report-wrap">)HTML";
}

std::string HtmlReportBodyClose()
{
    return R"HTML(</main></div>
<div id="navDoc" class="nav-content">)HTML" +
           NavBarDocContent() +
           R"HTML(</div><div id="navHelp" class="nav-content">)HTML" +
           NavBarHelpCenterContent() +
           "</div>";
}

std::string HtmlReportData(const std::string& varName, const std::string& jsonText)
{
    return "<script>const " + varName + "=" + jsonText + ";\n</script>";
}

void HtmlReportDataStreamed(std::ostream& out,
    const std::string& varName,
    const std::function<void(std::ostream&)>& writeAll)
{
    out << "<script>const " << varName << "=";
    writeAll(out);
    out << ";\n</script>";
}

std::string HtmlReportScript(const std::string& js)
{
    return "<script>\n" + js + "\n</script>";
}

std::string HtmlReportCoreScript()
{
    return HtmlReportScript(NavBarSwitchNavFunc() + "\n" + HelpTooltipJs());
}

std::string HtmlReportEnd()
{
    return VersionContent() + "\n</body></html>";
}

std::string HtmlEscapeScriptJson(const std::string& jsonText)
{
    std::string result = jsonText;
    size_t pos = 0;
    while ((pos = result.find("</", pos)) != std::string::npos) {
        result.replace(pos, CLOSE_TAG_LEN, "<\\/");
        pos += ESCAPED_CLOSE_TAG_LEN;
    }
    return result;
}

} // namespace mem_analyzer
