// blackhole_ds/data/csv_writer.hpp
// Minimal CSV emitter for analytic-observable runs.
//
// Truth-tier-aware: every output row carries its model_status label so a
// downstream consumer can distinguish tiers when they coexist in one file.

#pragma once

#include <ostream>
#include <string_view>

#include "blackhole_ds/core/truth_label.hpp"

namespace blackhole_ds::data {

class CsvWriter {
public:
    explicit CsvWriter(std::ostream& out) : out_(out) {}

    void write_header() {
        out_ << "model_status,mass_solar,spin_a_over_M,quantity,value_si_meters\n";
    }

    void write_row(core::TruthLabel label,
                   double mass_solar,
                   double spin_a_over_M,
                   std::string_view quantity,
                   double value_si_meters) {
        out_ << core::to_string(label) << ','
             << mass_solar << ','
             << spin_a_over_M << ','
             << quantity << ','
             << value_si_meters << '\n';
    }

private:
    std::ostream& out_;
};

} // namespace blackhole_ds::data
