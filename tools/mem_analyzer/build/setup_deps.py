#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""
Build environment initialization script for mem-analyzer (C++ engine).

Downloads and verifies all third-party dependencies:
  - nlohmann/json         (header-only, git clone + commit check)
  - SQLite3 amalgamation  (single-file, git clone + tag check)
  - trace_streamer        (platform binary)
  - rawheap_translator    (platform binary)

Usage:
  python build/setup_deps.py                         # cache mode (默认): 增量检查，缺失则下载
  python build/setup_deps.py --mode release           # release 模式: 删除全部依赖重新下载，并执行 CMake 构建
  python build/setup_deps.py --mode cache --skip-binary   # cache 模式，跳过二进制
"""

import argparse
import concurrent.futures
import functools
import os
import platform as sys_platform_info
import shutil
import stat
import subprocess
import sys
import urllib.request
import zipfile
import tarfile
from pathlib import Path

# ============================================================================
#  Configuration — adjust these when upgrading dependencies
# ============================================================================

REPO_ROOT = Path(__file__).resolve().parent.parent
THIRD_PARTY = REPO_ROOT / "third_party"

# ── nlohmann/json ──────────────────────────────────────────────────────────
# Include path: single_include/nlohmann/json.hpp
JSON_REPO_URL = "https://gitcode.com/openharmony/third_party_json.git"
JSON_COMMIT = "OpenHarmony-v7.0-Beta1"          # tag or commit hash
JSON_DST = THIRD_PARTY / "src" / "third_party_json"

# ── SQLite3 amalgamation ──────────────────────────────────────────────────
# Files: sqlite3.c, sqlite3.h, sqlite3ext.h
SQLITE_REPO_URL = "https://gitcode.com/openharmony/third_party_sqlite.git"
SQLITE_COMMIT = "OpenHarmony-v7.0-Beta1"  # tag
SQLITE_DST = THIRD_PARTY / "src" / "third_party_sqlite"
SQLITE_SRC_DIR = "src"            # subdirectory inside cloned repo with sqlite3.c

# ── zlib（含 minizip，用于 tshandle 以 API 方式解压 ZIP）──────────────────
# 源文件：根目录 zlib 核心（adler32/crc32/inflate/inffast/inftrees/zutil）
# contrib/minizip 核心 {unzip.c, ioapi.c, iowin32.c(win)}
ZLIB_REPO_URL = "https://gitcode.com/openharmony/third_party_zlib.git"
ZLIB_COMMIT = "OpenHarmony-v7.0-Beta1"  # tag
ZLIB_DST = THIRD_PARTY / "src" / "third_party_zlib"

# ── trace_streamer binary ─────────────────────────────────────────────────
TRACE_STREAMER_VERSION = "HiSmartPerf_20260730"
TRACE_STREAMER_ZIP_URL = "https://gitcode.com/openharmony/developtools_smartperf_host/releases/download/HiSmartPerf_20260730/trace_streamer_binary.zip"
TRACE_STREAMER_DIR = THIRD_PARTY / "bin" / TRACE_STREAMER_VERSION / "trace_streamer_binary"

# ── rawheap_translator binary ──────────────────────────────────────────────
RAW_HEAP_VERSION = "7.0-Beta1"
RAW_HEAP_DIR = THIRD_PARTY / "bin" / "rawheaptranslator" / RAW_HEAP_VERSION
RAW_HEAP_FLAT_DIR = THIRD_PARTY / "bin" / "rawheaptranslator"  # 版本剥离后的固定路径，供构建脚本使用
RAW_HEAP_SDK_WIN_LINUX_URL = "https://repo.huaweicloud.com/openharmony/os/7.0-Beta1/ohos-sdk-windows_linux-public.tar.gz"
RAW_HEAP_SDK_MAC_X64_URL = "https://repo.huaweicloud.com/openharmony/os/7.0-Beta1/ohos-sdk-mac-public.tar.gz"
RAW_HEAP_SDK_MAC_ARM64_URL = "https://repo.huaweicloud.com/openharmony/os/7.0-Beta1/L2-SDK-MAC-M1-PUBLIC.tar.gz"
# macOS rawheap_translator 需要单独的 SDK 包下载

# ============================================================================
#  Helpers
# ============================================================================


def info(msg: str) -> None:
    print(f"  [INFO] {msg}")


def ok(msg: str) -> None:
    print(f"  [  OK] {msg}")


def warn(msg: str) -> None:
    print(f"  [WARN] {msg}")


def fail(msg: str) -> None:
    print(f"  [FAIL] {msg}")
    sys.exit(1)


class DownloadProgress:
    """Report download progress to console with percentage and size."""

    def __init__(self, desc: str):
        self.desc = desc
        self.last_pct = -1

    def __call__(self, block_count: int, block_size: int, total_size: int) -> None:
        if total_size <= 0:
            return
        downloaded = block_count * block_size
        pct = int(downloaded * 100 / total_size)
        if pct != self.last_pct and pct % 5 == 0:  # print every 5%
            self.last_pct = pct
            mb = downloaded / (1024 * 1024)
            total_mb = total_size / (1024 * 1024)
            print(f"\r  [{self.desc}] {pct}% ({mb:.1f}/{total_mb:.1f} MB)", end="", flush=True)
            if pct >= 100:
                print()


def detect_platform() -> str:
    """Return 'linux', 'darwin', or 'windows'."""
    sys_platform = sys.platform.lower()
    if sys_platform.startswith("linux"):
        return "linux"
    if sys_platform.startswith("darwin"):
        return "darwin"
    if sys_platform.startswith("win"):
        return "windows"
    fail(f"Unsupported platform: {sys.platform}")
    return ""


def current_rawheap_platform() -> str:
    """Return the current platform key for rawheap binaries: linux-x64 / mac-arm64 / mac-x64 / windows-x64."""
    plat = detect_platform()
    if plat == "darwin":
        machine = sys_platform_info.machine().lower()
        return "mac-arm64" if machine in ("arm64", "aarch64") else "mac-x64"
    if plat == "linux":
        return "linux-x64"
    return "windows-x64"


def run_git(args: list[str], cwd: Path) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["git"] + args,
        cwd=cwd,
        capture_output=True,
        text=True,
    )


def rmtree_force(path: Path) -> None:
    """Remove a directory tree, clearing read-only attributes first (Windows git files are read-only)."""
    def _on_rm_error(func, p, exc_info):
        try:
            os.chmod(p, stat.S_IWRITE)
        except Exception:
            pass
        try:
            func(p)
        except Exception:
            pass
    shutil.rmtree(path, onerror=_on_rm_error)

# ============================================================================
#  Git dependency check & clone
# ============================================================================


def check_git_installed() -> None:
    try:
        subprocess.run(["git", "--version"], capture_output=True, check=True)
    except (FileNotFoundError, subprocess.CalledProcessError):
        fail("git not found. Install git and try again.")


def ensure_git_repo(dst: Path, url: str, commit: str, check_file: Path, name: str) -> bool:
    """
    Ensure a git repo is present at `dst` and checked out to `commit`.

    If `check_file` (a path relative to `dst`) already exists, skip cloning.
    Otherwise clone the repo (removing dst first if it exists as a non-git dir).
    Returns True if the dependency is satisfied.
    """
    dst_file = dst / check_file
    if dst_file.is_file():
        # File already present — try to verify git repo state
        result = run_git(["rev-parse", "HEAD"], dst)
        if result.returncode == 0:
            # Check for local modifications
            status = run_git(["status", "--porcelain"], dst)
            if status.stdout.strip():
                info(f"{name}: detected local modifications, reverting ...")
                run_git(["checkout", "."], dst)
                run_git(["clean", "-fd"], dst)
                remaining = run_git(["status", "--porcelain"], dst)
                if remaining.stdout.strip():
                    fail(f"{name}: local changes could not be fully reverted: {remaining.stdout.strip()[:240]}")

            # Check commit
            current = run_git(["rev-parse", "HEAD"], dst).stdout.strip()
            resolve = run_git(["rev-list", "-1", commit], dst)
            expected = resolve.stdout.strip() if resolve.returncode == 0 else commit
            if current == expected or current.startswith(expected):
                ok(f"{name}: at commit {current[:12]}")
                return True
            else:
                warn(f"{name}: at {current[:12]}, expected {expected[:12]}, re-cloning ...")
        else:
            # File exists but not a git repo — assume manually extracted
            ok(f"{name}: file found (not a git repo, commit check skipped)")
            return True

    # Re-clone: remove existing dir and clone fresh
    if dst.is_dir():
        info(f"{name}: removing existing dir for fresh clone")
        rmtree_force(dst)

    info(f"{name}: cloning {url} @ {commit}")
    result = run_git(["clone", "--depth", "1", "--branch", commit, url, str(dst)], REPO_ROOT)
    if result.returncode != 0:
        # Some tags are not branch heads — fallback to full clone then checkout
        info(f"{name}: shallow clone failed, trying full clone ...")
        rmtree_force(dst)
        result = run_git(["clone", url, str(dst)], REPO_ROOT)
        if result.returncode != 0:
            fail(f"Failed to clone {name}: {result.stderr}")
        result = run_git(["checkout", commit], dst)
        if result.returncode != 0:
            fail(f"Failed to checkout {commit} for {name}: {result.stderr}")

    verify = run_git(["rev-parse", "HEAD"], dst)
    current = verify.stdout.strip()
    resolve = run_git(["rev-list", "-1", commit], dst)
    expected = resolve.stdout.strip() if resolve.returncode == 0 else commit
    if current == expected or current.startswith(expected):
        ok(f"{name}: at commit {current[:12]}")
        return True
    else:
        fail(f"{name}: clone commit mismatch: {current[:12]} vs expected {expected[:12]}")
        return False


# ============================================================================
#  Binary download helpers
# ============================================================================


def download_file(url: str, dst: Path, desc: str) -> None:
    """Download a file from `url` to `dst`."""
    dst.parent.mkdir(parents=True, exist_ok=True)
    progress = DownloadProgress(desc)
    try:
        urllib.request.urlretrieve(url, dst, reporthook=progress)
    except Exception as e:
        print()
        fail(f"{desc}: download failed: {e}")
    ok(f"{desc}: saved to {dst}")


def download_and_extract(url: str, dst_dir: Path, desc: str) -> None:
    """Download an archive and extract it into `dst_dir`."""
    dst_dir.mkdir(parents=True, exist_ok=True)
    tmp = dst_dir / "_download_tmp"
    progress = DownloadProgress(desc)
    try:
        urllib.request.urlretrieve(url, tmp, reporthook=progress)
    except Exception as e:
        print()
        fail(f"{desc}: download failed: {e}")
    print()  # newline after progress

    info(f"{desc}: extracting ...")
    if url.endswith(".zip"):
        with zipfile.ZipFile(tmp, "r") as zf:
            zf.extractall(dst_dir)
    elif url.endswith(".tar.gz") or url.endswith(".tgz"):
        with tarfile.open(tmp, "r:gz") as tf:
            tf.extractall(dst_dir)
    elif url.endswith(".tar.xz"):
        with tarfile.open(tmp, "r:xz") as tf:
            tf.extractall(dst_dir)
    else:
        # Single file
        pass
    tmp.unlink(missing_ok=True)
    ok(f"{desc}: extracted to {dst_dir}")


def extract_rawheap_from_sdk(url: str, dst_dir: Path, platform_map: list[tuple[str, str]], desc: str, tmp_name: str = "_sdk_download.tar.gz") -> int:
    """
    从 OHOS SDK tar.gz 中提取 rawheap_translator（嵌套压缩：tar.gz → inner.zip → toolchains/）。
    只解压需要的文件，避免解压整个大包。

    Args:
        url: SDK tar.gz 下载链接
        dst_dir: 目标目录
        platform_map: 列表，每个元素为 (sdk内前缀, arch子目录名)，
                      例如 ("ohos-sdk/windows/", "windows-x64")
        desc: 描述文本
        tmp_name: 临时文件名（并行下载时需使用唯一名称）

    Returns:
        成功提取的平台数
    """
    dst_dir.mkdir(parents=True, exist_ok=True)
    tmp_tar = dst_dir / tmp_name
    progress = DownloadProgress(desc)
    try:
        urllib.request.urlretrieve(url, tmp_tar, reporthook=progress)
    except Exception as e:
        print()
        fail(f"{desc}: SDK download failed: {e}")
    print()

    info(f"{desc}: extracting rawheap_translator from SDK ...")
    found = 0
    with tarfile.open(tmp_tar, "r:gz") as tf:
        for sdk_prefix, arch_subdir in platform_map:
            # Find the toolchains zip inside SDK
            zip_member = None
            for m in tf.getmembers():
                if m.name.startswith(sdk_prefix) and "toolchains" in m.name and m.name.endswith(".zip"):
                    zip_member = m
                    break
            if not zip_member:
                warn(f"rawheap_translator ({arch_subdir}): 在 SDK 中未找到 toolchains zip")
                continue

            # Extract the zip to memory
            zip_data = tf.extractfile(zip_member)
            if not zip_data:
                warn(f"rawheap_translator ({arch_subdir}): 无法读取 {zip_member.name}")
                continue

            # Determine binary name by architecture
            bin_name = "rawheap_translator.exe" if "windows" in arch_subdir else "rawheap_translator"
            platform_dir = dst_dir / arch_subdir
            platform_dir.mkdir(parents=True, exist_ok=True)
            with zipfile.ZipFile(zip_data) as inner_zf:
                target = f"toolchains/{bin_name}"
                if target in inner_zf.namelist():
                    inner_zf.extract(target, dst_dir)
                    src = dst_dir / target
                    dst = platform_dir / bin_name
                    if src != dst:
                        shutil.move(str(src), str(dst))
                else:
                    warn(f"rawheap_translator ({arch_subdir}): SDK 内未找到 {target}")
                    continue
            found += 1
            ok(f"rawheap_translator ({arch_subdir}): extracted")

    # Clean up SDK tar.gz
    tmp_tar.unlink(missing_ok=True)

    # Clean up stray toolchains/ dir if left behind
    toolchains_dir = dst_dir / "toolchains"
    if toolchains_dir.is_dir():
        rmtree_force(toolchains_dir)

    return found

# ============================================================================
#  Binary definitions (all platforms, for cross-compilation support)
# ============================================================================

TRACE_STREAMER_BINARIES = {
    "linux": TRACE_STREAMER_DIR / "trace_streamer_linux_static",
    "darwin": TRACE_STREAMER_DIR / "trace_streamer_mac",
    "windows": TRACE_STREAMER_DIR / "trace_streamer_windows.exe",
}

RAW_HEAP_BINARIES = {
    "linux-x64": RAW_HEAP_FLAT_DIR / "linux-x64" / "rawheap_translator",
    "mac-arm64": RAW_HEAP_FLAT_DIR / "mac-arm64" / "rawheap_translator",
    "mac-x64": RAW_HEAP_FLAT_DIR / "mac-x64" / "rawheap_translator",
    "windows-x64": RAW_HEAP_FLAT_DIR / "windows-x64" / "rawheap_translator.exe",
}

# ============================================================================
#  Main
# ============================================================================


def main() -> None:
    parser = argparse.ArgumentParser(description="Setup mem-analyzer C++ dependencies (git sources + binaries)")
    parser.add_argument("--mode", choices=["cache", "release"], default="cache",
                        help="cache: 增量检查（默认）; release: 清除全部依赖并重新下载")
    parser.add_argument("--skip-binary", action="store_true", help="跳过二进制下载")
    args = parser.parse_args()

    is_release = args.mode == "release"
    check_all = is_release          # release 模式检查全平台；cache 模式仅当前平台
    current_ts = detect_platform()  # trace_streamer 平台 key: linux/darwin/windows
    current_rh = current_rawheap_platform()  # rawheap 平台 key: linux-x64/mac-arm64/mac-x64/windows-x64

    mode_label = "Release（全量刷新）" if is_release else "Cache（增量）"
    print("=" * 55)
    print(f" mem-analyzer 依赖初始化 [{mode_label}]")
    print(f" 根目录: {REPO_ROOT}")
    print("=" * 55)
    print()

    # ── Release mode: clean ──────────────────────────────────────────────
    if is_release:
        print("[pre] 清除所有第三方依赖")
        print("-" * 40)
        for d in [JSON_DST, SQLITE_DST, ZLIB_DST]:  # TRACE_STREAMER_DIR, RAW_HEAP_DIR 暂不清理
            if d.is_dir():
                info(f"删除: {d.relative_to(REPO_ROOT)}")
                rmtree_force(d)
        ok("清除完成")
        print()

    # ── 1. Git sources ──────────────────────────────────────────────────
    print("[1/3] Git 源码依赖")
    print("-" * 40)
    check_git_installed()

    ensure_git_repo(JSON_DST, JSON_REPO_URL, JSON_COMMIT,
                    Path("single_include") / "nlohmann" / "json.hpp", "nlohmann/json")
    ensure_git_repo(SQLITE_DST, SQLITE_REPO_URL, SQLITE_COMMIT,
                    Path(SQLITE_SRC_DIR) / "sqlite3.c", "SQLite3")
    ensure_git_repo(ZLIB_DST, ZLIB_REPO_URL, ZLIB_COMMIT,
                    Path("zlib.h"), "zlib/minizip")
    print()

    # ── 2. Binary dependencies ──────────────────────────────────────────
    scope_label = "全平台" if check_all else f"当前平台 ({current_ts} / {current_rh})"
    print(f"[2/3] 二进制依赖（{scope_label}）")
    print("-" * 40)

    # Helper: 将版本目录复制到固定路径，消除版本号依赖
    def _copy_trace_streamer_dir(ts_ver_dir: Path) -> None:
        strip_dir = THIRD_PARTY / "bin" / "trace_streamer_binary"
        if strip_dir.is_dir():
            rmtree_force(strip_dir)
        shutil.copytree(str(ts_ver_dir / "trace_streamer_binary"), str(strip_dir))
        info(f"trace_streamer: 已复制到 {strip_dir.relative_to(REPO_ROOT)}")

    # Define reusable helpers for parallel binary download ---
    def _download_trace_streamer() -> list[tuple[str, str]]:
        """Download trace_streamer zip. Returns [(result_type, msg), ...]."""
        ts_ver_dir = TRACE_STREAMER_DIR.parent
        # 需要检查/补齐的平台二进制：release 模式全平台，cache 模式仅当前平台
        bins = (TRACE_STREAMER_BINARIES if check_all
                else {current_ts: TRACE_STREAMER_BINARIES[current_ts]})
        if any(p.is_file() for p in bins.values()):
            # 二进制已存在，仍需确保固定路径副本是最新的
            _copy_trace_streamer_dir(ts_ver_dir)
            return []
        if not TRACE_STREAMER_ZIP_URL:
            return []
        try:
            # 解压到对应版本目录下（zip 顶层即 trace_streamer_binary/），不做移动
            download_and_extract(TRACE_STREAMER_ZIP_URL, ts_ver_dir, "trace_streamer")
        except SystemExit as e:
            return [("fail", f"trace_streamer: download/extract failed: {e}")]
        if not ts_ver_dir.is_dir():
            return [("fail", f"trace_streamer: 下载后版本目录 {TRACE_STREAMER_VERSION} 未创建")]
        # 创建固定路径符号链接/复制，避免版本号变化时修改所有脚本
        _copy_trace_streamer_dir(ts_ver_dir)
        return []

    def _copy_rawheap_to_flat() -> None:
        """将版本目录下的 rawheap 二进制复制到去版本号的固定路径，供构建脚本使用。"""
        if not RAW_HEAP_DIR.is_dir():
            return
        for arch in RAW_HEAP_BINARIES:
            src_arch = RAW_HEAP_DIR / arch
            dst_arch = RAW_HEAP_FLAT_DIR / arch
            if not src_arch.is_dir():
                continue
            dst_arch.parent.mkdir(parents=True, exist_ok=True)
            if dst_arch.is_dir():
                rmtree_force(dst_arch)
            shutil.copytree(str(src_arch), str(dst_arch))
            info(f"rawheap_translator ({arch}): 已复制到 {dst_arch.relative_to(REPO_ROOT)}")

    def _download_rawheap(label: str, url: str, platform_map: list[tuple[str, str]]) -> list[tuple[str, str]]:
        """Download & extract one rawheap SDK. Returns [(result_type, msg), ...]."""
        # Check which platforms are missing (against flat path used by build scripts)
        missing = []
        for _, arch in platform_map:
            bin_name = "rawheap_translator.exe" if "windows" in arch else "rawheap_translator"
            if not (RAW_HEAP_FLAT_DIR / arch / bin_name).is_file():
                missing.append(arch)
        if not missing:
            _copy_rawheap_to_flat()  # 刷新固定路径副本（处理版本升级场景）
            return []
        tmp_name = f"_sdk_download_{label}.tar.gz"
        desc = f"rawheap_translator ({label})"
        try:
            info(f"  -> 开始: {label} (缺失 {', '.join(missing)})")
            found = extract_rawheap_from_sdk(url, RAW_HEAP_DIR, platform_map, desc, tmp_name)
        except SystemExit as e:
            return [("fail", f"rawheap_translator ({label}): {e}")]
        except Exception as e:
            return [("fail", f"rawheap_translator ({label}): 异常: {e}")]
        if found == 0:
            return [("warn", f"rawheap_translator ({label}): 提取失败，SDK 内容可能与预期结构不匹配")]
        _copy_rawheap_to_flat()
        return [("ok", f"rawheap_translator ({label}): extracted")]

    # --- Build all binary tasks ---
    binary_tasks: list[tuple[str, ...]] = []  # (task_name, callable)
    if not args.skip_binary:
        # trace_streamer
        binary_tasks.append(("trace_streamer", _download_trace_streamer))

        # rawheap SDK sources
        raw_heap_sdk_sources = [
            ("win+linux", RAW_HEAP_SDK_WIN_LINUX_URL,
             [("ohos-sdk/windows/", "windows-x64"), ("ohos-sdk/linux/", "linux-x64")]),
            ("mac-x64", RAW_HEAP_SDK_MAC_X64_URL,
             [("sdk/packages/ohos-sdk/darwin/", "mac-x64")]),
            ("mac-arm64", RAW_HEAP_SDK_MAC_ARM64_URL,
             [("sdk/packages/ohos-sdk/darwin/", "mac-arm64")]),
        ]
        if not check_all:
            # cache 模式：仅保留能提供当前平台 rawheap 的 SDK 源
            raw_heap_sdk_sources = [
                (label, url, [(prefix, arch) for prefix, arch in pm if arch == current_rh])
                for label, url, pm in raw_heap_sdk_sources
                if any(arch == current_rh for _, arch in pm)
            ]
            info(f"cache 模式: rawheap 仅检查当前平台 {current_rh}")
        for sdk_label, sdk_url, platform_map in raw_heap_sdk_sources:
            binary_tasks.append((
                f"rawheap_{sdk_label}",
                functools.partial(_download_rawheap, sdk_label, sdk_url, platform_map),
            ))

    # --- Run all binary tasks in parallel ---
    if binary_tasks:
        n = len(binary_tasks)
        info(f"二进制下载: {n} 个任务启动并行 ...")
        with concurrent.futures.ThreadPoolExecutor(max_workers=n) as executor:
            fut_map = {executor.submit(fn): name for name, fn in binary_tasks}
            for fut in concurrent.futures.as_completed(fut_map):
                name = fut_map[fut]
                try:
                    results = fut.result()
                    for rtype, rmsg in results:
                        if rtype == "fail":
                            warn(rmsg)
                        elif rtype == "warn":
                            warn(rmsg)
                        elif rtype == "ok":
                            ok(rmsg)
                except Exception as e:
                    warn(f"{name}: 任务异常: {e}")
        ok("全部二进制处理完毕")

    # --- Report trace_streamer status ---
    ts_report = TRACE_STREAMER_BINARIES if check_all else {current_ts: TRACE_STREAMER_BINARIES[current_ts]}
    for key, path in ts_report.items():
        if path.is_file():
            ok(f"trace_streamer ({key}): {path}")
        elif args.skip_binary:
            info(f"trace_streamer ({key}): 跳过 (--skip-binary)")
        else:
            warn(f"trace_streamer ({key}): 未找到于 {path}")

    # --- Report rawheap_translator status ---
    rh_report = RAW_HEAP_BINARIES if check_all else {current_rh: RAW_HEAP_BINARIES[current_rh]}
    for key, path in rh_report.items():
        if path.is_file():
            ok(f"rawheap_translator ({key}): {path}")
        elif args.skip_binary:
            info(f"rawheap_translator ({key}): 跳过 (--skip-binary)")
        else:
            warn(f"rawheap_translator ({key}): 未找到")
    print()

    # ── 3. Verification ────────────────────────────────────────────────
    print("[3/3] 检查结果")
    print("-" * 40)

    # Verify JSON include
    json_inc = JSON_DST / "single_include" / "nlohmann" / "json.hpp"
    if json_inc.is_file():
        ok(f"nlohmann/json: {json_inc}")
    else:
        fail(f"nlohmann/json not found at {json_inc}")

    # Verify SQLite source
    sqlite_c = SQLITE_DST / SQLITE_SRC_DIR / "sqlite3.c"
    sqlite_h = SQLITE_DST / "include" / "sqlite3.h"
    if sqlite_c.is_file() and sqlite_h.is_file():
        ok(f"SQLite3: {sqlite_c}")
        ok(f"SQLite3: {sqlite_h}")
    else:
        fail(f"SQLite3 source not found at {sqlite_c}")

    # Verify zlib + minizip source
    zlib_h = ZLIB_DST / "zlib.h"
    unzip_c = ZLIB_DST / "contrib" / "minizip" / "unzip.c"
    if zlib_h.is_file() and unzip_c.is_file():
        ok(f"zlib/minizip: {ZLIB_DST}")
    else:
        fail(f"zlib/minizip source not found at {zlib_h}")

    # ── Verify binaries are NOT git-lfs pointer files ──────────────────
    git_lfs_signature = "version https://git-lfs.github.com/spec/v1"
    bin_dir = THIRD_PARTY / "bin"
    if bin_dir.is_dir():
        lfs_fakes = []
        for f in bin_dir.rglob("*"):
            if not f.is_file():
                continue
            try:
                with open(f, "rb") as fh:
                    head = fh.read(64)
                try:
                    text = head.decode("utf-8", errors="strict")
                except UnicodeDecodeError:
                    continue  # binary file, definitely not lfs pointer
                if text.startswith(git_lfs_signature):
                    lfs_fakes.append(f.relative_to(REPO_ROOT))
            except OSError:
                pass
        if lfs_fakes:
            print()
            fail("检测到 Git LFS 指针文件（未正确拉取 LFS 内容），请运行 git lfs pull 后重试：\n    " +
                 "\n    ".join(str(p) for p in lfs_fakes))
        ok("binary files: 无 Git LFS 指针文件")
    else:
        info("binary files: third_party/bin 不存在，跳过检查")

    print()
    print("=" * 55)
    if json_inc.is_file() and sqlite_c.is_file() and sqlite_h.is_file() and zlib_h.is_file():
        print(" 核心源码依赖就绪。")
        print(" 二进制文件状态见上方 [WARN] 列表。")
    else:
        print(" 部分依赖缺失，请修复后重试。")
    print("=" * 55)


if __name__ == "__main__":
    main()
