#!/usr/bin/env bash
#
# Copyright (c) Huawei Technologies Co., Ltd. 2021. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# mem-analyzer 跨平台交叉编译 & 发布脚本
# 用法: ./release-cross.sh
#
# 版本号自动从 src/utils/version.cpp 提取。
#
# 功能:
#   1) Linux (x86_64)  — 原生编译
#   2) Windows (x64)   — MinGW 交叉编译（需安装 mingw-w64）
#
# 每个平台生成独立的 ZIP 发布包，包含对应的 trace_streamer 和 rawheap_translator。
#

set -euo pipefail
cd "$(dirname "$0")"
PROJECT_ROOT="$(cd .. && pwd)"

# ── 颜色 ────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info()  { echo -e "${GREEN}[INFO]${NC} $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; }

# ── 环境检查 ────────────────────────────────
check_env() {
    local failed=false
    echo "----------------------------------------"
    echo " Checking build environment..."
    echo "----------------------------------------"

    # cmake
    if ! command -v cmake &>/dev/null; then
        error "cmake not found!"
        echo "  Install CMake 3.16+:"
        echo "    sudo apt install cmake        # Debian/Ubuntu"
        echo "    sudo dnf install cmake        # Fedora"
        echo "    sudo pacman -S cmake          # Arch"
        failed=true
    else
        local cmake_ver=$(cmake --version | head -1 | grep -oP '\d+\.\d+')
        info "cmake: $(command -v cmake) (version $(cmake --version | head -1))"
        if awk "BEGIN {exit !($cmake_ver < 3.16)}"; then
            warn "  cmake version < 3.16, may not support C++17 properly"
        fi
    fi

    # C++ compiler
    local cxx=""
    for candidate in g++ clang++; do
        if command -v "$candidate" &>/dev/null; then
            cxx="$candidate"
            break
        fi
    done
    if [ -z "$cxx" ]; then
        error "C++ compiler (g++ or clang++) not found!"
        echo "  Install GCC:"
        echo "    sudo apt install g++ build-essential   # Debian/Ubuntu"
        echo "    sudo dnf install gcc-c++               # Fedora"
        failed=true
    else
        info "C++ compiler: $(command -v $cxx)"
    fi

    # Third-party binary dependencies (non-fatal warnings)
    for ts in "trace_streamer_linux_static" "trace_streamer_windows.exe"; do
        local ts_path="${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/${ts}"
        [ -f "$ts_path" ] || warn "trace_streamer_binary/${ts} not found (will be skipped at package step)"
    done
    for rh in "linux-x64/rawheap_translator" "windows-x64/rawheap_translator.exe"; do
        local rh_path="${PROJECT_ROOT}/third_party/bin/rawheaptranslator/${rh}"
        [ -f "$rh_path" ] || warn "rawheaptranslator/${rh} not found (will be skipped at package step)"
    done

    if $failed; then
        echo ""
        echo "----------------------------------------"
        exit 1
    fi

    echo "----------------------------------------"
    echo ""
}

check_env

# ── 版本号（从 version.cpp 自动提取）────────
VERSION=$(grep -oP 'return\s+"\K[^"]+' src/utils/version.cpp || echo "1.0.0")
info "Version: $VERSION"

RELEASE_DIR_BASE="mem-analyzer-${VERSION}"

CMAKE_OPTS="-DCMAKE_BUILD_TYPE=Release"
JOBS=$(nproc 2>/dev/null || echo 4)

# ── Step 0: Download dependencies ─────────────
info "[0] Downloading dependencies (setup_deps.py)..."
PY=""
if command -v python3 &>/dev/null; then PY=python3; elif command -v python &>/dev/null; then PY=python; fi
if [ -z "$PY" ]; then
    error "python/python3 not found, required for setup_deps.py"
    exit 1
fi
if ! "$PY" "$PROJECT_ROOT/build/setup_deps.py" --mode release; then
    error "Dependency download failed. Aborting."
    exit 1
fi
info "  Done."
echo ""

