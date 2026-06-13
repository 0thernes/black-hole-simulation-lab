// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 0thernes <0_0@0thernes.art>
// blackhole_ds/integrators/rk4.hpp
// Classic fixed-step fourth-order Runge-Kutta integrator.
//
// This is the simplest workhorse: fixed step h, global error O(h^4),
// four derivative evaluations per step. Useful as a baseline and for
// problems where the step size is known to be safe. For stiff regions or
// near-horizon geodesics, prefer the adaptive Dormand-Prince stepper in
// rk45.hpp.
//
// Truth tier: numerical_approximation. Every result carries a documented
// error order; convergence is verified in tests/integrator_tests.cpp.
//
// Complexity (per step): 4 derivative evaluations + O(N) vector ops.
// For M steps: O(M) derivative evaluations, O(1) extra memory.
// See docs/engineering/COMPLEXITY.md.
//
// Reference: Hairer, Norsett, Wanner, "Solving Ordinary Differential
// Equations I" (1993), section II.1.

#pragma once

#include <cstddef>

#include "blackhole_ds/integrators/ode_state.hpp"

namespace blackhole_ds::integrators {

// Advance one fixed RK4 step.
//   deriv:  callable (double t, const State<N>& y) -> State<N>
//   t:      current independent variable
//   y:      current state
//   h:      step size
// Returns the state at t + h.
template <std::size_t N, typename Deriv>
[[nodiscard]] State<N> rk4_step(Deriv&& deriv, double t, const State<N>& y,
                                double h) {
    const State<N> k1 = deriv(t, y);
    const State<N> k2 = deriv(t + 0.5 * h, axpy(y, 0.5 * h, k1));
    const State<N> k3 = deriv(t + 0.5 * h, axpy(y, 0.5 * h, k2));
    const State<N> k4 = deriv(t + h, axpy(y, h, k3));

    State<N> out{};
    const double sixth = h / 6.0;
    for (std::size_t i = 0; i < N; ++i) {
        out[i] = y[i] + sixth * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);
    }
    return out;
}

// Integrate from t0 to t1 with a fixed number of uniform steps.
// Returns the final state. The number of steps must be >= 1.
template <std::size_t N, typename Deriv>
[[nodiscard]] State<N> rk4_integrate(Deriv&& deriv, double t0, double t1,
                                     const State<N>& y0, int steps) {
    if (steps < 1) {
        steps = 1;
    }
    const double h = (t1 - t0) / static_cast<double>(steps);
    State<N> y = y0;
    double t = t0;
    for (int i = 0; i < steps; ++i) {
        y = rk4_step<N>(deriv, t, y, h);
        t += h;
    }
    return y;
}

} // namespace blackhole_ds::integrators
