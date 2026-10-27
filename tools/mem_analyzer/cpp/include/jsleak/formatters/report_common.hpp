#pragma once
#include <string>

namespace mem_analyzer {

// jsleak 域报告公共组件：单快照报告与对比报告共用的基础样式、基础 JS 工具，
// 以及二级页签 + 引用链总览图（冰柱图）相关 CSS/JS。
// 无状态自由函数，只产出待注入的 CSS/JS 片段，由各 jsleak formatter 注入。

// jsleak 报告基础 CSS：reset / body / 标题 / meta 卡片 / 表格 / 详情条目 /
// 引用链 / 节点 ID 标签 / 空态提示。各 formatter 注入后可追加自己的专属样式。
std::string report_common_css();

// jsleak 报告基础 JS 工具：esc / bytes / num / delta / nodeIdsHtml / expandNodeIds /
// showDetail / toggleDetail / chainHtml。各 formatter 注入后可追加自己的专属交互逻辑。
std::string report_common_js();

// 二级页签（详情列表 / 引用链总览图）+ 冰柱图画布容器的共享 CSS。
std::string report_common_sub_tab_css();

// 二级页签切换 / 「查看」跳总览图 / 引用链树表展开折叠与复制的共享 JS；
// 暴露 window.markSelectedRow / switchSubTab / showInOverview / toggleIcicleTreeRow /
// toggleIcicleTreeAll / copyIcicleTree。
std::string report_common_sub_tab_js();

// 引用链总览图（冰柱图）初始化 JS：把 window.ICICLE_CHARTS_DATA 交给图表模块，
// 启用「子树上包络」布局。
std::string report_common_chart_init_js();

} // namespace mem_analyzer