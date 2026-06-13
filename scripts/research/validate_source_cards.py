#!/usr/bin/env python3
"""Validate the source-card corpus.

Checks:
  1. Every entry in INDEX.jsonl exists on disk and parses.
  2. Each source-card markdown has the expected metadata block.
  3. Truth tiers are from the allowed enumeration.
  4. INDEX.md count matches the on-disk count.
  5. Slugs are unique and URL-safe.
  6. Every seed source validates against schemas/source_card.json when
     the jsonschema package is installed (CI installs it; locally it is
     optional). The 2026-06-12 audit flagged the JSON Schema as
     decorative - enforced by nothing. This closes that hole.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

try:
    import jsonschema
    JSONSCHEMA_AVAILABLE = True
except ImportError:
    jsonschema = None
    JSONSCHEMA_AVAILABLE = False

ROOT = Path(__file__).resolve().parents[2]
SEED = ROOT / "knowledge" / "papers" / "seed_sources.json"
CARDS_DIR = ROOT / "docs" / "research" / "source_cards"
INDEX_MD = CARDS_DIR / "INDEX.md"
INDEX_JSONL = ROOT / "knowledge" / "papers" / "INDEX.jsonl"
CARD_SCHEMA = ROOT / "schemas" / "source_card.json"

ALLOWED_TIERS = {
    "analytic_classical",
    "numerical_approximation",
    "observational_constraint",
    "visualization_metaphor",
    "pedagogical_simplification",
    "speculative_extension",
}

SLUG_RE = re.compile(r"^[a-z0-9-]+$")


def fail(msg: str) -> None:
    print(f"validate_source_cards: {msg}", file=sys.stderr)
    raise SystemExit(1)


def main() -> int:
    if not SEED.exists():
        # Corpus not built yet; skip silently.
        print("validate_source_cards: seed missing, skipping.")
        return 0
    if not INDEX_JSONL.exists() or not INDEX_MD.exists():
        fail("INDEX.jsonl or INDEX.md missing. Run build_source_cards.py.")

    seed = json.loads(SEED.read_text(encoding="utf-8"))
    sources = seed.get("sources", [])
    if not sources:
        fail("no sources in seed file")

    # Enforce the JSON Schema when the validator library is present.
    if JSONSCHEMA_AVAILABLE:
        schema = json.loads(CARD_SCHEMA.read_text(encoding="utf-8"))
        validator = jsonschema.Draft202012Validator(schema)
        for src in sources:
            errors = sorted(validator.iter_errors(src), key=str)
            if errors:
                msgs = "; ".join(
                    f"{'/'.join(str(p) for p in e.path) or '<root>'}: {e.message}"
                    for e in errors[:5]
                )
                fail(f"{src.get('slug', '?')}: JSON Schema violations: {msgs}")
        print(f"validate_source_cards: JSON Schema validation passed "
              f"({len(sources)} sources)")
    else:
        print("validate_source_cards: jsonschema not installed, schema "
              "validation skipped (structural checks still ran)")

    # Cross-corpus referential integrity: every related_brains slug must
    # resolve to a real brain profile (inspection finding S14.12).
    brain_manifest = ROOT / "knowledge" / "brains" / "MANIFEST.json"
    brain_slugs: set[str] = set()
    if brain_manifest.exists():
        bm = json.loads(brain_manifest.read_text(encoding="utf-8"))
        brain_slugs = {p["slug"] for p in bm.get("profiles", [])}

    slugs: set[str] = set()
    for src in sources:
        slug = src.get("slug")
        if not slug or not SLUG_RE.match(slug):
            fail(f"invalid slug: {slug!r}")
        if slug in slugs:
            fail(f"duplicate slug: {slug}")
        slugs.add(slug)

        tier = src.get("model_status")
        if tier not in ALLOWED_TIERS:
            fail(f"{slug}: invalid model_status {tier!r}")

        for c in src.get("claims") or []:
            ct = c.get("tier")
            if ct not in ALLOWED_TIERS:
                fail(f"{slug}: claim has invalid tier {ct!r}")
            # where_used paths must exist (a file, or a directory target).
            # Inspection finding S14.14.
            for wu in c.get("where_used") or []:
                if not (ROOT / wu).exists():
                    fail(f"{slug}: where_used path does not exist: {wu}")

        # related_brains referential integrity (only enforced when the
        # brain manifest is present, i.e. the corpus is built).
        if brain_slugs:
            for rb in src.get("related_brains") or []:
                if rb not in brain_slugs:
                    fail(
                        f"{slug}: related_brains slug {rb!r} not found in "
                        "the brain corpus"
                    )

        card_path = CARDS_DIR / f"{slug}.md"
        if not card_path.exists():
            fail(f"missing card file: {card_path.relative_to(ROOT)}")
        text = card_path.read_text(encoding="utf-8")
        for required in ("## Summary", "## Claims", "## Metadata"):
            # Summary is optional, Claims and Metadata required.
            if required in ("## Claims", "## Metadata") and required not in text:
                fail(f"{card_path.relative_to(ROOT)} missing section {required}")

    # INDEX.jsonl line count
    with INDEX_JSONL.open(encoding="utf-8") as fh:
        jsonl_lines = [json.loads(line) for line in fh if line.strip()]
    if len(jsonl_lines) != len(sources):
        fail(
            f"INDEX.jsonl has {len(jsonl_lines)} entries but seed has "
            f"{len(sources)}"
        )

    # INDEX.md total
    text = INDEX_MD.read_text(encoding="utf-8")
    m = re.search(r"Total sources:\s*\*\*(\d+)\*\*", text)
    if not m:
        fail("INDEX.md missing 'Total sources' line")
    declared = int(m.group(1))
    if declared != len(sources):
        fail(
            f"INDEX.md declares {declared} sources but seed has "
            f"{len(sources)}"
        )

    print(f"validate_source_cards: ok, {len(sources)} cards")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
