// blackhole_ds/integrators/rk45.hpp
// Adaptive Dormand-Prince 5(4) Runge-Kutta integrator (the "ode45" method).
//
// Seven-stage explicit RK pair: a 5th-order solution with an embedded
// 4th-order solution for error estimation. The step size is adapted from
// the local error estimate so the integrator takes large steps where the
// solution is smooth and small steps near features (e.g. the photon
// sphere, where null geodesics are sensitive). First-Same-As-Last (FSAL):
// the 7th stage of a step equals the 1st stage of the next, so an accepted
// step costs six new derivative evaluations, not seven.
//
// Truth tier: numerical_approximation. The order is documented and the
// convergence and error control are verified in tests/integrator_tests.cpp.
//
// Complexity (per accepted step): 6 derivative evaluations (FSAL) + O(N)
// vector ops + O(N) error norm. Rejected steps cost the same and retry
// with a smaller h. Total evaluations scale with the solution's stiffness
// and the requested tolerance, not with a fixed step count.
// See docs/engineering/COMPLEXITY.md.
//
// References:
//   - Dormand, J. R., Prince, P. J. (1980). "A family of embedded
//     Runge-Kutta formulae." J. Comp. Appl. Math. 6 (1), 19-26.
//   - Hairer, Norsett, Wanner, "Solving ODEs I" (1993), section II.4-5.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>

#include "blackhole_ds/integrators/ode_state.hpp"

