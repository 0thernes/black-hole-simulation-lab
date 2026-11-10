// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/metrics/schwarzschild_christoffel.hpp
// Nonzero Schwarzschild Christoffel symbols in the coordinate basis,
// geometric units, M = 1. Signature (-,+,+,+), standard Schwarzschild
// coordinates. Truth tier: analytic_classical.
//
//   g_tt = -(1 - 2/r),   g_rr = 1/(1 - 2/r),
//   g_thth = r^2,        g_phph = r^2 sin^2 theta.
//
//   Gamma^r_tt = (1 - 2/r) / r^2
//   Gamma^t_tr = 1 / (r (r - 2))          (and Gamma^t_rt the same)
//   Gamma^r_rr = -1 / (r (r - 2))
//   Gamma^r_thth = -(r - 2)
//   Gamma^th_rth = 1/r
//
// Source: the metric in Schwarzschild 1916 and the standard coordinate
// formulae in Misner, Thorne & Wheeler 1973, chapter 31. The tests
// rebuild Gamma^r_tt from a finite difference of g_tt, which is the
// check an automatic derivative of the metric would have to pass.
// Eshkol's compiler AD is not linked here; see docs/PHYSICS.md.

#pragma once

#include <cmath>

namespace blackhole_ds::metrics::schwarzschild {

[[nodiscard]] inline double g_tt(double r) noexcept {
    return -(1.0 - 2.0 / r);
}

[[nodiscard]] inline double g_rr(double r) noexcept {
    return 1.0 / (1.0 - 2.0 / r);
}

[[nodiscard]] inline double gamma_r_tt(double r) noexcept {
    return (1.0 - 2.0 / r) / (r * r);
}

[[nodiscard]] inline double gamma_t_tr(double r) noexcept {
    return 1.0 / (r * (r - 2.0));
}

[[nodiscard]] inline double gamma_r_rr(double r) noexcept {
    return -1.0 / (r * (r - 2.0));
}

[[nodiscard]] inline double gamma_r_thth(double r) noexcept {
    return -(r - 2.0);
}

[[nodiscard]] inline double gamma_th_rth(double r) noexcept {
    return 1.0 / r;
}

// Gamma^r_tt = -1/2 g^{rr} d(g_tt)/dr, with g^{rr} = 1 - 2/r.
[[nodiscard]] inline double
gamma_r_tt_from_metric_derivative(double r, double h) noexcept {
    const double dg = (g_tt(r + h) - g_tt(r - h)) / (2.0 * h);
    const double grr_up = 1.0 - 2.0 / r;
    return -0.5 * grr_up * dg;
}

} // namespace blackhole_ds::metrics::schwarzschild
