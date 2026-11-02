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
profiler 子命令功能 / 集成测试脚本。

通过 hdc 连接真实 HarmonyOS 设备，对 mem-analyzer profiler 子命令做全面功能验证：
  - --help 输出验证（主命令 + profiler 子命令 + prof 别名）
  - 参数校验（负面用例：无效 category、缺 bundle、空 --device 等）
  - --list-apps 功能
  - 全量 10 种采集类型实际采集（nativeheap / nativeleaks / snapshot /
    globalhandle / allhandle / localhandle / fd / dma / thread / ashmem）
  - 采集类型别名验证（nh / native / nl / snap / sn / global / gh /
    handle / ah / local / lh）
  - 选项组合测试（-o 自动报告 / -d 自定义输出目录 / -t 显式时长 /
    --device 显式设备 / snapshot 持续模式 / --verbose 调试日志）
  - 采集产出文件验证

适配 Windows / Linux / macOS。测试应用可通过 --bundle 参数传入，默认为「抖音」。
测试人员需要保证已连接 HarmonyOS 设备（USB 调试），且 hdc 已加入 PATH。

用法：
  # 基本用法（使用默认 exe + 默认测试应用抖音）
  python build/test_profiler.py

  # 指定 exe 和测试应用
  python build/test_profiler.py --exe path/to/mem-analyzer --bundle com.example.app

  # 跳过构建，直接使用已有 exe
  python build/test_profiler.py --skip-build --exe cpp/build_base/mem-analyzer.exe

  # 从指定 commit 构建
  python build/test_profiler.py --commit abc1234

  # 只跑特定测试套件
  python build/test_profiler.py --suites help,negative,list_apps,capture,alias,combo

  # 指定采集时长（秒）
  python build/test_profiler.py --duration 15

  # 指定设备（多设备时必填）
  python build/test_profiler.py --device 33Z0224A22021343

自保护机制：
  脚本启动时会自动将自身复制到临时目录，从临时目录重新执行。
  即使 git worktree 创建/删除过程中修改了原始文件，运行中的脚本也不会受影响。
"""

import subprocess
import os
import sys
import json
import shutil
import time
import re
import zipfile
import tempfile
import secrets
import argparse
import platform as py_platform
from pathlib import Path
from datetime import datetime
from typing import List, Dict, Tuple, Optional, Any

# ============================================================
# 自保护：将脚本复制到临时目录并重新执行
# ============================================================
_ENV_KEY = "_TEST_PROFILER_TEMP_DIR"
_ENV_PROJECT_ROOT = "_TEST_PROFILER_PROJECT_ROOT"


def _ensure_self_contained():
    """确保脚本在临时目录中运行，避免 git 操作影响自身。"""
    if _ENV_KEY in os.environ:
        return  # 已在临时目录中运行

    script_dir = Path(__file__).parent.resolve()
    original_root = script_dir.parent
    os.environ[_ENV_PROJECT_ROOT] = str(original_root)

    temp_dir = Path(tempfile.mkdtemp(prefix="test_profiler_"))
    temp_script = temp_dir / Path(__file__).name
    shutil.copy2(__file__, temp_script)
    os.environ[_ENV_KEY] = str(temp_dir)

    log_raw(f"[SELF] Original project root: {original_root}")
    log_raw(f"[SELF] Copied script to: {temp_script}")
    log_raw(f"[SELF] Re-executing from temp location...")

    os.execv(sys.executable, [sys.executable, str(temp_script)] + sys.argv[1:])


# ============================================================
# 日志工具
# ============================================================
def log_raw(msg: str):
    ts = datetime.now().strftime("%H:%M:%S")
    print(f"[{ts}] {msg}", flush=True)


def log(msg: str):
    log_raw(msg)


def log_header(title: str):
    log("=" * 60)
    log(f"  {title}")
    log("=" * 60)


# ============================================================
# 路径安全工具
# ============================================================
def is_unc_path(path: Path) -> bool:
    """判断路径是否为 UNC 网络路径（\\\\server\\share 或 //server/share）。"""
    s = str(path)
    return s.startswith("\\\\") or s.startswith("//")


def needs_local_redirect(path: Path) -> bool:
    """判断输出路径是否需要重定向到本地短路径。

    profiler 的 hdc file recv 在 Windows 上向 UNC 路径或超长路径写文件时会失败
    (hdc 报告 FileTransfer finish 但文件未落地，profiler 的 fs::exists 检查不过，
    退出码 3 = EXIT_EXPORT_FAILED)。此函数检测这类路径并返回 True。
    """
    if is_unc_path(path):
        return True
    # Windows MAX_PATH = 260；留出 session 目录名 + 采集文件名的余量
    if len(str(path)) > 180:
        return True
    return False


# ============================================================
# 平台检测
# ============================================================
def detect_platform() -> str:
    """返回 'windows' / 'linux' / 'mac'。"""
    system = py_platform.system().lower()
    if system == "windows":
        return "windows"
    if system == "darwin":
        return "mac"
    return "linux"


def get_exe_name(plat: str) -> str:
    return "mem-analyzer.exe" if plat == "windows" else "mem-analyzer"


def get_build_script(cpp_dir: Path, plat: str) -> Path:
    """返回对应平台的构建脚本路径。"""
    if plat == "windows":
        return cpp_dir / "build-win.ps1"
    if plat == "mac":
        return cpp_dir / "release-mac.sh"
    return cpp_dir / "release-cross.sh"


def get_zip_pattern(plat: str) -> str:
    if plat == "windows":
        return "mem-analyzer-*-win64.zip"
    if plat == "mac":
        return "mem-analyzer-*-macos*.zip"
    return "mem-analyzer-*-linux*.zip"


