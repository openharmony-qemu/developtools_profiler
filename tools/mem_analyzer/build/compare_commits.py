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
跨 commit 结果对比脚本（通用版）。

通过 git worktree 在不同 commit 上构建，避免 git checkout 导致脚本自身丢失的问题。

流程：
  Phase 1: 通过 git worktree 分别构建两个版本
  Phase 2: 分别跑测试（base 串行，new 并行；两者同时进行）
  Phase 3: 对比结果

用法：
  # 基本用法
  python build/compare_commits.py <base_commit> <new_commit>

  # 跳过构建（使用已有的 exe）
  python build/compare_commits.py <base_commit> <new_commit> --skip-build \
      --base-exe path/to/base/mem-analyzer.exe \
      --new-exe  path/to/new/mem-analyzer.exe

  # 指定项目根目录和测试目录
  python build/compare_commits.py <base_commit> <new_commit> \
      --project-root /path/to/project \
      --test-dir /path/to/tests

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
import difflib
from pathlib import Path
from datetime import datetime
from concurrent.futures import ThreadPoolExecutor, as_completed
from typing import List, Dict, Tuple, Optional, Any

# ============================================================
# 自保护：将脚本复制到临时目录并重新执行
# ============================================================
_ENV_KEY = "_COMPARE_COMMITS_TEMP_DIR"
_ENV_PROJECT_ROOT = "_COMPARE_COMMITS_PROJECT_ROOT"


def _ensure_self_contained():
    """确保脚本在临时目录中运行，避免 git 操作影响自身。"""
    if _ENV_KEY in os.environ:
        return  # 已在临时目录中运行

    # 保存原始 project root（在自复制前，__file__ 指向原始位置）
    script_dir = Path(__file__).parent.resolve()
    original_root = script_dir.parent
    os.environ[_ENV_PROJECT_ROOT] = str(original_root)

    temp_dir = Path(tempfile.mkdtemp(prefix="compare_commits_"))
    temp_script = temp_dir / Path(__file__).name
    shutil.copy2(__file__, temp_script)
    os.environ[_ENV_KEY] = str(temp_dir)

    log_raw(f"[SELF] Original project root: {original_root}")
    log_raw(f"[SELF] Copied script to: {temp_script}")
    log_raw(f"[SELF] Re-executing from temp location...")

    # 替换当前进程为临时副本
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
            log(f"  [FAIL] stderr: {result.stderr[-300:]}")
        else:
            log(f"  [FAIL] returncode={result.returncode}")
        if check:
            raise RuntimeError(f"Command failed: {' '.join(cmd)}")
    return result


def run_powershell(script: str, cwd: str = None, timeout: int = 1800) -> subprocess.CompletedProcess:
    """运行 PowerShell 脚本，输出直接到终端。"""
    return run_cmd(["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass",
                     "-File", script], cwd=cwd, timeout=timeout, capture=False)


# ============================================================
# git worktree 操作（替代 git checkout）
# ============================================================
class GitWorktree:
    """管理 git worktree 的生命周期。"""

    def __init__(self, repo_path: Path, commit: str, label: str):
        self.repo_path = repo_path
        self.commit = commit
        self.label = label
        # 使用随机后缀避免并发冲突
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
        """强制移除 worktree（即使有残留锁文件）。"""
        # 先尝试正常移除
        run_cmd(["git", "worktree", "remove", "--force", str(self.worktree_path)],
                cwd=str(self.repo_path), timeout=60, capture=True)
        # 如果 git worktree remove 失败，手动清理目录和 git 元数据
        if self.worktree_path.exists():
            try:
                shutil.rmtree(self.worktree_path, ignore_errors=True)
            except PermissionError:
                pass
        # 清理 git 元数据
        wt_meta = self.repo_path / ".git" / "worktrees" / self.worktree_path.name
        if wt_meta.exists():
            try:
                shutil.rmtree(wt_meta, ignore_errors=True)
            except PermissionError:
                pass
        # 清理 git worktree 列表中的条目
        run_cmd(["git", "worktree", "prune"], cwd=str(self.repo_path),
                timeout=30, capture=True)


# ============================================================
# 构建
# ============================================================
def build_to_dir(commit: str, build_dir_name: str, label: str,
                 project_root: Path, cpp_dir: Path) -> Path:
    """
    通过 git worktree 检出 commit，构建，解压 ZIP 到目标目录。
    返回 mem-analyzer.exe 的路径。
    """
    log_header(f"BUILD: {label} ({commit[:12]}...) -> cpp/{build_dir_name}/")

    final_dir = cpp_dir / build_dir_name
    if final_dir.exists():
        log(f"  Cleaning: {final_dir}")
        shutil.rmtree(final_dir)

    with GitWorktree(project_root, commit, build_dir_name) as wt_path:
        wt_cpp = wt_path / "cpp"
        build_script = wt_cpp / "build-win.ps1"

        if not build_script.exists():
            # 尝试查找其他构建脚本
            alt_scripts = list(wt_cpp.glob("build*.ps1")) + list(wt_cpp.glob("build*.sh"))
            if alt_scripts:
                build_script = alt_scripts[0]
            else:
                raise FileNotFoundError(f"No build script found in {wt_cpp}")

        log(f"  Running {build_script.name} ...")
        r = run_powershell(str(build_script), cwd=str(wt_cpp), timeout=1800)

        # 查找生成的 ZIP 文件（在 worktree 的 cpp 目录下）
        zip_files = sorted(wt_cpp.glob("mem-analyzer-*-win64.zip"))
        if not zip_files:
            zip_files = sorted(wt_cpp.glob("mem-analyzer-*-macos.zip"))
        if not zip_files:
            zip_files = sorted(wt_cpp.glob("mem-analyzer-*-linux.zip"))

        if not zip_files:
            # ZIP 未生成，尝试从 build/ 目录直接取
            tmp_build = wt_cpp / "build"
            exe_candidate = tmp_build / "mem-analyzer.exe"
            if not exe_candidate.exists():
                exe_candidate = tmp_build / "Release" / "mem-analyzer.exe"
            if not exe_candidate.exists():
                raise RuntimeError(
                    f"Build failed for {label}! exit={r.returncode}, no ZIP and no exe found"
                )
            log(f"  Copying {tmp_build} -> {final_dir}")
            shutil.copytree(str(tmp_build), str(final_dir))
        else:
            zip_path = zip_files[-1]
            log(f"  Extracting ZIP: {zip_path.name} -> {final_dir}/")
            final_dir.mkdir(parents=True, exist_ok=True)
            with zipfile.ZipFile(zip_path, "r") as zf:
                zf.extractall(str(final_dir))
            log(f"  Extracted OK.")

        exe = final_dir / "mem-analyzer.exe"
        if not exe.exists():
            # macOS / Linux compatibility
            exe = final_dir / "mem-analyzer"
        if not exe.exists():
            raise FileNotFoundError(f"mem-analyzer executable not found in {final_dir}")
        log(f"  Build OK: {exe}")
        return exe


