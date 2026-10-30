# ══════════════════════════════════════════════════════════════════
# 禁止直接使用 cmake 编译！必须使用与当前脚本同级目录下对应的构建脚本，做完整流程的编译和打包，否则会执行失败：
#   Windows平台:  build-win.ps1
#   Linux平台:    release-cross.sh
#   macOS平台:    release-mac.sh
# ══════════════════════════════════════════════════════════════════
cmake_minimum_required(VERSION 3.16)

# ── Prefer Clang on Linux ──────────────────────────────────────
# CMake selects the compiler during project() using CC/CXX env vars or
# cached CMAKE_C/CXX_COMPILER. Setting cache variables before project()
# is unreliable; setting ENV{CC/CXX} is the documented way to influence
# compiler selection from within CMakeLists.txt.
if(UNIX AND NOT APPLE AND NOT MINGW)
    if(NOT CMAKE_C_COMPILER AND NOT CMAKE_CXX_COMPILER AND NOT ENV{CC} AND NOT ENV{CXX})
        set(_CLANG_C "")
        set(_CLANG_CXX "")
        foreach(ver "" -18 -17 -16 -15 -14 -13 -12 -11)
            set(_c "/usr/bin/clang${ver}")
            set(_cxx "/usr/bin/clang++${ver}")
            if(EXISTS "${_c}" AND EXISTS "${_cxx}")
                set(_CLANG_C "${_c}")
                set(_CLANG_CXX "${_cxx}")
                break()
            endif()
        endforeach()
        if(_CLANG_C AND _CLANG_CXX)
            set(ENV{CC}  "${_CLANG_C}")
            set(ENV{CXX} "${_CLANG_CXX}")
        endif()
    endif()
endif()

project(mem-analyzer VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Third-party library paths ──────────────────────────────────
file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../third_party" THIRD_PARTY_DIR)
set(SQLITE_SRC    "${THIRD_PARTY_DIR}/src/third_party_sqlite/src/sqlite3.c")
set(SQLITE_INC    "${THIRD_PARTY_DIR}/src/third_party_sqlite/include")
set(JSON_INC      "${THIRD_PARTY_DIR}/src/third_party_json/single_include")
set(ZLIB_LIB      "${THIRD_PARTY_DIR}/src/third_party_zlib")
set(MINIZIP_DIR   "${ZLIB_LIB}/contrib/minizip")
set(ZLIB_INC      "${ZLIB_LIB}")
set(MINIZIP_INC   "${MINIZIP_DIR}")

# ── Sources ────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/cli/command.cpp
    src/cli/pipeline_runner.cpp
    src/cli/trace_command.cpp
    src/cli/tshandle_command.cpp
    src/cli/jsleak_command.cpp
    src/cli/profiler_command.cpp
    src/cli/preview_command.cpp
    src/shared/htrace_model/htrace_data.cpp
    src/core/module_registry.cpp
    src/utils/version.cpp
    src/utils/leak_file_reader.cpp
    src/utils/platform.cpp
    src/utils/file_utils.cpp
    src/utils/config.cpp
    src/cli/cli_parser.cpp
    src/utils/temp_dir.cpp
    src/utils/process_spinner.cpp
    src/utils/terminal_style.cpp
    src/utils/logger.cpp
    src/shared/htrace_model/htrace_reader.cpp
    src/shared/htrace_model/htrace_db_helper.cpp
    src/shared/htrace_model/trace_streamer_runner.cpp
    src/htrace/htrace_processor.cpp
    src/htrace/htrace_category.cpp
    src/htrace/htrace_parser.cpp
    src/htrace/htrace_module.cpp
    src/tshandle/tshandle_processor.cpp
    src/tshandle/zip_parser.cpp
    src/tshandle/tshandle_module.cpp
    src/htrace/htrace_analyzer.cpp
    src/jsleak/snapshot_analyzer.cpp
    src/jsleak/property_tag_stage.cpp
    src/jsleak/ref_chain_stage.cpp
    src/jsleak/snapshot_compare.cpp
    src/jsleak/formatters/report_common.cpp
    src/jsleak/formatters/tree_table.cpp
    src/jsleak/formatters/snapshot_compare_formatter.cpp
    src/jsleak/formatters/icicle_tree.cpp
    src/jsleak/formatters/snapshot_single_html_body.cpp
    src/jsleak/formatters/snapshot_single_html_js.cpp
    src/htrace/flame_icicle.cpp
    src/htrace/leak_filter_analyzer.cpp
    src/formatter/json/json_writer.cpp
    src/formatter/json/json_stream_writer.cpp
    src/formatter/json/output_meta.cpp
    src/core/format_resolver.cpp
    src/jsleak/formatters/snapshot_single_html_formatter.cpp
    src/jsleak/formatters/snapshot_single_json_formatter.cpp
    src/htrace/formatters/htrace_json_formatter.cpp
    src/htrace/formatters/htrace_parse_json_formatter.cpp
    src/htrace/formatters/htrace_analysis_json_formatter.cpp
    src/htrace/formatters/htrace_html_formatter.cpp
    src/htrace/formatters/htrace_flame_json.cpp
    src/formatter/html/html_analysis_common.cpp
    src/formatter/html/analysis_html_formatter_base.cpp
    src/tshandle/formatters/tshandle_formatter.cpp
    src/tshandle/formatters/tshandle_html_formatter.cpp
    src/tshandle/formatters/tshandle_call_graph.cpp
    src/tshandle/formatters/tshandle_analysis_json_formatter.cpp
    src/formatter/html/components/navigation_bar.cpp
    src/formatter/html/components/chart_module.cpp
    src/formatter/html/components/help_tooltip.cpp
    src/formatter/html/components/version.cpp
    src/formatter/html/components/tab_bar.cpp
    src/formatter/html/report_skeleton.cpp
    src/shared/heapsnapshot/heap_snapshot.cpp
    src/shared/heapsnapshot/node_size_calculator.cpp
    src/tshandle/map_parser.cpp
    src/jsleak/snapshot_parser.cpp
    src/jsleak/jsleak_module.cpp
    src/shared/heapsnapshot/source_map.cpp
    src/shared/heapsnapshot/name_cache.cpp
    src/shared/heapsnapshot/deobfuscator.cpp
    src/shared/heapsnapshot/rawheap_converter.cpp
    src/tshandle/arkts_analyzer.cpp
    src/tshandle/tshandle_analyzer.cpp
    src/tshandle/tshandle_retained_size.cpp
    src/profiler/config_loader.cpp
    src/profiler/process_utils.cpp
    src/profiler/template_processor.cpp
    src/profiler/hdc_runner.cpp
    src/profiler/hdc_target.cpp
    src/profiler/profiler_cmd.cpp
    src/config_mgr/htrace_config_mgr.cpp
    src/config_mgr/preview_features_mgr.cpp
    src/utils/zip_utils.cpp
    # zlib（inflate 解压）+ minizip（API 解压 ZIP，替代 shell 调用 unzip/Expand-Archive）
    ${ZLIB_LIB}/adler32.c
    ${ZLIB_LIB}/crc32.c
    ${ZLIB_LIB}/inflate.c
    ${ZLIB_LIB}/inffast.c
    ${ZLIB_LIB}/inftrees.c
    ${ZLIB_LIB}/zutil.c
    ${MINIZIP_DIR}/ioapi.c
    ${MINIZIP_DIR}/unzip.c
    ${SQLITE_SRC}
)

