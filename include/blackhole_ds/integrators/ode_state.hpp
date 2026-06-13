// blackhole_ds/integrators/ode_state.hpp
// Fixed-size state vector for ODE integration.
//
// A geodesic in a 4D spacetime is an 8-component first-order system
// (4 position + 4 momentum), so the integrators are templated on the
// compile-time dimension N. Using std::array keeps the state on the stack
// with no heap allocation in the hot loop.
//
// Complexity: every operation here is O(N) with N fixed and small
// (typically 8). No allocation. See docs/engineering/COMPLEXITY.md.

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace blackhole_ds::integrators {

template <std::size_t N>
using State = std::array<double, N>;

// y = a + b
template <std::size_t N>
[[nodiscard]] constexpr State<N> add(const State<N>& a,
                                     const State<N>& b) noexcept {
    State<N> out{};
    for (std::size_t i = 0; i < N; ++i) {
        out[i] = a[i] + b[i];
    }
    return out;
}

// y = a + s * b   (fused: the workhorse of every Runge-Kutta stage)
template <std::size_t N>
[[nodiscard]] constexpr State<N> axpy(const State<N>& a, double s,
                                      const State<N>& b) noexcept {
    State<N> out{};
    for (std::size_t i = 0; i < N; ++i) {
        out[i] = a[i] + s * b[i];
    }
    return out;
}

// y = s * a
template <std::size_t N>
[[nodiscard]] constexpr State<N> scale(double s,
                                       const State<N>& a) noexcept {
    State<N> out{};
    for (std::size_t i = 0; i < N; ++i) {
        out[i] = s * a[i];
    }
    return out;
}

// Weighted RMS norm used by the adaptive controller. Each component is
// scaled by atol + rtol*|y| so mixed-magnitude states (position vs
// momentum) are compared fairly. Hairer, Norsett, Wanner, "Solving
// Ordinary Differential Equations I" (1993), section II.4.
template <std::size_t N>
[[nodiscard]] double weighted_rms_norm(const State<N>& error,
                                       const State<N>& y_old,
                                       const State<N>& y_new,
                                       double atol, double rtol) noexcept {
    double sum_sq = 0.0;
    for (std::size_t i = 0; i < N; ++i) {
        const double scale_i =
            atol + rtol * std::max(std::abs(y_old[i]), std::abs(y_new[i]));
        const double ratio = error[i] / scale_i;
        sum_sq += ratio * ratio;
    }
    return std::sqrt(sum_sq / static_cast<double>(N));
}

} // namespace blackhole_ds::integrators
