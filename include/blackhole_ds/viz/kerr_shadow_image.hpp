// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/viz/kerr_shadow_image.hpp
// Renders the asymmetric Kerr (spinning black hole) shadow to a raster
// image, using the closed-form Bardeen 1973 boundary in
// geodesics/kerr_shadow.hpp.
//
// The shadow itself (the D-shaped silhouette) is analytic_classical. A thin
// bright rim is drawn just outside the boundary to suggest the photon ring
// (a visualization_metaphor). No accretion disk or Doppler here -- this is
// the bare spin-shadow, the cleanest demonstration of the spin asymmetry.

#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "blackhole_ds/geodesics/kerr_shadow.hpp"
#include "blackhole_ds/viz/ppm_writer.hpp"

namespace blackhole_ds::viz {

struct KerrShadowView {
    int width = 400;
    int height = 400;
    double half_extent_M = 9.0;    // image spans [-H, H] in M each axis
    double spin = 0.99;            // a/M in [0, 1)
    double inclination_deg = 80.0; // observer inclination
};

// Signed-distance-ish helper: minimum distance from a point to the boundary
// polygon (used to draw a thin photon-ring rim outside the shadow).
[[nodiscard]] inline double
distance_to_boundary(double alpha, double beta,
                     const std::vector<geodesics::kerr::Celestial>& poly) {
    double best = 1e300;
    const std::size_t n = poly.size();
    for (std::size_t i = 0, j = n - 1; i < n; j = i++) {
        const double ax = poly[j].alpha, ay = poly[j].beta;
        const double bx = poly[i].alpha, by = poly[i].beta;
        const double dx = bx - ax, dy = by - ay;
        const double len2 = dx * dx + dy * dy;
        double t =
            (len2 > 0.0) ? ((alpha - ax) * dx + (beta - ay) * dy) / len2 : 0.0;
        t = std::clamp(t, 0.0, 1.0);
        const double px = ax + t * dx, py = ay + t * dy;
        const double d = std::hypot(alpha - px, beta - py);
        best = std::min(best, d);
    }
    return best;
}

[[nodiscard]] inline Image render_kerr_shadow(const KerrShadowView& v = {}) {
    Image img(v.width, v.height);
    const double H = v.half_extent_M;
    const int w = img.width();
    const int h = img.height();
    const auto poly =
        geodesics::kerr::shadow_boundary(v.spin, v.inclination_deg);
    const Rgb background{6, 6, 12};
    const double rim = 0.18; // photon-ring rim half-width in M

    for (int j = 0; j < h; ++j) {
        const double beta = H - (2.0 * H) * j / (h - 1);
        for (int i = 0; i < w; ++i) {
            const double alpha = -H + (2.0 * H) * i / (w - 1);
            if (geodesics::kerr::point_in_shadow(alpha, beta, poly)) {
                img.set(i, j, Rgb{0, 0, 0}); // the shadow
            } else {
                const double d = distance_to_boundary(alpha, beta, poly);
                if (d < rim) {
                    const double t = 1.0 - d / rim; // bright at the edge
                    const auto u8 = [](double c) {
                        return static_cast<std::uint8_t>(
                            std::clamp(c, 0.0, 1.0) * 255.0 + 0.5);
                    };
                    img.set(
                        i, j,
                        Rgb{u8(0.6 + 0.4 * t), u8(0.5 + 0.5 * t), u8(0.2 * t)});
                } else {
                    img.set(i, j, background);
                }
            }
        }
    }
    return img;
}

// Measure shadow extent along the alpha (horizontal) axis at beta = 0:
// returns {alpha_min, alpha_max}. For a = 0 this is symmetric about 0
// (= +/- sqrt(27)); for a > 0 it is displaced/flattened (the spin signature).
[[nodiscard]] inline std::pair<double, double>
shadow_alpha_extent(const std::vector<geodesics::kerr::Celestial>& poly) {
    double amin = 1e300, amax = -1e300;
    for (const auto& p : poly) {
        amin = std::min(amin, p.alpha);
        amax = std::max(amax, p.alpha);
    }
    return {amin, amax};
}

} // namespace blackhole_ds::viz
