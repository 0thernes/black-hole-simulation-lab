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

using blackhole_ds::core::TruthLabel;
using blackhole_ds::core::to_string;
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

int main() {
    // Units header still resolves.
    static_assert(!std::is_same_v<Length, Time>);
    static_assert(std::is_same_v<decltype(10.0_m / 2.0_s), Velocity>);

    // Truth labels stringify as expected.
    CHECK(to_string(TruthLabel::AnalyticClassical) == "analytic_classical");
    CHECK(to_string(TruthLabel::SpeculativeExtension) == "speculative_extension");

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

    // Existing analytic validators still pass for canonical inputs.
    CHECK(validators::photon_sphere_radius_valid(Length{3.0}, Mass{1.0}));
    CHECK(validators::isco_radius_valid(Length{6.0}, Mass{1.0}));
    CHECK(validators::shadow_diameter_valid(Length{5.196}, Mass{1.0},
                                            Dimensionless{0.0}));

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
