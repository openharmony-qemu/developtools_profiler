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

TraceDB HtraceParse(const std::string& filePath, const RunContext& ctx)
{
    StageTimer timer("Parse");

    // 1. Validate magic number + dataType (profiler file)
    timer.Stage("Validating file magic number");
    if (!HtraceReader::IsProfilerFile(filePath)) {
        throw std::runtime_error("Invalid file signature");
    }
    timer.Tick("validate");

    // 2. Create temp directory (in system temp dir, not output dir).
    //    RAII：DB 文件在函数返回前已被消费完，故可随作用域自动清理。
    TempDir tempDir("htrace");
    auto sourceName = platform::stem(filePath);

    auto dbPath = platform::JoinPath(tempDir.path(), sourceName + "_trace_streamer.db");

    // 3. Run trace_streamer
    std::string streamerErr;
    if (!RunTraceStreamer(filePath, dbPath, ctx.resources.soDir, &streamerErr)) {
        throw std::runtime_error(streamerErr.empty() ? "trace_streamer conversion failed" :
                                                       streamerErr);
    }
    log::Logok("    DB file generated: ", dbPath);
    timer.Tick("trace_streamer");

    // 4. Process DB into TraceDB with compressed stacks + metadata
    timer.Stage("Processing DB into call stacks");
    auto db = ProcessHtraceDb(dbPath, ctx.htrace.category, !ctx.output.parseOnly, !ctx.htrace.leakAddrs.empty());
    if (db.meta.processName.empty()) {
        std::string nm = HtraceReader::ReadProcessName(filePath);
        if (!nm.empty()) {
            db.meta.processName = std::move(nm);
        }
    }
    auto* clist = std::get_if<CompressedCallStackList>(&db.stacks);
    timer.Tick("db_process", std::to_string(clist ? clist->stacks.size() : 0) + " call stacks");

    timer.Done();
    return db;
}

} // namespace mem_analyzer
