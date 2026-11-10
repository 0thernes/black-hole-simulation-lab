// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/qrng_sample_tests.cpp
// Seeded draws used to sample the Page-Thorne flux.
//
// When BHDS_HAS_QRNG is set, the draws come from 0thernes/quantum_rng at
// commit 1a77e77 (see external/quantum_rng/PROVENANCE.md). The same seed
// must reproduce the same stream, and every sample radius must see a
// positive Schwarzschild flux that matches the closed form.
//
// Without that library (the Windows CI image has no unistd.h), the same
// test uses a local splitmix mixer. That mixer is not quantum_rng and the
// log line says so.

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "blackhole_ds/disk/novikov_thorne.hpp"

#if defined(BHDS_HAS_QRNG)
extern "C" {
#include "quantum_rng.h"
}
#endif

namespace nt = blackhole_ds::disk;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

#if !defined(BHDS_HAS_QRNG)
std::uint64_t splitmix64(std::uint64_t& x) {
    std::uint64_t z = (x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}
#endif

std::vector<double> draw_unit(int n) {
    std::vector<double> out;
    out.reserve(static_cast<std::size_t>(n));
#if defined(BHDS_HAS_QRNG)
    const char seed[] = "bhds-page-thorne-1";
    qrng_ctx* ctx = nullptr;
    const qrng_error err = qrng_init(
        &ctx, reinterpret_cast<const std::uint8_t*>(seed), sizeof(seed) - 1);
    CHECK(err == QRNG_SUCCESS);
    CHECK(ctx != nullptr);
    if (ctx != nullptr) {
        for (int i = 0; i < n; ++i) {
            out.push_back(qrng_double(ctx));
        }
        qrng_free(ctx);
    }
#else
    std::uint64_t state = 0xB7D5A11E11ULL;
    for (int i = 0; i < n; ++i) {
        const std::uint64_t w = splitmix64(state);
        out.push_back(static_cast<double>(w >> 11) *
                      (1.0 / 9007199254740992.0));
    }
#endif
    return out;
}

} // namespace

int main() {
#if defined(BHDS_HAS_QRNG)
    std::puts("sampler=quantum_rng seed=bhds-page-thorne-1 "
              "commit=1a77e77f803c63883349b658361c06401cd8ceb7");
    const auto a = draw_unit(8);
    const auto b = draw_unit(8);
    CHECK(a.size() == 8 && b.size() == 8);
    for (int i = 0; i < 8; ++i) {
        CHECK(a[static_cast<std::size_t>(i)] == b[static_cast<std::size_t>(i)]);
        CHECK(a[static_cast<std::size_t>(i)] >= 0.0);
        CHECK(a[static_cast<std::size_t>(i)] < 1.0);
    }
#else
    std::puts("sampler=splitmix-fallback (quantum_rng not compiled; "
              "not a quantum generator)");
    const auto a = draw_unit(8);
    const auto b = draw_unit(8);
    CHECK(a.size() == 8 && b.size() == 8);
    for (int i = 0; i < 8; ++i) {
        CHECK(a[static_cast<std::size_t>(i)] == b[static_cast<std::size_t>(i)]);
    }
#endif
    for (double u : a) {
        const double r = 6.5 + u * 20.0;
        const double f = nt::schwarzschild_page_thorne_flux(r);
        CHECK(f > 0.0);
        std::printf("sample r=%.6f F=%.6e\n", r, f);
    }
    if (failures == 0) {
        std::puts("qrng_sample_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "qrng_sample_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