# ============================================================
# 测试文件收集 & category 映射
# ============================================================
def collect_test_files(test_dir: Path) -> Dict[str, List[Path]]:
    files = {
        "htrace": sorted(test_dir.glob("*.htrace")),
        "snapshot": sorted(test_dir.glob("*.heapsnapshot")) + sorted(test_dir.glob("*.rawheap")),
        "zip": sorted(test_dir.glob("*.zip")),
        "rawheap_compare": collect_rawheap_pairs(test_dir),
    }
    log("  Test files:")
    for typ, flist in files.items():
        if typ == "rawheap_compare":
            log(f"    {typ}: {len(flist)} pair(s)")
            for start_f, end_f in flist:
                log(f"      - {start_f.name} vs {end_f.name}")
        else:
            log(f"    {typ}: {len(flist)} file(s)")
            for f in flist:
                log(f"      - {f.name}")
    return files


# 模块名（用户视角）→ 测试类别（collect_test_files 的 key）
_MODULE_TO_CATEGORY = {
    "htrace": "htrace",
    "jsleak": "snapshot",
    "tshandle": "zip",
}


def collect_rawheap_pairs(test_dir: Path) -> List[Tuple[Path, Path]]:
    """收集 *-start.rawheap / *-end.rawheap 配对，用于 jsleak -b 差异分析测试。

    文件名格式：hidumper-jsheap-{pid}-{tid}-{timestamp}-{role}.rawheap
    其中 timestamp 在 start/end 之间不同，需要去掉末尾时间戳后再按前缀配对。
    例如：
      hidumper-jsheap-30192-30192-1793067300751-start.rawheap
      hidumper-jsheap-30192-30192-1793067436714-end.rawheap
    去掉 -{timestamp} 后前缀均为 hidumper-jsheap-30192-30192，配对成功。
    """
    rawheap_files = sorted(test_dir.glob("*.rawheap"))
    starts = {}
    ends = {}
    for f in rawheap_files:
        name_lower = f.name.lower()
        if name_lower.endswith("-start.rawheap"):
            prefix = name_lower[:-len("-start.rawheap")]
            prefix = re.sub(r'-\d+$', '', prefix)  # 去掉末尾时间戳
            starts[prefix] = f
        elif name_lower.endswith("-end.rawheap"):
            prefix = name_lower[:-len("-end.rawheap")]
            prefix = re.sub(r'-\d+$', '', prefix)  # 去掉末尾时间戳
            ends[prefix] = f
    pairs = []
    for prefix in sorted(set(starts.keys()) & set(ends.keys())):
        pairs.append((starts[prefix], ends[prefix]))
    return pairs


def resolve_modules(modules_str: str) -> List[str]:
    """解析 --modules，返回要跑的类别列表（htrace / snapshot / zip）。

    接受逗号分隔的模块名：htrace / jsleak / tshandle / all（默认 all = 全量）。
    """
    selected = []
    for part in (modules_str or "all").split(","):
        name = part.strip().lower()
        if not name:
            continue
        if name == "all":
            return ["htrace", "snapshot", "zip"]
        if name not in _MODULE_TO_CATEGORY:
            raise SystemExit(
                f"unknown module: {part!r} (expect htrace / jsleak / tshandle / all)")
        cat = _MODULE_TO_CATEGORY[name]
        if cat not in selected:
            selected.append(cat)
    return selected


def get_htrace_category(filename: str) -> Optional[str]:
    name_lower = filename.lower()
    if "dma" in name_lower:
        return "dma"
    if "ashmem" in name_lower:
        return "ashmem"
    if "thread" in name_lower:
        return "thread"
    if "fd" in name_lower:
        return "fd"
    return None


# ============================================================
# 测试执行
# ============================================================
def run_htrace_tests_sequential(exe: Path, files: List[Path], out_base: Path):
    total = len(files)
    for idx, f in enumerate(files, 1):
        stem = f.stem
        category = get_htrace_category(f.name)
        cat_flag = ["-c", category] if category else []
        cat_str = f" (category={category})" if category else ""

        log(f"\n  [{idx}/{total}] htrace-parse: {f.name}")
        run_cmd([str(exe), "trace", str(f), "-p", "-f", "json",
                 "-d", str(out_base / "parse")], label=f"parse:{stem}")

        log(f"\n  [{idx}/{total}] htrace-analysis-json: {f.name}{cat_str}")
        run_cmd([str(exe), "trace", str(f), "-f", "json",
                 "-d", str(out_base / "analysis_json")] + cat_flag,
                label=f"analysis-json:{stem}")

        log(f"\n  [{idx}/{total}] htrace-analysis-html: {f.name}{cat_str}")
        run_cmd([str(exe), "trace", str(f), "-f", "html",
                 "-d", str(out_base / "analysis_html")] + cat_flag,
                label=f"analysis-html:{stem}")


def run_htrace_tests_parallel(exe: Path, files: List[Path], out_base: Path) -> List[Tuple]:
    tasks = []
    for f in files:
        stem = f.stem
        category = get_htrace_category(f.name)
        cat_flag = ["-c", category] if category else []
        tasks.append((f"parse:{stem}", [str(exe), "trace", str(f), "-p", "-f", "json",
                                        "-d", str(out_base / "parse")]))
        tasks.append((f"analysis-json:{stem}", [str(exe), "trace", str(f), "-f", "json",
                                                "-d", str(out_base / "analysis_json")] + cat_flag))
        tasks.append((f"analysis-html:{stem}", [str(exe), "trace", str(f), "-f", "html",
                                                "-d", str(out_base / "analysis_html")] + cat_flag))
    return tasks


def run_zip_tests_sequential(exe: Path, files: List[Path], out_base: Path):
    total = len(files)
    for idx, f in enumerate(files, 1):
        stem = f.stem
        log(f"\n  [{idx}/{total}] tshandle-parse: {f.name}")
        run_cmd([str(exe), "tshandle", str(f), "-p", "-f", "json",
                 "-d", str(out_base / "parse")], label=f"parse:{stem}")
        log(f"\n  [{idx}/{total}] tshandle-analysis-json: {f.name}")
        run_cmd([str(exe), "tshandle", str(f), "-f", "json",
                 "-d", str(out_base / "analysis_json")], label=f"analysis-json:{stem}")
        log(f"\n  [{idx}/{total}] tshandle-analysis-html: {f.name}")
        run_cmd([str(exe), "tshandle", str(f), "-f", "html",
                 "-d", str(out_base / "analysis_html")], label=f"analysis-html:{stem}")


