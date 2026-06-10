#!/usr/bin/env python3
"""Generate source-card markdown files from seed_sources.json.

Reads `knowledge/papers/seed_sources.json` and writes one Markdown file
per source under `docs/research/source_cards/<slug>.md`, plus
`knowledge/papers/INDEX.jsonl` (one source per line for cheap RAG) and
`docs/research/source_cards/INDEX.md`.
"""

from __future__ import annotations

import datetime as dt
import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[2]
SEED = ROOT / "knowledge" / "papers" / "seed_sources.json"
CARDS_DIR = ROOT / "docs" / "research" / "source_cards"
INDEX_MD = CARDS_DIR / "INDEX.md"
INDEX_JSONL = ROOT / "knowledge" / "papers" / "INDEX.jsonl"


def fail(msg: str) -> None:
    print(f"build_source_cards: {msg}", file=sys.stderr)
    raise SystemExit(1)


def format_identifier(ident: dict[str, Any] | None) -> str:
    if not ident:
        return ""
    parts: list[str] = []
    if ident.get("doi"):
        parts.append(f"- DOI: `{ident['doi']}`")
    if ident.get("arxiv_id"):
        parts.append(f"- arXiv: `{ident['arxiv_id']}`")
    if ident.get("ads_bibcode"):
        parts.append(f"- ADS: `{ident['ads_bibcode']}`")
    if ident.get("isbn"):
        parts.append(f"- ISBN: `{ident['isbn']}`")
    if ident.get("url"):
        parts.append(f"- URL: {ident['url']}")
    return "\n".join(parts)


def render_card(source: dict[str, Any], today: str) -> str:
    authors = source.get("authors", [])
    authors_str = ", ".join(authors) if authors else "Unknown"
    ident = format_identifier(source.get("identifier"))
    venue = source.get("venue", "")
    year = source.get("year", "")

    lines: list[str] = []
    lines.append(f"# Source Card: {source['title']}")
    lines.append("")
    lines.append(f"- **Slug:** `{source['slug']}`")
    lines.append(f"- **Kind:** `{source['kind']}`")
    lines.append(f"- **Authors:** {authors_str}")
    lines.append(f"- **Year:** {year}")
    if venue:
        lines.append(f"- **Venue:** {venue}")
    lines.append(f"- **Model status:** `{source['model_status']}`")
    lines.append("")
    if ident:
        lines.append("## Identifier")
        lines.append("")
        lines.append(ident)
        lines.append("")

    if source.get("summary"):
        lines.append("## Summary")
        lines.append("")
        lines.append(source["summary"])
        lines.append("")

    lines.append("## Claims")
    lines.append("")
    for c in source.get("claims", []):
        lines.append(f"- **Statement:** {c['statement']}")
        lines.append(f"  - Tier: `{c['tier']}`")
        if c.get("equation"):
            lines.append(f"  - Equation: `{c['equation']}`")
        if c.get("value"):
            val = c["value"]
            if isinstance(val, dict):
                val = ", ".join(f"{k}={v}" for k, v in val.items())
            lines.append(f"  - Value: {val}")
        if c.get("uncertainty"):
            lines.append(f"  - Uncertainty: {c['uncertainty']}")
        if c.get("where_used"):
            wu = ", ".join(f"`{w}`" for w in c["where_used"])
            lines.append(f"  - Used in: {wu}")
        lines.append("")

    if source.get("related_brains"):
        lines.append("## Related Brain Profiles")
        lines.append("")
        for b in source["related_brains"]:
            lines.append(
                f"- [`{b}`](../../../knowledge/brains/INDEX.md)"
            )
        lines.append("")

    if source.get("notes"):
        lines.append("## Notes")
        lines.append("")
        lines.append(source["notes"])
        lines.append("")

    meta = source.get("metadata") or {}
    lines.append("## Metadata")
    lines.append("")
    lines.append(f"- Card version: {meta.get('card_version', '1.0.0')}")
    lines.append(f"- Created: {meta.get('created', today)}")
    lines.append(f"- Updated: {meta.get('updated', today)}")
    lines.append(
        f"- Author: {meta.get('author', 'blackhole_ds-seed-corpus')}"
    )
    lines.append(f"- Confidence: {meta.get('confidence', 'seed')}")

    return "\n".join(lines) + "\n"


def main() -> int:
    if not SEED.exists():
        fail(f"seed file missing: {SEED}")

    seed = json.loads(SEED.read_text(encoding="utf-8"))
    sources = seed.get("sources", [])
    if not sources:
        fail("no sources in seed file")

    today = dt.date.today().isoformat()

    CARDS_DIR.mkdir(parents=True, exist_ok=True)

    rendered: list[dict[str, Any]] = []
    for src in sources:
        slug = src["slug"]
        out_path = CARDS_DIR / f"{slug}.md"
        out_path.write_text(render_card(src, today), encoding="utf-8")
        rendered.append(
            {
                "slug": slug,
                "title": src["title"],
                "authors": src["authors"],
                "year": src["year"],
                "kind": src["kind"],
                "model_status": src["model_status"],
                "path": str(out_path.relative_to(ROOT)).replace("\\", "/"),
            }
        )

    # INDEX.jsonl: one source per line for cheap RAG
    INDEX_JSONL.parent.mkdir(parents=True, exist_ok=True)
    with INDEX_JSONL.open("w", encoding="utf-8") as fh:
        for entry in sorted(rendered, key=lambda x: (x["year"], x["slug"])):
            fh.write(json.dumps(entry) + "\n")

    # INDEX.md
    lines: list[str] = []
    lines.append("# Source Card Index")
    lines.append("")
    lines.append(
        "Generated by `scripts/research/build_source_cards.py` from "
        "`knowledge/papers/seed_sources.json`."
    )
    lines.append("")
    lines.append(f"Total sources: **{len(rendered)}**")
    lines.append("")

    by_tier: dict[str, list[dict[str, Any]]] = {}
    for entry in rendered:
        by_tier.setdefault(entry["model_status"], []).append(entry)

    for tier in (
        "analytic_classical",
        "numerical_approximation",
        "observational_constraint",
        "visualization_metaphor",
        "pedagogical_simplification",
        "speculative_extension",
    ):
        in_tier = by_tier.get(tier, [])
        if not in_tier:
            continue
        lines.append(f"## Tier: {tier}")
        lines.append("")
        for entry in sorted(in_tier, key=lambda x: x["year"]):
            authors = entry["authors"][0] if entry["authors"] else "?"
            if len(entry["authors"]) > 1:
                authors += " et al."
            link = Path(entry["path"]).name
            lines.append(
                f"- [{entry['year']} - {authors} - {entry['title']}]({link})"
            )
        lines.append("")

    INDEX_MD.write_text("\n".join(lines), encoding="utf-8")

    print(f"build_source_cards: wrote {len(rendered)} cards")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
