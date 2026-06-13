// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// blackhole_ds/viz/ascii_shadow.hpp
// The project's first image: an ASCII render of a Schwarzschild black
// hole's shadow.
//
// Physics: for a distant observer, a light ray aimed at image-plane offset
// (x, y) (in units of M) has impact parameter b = sqrt(x^2 + y^2). Rays
// with b < b_crit = sqrt(27) M are captured by the black hole; the set of
// captured directions is the shadow. So the shadow is a disk of radius
// sqrt(27) M ~ 5.196 M in the image plane. This is exact for Schwarzschild
// (the apparent shadow radius equals the critical photon impact parameter).
//
// Truth tiers: the shadow radius b_crit is analytic_classical (it reuses
// the validated geodesic classifier). The ASCII rendering itself is a
// visualization_metaphor -- a silhouette, not a physically-shaded image
// (no accretion disk, lensing of a background, Doppler, or colour yet).

#pragma once

#include <cmath>
#include <ostream>

#include "blackhole_ds/geodesics/schwarzschild_photon.hpp"

namespace blackhole_ds::viz {

// Apparent shadow radius in units of M (= critical photon impact parameter).
[[nodiscard]] inline double shadow_radius_M() {
    return geodesics::schwarzschild::b_critical;
}

// Is the image-plane point (x, y) (in units of M) inside the shadow?
[[nodiscard]] inline bool in_shadow(double x_M, double y_M) {
    const double b = std::sqrt(x_M * x_M + y_M * y_M);
    return geodesics::schwarzschild::classify(b) ==
           geodesics::schwarzschild::RayFate::Captured;
}

struct ShadowView {
    int width = 70;              // columns
    int height = 35;             // rows (about width/2 so the disk looks round
                                 // in a terminal, where cells are ~2x tall)
    double half_extent_M = 10.0; // image plane spans [-H, H] in M on each axis
    char shadow_char = ' ';      // inside the shadow (the dark disk)
    char ring_char = '@';        // the rim, within ~3% of b_crit
    char sky_char = '.';         // escaping rays (background)
};

// Render the shadow as ASCII rows into `out`. Each cell maps to an
// image-plane point; the rim (just outside the captured region, within 3%
// of b_crit) is drawn with ring_char so the photon ring is visible.
inline void render_shadow(std::ostream& out, const ShadowView& v = {}) {
    const double b_crit = geodesics::schwarzschild::b_critical;
    const int w = v.width > 1 ? v.width : 2;
    const int h = v.height > 1 ? v.height : 2;
    const double H = v.half_extent_M;

    for (int j = 0; j < h; ++j) {
        const double y = H - (2.0 * H) * j / (h - 1);
        for (int i = 0; i < w; ++i) {
            const double x = -H + (2.0 * H) * i / (w - 1);
            const double b = std::sqrt(x * x + y * y);
            char c;
            if (b < b_crit) {
                c = v.shadow_char;
            } else if (b < 1.03 * b_crit) {
                c = v.ring_char; // photon-ring rim
            } else {
                c = v.sky_char;
            }
            out << c;
        }
        out << '\n';
    }
}

} // namespace blackhole_ds::viz
