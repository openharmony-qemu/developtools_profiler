#pragma once
#include <string>

namespace mem_analyzer {

// html_components 下的组件统一为「命名空间内自由函数」：无状态、只产出待注入模板的
// HTML/CSS/JS 片段，由各 formatter 通过占位符注入（见 chart_module.hpp 的同名约定）。

// 设置导航栏格式
std::string nav_bar_format();

// 构建标题
std::string nav_bar_title();

// 构建资料文档导航栏内容
std::string nav_bar_doc_content();

// 构建帮助中心导航栏内容
std::string nav_bar_help_center_content();

// 设置切换导航栏函数
std::string nav_bar_switch_nav_func();

}
