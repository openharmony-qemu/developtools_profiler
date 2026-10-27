#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace mem_analyzer {

// HiProfiler NativeHookConfig（native_hook_config.proto）
// 未出现在文件中的字段保持默认值。
struct HtraceNativeHookConfig {
    bool present = false;               // 是否解析到 nativehook_config 段

    int32_t  pid = 0;                   // 1
    bool     save_file = false;         // 2
    std::string file_name;              // 3
    int32_t  filter_size = 0;           // 4
    int32_t  smb_pages = 0;             // 5
    int32_t  max_stack_depth = 0;       // 6
    std::string process_name;           // 7
    bool     malloc_disable = false;    // 8
    bool     mmap_disable = false;      // 9
    bool     free_stack_report = false; // 10
    bool     munmap_stack_report = false; // 11
    uint32_t malloc_free_matching_interval = 0; // 12
    uint32_t malloc_free_matching_cnt = 0;      // 13
    bool     string_compressed = false; // 14
    bool     fp_unwind = false;         // 15
    bool     blocked = false;           // 16
    bool     record_accurately = false; // 17
    bool     startup_mode = false;      // 18
    bool     memtrace_enable = false;   // 19
    bool     offline_symbolization = false; // 20
    bool     callframe_compress = false;    // 21
    uint32_t statistics_interval = 0;   // 22
    std::string clock;                  // 23
    uint32_t sample_interval = 0;       // 24
    bool     response_library_mode = false; // 25
    std::vector<int32_t> expand_pids;   // 26 (repeated int32)
    int32_t  js_stack_report = 0;       // 27
    uint32_t max_js_stack_depth = 0;    // 28
    std::string filter_napi_name;       // 29
    bool     dump_nmd = false;          // 30
    std::string target_so_name;         // 31
    std::vector<std::string> restrace_tag; // 32 (repeated string)
    bool     async_stack_enable = false;// 33
    std::vector<uint32_t> async_type;   // 34 (repeated enum)
    uint32_t unique_stack_table_size = 0; // 35
    bool     use_file_cache_mode = false; // 36
    uint32_t file_cache_buffer_size = 0;  // 37
    bool     allocator_sampling_enable = false; // 38
    bool     native_orphansnapshots_enable = false; // 39
    bool     nested_async_stack_enable = false;     // 40
    std::vector<std::string> filter_lib_list; // 41 (repeated string)
    uint32_t async_nesting_depth = 0;     // 42
    uint32_t async_task_stack_depth = 0;  // 43
    bool     arkui_mem_trace_enable = false; // 44
    bool     discard_destroyed_traces = false; // 45
};

// 直接读取原始 .htrace 文件信息（零依赖，手工解析 protobuf wire format）。
// 用于在 trace_streamer 转换结果缺失时兜底读取 process_name，以及文件类型判定。
class HtraceReader {
public:
    // profiler 文件判断：魔数合法 && dataType == HIPROFILER_PROTOBUF_BIN(0)。
    // 判断型函数：文件打不开/格式异常统一返回 false，不抛异常。
    static bool is_profiler_file(const std::string& path);

    // 解析第一个 nativehook_config 段并填充全部字段。
    static bool read_nativehook_config(const std::string& path, HtraceNativeHookConfig& out);

    // 便捷入口：只取 process_name（内部复用 read_nativehook_config）。
    static std::string read_process_name(const std::string& path);
};

} // namespace mem_analyzer