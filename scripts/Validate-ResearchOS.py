#!/usr/bin/env python3
"""Validate the baseline Research OS structure for the local repo."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

REQUIRED_FILES = [
    # Root governance
    "README.md",
    "CMakeLists.txt",
    "AUDIT-250-POINT-GOLD-STANDARD.md",
    "CHANGELOG.md",
    "CONTRIBUTING.md",
    "SECURITY.md",
    "CODEOWNERS",
    "LICENSE",
    # Architecture and process docs
    "docs/architecture/ARCHITECTURE.md",
    "docs/architecture/HIERARCHY.md",
    "docs/architecture/ERD.md",
    "docs/architecture/ERM.md",
    "docs/architecture/SYSTEM_DIAGRAM.md",
    "docs/INDEX.md",
    "docs/GLOSSARY.md",
    "docs/process/KANBAN.md",
    "docs/planning/ROADMAP.md",
    "docs/operations/ENGINEERING_RESOURCE_PLAN.md",
    "docs/architecture/diagrams/erd.mmd",
    "docs/architecture/diagrams/system_pipeline.mmd",
    "docs/audits/INITIAL_250_POINT_GOLD_STANDARD_AUDIT.md",
    "docs/audits/AUDIT-2026-06-12-FULL-REPO-REVIEW.md",
    "docs/audits/INSPECTION-500-POINT.md",
    "docs/log/DAILY_LOG.md",
    "docs/log/DECISIONS.md",
    "docs/operations/DIRECT_LOCAL_GITHUB_WORKFLOW.md",
    "docs/planning/DEVELOPMENT_GAMEPLAN.md",
    "docs/process/REPO_WORKFLOW.md",
    "docs/process/DAILY_WORKFLOW.md",
    "docs/reports/PROJECT_LOG.md",
    "docs/research/RESEARCH_PROGRAM.md",
    "docs/testing/TEST_STRATEGY.md",
    # Vision layer
    "docs/vision/VISION.md",
    "docs/vision/MISSION.md",
    "docs/vision/SCIENTIFIC_INTEGRITY_CHARTER.md",
    # Integrations
    "docs/integrations/TSOTCHKE_ECOSYSTEM.md",
    "docs/integrations/ESHKOL_INTEGRATION.md",
    "external/README.md",
    # C++ kernel
    "include/blackhole_ds/units.hpp",
    "include/blackhole_ds/core/constants.hpp",
    "include/blackhole_ds/core/truth_label.hpp",
    "include/blackhole_ds/metrics/schwarzschild.hpp",
    "include/blackhole_ds/metrics/kerr.hpp",
    "include/blackhole_ds/data/csv_writer.hpp",
    "include/blackhole_ds/integrators/ode_state.hpp",
    "include/blackhole_ds/integrators/rk4.hpp",
    "include/blackhole_ds/integrators/rk45.hpp",
    "include/blackhole_ds/cli/options.hpp",
    "include/blackhole_ds/geodesics/schwarzschild_photon.hpp",
    "include/blackhole_ds/geodesics/kerr_shadow.hpp",
    "include/blackhole_ds/geodesics/kerr_geodesic.hpp",
    "include/blackhole_ds/viz/ascii_shadow.hpp",
    "include/blackhole_ds/viz/ppm_writer.hpp",
    "include/blackhole_ds/viz/shadow_image.hpp",
    "include/blackhole_ds/viz/disk_image.hpp",
    "include/blackhole_ds/viz/kerr_shadow_image.hpp",
    "include/blackhole_ds/viz/kerr_disk_image.hpp",
    "src/BlackHoleDS.cpp",
    "src/cli/main.cpp",
    "tests/smoke_tests.cpp",
    "tests/integrator_tests.cpp",
    "tests/cli_tests.cpp",
    "tests/geodesic_tests.cpp",
    "tests/shadow_tests.cpp",
    "tests/render_tests.cpp",
    "tests/disk_tests.cpp",
    "tests/kerr_shadow_tests.cpp",
    "tests/kerr_geodesic_tests.cpp",
    "tests/kerr_disk_tests.cpp",
    "docs/engineering/COMPLEXITY.md",
    "NOTICE",
    "AUTHORS",
    # Data and tooling
    "tools/blackhole_ds_harness.py",
    "data/schema.sql",
    # Knowledge corpus
    "schemas/brain_soul.xsd",
    "schemas/source_card.json",
    "knowledge/brains/seed_profiles.json",
    "knowledge/papers/seed_sources.json",
    "scripts/brains/build_brains.py",
    "scripts/brains/validate_brains.py",
    "scripts/research/build_source_cards.py",
    "scripts/research/validate_source_cards.py",
    # Dev workflow
    "scripts/dev/build.ps1",
    "scripts/dev/test.ps1",
    "scripts/dev/audit.ps1",
    "scripts/dev/format.ps1",
    "scripts/dev/Daily-Commit.ps1",
    # Tooling config
    ".clang-format",
    ".gitattributes",
    "requirements-dev.txt",
]

FORBIDDEN_TRACKED_PATTERNS = [
    re.compile(r"(^|/)_incoming/"),
    re.compile(r"(^|/)build/"),
    re.compile(r"(^|/)exports/"),
    re.compile(r"(^|/)Nautilus/"),
    re.compile(r"\.(zip|7z|rar|tar|tgz|tar\.gz)$", re.IGNORECASE),
]

SECRET_NAME_PATTERN = re.compile(
    # .env, .env.prod (dotfile forms) AND app.env, prod.env (suffix forms)
    r"(^|[/\\])\.env(\.|$)"
    r"|(^|[/\\])[^/\\]*\.env$"
    # credential-ish basenames anywhere in the name
    r"|(^|[/\\]).*(secret|secrets|credential|credentials|token|api[_-]?key|apikey|private[_-]?key|password|passwd).*$"
    # key material by extension
    r"|(\.pem|\.pfx|\.p12|\.key|\.keystore|\.jks)$"
    # SSH private keys
    r"|(^|[/\\])id_(rsa|dsa|ecdsa|ed25519)",
    re.IGNORECASE,
)


def run_git(*args: str) -> list[str]:
    result = subprocess.run(
        ["git", "-C", str(ROOT), *args],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return result.stdout.splitlines()


def fail(message: str) -> None:
    print(f"Research OS validation failed: {message}", file=sys.stderr)
    raise SystemExit(1)


def require_files() -> None:
    missing = [path for path in REQUIRED_FILES if not (ROOT / path).exists()]
    if missing:
        fail("missing required files: " + ", ".join(missing))


def validate_no_forbidden_tracked_files() -> None:
    tracked = run_git("ls-files")
    forbidden = [
        path
        for path in tracked
        if any(pattern.search(path) for pattern in FORBIDDEN_TRACKED_PATTERNS)
    ]
    if forbidden:
        fail("forbidden tracked files: " + ", ".join(forbidden))

    secret_like = [path for path in tracked if SECRET_NAME_PATTERN.search(path)]
    if secret_like:
        fail("secret-like tracked file names: " + ", ".join(secret_like))


def validate_docs() -> None:
    readme = (ROOT / "README.md").read_text(encoding="utf-8")
    readme_lower = readme.lower()
    required_phrases = [
        "zip files are intake artifacts only",
        "scientific truth labels",
        "build and test",
        "next milestones",
    ]
    for phrase in required_phrases:
        if phrase not in readme_lower:
            fail(f"README missing required phrase: {phrase}")

    architecture = (ROOT / "docs/architecture/ARCHITECTURE.md").read_text(encoding="utf-8")
    architecture_lower = architecture.lower()
    for phrase in ["truth labels", "module boundaries", "data contract"]:
        if phrase not in architecture_lower:
            fail(f"architecture doc missing required phrase: {phrase}")


def validate_audit_seed() -> None:
    audit = (ROOT / "AUDIT-250-POINT-GOLD-STANDARD.md").read_text(encoding="utf-8")
    headings = ["Section 1: Coding", "Section 2: Development", "Section 3: Engineering", "Section 4: Architecture", "Section 5: Design"]
    for heading in headings:
        if heading not in audit:
            fail(f"audit missing section heading: {heading}")


def _run_subvalidator(rel_path: str, marker: Path) -> None:
    if not marker.exists():
        return
    validator = ROOT / rel_path
    if not validator.exists():
        fail(f"{marker.name} exists but {rel_path} is missing")
    result = subprocess.run(
        [sys.executable, str(validator)],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        fail(
            f"{rel_path} failed:\n"
            + (result.stderr or result.stdout or "no output")
        )


def validate_brain_corpus() -> None:
    """Run the brain validator if the corpus exists."""
    _run_subvalidator(
        "scripts/brains/validate_brains.py",
        ROOT / "knowledge" / "brains" / "MANIFEST.json",
    )


def validate_source_card_corpus() -> None:
    """Run the source-card validator if the corpus exists."""
    _run_subvalidator(
        "scripts/research/validate_source_cards.py",
        ROOT / "knowledge" / "papers" / "INDEX.jsonl",
    )


def main() -> int:
    require_files()
    validate_docs()
    validate_audit_seed()
    validate_no_forbidden_tracked_files()
    validate_brain_corpus()
    validate_source_card_corpus()
    print("Research OS validation passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
