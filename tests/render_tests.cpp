// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// tests/render_tests.cpp
// Tests the PPM writer and the shadow + photon-ring raster renderer.
// The headline check: the shadow radius measured FROM the rendered image
// matches the analytic sqrt(27) M to within one pixel (the roadmap M3
// validation idea, applied to the first raster image).

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

#include "blackhole_ds/geodesics/schwarzschild_photon.hpp"
#include "blackhole_ds/viz/ppm_writer.hpp"
#include "blackhole_ds/viz/shadow_image.hpp"

using namespace blackhole_ds::viz;
namespace sch = blackhole_ds::geodesics::schwarzschild;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close_to(double a, double b, double tol) {
    return std::abs(a - b) <= tol;
}

} // namespace

int main() {
    // --- PPM P6 header + byte payload for a known 2x1 image ---
    {
        Image img(2, 1);
        img.set(0, 0, Rgb{255, 0, 0});
        img.set(1, 0, Rgb{0, 128, 255});
        std::ostringstream out(std::ios::binary);
        img.write_ppm(out);
        const std::string s = out.str();
        // Header then 6 raw bytes.
        CHECK(s.rfind("P6\n2 1\n255\n", 0) == 0);
        const std::string payload = s.substr(s.find("255\n") + 4);
        CHECK(payload.size() == 6);
        CHECK(static_cast<unsigned char>(payload[0]) == 255);
        CHECK(static_cast<unsigned char>(payload[1]) == 0);
        CHECK(static_cast<unsigned char>(payload[2]) == 0);
        CHECK(static_cast<unsigned char>(payload[3]) == 0);
        CHECK(static_cast<unsigned char>(payload[4]) == 128);
        CHECK(static_cast<unsigned char>(payload[5]) == 255);
    }

    // --- P3 ASCII PPM round-trips the same pixels as integers ---
    {
        Image img(1, 1);
        img.set(0, 0, Rgb{10, 20, 30});
        std::ostringstream out;
        img.write_ppm_ascii(out);
        const std::string s = out.str();
        CHECK(s.rfind("P3\n1 1\n255\n", 0) == 0);
        CHECK(s.find("10 20 30") != std::string::npos);
    }

    // --- Out-of-bounds set is a no-op (no crash, no write) ---
    {
        Image img(2, 2);
        img.set(-1, 0, Rgb{1, 1, 1});
        img.set(0, 5, Rgb{1, 1, 1});
        CHECK(img.at(0, 0).r == 0); // unchanged
    }

    // --- ring_brightness ordering: shadow is dark, just outside is bright,
    //     far away decays toward the sky floor ---
    {
        CHECK(ring_brightness(sch::b_critical - 0.5) == 0.0); // inside shadow
        const double near_ring = ring_brightness(sch::b_critical + 0.05);
        const double far = ring_brightness(40.0);
        CHECK(near_ring > far);         // ring brighter than far field
        CHECK(far > 0.0 && far <= 0.2); // dim but nonzero sky floor
    }

    // --- The rendered image's shadow radius matches sqrt(27) M to < 1 px ---
    {
        ShadowImageView v;
        v.width = 401; // odd so there is an exact center column
        v.height = 401;
        v.half_extent_M = 12.0;
        const Image img = render_shadow_image(v);

        // Center pixel is inside the shadow -> black.
        const Rgb center = img.at(v.width / 2, v.height / 2);
        CHECK(center.r == 0 && center.g == 0 && center.b == 0);

        const double px_per_M = (v.width - 1) / (2.0 * v.half_extent_M);
        const double one_px_M = 1.0 / px_per_M;
        const double measured = measured_shadow_radius_M(img, v.half_extent_M);
        CHECK(close_to(measured, sch::b_critical, one_px_M + 1e-9));
    }

    if (failures == 0) {
        std::puts("render_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "render_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
