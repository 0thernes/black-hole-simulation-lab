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

// Convenience: GM_sun / c^2 in meters. The geometric-units scale factor
// for one solar mass. See MTW 1973 and Bardeen, Press, Teukolsky 1972.
inline constexpr double geometric_meters_per_solar_mass =
    1.476625038e3; // meters

} // namespace blackhole_ds::core::constants