# ============================================================
# 命令执行
# ============================================================
def run_cmd(cmd: List[str], cwd: str = None, timeout: int = 600,
            capture: bool = True, label: str = "",
            check: bool = False) -> subprocess.CompletedProcess:
    if label:
        log(f"  [{label}] {' '.join(cmd)}")
    else:
        log(f"  [CMD] {' '.join(cmd)}")

    kwargs = dict(cwd=cwd, timeout=timeout)
    if capture:
        kwargs.update(capture_output=True, text=True, encoding="utf-8", errors="replace")

    result = subprocess.run(cmd, **kwargs)

    if result.returncode != 0:
        if capture:
            log(f"  [FAIL] stderr: {result.stderr[-300:] if result.stderr else ''}")
        else:
            log(f"  [FAIL] returncode={result.returncode}")
        if check:
            raise RuntimeError(f"Command failed: {' '.join(cmd)}")
    return result


def run_build_script(script_path: Path, cpp_dir: Path, plat: str,
                     timeout: int = 1800) -> subprocess.CompletedProcess:
    """跨平台执行构建脚本。"""
    if plat == "windows":
        return run_cmd(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                        "-File", str(script_path)], cwd=str(cpp_dir),
                       timeout=timeout, capture=False)
    else:
        return run_cmd(["bash", str(script_path)], cwd=str(cpp_dir),
                       timeout=timeout, capture=False)


# ============================================================
# git worktree 操作（替代 git checkout）
# ============================================================
class GitWorktree:
    """管理 git worktree 的生命周期。"""

    def __init__(self, repo_path: Path, commit: str, label: str):
        self.repo_path = repo_path
        self.commit = commit
        self.label = label
        rand_suffix = secrets.token_hex(4)
        self.worktree_path = repo_path.parent / f".worktree_{label}_{rand_suffix}"

    def __enter__(self):
        log_raw(f"  [WORKTREE] Creating worktree for {self.label} @ {self.commit[:12]}...")
        run_cmd(["git", "worktree", "add", "--detach", str(self.worktree_path), self.commit],
                cwd=str(self.repo_path), timeout=120, check=True)
        log_raw(f"  [WORKTREE] Created: {self.worktree_path}")
        return self.worktree_path

    def __exit__(self, exc_type, exc_val, exc_tb):
        log_raw(f"  [WORKTREE] Removing worktree: {self.worktree_path}")
        self._force_remove()

    def _force_remove(self):
        run_cmd(["git", "worktree", "remove", "--force", str(self.worktree_path)],
                cwd=str(self.repo_path), timeout=60, capture=True)
        if self.worktree_path.exists():
            try:
                shutil.rmtree(self.worktree_path, ignore_errors=True)
            except PermissionError:
                pass
        wt_meta = self.repo_path / ".git" / "worktrees" / self.worktree_path.name
        if wt_meta.exists():
            try:
                shutil.rmtree(wt_meta, ignore_errors=True)
            except PermissionError:
                pass
        run_cmd(["git", "worktree", "prune"], cwd=str(self.repo_path),
                timeout=30, capture=True)


# ============================================================
# 构建
# ============================================================
def build_exe(project_root: Path, cpp_dir: Path, plat: str,
              commit: Optional[str] = None) -> Path:
    """
    构建 mem-analyzer，返回 exe 所在的打包目录路径（含 config/ third_party/）。
    commit 为 None 时从当前源码树构建，否则从指定 commit 构建。
    """
    label = f"BUILD ({commit[:12] if commit else 'current'})"
    log_header(f"BUILD: {label}")

    final_dir_name = "build_test_profiler"
    final_dir = cpp_dir / final_dir_name
    if final_dir.exists():
        log(f"  Cleaning: {final_dir}")
        shutil.rmtree(final_dir)

    if commit:
        with GitWorktree(project_root, commit, final_dir_name) as wt_path:
            wt_cpp = wt_path / "cpp"
            return _do_build_and_extract(wt_cpp, final_dir, plat)
    else:
        return _do_build_and_extract(cpp_dir, final_dir, plat)


def _do_build_and_extract(cpp_src: Path, final_dir: Path, plat: str) -> Path:
    build_script = get_build_script(cpp_src, plat)
    if not build_script.exists():
        raise FileNotFoundError(f"No build script found: {build_script}")

    log(f"  Running {build_script.name} ...")
    r = run_build_script(build_script, cpp_src, plat, timeout=1800)

    # 查找生成的 ZIP 文件
    zip_pattern = get_zip_pattern(plat)
    zip_files = sorted(cpp_src.glob(zip_pattern))

    if not zip_files:
        # ZIP 未生成，尝试从 build/ 目录直接取
        tmp_build = cpp_src / "build"
        exe_name = get_exe_name(plat)
        exe_candidate = tmp_build / exe_name
        if not exe_candidate.exists():
            # Windows MSVC 可能在 Release/ 子目录
            exe_candidate = tmp_build / "Release" / exe_name
        if not exe_candidate.exists():
            raise RuntimeError(
                f"Build failed! exit={r.returncode}, no ZIP and no exe found")
        log(f"  Copying {tmp_build} -> {final_dir}")
        shutil.copytree(str(tmp_build), str(final_dir))
    else:
        zip_path = zip_files[-1]
        log(f"  Extracting ZIP: {zip_path.name} -> {final_dir}/")
        final_dir.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(zip_path, "r") as zf:
            zf.extractall(str(final_dir))
        log(f"  Extracted OK.")

    exe_name = get_exe_name(plat)
    exe = final_dir / exe_name
    if not exe.exists():
        # 检查是否在子目录中（ZIP 可能包含一层目录）
        for sub in final_dir.iterdir():
            if sub.is_dir() and (sub / exe_name).exists():
                exe = sub / exe_name
                break
    if not exe.exists():
        raise FileNotFoundError(f"{exe_name} not found in {final_dir}")

    # 确认 config 目录存在（profiler 运行依赖）
    exe_dir = exe.parent
    config_dir = exe_dir / "config" / "profiler"
    if not config_dir.exists():
        log(f"  [WARN] config/profiler/ not found in {exe_dir}")
        log(f"  [WARN] profiler 子命令可能因找不到配置而失败")

    log(f"  Build OK: {exe}")
    return exe


# ============================================================
# 前置检查
# ============================================================
def check_hdc() -> bool:
    """检查 hdc 是否可用。"""
    r = run_cmd(["hdc", "version"], timeout=10, capture=True, label="hdc-version")
    return r.returncode == 0


