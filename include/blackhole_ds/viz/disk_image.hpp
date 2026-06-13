// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// blackhole_ds/viz/disk_image.hpp
// Roadmap M3: the iconic image -- a gravitationally lensed thin accretion
// disk around a Schwarzschild black hole, by backward ray tracing.
//
// Geometry (this is the physics that must be right):
//   Black hole at the origin; the accretion disk is a thin annulus in the
//   equatorial plane (theta = pi/2), r_in <= r <= r_out. A distant observer
//   sits at inclination i from the disk axis (i = 0 face-on, i -> 90 deg
//   edge-on). The unit vector to the observer is o = (sin i, 0, cos i).
//   The image plane uses axes e_X = (1, 0, 0) (horizontal) and
//   e_Y = (0, cos i, -sin i) (vertical, the tilt direction). A pixel at
//   image coordinates (X, Y) in units of M corresponds to a photon of
//   impact parameter b = sqrt(X^2 + Y^2). Tilting along Y puts the
//   characteristic lensing "warp" along the vertical, matching the
//   conventional Interstellar/EHT orientation.
//
//   Every Schwarzschild photon orbit is planar (the orbit plane contains the
//   origin and the line of sight). As the photon sweeps azimuth phi in its
//   orbit plane -- the SAME phi as the orbit equation d^2u/dphi^2 + u = 3u^2
//   -- the z-component of its radius-vector direction is
//       g(phi) = cos(phi) cos i  -  sin(phi) (Y/b) sin i.
//   The photon crosses the equatorial (disk) plane wherever g(phi) = 0. At
//   each crossing we read the photon radius r = 1/u(phi); if it lies in
//   [r_in, r_out], the pixel sees the disk at that radius. The first crossing
//   is the front of the disk; later crossings are higher-order (lensed)
//   images -- this is what bends the far side of the disk up over the hole
//   and produces the famous photon ring / "warped disk" look.
//
//   Reductions that make this checkable:
//     - i = 0 (face-on): g(phi) = cos(phi), independent of pixel azimuth, so
//       the image is exactly rotationally symmetric. (Tested.)
//     - i != 0: g depends on Y but not the sign of X, so the image is
//       mirror-symmetric left<->right and asymmetric top<->bottom. (Tested.)
//     - A ray with b < sqrt(27) M that reaches u = 1/2 (r = 2M) before any
//       disk crossing is captured: the SHADOW. (Tested.)
//
// Truth tiers: the lensing GEOMETRY (which disk radius each pixel sees) is a
// numerical_approximation built on the validated geodesic; the shadow radius
// is analytic_classical. The relativistic redshift factor g (gravitational +
// orbital time dilation + Doppler) is an exact GR formula (analytic_classical;
// see redshift_factor below), applied as g^4 beaming. The intrinsic
// emissivity profile and colour ramp remain a visualization_metaphor (not a
// radiometric surface-brightness model). Kerr spin is not modelled yet (M5).
//
// References (analytic_classical): Luminet 1979 (first thin-disk BH image);
// Cunningham 1975 (redshift factor); Misner-Thorne-Wheeler 1973 ch. 25.

#pragma once

#include <algorithm>
#include <cmath>

#include "blackhole_ds/geodesics/schwarzschild_photon.hpp"
#include "blackhole_ds/integrators/ode_state.hpp"
#include "blackhole_ds/integrators/rk4.hpp"
#include "blackhole_ds/viz/ppm_writer.hpp"

