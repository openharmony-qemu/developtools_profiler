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
#include "jsleak/formatters/snapshot_single_json_formatter.hpp"
#include "formatter/json/json_stream_writer.hpp"
#include "formatter/json/output_meta.hpp"
#include "utils/file_utils.hpp"
#include "utils/platform.hpp"
#include "utils/logger.hpp"
#include "utils/terminal_style.hpp"
#include "utils/timer.hpp"

#include <iostream>

namespace mem_analyzer {

std::string jsleak_write_analysis_json(const json& data, const std::string& file_path,
                                       const RunContext& ctx, const std::string& out_dir) {
    StageTimer timer("JSON write");

    auto base = platform::stem(file_path);
    if (base.empty()) base = "output";
    auto out_path = platform::join_path(out_dir, base + ".json");
    platform::create_directories(out_dir);

    JsonStreamWriter writer(out_path);

    // 与 htrace JSON 保持一致：name 用完整路径；fileType 按输入扩展名区分。
    json meta = build_output_meta_base(ctx);
    meta["sourceFileInfo"] = {
        {"name", file_path},
        {"fileType", ends_with_ignore_case(file_path, "rawheap") ? "rawheap" : "heapsnapshot"}
    };

    json content = json::object();

    // summary.snapshot.totalHeapSize
    content["summary"] = json::object();
    content["summary"]["snapshot"] = json::object();
    content["summary"]["snapshot"]["totalHeapSize"] =
        data.value("totalHeapSize", (uint64_t)0);

    // 属性标签对象（内部 key 已与输出 key 一致）
    content["propertyTagObjects"] = data.value("propertyTagObjects", json::object());

    // 最短引用链（top-chain）
    if (data.contains("topRefChains")) {
        content["topRefChains"] = data["topRefChains"];
    }

    // napi-wrap 引用链（仅 -m all 且 napi_wrap 预览开启时存在）
    if (data.contains("napiWrapRefChains")) {
        content["napiWrapRefChains"] = data["napiWrapRefChains"];
    }

    writer.begin_object();
    writer.write_key("meta");
    writer.write_raw(meta.dump(4));
    writer.write_key("content");
    writer.write_raw(content.dump(4));
    writer.end_object();

    writer.close();

    timer.tick("total");
    log::logok("Output written to: ", out_path);
    return out_path;
}

} // namespace mem_analyzer
