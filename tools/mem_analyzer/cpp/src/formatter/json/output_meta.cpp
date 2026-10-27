#include "formatter/json/output_meta.hpp"

#include "formatter/json/json_writer.hpp"
#include "utils/version.hpp"

#include <chrono>

namespace mem_analyzer {

json build_output_meta_base(const RunContext& ctx) {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    json meta;
    meta["generator"] = "OpenHarmony Memory Analyzer";
    meta["generatorVersion"] = get_version();
    meta["generateTimestamp"] = format_readable_timestamp(ms);
    meta["command"] = ctx.command_line;
    return meta;
}

} // namespace mem_analyzer