namespace blackhole_ds::integrators {

struct AdaptiveOptions {
    double abs_tol = 1e-10;     // absolute tolerance per component
    double rel_tol = 1e-10;     // relative tolerance per component
    double initial_step = 1e-3; // first trial step
    double min_step = 1e-12;    // floor; below this we give up
    double max_step = 1e6;      // ceiling on a single step
    double safety = 0.9;        // step-growth safety factor
    double min_scale = 0.2;     // never shrink faster than this
    double max_scale = 10.0;    // never grow faster than this
    int max_steps = 1'000'000;  // hard iteration cap (runaway guard)
};

struct AdaptiveResult {
    bool success = false;
    int accepted_steps = 0;
    int rejected_steps = 0;
    double t_final = 0.0;
};

namespace dp {
// Dormand-Prince 5(4) Butcher tableau (the standard coefficients).
inline constexpr double c2 = 1.0 / 5.0;
inline constexpr double c3 = 3.0 / 10.0;
inline constexpr double c4 = 4.0 / 5.0;
inline constexpr double c5 = 8.0 / 9.0;

inline constexpr double a21 = 1.0 / 5.0;

inline constexpr double a31 = 3.0 / 40.0;
inline constexpr double a32 = 9.0 / 40.0;

inline constexpr double a41 = 44.0 / 45.0;
inline constexpr double a42 = -56.0 / 15.0;
inline constexpr double a43 = 32.0 / 9.0;

inline constexpr double a51 = 19372.0 / 6561.0;
inline constexpr double a52 = -25360.0 / 2187.0;
inline constexpr double a53 = 64448.0 / 6561.0;
inline constexpr double a54 = -212.0 / 729.0;

inline constexpr double a61 = 9017.0 / 3168.0;
inline constexpr double a62 = -355.0 / 33.0;
inline constexpr double a63 = 46732.0 / 5247.0;
inline constexpr double a64 = 49.0 / 176.0;
inline constexpr double a65 = -5103.0 / 18656.0;

// 5th-order solution weights (also the 7th-stage a-coefficients: FSAL).
inline constexpr double b1 = 35.0 / 384.0;
inline constexpr double b3 = 500.0 / 1113.0;
inline constexpr double b4 = 125.0 / 192.0;
inline constexpr double b5 = -2187.0 / 6784.0;
inline constexpr double b6 = 11.0 / 84.0;

// 4th-order embedded solution weights.
inline constexpr double bs1 = 5179.0 / 57600.0;
inline constexpr double bs3 = 7571.0 / 16695.0;
inline constexpr double bs4 = 393.0 / 640.0;
inline constexpr double bs5 = -92097.0 / 339200.0;
inline constexpr double bs6 = 187.0 / 2100.0;
inline constexpr double bs7 = 1.0 / 40.0;
} // namespace dp

// One Dormand-Prince step. Writes the 5th-order result to y5 and the
// error estimate (5th minus 4th order) to err. k1 is supplied (FSAL reuse)
// and k7 (== deriv at t+h, y5) is written for the next step's reuse.
template <std::size_t N, typename Deriv>
void dp_step(Deriv&& deriv, double t, const State<N>& y, double h,
             const State<N>& k1, State<N>& y5, State<N>& err, State<N>& k7) {
    using namespace dp;

    const State<N> k2 = deriv(t + c2 * h, axpy(y, h * a21, k1));

    State<N> y3 = y;
    for (std::size_t i = 0; i < N; ++i) {
        y3[i] += h * (a31 * k1[i] + a32 * k2[i]);
    }
    const State<N> k3 = deriv(t + c3 * h, y3);

    State<N> y4 = y;
    for (std::size_t i = 0; i < N; ++i) {
        y4[i] += h * (a41 * k1[i] + a42 * k2[i] + a43 * k3[i]);
    }
    const State<N> k4 = deriv(t + c4 * h, y4);

    State<N> y5s = y;
    for (std::size_t i = 0; i < N; ++i) {
        y5s[i] += h * (a51 * k1[i] + a52 * k2[i] + a53 * k3[i] + a54 * k4[i]);
    }
    const State<N> k5 = deriv(t + c5 * h, y5s);

    State<N> y6 = y;
    for (std::size_t i = 0; i < N; ++i) {
        y6[i] += h * (a61 * k1[i] + a62 * k2[i] + a63 * k3[i] + a64 * k4[i] +
                      a65 * k5[i]);
    }
    const State<N> k6 = deriv(t + h, y6);

    // 5th-order solution.
    for (std::size_t i = 0; i < N; ++i) {
        y5[i] = y[i] + h * (b1 * k1[i] + b3 * k3[i] + b4 * k4[i] + b5 * k5[i] +
                            b6 * k6[i]);
    }

    // FSAL: 7th stage is the derivative at the new point.
    k7 = deriv(t + h, y5);

    // Error = 5th order minus 4th order embedded solution.
    for (std::size_t i = 0; i < N; ++i) {
        const double y4th =
            y[i] + h * (bs1 * k1[i] + bs3 * k3[i] + bs4 * k4[i] + bs5 * k5[i] +
                        bs6 * k6[i] + bs7 * k7[i]);
        err[i] = y5[i] - y4th;
    }
}

// Integrate from t0 to t1 with adaptive step control. The state at t1 is
// written into y (in/out). Returns statistics and a success flag.
//
// The controller is the standard elementary I-controller: from a local
// error norm `e` (target 1.0), the optimal step scale is
//   scale = safety * e^(-1/5)
// clamped to [min_scale, max_scale]. A step is accepted when e <= 1.
template <std::size_t N, typename Deriv>
AdaptiveResult rk45_integrate(Deriv&& deriv, double t0, double t1, State<N>& y,
                              const AdaptiveOptions& opt = {}) {
    AdaptiveResult result;
    const double direction = (t1 >= t0) ? 1.0 : -1.0;
    double t = t0;
    double h = std::min(opt.initial_step, std::abs(t1 - t0)) * direction;
    if (h == 0.0) {
        result.success = (t0 == t1);
        result.t_final = t;
        return result;
    }

    State<N> k1 = deriv(t, y); // FSAL seed
    State<N> y5{};
    State<N> err{};
    State<N> k7{};

    for (int iter = 0; iter < opt.max_steps; ++iter) {
        if ((direction > 0 && t >= t1) || (direction < 0 && t <= t1)) {
            result.success = true;
            break;
        }
        // Do not overshoot the endpoint.
        if (std::abs(h) > std::abs(t1 - t)) {
            h = (t1 - t);
        }

        dp_step<N>(deriv, t, y, h, k1, y5, err, k7);

        const double e =
            weighted_rms_norm<N>(err, y, y5, opt.abs_tol, opt.rel_tol);

        if (e <= 1.0) {
            // Accept.
            t += h;
            y = y5;
            k1 = k7; // FSAL
            ++result.accepted_steps;
        } else {
            ++result.rejected_steps;
        }

        // Propose the next step. e == 0 means take the biggest allowed jump.
        double scale;
        if (e == 0.0) {
            scale = opt.max_scale;
        } else {
            scale = opt.safety * std::pow(e, -0.2);
            scale = std::clamp(scale, opt.min_scale, opt.max_scale);
        }
        h *= scale;

        // Respect the step ceiling and floor.
        if (std::abs(h) > opt.max_step) {
            h = opt.max_step * direction;
        }
        if (std::abs(h) < opt.min_step) {
            // Cannot make progress at the requested tolerance.
            result.success = false;
            result.t_final = t;
            return result;
        }
    }

    result.t_final = t;
    if (!result.success) {
        // Loop exited on max_steps without reaching t1.
        result.success =
            ((direction > 0 && t >= t1) || (direction < 0 && t <= t1));
    }
    return result;
}

} // namespace blackhole_ds::integrators
