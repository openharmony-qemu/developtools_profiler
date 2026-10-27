#pragma once
#include <string>

namespace mem_analyzer {

// help-icon / help-tooltip 公共组件：自适应 tooltip（视口边缘自动翻转/夹紧，
// 避免文字被裁切）的 CSS 样式与初始化脚本。无状态、只产出待注入的片段，
// 由各 HTML 报告通过占位符或流式拼接注入（见 navigation_bar.hpp 的同名约定）。

// help-icon / help-tooltip 样式（含箭头与 above/below 翻转态）
std::string help_tooltip_css();

// initHelpTooltip 脚本（函数定义 + 调用，注入在 </script> 之前）
std::string help_tooltip_js();

}