def run_zip_tests_parallel(exe: Path, files: List[Path], out_base: Path) -> List[Tuple]:
    tasks = []
    for f in files:
        stem = f.stem
        tasks.append((f"parse:{stem}", [str(exe), "tshandle", str(f), "-p", "-f", "json",
                                        "-d", str(out_base / "parse")]))
        tasks.append((f"analysis-json:{stem}", [str(exe), "tshandle", str(f), "-f", "json",
                                                "-d", str(out_base / "analysis_json")]))
        tasks.append((f"analysis-html:{stem}", [str(exe), "tshandle", str(f), "-f", "html",
                                                "-d", str(out_base / "analysis_html")]))
    return tasks


def run_snapshot_tests_sequential(exe: Path, files: List[Path], out_base: Path):
    total = len(files)
    for idx, f in enumerate(files, 1):
        stem = f.stem
        log(f"\n  [{idx}/{total}] jsleak-html: {f.name}")
        run_cmd([str(exe), "jsleak", str(f), "-d", str(out_base / "analysis_html")],
                label=f"jsleak-html:{stem}")
        log(f"\n  [{idx}/{total}] jsleak-json: {f.name}")
        run_cmd([str(exe), "jsleak", str(f), "-f", "json",
                 "-d", str(out_base / "analysis_json")], label=f"jsleak-json:{stem}")


def _make_short_out_dir(out_base: Path) -> Path:
    """创建短路径输出目录，避免 WSL UNC 路径 + 长文件名超过 Windows MAX_PATH。

    在系统 TEMP 目录下创建临时目录，测试完成后由调用方拷贝回 out_base/cmp。
    """
    short_dir = Path(tempfile.mkdtemp(prefix="rc_out_"))
    return short_dir


def _copy_short_results(short_dir: Path, out_base: Path):
    """将短路径目录的结果拷贝回 out_base/cmp。

    mem-analyzer 的 ProcessFile 会在 outDir 下创建以源文件 stem 命名的子目录，
    所以需要 rglob 递归搜索 *.html 和 *.json，不能只搜顶层。
    拷贝回 WSL 路径时用短文件名避免 MAX_PATH。
    """
    dest = out_base / "cmp"
    dest.mkdir(parents=True, exist_ok=True)
    if short_dir.exists():
        html_files = sorted(short_dir.rglob("*.html"))
        json_files = sorted(short_dir.rglob("*.json"))
        for idx, f in enumerate(html_files):
            short_name = f"compare_{idx}.html"
            shutil.copy2(str(f), str(dest / short_name))
            log(f"  [COPY] {f.relative_to(short_dir)} -> {short_name}")
        for idx, f in enumerate(json_files):
            short_name = f"compare_{idx}.json"
            shutil.copy2(str(f), str(dest / short_name))
            log(f"  [COPY] {f.relative_to(short_dir)} -> {short_name}")
        shutil.rmtree(short_dir, ignore_errors=True)


def run_rawheap_compare_tests_sequential(exe: Path, pairs: List[Tuple[Path, Path]],
                                         out_base: Path):
    total = len(pairs)
    short_dir = _make_short_out_dir(out_base)
    try:
        for idx, (start_f, end_f) in enumerate(pairs, 1):
            label_stem = f"{start_f.stem}-vs-{end_f.stem}"
            log(f"\n  [{idx}/{total}] jsleak-compare-html: {start_f.name} vs {end_f.name}")
            run_cmd([str(exe), "jsleak", "-b", str(start_f), str(end_f),
                     "-d", str(short_dir)],
                    label=f"jsleak-compare-html:{label_stem}")
    finally:
        _copy_short_results(short_dir, out_base)


def run_snapshot_tests_parallel(exe: Path, files: List[Path], out_base: Path) -> List[Tuple]:
    tasks = []
    for f in files:
        stem = f.stem
        tasks.append((f"jsleak-html:{stem}", [str(exe), "jsleak", str(f),
                                              "-d", str(out_base / "analysis_html")]))
        tasks.append((f"jsleak-json:{stem}", [str(exe), "jsleak", str(f), "-f", "json",
                                              "-d", str(out_base / "analysis_json")]))
    return tasks


def run_rawheap_compare_tests_parallel(exe: Path, pairs: List[Tuple[Path, Path]],
                                       out_base: Path) -> List[Tuple]:
    # 使用系统 TEMP 短路径，避免 WSL UNC 路径 + 长文件名超过 Windows MAX_PATH
    short_dir = _make_short_out_dir(out_base)
    tasks = []
    for start_f, end_f in pairs:
        label_stem = f"{start_f.stem}-vs-{end_f.stem}"
        tasks.append((f"jsleak-compare-html:{label_stem}",
                      [str(exe), "jsleak", "-b", str(start_f), str(end_f),
                       "-d", str(short_dir)]))
    # 将短路径信息附加到任务列表末尾（哨兵任务），供 run_tests_parallel 在完成后拷贝
    tasks.append(("__copy_short_results__", [str(short_dir)]))
    return tasks


def run_tests_sequential(exe: Path, out_base: Path, test_files: Dict[str, List[Path]],
                         label: str):
    log_header(f"TEST: {label} (SEQUENTIAL)")
    if out_base.exists():
        shutil.rmtree(out_base)
    out_base.mkdir(parents=True, exist_ok=True)
    run_htrace_tests_sequential(exe, test_files["htrace"], out_base)
    run_zip_tests_sequential(exe, test_files["zip"], out_base)
    run_snapshot_tests_sequential(exe, test_files["snapshot"], out_base)
    run_rawheap_compare_tests_sequential(exe, test_files.get("rawheap_compare", []), out_base)
    log(f"  {label} tests DONE.")


