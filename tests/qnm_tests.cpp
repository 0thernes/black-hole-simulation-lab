// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/qnm_tests.cpp
//
// Validation of the quasinormal-mode (QNM) module against an INDEPENDENT
// oracle: published reference frequencies cross-checked across Berti's ringdown
// data, Leo Stein's `qnm` package (Cook-Zalutskiy spectral method), and the
// Konoplya tables. The Leaver continued-fraction solver must reproduce them to
// high precision; the WKB and Berti-Cardoso-Will methods are looser
// cross-checks.
//
// All frequencies are the dimensionless M*omega = re + i*im (im < 0, damped),
// e^{-i omega t} convention.

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/qnm/kerr_fit.hpp"
#include "blackhole_ds/qnm/leaver.hpp"
#include "blackhole_ds/qnm/spectroscopy.hpp"
#include "blackhole_ds/qnm/wkb.hpp"

namespace q = blackhole_ds::qnm;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close_abs(double a, double b, double tol) {
    return std::abs(a - b) <= tol;
}
bool close_rel(double a, double b, double rel) {
    return std::abs(a - b) <= rel * std::abs(b);
}

} // namespace

int main() {
    // --- Schwarzschild Leaver continued fraction vs published oracle ---------
    struct Sch {
        int s, l, n;
        double re, im;
    };
    const std::array<Sch, 7> sch = {{
        {2, 2, 0, 0.373672, -0.088962}, // gravitational fundamental
        {2, 2, 1, 0.346711, -0.273915}, // first overtone (n-th inversion)
        {2, 2, 2, 0.301053, -0.478277}, // second overtone
        {2, 3, 0, 0.599443, -0.092703},
        {2, 4, 0, 0.809178, -0.094164},
        {0, 0, 0, 0.110455, -0.104896}, // scalar
        {1, 1, 0, 0.248263, -0.092488}, // electromagnetic
    }};
    for (const auto& m : sch) {
        const q::QnmFrequency r = q::schwarzschild_qnm(m.s, m.l, m.n);
        CHECK(r.converged);
        CHECK(close_abs(r.re, m.re, 1e-4));
        CHECK(close_abs(r.im, m.im, 1e-4));
    }

    // --- Kerr Leaver (coupled angular+radial CF) vs oracle, l=m=2, n=0 -------
    struct Ker {
        double j, re, im;
    };
    const std::array<Ker, 4> ker = {{
        {0.0, 0.373672, -0.088962}, // Schwarzschild limit
        {0.5, 0.464123, -0.085639},
        {0.9, 0.671614, -0.064869},
        {0.98, 0.825429, -0.038630},
    }};
    for (const auto& m : ker) {
        const q::QnmFrequency r = q::kerr_qnm(-2, 2, 2, 0, m.j);
        CHECK(r.converged);
        CHECK(close_abs(r.re, m.re, 1e-4));
        CHECK(close_abs(r.im, m.im, 1e-4));
    }

    // --- Cross-check: the two INDEPENDENT solvers agree at a=0
    // ---------------- schwarzschild_qnm uses the Regge-Wheeler s=+2
    // convention; kerr_qnm uses the Teukolsky s=-2 spin weight. The spectrum
    // depends on s^2, so both must give the identical root -- a strong
    // consistency test of both codes.
    {
        const q::QnmFrequency a = q::schwarzschild_qnm(2, 2, 0);
        const q::QnmFrequency b = q::kerr_qnm(-2, 2, 2, 0, 0.0);
        CHECK(close_abs(a.re, b.re, 1e-6));
        CHECK(close_abs(a.im, b.im, 1e-6));
    }

    // --- WKB 3rd order: independent method, loose cross-check
    // -----------------
    {
        const q::QnmFrequency r = q::wkb3_schwarzschild(2, 2, 0);
        CHECK(close_rel(r.re, 0.373672, 0.005)); // real to <0.5%
        CHECK(
            close_rel(r.im, -0.088962, 0.06)); // imag to <6% (3rd-order limit)
    }
    {
        const q::QnmFrequency r = q::wkb3_schwarzschild(2, 3, 0);
        CHECK(close_rel(r.re, 0.599443, 0.003));
        CHECK(close_rel(r.im, -0.092703, 0.02));
    }

    // --- Berti-Cardoso-Will Kerr fit: loose cross-check (~1-2%)
    // ---------------
    {
        const q::QnmFrequency r = q::bcw_kerr_220(0.9);
        CHECK(close_rel(r.re, 0.671614, 0.02));
        CHECK(close_rel(r.im, -0.064869, 0.02));
        // The fit and the exact Leaver value should agree to a couple percent.
        const q::QnmFrequency exact = q::kerr_qnm(-2, 2, 2, 0, 0.9);
        CHECK(close_rel(r.re, exact.re, 0.02));
    }

    // --- Physical conversion: t_g(1 Msun) ~ 4.9255e-6 s
    // -----------------------
    CHECK(close_abs(q::light_crossing_time_s(1.0), 4.9255e-6, 1e-8));

    // --- GW150914-like remnant (~62 Msun, j~0.69): f_220 ~ 250 Hz, tau ~ 4 ms
    // -
    {
        const std::array<q::RingdownMode, 4> sp =
            q::ringdown_spectrum(62.0, 0.69);
        CHECK(sp[0].converged);
        CHECK(sp[0].mode.l == 2 && sp[0].mode.m == 2);
        CHECK(sp[0].frequency_hz > 200.0 && sp[0].frequency_hz < 330.0);
        CHECK(sp[0].damping_time_s > 1e-3 && sp[0].damping_time_s < 8e-3);
        // Higher multipoles ring faster (higher frequency) than the dominant
        // mode.
        CHECK(sp[1].frequency_hz > sp[0].frequency_hz); // (3,3,0) > (2,2,0)
        CHECK(sp[2].frequency_hz > sp[1].frequency_hz); // (4,4,0) > (3,3,0)
    }

    // --- No-hair fingerprint: mass-independent ratios, monotone in spin
    // -------
    {
        const q::NoHairSignature lo = q::no_hair_signature(0.1);
        const q::NoHairSignature hi = q::no_hair_signature(0.9);
        CHECK(lo.f330_over_f220 > 1.0); // (3,3,0) above (2,2,0)
        CHECK(hi.f330_over_f220 > 1.0);
        CHECK(hi.q_220 > lo.q_220); // higher spin -> longer-lived -> higher Q
        CHECK(lo.f210_over_f220 < 1.0); // (2,1,0) below (2,2,0)
    }

    if (failures == 0) {
        std::puts("qnm_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "qnm_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
