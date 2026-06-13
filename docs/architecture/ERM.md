# Entity-Relationship Model and Data Dictionary

This document is the **conceptual** data model: the entities, their
attributes, the relationships between them, cardinalities, and the business
rules that govern them. It complements two other artifacts:

- `docs/architecture/ERD.md` + `diagrams/erd.mmd` — the **logical** diagram
  (the picture).
- `data/schema.sql` — the **physical** model (the executable SQLite DDL).

Conceptual (this file) → Logical (ERD) → Physical (schema.sql). When they
disagree, `data/schema.sql` is authoritative and the others are bugs.

---

## 1. Entities

### 1.1 Dimension entities (descriptive, slowly changing)

| Entity | Grain | Purpose |
|---|---|---|
| `dim_metric` | One spacetime family | Schwarzschild, Kerr, Kerr-Newman, Kerr-de Sitter. Declares which physical features (spin, charge, cosmological constant) the metric carries. |
| `dim_observer` | One viewpoint | Inclination, distance, azimuthal offset of the observer relative to the black hole. |
| `dim_ensemble` | One batch of runs | Groups runs sharing a base seed and a theory version (git SHA/tag), for reproducible ensemble statistics. |

### 1.2 Fact entities (measurements / computed results)

| Entity | Grain | Purpose |
|---|---|---|
| `runs` | One (metric, parameter set, seed) | The central fact: black-hole parameters and the derived observables computed for them. |
| `trajectories` | One integrated particle path | High-resolution time series for a single geodesic or fluid element (the "big data" table). |
| `chaos_stats` | One run | Per-run aggregates of chaos and graph-theoretic diagnostics. |

---

## 2. Relationships and cardinality

```text
dim_ensemble  1 ──< N  runs         (an ensemble groups many runs)
dim_metric    1 ──< N  runs         (a metric describes many runs)
dim_observer  1 ──< N  runs         (an observer views many runs)
runs          1 ──< N  trajectories (a run produces many particle paths)
runs          1 ──  1  chaos_stats  (a run has exactly one stats summary)
```

Business rules encoded by these cardinalities:

1. **Every run belongs to exactly one metric and one observer.** A run
   without a spacetime or a viewpoint is meaningless.
2. **An ensemble is optional** (`runs.ensemble_id` is nullable) so a
   one-off run need not invent an ensemble.
3. **`chaos_stats` is 1:1 with `runs`** (enforced by a `UNIQUE` on
   `chaos_stats.run_id`). A run is summarized once.
4. **`trajectories` is 1:N** and may be empty (analytic-only runs produce
   observables but no integrated paths yet).

---

## 3. Data dictionary

Types are SQLite storage classes. Units are geometric (G = c = 1, lengths
in units of GM/c^2 = "rg") unless a column name says otherwise. Every
quantity that crosses into the C++ kernel must use the strong types from
`include/blackhole_ds/units.hpp`.

### 3.1 `dim_metric`

| Column | Type | Unit / domain | Constraint | Meaning |
|---|---|---|---|---|
| `metric_id` | INTEGER | - | PK | Surrogate key. |
| `name` | TEXT | enum-like | NOT NULL, UNIQUE | 'Schwarzschild', 'Kerr', 'Kerr-Newman', 'Kerr-deSitter'. |
| `has_spin` | BOOLEAN | {0,1} | NOT NULL, default 0 | Whether the metric carries angular momentum. |
| `has_charge` | BOOLEAN | {0,1} | NOT NULL, default 0 | Whether it carries electric charge. |
| `cosmological` | BOOLEAN | {0,1} | NOT NULL, default 0 | Whether it includes a cosmological constant. |
| `notes` | TEXT | free text | - | Provenance / caveats. |

### 3.2 `dim_observer`

| Column | Type | Unit / domain | Constraint | Meaning |
|---|---|---|---|---|
| `observer_id` | INTEGER | - | PK | Surrogate key. |
| `inclination_deg` | REAL | degrees, 0..90 | NOT NULL | 0 = face-on, 90 = edge-on. |
| `distance_rg` | REAL | rg | NOT NULL | Observer distance from the black hole. |
| `phi_offset_deg` | REAL | degrees | default 0 | Azimuthal offset. |

### 3.3 `dim_ensemble`

| Column | Type | Unit / domain | Constraint | Meaning |
|---|---|---|---|---|
| `ensemble_id` | INTEGER | - | PK | Surrogate key. |
| `name` | TEXT | - | NOT NULL | Human label. |
| `n_runs` | INTEGER | count | NOT NULL | Number of runs in the batch. |
| `base_seed` | INTEGER | - | NOT NULL | RNG base seed for reproducibility. |
| `created_at` | TEXT | ISO-8601 | default now | Creation timestamp. |
| `theory_version` | TEXT | git SHA/tag | NOT NULL | Physics-module version used. |
| `notes` | TEXT | free text | - | - |

### 3.4 `runs` (central fact)

