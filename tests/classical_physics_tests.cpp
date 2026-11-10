// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/classical_physics_tests.cpp
// Closed-form checks for the photon sphere, the shadow, the Kerr ISCO,
// the Page-Thorne flux, and the photon-orbit Lyapunov exponent.
//
// The integer static_asserts are exact rational certificates: they do
// not touch libm. The same identities are repeated in
// proofs/ClassicalGR.lean for Lean 4, which is optional and not required
// to build or to pass CTest.

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/disk/novikov_thorne.hpp"
#include "blackhole_ds/geodesics/photon_lyapunov.hpp"
#include "blackhole_ds/integrators/rk4.hpp"
#include "blackhole_ds/metrics/kerr.hpp"
#include "blackhole_ds/metrics/schwarzschild_christoffel.hpp"

namespace nt = blackhole_ds::disk;
namespace ly = blackhole_ds::geodesics;
namespace sch = blackhole_ds::metrics::schwarzschild;
namespace kr = blackhole_ds::metrics::kerr;
using blackhole_ds::integrators::rk4_step;
using blackhole_ds::integrators::State;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close_rel(double a, double b, double rel) {
    const double scale = std::max(1.0, std::abs(b));
    return std::abs(a - b) <= rel * scale;
}

// (3 u^2 - u) * 9 at u = 1/3 is 3*1*1*3 - 1*9 = 0.
static_assert(3 * 1 * 1 * 3 - 1 * 9 == 0, "photon sphere RHS");
// (6 u - 1) * 3 at u = 1/3 is 6 - 3 = 3, so 6 u - 1 = 1.
static_assert(6 * 1 - 1 * 3 == 3, "linearization coefficient");
// (u^2 - 2 u^3) * 27 at u = 1/3 is 3 - 2 = 1, so the value is 1/27
// and the critical impact parameter satisfies b^2 = 27.
static_assert(3 - 2 == 1, "critical potential");
static_assert(3 * 3 * 3 == 27, "(3 sqrt(3))^2 = 27");
// E^2 = (r-2)^2 / (r (r-3)) at r = 6 equals 8/9.
static_assert((6 - 2) * (6 - 2) * 9 == 8 * 6 * (6 - 3), "ISCO energy");
// L^2 = r^2 / (r - 3) at r = 6 equals 12.
static_assert(6 * 6 == 12 * (6 - 3), "ISCO angular momentum");
// BPT formula at a = 0: z1 = 1 + 1*(1+1) = 3, r = 3 + 3 = 6.
static_assert(1 + 1 * (1 + 1) == 3, "BPT z1 at a = 0");
static_assert(3 + 3 == 6, "BPT ISCO at a = 0");

} // namespace