def run_tests_parallel(exe: Path, out_base: Path, test_files: Dict[str, List[Path]],
                       label: str, max_workers: int = 4):
    log_header(f"TEST: {label} (PARALLEL, {max_workers} workers)")
    if out_base.exists():
        shutil.rmtree(out_base)
    out_base.mkdir(parents=True, exist_ok=True)

    all_tasks = []
    all_tasks.extend(run_htrace_tests_parallel(exe, test_files["htrace"], out_base))
    all_tasks.extend(run_zip_tests_parallel(exe, test_files["zip"], out_base))
    all_tasks.extend(run_snapshot_tests_parallel(exe, test_files["snapshot"], out_base))
    all_tasks.extend(run_rawheap_compare_tests_parallel(
        exe, test_files.get("rawheap_compare", []), out_base))

    # 分离真实任务和哨兵任务（短路径拷贝）
    real_tasks = [t for t in all_tasks if t[0] != "__copy_short_results__"]
    sentinel_tasks = [t for t in all_tasks if t[0] == "__copy_short_results__"]

    total = len(real_tasks)
    completed = 0
    failed = 0

    def run_one(task):
        label_t, cmd = task
        result = subprocess.run(cmd, capture_output=True, text=True,
                                timeout=600, encoding="utf-8", errors="replace")
        return label_t, result.returncode, result.stderr[-200:] if result.returncode != 0 else ""

    with ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = {executor.submit(run_one, t): t for t in real_tasks}
        for future in as_completed(futures):
            label_t, rc, err = future.result()
            completed += 1
            if rc != 0:
                failed += 1
                log(f"  [{completed}/{total}] FAIL: {label_t}")
                if err:
                    log(f"    {err}")
            else:
                log(f"  [{completed}/{total}] OK: {label_t}")

    # 所有并行任务完成后，拷贝短路径结果到 out_base/cmp
    for _, cmd in sentinel_tasks:
        short_dir = Path(cmd[0])
        log(f"  [COPY] Copying from short dir: {short_dir}")
        if short_dir.exists():
            files_found = list(short_dir.glob("*.html")) + list(short_dir.glob("*.json"))
            log(f"  [COPY] Found {len(files_found)} file(s) in short dir")
            _copy_short_results(short_dir, out_base)
        else:
            log(f"  [COPY] WARNING: short dir does not exist!")

    log(f"  {label} tests DONE: {total - failed}/{total} passed, {failed} failed.")


# ============================================================
# JSON 对比
# ============================================================
def load_json(path: Path) -> Optional[dict]:
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        log(f"  [WARN] Failed to load {path}: {e}")
        return None


def strip_meta(data: dict) -> dict:
    if "meta" in data:
        for key in ["generateTimestamp", "generatorVersion", "command",
                     "sourceFileInfo", "benchmark"]:
            data["meta"].pop(key, None)
    return data


def try_convert_str(v):
    """尝试将字符串转为 int/float，返回 (converted_value, type_name) 或 (None, None)。"""
    if not isinstance(v, str):
        return None, None
    try:
        return int(v), 'int'
    except ValueError:
        pass
    try:
        return float(v), 'float'
    except ValueError:
        pass
    return None, None


def values_equivalent(a: Any, b: Any) -> bool:
    """判断两个值是否语义等价（类型可能不同，如 '123' vs 123）。"""
    if isinstance(a, type(b)):
        return a == b
    if isinstance(a, str) and isinstance(b, (int, float)):
        cv, _ = try_convert_str(a)
        return cv is not None and cv == b
    if isinstance(b, str) and isinstance(a, (int, float)):
        cv, _ = try_convert_str(b)
        return cv is not None and cv == a
    if isinstance(a, (int, float)) and isinstance(b, (int, float)):
        return float(a) == float(b)
    return False


def deep_compare(a: Any, b: Any, path: str = "$") -> List[str]:
    """深度对比，对 str↔int/float 类型变化做语义等价检测。"""
    diffs = []
    if not isinstance(a, type(b)):
        if values_equivalent(a, b):
            diffs.append(f"{path}: TYPE CHANGE {type(a).__name__}->{type(b).__name__} "
                         f"[EQUIVALENT] base={a!r} new={b!r}")
        else:
            diffs.append(f"{path}: TYPE CHANGE {type(a).__name__}->{type(b).__name__} "
                         f"[REAL DIFF] base={a!r} new={b!r}")
        return diffs
    if isinstance(a, dict):
        keys_a, keys_b = set(a.keys()), set(b.keys())
        for k in keys_a - keys_b:
            diffs.append(f"{path}.{k}: missing in new")
        for k in keys_b - keys_a:
            diffs.append(f"{path}.{k}: missing in base")
        for k in keys_a & keys_b:
            diffs.extend(deep_compare(a[k], b[k], f"{path}.{k}"))
    elif isinstance(a, list):
        if len(a) != len(b):
            diffs.append(f"{path}: list length mismatch ({len(a)} vs {len(b)})")
        for i in range(min(len(a), len(b))):
            diffs.extend(deep_compare(a[i], b[i], f"{path}[{i}]"))
    elif isinstance(a, float) and isinstance(b, float):
        if abs(a - b) > 1e-6 and (abs(a) > 1e-10 or abs(b) > 1e-10):
            diffs.append(f"{path}: value mismatch ({a} vs {b})")
    else:
        if a != b:
            diffs.append(f"{path}: value mismatch ({a!r} vs {b!r})")
    return diffs


def classify_diffs(diffs: List[str]) -> Dict[str, int]:
    """统计差异类型。"""
    counts = {"equivalent": 0, "real_diff": 0, "key_diff": 0, "value_diff": 0}
    for d in diffs:
        if "[EQUIVALENT]" in d:
            counts["equivalent"] += 1
        elif "[REAL DIFF]" in d:
            counts["real_diff"] += 1
        elif "missing in" in d:
            counts["key_diff"] += 1
        else:
            counts["value_diff"] += 1
    return counts


def compare_json_files(base_path: Path, new_path: Path) -> Tuple[int, List[str]]:
    base_data = load_json(base_path)
    new_data = load_json(new_path)
    if base_data is None and new_data is None:
        return 0, []
    if base_data is None:
        return 1, ["base file failed to load"]
    if new_data is None:
        return 1, ["new file failed to load"]
    base_data = strip_meta(base_data)
    new_data = strip_meta(new_data)
    diffs = deep_compare(base_data, new_data)
    return len(diffs), diffs