# Verify third-party source dependencies after setup_deps
sqlite3="${PROJECT_ROOT}/third_party/src/third_party_sqlite/src/sqlite3.c"
json_inc="${PROJECT_ROOT}/third_party/src/third_party_json/single_include"
if [ ! -f "$sqlite3" ] || [ ! -d "$json_inc" ]; then
    error "Third-party source dependencies still missing after setup_deps!"
    exit 1
fi
info "Source dependencies: all OK"

# ── 辅助：干净构建（删除旧目录 → 创建）───
clean_build_dir() {
    local dir="$1"
    if [ -d "$dir" ]; then
        rm -rf "$dir"
        info "  Cleaned existing build: $dir"
    fi
    mkdir -p "$dir"
}

# ── 辅助：打包单个平台 ──────────────────────
package_platform() {
    local plat="$1"       # linux / win64
    local bin_name="$2"   # "" or ".exe" (PowerShell style, unused in bash)
    local ts_name="$3"    # trace_streamer_linux_static / trace_streamer_windows.exe
    local ts_lib_dir="$4" # "" or "lib"
    local rh_dir="$5"     # linux-x64 / windows-x64
    local build_dir="$6"  # 构建输出目录

    local release_dir="${RELEASE_DIR_BASE}-${plat}"
    info "=== Packaging for ${plat} → ${release_dir} ==="

    rm -rf "${release_dir}"
    mkdir -p "${release_dir}"
    mkdir -p "${release_dir}/config/profiler"
    mkdir -p "${release_dir}/config/htrace"
    mkdir -p "${release_dir}/config/jsleak"
    mkdir -p "${release_dir}/third_party/bin/trace_streamer_binary"
    mkdir -p "${release_dir}/third_party/bin/rawheaptranslator/${rh_dir}"

    # C++ 可执行文件
    if [ -f "${build_dir}/mem-analyzer.exe" ]; then
        cp "${build_dir}/mem-analyzer.exe" "${release_dir}/mem-analyzer.exe"
        info "  + mem-analyzer.exe"
    elif [ -f "${build_dir}/mem-analyzer" ]; then
        cp "${build_dir}/mem-analyzer" "${release_dir}/mem-analyzer"
        info "  + mem-analyzer"
    else
        error "mem-analyzer binary not found in ${build_dir}"
        return 1
    fi

    # usage.md 使用手册：打包时将 webp 引用替换为同名 png（本地包体积小）；
    # 线上 git 中的 docs/usage.md 与 webp 原样保留，保持动画效果。
    if [ -f "${PROJECT_ROOT}/docs/usage.md" ]; then
        sed 's/\.webp/.png/g' "${PROJECT_ROOT}/docs/usage.md" > "${release_dir}/usage.md"
        info "  + usage.md (webp refs -> png)"
    else
        warn "  docs/usage.md not found, skipping"
    fi

    # images 目录：本地包只带 png（与替换后的 usage.md 引用一致），webp 不进包
    if [ -d "${PROJECT_ROOT}/docs/images" ]; then
        mkdir -p "${release_dir}/images"
        if ls "${PROJECT_ROOT}/docs/images/"*.png >/dev/null 2>&1; then
            cp "${PROJECT_ROOT}/docs/images/"*.png "${release_dir}/images/"
            info "  + images/ (*.png only, webp excluded)"
        else
            warn "  No .png in docs/images, falling back to copying all images"
            cp -r "${PROJECT_ROOT}/docs/images" "${release_dir}/images"
        fi
    else
        warn "  docs/images/ not found, skipping"
    fi

    # Profiler config 目录（完整拷贝，包含 json 和所有 cmd 模板 txt）
    PROFILER_CONFIG_SRC_DIR="config/profiler"
    if [ -d "${PROFILER_CONFIG_SRC_DIR}" ]; then
        cp -R "${PROFILER_CONFIG_SRC_DIR}/"* "${release_dir}/config/profiler/"
        info "  + config/profiler/ (profiler config + cmd templates)"
    else
        warn "  config/profiler/ directory not found, skipping"
    fi

    # Htrace config 目录
    HTRACE_CONFIG_SRC_DIR="config/htrace"
    if [ -d "${HTRACE_CONFIG_SRC_DIR}" ]; then
        cp -R "${HTRACE_CONFIG_SRC_DIR}/"* "${release_dir}/config/htrace/"
        info "  + config/htrace/ (htrace analysis config)"
    fi

    # Jsleak config 目录
    JSLEAK_CONFIG_SRC_DIR="config/jsleak"
    if [ -d "${JSLEAK_CONFIG_SRC_DIR}" ]; then
        cp -R "${JSLEAK_CONFIG_SRC_DIR}/"* "${release_dir}/config/jsleak/"
        info "  + config/jsleak/ (jsleak property tag config)"
    fi

    # trace_streamer
    local ts_src="${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/${ts_name}"
    if [ -f "${ts_src}" ]; then
        cp "${ts_src}" "${release_dir}/third_party/bin/trace_streamer_binary/"
        info "  + third_party/bin/trace_streamer_binary/${ts_name}"
    else
        warn "  trace_streamer_binary/${ts_name} not found, skipping"
    fi

    # trace_streamer 扩展文件
    if [ ! -z "${ts_lib_dir}" ] && [ -d "${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/${ts_lib_dir}" ]; then
        cp -r "${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/${ts_lib_dir}" "${release_dir}/third_party/bin/trace_streamer_binary/"
        info "  + third_party/bin/trace_streamer_binary/${ts_lib_dir}/"
    fi

    # trace_streamer config
    if [ -f "${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/config/config.json" ]; then
        mkdir -p "${release_dir}/third_party/bin/trace_streamer_binary/config"
        cp "${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/config/config.json" "${release_dir}/third_party/bin/trace_streamer_binary/config/"
        info "  + third_party/bin/trace_streamer_binary/config/config.json"
    fi

    # rawheap_translator
    local rh_src=""
    if [ "${plat}" = "win64" ]; then
        rh_src="${PROJECT_ROOT}/third_party/bin/rawheaptranslator/${rh_dir}/rawheap_translator.exe"
    else
        rh_src="${PROJECT_ROOT}/third_party/bin/rawheaptranslator/${rh_dir}/rawheap_translator"
    fi
    if [ -f "${rh_src}" ]; then
        cp "${rh_src}" "${release_dir}/third_party/bin/rawheaptranslator/${rh_dir}/"
        info "  + third_party/bin/rawheaptranslator/${rh_dir}/$(basename ${rh_src})"
    else
        warn "  rawheaptranslator/${rh_dir}/rawheap_translator not found, skipping"
    fi

    # OSS 开源信息（json/sqlite/zlib；trace_streamer/rawheap 待上游许可证确认后补充）
    mkdir -p "${release_dir}/third_party/licenses/json"
    mkdir -p "${release_dir}/third_party/licenses/sqlite"
    mkdir -p "${release_dir}/third_party/licenses/zlib"
    cp "${PROJECT_ROOT}/third_party/src/third_party_json/README.OpenSource" "${release_dir}/third_party/licenses/json/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_json/LICENSE.MIT"      "${release_dir}/third_party/licenses/json/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_sqlite/README.OpenSource" "${release_dir}/third_party/licenses/sqlite/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_sqlite/LICENSE"           "${release_dir}/third_party/licenses/sqlite/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_zlib/README.OpenSource" "${release_dir}/third_party/licenses/zlib/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_zlib/LICENSE"          "${release_dir}/third_party/licenses/zlib/"
    info "  + third_party/licenses/ (json, sqlite, zlib OSS info)"

    # 生成聚合 NOTICE（失败则终止打包，保证开源合规信息完整）
    local PY=""
    if command -v python3 &>/dev/null; then PY=python3; elif command -v python &>/dev/null; then PY=python; fi
    if [ -z "${PY}" ]; then
        error "python/python3 not found, required for NOTICE generation"
        exit 1
    fi
    "${PY}" "${PROJECT_ROOT}/build/gen_notice.py" --dest "${release_dir}/NOTICE" \
        "${PROJECT_ROOT}/third_party/src/third_party_json" \
        "${PROJECT_ROOT}/third_party/src/third_party_sqlite" \
        "${PROJECT_ROOT}/third_party/src/third_party_zlib" || {
        error "NOTICE generation failed, aborting packaging"
        exit 1
    }

    # 打包（Linux 用 tar.gz，Windows 用 zip）
    local archive=""
    if [ "${plat}" = "win64" ]; then
        info "  Creating ZIP..."
        if command -v zip &>/dev/null; then
            (cd "${release_dir}" && zip -r "../${release_dir}.zip" .) >/dev/null
        else
            info "  (zip not found, using PowerShell Compress-Archive)"
            powershell -NoProfile -Command "Compress-Archive -Path '${release_dir}' -DestinationPath '${release_dir}.zip' -Force"
        fi
        archive="${release_dir}.zip"
        info "  Created: ${archive}"
    else
        info "  Creating tar.gz..."
        archive="${release_dir}.tar.gz"
        tar -czf "${archive}" "${release_dir}"
        info "  Created: ${archive}"
    fi

    # 计算 SHA-256 校验和，写 <archive>.sha256（sha256sum 兼容格式）
    if ! command -v sha256sum &>/dev/null; then
        error "sha256sum not found, required for checksum generation"
        return 1
    fi
    local hash
    hash=$(sha256sum "${archive}" | awk '{print $1}')
    printf '%s  %s\n' "${hash}" "$(basename "${archive}")" > "${archive}.sha256"
    info "  SHA256:   ${hash}"
    info "  Checksum: ${archive}.sha256"
    echo ""
}

