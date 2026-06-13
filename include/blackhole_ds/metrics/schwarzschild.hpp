// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// blackhole_ds/metrics/schwarzschild.hpp
// Analytic Schwarzschild observables.
//
// Truth tier: analytic_classical.
// Source: Schwarzschild 1916; MTW 1973; Bardeen, Press, Teukolsky 1972.

#pragma once

#include "blackhole_ds/core/constants.hpp"
#include "blackhole_ds/units.hpp"

namespace blackhole_ds::metrics::schwarzschild {

// Schwarzschild radius r_s = 2 G M / c^2 in meters, for a mass in solar masses.
[[nodiscard]] constexpr double radius_m(double mass_solar) noexcept {
    return 2.0 * mass_solar * core::constants::geometric_meters_per_solar_mass;
}

// Photon sphere radius 3 G M / c^2 in meters.
[[nodiscard]] constexpr double photon_sphere_m(double mass_solar) noexcept {
    return 3.0 * mass_solar * core::constants::geometric_meters_per_solar_mass;
}

// Innermost stable circular orbit radius 6 G M / c^2 in meters.
[[nodiscard]] constexpr double isco_m(double mass_solar) noexcept {
    return 6.0 * mass_solar * core::constants::geometric_meters_per_solar_mass;
}

} // namespace blackhole_ds::metrics::schwarzschild
