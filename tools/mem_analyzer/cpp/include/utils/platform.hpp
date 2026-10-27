#pragma once
#include <string>
#include <vector>

struct sqlite3;

namespace mem_analyzer {

enum class Platform { Windows, Linux, Mac };

Platform current_platform();

namespace platform {

/**
 * Directory containing the running executable (UTF-8).
 * Used to locate bundled resources (third_party/, config/) so the release
 * package stays portable. Returns an empty string if it cannot be determined.
 */
std::string get_executable_dir();

std::string get_trace_streamer_path();
std::string get_rawheap_translator_path();
bool file_exists(const std::string& path);
bool directory_exists(const std::string& path);
bool create_directories(const std::string& path);
bool create_directory(const std::string& path);
std::string dirname(const std::string& path);
std::string basename(const std::string& path);
std::string stem(const std::string& path);
std::string filename(const std::string& path);
std::string join_path(const std::string& a, const std::string& b);
std::string absolute_path(const std::string& path);
bool remove_all(const std::string& path);

constexpr int OUTPUT_FD_INHERIT = -1;

/**
 * Run a process.
 * @param output_fd  OUTPUT_FD_INHERIT → inherit parent stdout/stderr
 *                   STDOUT_FILENO      → redirect to stdout
 *                   STDERR_FILENO      → redirect to stderr
 *                   other fd           → redirect to given file descriptor
 */
int run_process(const std::string& exe,
                const std::vector<std::string>& args,
                int output_fd = OUTPUT_FD_INHERIT);

/**
 * Run a process with a spinner indicator.
 * Shows "Running <description>... ⠋ (elapsed)" while the process runs,
 * replaced by "done" or "failed" on completion. Non-TTY output is plain text.
 * On failure, captured subprocess output is printed to stderr (up to 1MB).
 */
int run_process_with_spinner(const std::string& exe,
                             const std::vector<std::string>& args,
                             const std::string& description);

/** Sanitize UTF-8: replace invalid byte sequences with '?'. */
std::string sanitize_utf8(const std::string& input);

/**
 * Open a SQLite3 database with cross-platform path handling.
 * On Windows, uses sqlite3_open16 (UTF-16) for proper Unicode path support.
 * On macOS/Linux, uses sqlite3_open (UTF-8) directly.
 * Throws std::runtime_error on failure.
 */
sqlite3* open_sqlite3_db(const std::string& db_path);

/** Open a file with the system default application. */
void open_with_default_app(const std::string& file_path);

} // namespace platform
} // namespace mem_analyzer
