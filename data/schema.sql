-- BlackHoleDS — Canonical Analytical Data Model (SQLite + Star Schema)
-- Data warehouse for simulation runs, trajectories, and chaos/graph metrics.
-- Consumable by Power BI, Excel (Power Query), pandas, and the C++ exporter.
--
-- Design goals:
-- - Append-only fact tables (runs, trajectories, chaos_stats).
-- - Full ensemble + provenance versioning (run_id, seed, git_commit, timestamp).
-- - Derived quantities (shadow diameter, ISCO, Lyapunov, ...) are stored so
--   downstream analysis is reproducible and fast.
-- - Star schema: fact tables surrounded by clean dimensions.
-- - Geometric units (G = c = 1) by default, matching the C++ kernel.
--
-- Process: this schema is hand-authored. Any change is gated by an ADR (see
-- docs/architecture/ERD.md) and must be matched in the C++ exporter,
-- tools/blackhole_ds_harness.py, and the ERM/ERD docs in the same commit.
--
-- KNOWN GAP: there is no per-row truth-tier (model_status) column yet, so
-- exact analytic values (e.g. isco_rg) and placeholder/heuristic values
-- (e.g. lyapunov_max, which the harness currently leaves NULL) share tables
-- without a row-level label. Adding that column is milestone M2 work
-- (see docs/planning/ROADMAP.md and the Scientific Integrity Charter).

PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;
PRAGMA synchronous = NORMAL;

-- ============================================================================
-- DIMENSION TABLES
-- ============================================================================

CREATE TABLE IF NOT EXISTS dim_metric (
    metric_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL UNIQUE,                    -- 'Schwarzschild', 'Kerr', 'Kerr-Newman', 'Kerr-deSitter'
    has_spin BOOLEAN NOT NULL DEFAULT 0,
    has_charge BOOLEAN NOT NULL DEFAULT 0,
    cosmological BOOLEAN NOT NULL DEFAULT 0,
    notes TEXT
);

CREATE TABLE IF NOT EXISTS dim_observer (
    observer_id INTEGER PRIMARY KEY,
    inclination_deg REAL NOT NULL,                -- 0..90 (edge-on = 90)
    distance_rg REAL NOT NULL,                    -- observer distance in r_g
    phi_offset_deg REAL NOT NULL DEFAULT 0,
    CHECK (inclination_deg BETWEEN 0 AND 90),     -- enforce documented domain
    CHECK (distance_rg > 0)
);

CREATE TABLE IF NOT EXISTS dim_ensemble (
    ensemble_id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    n_runs INTEGER NOT NULL,
    base_seed INTEGER NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now')),
    theory_version TEXT NOT NULL,                 -- git SHA or tag of the physics module used
    notes TEXT
);

-- ============================================================================
-- FACT TABLE: SIMULATION RUNS (one row per (metric, parameter set, seed))
-- ============================================================================

CREATE TABLE IF NOT EXISTS runs (
    run_id INTEGER PRIMARY KEY,
    ensemble_id INTEGER REFERENCES dim_ensemble(ensemble_id),
    metric_id INTEGER NOT NULL REFERENCES dim_metric(metric_id),
    observer_id INTEGER NOT NULL REFERENCES dim_observer(observer_id),

    -- Black hole parameters (geometric units unless noted)
    mass_Msun REAL NOT NULL,                      -- for real-world scaling
    mass_rg REAL NOT NULL,                        -- M in geometric units (usually 1.0 for normalized runs)
    spin_a_over_M REAL NOT NULL,                  -- |a/M| < 1 (0 = Schwarzschild, 0.998 = near-extremal)
    charge_q_over_M REAL NOT NULL DEFAULT 0.0,

    -- Accretion / disk parameters (for data science feature richness)
    accretion_rate_eddington REAL,                -- 0.001 .. 0.3 typical
    disk_inner_edge_rg REAL,                      -- usually ISCO or ISCO+margin
    magnetic_beta REAL,                           -- plasma beta for jet power proxies

    -- Key derived observables (computed by C++ or validated Python reference)
    photon_sphere_rg REAL NOT NULL,               -- must be ~3.0 for a=0
    isco_rg REAL NOT NULL,                        -- 6.0 for a=0, down to ~1.0 for a~1
    shadow_diameter_rg REAL NOT NULL,             -- DIAMETER ~10.39 (2*sqrt(27)) for a=0. NOTE: 5.196 is the shadow RADIUS (b_crit); EHT's "~5.2" is in Schwarzschild radii since r_s = 2M.
    hawking_temp_kelvin REAL,                     -- for quantum-corrected runs

    -- Chaos & Graph Theory features (the "why this is data science, not just pretty pictures")
    lyapunov_max REAL,                            -- largest Lyapunov exponent (Rosenstein / shadow method)
    lyapunov_spectrum TEXT,                       -- JSON array of the full spectrum
    correlation_dim REAL,                         -- correlation dimension of the attractor
    causal_graph_vertices INTEGER,                -- number of nodes in the extracted causal set
    causal_graph_centrality_max REAL,             -- max betweenness or closeness in the null geodesic graph

    -- Reproducibility & Provenance
    rng_seed INTEGER NOT NULL,
    integrator TEXT NOT NULL,                     -- 'RKF45', 'Verlet-symplectic', 'adaptive-geodesic'
    integrator_tol REAL,
    n_steps INTEGER,
    wall_time_s REAL,
    git_commit TEXT NOT NULL,
    created_at TEXT NOT NULL DEFAULT (datetime('now')),

    CHECK (spin_a_over_M >= -0.9999 AND spin_a_over_M <= 0.9999),
    CHECK (shadow_diameter_rg > 0)
);

