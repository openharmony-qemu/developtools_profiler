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
