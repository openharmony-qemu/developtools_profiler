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
#include "shared/heapsnapshot/source_map.hpp"
#include "shared/heapsnapshot/name_cache.hpp"

namespace mem_analyzer {

/// 对 heapsnapshot 做反混淆与源码还原：流式定位顶层 "strings" 字符串数组，
/// 仅解析 strings 段（混淆名 → 原始名、混淆编译产物路径 → 源码路径），
/// nodes / edges / meta 等其余字段以原始字节流式拷贝，避免大快照全量载入内存。
/// @param input_path  输入的 heapsnapshot 文件
/// @param output_path 还原后的输出文件
/// @param smap        sourceMap 索引（可为空指针，跳过路径还原）
/// @param ncache       nameCache 索引（可为空指针，跳过名字还原）
/// @return 成功写出返回 true；文件不存在、解析失败或写出失败返回 false
bool DeobfuscateHeapsnapshot(const std::string& inputPath, const std::string& outputPath,
                              const SourceMapIndex* smap, const NameCacheIndex* ncache);

/// 加载 sourceMap / nameCache 索引并对 heapsnapshot 执行反混淆的公共时序：
/// 校验路径 → 加载索引 → 创建输出父目录 → 调用 deobfuscate_heapsnapshot。
/// @param sourcemap_dir sourceMap 目录/文件路径（空则跳过路径还原）
/// @param namecache_dir nameCache 目录/文件路径（空则跳过名字还原）
/// @return 成功写出返回 true；
///         两个目录均为空时返回 false（调用方应直接降级为原始输入，不报警告）；
///         任一目录非空但索引加载为空时打印警告并返回 false（降级为原始输入）；
///         反混淆失败时返回 false。
bool DeobfuscateWithIndices(const std::string& inputPath, const std::string& outputPath,
                              const std::string& sourcemapDir,
                              const std::string& namecacheDir);

} // namespace mem_analyzer
