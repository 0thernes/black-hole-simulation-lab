// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/cli/options.hpp
// CLI option type and argument parser, extracted from main so it can be
// unit-tested directly (inspection finding S07.06).
//
// The parser is pure: it reads argv, writes parsed values into Options, and
// writes any diagnostics to the supplied error stream. No global state, no
// I/O beyond the error stream. Returns false on any malformed input.

#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace blackhole_ds::cli {

struct Options {
    double mass_solar = 1.0;
    double spin_a_over_M = 0.0;
    std::string format = "text"; // "text" or "csv"
    int steps = 9;               // spin intervals; table has steps + 1 rows
    bool show_help = false;
    bool deflection_set = false;   // whether --deflection was given
    double deflection_b = 0.0;     // impact parameter b/M for light bending
    bool shadow = false;           // render the ASCII shadow
    std::string image_path;        // --image <file.ppm>: render to PPM
    bool image_set = false;        // whether --image was given
    std::string disk_path;         // --disk <file.ppm>: lensed accretion disk
    bool disk_set = false;         // whether --disk was given
    double inclination_deg = 78.0; // --inclination <deg> for the disk view
    std::string kerr_shadow_path;  // --kerr-shadow <file.ppm>
    bool kerr_shadow_set = false;  // whether --kerr-shadow was given
    std::string kerr_disk_path;    // --kerr-disk <file.ppm>
    bool kerr_disk_set = false;    // whether --kerr-disk was given
};

[[nodiscard]] inline bool parse_double(std::string_view s, double& out) {
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

[[nodiscard]] inline bool parse_int(std::string_view s, int& out) {
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

// Parse argv into opt. Returns false (and writes a diagnostic to err) on any
// unknown flag, missing value, or unparseable value.
[[nodiscard]] inline bool parse_args(int argc, const char* const* argv,
                                     Options& opt, std::ostream& err) {
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
                err << "invalid --mass value\n";
                return false;
            }
        } else if (arg == "--spin") {
            const auto v = need_value("--spin");
            if (v.empty() || !parse_double(v, opt.spin_a_over_M)) {
                err << "invalid --spin value\n";
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
        } else if (arg == "--deflection") {
            const auto v = need_value("--deflection");
            if (v.empty() || !parse_double(v, opt.deflection_b) ||
                opt.deflection_b <= 0.0) {
                err << "invalid --deflection value (need b/M > 0)\n";
                return false;
            }
            opt.deflection_set = true;
        } else if (arg == "--shadow") {
            opt.shadow = true;
        } else if (arg == "--image") {
            const auto v = need_value("--image");
            if (v.empty()) {
                err << "invalid --image value (need an output path)\n";
                return false;
            }
            opt.image_path = std::string(v);
            opt.image_set = true;
        } else if (arg == "--disk") {
            const auto v = need_value("--disk");
            if (v.empty()) {
                err << "invalid --disk value (need an output path)\n";
                return false;
            }
            opt.disk_path = std::string(v);
            opt.disk_set = true;
        } else if (arg == "--kerr-shadow") {
            const auto v = need_value("--kerr-shadow");
            if (v.empty()) {
                err << "invalid --kerr-shadow value (need an output path)\n";
                return false;
            }
            opt.kerr_shadow_path = std::string(v);
            opt.kerr_shadow_set = true;
        } else if (arg == "--kerr-disk") {
            const auto v = need_value("--kerr-disk");
            if (v.empty()) {
                err << "invalid --kerr-disk value (need an output path)\n";
                return false;
            }
            opt.kerr_disk_path = std::string(v);
            opt.kerr_disk_set = true;
        } else if (arg == "--inclination") {
            const auto v = need_value("--inclination");
            if (v.empty() || !parse_double(v, opt.inclination_deg) ||
                opt.inclination_deg < 0.0 || opt.inclination_deg > 89.9) {
                err << "invalid --inclination value (need 0..89.9 deg)\n";
                return false;
            }
        } else {
            err << "unknown argument: " << arg << '\n';
            return false;
        }
    }
    return true;
}

} // namespace blackhole_ds::cli