def get_device_list() -> List[str]:
    """获取已连接设备列表（序列号）。"""
    r = run_cmd(["hdc", "list", "targets"], timeout=10, capture=True, label="hdc-list")
    if r.returncode != 0:
        return []
    devices = []
    for line in r.stdout.strip().splitlines():
        line = line.strip()
        if line and not line.startswith("[Empty]") and not line.startswith("[Fail]"):
            devices.append(line)
    return devices


# ============================================================
# 采集类型定义
# ============================================================
# 每种采集类型的元数据：全名、别名列表、期望产出文件模式
CAPTURE_TYPES = [
    {
        "category": "snapshot",
        "aliases": ["snap", "sn"],
        "file_patterns": ["*.heapsnapshot", "*.rawheap"],
        "smaps": False,
        "immediate": True,  # 无 -t 时立即抓取
    },
    {
        "category": "nativeheap",
        "aliases": ["nh", "native"],
        "file_patterns": ["mem_analyzer_hiprofiler_data_*.htrace"],
        "smaps": True,
        "immediate": False,
    },
    {
        "category": "nativeleaks",
        "aliases": ["nl"],
        "file_patterns": ["mem_analyzer_hiprofiler_data_*.htrace",
                          "memleak-native-orphansnapshot-*.txt"],
        "smaps": True,
        "immediate": False,
    },
    {
        "category": "globalhandle",
        "aliases": ["global", "gh"],
        "file_patterns": ["hiprofiler_data_*.zip",
                          "mem_analyzer_hiprofiler_data_*.zip"],
        "smaps": True,
        "immediate": False,
    },
    {
        "category": "allhandle",
        "aliases": ["handle", "ah"],
        "file_patterns": ["hiprofiler_data_*.zip",
                          "mem_analyzer_hiprofiler_data_*.zip"],
        "smaps": True,
        "immediate": False,
    },
    {
        "category": "localhandle",
        "aliases": ["local", "lh"],
        "file_patterns": ["hiprofiler_data_*.zip",
                          "mem_analyzer_hiprofiler_data_*.zip"],
        "smaps": True,
        "immediate": False,
    },
    {
        "category": "fd",
        "aliases": [],
        "file_patterns": ["mem_analyzer_hiprofiler_data_*.htrace"],
        "smaps_prefix": "fds",
        "immediate": False,
    },
    {
        "category": "dma",
        "aliases": [],
        "file_patterns": ["mem_analyzer_hiprofiler_data_*.htrace"],
        "smaps": True,
        "immediate": False,
    },
    {
        "category": "thread",
        "aliases": [],
        "file_patterns": ["mem_analyzer_hiprofiler_data_*.htrace"],
        "smaps": True,
        "immediate": False,
    },
    {
        "category": "ashmem",
        "aliases": [],
        "file_patterns": ["mem_analyzer_hiprofiler_data_*.htrace"],
        "smaps": True,
        "immediate": False,
    },
]

# 所有别名（展平，用于别名测试套件）
ALL_ALIASES = []
for ct in CAPTURE_TYPES:
    for alias in ct["aliases"]:
        ALL_ALIASES.append((alias, ct["category"]))


# ============================================================
# 测试框架
# ============================================================
class TestResult:
    """单个测试用例的执行结果。"""

    def __init__(self, name: str, suite: str):
        self.name = name
        self.suite = suite
        self.passed = False
        self.exit_code: Optional[int] = None
        self.duration_s: float = 0.0
        self.stdout: str = ""
        self.stderr: str = ""
        self.detail: str = ""
        self.skipped: bool = False
        self.cmd: List[str] = []
        self.checks: List[Dict[str, Any]] = []

    def add_check(self, name: str, passed: bool, detail: str):
        """记录一项检查的结果（名称 / 是否通过 / 判断依据）。"""
        self.checks.append({"name": name, "passed": passed, "detail": detail})

    def mark_pass(self, detail: str = ""):
        self.passed = True
        self.detail = detail

    def mark_fail(self, detail: str = ""):
        self.passed = False
        self.detail = detail

    def mark_skip(self, reason: str = ""):
        self.skipped = True
        self.detail = reason


class TestCase:
    """单个测试用例定义。"""

    def __init__(self, name: str, suite: str, cmd: List[str],
                 expect_exit_zero: bool = True,
                 expect_stderr_contains: Optional[str] = None,
                 expect_stdout_contains: Optional[str] = None,
                 expect_not_contains: Optional[str] = None,
                 timeout: int = 300,
                 cwd: Optional[str] = None,
                 verify_output: Optional[Any] = None,
                 out_dir: Optional[Path] = None,
                 note: str = ""):
        self.name = name
        self.suite = suite
        self.cmd = cmd
        self.expect_exit_zero = expect_exit_zero
        self.expect_stderr_contains = expect_stderr_contains
        self.expect_stdout_contains = expect_stdout_contains
        self.expect_not_contains = expect_not_contains
        self.timeout = timeout
        self.cwd = cwd
        self.verify_output = verify_output  # callable(session_dir) -> (bool, str)
        self.out_dir = out_dir
        self.note = note


