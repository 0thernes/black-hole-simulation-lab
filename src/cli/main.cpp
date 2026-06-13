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
#include <string>
#include <string_view>

#include "blackhole_ds/core/truth_label.hpp"
#include "blackhole_ds/data/csv_writer.hpp"
#include "blackhole_ds/metrics/kerr.hpp"
#include "blackhole_ds/metrics/schwarzschild.hpp"

namespace bhds = blackhole_ds;

namespace {

struct Options {
    double mass_solar = 1.0;
    double spin_a_over_M = 0.0;
    std::string format = "text";
    int steps = 9;
    bool show_help = false;
};

void print_help(std::ostream& os) {
    os << "blackhole_ds: analytic Schwarzschild and Kerr observables.\n"
       << "\n"
       << "Flags:\n"
       << "  --mass <Msun>        Mass in solar masses (default 1.0)\n"
       << "  --spin <a/M>         Dimensionless Kerr spin (default 0.0)\n"
       << "  --format <text|csv>  Output format (default text)\n"
       << "  --steps <N>          Rows in the Kerr table (default 9)\n"
       << "  --help               Print this help\n"
       << "\n"
       << "Truth label of all printed values: analytic_classical.\n";
}

bool parse_double(std::string_view s, double& out) {
    try {
        size_t pos = 0;
        const double v = std::stod(std::string(s), &pos);
        if (pos != s.size()) {
            return false;
        }
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_int(std::string_view s, int& out) {
    try {
        size_t pos = 0;
        const int v = std::stoi(std::string(s), &pos);
        if (pos != s.size()) {
            return false;
        }
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_args(int argc, char** argv, Options& opt, std::ostream& err) {
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        const auto need_value = [&](std::string_view name) -> std::string_view {
            if (i + 1 >= argc) {
                err << "missing value for " << name << '\n';
                return {};
            }
            return argv[++i];
        };
        if (arg == "--help" || arg == "-h") {
            opt.show_help = true;
        } else if (arg == "--mass") {
            const auto v = need_value("--mass");
            if (v.empty() || !parse_double(v, opt.mass_solar)) {
                return false;
            }
        } else if (arg == "--spin") {
            const auto v = need_value("--spin");
            if (v.empty() || !parse_double(v, opt.spin_a_over_M)) {
                return false;
            }
        } else if (arg == "--format") {
            const auto v = need_value("--format");
            if (v.empty()) {
                return false;
            }
            opt.format = v;
            if (opt.format != "text" && opt.format != "csv") {
                err << "unknown format: " << opt.format << '\n';
                return false;
            }
        } else if (arg == "--steps") {
            const auto v = need_value("--steps");
            if (v.empty() || !parse_int(v, opt.steps) || opt.steps < 1) {
                err << "invalid --steps value\n";
                return false;
            }
        } else {
            err << "unknown argument: " << arg << '\n';
            return false;
        }
    }
    return true;
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

} // namespace

int main(int argc, char** argv) {
    Options opt;
    std::ostringstream err;
    if (!parse_args(argc, argv, opt, err)) {
        std::cerr << err.str();
        print_help(std::cerr);
        return EXIT_FAILURE;
    }
    if (opt.show_help) {
        print_help(std::cout);
        return EXIT_SUCCESS;
    }
    if (opt.format == "csv") {
        emit_csv(opt);
    } else {
        emit_text(opt);
    }
    return EXIT_SUCCESS;
}
