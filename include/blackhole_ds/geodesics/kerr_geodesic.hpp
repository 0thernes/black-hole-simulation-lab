// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/geodesics/kerr_geodesic.hpp
// Roadmap M5: NULL GEODESICS in the Kerr (spinning) spacetime.
//
// This is the engine the spinning-disk ray trace will ride on. Where the
// Schwarzschild photon obeys the planar orbit equation d2u/dphi2 + u = 3u^2,
// a Kerr photon does NOT stay in a plane -- frame dragging twists it -- so we
// integrate the full geodesic in Boyer-Lindquist coordinates (t, r, theta,
// phi) using the SEPARATED constants of motion (Carter 1968): energy E,
// axial angular momentum L_z, and the Carter constant Q.
//
// We use the SECOND-ORDER form in Mino time lambda (d/d-affine = Sigma d/d-
// lambda). The great virtue of Mino time is that r and theta DECOUPLE into
// one-dimensional "particle in a potential" equations:
//
//     (dr/dlambda)^2     = R(r)
//     (dtheta/dlambda)^2 = Theta(theta)
//
// and differentiating once removes the square root -- so there is NO sign
// flip to track at the radial/polar turning points (where R or Theta -> 0),
// which is exactly where a naive first-order sqrt integrator breaks. We
// integrate the accelerations
//
//     d2r/dlambda^2     = (1/2) R'(r)
//     d2theta/dlambda^2 = (1/2) Theta'(theta)
//
// and the cyclic coordinates from the first integrals:
//
//     dphi/dlambda = -(a E - L_z/sin^2 th) + a P / Delta
//     dt/dlambda   = -a (a E sin^2 th - L_z) + (r^2 + a^2) P / Delta
//
// with (M = 1):
//     Sigma = r^2 + a^2 cos^2 th,   Delta = r^2 - 2r + a^2,
//     P     = E (r^2 + a^2) - a L_z,
//     R(r)     = P^2 - Delta [ (L_z - a E)^2 + Q ],
//     Theta(th)= Q - cos^2 th [ L_z^2 / sin^2 th - a^2 E^2 ].
//
// Conventions: signature (-,+,+,+); Q is the photon Carter constant in the
// convention where an EQUATORIAL orbit has Q = 0. Verified analytic limits:
// at a -> 0 this reduces to the Schwarzschild null orbit (R(r) ->
// E^2 r^4 - L^2 r(r-2), dt/d-affine -> E/(1-2/r)), and the equatorial photon
// circle sits at R = R' = 0 (r = 3 for a = 0).
//
// Truth tier: numerical_approximation (RK4-integrated). The equations of
// motion themselves are analytic_classical; the conserved-quantity drift is
// bounded and checked in tests/kerr_geodesic_tests.cpp.
// References: Carter 1968; Bardeen 1973; Chandrasekhar 1983; Gralla &
// Lupsasca 2020 (Mino-time formulation).

#pragma once

#include <cmath>

#include "blackhole_ds/integrators/ode_state.hpp"