# ============================================================
# tshandle parse 专项对比
# ============================================================
def compare_tshandle_parse(base_path: Path, new_path: Path) -> dict:
    base = load_json(base_path)
    new = load_json(new_path)
    report = {
        "file": base_path.name,
        "base_exists": base is not None,
        "new_exists": new is not None,
        "tracemeta_diffs": [],
        "addrs_diffs": [],
        "other_diffs": [],
        "summary": ""
    }
    if base is None or new is None:
        report["summary"] = "one or both files missing"
        return report

    base_c = base.get("content", {})
    new_c = new.get("content", {})

    # --- tracemeta ---
    b_tm = base_c.get("tracemeta", {})
    n_tm = new_c.get("tracemeta", {})
    if isinstance(b_tm, dict) and isinstance(n_tm, dict):
        for key in sorted(set(b_tm.keys()) | set(n_tm.keys())):
            bv = b_tm.get(key)
            nv = n_tm.get(key)
            if key not in b_tm:
                report["tracemeta_diffs"].append(f"  '{key}': only in new = {nv!r}")
            elif key not in n_tm:
                report["tracemeta_diffs"].append(f"  '{key}': only in base = {bv!r}")
            elif not isinstance(bv, type(nv)):
                tag = "[EQUIVALENT]" if values_equivalent(bv, nv) else "[REAL DIFF]"
                report["tracemeta_diffs"].append(
                    f"  '{key}': {type(bv).__name__}->{type(nv).__name__} {tag} base={bv!r} new={nv!r}")
            elif bv != nv:
                report["tracemeta_diffs"].append(f"  '{key}': value mismatch  base={bv!r}  new={nv!r}")

    # --- addrs ---
    b_addrs = base_c.get("addrs", [])
    n_addrs = new_c.get("addrs", [])
    if isinstance(b_addrs, list) and isinstance(n_addrs, list):
        if len(b_addrs) != len(n_addrs):
            report["addrs_diffs"].append(f"  length mismatch: base={len(b_addrs)} new={len(n_addrs)}")
        for i in range(min(len(b_addrs), len(n_addrs))):
            ba, na = b_addrs[i], n_addrs[i]
            if isinstance(ba, dict) and isinstance(na, dict):
                for ak in sorted(set(ba.keys()) | set(na.keys())):
                    bav, nav = ba.get(ak), na.get(ak)
                    if ak not in ba:
                        report["addrs_diffs"].append(f"  addr[{i}].{ak}: only in new")
                    elif ak not in na:
                        report["addrs_diffs"].append(f"  addr[{i}].{ak}: only in base")
                    elif not isinstance(bav, type(nav)):
                        tag = "[EQUIVALENT]" if values_equivalent(bav, nav) else "[REAL DIFF]"
                        report["addrs_diffs"].append(
                            f"  addr[{i}].{ak}: {type(bav).__name__}->{type(nav).__name__} {tag} base={bav!r} new={nav!r}")
                    elif bav != nav:
                        report["addrs_diffs"].append(f"  addr[{i}].{ak}: value mismatch  base={bav!r}  new={nav!r}")

    exclude = {"tracemeta", "addrs"}
    for key in sorted(set(base_c.keys()) | set(new_c.keys())):
        if key in exclude:
            continue
        bv, nv = base_c.get(key), new_c.get(key)
        if key not in base_c:
            report["other_diffs"].append(f"  content.{key}: only in new")
        elif key not in new_c:
            report["other_diffs"].append(f"  content.{key}: only in base")
        else:
            for d in deep_compare(bv, nv, f"content.{key}"):
                report["other_diffs"].append(f"  {d}")

    total = (len(report["tracemeta_diffs"]) + len(report["addrs_diffs"]) +
             len(report["other_diffs"]))
    if total == 0:
        report["summary"] = "IDENTICAL"
    else:
        parts = []
        if report["tracemeta_diffs"]:
            parts.append(f"tracemeta:{len(report['tracemeta_diffs'])}")
        if report["addrs_diffs"]:
            parts.append(f"addrs:{len(report['addrs_diffs'])}")
        if report["other_diffs"]:
            parts.append(f"other:{len(report['other_diffs'])}")
        report["summary"] = "; ".join(parts)
    return report


# ============================================================
# HTML 对比（增强版：归一化 + 全文 diff + 自动分类）
# ============================================================

# 归一化规则：将挥发性内容替换为占位符
NORMALIZE_RULES = [
    # 日期时间 (YYYY-MM-DD HH:MM:SS 或 YYYY-MM-DDTHH:MM:SS)
    (r'\d{4}-\d{2}-\d{2}[T ]\d{2}:\d{2}:\d{2}', '__TS__'),
    # 纯时间戳 (13位毫秒)
    (r'\b\d{13}\b', '__TS_MS__'),
    # Windows 绝对路径
    (r'[A-Za-z]:\\[^\s"\'<>]+', '__PATH__'),
    # Unix 绝对路径 (含文件名)
    (r'/(?:[\w.-]+/)+[\w.-]+\.(?:htrace|heapsnapshot|zip|html|json|db|txt)', '__PATH__'),
    # generateTimestamp / parsedAt / command / sourceFileInfo 等 meta 字段值
    (r'"(generateTimestamp|parsedAt|command|sourceFileInfo)"\s*:\s*"[^"]*"', '"__": "__"'),
    # 长哈希值 (32-64 hex)
    (r'\b[a-f0-9]{32,64}\b', '__HASH__'),
]

# 差异分类规则
DIFF_CATEGORY_RULES = {
    "structure": [
        r'^\s*</?\w+',           # HTML 标签
        r'class="',               # class 属性
        r'id="',                  # id 属性
    ],
    "style": [
        r'^\s*(\.|#|@|[\w-]+\s*\{)',  # CSS 选择器/规则
        r'(color|font|margin|padding|width|height|display|background|border)[\s:]*:',
        r'<style\b',             # <style> 标签
    ],
    "script": [
        r'^\s*function\s',       # JS 函数声明
        r'^\s*(const|let|var)\s', # JS 变量声明
        r'(addEventListener|onclick|onload|on\w+\s*=)',  # JS 事件
        r'<script\b',            # <script> 标签
        r'^\s*\}\s*$',           # JS 闭括号（单独一行）
        r'^\s*\)\s*;\s*$',       # JS 语句结束
    ],
    "data": [
        r'^\s*"[^"]+":\s*',      # JSON 键值对
        r'^\s*[\[\{]',           # JSON 数组/对象开始
        r'^\s*\d+[,\]]?',        # 纯数字
        r'<td[^>]*>\d+',         # 表格数字
        r'<td[^>]*>[\d.]+',      # 表格数值
        r'^\s*"[^"]*"[,]?',      # JSON 字符串值
        r'(true|false|null)',    # JSON 字面量
    ],
    "content": [
        r'>[^<]{5,}<',           # 5字符以上文本内容
        r'<title>',              # 页面标题
        r'<meta\s',              # meta 标签
        r'<h[1-6]\b',            # 标题
        r'<p\b',                 # 段落
        r'<a\b',                 # 链接
        r'<span[^>]*>[^<]+',     # span 文本
    ],
}


