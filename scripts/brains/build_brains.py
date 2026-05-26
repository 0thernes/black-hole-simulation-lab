#!/usr/bin/env python3
"""Generate reasoning-lens XML profiles from seed_profiles.json.

Reads `knowledge/brains/seed_profiles.json` and writes one XML file per
profile under `knowledge/brains/<category>s/<slug>.xml`, plus the index
`knowledge/brains/INDEX.md` and `knowledge/brains/MANIFEST.json`.

The XML conforms to `schemas/brain_soul.xsd`.
"""

from __future__ import annotations

import datetime as dt
import json
import sys
from pathlib import Path
from typing import Any
from xml.sax.saxutils import escape

ROOT = Path(__file__).resolve().parents[2]
SEED = ROOT / "knowledge" / "brains" / "seed_profiles.json"
BRAINS = ROOT / "knowledge" / "brains"
INDEX_MD = BRAINS / "INDEX.md"
MANIFEST = BRAINS / "MANIFEST.json"

CATEGORY_DIRS = {
    "mathematician": "mathematicians",
    "physicist": "physicists",
    "astronomer": "astronomers",
    "coder": "coders",
    "developer": "developers",
    "engineer": "engineers",
    "architect": "architects",
    "scientist": "scientists",
}


def fail(msg: str) -> None:
    print(f"build_brains: {msg}", file=sys.stderr)
    raise SystemExit(1)


def xml_text(s: str) -> str:
    return escape(s, {'"': "&quot;"})


def render_profile(p: dict[str, Any], today: str) -> str:
    cat = p["category"]
    if cat not in CATEGORY_DIRS:
        fail(f"unknown category {cat!r} for {p.get('slug', '?')}")

    parts: list[str] = []
    parts.append('<?xml version="1.0" encoding="UTF-8"?>')
    parts.append(
        f'<brain slug="{xml_text(p["slug"])}" '
        f'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" '
        f'xsi:noNamespaceSchemaLocation="../../../schemas/brain_soul.xsd">'
    )

    # identity
    parts.append(f'  <identity category="{xml_text(cat)}">')
    parts.append(f"    <name>{xml_text(p['name'])}</name>")
    if p.get("lifespan"):
        parts.append(f"    <lifespan>{xml_text(p['lifespan'])}</lifespan>")
    if p.get("nationality"):
        parts.append(
            f"    <nationality>{xml_text(p['nationality'])}</nationality>"
        )
    parts.append(
        f"    <primary_field>{xml_text(p['primary_field'])}</primary_field>"
    )
    sec = p.get("secondary_fields") or []
    if sec:
        parts.append("    <secondary_fields>")
        for f in sec:
            parts.append(f"      <field>{xml_text(f)}</field>")
        parts.append("    </secondary_fields>")
    parts.append("  </identity>")

    # contributions
    parts.append("  <contributions>")
    for c in p.get("contributions", []):
        parts.append("    <contribution>")
        parts.append(f"      <title>{xml_text(c['title'])}</title>")
        if c.get("year"):
            parts.append(f"      <year>{xml_text(c['year'])}</year>")
        parts.append(
            f"      <description>{xml_text(c['description'])}</description>"
        )
        if c.get("relevance"):
            parts.append(
                "      <relevance_to_blackhole_ds>"
                f"{xml_text(c['relevance'])}"
                "</relevance_to_blackhole_ds>"
            )
        parts.append("    </contribution>")
    parts.append("  </contributions>")

    # reasoning_lens
    r = p.get("reasoning") or {}
    parts.append("  <reasoning_lens>")
    parts.append(
        f"    <proof_standard>"
        f"{xml_text(r.get('proof_standard', 'unspecified'))}"
        "</proof_standard>"
    )
    for q in r.get("questions", []):
        parts.append(
            f"    <favorite_question>{xml_text(q)}</favorite_question>"
        )
    for fail_mode in r.get("failures", []):
        parts.append(
            f"    <failure_mode>{xml_text(fail_mode)}</failure_mode>"
        )
    parts.append("  </reasoning_lens>")

    # bibliography
    bib = p.get("bibliography") or []
    if bib:
        parts.append("  <bibliography>")
        for ref in bib:
            parts.append("    <reference>")
            parts.append(
                f"      <citation>{xml_text(ref['citation'])}</citation>"
            )
            if ref.get("url"):
                parts.append(f"      <url>{xml_text(ref['url'])}</url>")
            if ref.get("model_status"):
                parts.append(
                    f"      <model_status>{xml_text(ref['model_status'])}"
                    "</model_status>"
                )
            parts.append("    </reference>")
        parts.append("  </bibliography>")

    # how_to_apply
    apply_ = p.get("apply") or {}
    parts.append("  <how_to_apply>")
    parts.append(
        f"    <when_to_consult>{xml_text(apply_.get('when', 'unspecified'))}"
        "</when_to_consult>"
    )
    parts.append("    <module_relevance>")
    for m in apply_.get("modules", []):
        parts.append(f"      <module>{xml_text(m)}</module>")
    parts.append("    </module_relevance>")
    if apply_.get("cautions"):
        parts.append(
            f"    <cautions>{xml_text(apply_['cautions'])}</cautions>"
        )
    parts.append("  </how_to_apply>")

    # metadata
    parts.append("  <metadata>")
    parts.append("    <profile_version>1.0.0</profile_version>")
    parts.append(f"    <created>{today}</created>")
    parts.append(f"    <updated>{today}</updated>")
    parts.append("    <author>blackhole_ds-seed-roster</author>")
    parts.append("    <confidence>seed</confidence>")
    parts.append("  </metadata>")

    parts.append("</brain>")
    return "\n".join(parts) + "\n"


