// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/qnm/leaver.hpp
// Quasinormal-mode frequencies via Leaver's continued-fraction method.
//
// Truth tier: numerical_approximation (a numerical eigenvalue solver that
// converges to the exact classical perturbation spectrum to machine precision).
// Source: E. W. Leaver, "An analytic representation for the quasi-normal modes
// of Kerr black holes," Proc. R. Soc. Lond. A 402, 285 (1985). Coefficient form
// and cross-checks: Berti, Cardoso & Starinets, Class. Quantum Grav. 26, 163001
// (2009), arXiv:0905.2975.
//
// A quasinormal mode is a damped oscillation of a perturbed black hole. With
// the e^{-i omega t} convention a damped mode has Im(omega) < 0; results below
// are the dimensionless M*omega (geometric units G = c = 1). Internally the
// solver uses Leaver's 2M = 1 normalization and divides the continued-fraction
// root by two to report M*omega.
//
// The Schwarzschild solver here reproduces published reference values to better
// than 5e-7 (see tests/qnm_tests.cpp): e.g. s=2, l=2, n=0 -> 0.373672 -
// 0.088962 i; the n-th inversion yields overtones (n=1: 0.346711 - 0.273915 i).

#pragma once

#include <cmath>
#include <complex>

#include "blackhole_ds/core/truth_label.hpp"

namespace blackhole_ds::qnm {

using cx = std::complex<double>;

// A black-hole perturbation mode: field spin-weight s (0 scalar, 1 EM, 2
// gravitational), multipole l, azimuthal m, overtone index n.
struct Mode {
    int s{2};
    int l{2};
    int m{2};
    int n{0};
};

// A quasinormal frequency as the dimensionless M*omega = re + i*im (im < 0 for
// a damped mode), with solver diagnostics and a truth-tier label.
struct QnmFrequency {
    double re{0.0};
    double im{0.0};
    bool converged{false};
    int iterations{0};
    int cf_terms{0};
    core::TruthLabel tier{core::TruthLabel::NumericalApproximation};
};

namespace detail {

// Schwarzschild Leaver recurrence coefficients (units 2M = 1; field spin s,
// multipole l). The three-term recurrence is
//   alpha_n a_{n+1} + beta_n a_n + gamma_n a_{n-1} = 0,  a_0 = 1.
struct SchwCoeffs {
    cx C0, C1, C2, C3, C4;

    SchwCoeffs(cx w, int s, int l) {
        const double A = static_cast<double>(l) * static_cast<double>(l + 1) -
                         static_cast<double>(s) * static_cast<double>(s + 1);
        const cx i(0.0, 1.0);
        const double sd = static_cast<double>(s);
        C0 = -sd + 1.0 - 2.0 * i * w;
        C1 = -4.0 + 8.0 * i * w;
        C2 = sd + 3.0 - 4.0 * i * w;
        C3 = -sd - 1.0 + 4.0 * i * w + 8.0 * w * w - A;
        C4 = sd + 1.0 - 2.0 * (sd + 2.0) * i * w - 4.0 * w * w;
    }

