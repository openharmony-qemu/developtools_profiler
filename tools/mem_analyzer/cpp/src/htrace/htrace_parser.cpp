#include "htrace/htrace_parser.hpp"
#include "htrace/htrace_processor.hpp"
#include "utils/file_utils.hpp"
#include "shared/htrace_model/htrace_reader.hpp"
#include "shared/htrace_model/trace_streamer_runner.hpp"
#include "utils/platform.hpp"
#include "utils/temp_dir.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"
#include <chrono>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <variant>
#include <vector>

namespace mem_analyzer {

TraceDB htrace_parse(const std::string& file_path, const RunContext& ctx) {
    StageTimer timer("Parse");

    // 1. Validate magic number + dataType (profiler file)
    timer.stage("Validating file magic number");
    if (!HtraceReader::is_profiler_file(file_path)) {
        throw std::runtime_error("Invalid file signature");
    }
    timer.tick("validate");

    // 2. Create temp directory (in system temp dir, not output dir).
    //    RAII：DB 文件在函数返回前已被消费完，故可随作用域自动清理。
    TempDir temp_dir("htrace");
    auto source_name = platform::stem(file_path);

    auto db_path = platform::join_path(temp_dir.path(), source_name + "_trace_streamer.db");

    // 3. Run trace_streamer
    std::string streamer_err;
    if (!run_trace_streamer(file_path, db_path, ctx.resources.so_dir, &streamer_err)) {
        throw std::runtime_error(streamer_err.empty() ? "trace_streamer conversion failed"
                                                       : streamer_err);
    }
    log::logok("    DB file generated: ", db_path);
    timer.tick("trace_streamer");

    // 4. Process DB into TraceDB with compressed stacks + metadata
    timer.stage("Processing DB into call stacks");
    auto db = process_htrace_db(db_path, ctx.htrace.category, !ctx.output.parse_only, !ctx.htrace.leak_addrs.empty());
    if (db.meta.process_name.empty()) {
        std::string nm = HtraceReader::read_process_name(file_path);
        if (!nm.empty()) db.meta.process_name = std::move(nm);
    }
    auto* clist = std::get_if<CompressedCallStackList>(&db.stacks);
    timer.tick("db_process", std::to_string(clist ? clist->stacks.size() : 0) + " call stacks");

    timer.done();
    return db;
}

} // namespace mem_analyzer
