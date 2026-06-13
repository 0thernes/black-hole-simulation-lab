// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/geodesic_tests.cpp
// Validates the first geodesic module (Schwarzschild photon orbits) against
// famous closed-form results: the weak-field light deflection 4M/b, the
// critical impact parameter sqrt(27) M, and ray capture/escape.
//
// Uses a tiny CHECK macro so behavior is identical in Debug and Release.

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/geodesics/schwarzschild_photon.hpp"

namespace sch = blackhole_ds::geodesics::schwarzschild;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close_rel(double actual, double expected, double rel_tol) {
    return std::abs(actual - expected) <= rel_tol * std::abs(expected);
}

} // namespace

int main() {
    // --- Critical impact parameter is sqrt(27) = 3*sqrt(3) ---
    CHECK(close_rel(sch::b_critical, std::sqrt(27.0), 1e-12));
    CHECK(close_rel(sch::b_critical, 3.0 * std::sqrt(3.0), 1e-12));

    // --- Ray classification around the shadow boundary ---
    CHECK(sch::classify(10.0) == sch::RayFate::Escapes);
    CHECK(sch::classify(3.0) == sch::RayFate::Captured);
    CHECK(sch::classify(sch::b_critical) == sch::RayFate::Critical);

    // --- Turning point satisfies its defining equation u^2 - 2u^3 = 1/b^2,
    //     and is ~1/b for a weak ray (with an O(1/b) correction) ---
    {
        const double b = 1000.0;
        const double u = sch::turning_point_u(b);
        const double residual = u * u - 2.0 * u * u * u - 1.0 / (b * b);
        CHECK(std::abs(residual) < 1e-12);  // rigorous: solves the equation
        CHECK(close_rel(u, 1.0 / b, 5e-3)); // leading order ~1/b
        CHECK(u < 1.0 / 3.0);               // outside the photon sphere
        // Captured rays have no periapsis -> negative sentinel.
        CHECK(sch::turning_point_u(3.0) < 0.0);
    }

    // --- Weak-field deflection converges to the Eddington value 4M/b ---
    {
        const double d1000 = sch::light_deflection_angle(1000.0);
        CHECK(close_rel(d1000, 4.0 / 1000.0, 0.01)); // 1% at b=1000

        const double d100 = sch::light_deflection_angle(100.0);
        CHECK(close_rel(d100, 4.0 / 100.0, 0.05)); // 5% at b=100

        // Deflection decreases with impact parameter (monotone).
        CHECK(d100 > d1000);

        // GR bends MORE than the leading 4M/b term (higher-order terms add).
        CHECK(sch::light_deflection_angle(20.0) > 4.0 / 20.0);
    }

    // --- A strongly-bent but escaping ray gives a finite, large deflection ---
    {
        const double d = sch::light_deflection_angle(5.5); // just above b_crit
        CHECK(std::isfinite(d));
        CHECK(d > 1.0); // bends by more than a radian near the photon sphere
    }

    // --- A captured ray returns NaN from the deflection routine ---
    {
        const double d = sch::light_deflection_angle(3.0);
        CHECK(std::isnan(d));
    }

    if (failures == 0) {
        std::puts("geodesic_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "geodesic_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
