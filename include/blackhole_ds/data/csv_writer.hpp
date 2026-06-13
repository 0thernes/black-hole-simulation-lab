// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/data/csv_writer.hpp
// Minimal CSV emitter for analytic-observable runs.
//
// Truth-tier-aware: every output row carries its model_status label so a
// downstream consumer can distinguish tiers when they coexist in one file.

#pragma once

#include <iomanip>
#include <limits>
#include <ostream>
#include <string_view>

#include "blackhole_ds/core/truth_label.hpp"

namespace blackhole_ds::data {

class CsvWriter {
public:
    // Saves and restores the caller's stream formatting; rows are written
    // at max_digits10 (17 significant digits) so exported values
    // round-trip bit-exactly. The project validates analytics to 1e-12;
    // the default 6-digit ostream precision would silently destroy that.
    explicit CsvWriter(std::ostream& out) : out_(out) {}

    void write_header() {
        out_ << "model_status,mass_solar,spin_a_over_M,quantity,value_si_"
                "meters\n";
    }

    void write_row(core::TruthLabel label, double mass_solar,
                   double spin_a_over_M, std::string_view quantity,
                   double value_si_meters) {
        const auto saved_precision = out_.precision();
        const auto saved_flags = out_.flags();
        out_ << std::setprecision(std::numeric_limits<double>::max_digits10);
        out_ << core::to_string(label) << ',' << mass_solar << ','
             << spin_a_over_M << ',' << quantity << ',' << value_si_meters
             << '\n';
        out_.precision(saved_precision);
        out_.flags(saved_flags);
    }

private:
    std::ostream& out_;
};

} // namespace blackhole_ds::data
