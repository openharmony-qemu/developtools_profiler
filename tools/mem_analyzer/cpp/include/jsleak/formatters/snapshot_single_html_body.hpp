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

std::string build_chain_path_html(const json& chain_frames);
std::string build_property_tag_tab_html(const json& property_tag_objects,
                                        const std::string& prefix,
                                        const std::string& tab_id, bool active,
                                        json& icicle_charts);
std::string build_top_chain_tab_html(const json& top_ref_chains,
                                     const std::string& prefix,
                                     const std::string& tab_id, bool active,
                                     json& icicle_charts, bool search_on_overview = false);

} // namespace mem_analyzer
