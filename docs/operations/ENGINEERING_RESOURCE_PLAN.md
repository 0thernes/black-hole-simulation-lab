# Engineering Resource Plan

This is the project's "ERP": a plain-language plan of the **compute and
toolchain resources** each roadmap milestone needs, who can run what, and
where the hardware ceilings are. It exists because the headline goal is a
GPU render — the project is explicitly *for people with serious GPU power* —
so resource planning is a first-class engineering concern, not an
afterthought.

## Reference development hardware

The author's machine, and the reference target for "runs on a laptop":

| Component | Spec |
|---|---|
| Machine | MSI Vector HX AI A2XW |
| OS | Windows 11 Pro |
| CPU | Intel Core Ultra 9 275HX |
| RAM | 80 GB DDR5 5600 MHz |
| GPU | NVIDIA RTX 5070 Ti 12 GB GDDR7 (~8900 CUDA cores) |
| Storage | Gen4 NVMe |

## Toolchain baseline

| Tool | Minimum | Notes |
|---|---|---|
| C++ compiler | C++20 (MSVC 19.3x or GCC 12+) | CI uses MSVC (`windows-latest`); local dev uses MinGW GCC 14.2. The code is portable across both — verified by CI being the MSVC canary. |
| CMake | ≥ 3.20 | Build system. |
| Python | 3.11 | Tooling, validators, harness. `lxml` + `jsonschema` for full validation. |
| CUDA Toolkit | 12.x (milestone M4+) | Only needed once the GPU ray marcher lands; not required to build the current kernel. |

## Resource tiers by milestone

The current and near-term work is CPU-only and runs on anything. GPU
requirements arrive at M4.

| Milestone | Compute need | Minimum to participate | Notes |
|---|---|---|---|
| M0 analytic core (done) | Trivial | Any C++20 toolchain | Sub-second builds, microsecond runs. |
| M1 geodesic integrator | Light CPU | Any laptop | Single-ray integration is microseconds; ray bundles are seconds. |
| M2 data contract | Light CPU + SQLite | Any laptop | I/O bound, not compute bound. |
| M3 CPU lensing | Moderate CPU; embarrassingly parallel | Multi-core CPU | A full-frame render is `O(P · M_avg)`; minutes on CPU at moderate resolution. Parallelize over pixels with threads. |
| M4 GPU ray marcher | **GPU required** | CUDA-capable NVIDIA GPU, ≥ 8 GB VRAM | One thread per pixel. The RTX 5070 Ti's core count is the constant-factor win; 12 GB VRAM bounds resolution × samples. |
| M5 Kerr + accretion | GPU | Same as M4 | More work per ray (Carter constant, disk physics); same parallel structure. |
| M6 ecosystem / GRMHD | Heavy; HPC-adjacent | Workstation/cluster for full GRMHD | Comparison runs against the Einstein Toolkit/BHAC may need more than a laptop; the OPPENGROK renderer itself stays laptop-GPU-scoped. |

## Capacity reasoning for the GPU goal

The render is **embarrassingly parallel over pixels**: each pixel's ray is
an independent ODE integration with no cross-pixel dependency. This is why
a GPU is the right tool and why the goal is realistic on a single
consumer card:

- Per-ray work: `O(M)` adaptive steps, `M` small except for the
  measure-zero ray set that asymptotes to the photon sphere.
- Total CPU work for `P` pixels: `O(P · M_avg)`, serial.
- GPU wall-clock: `O(P · M_avg / cores)`. With ~8900 CUDA cores the
  constant-factor speedup over a single CPU core is large; the practical
  ceiling is VRAM (12 GB) bounding resolution × supersampling, and memory
  bandwidth bounding throughput once cores are saturated.

VRAM budget sketch (M4): a 1080p frame at 4 bytes/channel × 4 channels is
~33 MB per buffer — negligible against 12 GB. The VRAM ceiling matters for
high supersampling, deep ray history capture, or volumetric disk models;
those are tunable, so the prototype fits comfortably.

## Who can use this repo

- **Build and run the analytic core / integrators / data tooling:** anyone
  with a C++20 toolchain and Python 3.11. No GPU.
- **Run the eventual visual prototype (M4+):** anyone with a CUDA-capable
  NVIDIA GPU and ≥ 8 GB VRAM. The RTX 5070 Ti is the reference; lower cards
  work at reduced resolution.
- **Run full GRMHD comparisons (M6, optional):** workstation or cluster.

The project is deliberately staged so that the science and the data
tooling never require a GPU; only the final render does. That keeps the
repo useful to researchers and students on modest hardware while rewarding
those with "the glorious GPU power" with the visual payoff.

## Cost and time budgets (rough, revisited per milestone)

- Build: seconds (header-only kernel + small TUs).
- Test suite: well under a second (three CTest suites).
- Corpus regeneration + validation: seconds.
- CI run end-to-end: ~40 seconds on `windows-latest` (measured).
- M3 CPU render: target minutes; M4 GPU render: target sub-second to
  seconds per frame at 1080p (to be measured when it lands).
