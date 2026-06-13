// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/viz/shadow_image.hpp
// The project's first RASTER image: a Schwarzschild black-hole shadow
// encircled by its photon ring, rendered to an RGB buffer.
//
// Physics, and what is honest here:
//   - For a distant observer, an image-plane point at offset (x, y) (units
//     of M) corresponds to a photon of impact parameter b = sqrt(x^2+y^2).
//   - b < b_crit = sqrt(27) M  -> the photon is captured: the SHADOW (black).
//     The shadow radius sqrt(27) M is exact (analytic_classical), reused
//     from the validated geodesic classifier.
//   - b just above b_crit -> the photon skims the photon sphere and is bent
//     through a large (formally diverging) angle before escaping. Light from
//     all around the sky piles up into these directions: the PHOTON RING.
//     We drive ring brightness from the actual integrated deflection angle
//     delta(b) (numerical_approximation), so the bright rim is a monotone
//     function of real geodesic bending, not a painted-on gradient.
//   - b >> b_crit -> delta -> 0: undeflected background (a dim sky floor).
//
// Truth tier of the IMAGE as a whole: visualization_metaphor. The shadow
// radius is analytic_classical and the brightness ordering is derived from
// the numerical deflection, but the colour mapping itself is illustrative,
// not a radiometric (flux) calculation. A real surface-brightness render
// of an accretion disk with Doppler/redshift is later roadmap work (M3/M5).

#pragma once

#include <algorithm>
#include <cmath>

#include "blackhole_ds/geodesics/schwarzschild_photon.hpp"
#include "blackhole_ds/viz/ppm_writer.hpp"

namespace blackhole_ds::viz {

struct ShadowImageView {
    int width = 240;
    int height = 240;
    double half_extent_M = 12.0; // image plane spans [-H, H] in M each axis
};

// Brightness in [0,1] for an escaping photon of impact parameter b, from
// its deflection. Near b_crit the deflection is large (or the integrator
// reports NaN because the ray winds past the phi cap) -> peak brightness.
// Far away the deflection -> 0 -> the dim sky floor.
[[nodiscard]] inline double ring_brightness(double b_over_M) {
    namespace sch = geodesics::schwarzschild;
    const double sky_floor =
        0.06; // faint background so the field is not pure black
    if (b_over_M <= sch::b_critical) {
        return 0.0; // inside the shadow: no escaping light along this ray
    }
    const double delta = sch::light_deflection_angle(b_over_M);
    if (std::isnan(delta)) {
        // Escaping but winds many times: right at the ring -> brightest.
        return 1.0;
    }
    // Map deflection (radians) to brightness. delta ~ pi marks roughly a
    // half-loop; scale so the rim saturates and the glow falls off outward.
    const double bright = delta / sch::pi;
    return std::clamp(std::max(sky_floor, bright), 0.0, 1.0);
}

// Warm (Doppler-agnostic) colour ramp for the photon ring: black -> deep
// red -> orange -> white as brightness rises. Purely illustrative.
[[nodiscard]] inline Rgb ring_colour(double brightness) {
    const double t = std::clamp(brightness, 0.0, 1.0);
    const auto u8 = [](double v) {
        return static_cast<std::uint8_t>(std::clamp(v, 0.0, 1.0) * 255.0 + 0.5);
    };
    // Piecewise: ramp R, then G, then B.
    const double r = std::min(1.0, t * 3.0);
    const double g = std::clamp((t - 0.33) * 3.0, 0.0, 1.0);
    const double b = std::clamp((t - 0.66) * 3.0, 0.0, 1.0);
    return Rgb{u8(r), u8(g), u8(b)};
}

// Render the shadow + photon ring into a new Image.
[[nodiscard]] inline Image render_shadow_image(const ShadowImageView& v = {}) {
    Image img(v.width, v.height);
    const double H = v.half_extent_M;
    const int w = img.width();
    const int h = img.height();
    for (int j = 0; j < h; ++j) {
        // y runs +H (top) to -H (bottom).
        const double y = H - (2.0 * H) * j / (h - 1);
        for (int i = 0; i < w; ++i) {
            const double x = -H + (2.0 * H) * i / (w - 1);
            const double b = std::sqrt(x * x + y * y);
            img.set(i, j, ring_colour(ring_brightness(b)));
        }
    }
    return img;
}

// Measure the shadow radius (in M) from a rendered image by scanning the
// middle row outward for the first non-black pixel. Used to validate the
// render against the analytic sqrt(27) M. Returns the radius in M.
[[nodiscard]] inline double measured_shadow_radius_M(const Image& img,
                                                     double half_extent_M) {
    const int w = img.width();
    const int h = img.height();
    const int cy = h / 2;
    const int cx = w / 2;
    const double px_per_M = (w - 1) / (2.0 * half_extent_M);
    for (int i = cx; i < w; ++i) {
        const Rgb c = img.at(i, cy);
        if (c.r != 0 || c.g != 0 || c.b != 0) {
            return (i - cx) / px_per_M;
        }
    }
    return half_extent_M;
}

} // namespace blackhole_ds::viz