def run_test_case(tc: TestCase) -> TestResult:
    """执行单个测试用例，返回结果。

    日志输出格式：
      [套件名] 测试名
        CMD: <完整命令>
        NOTE: <备注>
        EXIT: <退出码> (<耗时>)
        [OK/FAIL] <检查项>: <判断依据>
        ...
        STDERR (last 5 lines):   ← 仅失败时
          | <行>
        VERDICT: PASS/FAIL — <判断依据汇总>
    """
    result = TestResult(tc.name, tc.suite)
    result.cmd = list(tc.cmd)

    cmd_str = " ".join(tc.cmd)
    log(f"\n  [{tc.suite}] {tc.name}")
    log(f"    CMD: {cmd_str}")
    if tc.note:
        log(f"    NOTE: {tc.note}")

    start = time.time()
    try:
        proc = subprocess.run(tc.cmd, capture_output=True, text=True,
                              timeout=tc.timeout, encoding="utf-8",
                              errors="replace", cwd=tc.cwd)
        result.exit_code = proc.returncode
        result.stdout = proc.stdout
        result.stderr = proc.stderr
    except subprocess.TimeoutExpired:
        result.duration_s = time.time() - start
        result.mark_fail(f"TIMEOUT after {tc.timeout}s")
        log(f"    EXIT: TIMEOUT ({tc.timeout}s exceeded)")
        log(f"    VERDICT: FAIL — timeout after {tc.timeout}s")
        return result
    except Exception as e:
        result.duration_s = time.time() - start
        result.mark_fail(f"EXCEPTION: {e}")
        log(f"    EXIT: EXCEPTION — {e}")
        log(f"    VERDICT: FAIL — {e}")
        return result

    result.duration_s = time.time() - start
    log(f"    EXIT: {result.exit_code}  ({result.duration_s:.1f}s)")

    # --- Check 1: exit code ---
    if tc.expect_exit_zero:
        exit_ok = (result.exit_code == 0)
        result.add_check("exit_code", exit_ok,
                         f"expected=0, actual={result.exit_code}")
    else:
        exit_ok = (result.exit_code != 0)
        result.add_check("exit_code", exit_ok,
                         f"expected!=0, actual={result.exit_code}")

    # --- Check 2: output contains expected text (stderr + stdout) ---
    if tc.expect_stderr_contains:
        combined = (result.stderr + result.stdout).lower()
        found = tc.expect_stderr_contains.lower() in combined
        result.add_check("output_contains", found,
                         f"expect '{tc.expect_stderr_contains}' in output: "
                         f"{'FOUND' if found else 'NOT FOUND'}")

    # --- Check 3: stdout contains expected text ---
    if tc.expect_stdout_contains:
        found = tc.expect_stdout_contains.lower() in result.stdout.lower()
        result.add_check("stdout_contains", found,
                         f"expect '{tc.expect_stdout_contains}' in stdout: "
                         f"{'FOUND' if found else 'NOT FOUND'}")

    # --- Check 4: output does NOT contain forbidden text ---
    if tc.expect_not_contains:
        combined = result.stderr + result.stdout
        found = tc.expect_not_contains.lower() in combined.lower()
        result.add_check("output_not_contains", not found,
                         f"expect '{tc.expect_not_contains}' absent: "
                         f"{'OK (absent)' if not found else 'FAIL (present)'}")

    # --- Check 5: output file verification ---
    if tc.verify_output and tc.out_dir:
        ok, detail = tc.verify_output(tc.out_dir)
        result.add_check("output_files", ok, detail)

    # --- Print all checks ---
    all_passed = all(c["passed"] for c in result.checks) if result.checks else True
    for c in result.checks:
        icon = "[OK]  " if c["passed"] else "[FAIL]"
        log(f"    {icon} {c['name']}: {c['detail']}")

    # --- Print stderr snippet on failure (helps diagnose root cause) ---
    if not all_passed and result.stderr:
        stderr_lines = result.stderr.strip().splitlines()
        if stderr_lines:
            show_lines = stderr_lines[-8:]
            log(f"    STDERR (last {len(show_lines)} lines):")
            for line in show_lines:
                log(f"      | {line}")

    # --- Verdict ---
    if all_passed:
        judgment = "; ".join(c["detail"] for c in result.checks)
        result.mark_pass(judgment)
        log(f"    VERDICT: PASS")
    else:
        failed_checks = [c for c in result.checks if not c["passed"]]
        judgment = "; ".join(c["detail"] for c in failed_checks)
        result.mark_fail(judgment)
        log(f"    VERDICT: FAIL — {judgment}")

    return result


# ============================================================
# 产出文件验证
# ============================================================
def find_session_dir(out_dir: Path) -> Optional[Path]:
    """在输出目录中查找采集会话目录。

    profiler 会在 out_dir 下创建 <type>-<bundle>-<timestamp>/ 子目录。
    有 --device 时还会多一层 <device-id>/。
    """
    if not out_dir.exists():
        return None
    # 查找最深层包含采集文件的目录（跳过空目录）
    candidates = []
    for item in out_dir.rglob("*"):
        if item.is_dir():
            # 检查目录内是否有非目录文件（采集产出）
            has_files = any(f.is_file() for f in item.iterdir())
            if has_files:
                candidates.append(item)
    if not candidates:
        return None
    # 取修改时间最新的
    candidates.sort(key=lambda p: p.stat().st_mtime, reverse=True)
    return candidates[0]


def make_capture_verifier(category: str):
    """创建一个采集产出验证函数。"""
    ct_info = None
    for ct in CAPTURE_TYPES:
        if ct["category"] == category:
            ct_info = ct
            break
    if not ct_info:
        return lambda out_dir: (True, "no verifier")

    def verify(out_dir: Path) -> Tuple[bool, str]:
        session_dir = find_session_dir(out_dir)
        if not session_dir:
            return False, f"no session directory found in {out_dir}"

        # 检查采集文件
        found_files = []
        for pattern in ct_info["file_patterns"]:
            matches = list(session_dir.glob(pattern))
            if matches:
                found_files.extend(matches)

        if not found_files:
            # 列出目录内容帮助诊断
            dir_contents = [f.name for f in session_dir.iterdir()]
            return False, (f"no capture file matching {ct_info['file_patterns']} "
                          f"in {session_dir}; contents: {dir_contents}")

        # 检查辅助文件（smaps / fds）
        smaps_prefix = ct_info.get("smaps_prefix", "")
        has_smaps = ct_info.get("smaps", False)
        if has_smaps or smaps_prefix:
            prefix = smaps_prefix if smaps_prefix else "smaps"
            smaps_files = list(session_dir.glob(f"{prefix}_*.txt"))
            if not smaps_files:
                # smaps/fds 可能因设备状态未导出，标记为 warning 而非 fail
                log(f"    [WARN] no {prefix}_*.txt in session dir (may be device state)")

        return True, f"session={session_dir.name}, files={[f.name for f in found_files]}"

    return verify


