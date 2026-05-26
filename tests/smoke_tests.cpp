#include <cassert>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <type_traits>

#include "blackhole_ds/units.hpp"

using namespace blackhole_ds::units;

namespace {

bool close_to(double actual, double expected, double tolerance) {
    return std::abs(actual - expected) <= tolerance;
}

double kerr_isco_radius(double spin_a_over_m) {
    const double a = std::clamp(spin_a_over_m, -0.999, 0.999);
    const double z1 = 1.0 + std::cbrt(1.0 - a * a) *
        (std::cbrt(1.0 + a) + std::cbrt(1.0 - a));
    const double z2 = std::sqrt(3.0 * a * a + z1 * z1);
    return 3.0 + z2 -
        std::copysign(std::sqrt((3.0 - z1) * (3.0 + z1 + 2.0 * z2)), a);
}

} // namespace

int main() {
    static_assert(!std::is_same_v<Length, Time>);
    static_assert(std::is_same_v<decltype(10.0_m / 2.0_s), Velocity>);

    assert(validators::photon_sphere_radius_valid(Length{3.0}, Mass{1.0}));
    assert(validators::isco_radius_valid(Length{6.0}, Mass{1.0}));
    assert(validators::shadow_diameter_valid(Length{5.196}, Mass{1.0}, Dimensionless{0.0}));

    KahanAccumulator<Length> accumulator;
    accumulator.add(Length{1.0});
    accumulator.add(Length{1.0e-16});
    accumulator.add(Length{1.0});
    assert(close_to(accumulator.total().value(), 2.0, 1.0e-12));

    assert(close_to(kerr_isco_radius(0.0), 6.0, 1.0e-12));
    assert(kerr_isco_radius(0.9) < 6.0);
    assert(kerr_isco_radius(-0.9) > 6.0);

    std::cout << "blackhole_ds_smoke_tests passed\n";
    return 0;
}
