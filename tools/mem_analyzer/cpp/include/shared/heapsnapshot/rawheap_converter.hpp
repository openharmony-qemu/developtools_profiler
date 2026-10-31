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

/**
 * 调用 rawheap_translator 将 .rawheap 转为 .heapsnapshot（jsleak 与 tshandle 共用）。
 * 统一处理：校验 translator 存在 → 创建输出父目录 → 转绝对路径 → 执行转换 →
 * 校验产物已生成。本函数只返回成功/失败与失败原因，不打印、不抛异常——
 * 失败后是报错中止还是降级继续，由调用方按各自域的语义决定（jsleak 硬错误、
 * tshandle 软告警跳过 ArkTS 类分析）。
 * @param rawheap_path        输入的 .rawheap 文件
 * @param output_heapsnapshot 输出的 .heapsnapshot 文件（父目录不存在会被创建）
 * @param error               可选，失败原因；成功时保持不变
 * @return 转换成功返回 true；translator 缺失 / 转换非零退出 / 产物未生成返回 false
 */
bool RawheapToHeapsnapshot(const std::string& rawheapPath,
    const std::string& outputHeapsnapshot,
    std::string* error = nullptr);

} // namespace mem_analyzer
