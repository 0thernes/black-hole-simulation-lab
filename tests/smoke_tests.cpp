// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/smoke_tests.cpp
// Smoke tests for the analytic foundation.
//
// Uses a tiny home-grown CHECK macro so tests run identically in Debug
// and Release. assert() is a no-op under NDEBUG; we cannot rely on it.

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <type_traits>

#include "blackhole_ds/core/constants.hpp"
#include "blackhole_ds/core/truth_label.hpp"
#include "blackhole_ds/metrics/kerr.hpp"
#include "blackhole_ds/metrics/schwarzschild.hpp"
#include "blackhole_ds/units.hpp"

using blackhole_ds::core::to_string;
using blackhole_ds::core::TruthLabel;
namespace kerr_m = blackhole_ds::metrics::kerr;
namespace sch_m = blackhole_ds::metrics::schwarzschild;
using namespace blackhole_ds::units;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close_to(double actual, double expected, double tolerance) {
    return std::abs(actual - expected) <= tolerance;
}

} // namespace

// Compile-time proof of the dimensional-safety contract: same-tag binary
// arithmetic must exist, mixed-tag must not. This is what makes the
// units.hpp Usage Contract a tested guarantee instead of a comment.
template <typename A, typename B>
concept Addable = requires(A a, B b) { a + b; };

static_assert(Addable<Length, Length>, "same-dimension addition must compile");
static_assert(Addable<Energy, Energy>, "same-dimension addition must compile");
static_assert(!Addable<Length, Time>,
              "cross-dimension addition must NOT compile");
static_assert(!Addable<Mass, Energy>,
              "cross-dimension addition must NOT compile");

int main() {
    // Units header still resolves.
    static_assert(!std::is_same_v<Length, Time>);
    static_assert(std::is_same_v<decltype(10.0_m / 2.0_s), Velocity>);

    // Same-dimension arithmetic produces correct values.
    static_assert((Length{1.5} + Length{2.5}).value() == 4.0);
    static_assert((Length{5.0} - Length{2.0}).value() == 3.0);
    static_assert((Length{2.0} * 3.0).value() == 6.0);
    static_assert((3.0 * Length{2.0}).value() == 6.0);
    static_assert((Length{6.0} / 3.0).value() == 2.0);

    // Truth labels stringify as expected.
    CHECK(to_string(TruthLabel::AnalyticClassical) == "analytic_classical");
    CHECK(to_string(TruthLabel::SpeculativeExtension) ==
          "speculative_extension");

    // Schwarzschild analytic values for 1 solar mass.
    const double r_s_expected =
        2.0 * blackhole_ds::core::constants::geometric_meters_per_solar_mass;
    CHECK(close_to(sch_m::radius_m(1.0), r_s_expected, 1.0e-9));
    CHECK(close_to(sch_m::photon_sphere_m(1.0), 1.5 * r_s_expected, 1.0e-9));
    CHECK(close_to(sch_m::isco_m(1.0), 3.0 * r_s_expected, 1.0e-9));

    // Kerr Schwarzschild limit.
    CHECK(close_to(kerr_m::isco_dimensionless(0.0), 6.0, 1.0e-12));
    CHECK(close_to(kerr_m::photon_sphere_dimensionless(0.0), 3.0, 1.0e-12));

    // Kerr monotonicity in spin direction.
    CHECK(kerr_m::isco_dimensionless(0.9) < 6.0);
    CHECK(kerr_m::isco_dimensionless(-0.9) > 6.0);
    CHECK(kerr_m::photon_sphere_dimensionless(0.9) < 3.0);
    CHECK(kerr_m::photon_sphere_dimensionless(-0.9) > 3.0);

    // Extremal + near-extremal spin (audit 2026-06-14: the spin clamp was
    // widened from 0.999 to the physical [-1, 1]). The BPT closed form is exact
    // at extremal: prograde ISCO -> 1, retrograde -> 9, prograde photon -> 1.
    CHECK(close_to(kerr_m::isco_dimensionless(1.0), 1.0, 1.0e-9));
    CHECK(close_to(kerr_m::isco_dimensionless(-1.0), 9.0, 1.0e-9));
    CHECK(close_to(kerr_m::photon_sphere_dimensionless(1.0), 1.0, 1.0e-9));
    // A valid near-extremal spin (schema admits up to 0.9999) is no longer
    // silently clamped to the a = 0.999 value (isco(0.999) ~ 1.182).
    CHECK(kerr_m::isco_dimensionless(0.9999) < 1.10);

    // Analytic validators pass for exact canonical inputs.
    CHECK(validators::photon_sphere_radius_valid(Length{3.0}, Mass{1.0}));
    CHECK(validators::isco_radius_valid(Length{6.0}, Mass{1.0}));

    // Shadow DIAMETER is 2*sqrt(27) M = 6*sqrt(3) M, NOT 5.196 M.
    // b_crit = sqrt(27) M ~ 5.196 M is the shadow RADIUS; the audit found
    // a factor-of-2 mislabel here. The validator must accept the true
    // diameter and reject the radius passed off as a diameter.
    const double shadow_diameter = 2.0 * std::sqrt(27.0);
    CHECK(validators::schwarzschild_shadow_diameter_valid(
        Length{shadow_diameter}, Mass{1.0}));
    CHECK(!validators::schwarzschild_shadow_diameter_valid(
        Length{5.196152422706632}, Mass{1.0}));

    // KahanAccumulator behaves on small additions.
    KahanAccumulator<Length> acc;
    acc.add(Length{1.0});
    acc.add(Length{1.0e-16});
    acc.add(Length{1.0});
    CHECK(close_to(acc.total().value(), 2.0, 1.0e-12));

    if (failures == 0) {
        std::puts("blackhole_ds_smoke_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "blackhole_ds_smoke_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
