// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/thermo/hawking.hpp
// Semiclassical black-hole thermodynamics: Hawking temperature, Bekenstein-
// Hawking entropy, horizon area, irreducible mass, and the evaporation
// timescale, for Schwarzschild and Kerr.
//
// TRUTH TIER. These are exact closed-form results of well-established theory,
// but they live at two different tiers and the distinction is kept honest:
//   * Horizon area, irreducible mass, the area ratios -> analytic_classical
//     (pure GR: the horizon geometry of the Kerr metric).
//   * Hawking temperature, Bekenstein-Hawking entropy, evaporation time ->
//     SEMICLASSICAL (quantum field theory on a fixed classical background;
//     Hawking 1974, Bekenstein 1973). They are theoretically exact in that
//     framework but UNOBSERVED -- no black-hole evaporation or BH entropy has
//     ever been measured. Treat them as theory, never as observation.
//
// References (in the repo's source-card corpus): Hawking 1974 (radiation),
// Bekenstein 1973 (entropy), Bardeen-Carter-Hawking 1973 (four laws),
// Misner-Thorne-Wheeler 1973.

#pragma once

#include <algorithm>
#include <cmath>

#include "blackhole_ds/core/constants.hpp"

namespace blackhole_ds::thermo {

inline constexpr double pi = 3.14159265358979323846;
inline constexpr double seconds_per_julian_year = 31557600.0; // 365.25 d

// --- Schwarzschild ----------------------------------------------------------

// Hawking temperature of a Schwarzschild black hole, in kelvin:
//     T_H = hbar c^3 / (8 pi G M k_B).
// We evaluate G M as GM_sun * (M / M_sun) using the precisely-known solar
// GM (constants::sun_gm_si), avoiding the poorly-measured bare G.
// For M = 1 M_sun this is ~6.17e-8 K.
[[nodiscard]] inline double
hawking_temperature_schwarzschild_K(double mass_solar) noexcept {
    namespace c = core::constants;
    const double GM = c::sun_gm_si * mass_solar; // m^3 s^-2
    const double c3 = c::speed_of_light_m_per_s * c::speed_of_light_m_per_s *
                      c::speed_of_light_m_per_s;
    return c::reduced_planck_si * c3 / (8.0 * pi * GM * c::boltzmann_si);
}

// Bekenstein-Hawking entropy of a Schwarzschild black hole, in units of k_B
// (dimensionless): S/k_B = 4 pi G M^2 / (hbar c). For 1 M_sun ~ 1.05e77.
[[nodiscard]] inline double
bekenstein_hawking_entropy_over_kB_schwarzschild(double mass_solar) noexcept {
    namespace c = core::constants;
    const double M = c::solar_mass_kg * mass_solar;   // kg
    const double GM2 = c::sun_gm_si * mass_solar * M; // G M^2 = (GM) * M
    return 4.0 * pi * GM2 / (c::reduced_planck_si * c::speed_of_light_m_per_s);
}

// Schwarzschild evaporation time (pure Hawking emission, geometric-optics /
// Stefan-Boltzmann estimate), in Julian years:
//     t = 5120 pi G^2 M^3 / (hbar c^4).
// For 1 M_sun ~ 2.1e67 yr (vastly older than the universe).
[[nodiscard]] inline double
evaporation_time_schwarzschild_years(double mass_solar) noexcept {
    namespace c = core::constants;
    const double M = c::solar_mass_kg * mass_solar;
    const double GM = c::sun_gm_si * mass_solar; // = G M
    const double G2M3 = GM * GM * M;             // G^2 M^3 = (GM)^2 * M
    const double c4 = std::pow(c::speed_of_light_m_per_s, 4.0);
    const double seconds = 5120.0 * pi * G2M3 / (c::reduced_planck_si * c4);
    return seconds / seconds_per_julian_year;
}

// --- Kerr (dimensionless spin a* = a/M in [-1, 1]) --------------------------
// Clamp to the physical domain; |a*| = 1 is extremal.

// sqrt(1 - a*^2), the recurring horizon factor; 0 at extremal.
[[nodiscard]] inline double horizon_root(double spin_a_over_M) noexcept {
    const double a = std::clamp(spin_a_over_M, -1.0, 1.0);
    return std::sqrt(std::max(0.0, 1.0 - a * a));
}

// Kerr Hawking temperature as a FACTOR multiplying the Schwarzschild value at
// the same mass: T_Kerr / T_Schw = 4 s / ((1 + s)^2 + a*^2), s = sqrt(1-a*^2).
// 1 at a* = 0; -> 0 at extremal (|a*| -> 1): an extremal Kerr hole is cold.
[[nodiscard]] inline double
kerr_temperature_factor(double spin_a_over_M) noexcept {
    const double a = std::clamp(spin_a_over_M, -1.0, 1.0);
    const double s = horizon_root(a);
    return 4.0 * s / ((1.0 + s) * (1.0 + s) + a * a);
}

[[nodiscard]] inline double
hawking_temperature_kerr_K(double mass_solar, double spin_a_over_M) noexcept {
    return hawking_temperature_schwarzschild_K(mass_solar) *
           kerr_temperature_factor(spin_a_over_M);
}

// Kerr horizon area as a factor of the Schwarzschild area at the same mass:
//     A_Kerr / A_Schw = (1 + sqrt(1 - a*^2)) / 2.
// 1 at a* = 0; 1/2 at extremal.
[[nodiscard]] inline double horizon_area_factor(double spin_a_over_M) noexcept {
    return (1.0 + horizon_root(spin_a_over_M)) / 2.0;
}

// Bekenstein-Hawking entropy is proportional to area, so it scales by the same
// factor; S/k_B(Kerr) = horizon_area_factor * S/k_B(Schw).
[[nodiscard]] inline double
bekenstein_hawking_entropy_over_kB_kerr(double mass_solar,
                                        double spin_a_over_M) noexcept {
    return bekenstein_hawking_entropy_over_kB_schwarzschild(mass_solar) *
           horizon_area_factor(spin_a_over_M);
}

// Irreducible mass fraction M_irr / M = sqrt((1 + sqrt(1 - a*^2)) / 2). This is
// the mass that cannot be extracted (e.g. by a Penrose process); 1 at a* = 0,
// 1/sqrt(2) ~ 0.7071 at extremal. M_irr^2 = A c^4 / (16 pi G^2).
[[nodiscard]] inline double
irreducible_mass_fraction(double spin_a_over_M) noexcept {
    return std::sqrt(horizon_area_factor(spin_a_over_M));
}

} // namespace blackhole_ds::thermo
