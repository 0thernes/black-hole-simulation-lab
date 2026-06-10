// blackhole_ds/metrics/kerr.hpp
// Analytic Kerr observables for prograde equatorial orbits.
//
// Truth tier: analytic_classical.
// Source: Bardeen, Press, Teukolsky 1972; Chandrasekhar 1983.

#pragma once

#include <algorithm>
#include <cmath>

namespace blackhole_ds::metrics::kerr {

// Dimensionless prograde ISCO radius in units of GM/c^2.
// For a == 0, returns exactly 6.0 (Schwarzschild limit).
[[nodiscard]] inline double isco_dimensionless(double spin_a_over_M) noexcept {
    const double a = std::clamp(spin_a_over_M, -0.999, 0.999);
    const double z1 = 1.0 +
        std::cbrt(1.0 - a * a) * (std::cbrt(1.0 + a) + std::cbrt(1.0 - a));
    const double z2 = std::sqrt(3.0 * a * a + z1 * z1);
    return 3.0 + z2 -
        std::copysign(std::sqrt((3.0 - z1) * (3.0 + z1 + 2.0 * z2)), a);
}

// Dimensionless prograde equatorial photon sphere in units of GM/c^2.
// For a == 0, returns exactly 3.0.
[[nodiscard]] inline double photon_sphere_dimensionless(double spin_a_over_M) noexcept {
    const double a = std::clamp(spin_a_over_M, -0.999, 0.999);
    return 2.0 * (1.0 + std::cos((2.0 / 3.0) * std::acos(-a)));
}

} // namespace blackhole_ds::metrics::kerr
