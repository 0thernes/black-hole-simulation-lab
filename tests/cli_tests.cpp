// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// tests/cli_tests.cpp
// Unit tests for the CLI argument parser (blackhole_ds/cli/options.hpp) and
// the CSV writer (blackhole_ds/data/csv_writer.hpp). Inspection findings
// S07.05 (CsvWriter untested) and S07.06 (parser untested).
//
// Uses a tiny CHECK macro so behavior is identical in Debug and Release.

#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

#include "blackhole_ds/cli/options.hpp"
#include "blackhole_ds/core/truth_label.hpp"
#include "blackhole_ds/data/csv_writer.hpp"

using blackhole_ds::cli::Options;
using blackhole_ds::cli::parse_args;
using blackhole_ds::core::TruthLabel;
using blackhole_ds::data::CsvWriter;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

// Build a const char* argv from a vector of strings (argv[0] is the program).
struct Argv {
    std::vector<std::string> store;
    std::vector<const char*> ptrs;
    explicit Argv(std::initializer_list<const char*> args) {
        store.emplace_back("blackhole_ds");
        for (const char* a : args) {
            store.emplace_back(a);
        }
        for (auto& s : store) {
            ptrs.push_back(s.c_str());
        }
    }
    int argc() const {
        return static_cast<int>(ptrs.size());
    }
    const char* const* argv() const {
        return ptrs.data();
    }
};

} // namespace

int main() {
    // --- Defaults when no flags are given ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{};
        CHECK(parse_args(a.argc(), a.argv(), opt, err));
        CHECK(opt.mass_solar == 1.0);
        CHECK(opt.spin_a_over_M == 0.0);
        CHECK(opt.format == "text");
        CHECK(opt.steps == 9);
        CHECK(!opt.show_help);
        CHECK(err.str().empty());
    }

    // --- Valid flags parse into the right fields ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{"--mass",   "10.5", "--spin",  "0.9",
               "--format", "csv",  "--steps", "4"};
        CHECK(parse_args(a.argc(), a.argv(), opt, err));
        CHECK(opt.mass_solar == 10.5);
        CHECK(opt.spin_a_over_M == 0.9);
        CHECK(opt.format == "csv");
        CHECK(opt.steps == 4);
    }

    // --- --help sets the flag and parsing still succeeds ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{"--help"};
        CHECK(parse_args(a.argc(), a.argv(), opt, err));
        CHECK(opt.show_help);
    }

    // --- Unparseable numeric value is rejected with a diagnostic ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{"--mass", "abc"};
        CHECK(!parse_args(a.argc(), a.argv(), opt, err));
        CHECK(err.str().find("--mass") != std::string::npos);
    }

    // --- Missing value for a flag is rejected with a diagnostic ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{"--mass"};
        CHECK(!parse_args(a.argc(), a.argv(), opt, err));
        CHECK(err.str().find("missing value") != std::string::npos);
    }

    // --- Unknown format is rejected ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{"--format", "yaml"};
        CHECK(!parse_args(a.argc(), a.argv(), opt, err));
        CHECK(err.str().find("unknown format") != std::string::npos);
    }

    // --- --steps must be >= 1 ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{"--steps", "0"};
        CHECK(!parse_args(a.argc(), a.argv(), opt, err));
    }

    // --- Unknown argument is rejected ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{"--nonsense"};
        CHECK(!parse_args(a.argc(), a.argv(), opt, err));
        CHECK(err.str().find("unknown argument") != std::string::npos);
    }

    // --- Trailing garbage after a number is rejected (strict parse) ---
    {
        Options opt;
        std::ostringstream err;
        Argv a{"--mass", "10x"};
        CHECK(!parse_args(a.argc(), a.argv(), opt, err));
    }

    // --- CsvWriter: header is exactly the documented column order ---
    {
        std::ostringstream out;
        CsvWriter w(out);
        w.write_header();
        CHECK(out.str() == "model_status,mass_solar,spin_a_over_M,quantity,"
                           "value_si_meters\n");
    }

    // --- CsvWriter: a row carries the truth label and full precision ---
    {
        std::ostringstream out;
        CsvWriter w(out);
        w.write_row(TruthLabel::AnalyticClassical, 1.0, 0.0,
                    "schwarzschild_radius_m", 2953.250076);
        const std::string row = out.str();
        CHECK(row.rfind("analytic_classical,", 0) == 0);
        CHECK(row.find("schwarzschild_radius_m") != std::string::npos);
        // 17-significant-digit precision must preserve more than 6 digits.
        CHECK(row.find("2953.25007") != std::string::npos);
        CHECK(row.back() == '\n');
    }

    // --- CsvWriter: writing restores the caller's stream precision ---
    {
        std::ostringstream out;
        out << 3.14159265358979; // default precision (6 sig figs)
        const std::string before = out.str();
        CsvWriter w(out);
        w.write_row(TruthLabel::AnalyticClassical, 1.0, 0.0, "q", 1.0);
        out.str(""); // clear buffer, keep flags
        out << 3.14159265358979;
        CHECK(out.str() == before); // precision was restored
    }

    if (failures == 0) {
        std::puts("cli_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "cli_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
