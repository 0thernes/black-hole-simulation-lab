// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/viz/kerr_disk_image.hpp
// Roadmap M5: the gravitationally lensed accretion disk around a SPINNING
// (Kerr) black hole -- the full frame-dragged image, by backward ray tracing
// real Kerr null geodesics.
//
// Where viz/disk_image.hpp traces the planar Schwarzschild orbit equation,
// this traces the genuine 3-D Kerr geodesic (geodesics/kerr_geodesic.hpp): a
// Kerr photon does not stay in a plane, so frame dragging twists the lensed
// disk and shifts the inner shadow off-centre. The pipeline per pixel:
//
//   1. (alpha, beta) image coords -> constants (E, L_z, Q)   [Cunningham-
//      Bardeen, constants_from_image]
//   2. build the on-shell ingoing photon at a far observer radius
//   3. integrate inward in Mino time (adaptive step), watching for:
//        - the equatorial plane theta = pi/2 (a disk crossing) with the
//          radius in [r_in, r_out]  -> the disk (first crossing = front,
//          later crossings = higher-order lensed images);
//        - r falling to the horizon r_+ = 1 + sqrt(1 - a^2)  -> the shadow;
//        - the ray turning around and escaping back out      -> background.
//   4. colour a disk hit by the KERR circular-orbit redshift factor g, applied
//      as g^4 beaming + a temperature shift.
//
// Correctness anchor: at a = 0 every piece reduces to the Schwarzschild disk
// -- the Kerr redshift factor reduces to viz/disk_image.hpp's redshift_factor
// (verified algebraically and in tests/kerr_disk_tests.cpp), and the traced
// hit map matches the Schwarzschild tracer on sample pixels.
//
// Truth tiers: the lensing GEOMETRY and the trajectory are
// numerical_approximation (RK4-integrated first-order on-shell geodesic).
// Correctness is validated by the a -> 0 regression against the Schwarzschild
// tracer in tests/kerr_disk_tests.cpp; the first-order scheme is pinned to the
// constraint surface by construction (it recomputes sqrt(R), sqrt(Theta) from
// the current position each step), so it does not accumulate the off-shell
// drift the second-order form would -- but note that the conserved-quantity-
// drift tests (recover_Q etc.) live in kerr_geodesic_tests.cpp and exercise the
// SECOND-order integrator, not this one. The redshift factor g is
// analytic_classical (exact GR for a circular-geodesic emitter). The shadow
// here is determined by NUMERICAL horizon capture (r <= r_+ * 1.02, a ~2%
// band) inside the approximate integrator -- numerical_approximation, distinct
// from the exact closed-form Bardeen-1973 shadow boundary in kerr_shadow.hpp.
// The intrinsic emissivity profile and colour ramp remain a
// visualization_metaphor.
// References: Bardeen 1973; Cunningham 1975; Luminet 1979; Gralla-Lupsasca
// 2020 (Mino-time geodesics).

#pragma once

#include <algorithm>
#include <cmath>

#include "blackhole_ds/geodesics/kerr_geodesic.hpp"
#include "blackhole_ds/integrators/rk4.hpp"
#include "blackhole_ds/viz/ppm_writer.hpp"