    [[nodiscard]] cx alpha(int n) const {
        const double dn = static_cast<double>(n);
        return dn * dn + (C0 + 1.0) * dn + C0;
    }
    [[nodiscard]] cx beta(int n) const {
        const double dn = static_cast<double>(n);
        return -2.0 * dn * dn + (C1 + 2.0) * dn + C3;
    }
    [[nodiscard]] cx gamma(int n) const {
        const double dn = static_cast<double>(n);
        return dn * dn + (C2 - 3.0) * dn + C4 - C2 + 2.0;
    }
};

// Residual of the n-th-inverted continued fraction. A QNM frequency is a root.
// The finite "downward" tail (levels n-1..0) selects the n-th overtone; the
// infinite "upward" tail (levels n+1..N) is summed by backward recurrence.
[[nodiscard]] inline cx schw_cf_residual(cx w, int s, int l, int n, int N) {
    const SchwCoeffs c(w, s, l);
    cx down(0.0, 0.0);
    if (n > 0) {
        cx den = c.beta(0);
        for (int j = 1; j < n; ++j) {
            den = c.beta(j) - c.alpha(j - 1) * c.gamma(j) / den;
        }
        down = c.alpha(n - 1) * c.gamma(n) / den;
    }
    cx up(0.0, 0.0);
    for (int k = N; k >= n; --k) {
        up = c.alpha(k) * c.gamma(k + 1) / (c.beta(k + 1) - up);
    }
    return c.beta(n) - down - up;
}

// Complex secant root-finder for the continued-fraction residual.
[[nodiscard]] inline cx secant(cx (*f)(cx, int, int, int, int), cx x0, cx x1,
                               int s, int l, int n, int N, bool& converged,
                               int& iterations) {
    cx f0 = f(x0, s, l, n, N);
    cx f1 = f(x1, s, l, n, N);
    cx x = x1;
    converged = false;
    for (int it = 0; it < 200; ++it) {
        iterations = it + 1;
        if (std::abs(f1 - f0) < 1e-300) {
            break;
        }
        const cx x2 = x1 - f1 * (x1 - x0) / (f1 - f0);
        if (std::abs(x2 - x1) < 1e-13) {
            x = x2;
            converged = true;
            break;
        }
        x0 = x1;
        f0 = f1;
        x1 = x2;
        f1 = f(x2, s, l, n, N);
        x = x2;
    }
    if (!converged) {
        converged = std::abs(f1) < 1e-8;
    }
    return x;
}

} // namespace detail

// Schwarzschild quasinormal frequency M*omega for the (s, l, n) mode, by
// Leaver's continued fraction with N terms. An optional seed (as M*omega)
// overrides the default; the default works for the low-lying gravitational
// spectrum. l must satisfy l >= |s| and n >= 0.
[[nodiscard]] inline QnmFrequency
schwarzschild_qnm(int s, int l, int n, int N = 400, cx seed_Mw = cx(0.0, 0.0)) {
    QnmFrequency out;
    out.cf_terms = N;
    if (seed_Mw == cx(0.0, 0.0)) {
        if (n == 0) {
            // eikonal real part, fundamental-scale imaginary part
            seed_Mw = cx((static_cast<double>(l) + 0.5) * 0.19245, -0.0890);
        } else {
            // n-continuation: seed from the (n-1) mode, shifted in damping, so
            // the secant lands on the n-th overtone rather than a neighbour.
            const QnmFrequency prev = schwarzschild_qnm(s, l, n - 1, N);
            seed_Mw = cx(prev.re, prev.im - 0.19);
        }
    }
    const cx w0 = 2.0 * seed_Mw; // Leaver 2M=1 units
    const cx w1 = w0 * 1.0001 + cx(1e-4, 0.0);
    cx w = detail::secant(&detail::schw_cf_residual, w0, w1, s, l, n, N,
                          out.converged, out.iterations);
    w /= 2.0; // back to M*omega
    out.re = w.real();
    out.im = w.imag();
    return out;
}

// --- Kerr: Leaver's coupled angular + radial continued fractions ------------
// Kerr QNMs require simultaneously solving an angular recurrence (spin-weighted
// spheroidal harmonics -> separation constant A_lm) and a radial recurrence
// (the QNM condition), coupled through A_lm and omega. Units 2M=1, spin
// a_Leaver = j/2 with j = a/M the dimensionless spin; gravitational s = -2.
// Coefficients verified to reproduce the qnm-package / Berti-Cardoso-Will
// reference values to < 1e-6 across spins (see tests/qnm_tests.cpp).
namespace detail {

inline double ang_k1(int s, int m) {
    return std::abs(static_cast<double>(m - s)) / 2.0;
}
inline double ang_k2(int s, int m) {
    return std::abs(static_cast<double>(m + s)) / 2.0;
}

inline cx ang_alpha(int n, int s, int m) {
    const double k1 = ang_k1(s, m);
    const double dn = static_cast<double>(n);
    return cx(-2.0 * (dn + 1.0) * (dn + 2.0 * k1 + 1.0), 0.0);
}
inline cx ang_beta(int n, cx aw, int s, int m, cx A) {
    const double k1 = ang_k1(s, m);
    const double k2 = ang_k2(s, m);
    const double dn = static_cast<double>(n);
    const double sd = static_cast<double>(s);
    return dn * (dn - 1.0) + 2.0 * dn * (k1 + k2 + 1.0 - 2.0 * aw) -
           (2.0 * aw * (2.0 * k1 + sd + 1.0) - (k1 + k2) * (k1 + k2 + 1.0)) -
           (aw * aw + sd * (sd + 1.0) + A);
}
inline cx ang_gamma(int n, cx aw, int s, int m) {
    const double k1 = ang_k1(s, m);
    const double k2 = ang_k2(s, m);
    const double dn = static_cast<double>(n);
    const double sd = static_cast<double>(s);
    return 2.0 * aw * (dn + k1 + k2 + sd);
}

inline cx ang_cf_residual(cx A, cx aw, int s, int m, int Nang) {
    cx f = ang_beta(Nang, aw, s, m, A);
    for (int n = Nang - 1; n >= 0; --n) {
        const cx ag = ang_alpha(n, s, m) * ang_gamma(n + 1, aw, s, m);
        f = ang_beta(n, aw, s, m, A) - ag / f;
    }
    return f;
}

// Solve the angular CF for the separation constant A_lm given c = a*omega,
// starting from A_seed (the spherical value or the previous spin step).
inline cx kerr_separation_constant(cx aw, int s, int m, cx A_seed, int Nang) {
    cx A0 = A_seed;
    cx A1 = A0 + cx(1e-4, 1e-4);
    cx f0 = ang_cf_residual(A0, aw, s, m, Nang);
    cx f1 = ang_cf_residual(A1, aw, s, m, Nang);
    for (int it = 0; it < 100; ++it) {
        const cx d = f1 - f0;
        if (std::abs(d) < 1e-300) {
            break;
        }
        const cx A2 = A1 - f1 * (A1 - A0) / d;
        A0 = A1;
        f0 = f1;
        A1 = A2;
        f1 = ang_cf_residual(A1, aw, s, m, Nang);
        if (std::abs(A1 - A0) < 1e-12) {
            break;
        }
    }
    return A1;
}

// Leaver radial constants c0..c4 (units 2M=1). q = omega/2 - a*m; the gamma
// recurrence carries the (- c2 + 2) shift so a->0 matches Schwarzschild.
struct KerrRadialConsts {
    cx c0, c1, c2, c3, c4;
    KerrRadialConsts(double a, int m, cx w, int s, cx A) {
        const cx b = std::sqrt(cx(1.0 - 4.0 * a * a, 0.0)); // r_+ - r_-
        const double sd = static_cast<double>(s);
        const double md = static_cast<double>(m);
        const cx I(0.0, 1.0);
        const cx q = w / 2.0 - a * md;
        c0 = 1.0 - sd - I * w - (2.0 * I / b) * q;
        c1 = -4.0 + 2.0 * I * w * (2.0 + b) + (4.0 * I / b) * q;
        c2 = sd + 3.0 - 3.0 * I * w - (2.0 * I / b) * q;
        c3 = w * w * (4.0 + 2.0 * b - a * a) - 2.0 * a * md * w - sd - 1.0 +
             (2.0 + b) * I * w - A + ((4.0 * w + 2.0 * I) / b) * q;
        c4 = sd + 1.0 - 2.0 * w * w - (2.0 * sd + 3.0) * I * w -
             ((4.0 * w + 2.0 * I) / b) * q;
    }
    [[nodiscard]] cx alpha(int n) const {
        const double dn = static_cast<double>(n);
        return dn * dn + (c0 + 1.0) * dn + c0;
    }
    [[nodiscard]] cx beta(int n) const {
        const double dn = static_cast<double>(n);
        return -2.0 * dn * dn + (c1 + 2.0) * dn + c3;
    }
    [[nodiscard]] cx gamma(int n) const {
        const double dn = static_cast<double>(n);
        return dn * dn + (c2 - 3.0) * dn + (c4 - c2 + 2.0);
    }
};

inline cx kerr_radial_cf_residual(double a, int m, cx w, int s, cx A,
                                  int Nrad) {
    const KerrRadialConsts c(a, m, w, s, A);
    cx f = c.beta(Nrad);
    for (int n = Nrad - 1; n >= 0; --n) {
        const cx ag = c.alpha(n) * c.gamma(n + 1);
        f = c.beta(n) - ag / f;
    }
    return f;
}

} // namespace detail

// Kerr fundamental (n = 0) quasinormal frequency M*omega for the gravitational
// (s = -2) l, m mode at dimensionless spin j = a/M in [0, 1). Marches a spin
// continuation from the Schwarzschild value (computed by the independent
// schwarzschild_qnm solver above) up to j, re-seeding omega and A_lm each step
// so the secant stays on the (l, m, n=0) branch. Overtones (n > 0) are not yet
// supported (they require the n-th radial inversion); n is accepted for API
// symmetry and must be 0.
[[nodiscard]] inline QnmFrequency kerr_qnm(int s, int l, int m, int n, double j,
                                           int Nang = 150, int Nrad = 500,
                                           double dj = 0.05) {
    QnmFrequency out;
    out.cf_terms = Nrad;
    if (n != 0) {
        out.converged = false; // unsupported; caller should check
        return out;
    }
    const int abs_s = s < 0 ? -s : s;
    const QnmFrequency seed = schwarzschild_qnm(abs_s, l, n);
    cx w = 2.0 * cx(seed.re, seed.im);
    cx Aw = cx(static_cast<double>(l) * static_cast<double>(l + 1) -
                   static_cast<double>(s) * static_cast<double>(s + 1),
               0.0);

    int nsteps = 0;
    if (j != 0.0) {
        nsteps = static_cast<int>(std::lround(j / dj));
        if (nsteps < 1) {
            nsteps = 1;
        }
    }

    bool conv = true;
    for (int k = 0; k <= nsteps; ++k) {
        const double jv = (nsteps == 0) ? 0.0
                                        : j * static_cast<double>(k) /
                                              static_cast<double>(nsteps);
        const double a = jv / 2.0; // Leaver spin
        auto resid = [&](cx ww) -> cx {
            const cx aw = a * ww;
            Aw = detail::kerr_separation_constant(aw, s, m, Aw, Nang);
            return detail::kerr_radial_cf_residual(a, m, ww, s, Aw, Nrad);
        };
        cx w0 = w;
        cx w1 = w0 * (1.0 + 1e-5) + cx(0.0, 1e-6);
        cx f0 = resid(w0);
        cx f1 = resid(w1);
        conv = false;
        for (int it = 0; it < 200; ++it) {
            out.iterations = it + 1;
            const cx d = f1 - f0;
            if (std::abs(d) < 1e-300) {
                break;
            }
            const cx w2 = w1 - f1 * (w1 - w0) / d;
            w0 = w1;
            f0 = f1;
            w1 = w2;
            f1 = resid(w1);
            if (std::abs(w1 - w0) < 1e-13) {
                conv = true;
                break;
            }
        }
        w = w1;
    }
    out.converged = conv;
    const cx Mw = w / 2.0;
    out.re = Mw.real();
    out.im = Mw.imag();
    return out;
}

} // namespace blackhole_ds::qnm
