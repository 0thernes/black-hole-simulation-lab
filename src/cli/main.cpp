// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// src/cli/main.cpp
// BlackHoleDS CLI entry point.
//
// Flags:
//   --mass <Msun>          Mass in solar masses (default: 1.0).
//   --spin <a/M>           Dimensionless spin (default: 0.0).
//   --format <text|csv>    Output format (default: text).
//   --steps <N>            Number of rows in the Kerr table (default: 9).
//   --help                 Print this help and exit.
//
// Truth label of all printed values: analytic_classical.

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "blackhole_ds/cli/options.hpp"
#include "blackhole_ds/core/truth_label.hpp"
#include "blackhole_ds/data/csv_writer.hpp"
#include "blackhole_ds/geodesics/schwarzschild_photon.hpp"
#include "blackhole_ds/metrics/kerr.hpp"
#include "blackhole_ds/metrics/schwarzschild.hpp"

namespace bhds = blackhole_ds;
using bhds::cli::Options;

namespace {

void print_help(std::ostream& os) {
    os << "blackhole_ds: analytic Schwarzschild and Kerr observables.\n"
       << "\n"
       << "Flags:\n"
       << "  --mass <Msun>        Mass in solar masses (default 1.0)\n"
       << "  --spin <a/M>         Dimensionless Kerr spin (default 0.0)\n"
       << "  --format <text|csv>  Output format (default text)\n"
       << "  --steps <N>          Kerr spin intervals; table has N+1 rows "
          "(default 9 -> 10 rows)\n"
       << "  --deflection <b/M>   Light deflection for impact parameter b "
          "(in units of M)\n"
       << "  --help               Print this help\n"
       << "\n"
       << "Truth label of all printed values: analytic_classical.\n";
}

void emit_text(const Options& opt) {
    using bhds::metrics::kerr::isco_dimensionless;
    using bhds::metrics::kerr::photon_sphere_dimensionless;
    using bhds::metrics::schwarzschild::isco_m;
    using bhds::metrics::schwarzschild::photon_sphere_m;
    using bhds::metrics::schwarzschild::radius_m;

    std::cout << "BlackHoleDS analytic observables\n"
              << "Truth tier: analytic_classical\n\n";

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Mass (Msun):           " << opt.mass_solar << '\n';
    std::cout << "Schwarzschild radius:  " << radius_m(opt.mass_solar)
              << " m\n";
    std::cout << "Photon sphere radius:  " << photon_sphere_m(opt.mass_solar)
              << " m\n";
    std::cout << "ISCO radius:           " << isco_m(opt.mass_solar)
              << " m\n\n";

    std::cout << "Kerr table (dimensionless, units of GM/c^2)\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Spin a/M    ISCO (r/M)    Photon (r/M)\n";
    for (int i = 0; i <= opt.steps; ++i) {
        const double a = -0.998 + (1.996 * i) / opt.steps;
        std::cout << std::setw(8) << a << "    " << std::setw(10)
                  << isco_dimensionless(a) << "    " << std::setw(10)
                  << photon_sphere_dimensionless(a) << '\n';
    }
}

void emit_csv(const Options& opt) {
    using bhds::core::TruthLabel;
    using bhds::data::CsvWriter;

    CsvWriter w(std::cout);
    w.write_header();
    w.write_row(TruthLabel::AnalyticClassical, opt.mass_solar, 0.0,
                "schwarzschild_radius_m",
                bhds::metrics::schwarzschild::radius_m(opt.mass_solar));
    w.write_row(TruthLabel::AnalyticClassical, opt.mass_solar, 0.0,
                "photon_sphere_m",
                bhds::metrics::schwarzschild::photon_sphere_m(opt.mass_solar));
    w.write_row(TruthLabel::AnalyticClassical, opt.mass_solar, 0.0, "isco_m",
                bhds::metrics::schwarzschild::isco_m(opt.mass_solar));

    for (int i = 0; i <= opt.steps; ++i) {
        const double a = -0.998 + (1.996 * i) / opt.steps;
        w.write_row(TruthLabel::AnalyticClassical, opt.mass_solar, a,
                    "kerr_isco_dimensionless",
                    bhds::metrics::kerr::isco_dimensionless(a));
        w.write_row(TruthLabel::AnalyticClassical, opt.mass_solar, a,
                    "kerr_photon_sphere_dimensionless",
                    bhds::metrics::kerr::photon_sphere_dimensionless(a));
    }
}

void emit_deflection(const Options& opt) {
    namespace sch = bhds::geodesics::schwarzschild;
    const double b = opt.deflection_b;

    std::cout << "Schwarzschild light bending (geometric units, M = 1)\n"
              << "Truth tier: numerical_approximation (RK4 geodesic; "
                 "weak-field limit 4M/b is analytic_classical)\n\n";
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Impact parameter b/M:   " << b << '\n';
    std::cout << "Critical b/M (shadow):  " << sch::b_critical
              << "  (= sqrt(27))\n";

    switch (sch::classify(b)) {
    case sch::RayFate::Captured:
        std::cout << "Fate: CAPTURED (b < b_critical) -- this ray falls into "
                     "the black hole.\n";
        break;
    case sch::RayFate::Critical:
        std::cout << "Fate: CRITICAL -- asymptotes to the photon sphere at "
                     "r = 3M.\n";
        break;
    case sch::RayFate::Escapes: {
        const double d = sch::light_deflection_angle(b);
        std::cout << "Fate: ESCAPES.\n";
        std::cout << "Deflection angle:       " << d << " rad ("
                  << d * 180.0 / sch::pi << " deg)\n";
        std::cout << "Weak-field 4M/b:        " << 4.0 / b << " rad\n";
        break;
    }
    }
}

} // namespace

int main(int argc, char** argv) {
    Options opt;
    std::ostringstream err;
    if (!bhds::cli::parse_args(argc, argv, opt, err)) {
        std::cerr << err.str();
        print_help(std::cerr);
        return EXIT_FAILURE;
    }
    if (opt.show_help) {
        print_help(std::cout);
        return EXIT_SUCCESS;
    }
    if (opt.deflection_set) {
        emit_deflection(opt);
        return EXIT_SUCCESS;
    }
    if (opt.format == "csv") {
        emit_csv(opt);
    } else {
        emit_text(opt);
    }
    return EXIT_SUCCESS;
}
