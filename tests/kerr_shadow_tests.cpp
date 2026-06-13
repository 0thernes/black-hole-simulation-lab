// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/kerr_shadow_tests.cpp
// Validates the closed-form Kerr shadow (geodesics/kerr_shadow.hpp) against
// known analytic results:
//   - a -> 0 reduces to the Schwarzschild circle of radius sqrt(27) M.
//   - the equatorial photon-orbit radii bracket 3M and match the a=1 limits.
//   - spin makes the shadow asymmetric in alpha (the spin signature), while
//     it stays symmetric in beta.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "blackhole_ds/geodesics/kerr_shadow.hpp"
#include "blackhole_ds/viz/kerr_shadow_image.hpp"

namespace kerr = blackhole_ds::geodesics::kerr;
using blackhole_ds::viz::shadow_alpha_extent;

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

} // namespace

int main() {
    // --- Photon-orbit radii: a=0 gives 3M both ways; a=1 gives 1M and 4M ---
    {
        CHECK(close_to(kerr::photon_orbit_prograde(0.0), 3.0, 1e-9));
        CHECK(close_to(kerr::photon_orbit_retrograde(0.0), 3.0, 1e-9));
        CHECK(close_to(kerr::photon_orbit_prograde(1.0), 1.0, 1e-9));
        CHECK(close_to(kerr::photon_orbit_retrograde(1.0), 4.0, 1e-9));
        // Prograde orbit shrinks with spin, retrograde grows.
        CHECK(kerr::photon_orbit_prograde(0.6) < 3.0);
        CHECK(kerr::photon_orbit_retrograde(0.6) > 3.0);
    }

    // --- a -> 0: shadow is the Schwarzschild circle, radius sqrt(27) M ---
    {
        const auto poly = kerr::shadow_boundary(0.0, 60.0);
        const double R = std::sqrt(27.0);
        CHECK(poly.size() > 100);
        for (const auto& p : poly) {
            CHECK(close_to(std::hypot(p.alpha, p.beta), R, 1e-6));
        }
        const auto [amin, amax] = shadow_alpha_extent(poly);
        CHECK(close_to(amin, -R, 1e-6));
        CHECK(close_to(amax, R, 1e-6));
        CHECK(close_to(amin + amax, 0.0, 1e-6)); // symmetric about 0
    }

    // --- Near-extremal spin, edge-on: shadow is asymmetric in alpha but
    //     symmetric in beta (the D-shape) ---
    {
        const auto poly = kerr::shadow_boundary(0.9, 90.0);
        CHECK(poly.size() > 100);
        const auto [amin, amax] = shadow_alpha_extent(poly);
        // Asymmetric: the centre of the alpha-extent is displaced from 0.
        CHECK(std::abs(amin + amax) > 0.5);
        // beta extent stays symmetric (reflection symmetry about the
        // equatorial plane).
        double bmin = 1e300, bmax = -1e300;
        for (const auto& p : poly) {
            bmin = std::min(bmin, p.beta);
            bmax = std::max(bmax, p.beta);
        }
        CHECK(close_to(bmin + bmax, 0.0, 1e-6));
    }

    // --- Spin shrinks/shifts the shadow vs Schwarzschild; point-in-shadow
    //     behaves (centre inside, far outside) ---
    {
        const auto poly = kerr::shadow_boundary(0.7, 80.0);
        // A point at the origin region should be inside; a far point outside.
        // (Kerr shadow is displaced, so test near its own centroid.)
        double cx = 0.0, cy = 0.0;
        for (const auto& p : poly) {
            cx += p.alpha;
            cy += p.beta;
        }
        cx /= poly.size();
        cy /= poly.size();
        CHECK(kerr::point_in_shadow(cx, cy, poly));      // centroid inside
        CHECK(!kerr::point_in_shadow(50.0, 50.0, poly)); // far outside
    }

    if (failures == 0) {
        std::puts("kerr_shadow_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "kerr_shadow_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