namespace blackhole_ds::viz {

struct KerrDiskView {
    int width = 400;
    int height = 400;
    double half_extent_M = 14.0;   // image plane spans [-H, H] in M each axis
    double spin = 0.9;             // a/M in [0, 1)
    double inclination_deg = 78.0; // 0 = face-on, ~90 = edge-on
    double r_in_M = 6.0;           // disk inner edge
    double r_out_M = 18.0;         // disk outer edge
    int max_crossings = 4;         // image orders to consider per ray
    double r_observer = 1000.0;    // where the backward ray starts
    int max_steps = 6000;          // integration step cap per ray
    double target_step = 0.04;     // controlled coordinate move per step (M)
    int samples = 1;               // supersampling: samples x samples per pixel
};

enum class KerrHitKind { Disk, Shadow, Background };

struct KerrPixelHit {
    KerrHitKind kind = KerrHitKind::Background;
    double r_disk = 0.0; // disk radius (M) if kind == Disk
    int order = 0;       // image order (0 = direct/front)
    double b = 0.0;      // photon L_z/E (impact param) for the redshift
};

// Kerr circular-geodesic redshift factor g = nu_obs / nu_em for a prograde
// equatorial emitter at radius r, photon impact parameter b = L_z/E, observer
// at infinity (M = 1):
//
//     Omega = 1 / (r^{3/2} + a)                       (orbital angular vel.)
//     u^t   = (r^{3/2} + a) / sqrt(r^3 - 3 r^2 + 2 a r^{3/2})
//     g     = 1 / [ u^t (1 - Omega b) ]
//
// Reduces to sqrt(1 - 3/r) / (1 - r^{-3/2} b) at a = 0, i.e. exactly
// disk_image.hpp's redshift_factor with b = -X sin i (Cunningham-Bardeen).
// Truth tier: analytic_classical.
[[nodiscard]] inline double kerr_redshift_factor(double r, double b, double a) {
    const double r15 = std::pow(r, 1.5);
    const double ut_denom = r * r * r - 3.0 * r * r + 2.0 * a * r15;
    if (ut_denom <= 0.0) {
        return 0.0; // no circular geodesic emitter here
    }
    const double Omega = 1.0 / (r15 + a);
    const double ut = (r15 + a) / std::sqrt(ut_denom);
    double denom = ut * (1.0 - Omega * b);
    if (denom < 0.05) {
        denom = 0.05; // guard the extreme blueshifted edge (rendering only)
    }
    return 1.0 / denom;
}

// Trace one image-plane point (alpha, beta) in units of M through the Kerr
// geometry and classify what it sees.
//
// We integrate the FIRST-ORDER on-shell Mino-time form: dr/dlambda =
// s_r sqrt(R(r)) and dtheta/dlambda = s_theta sqrt(Theta(theta)), recomputing
// the speeds from the current position every step so the trajectory can never
// drift off the constraint surface. (Integrating the second-order
// accelerations across the huge r-range from the observer lets dr/dlambda
// drift through zero and spuriously turn an infalling ray around.) The
// direction signs s_r, s_theta flip only at genuine turning points, detected
// when R or Theta crosses zero. The azimuth phi is not needed for
// classification, so we evolve only (r, theta).
[[nodiscard]] inline KerrPixelHit trace_kerr_pixel(double alpha, double beta,
                                                   const KerrDiskView& v) {
    namespace kerr = geodesics::kerr;
    using integrators::rk4_step;
    using integrators::State;

    const double a = v.spin;
    const kerr::GeodesicConstants k =
        kerr::constants_from_image(alpha, beta, v.inclination_deg, a);
    const double r_plus = 1.0 + std::sqrt(std::max(0.0, 1.0 - a * a));
    const double half_pi = kerr::kg_pi / 2.0;
    const double b = k.Lz / k.E;

    const double i_rad = v.inclination_deg * kerr::kg_pi / 180.0;
    double sr = -1.0;                       // ingoing
    double sth = (beta > 0.0) ? 1.0 : -1.0; // initial polar direction
    double th0 = i_rad;
    if (std::abs(beta) < 1.0e-3) {
        // beta ~ 0 starts exactly on a polar turning point (Theta(i) = beta^2),
        // where the first-order speed sqrt(Theta) vanishes and the ray would
        // stall. The allowed region is toward the equatorial plane (Theta
        // grows that way for L_z != 0); nudge off the turning point so the
        // motion proceeds toward the disk.
        sth = (half_pi > i_rad) ? 1.0 : -1.0;
        th0 = i_rad + sth * 1.0e-3;
    }

    // On-shell speed RHS for y = [r, theta], with the current direction signs.
    const auto deriv = [&](double, const State<2>& s) -> State<2> {
        const double R = kerr::kg_R(s[0], k);
        const double Th = kerr::kg_Theta(s[1], k);
        return State<2>{sr * std::sqrt(R > 0.0 ? R : 0.0),
                        sth * std::sqrt(Th > 0.0 ? Th : 0.0)};
    };

    State<2> y{v.r_observer, th0};
    double s_prev = y[1] - half_pi; // signed distance to the equatorial plane
    int order = 0;

    for (int step = 0; step < v.max_steps; ++step) {
        const double r = y[0];
        // Adaptive Mino step: bound the coordinate move to ~target_step near
        // the hole, bigger strides far out. Floor the speed so the step cannot
        // blow up near a turning point (where sqrt(R) -> 0).
        const State<2> d = deriv(0.0, y);
        const double speed = std::max({std::abs(d[0]), std::abs(d[1]), 1.0});
        const double tgt = (r > 2.0 * v.r_out_M)
                               ? std::clamp(0.05 * r, v.target_step, 8.0)
                               : v.target_step;
        const double h = std::clamp(tgt / speed, 1.0e-9, 8.0);

        State<2> yn = rk4_step<2>(deriv, 0.0, y, h);

        // Radial turning point: R < 0 means we overshot it -> reflect + flip.
        if (kerr::kg_R(yn[0], k) < 0.0) {
            sr = -sr;
            yn[0] = r;
        }
        // Polar turning point.
        if (kerr::kg_Theta(yn[1], k) < 0.0) {
            sth = -sth;
            yn[1] = y[1];
        }

        // Captured by the hole.
        if (yn[0] <= r_plus * 1.02) {
            return KerrPixelHit{KerrHitKind::Shadow, 0.0, order, b};
        }
        // Outgoing and back past the observer: escaped to the background.
        if (sr > 0.0 && yn[0] > v.r_observer) {
            return KerrPixelHit{KerrHitKind::Background, 0.0, order, b};
        }

        // Equatorial-plane (disk) crossing between this step and the last.
        const double s_n = yn[1] - half_pi;
        if ((s_prev > 0.0) != (s_n > 0.0)) {
            const double f = s_prev / (s_prev - s_n); // in [0, 1]
            const double rc = r + f * (yn[0] - r);
            if (rc >= v.r_in_M && rc <= v.r_out_M) {
                return KerrPixelHit{KerrHitKind::Disk, rc, order, b};
            }
            ++order;
            if (order > v.max_crossings) {
                return KerrPixelHit{KerrHitKind::Background, 0.0, order, b};
            }
        }

        s_prev = s_n;
        y = yn;
    }
    return KerrPixelHit{KerrHitKind::Background, 0.0, order, b};
}

// Warm colour for a Kerr disk hit. Mirrors disk_image.hpp's disk_colour but
// takes the precomputed Kerr redshift factor g: brightness falls with radius
// (illustrative), times g^4 (beaming); colour temperature shifted by g. The
// emissivity/colour ramp is a visualization_metaphor; g is exact GR.
[[nodiscard]] inline Rgb kerr_disk_colour(double r, int order, double g,
                                          double inner) {
    double emiss = std::pow(std::clamp(inner / r, 0.0, 1.0), 3.0);
    if (order > 0) {
        emiss *= 0.45 / static_cast<double>(order);
    }
    const double lum = std::clamp(emiss * std::pow(g, 4.0), 0.0, 1.0);
    const double t = std::clamp((inner / r) * g, 0.0, 1.0);
    const auto u8 = [](double c) {
        return static_cast<std::uint8_t>(std::clamp(c, 0.0, 1.0) * 255.0 + 0.5);
    };
    return Rgb{u8(1.0 * lum), u8((0.45 + 0.55 * t) * lum),
               u8(0.15 * t * t * lum)};
}

// Colour of a single image-plane sample at (alpha, beta) in units of M.
[[nodiscard]] inline Rgb kerr_sample_colour(double alpha, double beta,
                                            const KerrDiskView& v) {
    static const Rgb background{4, 4, 9};
    const KerrPixelHit hit = trace_kerr_pixel(alpha, beta, v);
    switch (hit.kind) {
    case KerrHitKind::Disk: {
        const double g = kerr_redshift_factor(hit.r_disk, hit.b, v.spin);
        return kerr_disk_colour(hit.r_disk, hit.order, g, v.r_in_M);
    }
    case KerrHitKind::Shadow:
        return Rgb{0, 0, 0};
    case KerrHitKind::Background:
    default:
        return background;
    }
}

// Render the frame-dragged Kerr lensed disk + shadow into a new Image,
// averaging v.samples x v.samples sub-pixel rays. NOTE: this integrates a real
// geodesic per ray and is far heavier than the Schwarzschild render.
[[nodiscard]] inline Image render_kerr_disk_image(const KerrDiskView& v = {}) {
    Image img(v.width, v.height);
    const double H = v.half_extent_M;
    const int w = img.width();
    const int h = img.height();
    const int ss = v.samples > 0 ? v.samples : 1;

    const double dx = (2.0 * H) / (w - 1);
    const double dy = (2.0 * H) / (h - 1);
    const int n = ss * ss;

    for (int j = 0; j < h; ++j) {
        const double Yc = H - dy * j;
        for (int i = 0; i < w; ++i) {
            const double Xc = -H + dx * i;
            double rsum = 0.0, gsum = 0.0, bsum = 0.0;
            for (int sy = 0; sy < ss; ++sy) {
                const double oy = (sy + 0.5) / ss - 0.5;
                for (int sx = 0; sx < ss; ++sx) {
                    const double ox = (sx + 0.5) / ss - 0.5;
                    const Rgb c =
                        kerr_sample_colour(Xc + ox * dx, Yc - oy * dy, v);
                    rsum += c.r;
                    gsum += c.g;
                    bsum += c.b;
                }
            }
            const auto avg = [n](double s) {
                return static_cast<std::uint8_t>(s / n + 0.5);
            };
            img.set(i, j, Rgb{avg(rsum), avg(gsum), avg(bsum)});
        }
    }
    return img;
}

} // namespace blackhole_ds::viz
