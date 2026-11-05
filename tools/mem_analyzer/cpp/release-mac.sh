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
# mem-analyzer macOS native release packaging script.
#
# 用法:
#   ./release-mac.sh                                  # 默认同时构建 arm64 + x86_64
#   ./release-mac.sh --arch arm64                     # 仅构建 arm64
#   ./release-mac.sh --arch x86_64                    # 仅构建 x86_64
#   ./release-mac.sh --arch all                       # 同时构建 arm64 + x86_64
#   ./release-mac.sh --cmake-path /path/to/cmake      # 指定 cmake 路径
#   ./release-mac.sh --arch arm64 --cmake-path ...    # 组合使用
#
# 版本号自动从 src/utils/version.cpp 提取。
#
# 功能:
#   1. 检查编译环境 (cmake + Xcode CLT + 三方依赖)
#   2. 原生编译 mem-analyzer (Release)，支持单/双架构
#   3. 打包 mem-analyzer + trace_streamer + rawheap_translator → ZIP
#

set -euo pipefail
cd "$(dirname "$0")"
PROJECT_ROOT="$(cd .. && pwd)"

# ── 颜色 ────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

info()  { echo -e "${GREEN}[INFO]${NC} $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; }

# ── 参数解析 ────────────────────────────────
CMAKE_PATH=""
ARCH=""

while [ $# -gt 0 ]; do
    case "$1" in
        --cmake-path)
            if [ -z "${2:-}" ]; then error "--cmake-path requires a value"; exit 1; fi
            CMAKE_PATH="$2"
            shift 2
            ;;
        --arch)
            if [ -z "${2:-}" ]; then error "--arch requires a value"; exit 1; fi
            ARCH="$2"
            shift 2
            ;;
        -h|--help)
            echo "Usage: $0 [--arch arm64|x86_64|all] [--cmake-path /path/to/cmake]"
            exit 0
            ;;
        *)
            error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# 确定要构建的架构列表
if [ -z "$ARCH" ]; then
    # 默认：同时构建 arm64 和 x86_64
    ARCHS=("arm64" "x86_64")
elif [ "$ARCH" = "all" ]; then
    ARCHS=("arm64" "x86_64")
else
    # 单架构模式：验证架构
    case "$ARCH" in
        arm64|aarch64|x86_64) ;;
        *) error "Unsupported architecture: $ARCH"; exit 1 ;;
    esac
    # 统一 arm64 别名
    if [ "$ARCH" = "aarch64" ]; then ARCH="arm64"; fi
    ARCHS=("$ARCH")
fi

# 为 check_env 预先设置 RH_DIR（取第一个架构的目录名）
case "${ARCHS[0]}" in
    arm64)  RH_DIR="mac-arm64";  ;;
    x86_64) RH_DIR="mac-x64";    ;;
esac

info "Target architectures: ${ARCHS[*]}"

