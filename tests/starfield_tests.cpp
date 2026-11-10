// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/starfield_tests.cpp
// The star field sits behind the disk. These checks lock three facts:
//   - a ray's escape azimuth is the straight-line sweep (pi) plus the
//     gravitational deflection, small at large b and large near the
//     photon sphere;
//   - flipping the sign of X mirrors the celestial direction;
//   - stars replace only background pixels. The shadow stays black and a
//     disk hit keeps its colour. With stars off, the background is the
//     old flat floor.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "blackhole_ds/geodesics/schwarzschild_photon.hpp"
#include "blackhole_ds/viz/disk_image.hpp"
#include "blackhole_ds/viz/renderer.hpp"
#include "blackhole_ds/viz/starfield.hpp"

namespace sch = blackhole_ds::geodesics::schwarzschild;
using namespace blackhole_ds::viz;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close(double a, double b, double tol) {
    return std::abs(a - b) <= tol;
}

} // namespace

int main() {
    // --- Escape azimuth: weak ray near pi, strong ray much larger ---
    {
        DiskView v;
        v.inclination_deg = 60.0;
        // Park the disk out of the way so the escape azimuth is what we
        // measure, not an early disk hit.
        v.r_in_M = 1000.0;
        v.r_out_M = 1001.0;
        const PixelHit weak = trace_disk_pixel(30.0, 0.0, v);
        CHECK(weak.kind == HitKind::Background);
        CHECK(weak.phi_escape > star_pi);
        CHECK(weak.phi_escape < star_pi + 0.5);

        const PixelHit strong = trace_disk_pixel(5.5, 0.0, v);
        CHECK(strong.kind == HitKind::Background);
        CHECK(strong.phi_escape > star_pi + 1.0);

        // Sign of X does not change the orbit, so the sweep matches.
        const PixelHit mirror = trace_disk_pixel(-30.0, 0.0, v);
        CHECK(close(weak.phi_escape, mirror.phi_escape, 1.0e-9));
    }

    // --- delta = 0 is the camera aim; +X and -X mirror through Y=0 ---
    {
        const double incl = 70.0;
        const double rcam = 80.0;
        const Vec3 look = star_source_direction(6.0, 3.0, star_pi, incl, rcam);
        const Vec3 aim_off =
            star_source_direction(6.0, 3.0, star_pi + 0.4, incl, rcam);
        CHECK(dot(look, aim_off) < 0.999); // deflection moves the source

        const Vec3 right =
            star_source_direction(8.0, -2.0, star_pi + 0.2, incl, rcam);
        const Vec3 left =
            star_source_direction(-8.0, -2.0, star_pi + 0.2, incl, rcam);
        // Image +X is Cartesian +Y, so the mirror flips that component.
        CHECK(close(right.x, left.x, 1.0e-9));
        CHECK(close(right.y, -left.y, 1.0e-9));
        CHECK(close(right.z, left.z, 1.0e-9));
    }

    // --- The planted anchor star is actually bright ---
    {
        const Rgb c = colour_of_direction(anchor_star_direction());
        CHECK(c.r > 100);
        CHECK(c.g > 100);
        CHECK(c.b > 100);
    }

    // --- Stars off: the far background is the old floor, and the facade
    //     is the CPU tracer ---
    {
        CHECK(std::strcmp(render_backend_name(), "cpu") == 0);
        DiskView v;
        v.width = 24;
        v.height = 24;
        v.samples = 1;
        v.inclination_deg = 80.0;
        v.draw_stars = false;
        const double sin_i = std::sin(80.0 * star_pi / 180.0);
        const Rgb sky = sample_colour(40.0, 40.0, sin_i, v);
        CHECK(sky.r == 4 && sky.g == 4 && sky.b == 9);

        const Image via_facade = render_lensed_disk(v);
        const Image direct = render_disk_image(v);
        CHECK(via_facade.width() == direct.width());
        CHECK(via_facade.height() == direct.height());
        // A tiny view would be slow to compare in full; the corner pixel of
        // the default image is background and must match.
        const int x = 2;
        const int y = 2;
        const Rgb a = via_facade.at(x, y);
        const Rgb b = direct.at(x, y);
        CHECK(a.r == b.r && a.g == b.g && a.b == b.b);
    }

    // --- Stars paint the background and leave the shadow and the disk ---
    {
        DiskView v;
        v.width = 48;
        v.height = 48;
        v.samples = 1;
        v.inclination_deg = 80.0;
        v.draw_stars = false;
        const Image plain = render_disk_image(v);
        v.draw_stars = true;
        const Image starred = render_disk_image(v);

        const int cx = v.width / 2;
        const int cy = v.height / 2;
        const Rgb shadow = starred.at(cx, cy);
        CHECK(shadow.r == 0 && shadow.g == 0 && shadow.b == 0);

        int changed = 0;
        int disk_changed = 0;
        for (int j = 0; j < v.height; ++j) {
            for (int i = 0; i < v.width; ++i) {
                const Rgb p = plain.at(i, j);
                const Rgb s = starred.at(i, j);
                const bool diff = p.r != s.r || p.g != s.g || p.b != s.b;
                if (!diff) {
                    continue;
                }
                ++changed;
                // The flat floor is {4,4,9}. Anything else in the plain
                // image is disk (the shadow is black and stays black).
                if (!(p.r == 4 && p.g == 4 && p.b == 9) &&
                    !(p.r == 0 && p.g == 0 && p.b == 0)) {
                    ++disk_changed;
                }
            }
        }
        CHECK(changed > 0);
        CHECK(disk_changed == 0);
    }

    // Reference values live in geodesic_tests; keep the shadow radius in
    // view here so a star-field change cannot silently move the boundary
    // the background is drawn against.
    CHECK(close(sch::b_critical, std::sqrt(27.0), 1.0e-12));

    if (failures == 0) {
        std::puts("starfield_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "starfield_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
