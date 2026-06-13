// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// blackhole_ds/geodesics/kerr_shadow.hpp
// Roadmap M5: the shadow of a SPINNING (Kerr) black hole.
//
// Unlike Schwarzschild (a circular shadow of radius sqrt(27) M), a Kerr
// black hole casts an ASYMMETRIC, D-shaped shadow: spin flattens the side
// where the photon orbit co-rotates with the hole. This is the defining
// visual signature of black-hole spin and is what the EHT measures.
//
// The shadow boundary is the projection of the unstable SPHERICAL photon
// orbits, and -- remarkably -- it is CLOSED FORM (Bardeen 1973). For a
// photon orbit of Boyer-Lindquist radius r around a hole of spin a (M = 1),
// the two conserved quantities are:
//
//     xi(r)  = L_z/E = -(r^3 - 3 r^2 + a^2 (r + 1)) / (a (r - 1))
//     eta(r) = Q/E^2 =  r^3 (4 a^2 - r (r - 3)^2) / (a^2 (r - 1)^2)
//
// and, for an observer at inclination i, the apparent (celestial) image
// coordinates of the shadow edge are:
//
//     alpha(r) = -xi(r) / sin i
//     beta(r)  = +/- sqrt( eta(r) + a^2 cos^2 i - xi(r)^2 cot^2 i )
//
// Sweeping r over the photon-orbit range [r_prograde, r_retrograde] traces
// the closed boundary curve.
//
// Correctness anchor (verified by the a -> 0 limit, and tested): as a -> 0
// the photon orbit radius -> 3M, xi -> 0, eta -> 27, so alpha^2 + beta^2 ->
// 27 -- exactly the Schwarzschild shadow circle of radius sqrt(27) M.
//
// Truth tier: analytic_classical. These are exact closed-form GR results;
// no numerical geodesic integration is involved in the boundary.
// References: Bardeen 1973; Teo 2003 (spherical photon orbits); Chandrasekhar
// 1983. The Kerr shadow underlies EHT spin constraints.

#pragma once

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace blackhole_ds::geodesics::kerr {

inline constexpr double pi = 3.14159265358979323846;

// Equatorial photon-orbit radii (M = 1): prograde (smaller) and retrograde
// (larger). For a = 0 both equal 3M. Closed form (Bardeen, Press, Teukolsky).
[[nodiscard]] inline double photon_orbit_prograde(double a) {
    return 2.0 * (1.0 + std::cos((2.0 / 3.0) * std::acos(-a)));
}
[[nodiscard]] inline double photon_orbit_retrograde(double a) {
    return 2.0 * (1.0 + std::cos((2.0 / 3.0) * std::acos(a)));
}

// Bardeen conserved quantities for a spherical photon orbit of radius r.
[[nodiscard]] inline double bardeen_xi(double r, double a) {
    return -(r * r * r - 3.0 * r * r + a * a * (r + 1.0)) / (a * (r - 1.0));
}
[[nodiscard]] inline double bardeen_eta(double r, double a) {
    const double rm1 = r - 1.0;
    return r * r * r * (4.0 * a * a - r * (r - 3.0) * (r - 3.0)) /
           (a * a * rm1 * rm1);
}

// A point on the apparent image plane (celestial coordinates, units of M).
struct Celestial {
    double alpha; // horizontal (perpendicular to projected spin axis)
    double beta;  // vertical (along projected spin axis)
};

// Build the Kerr shadow boundary as an ordered, closed polygon of image-plane
// points, for spin a in [0, 1) and observer inclination i (degrees). The
// points are angle-sorted around the centroid (the Kerr shadow is convex).
[[nodiscard]] inline std::vector<Celestial>
shadow_boundary(double a, double inclination_deg, int samples = 1200) {
    std::vector<Celestial> pts;
    const double i = inclination_deg * pi / 180.0;
    const double sin_i = std::sin(i);
    const double cos_i = std::cos(i);
    const double cot2 =
        (sin_i > 1e-12) ? (cos_i * cos_i) / (sin_i * sin_i) : 0.0;

    if (a < 1e-6) {
        // Schwarzschild limit: exact circle of radius sqrt(27) M.
        const double R = std::sqrt(27.0);
        for (int k = 0; k < samples; ++k) {
            const double th = 2.0 * pi * k / samples;
            pts.push_back({R * std::cos(th), R * std::sin(th)});
        }
        return pts;
    }

    const double r1 = photon_orbit_prograde(a);
    const double r2 = photon_orbit_retrograde(a);
    const double lo = std::min(r1, r2);
    const double hi = std::max(r1, r2);

    for (int k = 0; k <= samples; ++k) {
        const double r = lo + (hi - lo) * k / samples;
        if (std::abs(r - 1.0) < 1e-9) {
            continue; // avoid the r = M coordinate singularity
        }
        const double xi = bardeen_xi(r, a);
        const double eta = bardeen_eta(r, a);
        const double beta2 = eta + a * a * cos_i * cos_i - xi * xi * cot2;
        if (beta2 < 0.0) {
            continue; // this orbit radius is not on the visible boundary
        }
        const double alpha = -xi / (sin_i > 1e-12 ? sin_i : 1e-12);
        const double beta = std::sqrt(beta2);
        pts.push_back({alpha, beta});
        if (beta > 1e-9) {
            pts.push_back({alpha, -beta});
        }
    }

    if (pts.size() < 3) {
        return pts;
    }

    // Order into a polygon by angle around the centroid (shadow is convex).
    double cx = 0.0, cy = 0.0;
    for (const auto& p : pts) {
        cx += p.alpha;
        cy += p.beta;
    }
    cx /= static_cast<double>(pts.size());
    cy /= static_cast<double>(pts.size());
    std::sort(pts.begin(), pts.end(),
              [cx, cy](const Celestial& p, const Celestial& q) {
                  return std::atan2(p.beta - cy, p.alpha - cx) <
                         std::atan2(q.beta - cy, q.alpha - cx);
              });
    return pts;
}

// Point-in-polygon test (ray casting) for the shadow boundary.
[[nodiscard]] inline bool point_in_shadow(double alpha, double beta,
                                          const std::vector<Celestial>& poly) {
    const std::size_t n = poly.size();
    if (n < 3) {
        return false;
    }
    bool inside = false;
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const double ai = poly[i].alpha, bi = poly[i].beta;
        const double aj = poly[j].alpha, bj = poly[j].beta;
        if (((bi > beta) != (bj > beta)) &&
            (alpha < (aj - ai) * (beta - bi) / (bj - bi) + ai)) {
            inside = !inside;
        }
    }
    return inside;
}

} // namespace blackhole_ds::geodesics::kerr
