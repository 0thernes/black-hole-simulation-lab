// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/kerr_metric_cross_check_tests.cpp
//
// INDEPENDENT verification oracle for the Kerr geodesic kernel.
//
// The kernel (geodesics/kerr_geodesic.hpp) integrates Kerr null geodesics using
// the Carter-SEPARATED constants of motion (the R(r), Theta(theta) potentials
// and the cyclic dphi/dlambda, dt/dlambda first integrals). That separated form
// is derived analytically and is the most error-prone physics in the project
// (a sign or coefficient slip in R/Theta would be silent). This test validates
// it against a COMPLETELY INDEPENDENT object: the full Boyer-Lindquist Kerr
// metric tensor g_munu, implemented from scratch here (MTW / Chandrasekhar).
//
// The check: for a photon, the 4-velocity must be NULL --
//     g_munu u^mu u^nu = 0.
// We take u^mu = dx^mu/dlambda straight from the kernel (sqrt(R), sqrt(Theta),
// and GeodesicDeriv's dt/dlambda, dphi/dlambda) and contract it with the
// independently-coded metric. Mino time only rescales u by Sigma, and 0 *
// Sigma^2 is still 0, so the null condition is parametrization-robust. The test
// passes ONLY if BOTH the separated potentials AND this metric are correct, so
// a slip in either is caught (compensating slips in both are vanishingly
// unlikely).
//
// This realizes the one genuinely useful idea from the Tsotchke study
// (docs/integrations/TSOTCHKE_ECOSYSTEM.md): use a metric-based differential-
// geometry computation (cf. quantum_geometric_tensor's differential_geometry.c)
// as a READ-ONLY verification oracle, never as a runtime dependency.

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/geodesics/kerr_geodesic.hpp"

namespace kg = blackhole_ds::geodesics::kerr;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

// --- Independent Boyer-Lindquist Kerr metric (M = 1, signature -+++). --------
// Standard form (Misner-Thorne-Wheeler 1973, eq. 33.2; Chandrasekhar 1983).
struct Metric {
    double tt, rr, thth, phph, tph; // nonzero covariant components
};

[[nodiscard]] Metric kerr_metric(double r, double theta, double a) {
    const double s2 = std::sin(theta) * std::sin(theta);
    const double c = std::cos(theta);
    const double Sigma = r * r + a * a * c * c;
    const double Delta = r * r - 2.0 * r + a * a;
    Metric g{};
    g.tt = -(1.0 - 2.0 * r / Sigma);
    g.rr = Sigma / Delta;
    g.thth = Sigma;
    g.phph = (r * r + a * a + 2.0 * r * a * a * s2 / Sigma) * s2;
    g.tph = -2.0 * r * a * s2 / Sigma;
    return g;
}

// Contract g_munu u^mu u^nu for u = (u^t, u^r, u^theta, u^phi).
[[nodiscard]] double null_norm(const Metric& g, double ut, double ur,
                               double uth, double uph) {
    return g.tt * ut * ut + 2.0 * g.tph * ut * uph + g.rr * ur * ur +
           g.thth * uth * uth + g.phph * uph * uph;
}

// Build the kernel's Mino-time 4-velocity at (r, theta) for constants k, with a
// chosen radial sign. Returns false if (r, theta) is outside the allowed region
// (R < 0 or Theta < 0), in which case there is no real photon there.
bool kernel_four_velocity(double r, double theta,
                          const kg::GeodesicConstants& k, double radial_sign,
                          double& ut, double& ur, double& uth, double& uph) {
    const double R = kg::kg_R(r, k);
    const double Th = kg::kg_Theta(theta, k);
    // A small negative tolerance: at theta = pi/2, cos(pi/2) is ~6e-17 (not
    // exactly 0) in floating point, so Theta can be a tiny negative for an
    // equatorial orbit. Treat tiny-negative as the boundary (0).
    if (R < -1e-9 || Th < -1e-9) {
        return false;
    }
    ur = radial_sign * std::sqrt(R > 0.0 ? R : 0.0);
    uth = std::sqrt(Th > 0.0 ? Th : 0.0); // sign irrelevant to the contraction
    // dt/dlambda and dphi/dlambda from the kernel's own equations of motion.
    kg::KGState y{};
    y[kg::KG_R] = r;
    y[kg::KG_TH] = theta;
    y[kg::KG_RDOT] = ur;
    y[kg::KG_THDOT] = uth;
    const kg::KGState d = kg::GeodesicDeriv{k}(0.0, y);
    ut = d[kg::KG_T];
    uph = d[kg::KG_PHI];
    return true;
}

