# Test Strategy

The project needs frequent A/B tests, E2E tests, audits, and scientific reviews.
The test strategy is layered so quick checks stay quick and heavy research checks
run only when appropriate.

## Test Layers

1. Static repo validation
   - `python scripts/Validate-ResearchOS.py`
   - required files, docs, forbidden tracked artifacts, secret-like names

2. C++ unit and smoke tests
   - `blackhole_ds_smoke_tests`
   - analytic limits and type-safety checks

3. CLI E2E tests
   - run `blackhole_ds`
   - assert stable output, exit code, and generated files once exporters exist

4. Data E2E tests
   - run the Python harness
   - validate SQLite schema and row counts
   - compare C++ exporter against Python reference

5. A/B numerical tests
   - compare integrator candidates on accuracy, runtime, stability, and code size
   - preserve losing candidates as regression evidence when useful

6. Audit/review gates
   - map implementation changes to audit points and truth labels
   - record decisions in `docs/reports/PROJECT_LOG.md`

## Required Local Command

```powershell
python scripts/Validate-ResearchOS.py
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## A/B Report Template

Each A/B test should record:

- candidates
- dataset or initial conditions
- metric definitions
- expected analytic behavior
- results table
- selected winner
- rollback path