Black-hole parameters:

| Column | Type | Unit / domain | Constraint | Meaning |
|---|---|---|---|---|
| `run_id` | INTEGER | - | PK | Surrogate key. |
| `ensemble_id` | INTEGER | - | FK → dim_ensemble, nullable | Owning ensemble. |
| `metric_id` | INTEGER | - | FK → dim_metric, NOT NULL | Spacetime family. |
| `observer_id` | INTEGER | - | FK → dim_observer, NOT NULL | Viewpoint. |
| `mass_Msun` | REAL | solar masses | NOT NULL | Real-world mass scale. |
| `mass_rg` | REAL | geometric | NOT NULL | Mass in geometric units (usually 1.0). |
| `spin_a_over_M` | REAL | dimensionless | NOT NULL, CHECK in [-0.9999, 0.9999] | Kerr spin a/M. |
| `charge_q_over_M` | REAL | dimensionless | default 0 | Charge parameter. |

Accretion parameters (nullable; richer runs only):

| `accretion_rate_eddington` | REAL | Eddington units | - | Mass accretion rate. |
| `disk_inner_edge_rg` | REAL | rg | - | Usually ISCO or ISCO+margin. |
| `magnetic_beta` | REAL | dimensionless | - | Plasma beta proxy for jet power. |

Derived observables:

| `photon_sphere_rg` | REAL | rg | NOT NULL | ~3.0 for a=0. |
| `isco_rg` | REAL | rg | NOT NULL | 6.0 for a=0, → 1.0 near-extremal prograde. |
| `shadow_diameter_rg` | REAL | rg | NOT NULL, CHECK > 0 | ~10.39 for a=0 (the DIAMETER 2√27, not the 5.196 radius — see the units.hpp guard). |
| `hawking_temp_kelvin` | REAL | K | - | speculative_extension tier. |

Chaos / graph features (nullable):

| `lyapunov_max` | REAL | 1/rg | - | Largest Lyapunov exponent. |
| `lyapunov_spectrum` | TEXT | JSON array | - | Full spectrum. |
| `correlation_dim` | REAL | dimensionless | - | Attractor correlation dimension. |
| `causal_graph_vertices` | INTEGER | count | - | Null-geodesic causal-set size. |
| `causal_graph_centrality_max` | REAL | dimensionless | - | Max centrality. |

Provenance:

| `rng_seed` | INTEGER | - | NOT NULL | Per-run seed. |
| `integrator` | TEXT | enum-like | NOT NULL | 'RKF45', 'dormand-prince-54', etc. |
| `integrator_tol` | REAL | - | - | Requested tolerance. |
| `n_steps` | INTEGER | count | - | Accepted integration steps. |
| `wall_time_s` | REAL | seconds | - | Compute cost. |
| `git_commit` | TEXT | git SHA | NOT NULL | Source version. |
| `created_at` | TEXT | ISO-8601 | default now | - |

### 3.5 `trajectories`

One row per integrated particle path; carries both a compact JSON time
series and pre-computed summary statistics so DAX/pandas queries do not
have to parse JSON. Key columns: `particle_type`
('photon'|'timelike'|'accretion-fluid'|'Hawking-quantum'), initial
position/momentum, `{t,r,theta,phi}_series_json`, summary min/max/mean r,
`escaped` flag, horizon and photon-sphere crossing counts, per-trajectory
Lyapunov estimate, and Poincare-section count. Full column list in
`data/schema.sql`.

### 3.6 `chaos_stats`

Per-run aggregates: Lyapunov max + spectrum, correlation dimension, Hurst
exponent, mutual information, and graph-theoretic features (node/edge
count, diameter, max betweenness, persistent-homology Betti numbers β0/β1).

---

## 4. Open data-model items (tracked, not yet done)

These are real gaps recorded in the 2026-06-12 audit and the roadmap:

1. **Truth-tier column.** `runs` should carry a `model_status` column so
   exact analytic values, numerical approximations, and speculative
   quantities (e.g. `hawking_temp_kelvin`) are distinguishable at the row
   level, not just by column convention. Currently a per-column convention.
2. **`parameters_hash` and `build_id`.** Strengthen reproducibility beyond
   `git_commit` + `rng_seed`.
3. **Trajectory storage at scale.** The JSON-blob approach is fine for
   prototypes; a companion wide/columnar table (or Parquet export) is the
   plan for large ensembles.

---

## 5. Why a star schema

The model is a classical Kimball star: immutable fact tables surrounded by
descriptive dimensions. Rationale:

- **Analytical, not transactional.** We append runs and never update them,
  so we optimize for scan-and-aggregate, not row-level mutation.
- **Power BI / DAX readiness.** The `v_powerbi_runs` view flattens the star
  into a single semantic-model table.
- **Reproducibility by construction.** Every fact row carries its full
  provenance (seed, integrator, tolerance, git commit), so any number can
  be traced back to the exact code and inputs that produced it.
