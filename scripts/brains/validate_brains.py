#!/usr/bin/env python3
"""Validate the brain/soul XML profiles.

Checks:
  1. Every profile in MANIFEST.json exists on disk.
  2. Every XML file is well-formed and parses cleanly.
  3. The slug attribute matches the file name stem.
  4. The category in <identity> matches the directory.
  5. Contribution <year> values match the schema's YearOrPeriod pattern
     (mirrored here so the gate works without lxml).
  6. No orphan XML files on disk that are missing from the manifest.
  7. INDEX.md profile count matches the on-disk count.
  8. Full XSD validation against schemas/brain_soul.xsd when lxml is
     installed (CI installs it; locally it is optional).

The 2026-06-12 audit flagged the previous version as a gate that does not
gate: it never checked XSD type validity, so xs:gYear violations shipped
silently. The pattern mirror plus opportunistic lxml validation closes
that hole.
"""

from __future__ import annotations

import json
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

try:
    from lxml import etree as lxml_etree
    LXML_AVAILABLE = True
except ImportError:
    lxml_etree = None
    LXML_AVAILABLE = False

ROOT = Path(__file__).resolve().parents[2]
BRAINS = ROOT / "knowledge" / "brains"
MANIFEST = BRAINS / "MANIFEST.json"
INDEX_MD = BRAINS / "INDEX.md"
XSD_PATH = ROOT / "schemas" / "brain_soul.xsd"

# Mirrors the YearOrPeriod simpleType in schemas/brain_soul.xsd. Keep the
# two in sync; the XSD is authoritative.
YEAR_PATTERN = re.compile(r"^[0-9]{4}s?(-([0-9]{4}s?)?)?$")

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
    for contribution in contributions.findall("contribution"):
        year = contribution.find("year")
        if year is not None:
            year_text = (year.text or "").strip()
            if not YEAR_PATTERN.match(year_text):
                fail(
                    f"{xml_path}: contribution year {year_text!r} does not "
                    "match the schema YearOrPeriod pattern"
                )

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

    # Orphan scan: every XML on disk must be in the manifest.
    manifest_paths = {str((ROOT / e["path"]).resolve()) for e in profiles}
    for cat_dir in CATEGORY_DIRS.values():
        for xml_file in sorted((BRAINS / cat_dir).glob("*.xml")):
            if str(xml_file.resolve()) not in manifest_paths:
                fail(
                    f"orphan profile not in manifest: "
                    f"{xml_file.relative_to(ROOT)}"
                )

    # Full XSD validation when lxml is available (CI always; local optional).
    if LXML_AVAILABLE:
        schema = lxml_etree.XMLSchema(lxml_etree.parse(str(XSD_PATH)))
        for entry in profiles:
            doc = lxml_etree.parse(str(ROOT / entry["path"]))
            if not schema.validate(doc):
                errors = "; ".join(str(e) for e in schema.error_log)
                fail(f"XSD validation failed for {entry['path']}: {errors}")
        print(f"validate_brains: XSD validation passed ({len(profiles)} files)")
    else:
        print("validate_brains: lxml not installed, XSD validation skipped "
              "(pattern mirror checks still ran)")

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
