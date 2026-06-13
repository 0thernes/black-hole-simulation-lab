// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// tests/integrator_tests.cpp
// Verifies the RK4 and adaptive Dormand-Prince integrators against ODEs
// with known closed-form solutions, and checks the convergence order.
//
// Uses a tiny CHECK macro so behavior is identical in Debug and Release.

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "blackhole_ds/integrators/ode_state.hpp"
#include "blackhole_ds/integrators/rk4.hpp"
#include "blackhole_ds/integrators/rk45.hpp"

using namespace blackhole_ds::integrators;

namespace {

// M_PI is not standard C++ (it needs a platform macro); define it locally.
constexpr double kPi = 3.14159265358979323846;

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

// Test 1: exponential growth, dy/dt = y, y(0) = 1, exact y(t) = e^t.
State<1> exp_deriv(double, const State<1>& y) {
    return State<1>{y[0]};
}

// Test 2: simple harmonic oscillator, y'' = -y, written as a first-order
// system [x, v] with x' = v, v' = -x. Exact: x(t) = cos(t), v(t) = -sin(t).
// Energy E = (x^2 + v^2)/2 is conserved (= 0.5 for these initial data).
State<2> sho_deriv(double, const State<2>& y) {
    return State<2>{y[1], -y[0]};
}

} // namespace

int main() {
    const double e = std::exp(1.0);

    // --- RK4 hits the exponential to high accuracy with enough steps ---
    {
        const State<1> y0{1.0};
        const State<1> y = rk4_integrate<1>(exp_deriv, 0.0, 1.0, y0, 1000);
        CHECK(close_to(y[0], e, 1e-10));
    }

    // --- RK4 fourth-order convergence: halving h cuts error ~16x ---
    {
        const State<1> y0{1.0};
        const auto err_at = [&](int steps) {
            const State<1> y = rk4_integrate<1>(exp_deriv, 0.0, 1.0, y0, steps);
            return std::abs(y[0] - e);
        };
        const double e1 = err_at(10);
        const double e2 = err_at(20);
        const double ratio = e1 / e2;
        // Theoretical ratio for a 4th-order method is 2^4 = 16.
        CHECK(ratio > 12.0 && ratio < 20.0);
    }

    // --- RK4 conserves SHO energy reasonably over one period ---
    {
        const State<2> y0{1.0, 0.0};
        const State<2> y =
            rk4_integrate<2>(sho_deriv, 0.0, 2.0 * kPi, y0, 2000);
        CHECK(close_to(y[0], 1.0, 1e-6)); // back to cos(2pi) = 1
        CHECK(close_to(y[1], 0.0, 1e-6)); // v = -sin(2pi) = 0
    }

    // --- Adaptive DP45 hits the exponential within tolerance ---
    {
        State<1> y{1.0};
        AdaptiveOptions opt;
        opt.abs_tol = 1e-10;
        opt.rel_tol = 1e-10;
        const AdaptiveResult r = rk45_integrate<1>(exp_deriv, 0.0, 1.0, y, opt);
        CHECK(r.success);
        CHECK(r.accepted_steps > 0);
        CHECK(close_to(y[0], e, 1e-8));
    }

    // --- Adaptive DP45 integrates the SHO over a full period accurately ---
    {
        State<2> y{1.0, 0.0};
        AdaptiveOptions opt;
        opt.abs_tol = 1e-11;
        opt.rel_tol = 1e-11;
        const AdaptiveResult r =
            rk45_integrate<2>(sho_deriv, 0.0, 2.0 * kPi, y, opt);
        CHECK(r.success);
        CHECK(close_to(y[0], 1.0, 1e-8));
        CHECK(close_to(y[1], 0.0, 1e-8));
    }

    // --- Tighter tolerance buys more accuracy (and more steps) ---
    {
        const auto final_err = [&](double tol, int& steps_out) {
            State<1> y{1.0};
            AdaptiveOptions opt;
            opt.abs_tol = tol;
            opt.rel_tol = tol;
            const AdaptiveResult r =
                rk45_integrate<1>(exp_deriv, 0.0, 1.0, y, opt);
            steps_out = r.accepted_steps;
            return std::abs(y[0] - e);
        };
        int loose_steps = 0, tight_steps = 0;
        const double loose_err = final_err(1e-6, loose_steps);
        const double tight_err = final_err(1e-12, tight_steps);
        CHECK(tight_err <= loose_err);
        CHECK(tight_steps >= loose_steps);
    }

    // --- Backward integration (t1 < t0) works ---
    {
        State<1> y{e}; // start at e^1
        AdaptiveOptions opt;
        const AdaptiveResult r = rk45_integrate<1>(exp_deriv, 1.0, 0.0, y, opt);
        CHECK(r.success);
        CHECK(close_to(y[0], 1.0, 1e-6)); // e^0 = 1
    }

    if (failures == 0) {
        std::puts("integrator_tests passed");
        return EXIT_SUCCESS;
    }
    std::fprintf(stderr, "integrator_tests: %d failure(s)\n", failures);
    return EXIT_FAILURE;
}
