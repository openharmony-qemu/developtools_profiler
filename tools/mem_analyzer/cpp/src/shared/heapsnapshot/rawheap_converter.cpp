#include "shared/heapsnapshot/rawheap_converter.hpp"

#include "utils/file_utils.hpp"
#include "utils/logger.hpp"
#include "utils/platform.hpp"

#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace mem_analyzer {

bool rawheap_to_heapsnapshot(const std::string& rawheap_path,
                             const std::string& output_heapsnapshot,
                             std::string* error) {
    const std::string translator = platform::get_rawheap_translator_path();
    if (translator.empty() || !platform::file_exists(translator)) {
        if (error) *error = "rawheap_translator not found at: " + translator;
        return false;
    }

    // 输出父目录（幂等；创建失败不阻断——translator 可能自建或目录已存在）。
    std::error_code ec;
    fs::create_directories(path_from_utf8(output_heapsnapshot).parent_path(), ec);

    log::logd("[rawheap_translator] ", translator, " ", platform::absolute_path(rawheap_path),
              " ", platform::absolute_path(output_heapsnapshot));

    const int result = platform::run_process_with_spinner(
        translator,
        {platform::absolute_path(rawheap_path), platform::absolute_path(output_heapsnapshot)},
        "Running rawheap_translator");
    if (result != 0) {
        if (error) *error = "rawheap_translator failed with code: " + std::to_string(result);
        return false;
    }
    if (!platform::file_exists(output_heapsnapshot)) {
        if (error) *error = "rawheap_translator exited 0 but no output file";
        return false;
    }
    return true;
}

} // namespace mem_analyzer