namespace blackhole_ds::geodesics::kerr {

// State layout for the 6-component Mino-time system.
// [0]=r  [1]=theta  [2]=phi  [3]=t  [4]=dr/dlambda  [5]=dtheta/dlambda
enum KGIndex {
    KG_R = 0,
    KG_TH = 1,
    KG_PHI = 2,
    KG_T = 3,
    KG_RDOT = 4,
    KG_THDOT = 5
};

using KGState = integrators::State<6>;

// Conserved parameters of a Kerr null geodesic (M = 1, photon).
struct GeodesicConstants {
    double a;  // spin a/M
    double E;  // energy (-p_t)
    double Lz; // axial angular momentum (p_phi)
    double Q;  // Carter constant (= 0 for equatorial orbits)
};

inline constexpr double kg_pi = 3.14159265358979323846;

// --- Metric helper quantities (M = 1) -----------------------------------
[[nodiscard]] inline double kg_delta(double r, double a) noexcept {
    return r * r - 2.0 * r + a * a;
}
[[nodiscard]] inline double kg_sigma(double r, double theta,
                                     double a) noexcept {
    const double c = std::cos(theta);
    return r * r + a * a * c * c;
}
// P(r) = E (r^2 + a^2) - a L_z
[[nodiscard]] inline double kg_P(double r,
                                 const GeodesicConstants& k) noexcept {
    return k.E * (r * r + k.a * k.a) - k.a * k.Lz;
}

// --- Radial and polar potentials and their half-derivatives -------------
// R(r) = P^2 - Delta [ (L_z - a E)^2 + Q ]
[[nodiscard]] inline double kg_R(double r,
                                 const GeodesicConstants& k) noexcept {
    const double P = kg_P(r, k);
    const double kappa = (k.Lz - k.a * k.E) * (k.Lz - k.a * k.E) + k.Q;
    return P * P - kg_delta(r, k.a) * kappa;
}
// (1/2) dR/dr = 2 E r P - (r - 1) [ (L_z - a E)^2 + Q ]
[[nodiscard]] inline double kg_half_dR(double r,
                                       const GeodesicConstants& k) noexcept {
    const double P = kg_P(r, k);
    const double kappa = (k.Lz - k.a * k.E) * (k.Lz - k.a * k.E) + k.Q;
    return 2.0 * k.E * r * P - (r - 1.0) * kappa;
}
// Theta(theta) = Q - cos^2 th [ L_z^2 / sin^2 th - a^2 E^2 ]
//             = Q + a^2 E^2 cos^2 th - L_z^2 cot^2 th
[[nodiscard]] inline double kg_Theta(double theta,
                                     const GeodesicConstants& k) noexcept {
    const double c = std::cos(theta);
    const double s = std::sin(theta);
    const double s2 = s * s;
    const double cot2 = (s2 > 1e-300) ? (c * c) / s2 : 0.0;
    return k.Q + k.a * k.a * k.E * k.E * c * c - k.Lz * k.Lz * cot2;
}
// (1/2) dTheta/dtheta = -a^2 E^2 sin th cos th + L_z^2 cos th / sin^3 th
[[nodiscard]] inline double
kg_half_dTheta(double theta, const GeodesicConstants& k) noexcept {
    const double c = std::cos(theta);
    double s = std::sin(theta);
    // Guard the pole: the L_z^2/sin^3 term diverges as theta -> 0, pi. Real
    // orbits with L_z != 0 never reach the axis (Theta < 0 there), but clamp
    // so the integrator degrades gracefully instead of producing NaNs.
    if (std::abs(s) < 1e-7) {
        s = std::copysign(1e-7, s == 0.0 ? 1.0 : s);
    }
    const double s3 = s * s * s;
    return -k.a * k.a * k.E * k.E * s * c + k.Lz * k.Lz * c / s3;
}

// --- Equation of motion: returns d(state)/d-lambda ----------------------
// A stateless functor so it composes with rk4_step / rk4_integrate.
struct GeodesicDeriv {
    GeodesicConstants k;

