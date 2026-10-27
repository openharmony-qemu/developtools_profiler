#pragma once
#include "core/run_context.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::ordered_json;

namespace mem_analyzer {

/// 输出 meta 的公共字段：generator / generatorVersion / generateTimestamp / command。
/// 各域 writer 在此之上补自己的 `sourceFileInfo` 等字段。
json build_output_meta_base(const RunContext& ctx);

} // namespace mem_analyzer
