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
#include "jsleak/snapshot_analyzer.hpp"
#include <optional>
#include <string>
#include <unordered_set>

namespace mem_analyzer {

/// 引用链聚合阶段：按谓词筛选节点后沿最短引用链聚合，按累计 retained size 降序输出。
/// top-chain 与 napi-wrap 都是它的实例，仅 target_class / top_k / exclude 不同。
class RefChainStage : public JsleakStage {
public:
    RefChainStage(std::string mode,
                  const char* preview_id,
                  const char* json_key,
                  std::optional<std::string> target_class,   // nullopt=全节点；非空=节点名精确匹配
                  std::optional<size_t> top_k,
                  std::unordered_set<std::string> exclude_chains);

    const std::string& mode() const override;
    const char* preview_id() const override;
    const char* json_key() const override;
    void run(const HeapSnapshot& heap) override;
    json to_json() const override;

private:
    std::string mode_;
    const char* preview_id_;
    const char* json_key_;
    std::optional<std::string> target_class_;
    std::optional<size_t> top_k_;
    std::unordered_set<std::string> exclude_chains_;
    std::vector<RefChainInfo> ref_chain_infos_;
};

} // namespace mem_analyzer
