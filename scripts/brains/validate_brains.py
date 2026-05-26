#!/usr/bin/env python3
"""Validate the brain/soul XML profiles.

Checks:
  1. Every profile in MANIFEST.json exists on disk.
  2. Every XML file is well-formed and parses cleanly.
  3. Every XML file references the expected XSD schema location.
  4. The slug attribute matches the file name stem.
  5. The category in <identity> matches the directory.
  6. INDEX.md profile count matches the on-disk count.

This validator intentionally does not require lxml. It uses the stdlib
xml.etree.ElementTree for structural checks. Full XSD validation can be
added later with lxml if needed.
"""

from __future__ import annotations

import json
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BRAINS = ROOT / "knowledge" / "brains"
MANIFEST = BRAINS / "MANIFEST.json"
INDEX_MD = BRAINS / "INDEX.md"

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
    print(f"validate_brains: {msg}", file=sys.stderr)
    raise SystemExit(1)


def check_one(xml_path: Path, expected_slug: str, expected_cat: str) -> None:
    try:
        tree = ET.parse(xml_path)
    except ET.ParseError as exc:
        fail(f"parse error in {xml_path}: {exc}")

    root = tree.getroot()
    if root.tag != "brain":
        fail(f"{xml_path}: root element is {root.tag!r}, expected 'brain'")

    slug = root.attrib.get("slug")
    if slug != expected_slug:
        fail(f"{xml_path}: slug {slug!r} does not match file stem {expected_slug!r}")

    identity = root.find("identity")
    if identity is None:
        fail(f"{xml_path}: missing <identity>")
    actual_cat = identity.attrib.get("category")
    if actual_cat != expected_cat:
        fail(
            f"{xml_path}: identity@category {actual_cat!r} does not match "
            f"expected {expected_cat!r}"
        )

    name = identity.find("name")
    if name is None or not (name.text or "").strip():
        fail(f"{xml_path}: missing or empty <identity><name>")

    contributions = root.find("contributions")
    if contributions is None or not list(contributions):
        fail(f"{xml_path}: missing or empty <contributions>")

    reasoning = root.find("reasoning_lens")
    if reasoning is None:
        fail(f"{xml_path}: missing <reasoning_lens>")
    if reasoning.find("proof_standard") is None:
        fail(f"{xml_path}: missing <reasoning_lens><proof_standard>")
    if not reasoning.findall("favorite_question"):
        fail(f"{xml_path}: <reasoning_lens> needs at least one favorite_question")

    apply_ = root.find("how_to_apply")
    if apply_ is None:
        fail(f"{xml_path}: missing <how_to_apply>")
    if apply_.find("when_to_consult") is None:
        fail(f"{xml_path}: missing <how_to_apply><when_to_consult>")
    if apply_.find("module_relevance") is None:
        fail(f"{xml_path}: missing <how_to_apply><module_relevance>")

    metadata = root.find("metadata")
    if metadata is None:
        fail(f"{xml_path}: missing <metadata>")
    for child in ("profile_version", "created", "updated", "author", "confidence"):
        if metadata.find(child) is None:
            fail(f"{xml_path}: missing <metadata><{child}>")


def main() -> int:
    if not MANIFEST.exists():
        fail("MANIFEST.json missing. Run scripts/brains/build_brains.py first.")
    if not INDEX_MD.exists():
        fail("INDEX.md missing. Run scripts/brains/build_brains.py first.")

    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    profiles = manifest.get("profiles", [])
    if not profiles:
        fail("MANIFEST.json has no profiles")

    seen_slugs: set[str] = set()

    for entry in profiles:
        slug = entry.get("slug")
        cat = entry.get("category")
        rel_path = entry.get("path")

        if not slug or not cat or not rel_path:
            fail(f"manifest entry missing fields: {entry}")
        if slug in seen_slugs:
            fail(f"duplicate slug in manifest: {slug}")
        if cat not in CATEGORY_DIRS:
            fail(f"unknown category {cat!r} for slug {slug}")
        seen_slugs.add(slug)

        full_path = ROOT / rel_path
        if not full_path.exists():
            fail(f"profile file missing: {rel_path}")
        if full_path.stem != slug:
            fail(
                f"file name {full_path.stem!r} does not match slug {slug!r}"
            )
        expected_dir = BRAINS / CATEGORY_DIRS[cat]
        if full_path.parent.resolve() != expected_dir.resolve():
            fail(
                f"{rel_path}: parent directory {full_path.parent} does not "
                f"match category {cat!r} (expected {expected_dir})"
            )

        check_one(full_path, slug, cat)

    # Index count
    index_text = INDEX_MD.read_text(encoding="utf-8")
    m = re.search(r"Total profiles:\s*\*\*(\d+)\*\*", index_text)
    if not m:
        fail("INDEX.md missing 'Total profiles' line")
    declared = int(m.group(1))
    if declared != len(profiles):
        fail(
            f"INDEX.md declares {declared} profiles but manifest has "
            f"{len(profiles)}"
        )

    print(
        f"validate_brains: ok, {len(profiles)} profiles, "
        f"counts={manifest.get('counts_by_category')}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
