# Research archive audit

Audit date: 2026-11-10. Scope: every research drop and digest in this repository, on every local and remote branch, including files that were added and later deleted.

## Result

- First daily drop: **2026-06-14** (`knowledge/papers/feed/2026-06-14.md`, commit `ea17a66`).
- Last daily drop: **2026-10-24** (commit `050455d`).
- Daily digest files: **34**.
- Paper entries inside those files: **328**.
- Distinct titles: **305** of which **19** titles recur.
- Entries with an arXiv id: **119**, all of them unique (no arXiv id is repeated).
- Entries whose only link is a Consensus page: **209**. The drop did not record an arXiv id or a DOI for those, and this audit does not invent one.
- Entries with no link at all: **0**.
- Identical file hashes among the digests: **none**.
- Digest files modified after they were added: **none** (`git log --diff-filter=M` on `knowledge/papers/feed` is empty).

## Nothing before June 2026, and nothing from 2025

The claim that the daily research goes back to day one in 2025 does not match the repository.

- First commit in the repo: `bf49792`, 2026-05-25, "Initial commit". `git log --all --before=2026-01-01` is empty.
- `knowledge/papers/.gitkeep` was added in `c7b5ce4`, 2026-05-26 (Iteration 2). That is an empty directory marker, not a digest.
- The curated source-card corpus is older than the feed and is not a daily drop. `knowledge/papers/seed_sources.json` says `created: 2026-05-26` and `updated: 2026-06-12`. It was committed in `d89fc6f` (2026-06-10, Iteration 5) and last touched in `b01a6e0` (2026-06-12). It generates the 20 cards under `docs/research/source_cards/` (1828 through the 2025 GWTC-4 card). Those are foundational sources, hand-entered, not a daily intake.
- Branches inspected: `main`, `origin/main`, `origin/gh-pages`, `origin/dependabot/github_actions/actions/setup-python-7`, `origin/dependabot/pip/lxml-gte-6.1.3`. `git log --all --diff-filter=A` for `knowledge/papers/**` lists only the files on `main`. `gh-pages` has no paper, feed, or digest paths. The Dependabot branches do not add research files.
- No other folder holds a dated digest. `docs/research/` has the source cards, `RESEARCH_PROGRAM.md`, and `M5_SHADOW_ASYMMETRY.md`. `docs/audits/` and `docs/log/` are engineering records.

So: there is no 2025 daily drop to recover. The daily feed starts on 2026-06-14.

## What already existed as a summary

No chronological report of the feed existed before this audit. The related documents are:

- `knowledge/papers/feed/README.md` — what the feed is, and that it is unreviewed.
- `docs/research/source_cards/INDEX.md` — the 20 curated cards.
- `docs/research/RESEARCH_PROGRAM.md` and `docs/research/M5_SHADOW_ASYMMETRY.md` — program notes, not digests.
- The audit write-ups under `docs/audits/` — repository reviews, not paper digests.

The mini reports written from this audit are `knowledge/papers/reports/`.

## Calendar gaps

The feed was never a file per calendar day. Between the first and last drop there are 133 days and 34 files. Gaps of one or more missing days:

| After | Before | Missing days |
|---|---|---:|
| 2026-06-14 | 2026-06-26 | 11 |
| 2026-06-26 | 2026-06-29 | 2 |
| 2026-07-01 | 2026-07-03 | 1 |
| 2026-07-03 | 2026-07-06 | 2 |
| 2026-07-11 | 2026-07-13 | 1 |
| 2026-07-15 | 2026-07-26 | 10 |
| 2026-07-27 | 2026-07-29 | 1 |
| 2026-07-31 | 2026-08-02 | 1 |
| 2026-08-02 | 2026-08-04 | 1 |
| 2026-08-04 | 2026-09-27 | 53 |
| 2026-09-29 | 2026-10-14 | 14 |
| 2026-10-16 | 2026-10-19 | 2 |

The long silences are 2026-08-05 through 2026-09-26 (53 days) and 2026-09-30 through 2026-10-13 (14 days). Shorter holes of 1–11 days were left as holes. Filling every missing weekday would invent a daily cadence the repo never had.

## Duplicates

No arXiv id appears in two files. The repeats are the same title ingested again, usually from the Consensus stream. They are listed in both mini reports, with a pointer back to the earlier date.