    [[nodiscard]] KGState operator()(double /*lambda*/,
                                     const KGState& y) const noexcept {
        const double r = y[KG_R];
        const double th = y[KG_TH];
        double s = std::sin(th);
        if (std::abs(s) < 1e-7) {
            s = std::copysign(1e-7, s == 0.0 ? 1.0 : s);
        }
        const double s2 = s * s;
        const double Delta = kg_delta(r, k.a);
        const double P = kg_P(r, k);

        KGState dy{};
        dy[KG_R] = y[KG_RDOT];
        dy[KG_TH] = y[KG_THDOT];
        dy[KG_PHI] = -(k.a * k.E - k.Lz / s2) + k.a * P / Delta;
        dy[KG_T] =
            -k.a * (k.a * k.E * s2 - k.Lz) + (r * r + k.a * k.a) * P / Delta;
        dy[KG_RDOT] = kg_half_dR(r, k);
        dy[KG_THDOT] = kg_half_dTheta(th, k);
        return dy;
    }
};

// --- Conserved-quantity recovery (for verification) ---------------------
// On the constraint surface (dr/dlambda)^2 = R(r); the residual measures
// integration drift off it. Should stay ~0 along a correct integration.
[[nodiscard]] inline double
radial_residual(const KGState& y, const GeodesicConstants& k) noexcept {
    return y[KG_RDOT] * y[KG_RDOT] - kg_R(y[KG_R], k);
}
[[nodiscard]] inline double
polar_residual(const KGState& y, const GeodesicConstants& k) noexcept {
    return y[KG_THDOT] * y[KG_THDOT] - kg_Theta(y[KG_TH], k);
}

// Recover the Carter constant from the instantaneous state:
//   Q = (dtheta/dlambda)^2 - a^2 E^2 cos^2 th + L_z^2 cot^2 th
// Should equal k.Q (up to integration error) at every step.
[[nodiscard]] inline double recover_Q(const KGState& y,
                                      const GeodesicConstants& k) noexcept {
    const double c = std::cos(y[KG_TH]);
    const double s = std::sin(y[KG_TH]);
    const double s2 = s * s;
    const double cot2 = (s2 > 1e-300) ? (c * c) / s2 : 0.0;
    return y[KG_THDOT] * y[KG_THDOT] - k.a * k.a * k.E * k.E * c * c +
           k.Lz * k.Lz * cot2;
}

// --- Observer image plane -> constants of motion (Cunningham-Bardeen) ----
// A photon received at image-plane coordinates (alpha, beta) by a distant
// observer at inclination i (polar angle of the observer, degrees) carries,
// with the natural normalization E = 1:
//
//     L_z = -alpha sin i
//     Q   =  beta^2 + (alpha^2 - a^2) cos^2 i
//
// alpha is the apparent displacement perpendicular to the projected spin
// axis; beta is parallel to it. This is the exact map that turns each pixel
// into a geodesic to trace backward. (Cunningham & Bardeen 1973.)
//
// Self-consistency identity, used as a test anchor: substituting these into
// the polar potential gives Theta(i) = beta^2 exactly, so the initial polar
// velocity is |beta| and the ray is on-shell at the observer.
[[nodiscard]] inline GeodesicConstants
constants_from_image(double alpha, double beta, double inclination_deg,
                     double a) noexcept {
    const double i = inclination_deg * kg_pi / 180.0;
    const double sin_i = std::sin(i);
    const double cos_i = std::cos(i);
    GeodesicConstants k{};
    k.a = a;
    k.E = 1.0;
    k.Lz = -alpha * sin_i;
    k.Q = beta * beta + (alpha * alpha - a * a) * cos_i * cos_i;
    return k;
}

// Build the on-shell initial state for that pixel's photon, placed at a large
// observer radius and aimed inward (ingoing: dr/dlambda < 0). theta starts at
// the observer inclination; the polar velocity sign follows -beta (the image
// "up" direction). Both residuals are zero at this state by construction.
// (alpha enters only through the constants k, so it is not a parameter here.)
[[nodiscard]] inline KGState
initial_state_from_image(double beta, double inclination_deg,
                         const GeodesicConstants& k,
                         double r_observer = 1000.0) noexcept {
    const double i = inclination_deg * kg_pi / 180.0;
    KGState y{};
    y[KG_R] = r_observer;
    y[KG_TH] = i;
    y[KG_PHI] = 0.0;
    y[KG_T] = 0.0;
    const double R0 = kg_R(r_observer, k);
    y[KG_RDOT] = -std::sqrt(R0 > 0.0 ? R0 : 0.0); // ingoing
    y[KG_THDOT] = -beta;                          // Theta(i) == beta^2
    return y;
}

} // namespace blackhole_ds::geodesics::kerr
