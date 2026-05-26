#!/usr/bin/env python3
"""Validate the baseline Research OS structure for the local repo."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

REQUIRED_FILES = [
    "README.md",
    "CMakeLists.txt",
    "AUDIT-250-POINT-GOLD-STANDARD.md",
    "CHANGELOG.md",
    "docs/architecture/ARCHITECTURE.md",
    "docs/audits/INITIAL_250_POINT_GOLD_STANDARD_AUDIT.md",
    "docs/operations/DIRECT_LOCAL_GITHUB_WORKFLOW.md",
    "docs/planning/DEVELOPMENT_GAMEPLAN.md",
    "docs/process/REPO_WORKFLOW.md",
    "docs/reports/PROJECT_LOG.md",
    "docs/research/RESEARCH_PROGRAM.md",
    "docs/testing/TEST_STRATEGY.md",
    "include/blackhole_ds/units.hpp",
    "src/BlackHoleDS.cpp",
    "tests/smoke_tests.cpp",
    "tools/blackhole_ds_harness.py",
    "data/schema.sql",
]

FORBIDDEN_TRACKED_PATTERNS = [
    re.compile(r"(^|/)_incoming/"),
    re.compile(r"(^|/)build/"),
    re.compile(r"(^|/)exports/"),
    re.compile(r"(^|/)Nautilus/"),
    re.compile(r"\.(zip|7z|rar|tar|tgz|tar\.gz)$", re.IGNORECASE),
]

SECRET_NAME_PATTERN = re.compile(
    r"(^|[/\\])(\.env(\.|$)|.*(secret|secrets|credential|credentials|token|api[_-]?key|apikey|private[_-]?key|password|passwd).*)$"
    r"|(\.pem|\.pfx|\.p12|\.key|\.keystore|\.jks)$"
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


def main() -> int:
    require_files()
    validate_docs()
    validate_audit_seed()
    validate_no_forbidden_tracked_files()
    print("Research OS validation passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
