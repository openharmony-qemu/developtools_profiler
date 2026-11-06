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
#include "shared/heapsnapshot/rawheap_converter.h"

#include "utils/file_utils.h"
#include "utils/logger.h"
#include "utils/platform.h"

#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace mem_analyzer {

bool RawheapToHeapsnapshot(const std::string& rawheapPath, const std::string& outputHeapsnapshot, std::string* error)
{
    const platform::HelperBinary translatorBin = platform::GetRawheapTranslatorHelper();
    const std::string& translator = translatorBin.path;
    if (translator.empty()) {
        std::string tried;
        for (const auto& candidate : translatorBin.candidates) {
            if (!tried.empty()) {
                tried += ", ";
            }
            tried += candidate;
        }
        if (error) {
            *error = "rawheap_translator not found. Tried: " + tried;
        }
        return false;
    }

    // 输出父目录（幂等；创建失败不阻断——translator 可能自建或目录已存在）。
    std::error_code ec;
    fs::create_directories(PathFromUtf8(outputHeapsnapshot).parent_path(), ec);

    log::Logd("[rawheap_translator] ", translator, " ", platform::AbsolutePath(rawheapPath),
        " ", platform::AbsolutePath(outputHeapsnapshot));

    const int result = platform::RunProcessWithSpinner(
        translator,
        {platform::AbsolutePath(rawheapPath), platform::AbsolutePath(outputHeapsnapshot)},
        "Running rawheap_translator");
    if (result != 0) {
        if (error) {
            *error = "rawheap_translator failed with code: " + std::to_string(result);
        }
        return false;
    }
    if (!platform::FileExists(outputHeapsnapshot)) {
        if (error) {
            *error = "rawheap_translator exited 0 but no output file";
        }
        return false;
    }
    return true;
}

} // namespace mem_analyzer
