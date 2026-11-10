// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/geodesics/photon_lyapunov.hpp
// Instability of the Schwarzschild photon sphere, M = 1.
//
// The equatorial null orbit equation is u'' = 3 u^2 - u, u = 1/r.
// At the photon sphere u = 1/3 the right-hand side vanishes. Its
// derivative with respect to u is 6 u - 1 = 1, so a radial perturbation
// delta obeys delta'' = delta. With delta(0) = eps and delta'(0) = 0,
// delta(phi) = eps cosh(phi).
//
// One half-orbit is Delta phi = pi, so the perturbation grows by e^{pi}.
// The screen demagnification of successive photon subrings is the
// reciprocal, e^{-pi}. Johnson, Lupsasca, Strominger, Wong et al. 2020,
// arXiv:1907.04329, quote gamma = pi for Schwarzschild and a subring
// flux ratio about e^{-gamma}.
//
// The same instability in coordinate time of a static observer at
// infinity is gamma_t = 1/(3 sqrt(3)) = 1/sqrt(27). Hadar, Kapec,
// Lupsasca & Strominger 2022, arXiv:2205.05064, equation (2.12). The
// orbital frequency of the photon sphere is the same number, so
// gamma_t * (pi / Omega) = pi.
//
// In affine time the growth rate is 1/sqrt(3). Coordinate time and
// affine time differ by dt/d lambda = 1/(1 - 2/r) = 3 on r = 3.
//
// Truth tier: analytic_classical for the closed forms. The integrated
// growth check in the tests is a numerical_approximation.

#pragma once

#include <cmath>

namespace blackhole_ds::geodesics {

inline constexpr double lyapunov_pi = 3.14159265358979323846;

// Per unit coordinate time, M = 1. Equals 1/sqrt(27).
[[nodiscard]] inline double schwarzschild_lyapunov_coordinate() noexcept {
    return 1.0 / (3.0 * std::sqrt(3.0));
}

// Per unit affine time, M = 1, for the normalization E = 1.
[[nodiscard]] inline double schwarzschild_lyapunov_affine() noexcept {
    return 1.0 / std::sqrt(3.0);
}

// Dimensionless exponent per half-orbit. Demagnification is e^{-this}.
[[nodiscard]] inline double schwarzschild_half_orbit_exponent() noexcept {
    return lyapunov_pi;
}

[[nodiscard]] inline double schwarzschild_subring_demagnification() noexcept {
    return std::exp(-lyapunov_pi);
}

// Linearized radial factor. delta(phi)/delta(0) = cosh(phi) when the
// initial radial velocity vanishes.
[[nodiscard]] inline double photon_perturbation_factor(double phi) noexcept {
    return std::cosh(phi);
}

} // namespace blackhole_ds::geodesics
