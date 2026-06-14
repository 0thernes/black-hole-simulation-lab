// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/qnm/spectroscopy.hpp
// Black-hole spectroscopy: physical ringdown frequencies/damping times and the
// no-hair consistency test, built on the Leaver QNM solver (leaver.hpp).
//
// Truth tier: numerical_approximation (QNM frequencies) / observational_-
// constraint (the no-hair test these enable).
// Source: Dreyer et al., Class. Quantum Grav. 21, 787 (2004); Berti, Cardoso &
// Will, Phys. Rev. D 73, 064030 (2006); Berti et al., Phys. Rev. Lett. 117,
// 101102 (2016) (GW150914 ringdown).
//
// The no-hair theorem says an astrophysical black hole is fully described by
// (M, J). Then EVERY ringdown quasinormal frequency is a function of (M, j)
// alone (j = a/M = cJ/(GM^2)). Measuring two modes fixes (M, j); a third
// over-determines the system -- consistency is a test that the remnant is a
// Kerr black hole. The cleanest observables are mass-INDEPENDENT: frequency
// ratios and quality factors depend only on j. This module computes both the
// physical (mass-dependent) ringdown observables and that mass-independent
// Kerr fingerprint. This is the simulator-side counterpart of catalogue
// problem #11 (testing the no-hair theorem).

#pragma once

#include <array>
#include <cmath>

#include "blackhole_ds/core/constants.hpp"
#include "blackhole_ds/core/truth_label.hpp"
#include "blackhole_ds/qnm/leaver.hpp"

namespace blackhole_ds::qnm {

inline constexpr double pi = 3.14159265358979323846;

// One ringdown mode with both dimensionless and physical observables.
struct RingdownMode {
    Mode mode{};
    double mw_re{0.0};          // Re(M*omega), dimensionless
    double mw_im{0.0};          // Im(M*omega) (< 0 for a damped mode)
    double frequency_hz{0.0};   // physical oscillation frequency
    double damping_time_s{0.0}; // physical e-folding (amplitude) time
    double quality{0.0};        // Q = omega_R / (2 omega_I), mass-independent
    bool converged{false};
    core::TruthLabel tier{core::TruthLabel::NumericalApproximation};
};

// Mass-independent Kerr fingerprint: functions of spin j alone. If a ringdown's
// measured ratios match these for a single j, the remnant is consistent with a
// Kerr black hole (no hair). Any significant deviation flags new physics.
struct NoHairSignature {
    double spin_j{0.0};
    double q_220{0.0};              // quality factor of the dominant mode
    double f330_over_f220{0.0};     // frequency ratio (3,3,0)/(2,2,0)
    double f210_over_f220{0.0};     // frequency ratio (2,1,0)/(2,2,0)
    double tau220_over_tau330{0.0}; // damping-time ratio (2,2,0)/(3,3,0)
};

// Light-crossing time of one gravitational radius, t_g = G M / c^3 [seconds].
// This is the single scale that turns dimensionless M*omega into physics.
[[nodiscard]] inline double light_crossing_time_s(double mass_solar) noexcept {
    return mass_solar * core::constants::geometric_meters_per_solar_mass /
           core::constants::speed_of_light_m_per_s;
}

[[nodiscard]] inline double frequency_hz(double mw_re,
                                         double mass_solar) noexcept {
    return mw_re / (2.0 * pi * light_crossing_time_s(mass_solar));
}

[[nodiscard]] inline double damping_time_s(double mw_im,
                                           double mass_solar) noexcept {
    return light_crossing_time_s(mass_solar) / std::abs(mw_im);
}

[[nodiscard]] inline double quality_factor(double mw_re,
                                           double mw_im) noexcept {
    return mw_re / (2.0 * std::abs(mw_im));
}

// Full physical ringdown observables for one (s,l,m,n) mode of a black hole of
// the given mass (solar) and dimensionless spin j. Uses the exact Leaver
// solver.
[[nodiscard]] inline RingdownMode
ringdown_mode(int s, int l, int m, int n, double mass_solar, double spin_j) {
    RingdownMode out;
    out.mode = Mode{s, l, m, n};
    const QnmFrequency q = kerr_qnm(s, l, m, n, spin_j);
    out.mw_re = q.re;
    out.mw_im = q.im;
    out.converged = q.converged;
    out.frequency_hz = frequency_hz(q.re, mass_solar);
    out.damping_time_s = damping_time_s(q.im, mass_solar);
    out.quality = quality_factor(q.re, q.im);
    out.tier = core::TruthLabel::NumericalApproximation;
    return out;
}

// The dominant gravitational ringdown multiplet (n=0): (2,2,0), (3,3,0),
// (4,4,0), (2,1,0). Gravitational spin weight s = -2.
[[nodiscard]] inline std::array<RingdownMode, 4>
ringdown_spectrum(double mass_solar, double spin_j) {
    return {ringdown_mode(-2, 2, 2, 0, mass_solar, spin_j),
            ringdown_mode(-2, 3, 3, 0, mass_solar, spin_j),
            ringdown_mode(-2, 4, 4, 0, mass_solar, spin_j),
            ringdown_mode(-2, 2, 1, 0, mass_solar, spin_j)};
}

// The mass-independent no-hair fingerprint at spin j. (Computed at a reference
// mass that cancels in every ratio/quality factor.)
[[nodiscard]] inline NoHairSignature no_hair_signature(double spin_j) {
    NoHairSignature sig;
    sig.spin_j = spin_j;
    const QnmFrequency m220 = kerr_qnm(-2, 2, 2, 0, spin_j);
    const QnmFrequency m330 = kerr_qnm(-2, 3, 3, 0, spin_j);
    const QnmFrequency m210 = kerr_qnm(-2, 2, 1, 0, spin_j);
    sig.q_220 = quality_factor(m220.re, m220.im);
    sig.f330_over_f220 = m330.re / m220.re;
    sig.f210_over_f220 = m210.re / m220.re;
    sig.tau220_over_tau330 = std::abs(m330.im) / std::abs(m220.im);
    return sig;
}

} // namespace blackhole_ds::qnm
