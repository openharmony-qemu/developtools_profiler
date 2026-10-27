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
#pragma once
#include "formatter/json/json_writer.hpp"
#include <string>

namespace mem_analyzer {

/// 快照对比报告的 HTML 输出（自包含交互式报告：泄漏对象 / 最短引用链两个 Tab）。
/// 入参为对比报告 json（schemaVersion 6）；writer 只做纯重组，不改分析层。
/// 返回完整 HTML 文档内容。
std::string jsleak_render_compare_html(const json& report);

/// 写出对比报告 HTML（out_dir/snapshot-compare.html），返回文件路径；失败返回空串。
std::string jsleak_write_compare_html(const json& report, const std::string& out_dir);

/// 写出对比报告 JSON（out_dir/snapshot-compare.json，schemaVersion 6），返回文件路径；
/// 失败返回空串。
std::string jsleak_write_compare_json(const json& report, const std::string& out_dir);

} // namespace mem_analyzer
