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
#include "formatter/html/components/version.h"

#include "utils/version.h"

namespace mem_analyzer {

std::string VersionFormat()
{
    return R"CSS(
.footer { margin: 30px 20px 20px; }
.footer-card { background: #f8f9fa; border: 1px solid #dee2e6; border-radius: 8px; padding: 16px 24px; }
.footer-label { color: #888; font-size: 14px; margin-bottom: 10px; }
.footer-links { display: flex; flex-direction: column; gap: 8px; }
.footer-links a { color: #1a73e8; text-decoration: none; font-size: 14px; transition: color 0.2s; }
.footer-links a:hover { color: #4285f4; text-decoration: underline; }
.footer-version { color: #888; font-size: 13px; text-align: right; margin-top: 12px; }
)CSS";
}

std::string VersionContent()
{
    const std::string version = GetVersion();
    return "<div class=\"footer\">\n"
           "  <div class=\"footer-version\">工具版本：v" + version + "</div>\n"
           "</div>";
}

} // namespace mem_analyzer
