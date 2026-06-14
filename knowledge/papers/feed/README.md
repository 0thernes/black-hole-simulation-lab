# Research Feed — bleeding-edge intake

This directory holds **dated digests of recent research** (black holes, general
relativity, numerical relativity, quantum gravity) pulled from arXiv / Semantic
Scholar and adjacent sources. It is the *live, unreviewed* intake stream.

## How this differs from the curated corpus

`docs/research/source_cards/` is the **curated, schema-validated** corpus of
*foundational* sources (1827 → present), each hand-reviewed and traced to a
specific implemented formula. The validator (`scripts/research/validate_source_cards.py`)
enforces its integrity.

This feed is the opposite end: **fast, broad, and unreviewed.** Entries are
recent preprints and papers, captured for awareness, not yet vetted or wired to
code. Truth tier defaults to `preprint` / `speculative_extension` until a human
promotes an item into the curated corpus.

**Promotion path:** when a feed item proves directly relevant and is vetted, it
graduates to a full source card under `docs/research/source_cards/` (with the
schema metadata + `where_used` links). The feed itself is never a build input.

## Format

One markdown file per pull, named `YYYY-MM-DD.md`, grouped by domain, each entry
with title, authors, year, venue, a one-line summary, a truth-tier label, and a
link. See any dated digest for the shape.

## Honesty note

Most feed items are **not peer-reviewed** (arXiv preprints) and some are
deliberately fringe-but-credible. They are labeled accordingly and must never be
presented as settled physics or allowed to influence the classical-GR kernel
without explicit review. This respects the Scientific Integrity Charter.