def normalize_html(text: str) -> str:
    """对 HTML 文本做归一化处理，替换挥发性内容。"""
    for pattern, replacement in NORMALIZE_RULES:
        text = re.sub(pattern, replacement, text)
    return text


def categorize_diff_line(line: str) -> str:
    """根据内容分类单行 diff。"""
    stripped = line[1:] if line.startswith(('+', '-', ' ')) else line
    stripped = stripped.strip()

    if not stripped:
        return "white_space"

    for category, rules in DIFF_CATEGORY_RULES.items():
        for rule in rules:
            if re.search(rule, stripped):
                return category
    return "uncategorized"


def merged_diff_blocks(diff_lines: list) -> list:
    """将相邻的 diff 行合并为块，每个块包含上下文行。"""
    blocks = []
    current = []
    for line in diff_lines:
        if line.startswith(('+', '-')):
            if not current:
                current.append(line)
            else:
                # 如果上一行是上下文行，开始新块
                last = current[-1]
                if last.startswith(' ') and not any(
                    l.startswith(('+', '-')) for l in current[-3:]
                ):
                    if len(current) > 1:
                        blocks.append(current)
                    current = [line]
                else:
                    current.append(line)
        elif line.startswith(' '):
            if current:
                current.append(line)
        elif line.startswith('@@'):
            if current:
                blocks.append(current)
                current = []
    if current:
        blocks.append(current)
    return blocks


def classify_diff_block(block: list) -> str:
    """对 diff 块进行分类，返回主要类别。"""
    categories = []
    for line in block:
        if line.startswith(('+', '-')):
            cat = categorize_diff_line(line)
            if cat not in ("white_space", "uncategorized"):
                categories.append(cat)
    if not categories:
        return "uncategorized"
    # 返回出现最多的类别
    return max(set(categories), key=categories.count)


def compare_html_files(base_path: Path, new_path: Path) -> dict:
    """
    增强 HTML 对比：
      1. size 对比
      2. 归一化字节对比
      3. 全文 diff + 自动分类
    """
    report = {
        "file": base_path.name,
        "base_exists": base_path.exists(),
        "new_exists": new_path.exists(),
        "base_size": base_path.stat().st_size if base_path.exists() else 0,
        "new_size": new_path.stat().st_size if new_path.exists() else 0,
        "method": "size-only",
        "summary": "",
        "diff_blocks": {},
        "details": [],
    }

    if not report["base_exists"] and not report["new_exists"]:
        report["summary"] = "both missing"
        return report
    if not report["base_exists"]:
        report["summary"] = "base missing"
        return report
    if not report["new_exists"]:
        report["summary"] = "new missing"
        return report

    # Step 1: size 相同 → Step 2
    if report["base_size"] != report["new_size"]:
        report["method"] = "size-only"
        diff = report["new_size"] - report["base_size"]
        pct = (diff / report["base_size"] * 100) if report["base_size"] > 0 else 0
        report["summary"] = f"size diff: {diff:+d} bytes ({pct:+.1f}%)"
        report["diff_blocks"] = {"uncategorized": 1}
        return report

    # Step 2: 归一化字节对比
    try:
        base_raw = base_path.read_text(encoding="utf-8", errors="replace")
        new_raw = new_path.read_text(encoding="utf-8", errors="replace")
    except Exception:
        report["method"] = "size-only"
        report["summary"] = "same size (read failed)"
        return report

    base_norm = normalize_html(base_raw)
    new_norm = normalize_html(new_raw)

    if base_norm == new_norm:
        report["method"] = "byte-level"
        report["summary"] = "IDENTICAL"
        return report

    # Step 3: 全文 diff + 自动分类
    report["method"] = "full_diff"
    base_lines = base_norm.splitlines(keepends=True)
    new_lines = new_norm.splitlines(keepends=True)

    diff = list(difflib.unified_diff(
        base_lines, new_lines,
        fromfile="base", tofile="new",
        n=0  # 0 行上下文，只显示变更行
    ))

    # 只保留变更行（+/- 开头的行）
    changed_lines = [l for l in diff if l.startswith(('+', '-', '@@')) or
                     (l.startswith(' ') and any(
                         changed_lines and changed_lines[-1].startswith(('+', '-'))
                         for _ in [1]))]

    # 合并为块
    blocks = merged_diff_blocks(diff)

    # 分类统计
    block_counts = {}
    block_details = []
    for block in blocks:
        cat = classify_diff_block(block)
        block_counts[cat] = block_counts.get(cat, 0) + 1
        # 取每个块的前 3 行作为摘要
        snippet = "\n".join(l.rstrip()[:120] for l in block[:3])
        block_details.append({
            "category": cat,
            "lines": len(block),
            "snippet": snippet[:300]
        })

    report["diff_blocks"] = block_counts
    report["details"] = block_details[:30]  # 最多保留 30 个块

    # 生成摘要
    parts = []
    priority = ["structure", "style", "script", "data", "content", "uncategorized"]
    for cat in priority:
        if cat in block_counts:
            parts.append(f"{cat}:{block_counts[cat]}")
    report["summary"] = " + ".join(parts) if parts else "diffs found"

    return report


# ============================================================
# 批量对比
# ============================================================
def find_output_files(out_dir: Path, subdir: str, ext: str) -> Dict[str, Path]:
    search_dir = out_dir / subdir
    if not search_dir.exists():
        return {}
    result = {}
    for f in search_dir.rglob(f"*.{ext}"):
        rel = f.relative_to(search_dir)
        result[str(rel).replace("\\", "/")] = f
    return result


