// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
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
// For a == 0, returns exactly 6.0 (Schwarzschild limit); a == +1 gives the
// exact extremal value 1.0, a == -1 gives 9.0. The spin is clamped to the
// physical domain [-1, 1] (the Bardeen-Press-Teukolsky closed form is finite
// throughout, since cbrt(1 - a^2) -> 0 smoothly at |a| = 1); a > 1 would be a
// naked singularity and is clamped to extremal. (Audit 2026-06-14: the prior
// 0.999 clamp silently mis-reported valid near-extremal spins up to the
// schema's 0.9999.)
[[nodiscard]] inline double isco_dimensionless(double spin_a_over_M) noexcept {
    const double a = std::clamp(spin_a_over_M, -1.0, 1.0);
    const double z1 = 1.0 + std::cbrt(1.0 - a * a) *
                                (std::cbrt(1.0 + a) + std::cbrt(1.0 - a));
    const double z2 = std::sqrt(3.0 * a * a + z1 * z1);
    return 3.0 + z2 -
           std::copysign(std::sqrt((3.0 - z1) * (3.0 + z1 + 2.0 * z2)), a);
}

// Dimensionless prograde equatorial photon sphere in units of GM/c^2.
// For a == 0, returns exactly 3.0; a == +1 gives the exact extremal 1.0.
// Spin is clamped to the physical domain [-1, 1] (closed form finite at
// |a| = 1). (Audit 2026-06-14: widened from the prior 0.999 clamp.)
[[nodiscard]] inline double
photon_sphere_dimensionless(double spin_a_over_M) noexcept {
    const double a = std::clamp(spin_a_over_M, -1.0, 1.0);
    return 2.0 * (1.0 + std::cos((2.0 / 3.0) * std::acos(-a)));
}

} // namespace blackhole_ds::metrics::kerr