# ── 环境检查 ────────────────────────────────
check_env() {
    local failed=false
    echo "----------------------------------------"
    echo " Checking build environment..."
    echo "----------------------------------------"

    # cmake
    local cmake_exe=""
    if [ -n "$CMAKE_PATH" ]; then
        if [ -f "$CMAKE_PATH" ]; then
            cmake_exe="$CMAKE_PATH"
        elif [ -f "${CMAKE_PATH}/cmake" ]; then
            cmake_exe="${CMAKE_PATH}/cmake"
        elif [ -d "$CMAKE_PATH" ] && [ -f "${CMAKE_PATH}/cmake" ]; then
            cmake_exe="${CMAKE_PATH}/cmake"
        else
            error "Specified cmake path not found: $CMAKE_PATH"
            failed=true
        fi
    elif command -v cmake &>/dev/null; then
        cmake_exe=$(command -v cmake)
    fi

    if [ -z "$cmake_exe" ]; then
        error "cmake not found!"
        echo "  Install CMake:"
        echo "    brew install cmake"
        echo "    or download from https://cmake.org/download/"
        echo "  Or specify --cmake-path /path/to/cmake"
        failed=true
    else
        CMAKE_EXE="$cmake_exe"
        local cmake_ver
        cmake_ver=$("$CMAKE_EXE" --version | head -1 | grep -oE '\d+\.\d+' | head -1)
        info "cmake: $CMAKE_EXE (version $("$CMAKE_EXE" --version | head -1))"
        if awk "BEGIN {exit !($cmake_ver < 3.19)}" 2>/dev/null; then
            warn "  cmake version < 3.19, may not support C++17 properly"
        fi
    fi

    # Xcode Command Line Tools
    if ! xcode-select -p &>/dev/null; then
        error "Xcode Command Line Tools not found!"
        echo "  Install: xcode-select --install"
        failed=true
    else
        info "Xcode CLT: $(xcode-select -p)"
        local xcode_ver
        xcode_ver=$(xcodebuild -version 2>/dev/null | head -1 || echo "unknown")
        info "  $xcode_ver"

        if ! command -v clang++ &>/dev/null; then
            error "clang++ not found!"
            echo "  Xcode CLT should provide clang++. Try: xcode-select --install"
            failed=true
        else
            info "clang++: $(command -v clang++)"
        fi
    fi

    # Third-party binary dependencies (non-fatal warnings)
    local ts_path="${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/trace_streamer_mac"
    local ts_lib="${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/lib"
    local rh_path="${PROJECT_ROOT}/third_party/bin/rawheaptranslator/${RH_DIR}/rawheap_translator"

    # 检查 LFS 文件是否为实际二进制（而非 LFS pointer 文本）
    check_lfs_binary() {
        local path="$1"
        local name="$2"
        if [ ! -f "$path" ]; then
            warn "${name} not found (will be skipped at package step)"
            return 1
        fi
        # LFS pointer 文件以 "version https://git-lfs.github.com/spec/v1" 开头
        if head -c 60 "$path" 2>/dev/null | grep -q "^version https://git-lfs.github.com/spec/v1"; then
            error "${name} is a Git LFS pointer file, not the actual binary!"
            echo "  Run: git lfs pull  (requires git-lfs)"
            echo "  See README.md → macOS 构建注意事项 → Git LFS 依赖"
            return 2
        fi
        info "  ${name}: OK ($(file -b "$path" | cut -d, -f1))"
        return 0
    }

    # — 先检查 git-lfs 是否可用 —
    local lfs_available=false
    if command -v git-lfs &>/dev/null || git config --get filter.lfs.clean &>/dev/null; then
        lfs_available=true
    fi

    check_lfs_binary "$ts_path" "trace_streamer_mac" || {
        local rc=$?
        [ $rc -eq 2 ] && failed=true
    }
    [ -d "$ts_lib" ] || warn "trace_streamer_binary/lib/ not found (will be skipped at package step)"
    check_lfs_binary "$rh_path" "rawheap_translator (${RH_DIR})" || {
        local rc=$?
        [ $rc -eq 2 ] && failed=true
    }

    if ! $lfs_available; then
        warn "git-lfs not found in PATH or git config"
        echo "  If LFS files show as pointers, run: git lfs pull"
        echo "  See README.md → macOS 构建注意事项 → Git LFS 依赖"
    fi

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
VERSION=$(sed -n 's/.*return "\([^"]*\)".*/\1/p' src/utils/version.cpp | head -1 || echo "1.0.0")
info "Version: $VERSION"

JOBS=$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)

# ── Step 0: Download dependencies ─────────────
info "[0/4] Downloading dependencies (setup_deps.py)..."
if ! python3 "$PROJECT_ROOT/build/setup_deps.py" --mode release; then
    error "Dependency download failed. Aborting."
    exit 1
fi
info "  Done."
echo ""