# ============================================================
# 测试套件构建
# ============================================================
def suite_help_tests(exe: Path) -> List[TestCase]:
    """--help 输出验证。"""
    return [
        TestCase(
            name="main --help",
            suite="help",
            cmd=[str(exe), "--help"],
            expect_exit_zero=True,
            expect_stdout_contains="profiler",
            note="主命令 --help 应列出 profiler 子命令"),
        TestCase(
            name="profiler --help",
            suite="help",
            cmd=[str(exe), "profiler", "--help"],
            expect_exit_zero=True,
            expect_stdout_contains="category",
            note="profiler 子命令 --help 应包含 category 选项说明"),
        TestCase(
            name="prof --help (subcommand alias)",
            suite="help",
            cmd=[str(exe), "prof", "--help"],
            expect_exit_zero=True,
            expect_stdout_contains="category",
            note="prof 是 profiler 的子命令别名，--help 应一致"),
    ]


def suite_negative_tests(exe: Path, bundle: str) -> List[TestCase]:
    """参数校验负面用例（期望失败）。"""
    return [
        TestCase(
            name="no bundle without --list-apps",
            suite="negative",
            cmd=[str(exe), "profiler"],
            expect_exit_zero=False,
            expect_stderr_contains="bundle",
            timeout=60,
            note="无 bundle 且无 --list-apps 时应报错"),
        TestCase(
            name="invalid category",
            suite="negative",
            cmd=[str(exe), "profiler", "-c", "invalid_category", bundle],
            expect_exit_zero=False,
            expect_stderr_contains="invalid category",
            timeout=60,
            note="无效 category 应报错"),
        TestCase(
            name="empty category",
            suite="negative",
            cmd=[str(exe), "profiler", "-c", "", bundle],
            expect_exit_zero=False,
            expect_stderr_contains="invalid category",
            timeout=60,
            note="空 category 应报错"),
        TestCase(
            name="empty --device value",
            suite="negative",
            cmd=[str(exe), "profiler", "--device", "", bundle],
            expect_exit_zero=False,
            expect_stderr_contains="non-empty",
            timeout=60,
            note="--device 空值应报错"),
    ]


def suite_list_apps_tests(exe: Path) -> List[TestCase]:
    """--list-apps 功能测试。"""
    return [
        TestCase(
            name="profiler --list-apps",
            suite="list_apps",
            cmd=[str(exe), "profiler", "--list-apps"],
            expect_exit_zero=True,
            expect_stdout_contains="Installed apps",
            timeout=30,
            note="--list-apps 应列出设备上已安装应用"),
        TestCase(
            name="prof --list-apps (alias)",
            suite="list_apps",
            cmd=[str(exe), "prof", "--list-apps"],
            expect_exit_zero=True,
            expect_stdout_contains="Installed apps",
            timeout=30,
            note="prof 别名应与 profiler 行为一致"),
    ]


def suite_capture_type_tests(exe: Path, bundle: str, duration: int,
                             out_base: Path) -> List[TestCase]:
    """全量 10 种采集类型测试（全名）。"""
    tests = []
    for ct in CAPTURE_TYPES:
        category = ct["category"]
        out_dir = out_base / f"capture_{category}"
        out_dir.mkdir(parents=True, exist_ok=True)

        cmd = [str(exe), "profiler", "-c", category, "-d", str(out_dir)]
        # snapshot 无 -t 时为立即抓取模式；其他类型使用指定时长
        if not ct["immediate"]:
            cmd.extend(["-t", str(duration)])
        cmd.append(bundle)

        verifier = make_capture_verifier(category)
        tests.append(TestCase(
            name=f"capture: {category}",
            suite="capture",
            cmd=cmd,
            expect_exit_zero=True,
            timeout=duration + 120,  # 采集时长 + 余量
            out_dir=out_dir,
            verify_output=verifier,
            note=f"采集类型 {category}" +
                 (f" (-t {duration}s)" if not ct["immediate"] else " (immediate)")))
    return tests


def suite_alias_tests(exe: Path, bundle: str, duration: int,
                      out_base: Path) -> List[TestCase]:
    """采集类型别名测试。"""
    tests = []
    for alias, full_name in ALL_ALIASES:
        ct = next(c for c in CAPTURE_TYPES if c["category"] == full_name)
        out_dir = out_base / f"alias_{alias}"
        out_dir.mkdir(parents=True, exist_ok=True)

        cmd = [str(exe), "profiler", "-c", alias, "-d", str(out_dir)]
        if not ct["immediate"]:
            cmd.extend(["-t", str(duration)])
        cmd.append(bundle)

        verifier = make_capture_verifier(full_name)
        tests.append(TestCase(
            name=f"alias: {alias} -> {full_name}",
            suite="alias",
            cmd=cmd,
            expect_exit_zero=True,
            expect_not_contains="invalid category",
            timeout=duration + 120,
            out_dir=out_dir,
            verify_output=verifier,
            note=f"别名 {alias} 应解析为 {full_name}"))
    return tests


