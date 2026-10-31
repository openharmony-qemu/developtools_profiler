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
// ============================================================================
//  leak_file_reader.h
//  读取 Native Leaks 不可达内存泄漏文件（trace -l 输入）：
//    偏移 0-7 :  magic "NSNAP1.0"
//    偏移 8-15:  count (uint64 LE) 泄漏块数
//    偏移 16+ :  count x 16 字节（8B 地址 LE + 8B 大小 LE）
//  返回所有泄漏地址的集合。count=0 返回空集合。
// ============================================================================
#include <cstdint>
#include <string>
#include <unordered_set>

namespace mem_analyzer {

std::unordered_set<uint64_t> ReadLeakFile(const std::string& path);

} // namespace mem_analyzer
