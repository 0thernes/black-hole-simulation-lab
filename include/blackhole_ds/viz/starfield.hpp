// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/viz/starfield.hpp
// Celestial-sphere background for the lensed Schwarzschild disk.
//
// A ray that misses the disk and the horizon escapes to infinity. The
// azimuth it has swept, phi_escape, is the angle between the two
// asymptotes of that null geodesic. A straight ray sweeps pi; the extra
// angle delta = phi_escape - pi is the gravitational deflection, the same
// quantity light_deflection_angle reports. The star the pixel shows is the
// one that lies in the direction obtained by rotating the camera's look
// direction through delta inside the ray's orbital plane.
//
// Truth tiers:
//   - delta comes from the integrated Schwarzschild orbit
//     (numerical_approximation). At large impact parameter it falls toward
//     the Eddington 4M/b bend; near sqrt(27) M it grows without bound.
//   - The catalog, the star colours, and the faint band are a
//     visualization_metaphor. They are a fixed synthetic sky, not Gaia
//     and not a surface-brightness model. The plate scale uses a finite
//     camera distance only to spread that sky across the image; the disk
//     intersection itself is still the distant-observer geodesic.
//
// Kerr frame dragging is not applied here. The Kerr disk tracer does not
// yet return an escape direction, so its background stays the dark floor.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include "blackhole_ds/viz/ppm_writer.hpp"