def suite_option_combo_tests(exe: Path, bundle: str, duration: int,
                             out_base: Path,
                             device_id: Optional[str] = None) -> List[TestCase]:
    """选项组合测试。"""
    tests = []
    short_dur = min(duration, 5)  # 组合测试用较短时长

    # 1. snapshot 持续模式（-t）
    out_dir = out_base / "combo_snapshot_continuous"
    out_dir.mkdir(parents=True, exist_ok=True)
    verifier = make_capture_verifier("snapshot")
    tests.append(TestCase(
        name="snapshot -t (continuous mode)",
        suite="combo",
        cmd=[str(exe), "profiler", "-c", "snapshot", "-t", str(duration),
             "-d", str(out_dir), bundle],
        expect_exit_zero=True,
        timeout=duration + 120,
        out_dir=out_dir,
        verify_output=verifier,
        note=f"snapshot -t {duration}s 持续采集模式（start + end + peak）"))

    # 2. -o 自动生成报告（会打开浏览器）
    out_dir = out_base / "combo_auto_open"
    out_dir.mkdir(parents=True, exist_ok=True)
    tests.append(TestCase(
        name="-o auto-open report",
        suite="combo",
        cmd=[str(exe), "profiler", "-c", "nativeheap", "-t", str(short_dur),
             "-o", "-d", str(out_dir), bundle],
        expect_exit_zero=True,
        timeout=short_dur + 120,
        out_dir=out_dir,
        verify_output=lambda d: _verify_html_report(d),
        note="-o 采集后自动生成 HTML 报告（会打开浏览器）"))

    # 3. -d 自定义输出目录
    custom_dir = out_base / "combo_custom_outdir"
    custom_dir.mkdir(parents=True, exist_ok=True)
    verifier = make_capture_verifier("nativeheap")
    tests.append(TestCase(
        name="-d custom output dir",
        suite="combo",
        cmd=[str(exe), "profiler", "-c", "nativeheap", "-t", str(short_dur),
             "-d", str(custom_dir), bundle],
        expect_exit_zero=True,
        timeout=short_dur + 120,
        out_dir=custom_dir,
        verify_output=verifier,
        note="自定义输出目录"))

    # 4. --device 显式指定设备
    if device_id:
        out_dir = out_base / "combo_explicit_device"
        out_dir.mkdir(parents=True, exist_ok=True)
        verifier = make_capture_verifier("nativeheap")
        tests.append(TestCase(
            name="--device explicit device",
            suite="combo",
            cmd=[str(exe), "profiler", "--device", device_id,
                 "-c", "nativeheap", "-t", str(short_dur),
                 "-d", str(out_dir), bundle],
            expect_exit_zero=True,
            timeout=short_dur + 120,
            out_dir=out_dir,
            verify_output=verifier,
            note=f"显式指定设备 --device {device_id}"))

    # 5. --verbose 调试日志
    out_dir = out_base / "combo_verbose"
    out_dir.mkdir(parents=True, exist_ok=True)
    verifier = make_capture_verifier("nativeheap")
    tests.append(TestCase(
        name="--verbose debug logging",
        suite="combo",
        cmd=[str(exe), "profiler", "-c", "nativeheap", "-t", str(short_dur),
             "--verbose", "-d", str(out_dir), bundle],
        expect_exit_zero=True,
        timeout=short_dur + 120,
        out_dir=out_dir,
        verify_output=verifier,
        note="--verbose 启用调试级别日志"))

    # 6. -t 3 最小时长（-t 1 会导致 hiprofiler CreateSession 在 stop 信号到达前
    #    尚未完成，概率性失败；3 秒给会话创建留足余量）
    out_dir = out_base / "combo_min_duration"
    out_dir.mkdir(parents=True, exist_ok=True)
    verifier = make_capture_verifier("fd")
    min_dur = 3
    tests.append(TestCase(
        name=f"-t {min_dur} minimal duration",
        suite="combo",
        cmd=[str(exe), "profiler", "-c", "fd", "-t", str(min_dur),
             "-d", str(out_dir), bundle],
        expect_exit_zero=True,
        timeout=60,
        out_dir=out_dir,
        verify_output=verifier,
        note=f"-t {min_dur} 最小采集时长（避免 -t 1 竞态导致 CreateSession 失败）"))

    return tests


def _verify_html_report(out_dir: Path) -> Tuple[bool, str]:
    """验证 -o 生成了 HTML 报告。"""
    session_dir = find_session_dir(out_dir)
    if not session_dir:
        return False, f"no session directory in {out_dir}"
    html_files = list(session_dir.glob("*.html"))
    if not html_files:
        # 也检查上层目录
        html_files = list(out_dir.rglob("*.html"))
    if not html_files:
        return False, f"no HTML report found in {out_dir}"
    return True, f"HTML report: {[f.name for f in html_files]}"


# ============================================================
# 报告输出
# ============================================================
def print_summary(results: List[TestResult], report_path: Path):
    """打印测试汇总报告并保存 JSON。

    每条用例输出格式：
      [OK/FAIL] 测试名 (exit=N, N.Ns)
        CMD: <完整命令>
        JUDGE: PASS/FAIL — <判断依据>
    """
    log_header("TEST SUMMARY")

    total = len(results)
    passed = sum(1 for r in results if r.passed)
    failed = sum(1 for r in results if not r.passed and not r.skipped)
    skipped = sum(1 for r in results if r.skipped)

    # 按套件分组
    suites: Dict[str, List[TestResult]] = {}
    for r in results:
        suites.setdefault(r.suite, []).append(r)

    for suite_name in sorted(suites.keys()):
        suite_results = suites[suite_name]
        s_passed = sum(1 for r in suite_results if r.passed)
        s_failed = sum(1 for r in suite_results if not r.passed and not r.skipped)
        s_skipped = sum(1 for r in suite_results if r.skipped)
        s_total = len(suite_results)
        status = "ALL PASS" if s_failed == 0 and s_skipped == 0 else "HAS FAILURES"
        log(f"\n  [{suite_name}] {s_passed}/{s_total} passed, "
            f"{s_failed} failed, {s_skipped} skipped  ({status})")
        for r in suite_results:
            icon = "OK  " if r.passed else ("SKIP" if r.skipped else "FAIL")
            time_str = f"{r.duration_s:.1f}s" if r.duration_s > 0 else "-"
            exit_str = f"exit={r.exit_code}" if r.exit_code is not None else "no-exit"
            log(f"    [{icon}] {r.name} ({exit_str}, {time_str})")
            log(f"      CMD: {' '.join(r.cmd)}")
            if r.skipped:
                log(f"      JUDGE: SKIP — {r.detail}")
            elif r.passed:
                log(f"      JUDGE: PASS — {r.detail}")
            else:
                log(f"      JUDGE: FAIL — {r.detail}")

    log(f"\n  {'=' * 56}")
    log(f"  TOTAL: {passed}/{total} passed, {failed} failed, {skipped} skipped")
    total_time = sum(r.duration_s for r in results)
    log(f"  Total time: {total_time:.1f}s")
    log(f"  {'=' * 56}")

    # 保存 JSON 报告
    report_data = {
        "summary": {
            "total": total,
            "passed": passed,
            "failed": failed,
            "skipped": skipped,
            "total_time_s": round(total_time, 1),
            "timestamp": datetime.now().isoformat(),
        },
        "suites": {},
    }
    for suite_name, suite_results in suites.items():
        report_data["suites"][suite_name] = [
            {
                "name": r.name,
                "passed": r.passed,
                "skipped": r.skipped,
                "exit_code": r.exit_code,
                "duration_s": round(r.duration_s, 1),
                "cmd": " ".join(r.cmd),
                "judgment": r.detail,
                "checks": r.checks,
            }
            for r in suite_results
        ]

    report_path.parent.mkdir(parents=True, exist_ok=True)
    with open(report_path, "w", encoding="utf-8") as f:
        json.dump(report_data, f, indent=2, ensure_ascii=False)
    log(f"\n  Report saved to: {report_path}")


