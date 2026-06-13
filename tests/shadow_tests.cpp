// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/shadow_tests.cpp
// Validates the ASCII shadow renderer: the shadow is a disk of radius
// b_crit = sqrt(27) M, and the rendered captured-pixel area matches the
// analytic disk area.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

#include "blackhole_ds/viz/ascii_shadow.hpp"

namespace viz = blackhole_ds::viz;

namespace {

constexpr double M_PI_const = 3.14159265358979323846;

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

} // namespace

int main() {
    const double b_crit = std::sqrt(27.0);

    // --- Shadow radius is the critical impact parameter ---
    CHECK(std::abs(viz::shadow_radius_M() - b_crit) < 1e-12);

    // --- Membership: centre is dark; far out is sky; the rim is sharp ---
    CHECK(viz::in_shadow(0.0, 0.0));   // dead centre -> captured
    CHECK(!viz::in_shadow(10.0, 0.0)); // far -> escapes
    CHECK(viz::in_shadow(5.0, 0.0));   // r=5 < 5.196 -> captured
    CHECK(!viz::in_shadow(5.4, 0.0));  // r=5.4 > 5.196 -> escapes
    CHECK(viz::in_shadow(0.0, 5.0));   // isotropy: same in y
    CHECK(!viz::in_shadow(3.7, 3.7));  // r ~ 5.23 > b_crit -> sky

    // --- Rendered captured-area matches the analytic disk area (pi b^2) ---
    {
        viz::ShadowView v;
        v.width = 201;
        v.height = 201;
        v.half_extent_M = 10.0;
        std::ostringstream out;
        viz::render_shadow(out, v);
        const std::string img = out.str();

        // Count shadow cells (shadow_char is a space) excluding newlines.
        long shadow_cells = 0;
        long drawn_cells = 0;
        for (char c : img) {
            if (c == '\n') {
                continue;
            }
            ++drawn_cells;
            if (c == v.shadow_char) {
                ++shadow_cells;
            }
        }
        CHECK(drawn_cells == static_cast<long>(v.width) * v.height);

        // Each cell covers (2H/(w-1)) x (2H/(h-1)) in M^2. The captured area
        // should approximate pi * b_crit^2.
        const double cell_area = (2.0 * v.half_extent_M / (v.width - 1)) *
                                 (2.0 * v.half_extent_M / (v.height - 1));
        const double measured_area = shadow_cells * cell_area;
        const double analytic_area = M_PI_const * b_crit * b_crit;
        // 3% tolerance: grid discretization of a circle.
        CHECK(std::abs(measured_area - analytic_area) < 0.03 * analytic_area);
    }

    if (failures == 0) {
        std::puts("shadow_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "shadow_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