namespace blackhole_ds::viz {

struct Vec3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

[[nodiscard]] inline Vec3 operator+(Vec3 a, Vec3 b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
[[nodiscard]] inline Vec3 operator-(Vec3 a, Vec3 b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
[[nodiscard]] inline Vec3 operator*(Vec3 a, double s) {
    return {a.x * s, a.y * s, a.z * s};
}
[[nodiscard]] inline double dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
[[nodiscard]] inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}
[[nodiscard]] inline Vec3 normalize(Vec3 a) {
    const double n = std::sqrt(dot(a, a));
    return a * (1.0 / n);
}

inline constexpr double star_pi = 3.14159265358979323846;

// Fixed anchor so tests can hit a star without depending on the random
// catalog. Direction is shared with the catalog entry of the same vector.
[[nodiscard]] inline Vec3 anchor_star_direction() {
    return normalize(Vec3{1.0, 1.0, 1.0});
}

// Camera basis. n0 points from the hole toward the observer. e_h is the
// image +X axis (horizontal). e_v is the image +Y axis (the tilt), and it
// is perpendicular to n0. This matches disk_image.hpp: Y enters g(phi), X
// does not.
[[nodiscard]] inline Vec3 observer_axis(double incl_rad) {
    return {std::sin(incl_rad), 0.0, std::cos(incl_rad)};
}
[[nodiscard]] inline Vec3 image_axis_h() {
    return {0.0, 1.0, 0.0};
}
[[nodiscard]] inline Vec3 image_axis_v(double incl_rad) {
    return {std::cos(incl_rad), 0.0, -std::sin(incl_rad)};
}

// Unit tangent in the orbital plane at phi = 0, perpendicular to n0, with
// the sign that reproduces disk_image.hpp's g(phi).
[[nodiscard]] inline Vec3 orbit_tangent(double X, double Y, double incl_rad) {
    const double b = std::sqrt(X * X + Y * Y);
    if (b < 1.0e-12) {
        return image_axis_h();
    }
    const double cosi = std::cos(incl_rad);
    const double sini = std::sin(incl_rad);
    return {(Y / b) * cosi, -X / b, -(Y / b) * sini};
}

// Rodrigues rotation of v about a unit axis by ang radians.
[[nodiscard]] inline Vec3 rotate_about(Vec3 v, Vec3 axis, double ang) {
    const Vec3 k = normalize(axis);
    const double c = std::cos(ang);
    const double s = std::sin(ang);
    return v * c + cross(k, v) * s + k * (dot(k, v) * (1.0 - c));
}

// Direction on the celestial sphere for a ray that escaped at phi_escape.
// delta = 0 leaves the camera look direction unchanged, so a weakly bent
// ray shows the star the pixel is aimed at. A large delta, near the photon
// sphere, swings that aim around the orbital plane.
[[nodiscard]] inline Vec3 star_source_direction(double X, double Y,
                                                double phi_escape,
                                                double inclination_deg,
                                                double camera_distance_M) {
    const double incl = inclination_deg * star_pi / 180.0;
    const Vec3 n0 = observer_axis(incl);
    const double r_cam = camera_distance_M > 1.0 ? camera_distance_M : 1.0;
    const Vec3 cam = n0 * r_cam;
    const Vec3 target = image_axis_h() * X + image_axis_v(incl) * Y;
    const Vec3 look = normalize(target - cam);
    const Vec3 tangent = orbit_tangent(X, Y, incl);
    const Vec3 normal = cross(n0, tangent);
    const double delta = phi_escape - star_pi;
    if (std::abs(delta) < 1.0e-12 || dot(normal, normal) < 1.0e-18) {
        return look;
    }
    return rotate_about(look, normal, delta);
}

struct Star {
    Vec3 dir;
    double brightness = 0.0;
    double temperature = 0.5; // 0 red, 1 blue
    double radius = 0.008;    // angular radius, radians
};

struct StarGrid {
    static constexpr int nlat = 32;
    static constexpr int nlon = 64;
    std::vector<Star> stars;
    std::vector<std::vector<int>> bins;
};

[[nodiscard]] inline int star_bin(Vec3 d) {
    const double z = std::clamp(d.z, -1.0, 1.0);
    int lat = static_cast<int>((z + 1.0) * 0.5 * StarGrid::nlat);
    if (lat >= StarGrid::nlat) {
        lat = StarGrid::nlat - 1;
    }
    if (lat < 0) {
        lat = 0;
    }
    double phi = std::atan2(d.y, d.x);
    int lon =
        static_cast<int>((phi + star_pi) / (2.0 * star_pi) * StarGrid::nlon);
    if (lon >= StarGrid::nlon) {
        lon = 0;
    }
    if (lon < 0) {
        lon = StarGrid::nlon - 1;
    }
    return lat * StarGrid::nlon + lon;
}

[[nodiscard]] inline const StarGrid& star_grid() {
    static const StarGrid grid = [] {
        StarGrid g;
        g.stars.reserve(2200);
        std::uint32_t state = 0x5A17F1EDu;
        const auto rnd = [&]() {
            state = state * 1664525u + 1013904223u;
            return static_cast<double>(state >> 8) / 16777216.0;
        };
        for (int i = 0; i < 2000; ++i) {
            const double z = rnd() * 2.0 - 1.0;
            const double th = rnd() * 2.0 * star_pi;
            const double rho = std::sqrt(std::max(0.0, 1.0 - z * z));
            Star s;
            s.dir = {rho * std::cos(th), rho * std::sin(th), z};
            // Mostly faint, a few bright. Radius grows with brightness so
            // a bright star covers more than one pixel at the default plate
            // scale.
            const double mag = std::pow(rnd(), 3.0);
            s.brightness = 0.25 + 0.75 * mag;
            s.temperature = rnd();
            s.radius = 0.0035 + 0.010 * s.brightness;
            g.stars.push_back(s);
        }
        Star anchor;
        anchor.dir = anchor_star_direction();
        anchor.brightness = 1.0;
        anchor.temperature = 0.72;
        anchor.radius = 0.02;
        g.stars.push_back(anchor);

        g.bins.resize(
            static_cast<std::size_t>(StarGrid::nlat * StarGrid::nlon));
        for (int i = 0; i < static_cast<int>(g.stars.size()); ++i) {
            g.bins[static_cast<std::size_t>(star_bin(g.stars[i].dir))]
                .push_back(i);
        }
        return g;
    }();
    return grid;
}

// Colour of one celestial direction. Sky floor is the old flat background
// {4, 4, 9} where no star and no band fall.
[[nodiscard]] inline Rgb colour_of_direction(Vec3 dir) {
    const double n2 = dot(dir, dir);
    if (n2 > 0.0) {
        dir = dir * (1.0 / std::sqrt(n2));
    }
    const StarGrid& grid = star_grid();
    const int nlat = StarGrid::nlat;
    const int nlon = StarGrid::nlon;
    const int cell = star_bin(dir);
    const int lat0 = cell / nlon;
    const int lon0 = cell - lat0 * nlon;

    double best = 0.0;
    double temp = 0.5;
    for (int dlat = -1; dlat <= 1; ++dlat) {
        const int lat = lat0 + dlat;
        if (lat < 0 || lat >= nlat) {
            continue;
        }
        for (int dlon = -1; dlon <= 1; ++dlon) {
            int lon = lon0 + dlon;
            if (lon < 0) {
                lon += nlon;
            }
            if (lon >= nlon) {
                lon -= nlon;
            }
            const auto& bin =
                grid.bins[static_cast<std::size_t>(lat * nlon + lon)];
            for (int idx : bin) {
                const Star& s = grid.stars[static_cast<std::size_t>(idx)];
                const double c = dot(dir, s.dir);
                if (c < std::cos(s.radius)) {
                    continue;
                }
                const double ang = std::acos(std::clamp(c, -1.0, 1.0));
                const double sigma = s.radius * 0.45;
                const double w = s.brightness *
                                 std::exp(-0.5 * (ang * ang) / (sigma * sigma));
                if (w > best) {
                    best = w;
                    temp = s.temperature;
                }
            }
        }
    }

    // A smooth band so lensing is visible as a warp, not only as dots.
    // Synthetic. Not a Milky Way model.
    const Vec3 pole = normalize(Vec3{0.35, 0.55, 0.76});
    const double latb = dot(dir, pole);
    const double band = std::exp(-0.5 * (latb * latb) / (0.10 * 0.10));
    const double band_lum = 0.055 * band;

    const double lum = std::clamp(best, 0.0, 1.0);
    // temperature: 0 red, 1 blue-white.
    const double sr = lum * (1.0 - 0.25 * temp);
    const double sg = lum * (0.72 + 0.28 * temp);
    const double sb = lum * (0.35 + 0.65 * temp);
    const auto u8 = [](double c) {
        return static_cast<std::uint8_t>(std::clamp(c, 0.0, 1.0) * 255.0 + 0.5);
    };
    const double R = std::max(sr + band_lum * 0.55, 4.0 / 255.0);
    const double G = std::max(sg + band_lum * 0.62, 4.0 / 255.0);
    const double B = std::max(sb + band_lum * 0.95, 9.0 / 255.0);
    return Rgb{u8(R), u8(G), u8(B)};
}

[[nodiscard]] inline Rgb starfield_colour(double X, double Y, double phi_escape,
                                          double inclination_deg,
                                          double camera_distance_M) {
    const Vec3 dir = star_source_direction(X, Y, phi_escape, inclination_deg,
                                           camera_distance_M);
    return colour_of_direction(dir);
}

} // namespace blackhole_ds::viz