if(WIN32)
    list(APPEND SOURCES ${MINIZIP_DIR}/iowin32.c)
endif()

# ── Include paths ──────────────────────────────────────────────
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLITE_INC}
    ${JSON_INC}
    ${ZLIB_INC}
    ${MINIZIP_INC}
)

add_executable(mem-analyzer ${SOURCES})

# ── Compiler flags ────────────────────────────────────────────
if(MSVC)
    target_compile_definitions(mem-analyzer PRIVATE
        CRT_SECURE_NO_WARNINGS
        SQLITE_THREADSAFE=0
        SQLITE_OMIT_LOAD_EXTENSION)
    target_compile_options(mem-analyzer PRIVATE /utf-8)
elseif(APPLE)
    target_compile_definitions(mem-analyzer PRIVATE
        SQLITE_THREADSAFE=0
        SQLITE_OMIT_LOAD_EXTENSION)
elseif(MINGW)
    target_compile_definitions(mem-analyzer PRIVATE
        SQLITE_THREADSAFE=0
        SQLITE_OMIT_LOAD_EXTENSION)
    target_link_options(mem-analyzer PRIVATE
        -static-libstdc++ -static-libgcc)
    # Note: for fully static link (including libpthread), pass
    # -DCMAKE_EXE_LINKER_FLAGS=-static from the release script.
else()
    # Linux / other — fully static linking (no glibc runtime dependency)
    target_compile_definitions(mem-analyzer PRIVATE
        SQLITE_THREADSAFE=0
        SQLITE_OMIT_LOAD_EXTENSION)
    target_link_options(mem-analyzer PRIVATE -static)
endif()

if(NOT (UNIX AND NOT APPLE AND NOT MINGW))
    target_link_libraries(mem-analyzer PRIVATE ${CMAKE_DL_LIBS})
endif()

# ── C++17 filesystem compatibility ────────────────────────────
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12") OR
   (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9"))
    target_link_libraries(mem-analyzer PRIVATE stdc++fs)
endif()

# ── macOS-specific ────────────────────────────────────────────
if(APPLE)
    # std::filesystem requires macOS 10.15+
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
endif()

# ── Install ────────────────────────────────────────────────────
install(TARGETS mem-analyzer RUNTIME DESTINATION bin)
install(DIRECTORY config/profiler DESTINATION config
    FILES_MATCHING PATTERN "*.json" PATTERN "*.txt")
install(DIRECTORY config/htrace DESTINATION config
    FILES_MATCHING PATTERN "*.json")
install(DIRECTORY config/jsleak DESTINATION config
    FILES_MATCHING PATTERN "*.json")
