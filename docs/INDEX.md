# Documentation Index

The map of every document in this repository, grouped by purpose. Start
here if you are new. The root [README](../README.md) is the elevator pitch;
this index is the table of contents.

## Start here

| Document | What it answers |
|---|---|
| [README](../README.md) | What is this, what works, how to build and run. |
| [GLOSSARY](GLOSSARY.md) | What does this term mean (physics, numerics, project)? |
| [VISION](vision/VISION.md) | Why does this project exist and where is it going? |
| [MISSION](vision/MISSION.md) | What is the focus for the next 12 months? |
| [ROADMAP](planning/ROADMAP.md) | What is the ordered path to the visual simulation? |
| [KANBAN](process/KANBAN.md) | What is in flight right now? |

## Governance and integrity

| Document | Purpose |
|---|---|
| [Scientific Integrity Charter](vision/SCIENTIFIC_INTEGRITY_CHARTER.md) | The six truth tiers and the rules every value obeys. |
| [Decisions / ADRs](log/DECISIONS.md) | Hard-to-reverse decisions, numbered and dated. |
| [Daily Log](log/DAILY_LOG.md) | The operational story, day by day. |
| [CONTRIBUTING](../CONTRIBUTING.md) | How to work in this repo. |
| [SECURITY](../SECURITY.md) | Secret hygiene and supply-chain policy. |
| [CODEOWNERS](../CODEOWNERS) | Who owns what. |

## Architecture and data model

| Document | Purpose |
|---|---|
| [ARCHITECTURE](architecture/ARCHITECTURE.md) | Module boundaries, integrators, truth labels, build contract. |
| [HIERARCHY](architecture/HIERARCHY.md) | The live directory map and ownership semantics. |
| [ERM](architecture/ERM.md) | Conceptual entity-relationship model + data dictionary. |
| [ERD](architecture/ERD.md) | Logical entity-relationship diagram (the picture). |
| [SYSTEM_DIAGRAM](architecture/SYSTEM_DIAGRAM.md) | End-to-end pipeline diagram. |
| Diagram sources | [erd.mmd](architecture/diagrams/erd.mmd), [system_pipeline.mmd](architecture/diagrams/system_pipeline.mmd), [module_dependency.mmd](architecture/diagrams/module_dependency.mmd) |

## Engineering

| Document | Purpose |
|---|---|
| [COMPLEXITY](engineering/COMPLEXITY.md) | Time/space complexity of every algorithm, current and planned. |
| [Daily Workflow](process/DAILY_WORKFLOW.md) | The validate → build → test → commit → push loop. |
| [Repo Workflow](process/REPO_WORKFLOW.md) | Branching and repo conventions. |
| [Test Strategy](testing/TEST_STRATEGY.md) | Unit, smoke, integration, A/B, audit testing plan. |
| [Engineering Resource Plan](operations/ENGINEERING_RESOURCE_PLAN.md) | Compute/toolchain needs per milestone; GPU capacity reasoning. |
| [Direct Local GitHub Workflow](operations/DIRECT_LOCAL_GITHUB_WORKFLOW.md) | Package intake and local→GitHub sync. |

## Research

| Document | Purpose |
|---|---|
| [Research Program](research/RESEARCH_PROGRAM.md) | The staged research plan and source-card pattern. |
| [Source Cards Index](research/source_cards/INDEX.md) | The 20-source foundational corpus (1828-2025). |
| [Brain Corpus Index](../knowledge/brains/INDEX.md) | The 20 reasoning-lens scientist profiles. |

## Integrations

| Document | Purpose |
|---|---|
| [Tsotchke Ecosystem](integrations/TSOTCHKE_ECOSYSTEM.md) | The integration map and sequencing. |
| [Eshkol Integration](integrations/ESHKOL_INTEGRATION.md) | The DSL-layer rollout plan. |
| [external/README](../external/README.md) | The adapter pattern for third-party code. |

## Audits

| Document | Purpose |
|---|---|
| [Full Repo Review 2026-06-12](audits/AUDIT-2026-06-12-FULL-REPO-REVIEW.md) | The current, evidence-based 67-finding audit. |
| [500-Point Inspection](audits/INSPECTION-500-POINT.md) | The 25-section, 500-point structured inspection. |
| [Root Audit Entry](../AUDIT-250-POINT-GOLD-STANDARD.md) | Pointer into the canonical audit docs. |
| [Initial 250-Point (historical)](audits/INITIAL_250_POINT_GOLD_STANDARD_AUDIT.md) | Pre-repo planning doc; kept for history, not current. |

## Planning

| Document | Purpose |
|---|---|
| [Roadmap](planning/ROADMAP.md) | Milestones M0-M6 with exit criteria. |
| [Development Gameplan](planning/DEVELOPMENT_GAMEPLAN.md) | The phased recovery and build plan. |

## Reading paths

- **New contributor:** README → GLOSSARY → ARCHITECTURE → CONTRIBUTING →
  KANBAN.
- **Academic / researcher:** Scientific Integrity Charter → Research
  Program → Source Cards → the full audit.
- **VC / evaluator:** README → ROADMAP → the full audit → Engineering
  Resource Plan.
- **Engineer joining the kernel:** ARCHITECTURE → COMPLEXITY → ERM →
  the integrator headers and tests.