# Verify third-party source dependencies after setup_deps
if [ ! -f "${PROJECT_ROOT}/third_party/src/third_party_sqlite/src/sqlite3.c" ] || \
   [ ! -d "${PROJECT_ROOT}/third_party/src/third_party_json/single_include" ]; then
    error "Third-party source dependencies still missing after setup_deps!"
    exit 1
fi
info "Source dependencies: all OK"

for CURRENT_ARCH in "${ARCHS[@]}"; do
    # 架构映射
    case "$CURRENT_ARCH" in
        arm64)  ARCH_TAG="mac-arm64";  RH_DIR="mac-arm64";  ;;
        x86_64) ARCH_TAG="mac-x64";    RH_DIR="mac-x64";    ;;
    esac

    RELEASE_DIR="mem-analyzer-${VERSION}-${ARCH_TAG}"
    BUILD_DIR="build/${ARCH_TAG}-release"

    echo ""
    echo "========================================"
    echo " mem-analyzer macOS release build"
    echo "========================================"
    echo "  Version:   $VERSION"
    echo "  Platform:  $ARCH_TAG"
    echo "  Output:    ${RELEASE_DIR}.zip"
    echo "  Jobs:      $JOBS"
    echo ""

    # ── Step 1: Clean previous build ──────────────
    info "[1/4] Cleaning previous build..."
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        info "  Removed existing build directory."
    fi

    # ── Step 2: CMake Configure + Build ───────────
    info "[2/4] Building mem-analyzer (Release)..."
    CMAKE_ARCH_FLAG=""
    if [ "$ARCH_TAG" = "mac-arm64" ]; then
        CMAKE_ARCH_FLAG="-DCMAKE_OSX_ARCHITECTURES=arm64"
    else
        CMAKE_ARCH_FLAG="-DCMAKE_OSX_ARCHITECTURES=x86_64"
    fi

    "$CMAKE_EXE" -B "$BUILD_DIR" -S . \
        -DCMAKE_BUILD_TYPE=Release \
        $CMAKE_ARCH_FLAG

    "$CMAKE_EXE" --build "$BUILD_DIR" --parallel "$JOBS"
    info "  Done."

    # ── Step 3: Create release directory ──────────
    info "[3/4] Creating release directory structure..."
    rm -rf "${RELEASE_DIR}"
    mkdir -p "${RELEASE_DIR}"
    mkdir -p "${RELEASE_DIR}/config/profiler"
    mkdir -p "${RELEASE_DIR}/config/htrace"
    mkdir -p "${RELEASE_DIR}/config/jsleak"
    mkdir -p "${RELEASE_DIR}/third_party/bin/trace_streamer_binary/config"
    mkdir -p "${RELEASE_DIR}/third_party/bin/rawheaptranslator/${RH_DIR}"
    mkdir -p "${RELEASE_DIR}/third_party/licenses/json"
    mkdir -p "${RELEASE_DIR}/third_party/licenses/sqlite"
    mkdir -p "${RELEASE_DIR}/third_party/licenses/zlib"

    # 文件映射表：src → dst（相对于 RELEASE_DIR）
    # 格式：src_path|dst_relpath|is_executable(true/false)
    FILE_MAPPINGS=(
        "${BUILD_DIR}/mem-analyzer|mem-analyzer|true"
        "${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/trace_streamer_mac|third_party/bin/trace_streamer_binary/trace_streamer_mac|true"
        "${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/config/config.json|third_party/bin/trace_streamer_binary/config/config.json|false"
        "${PROJECT_ROOT}/third_party/bin/rawheaptranslator/${RH_DIR}/rawheap_translator|third_party/bin/rawheaptranslator/${RH_DIR}/rawheap_translator|true"
    )

    COPY_ERRORS=0
    for entry in "${FILE_MAPPINGS[@]}"; do
        IFS='|' read -r src dst is_exec <<< "$entry"
        if [ -f "$src" ]; then
            cp "$src" "${RELEASE_DIR}/${dst}"
            if [ "$is_exec" = "true" ]; then
                chmod +x "${RELEASE_DIR}/${dst}"
            fi
            info "  + ${dst}"
        else
            warn "  MISSING: ${src}"
            COPY_ERRORS=$((COPY_ERRORS + 1))
        fi
    done

    # Profiler config 目录（完整拷贝，包含 json 和所有 cmd 模板 txt）
    PROFILER_CONFIG_SRC_DIR="config/profiler"
    if [ -d "${PROFILER_CONFIG_SRC_DIR}" ]; then
        cp -R "${PROFILER_CONFIG_SRC_DIR}/"* "${RELEASE_DIR}/config/profiler/"
        info "  + config/profiler/ (profiler config + cmd templates)"
    else
        warn "  MISSING: ${PROFILER_CONFIG_SRC_DIR}/ (profiler config directory not found)"
        COPY_ERRORS=$((COPY_ERRORS + 1))
    fi

    # Htrace config 目录
    HTRACE_CONFIG_SRC_DIR="config/htrace"
    if [ -d "${HTRACE_CONFIG_SRC_DIR}" ]; then
        cp -R "${HTRACE_CONFIG_SRC_DIR}/"* "${RELEASE_DIR}/config/htrace/"
        info "  + config/htrace/ (htrace analysis config)"
    fi

    # Jsleak config 目录
    JSLEAK_CONFIG_SRC_DIR="config/jsleak"
    if [ -d "${JSLEAK_CONFIG_SRC_DIR}" ]; then
        cp -R "${JSLEAK_CONFIG_SRC_DIR}/"* "${RELEASE_DIR}/config/jsleak/"
        info "  + config/jsleak/ (jsleak property tag config)"
    fi

    # usage.md: 打包时将 webp 引用替换为同名 png（本地包体积小）；
    # 线上 git 中的 docs/usage.md 与 webp 原样保留，保持动画效果。
    if [ -f "${PROJECT_ROOT}/docs/usage.md" ]; then
        sed 's/\.webp/.png/g' "${PROJECT_ROOT}/docs/usage.md" > "${RELEASE_DIR}/usage.md"
        info "  + usage.md (webp refs -> png)"
    else
        warn "  MISSING: ${PROJECT_ROOT}/docs/usage.md"
        COPY_ERRORS=$((COPY_ERRORS + 1))
    fi

    # images 目录: 本地包只带 png（与替换后的 usage.md 引用一致），webp 不进包
    if [ -d "${PROJECT_ROOT}/docs/images" ]; then
        mkdir -p "${RELEASE_DIR}/images"
        if ls "${PROJECT_ROOT}/docs/images/"*.png >/dev/null 2>&1; then
            cp "${PROJECT_ROOT}/docs/images/"*.png "${RELEASE_DIR}/images/"
            info "  + images/ (*.png only, webp excluded)"
        else
            warn "  No .png in docs/images, falling back to copying all images"
            cp -R "${PROJECT_ROOT}/docs/images" "${RELEASE_DIR}/images"
            COPY_ERRORS=$((COPY_ERRORS + 1))
        fi
    else
        warn "  MISSING: ${PROJECT_ROOT}/docs/images/ (images directory not found)"
        COPY_ERRORS=$((COPY_ERRORS + 1))
    fi

    # trace_streamer dylibs 目录 (macOS 特有，目录拷贝单独处理)
    if [ -d "${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/lib" ]; then
        cp -R "${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/lib" "${RELEASE_DIR}/third_party/bin/trace_streamer_binary/"
        info "  + third_party/bin/trace_streamer_binary/lib/ (dylibs)"
    else
        warn "  MISSING: ${PROJECT_ROOT}/third_party/bin/trace_streamer_binary/lib/ (dylibs)"
        COPY_ERRORS=$((COPY_ERRORS + 1))
    fi

    # OSS 开源信息（json/sqlite/zlib；trace_streamer/rawheap 待上游许可证确认后补充）
    cp "${PROJECT_ROOT}/third_party/src/third_party_json/README.OpenSource" "${RELEASE_DIR}/third_party/licenses/json/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_json/LICENSE.MIT"      "${RELEASE_DIR}/third_party/licenses/json/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_sqlite/README.OpenSource" "${RELEASE_DIR}/third_party/licenses/sqlite/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_sqlite/LICENSE"           "${RELEASE_DIR}/third_party/licenses/sqlite/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_zlib/README.OpenSource" "${RELEASE_DIR}/third_party/licenses/zlib/"
    cp "${PROJECT_ROOT}/third_party/src/third_party_zlib/LICENSE"          "${RELEASE_DIR}/third_party/licenses/zlib/"
    info "  + third_party/licenses/ (json, sqlite, zlib OSS info)"

    # 生成聚合 NOTICE（失败则终止打包，保证开源合规信息完整）
    PY=""
    if command -v python3 &>/dev/null; then PY=python3; elif command -v python &>/dev/null; then PY=python; fi
    if [ -z "$PY" ]; then
        error "python/python3 not found, required for NOTICE generation"
        exit 1
    fi
    "$PY" "${PROJECT_ROOT}/build/gen_notice.py" --dest "${RELEASE_DIR}/NOTICE" \
        "${PROJECT_ROOT}/third_party/src/third_party_json" \
        "${PROJECT_ROOT}/third_party/src/third_party_sqlite" \
        "${PROJECT_ROOT}/third_party/src/third_party_zlib" || {
        error "NOTICE generation failed, aborting packaging"
        exit 1
    }

    if [ $COPY_ERRORS -gt 0 ]; then
        warn "  $COPY_ERRORS file(s) were missing, continuing..."
    fi

    info "  Done."

    # ── Step 4: Create ZIP archive ───────────────
    info "[4/4] Creating ZIP archive..."

    # 移除旧 ZIP
    if [ -f "${RELEASE_DIR}.zip" ]; then
        rm -f "${RELEASE_DIR}.zip"
    fi

    if command -v zip &>/dev/null; then
        (cd "${RELEASE_DIR}" && zip -r "../${RELEASE_DIR}.zip" .) >/dev/null
        ZIP_FILE="${RELEASE_DIR}.zip"
    else
        tar -czf "${RELEASE_DIR}.tar.gz" "${RELEASE_DIR}"
        ZIP_FILE="${RELEASE_DIR}.tar.gz"
        info "  (zip not found, created tar.gz instead)"
    fi

    if [ -f "$ZIP_FILE" ]; then
        ZIP_SIZE=$(du -h "$ZIP_FILE" | cut -f1)
        ZIP_BYTES=$(stat -f%z "$ZIP_FILE" 2>/dev/null || stat -c%s "$ZIP_FILE" 2>/dev/null || echo "?")
        info "  Created: $ZIP_FILE (${ZIP_SIZE}, ${ZIP_BYTES} bytes)"

        # 计算 SHA-256 校验和，写 <archive>.sha256（shasum 兼容格式）
        if ! command -v shasum &>/dev/null; then
            error "shasum not found, required for checksum generation"
            exit 1
        fi
        SHA_HASH=$(shasum -a 256 "$ZIP_FILE" | awk '{print $1}')
        printf '%s  %s\n' "$SHA_HASH" "$(basename "$ZIP_FILE")" > "$ZIP_FILE.sha256"
        info "  SHA256:   $SHA_HASH"
        info "  Checksum: $ZIP_FILE.sha256"
    fi

    echo ""
    info "========================================"
    info "  Release package created successfully!"
    info "  ZIP: $ZIP_FILE"
    info "  Size: ${ZIP_SIZE}"
    info "========================================"

done

echo ""
info "All architectures built successfully!"