#pragma once
#include <memory>
#include <string>
#include <vector>

#include "core/module.hpp"

namespace mem_analyzer {

/// 模块注册表：L0 只认这张表，不认具体数据域。
class ModuleRegistry {
public:
    static void add(std::shared_ptr<AnalysisModule> module);

    /// 按模块名查找。
    static AnalysisModule* find(const std::string& name);

    /// 按扩展名查找（大小写不敏感，允许带前导点）。
    static AnalysisModule* find_by_extension(const std::string& ext);

    /// 按文件路径的扩展名查找。
    static AnalysisModule* detect(const std::string& file_path);

    static std::vector<AnalysisModule*> all();

    /// 能力表一致性自检（见 `ModuleCapability` 的约定）。
    /// 返回不一致条目，空 = 一致；正常构建下应始终为空。
    static std::vector<std::string> self_check();
};

} // namespace mem_analyzer
