// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// blackhole_ds/geodesics/schwarzschild_photon.hpp
// First geodesic module (roadmap M1): equatorial null geodesics in
// Schwarzschild spacetime, i.e. how light bends around a non-rotating
// black hole.
//
// Formulation: the photon orbit equation in the inverse radius u = M/r,
// with the azimuthal angle phi as the independent variable (geometric
// units, M = 1, so u is dimensionless and the impact parameter b is in
// units of M):
//
//     d^2 u / dphi^2 + u = 3 u^2
//
// The "+ u" is the flat-space term; the "3 u^2" is the general-relativistic
// correction that bends light extra and creates the photon sphere at
// r = 3M (u = 1/3). This is the cleanest second-order ODE for lensing and
// is integrated by the project's adaptive/fixed Runge-Kutta steppers.
//
// Truth tiers: the critical impact parameter and the orbit equation are
// analytic_classical; the integrated deflection angle is a
// numerical_approximation with a documented error order (RK4).
//
// References (analytic_classical): Misner-Thorne-Wheeler 1973 ch. 25;
// the weak-field deflection 4M/b is the classic Eddington 1919 result.

#pragma once

#include <cmath>

#include "blackhole_ds/integrators/ode_state.hpp"
#include "blackhole_ds/integrators/rk4.hpp"

namespace blackhole_ds::geodesics::schwarzschild {

// pi without relying on the non-standard M_PI macro.
inline constexpr double pi = 3.14159265358979323846;

// Critical photon impact parameter b_crit = sqrt(27) M = 3*sqrt(3) M.
// Rays with b > b_crit escape; b < b_crit are captured; b = b_crit
// asymptote to the photon sphere. This is the shadow boundary.
inline const double b_critical = std::sqrt(27.0);

enum class RayFate { Escapes, Captured, Critical };

// Classify a ray by its impact parameter b (in units of M).
[[nodiscard]] inline RayFate classify(double b_over_M,
                                      double tol = 1e-6) noexcept {
    const double d = b_over_M - b_critical;
    if (std::abs(d) <= tol) {
        return RayFate::Critical;
    }
    return d > 0.0 ? RayFate::Escapes : RayFate::Captured;
}

// Photon turning point u_max = M / r_min for an escaping ray of impact
// parameter b. From the first integral (du/dphi)^2 = 1/b^2 - u^2 + 2 u^3,
// at periapsis du/dphi = 0, so u_max solves  u^2 - 2 u^3 = 1/b^2  with the
// relevant root in (0, 1/3). Returns a negative sentinel if the ray is
// captured (no such root, i.e. b <= b_critical).
[[nodiscard]] inline double turning_point_u(double b_over_M) {
    if (b_over_M <= b_critical) {
        return -1.0; // captured: no periapsis outside the photon sphere
    }
    const double target = 1.0 / (b_over_M * b_over_M);
    // h(u) = u^2 - 2 u^3 is increasing on (0, 1/3) from 0 to 1/27.
    // Bisect for h(u) = target. Robust, derivative-free.
    double lo = 0.0;
    double hi = 1.0 / 3.0;
    const auto h = [](double u) { return u * u - 2.0 * u * u * u; };
    for (int i = 0; i < 200; ++i) {
        const double mid = 0.5 * (lo + hi);
        if (h(mid) < target) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return 0.5 * (lo + hi);
}

// Total light-deflection angle (radians) for an escaping ray of impact
// parameter b, measured as the bend beyond a straight line. For a straight
// ray the swept azimuth from incoming to outgoing infinity is pi; the
// deflection is the excess.
//
// Method: integrate the orbit ODE from periapsis (phi = 0, u = u_max,
// du/dphi = 0) outward in phi with fixed-step RK4 until u returns to 0
// (escape to infinity), linearly interpolating the crossing. By symmetry
// the full deflection is 2*phi_escape - pi.
//
// Returns NaN if the ray is captured, or if it fails to escape within the
// phi cap (near-critical rays wind many times and the deflection diverges).
//
// Truth tier: numerical_approximation (global error O(dphi^4)).
[[nodiscard]] inline double light_deflection_angle(double b_over_M,
                                                   double dphi = 1.0e-3,
                                                   double phi_cap = 200.0) {
    const double u_max = turning_point_u(b_over_M);
    if (u_max < 0.0) {
        return std::nan("");
    }

    // State y = [u, du/dphi]. Orbit RHS: u' = w, w' = 3 u^2 - u.
    using integrators::State;
    const auto deriv = [](double /*phi*/, const State<2>& y) -> State<2> {
        return State<2>{y[1], 3.0 * y[0] * y[0] - y[0]};
    };

    State<2> y{u_max, 0.0};
    double phi = 0.0;
    const int max_steps = static_cast<int>(phi_cap / dphi);
    for (int i = 0; i < max_steps; ++i) {
        const State<2> next = integrators::rk4_step<2>(deriv, phi, y, dphi);
        if (next[0] <= 0.0) {
            // u crossed zero between phi and phi+dphi; linearly interpolate.
            const double frac = y[0] / (y[0] - next[0]); // in [0,1]
            const double phi_escape = phi + frac * dphi;
            return 2.0 * phi_escape - pi;
        }
        y = next;
        phi += dphi;
    }
    return std::nan(""); // did not escape within the cap (near-critical)
}

} // namespace blackhole_ds::geodesics::schwarzschild