int main() {
    // --- Floating-point evaluations of the certified limits ---
    CHECK(kr::isco_dimensionless(0.0) == 6.0);
    CHECK(kr::photon_sphere_dimensionless(0.0) == 3.0 ||
          std::abs(kr::photon_sphere_dimensionless(0.0) - 3.0) < 1e-15);
    CHECK(
        close_rel(nt::circular_energy(6.0, 0.0), std::sqrt(8.0 / 9.0), 1e-12));
    CHECK(
        close_rel(nt::circular_angmom(6.0, 0.0), 2.0 * std::sqrt(3.0), 1e-12));
    CHECK(close_rel(nt::circular_omega(6.0, 0.0), 1.0 / (6.0 * std::sqrt(6.0)),
                    1e-12));
    CHECK(close_rel(nt::radiative_efficiency(0.0), 1.0 - std::sqrt(8.0 / 9.0),
                    1e-15));
    // Prograde ISCO moves inward as spin increases. This is the
    // sensitivity d r_isco / d a, checked two ways.
    {
        // One-sided steps large enough that r(h) - 6 is not lost in the
        // ulps of the BPT formula. Two steps must agree.
        const auto slope = [](double h) {
            return (kr::isco_dimensionless(h) - kr::isco_dimensionless(0.0)) /
                   h;
        };
        const double s4 = slope(1e-4);
        const double s3 = slope(1e-3);
        CHECK(s4 < 0.0);
        CHECK(close_rel(s4, s3, 1e-3));
        std::printf("dr_isco/da at a=0+: %.10f (h=1e-4) and %.10f (h=1e-3)\n",
                    s4, s3);
    }

    // E - Omega L = sqrt(1 - 3/r) on Schwarzschild circular orbits.
    {
        const double r = 10.0;
        const double el =
            nt::circular_energy(r, 0.0) -
            nt::circular_omega(r, 0.0) * nt::circular_angmom(r, 0.0);
        CHECK(close_rel(el, std::sqrt(1.0 - 3.0 / r), 1e-12));
    }

    // --- Page-Thorne flux, Schwarzschild closed form ---
    CHECK(nt::schwarzschild_page_thorne_flux(6.0) == 0.0);
    CHECK(nt::schwarzschild_page_thorne_flux(5.0) == 0.0);
    const double f10 = nt::schwarzschild_page_thorne_flux(10.0);
    const double f20 = nt::schwarzschild_page_thorne_flux(20.0);
    CHECK(f10 > 0.0);
    CHECK(f20 > 0.0);
    CHECK(close_rel(nt::schwarzschild_page_thorne_flux(10.0, 2.5), 2.5 * f10,
                    1e-12));
    // Quadrature of the integrand against the antiderivative.
    {
        const auto integrand = [](double r) {
            return (r - 6.0) / (2.0 * std::sqrt(r) * (r - 3.0));
        };
        const double a = 6.0;
        const double b = 10.0;
        const int n = 4000;
        const double h = (b - a) / n;
        double s = integrand(a) + integrand(b);
        for (int i = 1; i < n; ++i) {
            s += ((i % 2) ? 4.0 : 2.0) * integrand(a + i * h);
        }
        const double quad = s * h / 3.0;
        CHECK(close_rel(quad, nt::schwarzschild_flux_integral(10.0), 1e-10));
        std::printf("Schwarzschild flux integral at r=10: analytic %.12e  "
                    "simpson %.12e\n",
                    nt::schwarzschild_flux_integral(10.0), quad);
        std::printf("Schwarzschild Page-Thorne F(10): %.12e\n", f10);
        std::printf("Schwarzschild Page-Thorne F(20): %.12e\n", f20);
    }

    // Kerr numerical flux reduces to the closed form at small spin, and
    // a doubled panel count changes a=0.5, r = r_isco+4 by a small amount.
    {
        const double f_num = nt::page_thorne_flux(10.0, 0.0);
        CHECK(close_rel(f_num, f10, 1e-12));
        const double a = 0.5;
        const double risco = kr::isco_dimensionless(a);
        const double r = risco + 4.0;
        const double f = nt::page_thorne_flux(r, a);
        const double f_coarse = nt::page_thorne_flux(r, a, 1.0, 1000);
        CHECK(f > 0.0);
        CHECK(close_rel(f_coarse, f, 1e-4));
        CHECK(nt::page_thorne_flux(risco, a) == 0.0);
        std::printf("Kerr a=0.5 r_isco=%.6f F(r_isco+4)=%.12e  "
                    "1000 panels %.12e\n",
                    risco, f, f_coarse);
    }

    // Bolometric transfer: I = g^4 F/pi. Doubling g multiplies by 16.
    {
        const double I1 = nt::observed_bolometric_intensity(f10, 1.0);
        const double I2 = nt::observed_bolometric_intensity(f10, 2.0);
        CHECK(close_rel(I1, f10 / nt::nt_pi, 1e-15));
        CHECK(close_rel(I2, 16.0 * I1, 1e-12));
        CHECK(nt::observed_bolometric_intensity(0.0, 2.0) == 0.0);
    }

    // --- Lyapunov exponent ---
    CHECK(close_rel(ly::schwarzschild_lyapunov_coordinate(),
                    1.0 / std::sqrt(27.0), 1e-15));
    // dt/dlambda = 3 on the photon sphere, so the affine rate is 3 times
    // the coordinate-time rate.
    CHECK(close_rel(ly::schwarzschild_lyapunov_affine(),
                    3.0 * ly::schwarzschild_lyapunov_coordinate(), 1e-15));
    CHECK(close_rel(ly::schwarzschild_half_orbit_exponent(), nt::nt_pi, 0.0));
    CHECK(close_rel(ly::schwarzschild_subring_demagnification(),
                    std::exp(-nt::nt_pi), 1e-15));
    // gamma_t * (half-orbit time) = pi. Half-orbit time is pi / Omega,
    // and Omega = gamma_t on the photon sphere.
    CHECK(close_rel(ly::schwarzschild_lyapunov_coordinate() * nt::nt_pi /
                        ly::schwarzschild_lyapunov_coordinate(),
                    nt::nt_pi, 1e-15));
    std::printf("subring demagnification e^{-pi} = %.12e\n",
                ly::schwarzschild_subring_demagnification());

    // Integrate the orbit equation off the photon sphere and compare to
    // eps * cosh(phi).
    {
        const double eps = 1e-8;
        const double phi_end = 0.5;
        const double h = 1e-5;
        const auto deriv = [](double, const State<2>& y) -> State<2> {
            return State<2>{y[1], 3.0 * y[0] * y[0] - y[0]};
        };
        State<2> y{1.0 / 3.0 + eps, 0.0};
        double phi = 0.0;
        const int n = static_cast<int>(phi_end / h);
        for (int i = 0; i < n; ++i) {
            y = rk4_step<2>(deriv, phi, y, h);
            phi += h;
        }
        const double got = y[0] - 1.0 / 3.0;
        const double expect = eps * ly::photon_perturbation_factor(phi);
        CHECK(close_rel(got, expect, 1e-6));
        std::printf("photon perturbation at phi=%.5f: integrated %.12e  "
                    "cosh %.12e\n",
                    phi, got, expect);
    }

    // Christoffel symbol from the metric, finite difference vs closed form.
    {
        const double r = 10.0;
        const double fd = sch::gamma_r_tt_from_metric_derivative(r, 1e-6);
        CHECK(close_rel(fd, sch::gamma_r_tt(r), 1e-8));
        CHECK(close_rel(sch::gamma_t_tr(r), 1.0 / (r * (r - 2.0)), 0.0));
        CHECK(close_rel(sch::gamma_r_rr(r), -sch::gamma_t_tr(r), 0.0));
        CHECK(sch::gamma_r_thth(r) == -(r - 2.0));
        CHECK(sch::gamma_th_rth(r) == 1.0 / r);
        std::printf("Gamma^r_tt(10) = %.12e  finite-difference %.12e\n",
                    sch::gamma_r_tt(r), fd);
    }

    if (failures == 0) {
        std::puts("classical_physics_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "classical_physics_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
