// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// tests/kerr_geodesic_tests.cpp
// Validates the Kerr null-geodesic integrator (geodesics/kerr_geodesic.hpp).
//
// The integrator is verified the way GR integrators MUST be verified: by the
// conserved quantities. Along a correctly integrated geodesic the Carter
// constant Q and the on-shell residuals (dr/dlambda)^2 - R(r) and
// (dtheta/dlambda)^2 - Theta(theta) must not drift. We also check exact
// analytic anchors: the a -> 0 Schwarzschild photon circle, the equatorial
// plane staying geodesic, frame dragging appearing only for a > 0, and -- as
// an independent cross-check -- that the Kerr equatorial photon-orbit radius
// from the closed-form shadow module (kerr_shadow.hpp) is exactly where this
// integrator finds R = R' = 0.

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/geodesics/kerr_geodesic.hpp"
#include "blackhole_ds/geodesics/kerr_shadow.hpp"
#include "blackhole_ds/integrators/rk4.hpp"

namespace kg = blackhole_ds::geodesics::kerr;
using blackhole_ds::integrators::rk4_step;

namespace {

int failures = 0;

void report(bool cond, const char* expr, const char* file, int line) {
    if (!cond) {
        std::fprintf(stderr, "FAIL: %s at %s:%d\n", expr, file, line);
        ++failures;
    }
}

#define CHECK(cond) report((cond), #cond, __FILE__, __LINE__)

bool close_to(double a, double b, double tol) {
    return std::abs(a - b) <= tol;
}

} // namespace

