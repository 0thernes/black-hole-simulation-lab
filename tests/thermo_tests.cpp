// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/thermo_tests.cpp
// Validates the semiclassical black-hole thermodynamics module
// (thermo/hawking.hpp) against textbook values and exact scaling/limits.

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/thermo/hawking.hpp"

namespace th = blackhole_ds::thermo;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close_rel(double a, double b, double rel) {
    return std::abs(a - b) <= rel * std::abs(b);
}

} // namespace

int main() {
    // --- Schwarzschild against textbook values for 1 solar mass ---
    {
        const double T = th::hawking_temperature_schwarzschild_K(1.0);
        CHECK(T > 6.10e-8 && T < 6.25e-8); // ~6.17e-8 K

        const double S =
            th::bekenstein_hawking_entropy_over_kB_schwarzschild(1.0);
        CHECK(S > 1.00e77 && S < 1.10e77); // ~1.05e77 (dimensionless, S/k_B)

        const double t = th::evaporation_time_schwarzschild_years(1.0);
        CHECK(t > 2.0e67 && t < 2.2e67); // ~2.1e67 yr
    }

    // --- Exact mass scalings: T ~ 1/M, S ~ M^2, t ~ M^3 ---
    {
        const double T1 = th::hawking_temperature_schwarzschild_K(1.0);
        const double T10 = th::hawking_temperature_schwarzschild_K(10.0);
        CHECK(close_rel(T10, T1 / 10.0, 1e-12));

        const double S1 =
            th::bekenstein_hawking_entropy_over_kB_schwarzschild(1.0);
        const double S2 =
            th::bekenstein_hawking_entropy_over_kB_schwarzschild(2.0);
        CHECK(close_rel(S2, 4.0 * S1, 1e-12));

        const double t1 = th::evaporation_time_schwarzschild_years(1.0);
        const double t2 = th::evaporation_time_schwarzschild_years(2.0);
        CHECK(close_rel(t2, 8.0 * t1, 1e-12));
    }

    // --- Kerr reduces to Schwarzschild at a = 0 ---
    {
        CHECK(close_rel(th::kerr_temperature_factor(0.0), 1.0, 1e-12));
        CHECK(close_rel(th::horizon_area_factor(0.0), 1.0, 1e-12));
        CHECK(close_rel(th::irreducible_mass_fraction(0.0), 1.0, 1e-12));
        CHECK(close_rel(th::hawking_temperature_kerr_K(5.0, 0.0),
                        th::hawking_temperature_schwarzschild_K(5.0), 1e-12));
    }

    // --- Extremal Kerr (a = 1): cold (T -> 0), area halves, M_irr = M/sqrt(2)
    // ---
    {
        CHECK(std::abs(th::kerr_temperature_factor(1.0)) < 1e-12);
        CHECK(close_rel(th::horizon_area_factor(1.0), 0.5, 1e-12));
        CHECK(close_rel(th::irreducible_mass_fraction(1.0),
                        1.0 / std::sqrt(2.0), 1e-12));
        CHECK(th::hawking_temperature_kerr_K(1.0, 1.0) <
              1e-3 * th::hawking_temperature_schwarzschild_K(1.0));
    }

    // --- Monotonicity: spinning up cools the hole and shrinks the horizon ---
    {
        CHECK(th::kerr_temperature_factor(0.9) <
              th::kerr_temperature_factor(0.5));
        CHECK(th::kerr_temperature_factor(0.5) <
              th::kerr_temperature_factor(0.0));
        CHECK(th::horizon_area_factor(0.9) < th::horizon_area_factor(0.5));
        CHECK(th::irreducible_mass_fraction(0.99) < 1.0);
        // Sign-symmetric in spin (prograde vs retrograde magnitude).
        CHECK(close_rel(th::kerr_temperature_factor(0.7),
                        th::kerr_temperature_factor(-0.7), 1e-12));
    }

    // --- Spin clamp: beyond extremal is treated as extremal, no NaN ---
    {
        CHECK(std::isfinite(th::kerr_temperature_factor(1.5)));
        CHECK(close_rel(th::horizon_area_factor(1.5),
                        th::horizon_area_factor(1.0), 1e-12));
    }

    if (failures == 0) {
        std::puts("thermo_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "thermo_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
