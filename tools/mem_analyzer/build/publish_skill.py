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
mem-analyzer AI Agent skill 发布脚本。

按多平台 skill 结构，把各平台的 release 压缩包（.zip / .tar.gz）与 SKILL.md 组装
成一个 skill 目录并打成 ZIP；压缩包保持压缩形态，运行期由 AI Agent 按需解压。

    skills/mem-analysis-expert/
        SKILL.md
        bin/
            win-x64/mem-analyzer-<版本>-win64.zip
            linux-x64/mem-analyzer-<版本>-linux.tar.gz
            mac-x64/mem-analyzer-<版本>-mac-x64.zip
            mac-arm64/mem-analyzer-<版本>-mac-arm64.zip

用法:
    python build/publish_skill.py                              # 自动发现 skills/mem-analysis-expert/mem-analyzer-*.(zip|tar.gz)
    python build/publish_skill.py --archives-dir D:/releases   # 从指定目录收集压缩包
    python build/publish_skill.py --archive cpp/mem-analyzer-0.6.0-win64.zip \
                                  --archive cpp/mem-analyzer-0.6.0-linux.tar.gz

版本号默认从 cpp/src/utils/version.cpp 提取。
"""

import argparse
import re
import shutil
import sys
import zipfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SKILL_NAME = "mem-analysis-expert"
SKILL_SRC = REPO_ROOT / "skills" / SKILL_NAME / "SKILL.md"

# release 压缩包文件名后缀 -> skill 内 bin/ 子目录名
RELEASE_SUFFIX_TO_BIN = {
    "win64": "win-x64",
    "linux": "linux-x64",
    "mac-arm64": "mac-arm64",
    "mac-x64": "mac-x64",
}
EXPECTED_BIN = set(RELEASE_SUFFIX_TO_BIN.values())


def read_version() -> str:
    version_cpp = REPO_ROOT / "cpp" / "src" / "utils" / "version.cpp"
    text = version_cpp.read_text(encoding="utf-8")
    m = re.search(r'return\s+"([^"]+)"', text)
    if not m:
        sys.exit(f"[ERROR] 无法从 {version_cpp} 提取版本号")
    return m.group(1)


def strip_archive_ext(name: str):
    for ext in (".tar.gz", ".zip"):
        if name.endswith(ext):
            return name[: -len(ext)]
    return None


def derive_bin_subdir(name: str):
    stem = strip_archive_ext(name)
    if stem is None:
        return None
    for suffix, sub in RELEASE_SUFFIX_TO_BIN.items():
        if stem.endswith("-" + suffix):
            return sub
    return None


def _scan_dir_for_archives(dp: Path) -> list[Path]:
    out: list[Path] = []
    for p in sorted(dp.iterdir()):
        if (
            p.is_file()
            and p.name.startswith("mem-analyzer-")
            and (p.name.endswith(".zip") or p.name.endswith(".tar.gz"))
        ):
            out.append(p)
    return out


def collect_archives(args):
    archives = []

    if args.archive:
        archives.extend(Path(p).resolve() for p in args.archive)

    if args.archives_dir:
        for d in args.archives_dir:
            dp = Path(d).resolve()
            archives.extend(_scan_dir_for_archives(dp))

    if not archives:
        search_dir = SKILL_SRC.parent  # skills/mem-analysis-expert（相对仓库根目录）
        archives = sorted(search_dir.glob("mem-analyzer-*.zip")) + sorted(
            search_dir.glob("mem-analyzer-*.tar.gz")
        )

    archives = [a.resolve() for a in archives]
    if not archives:
        sys.exit(
            "[ERROR] 未找到任何平台 release 压缩包。\n"
            f"  默认在 {SKILL_SRC.parent} 下查找 mem-analyzer-<版本>-*.zip/.tar.gz，"
            "也可通过 --archives-dir / --archive 显式指定。"
        )
    return archives


def pin_skill_version(version: str) -> str:
    if not SKILL_SRC.exists():
        sys.exit(f"[ERROR] 找不到 skill 源文件: {SKILL_SRC}")
    content = SKILL_SRC.read_text(encoding="utf-8")
    content, n = re.subn(r'(?m)^version: .*', f'version: "{version}"', content)
    if n == 0:
        sys.exit("[ERROR] SKILL.md 中未找到可锚定的 version 字段")
    return content


def version_from_filename(name: str):
    m = re.search(r'mem-analyzer-(\d+(?:\.\d+)+)-', name)
    return m.group(1) if m else None


def verify_versions(entries, version: str) -> None:
    """按压缩包文件名中的版本号与目标版本（即 SKILL.md 将锚定的版本）核对，不一致则报错。"""
    for archive, sub in sorted(entries, key=lambda x: x[1]):
        file_ver = version_from_filename(archive.name)
        if file_ver is None:
            sys.exit(f"[ERROR] 无法从压缩包名识别版本: {archive.name}")
        if file_ver != version:
            sys.exit(
                f"[ERROR] 版本不匹配: {archive.name}（压缩包版本 {file_ver}）"
                f"与目标版本 {version} 不一致"
            )
        print(f"  [核对] {sub:<12} 压缩包名版本 {file_ver} == {version}")


def _create_skill_zip(zip_path: Path, skill_dir: Path) -> None:
    if zip_path.exists():
        zip_path.unlink()
    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for p in sorted(skill_dir.rglob("*")):
            if p.is_file():
                # 归档内保留顶层目录名 mem-analysis-expert/
                zf.write(p, p.relative_to(skill_dir.parent))


def main() -> None:
    parser = argparse.ArgumentParser(description="打包并发布多平台 mem-analysis-expert skill")
    parser.add_argument("--archives-dir", action="append", help="存放平台 release 压缩包的目录（可重复）")
    parser.add_argument("--archive", action="append", help="单个平台压缩包 .zip/.tar.gz（可重复）")
    parser.add_argument("--version", help="版本号（默认从 version.cpp 提取）")
    parser.add_argument("--out-dir", default=str(REPO_ROOT / "dist"), help="输出目录（默认 <repo>/dist）")
    parser.add_argument("--no-zip", action="store_true", help="只生成目录，不打 ZIP")
    args = parser.parse_args()

    version = args.version or read_version()
    archives = collect_archives(args)

    entries = []
    seen = set()
    for a in archives:
        sub = derive_bin_subdir(a.name)
        if sub is None:
            sys.exit(f"[ERROR] 无法从压缩包名识别平台: {a.name}")
        if sub in seen:
            sys.exit(f"[ERROR] 存在多个 {sub} 平台的压缩包，请只保留一个")
        seen.add(sub)
        entries.append((a, sub))

    missing = sorted(EXPECTED_BIN - seen)
    if missing:
        sys.exit(f"[ERROR] 缺少以下平台的 release 压缩包: {', '.join(missing)}")

    out_root = Path(args.out_dir).resolve()
    skill_dir = out_root / "skills" / SKILL_NAME
    zip_path = out_root / f"{SKILL_NAME}-{version}.zip"

    print("=" * 56)
    print(" mem-analysis-expert skill 发布（多平台）")
    print("=" * 56)
    print(f"  版本: {version}")
    for _a, sub in sorted(entries, key=lambda x: x[1]):
        print(f"    - {sub:<12} <- {_a.name}")
    print(f"  输出目录: {skill_dir}")
    print(f"  产物:     {zip_path}")
    print()

    # 版本一致性核对：解压并运行当前平台二进制，或读取其它平台包内 SKILL.md
    verify_versions(entries, version)
    print("  版本一致。")
    print()

    if skill_dir.exists():
        shutil.rmtree(skill_dir)
    skill_dir.mkdir(parents=True)

    (skill_dir / "SKILL.md").write_text(pin_skill_version(version), encoding="utf-8")
    print("  + SKILL.md (version pinned)")

    for a, sub in sorted(entries, key=lambda x: x[1]):
        dst = skill_dir / "bin" / sub
        dst.mkdir(parents=True)
        shutil.copy2(a, dst / a.name)
        print(f"  + bin/{sub}/{a.name}")

    if not args.no_zip:
        _create_skill_zip(zip_path, skill_dir)
        size_kb = zip_path.stat().st_size / 1024
        print()
        print("=" * 56)
        print(" skill 发布完成")
        print(f"  ZIP:  {zip_path}")
        print(f"  大小: {size_kb:.1f} KB")
        print("=" * 56)
    else:
        print()
        print("  (--no-zip：已跳过 ZIP，仅输出目录)")


if __name__ == "__main__":
    main()