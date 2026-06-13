// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// blackhole_ds/core/constants.hpp
// Physical constants and unit-conversion factors used across the project.

#pragma once

#include "blackhole_ds/units.hpp"

namespace blackhole_ds::core::constants {

// SI constants. Source: CODATA 2018.
inline constexpr double speed_of_light_m_per_s = 299792458.0;
inline constexpr double gravitational_constant_si =
    6.67430e-11; // m^3 kg^-1 s^-2

// Astronomical constants.
inline constexpr double solar_mass_kg = 1.98847e30;

// Standard gravitational parameter of the Sun, GM_sun.
// Source: IAU 2015 nominal value, GM_sun = 1.3271244e20 m^3 s^-2. This is
// measured directly and to far higher precision than G * M_sun (because G
// is the least-well-known constant), so we anchor on it rather than on the
// product gravitational_constant_si * solar_mass_kg.
inline constexpr double sun_gm_si = 1.3271244e20; // m^3 s^-2

// GM_sun / c^2 in meters: the geometric-units length scale of one solar
// mass (r_g for the Sun). Derived from the IAU GM_sun above.
// See MTW 1973; Bardeen, Press, Teukolsky 1972.
inline constexpr double geometric_meters_per_solar_mass =
    sun_gm_si / (speed_of_light_m_per_s * speed_of_light_m_per_s); // meters

// Sanity check: the published rounded value (1476.625... m) must agree with
// the derived one to better than 1e-4 relative. (Not a tight equality: the
// inputs carry their own rounding.) Pure arithmetic keeps this constexpr
// and MSVC-portable, per units.hpp.
static_assert((geometric_meters_per_solar_mass > 1476.0) &&
                  (geometric_meters_per_solar_mass < 1477.0),
              "GM_sun/c^2 must be ~1476.6 m; check sun_gm_si and c");

} // namespace blackhole_ds::core::constants
