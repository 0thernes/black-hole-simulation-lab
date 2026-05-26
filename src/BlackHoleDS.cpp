// BlackHoleDS — Visual Black Hole Simulation and Data Science Platform
// C++20 seed executable.
//
// This program computes analytic Schwarzschild and Kerr black hole observables
// and prints them as a small data-science-friendly table. It is intentionally
// small and dependency-free so it can build on any C++20 toolchain. The full
// modular engine (metrics/, integrators/, data/, viz/) will grow around it.
//
// Scientific honesty: every value printed here is labeled with a model status.
// We never mix analytic GR results with frontier or speculative claims.
//
// References used for the analytic formulas in this seed:
//   - Misner, Thorne, Wheeler, Gravitation (1973).
//   - Bardeen, Press, Teukolsky (1972).
//   - Chandrasekhar, The Mathematical Theory of Black Holes (1983).

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>

#include "blackhole_ds/units.hpp"

namespace bhds = blackhole_ds::units;

namespace {

// Solar mass in geometric units (meters). 1 Msun -> G*Msun/c^2 in meters.
// This converts a value in geometric units (M=1) to SI meters.
constexpr double kGeometricMetersPerSolarMass = 1.476625038e3;

// Kerr prograde ISCO in geometric units. Closed-form Bardeen, Press, Teukolsky
// (1972). For a == 0 this returns exactly 6.0 (Schwarzschild).
double kerr_isco_dimensionless(double spin_a_over_M) {
    const double a = std::clamp(spin_a_over_M, -0.999, 0.999);
    const double z1 = 1.0 +
        std::cbrt(1.0 - a * a) * (std::cbrt(1.0 + a) + std::cbrt(1.0 - a));
    const double z2 = std::sqrt(3.0 * a * a + z1 * z1);
    return 3.0 + z2 -
        std::copysign(std::sqrt((3.0 - z1) * (3.0 + z1 + 2.0 * z2)), a);
}

// Kerr prograde equatorial photon sphere in geometric units.
// r_ph = 2 * (1 + cos((2/3) * acos(-a))). For a == 0 this returns exactly 3.0.
double kerr_photon_sphere_dimensionless(double spin_a_over_M) {
    const double a = std::clamp(spin_a_over_M, -0.999, 0.999);
    return 2.0 * (1.0 + std::cos((2.0 / 3.0) * std::acos(-a)));
}

void print_header(std::string_view title) {
    std::cout << '\n' << title << '\n';
    std::cout << std::string(title.size(), '-') << '\n';
}

void print_run(double mass_solar) {
    print_header("Analytic Schwarzschild observables (model_status: analytic_classical)");

    // In geometric units, r_s = 2 M. Convert to meters via solar-mass scale.
    const double rs_m = 2.0 * mass_solar * kGeometricMetersPerSolarMass;
    const double rph_m = 3.0 * mass_solar * kGeometricMetersPerSolarMass;
    const double risco_m = 6.0 * mass_solar * kGeometricMetersPerSolarMass;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Mass (Msun):           " << mass_solar << '\n';
    std::cout << "Schwarzschild radius:  " << rs_m << " m\n";
    std::cout << "Photon sphere radius:  " << rph_m << " m\n";
    std::cout << "ISCO radius:           " << risco_m << " m\n";

    // Sanity check: photon sphere is 1.5 * r_s, ISCO is 3 * r_s.
    using bhds::validators::photon_sphere_radius_valid;
    using bhds::validators::isco_radius_valid;
    const bhds::Mass M{mass_solar};
    const bhds::Length r_photon{3.0 * mass_solar};
    const bhds::Length r_isco{6.0 * mass_solar};
    std::cout << "Validator photon sphere: "
              << (photon_sphere_radius_valid(r_photon, M) ? "PASS" : "FAIL")
              << '\n';
    std::cout << "Validator ISCO:          "
              << (isco_radius_valid(r_isco, M) ? "PASS" : "FAIL") << '\n';
}

void print_kerr_table() {
    print_header("Kerr ISCO and photon sphere vs spin "
                 "(model_status: analytic_classical)");

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Spin a/M    ISCO (r/M)    Photon (r/M)\n";

    const int rows = 9;
    for (int i = 0; i <= rows; ++i) {
        const double a = -0.998 + (1.996 * i) / rows;
        std::cout << std::setw(8) << a << "    "
                  << std::setw(10) << kerr_isco_dimensionless(a) << "    "
                  << std::setw(10) << kerr_photon_sphere_dimensionless(a)
                  << '\n';
    }

    std::cout << '\n'
              << "Schwarzschild limit (a == 0): ISCO must equal 6.0, "
                 "photon sphere must equal 3.0.\n";
}

void print_footer() {
    print_header("Notes");
    std::cout
        << "1. All values above are analytic, classical general-relativity\n"
           "   results. Numerical, observational, visual, pedagogical, and\n"
           "   speculative results live in clearly labeled modules.\n"
           "2. The next milestones split this seed into core, metrics,\n"
           "   integrators, and data modules and add a CLI plus CSV export.\n"
           "3. See AUDIT-250-POINT-GOLD-STANDARD.md and docs/ for the\n"
           "   research, audit, and development plan.\n";
}

} // namespace

int main() {
    std::cout << "BlackHoleDS seed simulation\n"
              << "C++20, units-safe analytic core, scientific-honesty labels\n";

    print_run(1.0); // 1 solar mass reference run
    print_kerr_table();
    print_footer();
    return 0;
}
