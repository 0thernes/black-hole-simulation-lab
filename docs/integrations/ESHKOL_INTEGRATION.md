# Eshkol Integration Plan

[Eshkol](https://github.com/tsotchke/eshkol) is a high-performance,
LISP-like language for scientific computing and AI, written in C++. It is
the natural DSL layer for BlackHoleDS: we can write metric definitions,
conserved-quantity formulas, and validation invariants in a compact form
that compiles down to fast C++ kernels.

This is the integration plan. No code lands here yet. Adoption requires
ADR-NNNN with explicit scope, build-system impact, and rollback plan.

## Goals

1. **DSL-first metric definitions.** Express Schwarzschild, Kerr,
   Kerr-Newman, and Kerr-de Sitter in Eshkol source files that read like
   the physics paper. The transpiler produces C++ headers that the kernel
   consumes.
2. **First-class invariants.** Conserved quantities and validator
   identities live as Eshkol expressions, not as ad-hoc C++ checks.
3. **Symbolic-to-numeric pipeline.** Eshkol's LISP-like syntax makes it
   easy to author and verify the symbolic forms. The build pipeline
   converts symbolic expressions into the typed C++ structures our
   `units.hpp` layer expects.

## Non-goals

- Replacing C++ as the primary implementation language. C++20 stays the
  kernel language. Eshkol is the DSL.
- Replacing the existing `units.hpp` strong-typed quantity system. Eshkol
  emits C++ that uses `units.hpp` types.
- Adopting Eshkol for visualization, data export, or CLI. Those stay in
  C++ and Python.

## Architecture

```text
eshkol/                           Eshkol source files (DSL)
  metrics/
    schwarzschild.es
    kerr.es
    kerr_newman.es
  invariants/
    christoffel_identities.es
    bianchi.es

scripts/eshkol/
  transpile.ps1                   Wraps the eshkol compiler invocation
  validate.py                     Sanity checks on emitted C++

include/blackhole_ds/eshkol_gen/  Generated headers (gitignored)
  metrics/schwarzschild.hpp
  metrics/kerr.hpp
```

Generated headers under `include/blackhole_ds/eshkol_gen/` are **not**
committed. They are produced at build time from the `.es` sources. The
`.es` sources are committed; the build artifacts are not. The transpile
step runs before the C++ compile in CMake.

## Build pipeline

```cmake
option(BHDS_ENABLE_ESHKOL "Build Eshkol DSL transpiler step" OFF)

if(BHDS_ENABLE_ESHKOL)
    find_program(ESHKOL_BIN eshkol REQUIRED)
    file(GLOB_RECURSE ESHKOL_SRC "${PROJECT_SOURCE_DIR}/eshkol/*.es")
    set(ESHKOL_GEN_DIR "${PROJECT_BINARY_DIR}/eshkol_gen")
    foreach(SRC ${ESHKOL_SRC})
        # Transpile each .es file to a .hpp under ESHKOL_GEN_DIR.
        # Wire the generated header into the kernel target's include path.
    endforeach()
    target_include_directories(blackhole_ds PRIVATE ${ESHKOL_GEN_DIR})
endif()
```

The option is OFF by default. Builds without Eshkol installed still work
because the C++ kernel ships with hand-written reference implementations
of the same metrics. Eshkol regenerates them when enabled.

## Rollout

1. **Stage 0 (now):** This document. No code change.
2. **Stage 1 (ADR + skeleton):** Add `eshkol/` directory with a single
   `.es` file containing a stub Schwarzschild metric. Add the CMake
   option (OFF by default). Add `scripts/eshkol/transpile.ps1` that just
   echoes its arguments.
3. **Stage 2 (real transpile):** Wire `eshkol` to actually compile the
   stub and emit a C++ header that matches the hand-written
   `metrics/schwarzschild.hpp` byte-for-byte (modulo whitespace) after
   normalization.
4. **Stage 3 (parity tests):** Add CTest targets that compare the
   Eshkol-generated and hand-written metrics' analytic outputs to within
   1e-12 relative error.
5. **Stage 4 (switch primary):** Eshkol becomes the primary source for
   metric definitions; the hand-written C++ versions move to a fallback
   directory that is only built when `BHDS_ENABLE_ESHKOL` is OFF.

## Risks

- **Toolchain availability on Windows.** Eshkol's primary install path is
  Homebrew on macOS. Windows installation requires either a manual build
  or WSL. Document the install path explicitly before Stage 1.
- **License compatibility.** Eshkol's license must be reviewed against
  AGPL-3.0 (this project's license, see ADR-0006). Record under
  `docs/legal/`.
- **Maintenance velocity drift.** If Eshkol releases break our transpile
  contract, Stage 4 forces a rebuild. Pin Eshkol by commit SHA in the
  ADR.

## References

- [Eshkol on GitHub](https://github.com/tsotchke/eshkol)
- [Eshkol homepage](https://eshkol.ai)
- ADR template: `docs/log/DECISIONS.md`
- Scientific Integrity Charter: `docs/vision/SCIENTIFIC_INTEGRITY_CHARTER.md`
