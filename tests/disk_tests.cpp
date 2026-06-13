// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// tests/disk_tests.cpp
// Validates the lensed accretion-disk ray tracer (viz/disk_image.hpp) by its
// exact geometric symmetries -- the strongest available correctness checks
// short of a published reference image:
//   - Face-on (i=0): the trace depends ONLY on impact parameter b, so the
//     image is exactly rotationally symmetric.
//   - Inclined: the geometry depends on X (the tilt axis) but not the sign of
//     Y, so the image is mirror-symmetric top<->bottom and asymmetric
//     left<->right (the lensed near/far disk).
//   - A central ray is captured (shadow); disk hits land in [r_in, r_out].

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/viz/disk_image.hpp"

using namespace blackhole_ds::viz;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool same_hit(const PixelHit& a, const PixelHit& b) {
    if (a.kind != b.kind) {
        return false;
    }
    if (a.kind == HitKind::Disk) {
        return a.order == b.order && std::abs(a.r_disk - b.r_disk) < 1e-6;
    }
    return true;
}

} // namespace

int main() {
    // --- Face-on (i = 0): result depends only on b = sqrt(X^2 + Y^2) ---
    {
        DiskView v;
        v.inclination_deg = 0.0;
        // Three points all at b = 5: (5,0), (0,5), (3,4).
        const PixelHit a = trace_disk_pixel(5.0, 0.0, v);
        const PixelHit b = trace_disk_pixel(0.0, 5.0, v);
        const PixelHit c = trace_disk_pixel(3.0, 4.0, v);
        CHECK(same_hit(a, b));
        CHECK(same_hit(a, c));
        // And a different b (=10) at two azimuths agrees too.
        const PixelHit d = trace_disk_pixel(10.0, 0.0, v);
        const PixelHit e = trace_disk_pixel(6.0, 8.0, v);
        CHECK(same_hit(d, e));
    }

    // --- Central ray is captured: the shadow ---
    {
        DiskView v;
        v.inclination_deg = 78.0;
        const PixelHit center = trace_disk_pixel(0.0, 0.0, v);
        CHECK(center.kind == HitKind::Shadow);
    }

    // --- Disk hits exist and land within [r_in, r_out] ---
    {
        DiskView v;
        v.inclination_deg = 0.0;
        int disk_hits = 0;
        for (double b = 0.5; b <= v.half_extent_M; b += 0.25) {
            const PixelHit h = trace_disk_pixel(b, 0.0, v);
            if (h.kind == HitKind::Disk) {
                ++disk_hits;
                CHECK(h.r_disk >= v.r_in_M - 1e-9 &&
                      h.r_disk <= v.r_out_M + 1e-9);
            }
        }
        CHECK(disk_hits > 0);
    }

    // --- Inclined: left<->right mirror symmetry (X sign does not matter,
    //     because the disk is tilted about the X axis) ---
    {
        DiskView v;
        v.inclination_deg = 78.0;
        const double xs[] = {2.0, 6.0, 11.0};
        const double ys[] = {-9.0, -4.0, 4.0, 9.0};
        for (double x : xs) {
            for (double y : ys) {
                const PixelHit right = trace_disk_pixel(x, y, v);
                const PixelHit left = trace_disk_pixel(-x, y, v);
                CHECK(same_hit(right, left));
            }
        }
    }

    // --- Inclined: top<->bottom asymmetry exists (the lensed near/far side)
    // ---
    {
        DiskView v;
        v.inclination_deg = 78.0;
        bool found_asymmetry = false;
        for (double y = 1.0; y <= 13.0 && !found_asymmetry; y += 0.5) {
            for (double x = 0.0; x <= 13.0; x += 0.5) {
                const PixelHit top = trace_disk_pixel(x, y, v);
                const PixelHit bot = trace_disk_pixel(x, -y, v);
                if (!same_hit(top, bot)) {
                    found_asymmetry = true;
                    break;
                }
            }
        }
        CHECK(found_asymmetry);
    }

    // --- A far-out corner pixel sees the background (no disk, no shadow) ---
    {
        DiskView v;
        v.inclination_deg = 78.0;
        const PixelHit corner = trace_disk_pixel(40.0, 40.0, v);
        CHECK(corner.kind == HitKind::Background);
    }

    if (failures == 0) {
        std::puts("disk_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "disk_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
