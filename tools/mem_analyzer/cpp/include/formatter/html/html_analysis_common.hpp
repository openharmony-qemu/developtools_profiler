#pragma once

// html_analysis_common.hpp — HTML分析报告公共模块
// 包含各数据域（htrace/tshandle/jsleak）共享的数据结构、辅助函数和HTML模板。
// 本头文件不得依赖任何数据域（htrace/tshandle/jsleak）——域专用的构建函数
// 放在各自域内，例如 htrace 的 `htrace/formatters/htrace_flame_json.hpp`。

#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include "utils/platform.hpp"

using json = nlohmann::ordered_json;

namespace mem_analyzer::html_analysis {

// ===== String table for name deduplication =====
// 99%是重复name，用索引引用替代字符串可减少约70%的name数据量
class StringTable {
    std::vector<std::string> strings_;
    std::unordered_map<std::string, int> indexMap_;
public:
    int intern(const std::string& s) {
        auto it = indexMap_.find(s);
        if (it != indexMap_.end()) return it->second;
        int idx = (int)strings_.size();
        strings_.push_back(s);
        indexMap_[s] = idx;
        return idx;
    }
    json toArray() const {
        json arr = json::array();
        for (const auto& s : strings_) arr.push_back(s);
        return arr;
    }
};

// ===== Data structures =====

// 调用链中的单帧信息，对应火焰图/冰柱图中的一个调用层级
struct CallChainFrame {
    std::string name;       // 函数符号名（含偏移或完整签名）
    std::string lib;        // 所属动态库路径
    std::string libOffset;  // 库内偏移地址
};

// 一条完整的调用链及其关联的内存大小
struct CallChainResult {
    std::vector<CallChainFrame> frames;  // 从顶到底的调用帧序列
    double size = 0;                     // 该调用链对应的泄漏/分配内存大小
    long long count = 0;                 // 该调用链对应的泄漏/分配对象数量
};

// 每个TopSymbol的完整数据条目，用于生成火焰图、冰柱图和调用链表格
struct SymbolDataEntry {
    json info;                                    // 符号基本信息（name/lib/existingSize等）
    json flameTree;                               // 火焰图JSON树（直接构建，高效）
    json icicleTree;                              // 冰柱图JSON树（直接构建，高效）
    json names;                                   // 字符串表：节点n字段为索引，解析后还原
    std::vector<CallChainResult> callChains;      // 该符号的所有调用链
};

// ===== 辅助函数 =====

// 字符串全局替换
std::string replace_all(std::string str, const std::string& from, const std::string& to);
// HTML特殊字符转义（逐字符switch方式，高效）
std::string escapeHtml(const std::string& str);
// 内存大小格式化（如 "1.23 KB"、"2.5 MB"）
std::string formatBytes(double bytes);
// 数值显示：countMode 下（fd/thread 等计数类型）直接输出整数个数，否则按字节格式化
std::string formatSizeText(double val, bool countMode);
// 时间段格式化（如 "3m 24s"）
std::string formatDuration(const std::string& durationStr);
// 函数全名格式化：从符号名+库名+偏移构建完整显示名
std::string formatFuncName(const std::string& symbol, const std::string& lib, const std::string& libOffset);
// 函数精简名格式化：截取括号前内容或lib+offset格式，用于火焰图节点显示
std::string formatShortFuncName(const std::string& symbol, const std::string& lib, const std::string& libOffset);
// 库名精简显示：提取so/dll的basename，unknown符号保留路径
std::string getLibForDisplay(const std::string& lib, const std::string& symbol);
// 浮点数固定小数位格式化
std::string toFixed(double value, int precision);
// 四舍五入（远离零方向，如 -0.5 → -1）
double roundHalfAwayFromZero(double value, int precision);

// ===== HTML构建辅助函数 =====

// 将SymbolDataEntry序列化为前端data JSON中的单个条目（move语义，避免拷贝flameTree/icicleTree）
json buildSymbolDataEntryJSON(SymbolDataEntry& entry);
// 生成调用链详情的HTML表格
std::string buildCallChainHTML(const std::vector<CallChainResult>& callChains, double totalExistingSize, bool countMode = false);
// 生成Top符号排行表格的行HTML
std::string buildTopTableRows(const std::vector<SymbolDataEntry>& symbols, size_t startIndex, bool countMode = false);
// 生成所有符号的详情区段HTML（含火焰图+冰柱图+调用链）
std::string buildDetailSections(const std::vector<SymbolDataEntry>& symbols, size_t startIndex, long long totalExistingSize, bool countMode = false);
// 生成单个符号的详情区段内容HTML
std::string buildDetailSectionContent(const SymbolDataEntry& entry, size_t idx, long long totalExistingSize, bool addBlankLine, bool countMode = false);

// ===== HTML模板 =====

// 完整的HTML页面模板字符串，包含CSS/JS和占位符（__DATA_JSON__等）
extern const std::string HTML_TEMPLATE;

} // namespace mem_analyzer::html_analysis