def run_comparison(base_out: Path, new_out: Path, compare_out: Path) -> dict:
    log_header("COMPARE: 对比结果")
    compare_out.mkdir(parents=True, exist_ok=True)

    all_reports = {"json_general": [], "tshandle_parse": [], "html": []}

    for subdir in ["parse", "analysis_json"]:
        base_files = find_output_files(base_out, subdir, "json")
        new_files = find_output_files(new_out, subdir, "json")
        for key in sorted(set(base_files.keys()) | set(new_files.keys())):
            bp = base_files.get(key)
            np = new_files.get(key)
            if not bp or not np:
                all_reports["json_general"].append({
                    "file": key, "subdir": subdir,
                    "summary": "missing in base" if not bp else "missing in new"
                })
                continue
            ndiffs, diffs = compare_json_files(bp, np)
            all_reports["json_general"].append({
                "file": key, "subdir": subdir,
                "diff_count": ndiffs, "diffs": diffs[:50],
                "summary": "IDENTICAL" if ndiffs == 0 else f"{ndiffs} differences"
            })

    base_parse = find_output_files(base_out, "parse", "json")
    new_parse = find_output_files(new_out, "parse", "json")
    for key in sorted(base_parse.keys()):
        bp, np = base_parse.get(key), new_parse.get(key)
        if not bp or not np:
            continue
        base_data = load_json(bp)
        if base_data and ("tracemeta" in base_data.get("content", {}) or
                          "arkTsClasses" in base_data.get("content", {})):
            all_reports["tshandle_parse"].append(compare_tshandle_parse(bp, np))

    base_html = find_output_files(base_out, "analysis_html", "html")
    new_html = find_output_files(new_out, "analysis_html", "html")
    for key in sorted(set(base_html.keys()) | set(new_html.keys())):
        bp = base_html.get(key, Path("__missing__"))
        np = new_html.get(key, Path("__missing__"))
        r = compare_html_files(bp, np)
        r["subdir"] = "analysis_html"
        all_reports["html"].append(r)

    # rawheap compare HTML (输出在 cmp/ 目录，用短名避免 MAX_PATH)
    base_compare_html = find_output_files(base_out, "cmp", "html")
    new_compare_html = find_output_files(new_out, "cmp", "html")
    for key in sorted(set(base_compare_html.keys()) | set(new_compare_html.keys())):
        bp = base_compare_html.get(key, Path("__missing__"))
        np = new_compare_html.get(key, Path("__missing__"))
        r = compare_html_files(bp, np)
        r["subdir"] = "cmp"
        all_reports["html"].append(r)

    return all_reports


def print_comparison_report(all_reports: dict, compare_out: Path):
    log_header("COMPARISON REPORT")

    # JSON
    log(f"\n  [1] JSON General ({len(all_reports['json_general'])} files)")
    identical = 0
    total_equiv = 0
    total_real = 0
    with_diff = 0
    for r in all_reports["json_general"]:
        if r["summary"] == "IDENTICAL":
            identical += 1
        else:
            with_diff += 1
            counts = classify_diffs(r.get("diffs", []))
            total_equiv += counts["equivalent"]
            total_real += counts["real_diff"]
            log(f"\n    [{r['subdir']}] {r['file']}")
            log(f"      -> {r['summary']}  (equiv:{counts['equivalent']} real:{counts['real_diff']} "
                f"key:{counts['key_diff']} val:{counts['value_diff']})")
            for d in r.get("diffs", []):
                if "[REAL DIFF]" in d:
                    log(f"      [REAL] {d}")
            for d in r.get("diffs", []):
                if "[EQUIVALENT]" in d:
                    log(f"      [EQUIV] {d}")
                    break
            real_and_val = [d for d in r.get("diffs", [])
                           if "[REAL DIFF]" not in d and "[EQUIVALENT]" not in d]
            for d in real_and_val[:5]:
                log(f"      {d}")
            if len(real_and_val) > 5:
                log(f"      ... and {len(real_and_val) - 5} more")
    log(f"\n    JSON Summary: {identical} identical, {with_diff} with differences")
    if with_diff > 0:
        log(f"    Type changes: {total_equiv} equivalent, {total_real} real")

    # tshandle parse
    log(f"\n  [2] Tshandle Parse Deep ({len(all_reports['tshandle_parse'])} files)")
    for r in all_reports["tshandle_parse"]:
        tm_equiv = sum(1 for d in r["tracemeta_diffs"] if "[EQUIVALENT]" in d)
        tm_real = sum(1 for d in r["tracemeta_diffs"] if "[REAL DIFF]" in d)
        ad_equiv = sum(1 for d in r["addrs_diffs"] if "[EQUIVALENT]" in d)
        ad_real = sum(1 for d in r["addrs_diffs"] if "[REAL DIFF]" in d)
        o_counts = classify_diffs(r["other_diffs"])

        log(f"\n    File: {r['file']}")
        log(f"    Summary: {r['summary']}")
        log(f"    tracemeta: {len(r['tracemeta_diffs'])} (equiv:{tm_equiv} real:{tm_real})")
        for d in r["tracemeta_diffs"][:5]:
            log(f"      {d}")
        log(f"    addrs: {len(r['addrs_diffs'])} (equiv:{ad_equiv} real:{ad_real})")
        for d in r["addrs_diffs"][:5]:
            log(f"      {d}")
        log(f"    other: {len(r['other_diffs'])} (equiv:{o_counts['equivalent']} real:{o_counts['real_diff']} "
            f"key:{o_counts['key_diff']} val:{o_counts['value_diff']})")
        for d in r["other_diffs"]:
            if "[REAL DIFF]" in d:
                log(f"      [REAL] {d}")
        for d in r["other_diffs"]:
            if "[REAL DIFF]" not in d and "[EQUIVALENT]" not in d:
                log(f"      {d}")
                break
        if len(r["other_diffs"]) > 10:
            log(f"      ... and {len(r['other_diffs']) - 10} more")

    # HTML
    log(f"\n  [3] HTML ({len(all_reports['html'])} files)")
    identical = 0
    with_diff = 0
    for r in all_reports["html"]:
        method = r.get("method", "size-only")
        if r["summary"] == "IDENTICAL":
            identical += 1
            log(f"    {r['file']}: IDENTICAL ({method})")
        elif r["summary"] in ("both missing", "base missing", "new missing"):
            log(f"    {r['file']}: {r['summary']}")
        elif method == "size-only":
            log(f"    {r['file']}: {r['summary']} ({method})")
            with_diff += 1
        else:
            with_diff += 1
            blocks = r.get("diff_blocks", {})
            log(f"    {r['file']}: {r['summary']} ({method})")
            for d in r.get("details", [])[:8]:
                log(f"      [{d['category']}] {d['snippet'][:120]}")
            if len(r.get("details", [])) > 8:
                log(f"      ... and {len(r['details']) - 8} more blocks")
    log(f"\n    HTML Summary: {identical} identical, {with_diff} with differences")

    report_path = compare_out / "comparison_report.json"

    def serialize(obj):
        if isinstance(obj, Path):
            return str(obj)
        return str(obj)
    with open(report_path, "w", encoding="utf-8") as f:
        json.dump(all_reports, f, indent=2, ensure_ascii=False, default=serialize)
    log(f"\n  Detailed report saved to: {report_path}")


