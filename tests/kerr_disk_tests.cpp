// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/kerr_disk_tests.cpp
// Validates the Kerr lensed-disk ray tracer (viz/kerr_disk_image.hpp).
//
// The central anchor is an a -> 0 REGRESSION: with zero spin the Kerr
// geodesic tracer must reproduce the validated Schwarzschild disk tracer
// (viz/disk_image.hpp). We check that (1) the Kerr circular-orbit redshift
// factor reduces to the Schwarzschild one, (2) the traced hit map agrees with
// the Schwarzschild tracer across a grid, and then Kerr-specific facts:
// (3) the shadow and disk both appear at high spin, and (4) spin displaces the
// shadow off-centre (frame dragging) -- the asymmetry that distinguishes a
// spinning hole.

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/viz/disk_image.hpp"
#include "blackhole_ds/viz/kerr_disk_image.hpp"

namespace viz = blackhole_ds::viz;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close_to(double a, double b, double tol) {
    return std::abs(a - b) <= tol;
}

// Map the two enums onto a common integer for comparison.
int kind_of(viz::HitKind k) {
    return static_cast<int>(k);
}
int kind_of(viz::KerrHitKind k) {
    switch (k) {
    case viz::KerrHitKind::Disk:
        return static_cast<int>(viz::HitKind::Disk);
    case viz::KerrHitKind::Shadow:
        return static_cast<int>(viz::HitKind::Shadow);
    default:
        return static_cast<int>(viz::HitKind::Background);
    }
}

} // namespace

int main() {
    // --- 1. Kerr redshift reduces to the Schwarzschild redshift at a = 0 --
    // kerr_redshift_factor(r, b, 0) == redshift_factor(r, X, sin_i) with the
    // Cunningham-Bardeen relation b = -X sin_i.
    {
        const double sin_i = 0.9;
        for (double r : {7.0, 10.0, 15.0, 25.0}) {
            for (double X : {-4.0, -1.0, 0.0, 2.5, 5.0}) {
                const double b = -X * sin_i;
                const double gk = viz::kerr_redshift_factor(r, b, 0.0);
                const double gs = viz::redshift_factor(r, X, sin_i);
                CHECK(close_to(gk, gs, 1.0e-9));
            }
        }
    }

    // --- 2. a -> 0 hit-map regression vs the Schwarzschild tracer ---------
    {
        viz::DiskView dv;
        dv.inclination_deg = 78.0; // matches defaults r_in=6, r_out=18, H=14

        viz::KerrDiskView kv;
        kv.spin = 0.0;
        kv.inclination_deg = 78.0;
        kv.r_in_M = dv.r_in_M;
        kv.r_out_M = dv.r_out_M;
        kv.half_extent_M = dv.half_extent_M;
        kv.max_crossings = dv.max_crossings;

        const double H = dv.half_extent_M;
        const int N = 20; // (N+1)^2 sample points across the image plane
        int agree = 0, total = 0, disk_pairs = 0, radius_ok = 0;
        for (int j = 0; j <= N; ++j) {
            const double Y = -H + (2.0 * H) * j / N;
            for (int i = 0; i <= N; ++i) {
                const double X = -H + (2.0 * H) * i / N;
                const viz::PixelHit s = viz::trace_disk_pixel(X, Y, dv);
                const viz::KerrPixelHit kk = viz::trace_kerr_pixel(X, Y, kv);
                ++total;
                if (kind_of(s.kind) == kind_of(kk.kind)) {
                    ++agree;
                }
                if (s.kind == viz::HitKind::Disk &&
                    kk.kind == viz::KerrHitKind::Disk) {
                    ++disk_pairs;
                    if (close_to(s.r_disk, kk.r_disk, 0.8)) {
                        ++radius_ok;
                    }
                }
            }
        }
        // Gross structural agreement (a top-bottom flip or wrong geometry
        // would drop this far below 0.9).
        CHECK(static_cast<double>(agree) / total > 0.9);
        // Where both see the disk, the radii match the Schwarzschild tracer.
        CHECK(disk_pairs > 10);
        CHECK(static_cast<double>(radius_ok) / disk_pairs > 0.85);

        // Unambiguous anchors: dead-centre is the shadow, far corner is sky.
        CHECK(viz::trace_kerr_pixel(0.0, 0.0, kv).kind ==
              viz::KerrHitKind::Shadow);
        CHECK(viz::trace_kerr_pixel(H, H, kv).kind ==
              viz::KerrHitKind::Background);
    }

    // --- 3. High spin: both shadow and disk are present -------------------
    {
        viz::KerrDiskView kv;
        kv.spin = 0.9;
        kv.inclination_deg = 78.0;
        CHECK(viz::trace_kerr_pixel(0.0, 0.0, kv).kind ==
              viz::KerrHitKind::Shadow);

        const double H = kv.half_extent_M;
        const int N = 16;
        int disk = 0, shadow = 0;
        for (int j = 0; j <= N; ++j) {
            const double Y = -H + (2.0 * H) * j / N;
            for (int i = 0; i <= N; ++i) {
                const double X = -H + (2.0 * H) * i / N;
                const auto kk = viz::trace_kerr_pixel(X, Y, kv).kind;
                if (kk == viz::KerrHitKind::Disk) {
                    ++disk;
                }
                if (kk == viz::KerrHitKind::Shadow) {
                    ++shadow;
                }
            }
        }
        CHECK(disk > 10);
        CHECK(shadow > 5);
    }

    // --- 4. Frame dragging displaces the shadow off-centre ----------------
    // The alpha-centroid of the shadow pixels is ~0 for a = 0 (symmetric) and
    // measurably displaced for a = 0.9.
    {
        const double H = 14.0;
        const int N = 40;
        auto shadow_alpha_centroid = [&](double spin) {
            viz::KerrDiskView kv;
            kv.spin = spin;
            kv.inclination_deg = 78.0;
            double sum = 0.0;
            int cnt = 0;
            for (int j = 0; j <= N; ++j) {
                const double Y = -H + (2.0 * H) * j / N;
                for (int i = 0; i <= N; ++i) {
                    const double X = -H + (2.0 * H) * i / N;
                    if (viz::trace_kerr_pixel(X, Y, kv).kind ==
                        viz::KerrHitKind::Shadow) {
                        sum += X;
                        ++cnt;
                    }
                }
            }
            return cnt > 0 ? sum / cnt : 0.0;
        };
        const double c0 = shadow_alpha_centroid(0.0);
        const double c9 = shadow_alpha_centroid(0.9);
        CHECK(std::abs(c0) < 0.6);                // symmetric without spin
        CHECK(std::abs(c9) > std::abs(c0) + 0.5); // displaced by spin
    }

    if (failures == 0) {
        std::puts("kerr_disk_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "kerr_disk_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