- ×3 Entanglement islands, Page curves and phase transitions of Kerr-AdS black holes — 2026-06-26, 2026-07-08, 2026-07-26
- ×3 Testing loop quantum gravity through EHT observations of M87* and Sgr A* using rotating holonomy-corrected black holes — 2026-06-29, 2026-07-08, 2026-07-26
- ×3 Rotating Black Holes with Primary Scalar Hair: Shadow Signatures in Beyond Horndeski Gravity — 2026-06-29, 2026-07-09, 2026-08-04
- ×3 Highly eccentric non-spinning binary black hole mergers: quadrupolar post-merger waveforms — 2026-07-01, 2026-07-08, 2026-08-04
- ×2 A nonlinear voice from GW250114 ringdown — 2026-06-26, 2026-08-04
- ×2 Direct Waves in Black-Hole Binary Mergers: Insights from the Backwards One Body Model — 2026-06-26, 2026-07-08
- ×2 Fast frequency-domain phenomenological modeling of eccentric aligned-spin binary black holes (IMRPhenomXE) — 2026-06-26, 2026-07-08
- ×2 Seeing Page Curves and Islands with Blinders On — 2026-06-26, 2026-08-04
- ×2 Impact of Perfect Fluid Dark Matter on the Appearance of Rotating Black Hole — 2026-06-29, 2026-07-09
- ×2 Black hole spectroscopy: from theory to experiment — 2026-06-29, 2026-08-04
- ×2 Novel ringdown tests of general relativity with black hole greybody factors — 2026-06-29, 2026-07-09
- ×2 Microscopic Origin of Page Curve — 2026-06-29, 2026-08-04
- ×2 Holographic affinity: Weak gravity conjecture favors CFT thermodynamics in RN-AdS black holes with perfect fluid dark matter — 2026-06-30, 2026-07-26
- ×2 Photon regions, shadow observables and constraints from M87* of a Kerr-Newman-like black hole in Bumblebee gravity surrounded by plasma — 2026-07-01, 2026-07-09
- ×2 Thermodynamics and information recovery of Schwarzschild AdS black holes in conformal Killing gravity — 2026-07-01, 2026-07-08
- ×2 Probing Kalb–Ramond gravity with charged rotating black holes: constraints from EHT observations — 2026-07-03, 2026-08-04
- ×2 Measuring the Black Hole and Accretion Parameters of Sagittarius A* from EHT Observations using a Semi-Analytic Model — 2026-07-03, 2026-07-26
- ×2 Modeling the merger-ringdown of an eccentric test-mass inspiral into a Kerr black hole using the effective-one-body framework — 2026-07-06, 2026-09-29
- ×2 Advancing the effective-one-body framework in the test-mass limit (SEOB-TML) — 2026-07-07, 2026-07-09

## arXiv check

On 2026-11-10 the arXiv API was asked for all 119 ids that appear in the drops (`export.arxiv.org`, `id_list`). All 119 returned a record, and each returned title matched the title in the digest (token overlap; zero mismatches). That check does not re-review the physics. It only says the ids are real and belong to those titles.

Consensus URLs were not resolved to arXiv ids. Where a drop gives only `consensus.app/papers/details/…`, the mini report uses that URL.

## Backfill

Two silences were filled with papers retrieved from the arXiv API on **2026-11-10**. The queries were keyword searches (shadow, lensing, accretion disk, photon ring, Kerr) in `gr-qc` and, for one August query, `astro-ph.HE`, capped at a handful of hits and sorted by submission date. They are not a complete scan of those weeks, and they are not historical drops.

- `knowledge/papers/feed/BACKFILL-2026-08-05-to-2026-09-26.md`
- `knowledge/papers/feed/BACKFILL-2026-09-30-to-2026-10-13.md`

Each backfilled entry is marked **backfilled**, with the retrieval date, the arXiv id, and a summary taken from the API abstract. No 2025 material was backfilled, because there was no drop to continue and inventing one would be false.

## Feed files in order

| Date | Commit | Papers |
|---|---|---:|
| 2026-06-14 | `ea17a66 2026-06-14` | 9 |
| 2026-06-26 | `acfdcee 2026-06-26` | 9 |
| 2026-06-29 | `fdd590a 2026-06-29` | 9 |
| 2026-06-30 | `cf827ac 2026-06-30` | 9 |
| 2026-07-01 | `91bc6d8 2026-07-01` | 9 |
| 2026-07-03 | `34d32ad 2026-07-03` | 9 |
| 2026-07-06 | `f25b1ac 2026-07-06` | 8 |
| 2026-07-07 | `df506f4 2026-07-07` | 8 |
| 2026-07-08 | `5c48d8c 2026-07-08` | 9 |
| 2026-07-09 | `9b09a37 2026-07-09` | 10 |
| 2026-07-10 | `48d5013 2026-07-10` | 9 |
| 2026-07-11 | `c92aa15 2026-07-11` | 9 |
| 2026-07-13 | `5cc6bb8 2026-07-13` | 9 |
| 2026-07-14 | `0408fe8 2026-07-14` | 8 |
| 2026-07-15 | `d5b7a0b 2026-07-15` | 9 |
| 2026-07-26 | `80c0418 2026-07-26` | 6 |
| 2026-07-27 | `d8d3098 2026-07-27` | 8 |
| 2026-07-29 | `e2d2aec 2026-07-29` | 8 |
| 2026-07-30 | `f8c511d 2026-07-30` | 11 |
| 2026-07-31 | `18df1d4 2026-07-31` | 9 |
| 2026-08-02 | `0f35908 2026-08-02` | 10 |
| 2026-08-04 | `ba7e5c9 2026-08-04` | 11 |
| 2026-09-27 | `e911069 2026-09-27` | 7 |
| 2026-09-28 | `047e3e8 2026-09-28` | 11 |
| 2026-09-29 | `65f50d2 2026-09-29` | 11 |
| 2026-10-14 | `6d806e0 2026-10-14` | 10 |
| 2026-10-15 | `41fff0e 2026-10-15` | 11 |
| 2026-10-16 | `1fc53d8 2026-10-16` | 12 |
| 2026-10-19 | `4206338 2026-10-19` | 12 |
| 2026-10-20 | `4553126 2026-10-20` | 12 |
| 2026-10-21 | `df7a91c 2026-10-21` | 12 |
| 2026-10-22 | `c6422d4 2026-10-22` | 12 |
| 2026-10-23 | `a4fcec5 2026-10-23` | 12 |
| 2026-10-24 | `050455d 2026-10-24` | 10 |