# ============================================================
# 主流程
# ============================================================
DEFAULT_BUNDLE = "抖音"
DEFAULT_DURATION = 10
DEFAULT_TIMEOUT_CAPTURE = 300  # 单次采集默认超时（秒）
DEFAULT_COOLDOWN = 5  # 采集间冷却秒数（等 hiprofilerd 清理上一个会话）

ALL_SUITE_NAMES = ["help", "negative", "list_apps", "capture", "alias", "combo"]
# 涉及设备采集的套件——这些测试之间需要冷却
CAPTURE_SUITES = {"capture", "alias", "combo"}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="profiler 子命令功能 / 集成测试脚本",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python build/test_profiler.py
  python build/test_profiler.py --exe path/to/mem-analyzer --bundle com.example.app
  python build/test_profiler.py --skip-build --exe out/base/mem-analyzer.exe
  python build/test_profiler.py --commit abc1234
  python build/test_profiler.py --suites help,negative,list_apps
  python build/test_profiler.py --duration 15 --device 33Z0224A22021343
        """
    )
    parser.add_argument("--exe", default=None,
                        help="直接指定 mem-analyzer 可执行文件路径（需在打包目录中，含 config/）")
    parser.add_argument("--skip-build", action="store_true",
                        help="跳过构建，使用 --exe 指定的可执行文件")
    parser.add_argument("--commit", default=None,
                        help="从指定 git commit 构建（使用 git worktree）")
    parser.add_argument("--project-root", default=None,
                        help="项目根目录（默认自动检测）")
    parser.add_argument("--bundle", default=DEFAULT_BUNDLE,
                        help=f"测试目标应用（默认: {DEFAULT_BUNDLE}，即抖音）")
    parser.add_argument("--duration", type=int, default=DEFAULT_DURATION,
                        help=f"采集时长（秒，默认: {DEFAULT_DURATION}）")
    parser.add_argument("--device", default=None,
                        help="目标设备序列号（多设备时必填，见 hdc list targets）")
    parser.add_argument("--output-dir", default=None,
                        help="测试输出目录（默认: <project-root>/out/test_profiler）")
    parser.add_argument("--suites", default="all",
                        help=f"要运行的测试套件（逗号分隔）：{','.join(ALL_SUITE_NAMES)} / all，默认 all")
    parser.add_argument("--timeout", type=int, default=DEFAULT_TIMEOUT_CAPTURE,
                        help=f"单次采集超时秒数（默认: {DEFAULT_TIMEOUT_CAPTURE}）")
    parser.add_argument("--capture-cooldown", type=int, default=DEFAULT_COOLDOWN,
                        help=f"采集间冷却秒数：连续采集间等待 hiprofilerd 清理上一个会话"
                             f"（默认: {DEFAULT_COOLDOWN}，设 0 禁用）")
    return parser.parse_args()


def resolve_suites(suites_str: str) -> List[str]:
    """解析 --suites 参数。"""
    if suites_str.lower() == "all":
        return list(ALL_SUITE_NAMES)
    selected = []
    for part in suites_str.split(","):
        name = part.strip().lower()
        if not name:
            continue
        if name == "all":
            return list(ALL_SUITE_NAMES)
        if name not in ALL_SUITE_NAMES:
            raise SystemExit(
                f"unknown suite: {part!r} (expect {','.join(ALL_SUITE_NAMES)} / all)")
        if name not in selected:
            selected.append(name)
    return selected


def main():
    # 自保护：复制到临时目录执行
    _ensure_self_contained()

    args = parse_args()
    plat = detect_platform()

    # 解析路径
    script_dir = Path(__file__).parent.resolve()
    saved_root = os.environ.get(_ENV_PROJECT_ROOT, "")
    project_root = (Path(args.project_root) if args.project_root
                    else Path(saved_root) if saved_root
                    else script_dir.parent)
    cpp_dir = project_root / "cpp"
    out_base = Path(args.output_dir) if args.output_dir else project_root / "out" / "test_profiler"
    suites = resolve_suites(args.suites)

    log("=" * 60)
    log("  mem-analyzer profiler subcommand test")
    log(f"  Platform:    {plat}")
    log(f"  Project:     {project_root}")
    log(f"  Bundle:      {args.bundle}")
    log(f"  Duration:    {args.duration}s")
    log(f"  Output:      {out_base}")
    log(f"  Suites:      {', '.join(suites)}")
    if args.device:
        log(f"  Device:      {args.device}")
    log("=" * 60)

    # 验证项目根目录
    if not project_root.exists():
        log(f"ERROR: Project root not found: {project_root}")
        sys.exit(1)

    # Phase 0: 前置检查
    log_header("PHASE 0: Pre-flight checks")

    log("  Checking hdc...")
    if not check_hdc():
        log("ERROR: hdc not found or not working. Please install HarmonyOS SDK and add hdc to PATH.")
        sys.exit(1)
    log("  hdc OK.")

    log("  Checking connected device...")
    devices = get_device_list()
    if not devices:
        log("ERROR: No connected device found. Please connect a HarmonyOS device via USB.")
        sys.exit(1)
    log(f"  Found {len(devices)} device(s): {devices}")

    device_id = args.device
    if not device_id:
        if len(devices) == 1:
            device_id = devices[0]
            log(f"  Auto-selected device: {device_id}")
        else:
            log(f"  WARNING: {len(devices)} devices connected. "
                "Use --device to specify the target.")
            log(f"  Profiler will auto-detect, but some tests may fail if it can't resolve.")
    elif device_id not in devices:
        log(f"  WARNING: --device {device_id} not in device list {devices}")

    # Phase 1: 构建 / 获取 exe
    if args.skip_build:
        log_header("PHASE 1: Skip build (using existing exe)")
        if not args.exe:
            log("ERROR: --skip-build requires --exe")
            sys.exit(1)
        exe = Path(args.exe)
        if not exe.exists():
            log(f"ERROR: exe not found: {exe}")
            sys.exit(1)
        log(f"  Using: {exe}")
    else:
        if args.exe:
            exe = Path(args.exe)
            if not exe.exists():
                log(f"ERROR: exe not found: {exe}")
                sys.exit(1)
            log(f"  Using existing exe: {exe}")
        else:
            log_header("PHASE 1: Build")
            exe = build_exe(project_root, cpp_dir, plat, commit=args.commit)

    # 确认 exe 存在
    if not exe.exists():
        log(f"ERROR: exe not found: {exe}")
        sys.exit(1)

    # exe 所在目录（profiler 需要从打包目录运行，config/ 按相对路径查找）
    exe_dir = exe.parent
    config_check = exe_dir / "config" / "profiler"
    if not config_check.exists():
        log(f"  [WARN] config/profiler/ not found at {config_check}")
        log(f"  [WARN] profiler may fail to load config. Ensure exe is in its release directory.")

    # 准备输出目录（用于保存测试报告 JSON）
    if out_base.exists():
        log(f"  Cleaning old output: {out_base}")
        shutil.rmtree(out_base)
    out_base.mkdir(parents=True, exist_ok=True)

    # 采集输出使用本地短路径，避免 UNC 路径 / 长路径导致 hdc file recv 失败。
    # profiler 的 PullFile 用 fs::exists 检查落地文件，UNC + 长路径 + 中文目录名
    # 会超出 Windows MAX_PATH(260)，hdc 报告 FileTransfer finish 但文件未落地，
    # profiler 判定 pullOk=false → EXIT_EXPORT_FAILED(3)。
    capture_out_base = out_base
    if needs_local_redirect(out_base):
        capture_out_base = Path(tempfile.mkdtemp(prefix="tp_capture_"))
        log(f"  [INFO] Capture outputs redirected to local temp: {capture_out_base}")
        log(f"  [INFO] Reason: UNC path or path too long (>180 chars) would cause")
        log(f"  [INFO]         hdc file recv failures (Windows MAX_PATH limitation).")
        log(f"  [INFO] Report JSON still saved to: {out_base}")

    # Phase 2: 构建测试用例
    log_header("PHASE 2: Building test cases")
    all_tests: List[TestCase] = []

    if "help" in suites:
        hlp = suite_help_tests(exe)
        all_tests.extend(hlp)
        log(f"  help: {len(hlp)} cases")

    if "negative" in suites:
        neg = suite_negative_tests(exe, args.bundle)
        all_tests.extend(neg)
        log(f"  negative: {len(neg)} cases")

    if "list_apps" in suites:
        la = suite_list_apps_tests(exe)
        all_tests.extend(la)
        log(f"  list_apps: {len(la)} cases")

    if "capture" in suites:
        cap = suite_capture_type_tests(exe, args.bundle, args.duration, capture_out_base)
        all_tests.extend(cap)
        log(f"  capture: {len(cap)} cases")

    if "alias" in suites:
        ali = suite_alias_tests(exe, args.bundle, args.duration, capture_out_base)
        all_tests.extend(ali)
        log(f"  alias: {len(ali)} cases")

    if "combo" in suites:
        cmb = suite_option_combo_tests(exe, args.bundle, args.duration, capture_out_base, device_id)
        all_tests.extend(cmb)
        log(f"  combo: {len(cmb)} cases")

    log(f"\n  Total: {len(all_tests)} test cases")

    # Phase 3: 执行测试（串行——设备采集不能并行）
    log_header("PHASE 3: Running tests (sequential)")
    if args.capture_cooldown > 0:
        log(f"  Capture cooldown: {args.capture_cooldown}s between capture tests")
        log(f"  (lets hiprofilerd clean up the previous session's shared memory/plugins)")

    results: List[TestResult] = []
    for idx, tc in enumerate(all_tests, 1):
        log(f"\n  --- Test {idx}/{len(all_tests)} ---")
        result = run_test_case(tc)
        results.append(result)

        # 采集测试之间冷却：等 hiprofilerd 异步清理上一个会话的共享内存/插件/临时文件，
        # 避免下一个 CreateSession 因资源未释放而概率性失败。
        if (args.capture_cooldown > 0
                and tc.suite in CAPTURE_SUITES
                and idx < len(all_tests)):
            log(f"  [COOLDOWN] Waiting {args.capture_cooldown}s for hiprofilerd cleanup...")
            time.sleep(args.capture_cooldown)

    # Phase 4: 汇总报告
    log_header("PHASE 4: Summary")
    report_path = out_base / "test_profiler_report.json"
    print_summary(results, report_path)

    # 退出码：有失败则输出失败用例详情并返回 1
    failed_results = [r for r in results if not r.passed and not r.skipped]
    if failed_results:
        log_header(f"FAILED CASES ({len(failed_results)})")
        for idx, r in enumerate(failed_results, 1):
            log(f"  [{idx}/{len(failed_results)}] [{r.suite}] {r.name}")
            log(f"    CMD: {' '.join(r.cmd)}")
            log(f"    EXIT: {r.exit_code}")
            log(f"    JUDGE: FAIL — {r.detail}")
        log(f"\n  {len(failed_results)} test(s) FAILED. Review output above.")
        sys.exit(1)
    else:
        log(f"\n  All tests PASSED.")
        sys.exit(0)


if __name__ == "__main__":
    main()
