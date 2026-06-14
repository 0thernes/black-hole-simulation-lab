// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/qnm/kerr_fit.hpp
// Berti-Cardoso-Will closed-form fit for the dominant Kerr ringdown mode -- a
// fast, low-accuracy alternative to the exact Leaver solver in leaver.hpp.
//
// Truth tier: numerical_approximation (an empirical fit, ~1-2% accurate).
// Source: E. Berti, V. Cardoso & C. M. Will, "On gravitational-wave
// spectroscopy of massive black holes with the space interferometer LISA,"
// Phys. Rev. D 73, 064030 (2006), arXiv:gr-qc/0512160 (Table VIII).
//
// This is the formula used in LISA/LIGO ringdown forecasts. It is provided as a
// fast seed / sanity check; for accuracy use kerr_qnm() (Leaver). It agrees
// with the exact value to ~1.4% at a=0 and ~0.2-0.8% at high spin (see
// qnm_tests.cpp).

#pragma once

#include <cmath>

#include "blackhole_ds/core/truth_label.hpp"
#include "blackhole_ds/qnm/leaver.hpp" // QnmFrequency

namespace blackhole_ds::qnm {

// Fitted M*omega for the gravitational l=m=2, n=0 Kerr QNM at dimensionless
// spin j = a/M in [0, 1). omega_R = f1 + f2 (1-j)^f3; quality factor
// Q = q1 + q2 (1-j)^q3; omega_I = omega_R/(2Q).
[[nodiscard]] inline QnmFrequency bcw_kerr_220(double spin_j) {
    QnmFrequency out;
    out.tier = core::TruthLabel::NumericalApproximation;
    out.converged = true;
    const double wr = 1.5251 - 1.1568 * std::pow(1.0 - spin_j, 0.1292);
    const double Q = 0.7000 + 1.4187 * std::pow(1.0 - spin_j, -0.4990);
    out.re = wr;
    out.im = -wr / (2.0 * Q);
    return out;
}

} // namespace blackhole_ds::qnm