// A photon's 4-velocity from the kernel must, per the INDEPENDENT metric:
//   (a) be null:  g_munu u^mu u^nu = 0; and
//   (b) reproduce the conserved energy E = -p_t and axial angular momentum
//       L_z = p_phi, where p_mu = g_mu_nu u^nu / Sigma (the kernel works in
//       Mino time lambda with dzeta = Sigma dlambda, so the affine momentum is
//       the Mino 4-velocity divided by Sigma). (b) tests the kernel's
//       dt/dlambda and dphi/dlambda equations of motion, not just the
//       constraint -- a stronger, independent check of the separated form.
void check_photon(double a, double E, double Lz, double Q, double r,
                  double theta, const char* label) {
    kg::GeodesicConstants k{a, E, Lz, Q};
    double ut, ur, uth, uph;
    if (!kernel_four_velocity(r, theta, k, -1.0, ut, ur, uth, uph)) {
        std::fprintf(stderr, "SKIP %s: no real photon at r=%.2f theta=%.2f\n",
                     label, r, theta);
        return;
    }
    const Metric g = kerr_metric(r, theta, a);

    // (a) Null condition. Scale by the largest term for a relative tolerance
    // (components are O(r^2) ~ O(1e2-1e4) and cancel to ~0).
    const double n = null_norm(g, ut, ur, uth, uph);
    const double scale = std::abs(g.rr * ur * ur) + std::abs(g.tt * ut * ut) +
                         std::abs(g.phph * uph * uph) + 1.0;
    if (std::abs(n) / scale > 1e-9) {
        std::fprintf(stderr, "FAIL null %s: |g_uu|/scale = %.3e (n=%.6e)\n",
                     label, std::abs(n) / scale, n);
        ++failures;
    }

    // (b) Conserved-quantity recovery via the metric (p_r, p_theta do not enter
    // p_t or p_phi because g has no t-r/t-theta/phi-r/phi-theta components).
    const double cth = std::cos(theta);
    const double Sigma = r * r + a * a * cth * cth;
    const double E_rec = -(g.tt * ut + g.tph * uph) / Sigma;
    const double Lz_rec = (g.tph * ut + g.phph * uph) / Sigma;
    if (std::abs(E_rec - E) > 1e-8) {
        std::fprintf(stderr, "FAIL E %s: recovered %.9f vs %.9f\n", label,
                     E_rec, E);
        ++failures;
    }
    if (std::abs(Lz_rec - Lz) > 1e-8) {
        std::fprintf(stderr, "FAIL Lz %s: recovered %.9f vs %.9f\n", label,
                     Lz_rec, Lz);
        ++failures;
    }
}

} // namespace

int main() {
    // Equatorial photons (Q = 0, theta = pi/2) across spin and radius.
    check_photon(0.0, 1.0, 4.0, 0.0, 8.0, kg::kg_pi / 2.0, "schwarzschild-eq");
    check_photon(0.6, 1.0, 2.0, 0.0, 10.0, kg::kg_pi / 2.0, "kerr-eq-a0.6");
    check_photon(0.9, 1.0, -3.0, 0.0, 6.0, kg::kg_pi / 2.0,
                 "kerr-eq-retrograde");
    check_photon(0.99, 1.0, 1.5, 0.0, 4.0, kg::kg_pi / 2.0,
                 "kerr-eq-near-extremal");

    // Non-equatorial photons (Q > 0, theta != pi/2) -- exercises Theta and the
    // a^2 cos^2 theta terms in both the kernel and the metric.
    check_photon(0.6, 1.0, 2.0, 3.0, 12.0, kg::kg_pi / 3.0,
                 "kerr-offplane-a0.6");
    check_photon(0.8, 1.0, 1.0, 5.0, 15.0, kg::kg_pi / 4.0,
                 "kerr-offplane-a0.8");
    check_photon(0.5, 1.0, 0.0, 9.0, 20.0, kg::kg_pi / 6.0, "kerr-polar-Lz0");

    // The a -> 0 metric must reduce to Schwarzschild: g_tt = -(1-2/r),
    // g_rr = 1/(1-2/r), g_phph = r^2 sin^2, g_tph = 0.
    {
        const Metric g = kerr_metric(10.0, kg::kg_pi / 2.0, 0.0);
        CHECK(std::abs(g.tt - (-(1.0 - 2.0 / 10.0))) < 1e-12);
        CHECK(std::abs(g.rr - 1.0 / (1.0 - 2.0 / 10.0)) < 1e-12);
        CHECK(std::abs(g.phph - 100.0) < 1e-12);
        CHECK(std::abs(g.tph) < 1e-15);
    }

    if (failures == 0) {
        std::puts("kerr_metric_cross_check_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "kerr_metric_cross_check_tests: %d failure(s)\n",
                 failures);
    return EXIT_FAILURE;
}
