// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/qnm/wkb.hpp
// Third-order WKB quasinormal-mode estimate (Schwarzschild) -- an INDEPENDENT
// cross-check of the Leaver continued-fraction solver in leaver.hpp.
//
// Truth tier: numerical_approximation.
// Source: B. F. Schutz & C. M. Will, ApJ 291, L33 (1985); S. Iyer & C. M. Will,
// Phys. Rev. D 35, 3621 (1987); R. A. Konoplya, Phys. Rev. D 68, 024018 (2003),
// arXiv:gr-qc/0303052 (potential eq. 17, master condition eq. 2).
//
// The WKB method peaks the Regge-Wheeler potential and expands about the peak.
// At 3rd order it gives the real part to ~0.1% and the imaginary part to ~1-4%
// for the low-lying modes -- precise enough to confirm the Leaver result is on
// the right branch, not a precision tool (Leaver is). Agreement between two
// independent methods is the cross-check (cf. the metric oracle pattern).

#pragma once

#include <array>
#include <cmath>
#include <complex>

#include "blackhole_ds/core/truth_label.hpp"
#include "blackhole_ds/qnm/leaver.hpp" // cx, QnmFrequency

namespace blackhole_ds::qnm {

namespace detail {

// Regge-Wheeler potential, units 2M=1: V = (1-1/r)[ l(l+1)/r^2 + (1-s^2)/r^3 ].
inline double rw_potential(double r, int s, int l) {
    const double f = 1.0 - 1.0 / r;
    const double ll = static_cast<double>(l) * static_cast<double>(l + 1);
    const double ss = static_cast<double>(s) * static_cast<double>(s);
    return f * (ll / (r * r) + (1.0 - ss) / (r * r * r));
}

inline double tortoise_x(double r) {
    return r + std::log(r - 1.0);
} // 2M=1

inline double r_of_x(double x, double guess) {
    double r = guess;
    for (int i = 0; i < 200; ++i) {
        const double g = r + std::log(r - 1.0) - x;
        const double dg = 1.0 + 1.0 / (r - 1.0);
        double rn = r - g / dg;
        if (rn <= 1.0) {
            rn = 1.0 + (r - 1.0) * 0.5;
        }
        if (std::abs(rn - r) < 1e-15) {
            return rn;
        }
        r = rn;
    }
    return r;
}

inline double peak_radius(int s, int l) {
    double lo = 1.0001;
    double hi = 30.0;
    for (int i = 0; i < 300; ++i) { // ternary search for the barrier maximum
        const double m1 = lo + (hi - lo) / 3.0;
        const double m2 = hi - (hi - lo) / 3.0;
        if (rw_potential(m1, s, l) < rw_potential(m2, s, l)) {
            lo = m1;
        } else {
            hi = m2;
        }
    }
    return 0.5 * (lo + hi);
}

// Solve a 7x7 linear system by Gaussian elimination with partial pivoting.
inline std::array<double, 7> solve7(std::array<std::array<double, 7>, 7> A,
                                    std::array<double, 7> b) {
    for (int c = 0; c < 7; ++c) {
        int p = c;
        double best = std::abs(A[c][c]);
        for (int r = c + 1; r < 7; ++r) {
            if (std::abs(A[r][c]) > best) {
                best = std::abs(A[r][c]);
                p = r;
            }
        }
        std::swap(A[c], A[p]);
        std::swap(b[c], b[p]);
        const double piv = A[c][c];
        for (int jc = c; jc < 7; ++jc) {
            A[c][jc] /= piv;
        }
        b[c] /= piv;
        for (int r = 0; r < 7; ++r) {
            if (r != c) {
                const double fac = A[r][c];
                if (fac != 0.0) {
                    for (int jc = c; jc < 7; ++jc) {
                        A[r][jc] -= fac * A[c][jc];
                    }
                    b[r] -= fac * b[c];
                }
            }
        }
    }
    return b;
}

} // namespace detail

// Third-order WKB estimate of the Schwarzschild QNM M*omega for mode (s,l,n).
[[nodiscard]] inline QnmFrequency wkb3_schwarzschild(int s, int l, int n) {
    using namespace detail;
    QnmFrequency out;
    out.tier = core::TruthLabel::NumericalApproximation;
    out.cf_terms = 0;

    const double r0 = peak_radius(s, l);
    const double x0 = tortoise_x(r0);
    const double h = 0.02;

    // Fit V(x) by a degree-6 polynomial through 7 points centered on the peak;
    // the polynomial coefficients give the x-derivatives V0,V2..V6 at the peak.
    std::array<std::array<double, 7>, 7> A{};
    std::array<double, 7> V{};
    double rg = r0;
    for (int idx = 0; idx < 7; ++idx) {
        const int t = idx - 3;
        const double x = x0 + static_cast<double>(t) * h;
        rg = r_of_x(x, rg > 1.0 ? rg : r0);
        const double tv = static_cast<double>(t);
        double p = 1.0;
        for (int jc = 0; jc < 7; ++jc) {
            A[static_cast<std::size_t>(idx)][static_cast<std::size_t>(jc)] = p;
            p *= tv;
        }
        V[static_cast<std::size_t>(idx)] = rw_potential(rg, s, l);
    }
    const std::array<double, 7> c = solve7(A, V);
    const std::array<double, 7> fact = {1.0, 1.0, 2.0, 6.0, 24.0, 120.0, 720.0};
    auto Vk = [&](int k) {
        return c[static_cast<std::size_t>(k)] *
               fact[static_cast<std::size_t>(k)] /
               std::pow(h, static_cast<double>(k));
    };
    const double V0 = Vk(0);
    const double V2 = Vk(2);
    const double V3 = Vk(3);
    const double V4 = Vk(4);
    const double V5 = Vk(5);
    const double V6 = Vk(6);

    const double a = static_cast<double>(n) + 0.5;
    const double R = std::sqrt(-2.0 * V2);
    const double Lam =
        (1.0 / 8.0) * (V4 / V2) * (0.25 + a * a) -
        (1.0 / 288.0) * (V3 / V2) * (V3 / V2) * (7.0 + 60.0 * a * a);
    const double Om =
        (5.0 / 6912.0) * std::pow(V3 / V2, 4.0) * (77.0 + 188.0 * a * a) -
        (1.0 / 384.0) * (V3 * V3 * V4 / (V2 * V2 * V2)) *
            (51.0 + 100.0 * a * a) +
        (1.0 / 2304.0) * (V4 / V2) * (V4 / V2) * (67.0 + 68.0 * a * a) +
        (1.0 / 288.0) * (V3 * V5 / (V2 * V2)) * (19.0 + 28.0 * a * a) -
        (1.0 / 288.0) * (V6 / V2) * (5.0 + 4.0 * a * a);

    cx w2 = cx(V0 + Lam, 0.0) - cx(0.0, 1.0) * a * R * (1.0 + Om);
    cx w = std::sqrt(w2);
    if (w.real() < 0.0) {
        w = -w;
    }
    w /= 2.0; // M*omega
    out.re = w.real();
    out.im = w.imag();
    out.converged = true;
    return out;
}

} // namespace blackhole_ds::qnm
