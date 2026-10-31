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
#include "formatter/json/output_meta.h"

#include "formatter/json/json_writer.h"
#include "utils/version.h"

#include <chrono>

namespace mem_analyzer {

json BuildOutputMetaBase(const RunContext& ctx)
{
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    json meta;
    meta["generator"] = "OpenHarmony Memory Analyzer";
    meta["generatorVersion"] = GetVersion();
    meta["generateTimestamp"] = FormatReadableTimestamp(ms);
    meta["command"] = ctx.commandLine;
    return meta;
}

} // namespace mem_analyzer
