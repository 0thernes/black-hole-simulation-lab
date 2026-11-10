# quantum_rng pin

Fork used: `0thernes/quantum_rng` (parent `tsotchke/quantum_rng`).

Commit: `1a77e77f803c63883349b658361c06401cd8ceb7`

Files, copied from that commit:

- `src/quantum_rng/quantum_rng.h`
- `src/quantum_rng/quantum_rng.c`
- `src/common/secure_memory.h`
- `LICENSE` (MIT, upstream)

Local edit, and the only one: `quantum_rng.c` includes `"secure_memory.h"`
instead of `"../common/secure_memory.h"`, so the three sources sit in this
directory. No other line was changed.

The seeded API is `qrng_init` / `qrng_double`. With a non-empty seed the
upstream header promises a stream that is a function of the seed alone.
That is what the sampler test uses. Unseeded mode draws OS entropy and is
not used.

The headers include `unistd.h` and `sys/time.h`. The MSVC CI image does not
have those, so CMake compiles this library only when `unistd.h` is present.
Otherwise the sampler test uses a logged splitmix fallback and does not call
it quantum_rng.