CREATE INDEX IF NOT EXISTS idx_runs_metric_spin ON runs(metric_id, spin_a_over_M);
CREATE INDEX IF NOT EXISTS idx_runs_lyapunov ON runs(lyapunov_max) WHERE lyapunov_max IS NOT NULL;

-- ============================================================================
-- FACT TABLE: TRAJECTORIES (high-resolution time series for a single geodesic or fluid element)
-- This is the "big data" table for inductive analysis, phase-space portraits, and chaos diagnostics.
-- ============================================================================

CREATE TABLE IF NOT EXISTS trajectories (
    traj_id INTEGER PRIMARY KEY,
    run_id INTEGER NOT NULL REFERENCES runs(run_id),

    particle_type TEXT NOT NULL,                  -- 'photon', 'timelike', 'accretion-fluid', 'Hawking-quantum'
    initial_r_rg REAL NOT NULL,
    initial_theta REAL NOT NULL,
    initial_phi REAL NOT NULL,
    initial_pr REAL,
    initial_pphi REAL,

    -- Time series (stored as JSON or in a companion wide table for extreme performance)
    -- For the initial implementation we use a compact JSON blob + summary statistics
    t_series_json TEXT,                           -- array of proper time or coordinate time
    r_series_json TEXT,
    theta_series_json TEXT,
    phi_series_json TEXT,

    -- Summary statistics (always present for fast DAX / pandas queries)
    min_r_rg REAL,
    max_r_rg REAL,
    mean_r_rg REAL,
    escaped BOOLEAN,                              -- did it reach the observer rather than falling into horizon?
    n_crossings_horizon INTEGER DEFAULT 0,
    n_crossings_photon_sphere INTEGER DEFAULT 0,

    -- Chaos diagnostics per trajectory
    local_lyapunov_estimate REAL,
    poincare_section_count INTEGER,

    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);

CREATE INDEX IF NOT EXISTS idx_trajectories_run ON trajectories(run_id);

-- ============================================================================
-- FACT TABLE: CHAOS & GRAPH FEATURES (per-run aggregates for ML / causal discovery)
-- ============================================================================

CREATE TABLE IF NOT EXISTS chaos_stats (
    stat_id INTEGER PRIMARY KEY,
    run_id INTEGER NOT NULL REFERENCES runs(run_id) UNIQUE,

    lyapunov_max REAL NOT NULL,
    lyapunov_spectrum_json TEXT,
    correlation_dimension REAL,
    hurst_exponent REAL,                          -- for long-memory behavior in accretion flows
    mutual_information REAL,                      -- between spin and observed jet power (information theory)

    -- Graph Theory on the causal structure (null geodesics, light sheets)
    graph_node_count INTEGER,
    graph_edge_count INTEGER,
    graph_diameter INTEGER,
    graph_betweenness_max REAL,
    persistent_homology_betti_0 INTEGER,
    persistent_homology_betti_1 INTEGER,

    created_at TEXT NOT NULL DEFAULT (datetime('now'))
);

-- ============================================================================
-- VIEW: POWER-BI / DAX READY STAR (flattened for semantic model)
-- ============================================================================

CREATE VIEW IF NOT EXISTS v_powerbi_runs AS
SELECT
    r.run_id,
    r.mass_Msun,
    r.mass_rg,
    r.spin_a_over_M,
    r.charge_q_over_M,
    m.name AS metric,
    o.inclination_deg,
    r.accretion_rate_eddington,
    r.photon_sphere_rg,
    r.isco_rg,
    r.shadow_diameter_rg,
    r.hawking_temp_kelvin,
    r.lyapunov_max,
    r.lyapunov_spectrum,
    r.correlation_dim,
    r.causal_graph_vertices,
    r.causal_graph_centrality_max,
    r.rng_seed,
    r.git_commit,
    r.created_at
FROM runs r
JOIN dim_metric m USING (metric_id)
JOIN dim_observer o USING (observer_id);

-- ============================================================================
-- SEED DATA — Minimal but scientifically meaningful (used for first Power BI template)
-- ============================================================================

INSERT OR IGNORE INTO dim_metric (metric_id, name, has_spin, has_charge, cosmological) VALUES
(1, 'Schwarzschild', 0, 0, 0),
(2, 'Kerr', 1, 0, 0),
(3, 'Kerr-Newman', 1, 1, 0),
(4, 'Kerr-deSitter', 1, 0, 1);

INSERT OR IGNORE INTO dim_observer (observer_id, inclination_deg, distance_rg) VALUES
(1, 17.0, 10000.0),   -- typical for Sgr A* / M87* reconstructions
(2, 45.0, 1000.0),
(3, 80.0, 500.0);     -- near edge-on, strong lensing

-- A tiny curated ensemble so the data science stack is immediately usable
INSERT OR IGNORE INTO dim_ensemble (ensemble_id, name, n_runs, base_seed, theory_version) VALUES
(1, 'First-Light-Kerr-Ensemble', 27, 42, 'v0.1-units-header');

-- End of canonical schema. The C++ exporter and Python harness must stay in
-- sync with this file. Any change is gated by an ADR (docs/architecture/ERD.md).
