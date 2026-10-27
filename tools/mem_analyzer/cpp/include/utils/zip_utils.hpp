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
#include <string>

namespace mem_analyzer {
namespace zipUtils {

/// 用 minizip API 将 zip 解压到 dest_dir，替代 shell 调用 `unzip` / `Expand-Archive`
/// （消除命令注入面）。校验条目路径防 zip-slip：拒绝绝对路径、盘符、`..` 组件。
/// 成功返回 true；失败返回 false 且 `error` 非空。
bool ExtractZip(const std::string& zipPath, const std::string& destDir,
                 std::string& error);

} // namespace zip_utils
} // namespace mem_analyzer
