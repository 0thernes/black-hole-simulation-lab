// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// blackhole_ds/core/truth_label.hpp
// Compile-time enumeration of the project's six scientific truth tiers.
//
// Every computed value, exported row, plot, and visualization must carry
// one of these labels. See docs/vision/SCIENTIFIC_INTEGRITY_CHARTER.md
// section I.

#pragma once

#include <string_view>

namespace blackhole_ds::core {

enum class TruthLabel {
    AnalyticClassical,
    NumericalApproximation,
    ObservationalConstraint,
    VisualizationMetaphor,
    PedagogicalSimplification,
    SpeculativeExtension,
};

[[nodiscard]] constexpr std::string_view to_string(TruthLabel l) noexcept {
    switch (l) {
    case TruthLabel::AnalyticClassical:
        return "analytic_classical";
    case TruthLabel::NumericalApproximation:
        return "numerical_approximation";
    case TruthLabel::ObservationalConstraint:
        return "observational_constraint";
    case TruthLabel::VisualizationMetaphor:
        return "visualization_metaphor";
    case TruthLabel::PedagogicalSimplification:
        return "pedagogical_simplification";
    case TruthLabel::SpeculativeExtension:
        return "speculative_extension";
    }
    return "unknown";
}

} // namespace blackhole_ds::core