int main() {
    // --- 1. Constants of motion conserved along a generic 3-D orbit ------
    // a = 0.5, E = 1, L_z = 2, Q = 3; start far out (r = 10) on the equator,
    // ingoing, with the polar velocity set on-shell. Integrate in Mino time
    // and confirm Q and the on-shell residuals do not drift.
    {
        const kg::GeodesicConstants k{0.5, 1.0, 2.0, 3.0};
        kg::GeodesicDeriv deriv{k};

        kg::KGState y{};
        y[kg::KG_R] = 10.0;
        y[kg::KG_TH] = kg::kg_pi / 2.0;
        y[kg::KG_PHI] = 0.0;
        y[kg::KG_T] = 0.0;
        const double R0 = kg::kg_R(10.0, k);
        const double Th0 = kg::kg_Theta(kg::kg_pi / 2.0, k);
        CHECK(R0 > 0.0);                 // a real radial range exists here
        CHECK(Th0 > 0.0);                // and a real polar range
        y[kg::KG_RDOT] = -std::sqrt(R0); // ingoing
        y[kg::KG_THDOT] = std::sqrt(Th0);

        const double Rscale = std::abs(R0) + 1.0;
        double lambda = 0.0;
        const double h = 1.0e-4;
        bool q_ok = true, rad_ok = true, pol_ok = true;
        for (int step = 0; step < 500; ++step) {
            y = rk4_step<6>(deriv, lambda, y, h);
            lambda += h;
            if (!close_to(kg::recover_Q(y, k), k.Q, 1.0e-4)) {
                q_ok = false;
            }
            if (std::abs(kg::radial_residual(y, k)) / Rscale > 1.0e-4) {
                rad_ok = false;
            }
            if (std::abs(kg::polar_residual(y, k)) > 1.0e-4) {
                pol_ok = false;
            }
            // stay well outside the outer horizon r+ = 1 + sqrt(1 - a^2)
            if (y[kg::KG_R] < 3.0) {
                break;
            }
        }
        CHECK(q_ok);   // Carter constant conserved
        CHECK(rad_ok); // (dr/dlambda)^2 stays on R(r)
        CHECK(pol_ok); // (dtheta/dlambda)^2 stays on Theta(theta)
    }

    // --- 2. a -> 0 Schwarzschild photon circle: r = 3 with b = sqrt(27) ---
    // E = 1, L_z = sqrt(27), Q = 0, equatorial, start at r = 3 with zero
    // radial velocity. R(3) = R'(3) = 0, so r must remain at 3.
    {
        const double Lz = std::sqrt(27.0);
        const kg::GeodesicConstants k{0.0, 1.0, Lz, 0.0};
        CHECK(close_to(kg::kg_R(3.0, k), 0.0, 1.0e-9));
        CHECK(close_to(kg::kg_half_dR(3.0, k), 0.0, 1.0e-9));

        kg::GeodesicDeriv deriv{k};
        kg::KGState y{};
        y[kg::KG_R] = 3.0;
        y[kg::KG_TH] = kg::kg_pi / 2.0;
        double lambda = 0.0;
        const double h = 1.0e-3;
        for (int step = 0; step < 2000; ++step) {
            y = rk4_step<6>(deriv, lambda, y, h);
            lambda += h;
        }
        CHECK(close_to(y[kg::KG_R], 3.0, 1.0e-6)); // stayed circular
        CHECK(close_to(y[kg::KG_TH], kg::kg_pi / 2.0, 1.0e-9)); // stayed planar
    }

    // --- 3. Equatorial plane is geodesic: theta = pi/2 stays put ----------
    // a = 0.7, Q = 0, start on the equator with no polar velocity.
    {
        const kg::GeodesicConstants k{0.7, 1.0, 3.0, 0.0};
        kg::GeodesicDeriv deriv{k};
        kg::KGState y{};
        y[kg::KG_R] = 8.0;
        y[kg::KG_TH] = kg::kg_pi / 2.0;
        const double R0 = kg::kg_R(8.0, k);
        CHECK(R0 > 0.0);
        y[kg::KG_RDOT] = -std::sqrt(R0);
        double lambda = 0.0;
        const double h = 1.0e-4;
        bool planar = true;
        for (int step = 0; step < 500; ++step) {
            y = rk4_step<6>(deriv, lambda, y, h);
            lambda += h;
            if (!close_to(y[kg::KG_TH], kg::kg_pi / 2.0, 1.0e-9)) {
                planar = false;
            }
            if (y[kg::KG_R] < 3.0) {
                break;
            }
        }
        CHECK(planar);
    }

    // --- 4. Frame dragging: a zero-angular-momentum photon (L_z = 0) still
    //     advances in phi for a > 0, but not for a = 0 -------------------
    {
        const kg::GeodesicConstants k0{0.0, 1.0, 0.0, 1.0};
        const kg::GeodesicConstants ka{0.9, 1.0, 0.0, 1.0};
        kg::KGState y{};
        y[kg::KG_R] = 10.0;
        y[kg::KG_TH] = kg::kg_pi / 2.0;
        const double dphi0 = kg::GeodesicDeriv{k0}(0.0, y)[kg::KG_PHI];
        const double dphia = kg::GeodesicDeriv{ka}(0.0, y)[kg::KG_PHI];
        CHECK(close_to(dphi0, 0.0, 1.0e-12)); // no dragging without spin
        CHECK(dphia > 1.0e-3);                // dragged forward by spin
    }

    // --- 5. Cross-check vs the closed-form shadow module: the prograde
    //     equatorial photon orbit radius is exactly where R = R' = 0,
    //     and the integrator keeps a photon launched there on a circle ----
    {
        const double a = 0.7;
        const double rc = kg::photon_orbit_prograde(a);
        const double xi = kg::bardeen_xi(rc, a); // L_z/E for this orbit
        const kg::GeodesicConstants k{a, 1.0, xi, 0.0};
        CHECK(close_to(kg::kg_R(rc, k), 0.0, 1.0e-6));
        CHECK(close_to(kg::kg_half_dR(rc, k), 0.0, 1.0e-6));

        kg::GeodesicDeriv deriv{k};
        kg::KGState y{};
        y[kg::KG_R] = rc;
        y[kg::KG_TH] = kg::kg_pi / 2.0;
        double lambda = 0.0;
        const double h = 1.0e-3;
        bool on_circle = true;
        for (int step = 0; step < 2000; ++step) {
            y = rk4_step<6>(deriv, lambda, y, h);
            lambda += h;
            if (!close_to(y[kg::KG_R], rc, 1.0e-5)) {
                on_circle = false;
            }
        }
        CHECK(on_circle);
    }

    // --- 6. Cunningham-Bardeen observer map: image plane -> constants -----
    {
        const double a = 0.6;
        const double inc = 70.0;
        const double i = inc * kg::kg_pi / 180.0;
        const double alpha = 3.5, beta = 2.0;
        const kg::GeodesicConstants k =
            kg::constants_from_image(alpha, beta, inc, a);

        // Definitional identities.
        CHECK(close_to(k.Lz, -alpha * std::sin(i), 1.0e-12));
        CHECK(close_to(k.Q,
                       beta * beta +
                           (alpha * alpha - a * a) * std::cos(i) * std::cos(i),
                       1.0e-12));
        // Self-consistency: Theta(i) == beta^2 (ties the map to the potential).
        CHECK(close_to(kg::kg_Theta(i, k), beta * beta, 1.0e-9));

        // a -> 0 form: Q = beta^2 + alpha^2 cos^2 i.
        const kg::GeodesicConstants k0 =
            kg::constants_from_image(alpha, beta, inc, 0.0);
        CHECK(close_to(k0.Q,
                       beta * beta + alpha * alpha * std::cos(i) * std::cos(i),
                       1.0e-12));

        // The initial state is on-shell and ingoing.
        const kg::KGState y0 =
            kg::initial_state_from_image(beta, inc, k, 1000.0);
        CHECK(y0[kg::KG_RDOT] < 0.0);
        CHECK(std::abs(kg::radial_residual(y0, k)) < 1.0e-3);
        CHECK(std::abs(kg::polar_residual(y0, k)) < 1.0e-9);
        CHECK(close_to(y0[kg::KG_TH], i, 1.0e-12));
    }

    // --- 7. Exact round-trip: a point built from a known photon orbit on
    //     the shadow boundary maps back to that orbit's (L_z, Q) -----------
    {
        const double a = 0.6;
        const double inc = 70.0;
        const double i = inc * kg::kg_pi / 180.0;
        const double sin_i = std::sin(i), cos_i = std::cos(i);
        const double cot2 = (cos_i * cos_i) / (sin_i * sin_i);

        const double rc = 0.5 * (kg::photon_orbit_prograde(a) +
                                 kg::photon_orbit_retrograde(a));
        const double xi = kg::bardeen_xi(rc, a);
        const double eta = kg::bardeen_eta(rc, a);
        const double beta2 = eta + a * a * cos_i * cos_i - xi * xi * cot2;
        CHECK(beta2 > 0.0); // this orbit is on the visible boundary

        const double alpha = -xi / sin_i;
        const double beta = std::sqrt(beta2);
        const kg::GeodesicConstants k =
            kg::constants_from_image(alpha, beta, inc, a);
        CHECK(close_to(k.Lz, xi, 1.0e-9)); // recovers L_z/E = xi
        CHECK(close_to(k.Q, eta, 1.0e-9)); // recovers Q/E^2 = eta
    }

    if (failures == 0) {
        std::puts("kerr_geodesic_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "kerr_geodesic_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
