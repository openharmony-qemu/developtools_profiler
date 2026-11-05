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
#include "jsleak/formatters/snapshot_single_json_formatter.h"
#include "formatter/json/json_stream_writer.h"
#include "formatter/json/output_meta.h"
#include "utils/file_utils.h"
#include "utils/platform.h"
#include "utils/logger.h"
#include "utils/terminal_style.h"
#include "utils/timer.h"

#include <iostream>

namespace mem_analyzer {

constexpr int JSON_INDENT_SPACES = 4;

std::string JsleakWriteAnalysisJson(const json& data, const std::string& filePath,
    const RunContext& ctx, const std::string& outDir)
{
    StageTimer timer("JSON write");

    auto base = platform::Stem(filePath);
    if (base.empty()) {
        base = "output";
    }
    auto outPath = platform::JoinPath(outDir, base + ".json");
    platform::CreateDirectories(outDir);

    JsonStreamWriter writer(outPath);

    // 与 htrace JSON 保持一致：name 用完整路径；fileType 按输入扩展名区分。
    json meta = BuildOutputMetaBase(ctx);
    meta["sourceFileInfo"] = {
        {"name", filePath},
        {"fileType", EndsWithIgnoreCase(filePath, "rawheap") ? "rawheap" : "heapsnapshot"}
    };

    json content = json::object();

    // summary.snapshot.totalHeapSize
    content["summary"] = json::object();
    content["summary"]["snapshot"] = json::object();
    content["summary"]["snapshot"]["totalHeapSize"] =
        data.value("totalHeapSize", static_cast<uint64_t>(0));

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

    writer.BeginObject();
    writer.WriteKey("meta");
    writer.WriteRaw(meta.dump(JSON_INDENT_SPACES));
    writer.WriteKey("content");
    writer.WriteRaw(content.dump(JSON_INDENT_SPACES));
    writer.EndObject();

    writer.Close();

    timer.Tick("total");
    log::Logok("Output written to: ", outPath);
    return outPath;
}

} // namespace mem_analyzer
