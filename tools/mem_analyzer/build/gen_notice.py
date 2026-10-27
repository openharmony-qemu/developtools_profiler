#!/usr/bin/env python3
"""
Generate a top-level NOTICE aggregating third-party OSS component info,
following the OpenHarmony compliance standard.

Each component directory must contain a README.OpenSource (JSON array) and
the license file(s) referenced by its "License File" field. The generated
NOTICE embeds both the metadata and the full license texts.

Usage:
  python build/gen_notice.py --dest <NOTICE_PATH> <component_dir>...
  python build/gen_notice.py --dest out/NOTICE \
      third_party/src/third_party_json-master third_party/src/third_party_sqlite-master
"""

import argparse
import json
import sys
from pathlib import Path


def main() -> None:
    ap = argparse.ArgumentParser(description="Generate aggregated OSS NOTICE")
    ap.add_argument("--dest", required=True, help="output NOTICE file path")
    ap.add_argument(
        "--title",
        default="OpenHarmony Memory Analysis Tool",
        help="product title shown at the top of the NOTICE",
    )
    ap.add_argument(
        "components",
        nargs="+",
        help="component dirs each containing a README.OpenSource",
    )
    args = ap.parse_args()

    sections: list[str] = []
    idx = 0
    for comp_dir in args.components:
        comp = Path(comp_dir)
        readme = comp / "README.OpenSource"
        if not readme.is_file():
            print(f"  [WARN] {comp} has no README.OpenSource, skipped", file=sys.stderr)
            continue
        try:
            metas = json.loads(readme.read_text(encoding="utf-8"))
        except Exception as e:
            print(f"  [WARN] failed to parse {readme}: {e}", file=sys.stderr)
            continue
        if not isinstance(metas, list):
            print(f"  [WARN] {readme} is not a JSON array, skipped", file=sys.stderr)
            continue

        for m in metas:
            if not isinstance(m, dict):
                continue
            idx += 1
            lines: list[str] = []
            lines.append("----------------------------------------------------------------------------")
            lines.append(f"{idx}. {m.get('Name', '')}")
            lines.append(f"    Version:      {m.get('Version Number', '')}")
            lines.append(f"    License:      {m.get('License', '')}")
            lines.append(f"    Upstream URL: {m.get('Upstream URL', '')}")
            if m.get("Owner"):
                lines.append(f"    Owner:        {m.get('Owner', '')}")
            if m.get("Description"):
                lines.append(f"    Description:  {m.get('Description', '')}")
            lic_files = m.get("License File", "")
            if lic_files:
                for lic_name in lic_files.split(","):
                    lic_name = lic_name.strip()
                    lic_path = comp / lic_name
                    if not lic_path.is_file():
                        print(f"  [WARN] license file not found: {lic_path}", file=sys.stderr)
                        continue
                    lines.append("")
                    lines.append("    License Text:")
                    lines.append("    ------------------------------------------------------------")
                    for ln in lic_path.read_text(encoding="utf-8", errors="replace").splitlines():
                        lines.append("    " + ln)
                    lines.append("    ------------------------------------------------------------")
            lines.append("")
            sections.append("\n".join(lines))

    header = [
        args.title,
        "Copyright (c) 2026 OpenHarmony contributors",
        "",
        "This product includes the following third-party open source components.",
        "Full license texts are embedded below and in the corresponding",
        "third_party/licenses/<component>/ directory of this package.",
        "",
    ]
    body = "\n".join(header + sections)

    dest = Path(args.dest)
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text(body, encoding="utf-8")
    print(f"  [ OK] NOTICE written: {dest} ({idx} component(s))")


if __name__ == "__main__":
    main()