def main() -> int:
    if not SEED.exists():
        fail(f"seed file missing: {SEED}")

    seed = json.loads(SEED.read_text(encoding="utf-8"))
    profiles = seed.get("profiles", [])
    if not profiles:
        fail("no profiles in seed file")

    today = dt.date.today().isoformat()

    counts: dict[str, int] = {}
    manifest_entries: list[dict[str, str]] = []

    for p in profiles:
        cat = p["category"]
        dir_name = CATEGORY_DIRS.get(cat)
        if dir_name is None:
            fail(f"unknown category {cat!r} for {p.get('slug', '?')}")

        out_dir = BRAINS / dir_name
        out_dir.mkdir(parents=True, exist_ok=True)
        out_path = out_dir / f"{p['slug']}.xml"
        out_path.write_text(render_profile(p, today), encoding="utf-8")

        counts[cat] = counts.get(cat, 0) + 1
        manifest_entries.append(
            {
                "slug": p["slug"],
                "name": p["name"],
                "category": cat,
                "path": str(out_path.relative_to(ROOT)).replace("\\", "/"),
            }
        )

    # MANIFEST.json
    manifest = {
        "schema_version": seed["schema_version"],
        "generated": today,
        "total": len(manifest_entries),
        "counts_by_category": counts,
        "profiles": sorted(manifest_entries, key=lambda x: x["slug"]),
    }
    MANIFEST.write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )

    # INDEX.md
    lines: list[str] = []
    lines.append("# Brain/Soul Index")
    lines.append("")
    lines.append(
        "Reasoning-lens XML profiles. Generated by "
        "`scripts/brains/build_brains.py` from "
        "`knowledge/brains/seed_profiles.json`."
    )
    lines.append("")
    lines.append(
        "Profiles are structured prompts that help us reason in the style "
        "of a known thinker. They are not claims about a person's mind. "
        "See `docs/vision/SCIENTIFIC_INTEGRITY_CHARTER.md` section VI."
    )
    lines.append("")
    lines.append(f"Total profiles: **{len(manifest_entries)}**")
    lines.append("")
    lines.append("Counts by category:")
    lines.append("")
    for cat in sorted(counts):
        lines.append(f"- {cat}: {counts[cat]}")
    lines.append("")

    for cat in sorted(CATEGORY_DIRS):
        in_cat = [e for e in manifest_entries if e["category"] == cat]
        if not in_cat:
            continue
        lines.append(f"## {cat.title()}s")
        lines.append("")
        for e in sorted(in_cat, key=lambda x: x["name"]):
            # Make path relative to INDEX.md (which lives in BRAINS).
            link = (
                Path(e["path"])
                .relative_to(BRAINS.relative_to(ROOT).as_posix())
                .as_posix()
            )
            lines.append(f"- [{e['name']}]({link})")
        lines.append("")

    INDEX_MD.write_text("\n".join(lines), encoding="utf-8")

    print(
        f"build_brains: wrote {len(manifest_entries)} profiles, "
        f"counts={counts}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
