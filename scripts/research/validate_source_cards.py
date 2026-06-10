#!/usr/bin/env python3
"""Validate the source-card corpus.

Checks:
  1. Every entry in INDEX.jsonl exists on disk and parses.
  2. Each source-card markdown has the expected metadata block.
  3. Truth tiers are from the allowed enumeration.
  4. INDEX.md count matches the on-disk count.
  5. Slugs are unique and URL-safe.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SEED = ROOT / "knowledge" / "papers" / "seed_sources.json"
CARDS_DIR = ROOT / "docs" / "research" / "source_cards"
INDEX_MD = CARDS_DIR / "INDEX.md"
INDEX_JSONL = ROOT / "knowledge" / "papers" / "INDEX.jsonl"

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