# ============================================================
# 主流程
# ============================================================
def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="跨 commit 对比 mem-analyzer 输出结果",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python build/compare_commits.py abc1234 def5678
  python build/compare_commits.py abc1234 def5678 --skip-build \\
      --base-exe out/base/mem-analyzer.exe --new-exe out/new/mem-analyzer.exe
  python build/compare_commits.py abc1234 def5678 --project-root /path/to/project
  python build/compare_commits.py abc1234 def5678 --modules jsleak
  python build/compare_commits.py abc1234 def5678 --modules htrace,tshandle
        """
    )
    parser.add_argument("base_commit", help="Base commit SHA")
    parser.add_argument("new_commit", help="New commit SHA")
    parser.add_argument("--project-root", default=None,
                        help="Project root directory (default: auto-detect from script location)")
    parser.add_argument("--test-dir", default=None,
                        help="Test data directory (default: <project-root>/test)")
    parser.add_argument("--output-dir", default=None,
                        help="Output directory (default: <project-root>/out)")
    parser.add_argument("--skip-build", action="store_true",
                        help="Skip build phase, use existing exes")
    parser.add_argument("--base-exe", default=None,
                        help="Path to base mem-analyzer.exe (requires --skip-build)")
    parser.add_argument("--new-exe", default=None,
                        help="Path to new mem-analyzer.exe (requires --skip-build)")
    parser.add_argument("--base-mode", choices=["serial", "parallel"], default="parallel",
                        help="Test execution mode for base version (default: parallel)")
    parser.add_argument("--new-mode", choices=["serial", "parallel"], default="parallel",
                        help="Test execution mode for new version (default: parallel)")
    parser.add_argument("--max-workers", type=int, default=4,
                        help="Max parallel workers (default: 4)")
    parser.add_argument("--modules", default="all",
                        help="要对比的模块（逗号分隔）：htrace / jsleak / tshandle / all，默认 all 全量")
    return parser.parse_args()


def main():
    # 自保护：复制到临时目录执行
    _ensure_self_contained()

    args = parse_args()

    # 解析路径
    script_dir = Path(__file__).parent.resolve()
    # 优先使用环境变量保存的原始 project root（自复制后 __file__ 指向 temp 目录）
    saved_root = os.environ.get(_ENV_PROJECT_ROOT, "")
    project_root = (Path(args.project_root) if args.project_root
                    else Path(saved_root) if saved_root
                    else script_dir.parent)
    test_dir = Path(args.test_dir) if args.test_dir else project_root / "test"
    out_dir = Path(args.output_dir) if args.output_dir else project_root / "out"
    cpp_dir = project_root / "cpp"

    base_out = out_dir / "base"
    new_out = out_dir / "new"
    compare_out = out_dir / "compare"

    log("=" * 60)
    log("  mem-analyzer cross-commit comparison")
    log(f"  Project:   {project_root}")
    log(f"  Base commit: {args.base_commit[:12]}...")
    log(f"  New commit:  {args.new_commit[:12]}...")
    log(f"  Test dir:  {test_dir}")
    log(f"  Output:    {out_dir}")
    log("=" * 60)

    # 验证必要的目录存在
    if not project_root.exists():
        log(f"ERROR: Project root not found: {project_root}")
        sys.exit(1)
    if not test_dir.exists():
        log(f"ERROR: Test directory not found: {test_dir}")
        sys.exit(1)

    log_header("STEP 0: 收集测试文件")
    test_files = collect_test_files(test_dir)
    if not any(v for k, v in test_files.items() if k != "rawheap_compare"):
        # rawheap_compare pairs alone are sufficient to proceed
        if not test_files.get("rawheap_compare"):
            log("ERROR: No test files found!")
            sys.exit(1)

    # 按 --modules 过滤要跑的类别；未选中的类别置空（保留 key，供后续 run_* 安全访问）
    modules = resolve_modules(args.modules)
    test_files = {k: (test_files[k] if k in modules else [])
                  for k in ("htrace", "snapshot", "zip")}
    # rawheap_compare 跟随 jsleak(snapshot) 模块：snapshot 选中时保留，否则置空
    if "snapshot" in modules:
        test_files["rawheap_compare"] = collect_rawheap_pairs(test_dir)
    else:
        test_files["rawheap_compare"] = []
    log(f"  Selected modules: {', '.join(modules)}")
    if test_files.get("rawheap_compare"):
        log(f"  Rawheap compare pairs: {len(test_files['rawheap_compare'])}")

    # 清理旧产物：运行结果（base/new）与对比结果（compare）。
    # 构建结果（cpp/build_base、cpp/build_new）在完整模式的 build_to_dir 内、构建前清理。
    log_header("清理旧产物")
    for d in (base_out, new_out, compare_out):
        if d.exists():
            shutil.rmtree(d)
            log(f"  Removed: {d}")

    # Phase 1: 构建
    if args.skip_build:
        log_header("PHASE 1: 跳过构建（使用已有 exe）")
        if not args.base_exe or not args.new_exe:
            log("ERROR: --skip-build requires --base-exe and --new-exe")
            sys.exit(1)
        exe_base = Path(args.base_exe)
        exe_new = Path(args.new_exe)
        if not exe_base.exists():
            log(f"ERROR: Base exe not found: {exe_base}")
            sys.exit(1)
        if not exe_new.exists():
            log(f"ERROR: New exe not found: {exe_new}")
            sys.exit(1)
    else:
        log_header("PHASE 1: 构建两个版本（git worktree）")
        exe_base = build_to_dir(args.base_commit, "build_base", "BASE",
                                project_root, cpp_dir)
        exe_new = build_to_dir(args.new_commit, "build_new", "NEW",
                               project_root, cpp_dir)

    # Phase 2: 跑测试
    log_header(f"PHASE 2: 跑测试 (BASE={args.base_mode} + NEW={args.new_mode}，同时进行)")

    def run_base():
        if args.base_mode == "parallel":
            run_tests_parallel(exe_base, base_out, test_files, "BASE", args.max_workers)
        else:
            run_tests_sequential(exe_base, base_out, test_files, "BASE")

    def run_new():
        if args.new_mode == "parallel":
            run_tests_parallel(exe_new, new_out, test_files, "NEW", args.max_workers)
        else:
            run_tests_sequential(exe_new, new_out, test_files, "NEW")

    with ThreadPoolExecutor(max_workers=2) as phase2_executor:
        f_base = phase2_executor.submit(run_base)
        f_new = phase2_executor.submit(run_new)
        for f in as_completed([f_base, f_new]):
            try:
                f.result()
            except Exception as e:
                log(f"  ERROR in test runner: {e}")

    # Phase 3: 对比
    log_header("PHASE 3: 对比")
    all_reports = run_comparison(base_out, new_out, compare_out)
    print_comparison_report(all_reports, compare_out)

    log_header("DONE")


if __name__ == "__main__":
    main()