# ============================================
#  1) Linux x86_64 — 原生编译（干净构建）
# ============================================
info "========================================"
info "  [1] Linux x86_64 (native)"
info "========================================"
BUILD_LINUX="build/linux-release"
clean_build_dir "${BUILD_LINUX}"
cmake -B "${BUILD_LINUX}" -S . ${CMAKE_OPTS}
cmake --build "${BUILD_LINUX}" --parallel "${JOBS}"
package_platform "linux" "" "trace_streamer_linux_static" "" "linux-x64" "${BUILD_LINUX}"

# ============================================
#  2) Windows x64 — MinGW 交叉编译（干净构建）
# ============================================
info "========================================"
info "  [2] Windows x64 (MinGW cross-compile)"
info "========================================"
BUILD_WIN64="build/win64-release"

# Prefer posix-threading MinGW (required for <thread>, <mutex>);
# fall back to default if posix variant not found.
MINGW_CXX=""
for candidate in x86_64-w64-mingw32-g++-posix x86_64-w64-mingw32-g++; do
    if command -v "${candidate}" &>/dev/null; then
        MINGW_CXX="${candidate}"
        break
    fi
done
MINGW_CC="${MINGW_CXX/g++/gcc}"

if [ -n "${MINGW_CXX}" ]; then
    info "Using MinGW CXX: ${MINGW_CXX}"
    TOOLCHAIN_FILE="build/toolchain-mingw64.cmake"
    cat > "${TOOLCHAIN_FILE}" << TCEOF
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_C_COMPILER   ${MINGW_CC})
set(CMAKE_CXX_COMPILER ${MINGW_CXX})
set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
TCEOF

    clean_build_dir "${BUILD_WIN64}"
    cmake -B "${BUILD_WIN64}" -S . ${CMAKE_OPTS} \
        -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
        -DCMAKE_EXE_LINKER_FLAGS="-static"
    cmake --build "${BUILD_WIN64}" --parallel "${JOBS}"
    package_platform "win64" "" "trace_streamer_windows.exe" "" "windows-x64" "${BUILD_WIN64}"
else
    warn "MinGW cross-compiler (${MINGW_CXX}) not found. Skipping Windows build."
    warn "  Install with: sudo apt install g++-mingw-w64-x86-64 mingw-w64-tools"
fi

# ============================================
#  完成
# ============================================
echo ""
info "========================================"
info "  Done! Release packages:"
ls -lh ${RELEASE_DIR_BASE}-*.zip ${RELEASE_DIR_BASE}-*.tar.gz 2>/dev/null || true
info "========================================"