namespace blackhole_ds::viz {

struct DiskView {
    int width = 400;
    int height = 400;
    double half_extent_M = 14.0;   // image plane spans [-H, H] in M each axis
    double inclination_deg = 78.0; // 0 = face-on, ~90 = edge-on
    double r_in_M = 6.0;           // disk inner edge (ISCO for Schwarzschild)
    double r_out_M = 18.0;         // disk outer edge
    int max_crossings = 4;         // image orders to consider per ray
    double dphi = 0.01;            // orbit-angle step (rad)
    double phi_cap = 6.0 * 3.141592653589793; // ~3 full loops
};

enum class HitKind { Disk, Shadow, Background };

struct PixelHit {
    HitKind kind = HitKind::Background;
    double r_disk = 0.0; // disk radius (M) if kind == Disk
    int order = 0;       // image order (0 = direct/front)
};

// Trace one image-plane point (X, Y) in units of M.
[[nodiscard]] inline PixelHit trace_disk_pixel(double X, double Y,
                                               const DiskView& v) {
    using integrators::rk4_step;
    using integrators::State;

    const double b = std::sqrt(X * X + Y * Y);
    const double incl = v.inclination_deg * 3.141592653589793 / 180.0;
    const double cosi = std::cos(incl);
    const double sini = std::sin(incl);

    if (b < 1.0e-9) {
        // Central ray straight down the axis: b = 0 < b_crit -> captured.
        return PixelHit{HitKind::Shadow, 0.0, 0};
    }

    // z-component of the photon's radius-vector direction at orbit angle phi.
    // The disk is tilted about the horizontal (X) axis, so Y is the tilt
    // direction and the lensing warp runs vertically (canonical orientation).
    const double yb = Y / b;
    const auto g = [&](double phi) {
        return std::cos(phi) * cosi - std::sin(phi) * yb * sini;
    };

    // Orbit ODE: y = [u, du/dphi], u'' = 3u^2 - u.
    const auto deriv = [](double /*phi*/, const State<2>& y) -> State<2> {
        return State<2>{y[1], 3.0 * y[0] * y[0] - y[0]};
    };

    const double u0 = 1.0e-6; // start "at infinity" (the observer)
    const double disc = 1.0 / (b * b) - u0 * u0 + 2.0 * u0 * u0 * u0;
    State<2> y{u0, std::sqrt(std::max(0.0, disc))}; // incoming (u increasing)

    double phi = 0.0;
    double gprev = g(0.0);
    bool past_peri = false;
    int order = 0;
    const int max_steps = static_cast<int>(v.phi_cap / v.dphi);

    for (int s = 0; s < max_steps; ++s) {
        const State<2> yn = rk4_step<2>(deriv, phi, y, v.dphi);
        const double phin = phi + v.dphi;

        if (yn[0] >= 0.5) {
            // Reached the horizon (r = 2M) before escaping: captured.
            return PixelHit{HitKind::Shadow, 0.0, order};
        }

        const double gn = g(phin);
        if ((gprev > 0.0) != (gn > 0.0)) {
            // Equatorial-plane crossing between phi and phin; interpolate.
            const double f = gprev / (gprev - gn); // in [0,1]
            const double uc = y[0] + f * (yn[0] - y[0]);
            if (uc > 0.0) {
                const double rc = 1.0 / uc;
                if (rc >= v.r_in_M && rc <= v.r_out_M) {
                    return PixelHit{HitKind::Disk, rc, order};
                }
            }
            ++order;
            if (order > v.max_crossings) {
                return PixelHit{HitKind::Background, 0.0, order};
            }
        }

        if (yn[1] < 0.0) {
            past_peri = true;
        }
        if (past_peri && yn[0] <= u0) {
            // Returned to infinity without hitting the disk: background.
            return PixelHit{HitKind::Background, 0.0, order};
        }

        gprev = gn;
        y = yn;
        phi = phin;
    }
    return PixelHit{HitKind::Background, 0.0, order};
}

// Relativistic redshift factor g = nu_observed / nu_emitted for material on
// a circular geodesic orbit at radius r in Schwarzschild, seen at observer
// inclination i, for a photon whose conserved axial angular momentum maps to
// image coordinate X (the horizontal celestial coordinate, perpendicular to
// the projected spin axis):
//
//     g = sqrt(1 - 3M/r) / (1 + r^{-3/2} X sin i)        (M = 1)
//
//   - Numerator sqrt(1 - 3M/r): combined gravitational + orbital time
//     dilation of a circular-geodesic emitter (1/u^t). Real for r > 3M; the
//     disk inner edge r_in >= 6M keeps it well-defined.
//   - Denominator: the longitudinal Doppler shift from the orbital velocity.
//     X sin i = -L_z/E is the Cunningham-Bardeen relation between the
//     conserved axial angular momentum and the image coordinate, so one side
//     of the disk (material moving toward the observer) is blueshifted
//     (g > 1) and the other redshifted (g < 1). This is the famous
//     brightness asymmetry, and it vanishes at i = 0 (face-on) as it must.
//
// Observed specific intensity transforms as I_obs = g^3 I_em (Liouville's
// theorem; I_nu/nu^3 is invariant), so bolometric flux scales as g^4.
//
// Truth tier: analytic_classical formula (the redshift factor is exact GR);
// applied to a visualization_metaphor emissivity model below.
// Reference: Cunningham 1975; Luminet 1979; MTW 1973.
[[nodiscard]] inline double redshift_factor(double r, double X, double sin_i) {
    if (r <= 3.0) {
        return 0.0; // no stable circular orbit emission inside 3M
    }
    const double numer = std::sqrt(1.0 - 3.0 / r);
    double denom = 1.0 + std::pow(r, -1.5) * X * sin_i;
    if (denom < 0.05) {
        denom = 0.05; // guard the extreme blueshifted edge (rendering only)
    }
    return numer / denom;
}

// Warm colour for a disk hit, now including relativistic beaming and the
// frequency shift. Brightness falls with radius (intrinsic, illustrative)
// and is multiplied by g^4 (beaming); the colour temperature is shifted by
// g (blueshift -> hotter/whiter, redshift -> redder). The emissivity profile
// and colour ramp are a visualization_metaphor; the g factor is exact GR.
[[nodiscard]] inline Rgb disk_colour(double r, int order, double X,
                                     double sin_i, const DiskView& v) {
    const double inner = v.r_in_M;
    const double g = redshift_factor(r, X, sin_i);

    double emiss = std::pow(std::clamp(inner / r, 0.0, 1.0), 3.0); // falloff
    if (order > 0) {
        emiss *= 0.45 / static_cast<double>(order); // higher orders dimmer
    }
    double lum = std::clamp(emiss * std::pow(g, 4.0), 0.0, 1.0); // beaming g^4

    const double t = std::clamp((inner / r) * g, 0.0, 1.0); // shifted temp
    const double R = 1.0;
    const double G = 0.45 + 0.55 * t;
    const double B = 0.15 * t * t;
    const auto u8 = [](double c) {
        return static_cast<std::uint8_t>(std::clamp(c, 0.0, 1.0) * 255.0 + 0.5);
    };
    return Rgb{u8(R * lum), u8(G * lum), u8(B * lum)};
}

// Render the lensed disk + shadow into a new Image.
[[nodiscard]] inline Image render_disk_image(const DiskView& v = {}) {
    Image img(v.width, v.height);
    const double H = v.half_extent_M;
    const int w = img.width();
    const int h = img.height();
    const Rgb background{4, 4, 9}; // near-black sky floor
    const double sin_i =
        std::sin(v.inclination_deg * 3.141592653589793 / 180.0);

    for (int j = 0; j < h; ++j) {
        const double Y = H - (2.0 * H) * j / (h - 1);
        for (int i = 0; i < w; ++i) {
            const double X = -H + (2.0 * H) * i / (w - 1);
            const PixelHit hit = trace_disk_pixel(X, Y, v);
            switch (hit.kind) {
            case HitKind::Disk:
                img.set(i, j, disk_colour(hit.r_disk, hit.order, X, sin_i, v));
                break;
            case HitKind::Shadow:
                img.set(i, j, Rgb{0, 0, 0});
                break;
            case HitKind::Background:
                img.set(i, j, background);
                break;
            }
        }
    }
    return img;
}

} // namespace blackhole_ds::viz
