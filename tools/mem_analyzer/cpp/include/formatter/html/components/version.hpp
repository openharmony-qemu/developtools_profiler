#pragma once
#include <string>

namespace mem_analyzer {

// 页脚版本号组件：footer 样式与内容片段。
// 无状态、只产出待注入的 CSS/HTML 片段（见 navigation_bar.hpp 的同名约定）。
// 三域（htrace/tshandle/jsleak）通用。

// footer 页脚样式（对应占位符 __VERSION_FORMAT__）。
std::string version_format();

// footer 页脚内容（含工具版本号，对应占位符 __VERSION_CONTENT__）。
std::string version_content();

} // namespace mem_analyzer
