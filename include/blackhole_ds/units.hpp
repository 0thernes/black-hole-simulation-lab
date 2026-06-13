// BlackHoleDS — Physical Units & Type Theory (C++20)
// Compile-time dimensional safety for cosmology and GR numerics.
//
// Policy (see CONTRIBUTING.md and ADR-0001): physical quantities in physics
// code use these strong types. Raw double is reserved for dimensionless
// ratios (e.g. spin a/M) and for I/O boundaries, with a justification
// comment at the use site.
//
// What the type system guarantees (all enforced by tests/smoke_tests.cpp):
//   - Same-dimension arithmetic works: Length + Length, Energy - Energy,
//     Quantity * scalar, Quantity / scalar.
//   - Cross-dimension addition does not compile: Length + Time is rejected
//     because the binary operators are defined only for identical tags.
//   - Physically meaningful cross-dimension operations are explicit,
//     individually defined functions (e.g. Length / Time -> Velocity).
//
// Numerical notes:
//   - Kahan summation is provided for conserved-quantity accumulation
//     (energy, angular momentum, Carter constant) over long integrations.
//   - Validators cross-check computed observables against exact analytic
//     values. They avoid ALL std-math calls (hand-rolled absolute value,
//     precomputed constants) because MSVC — correctly, per the C++20
//     standard — rejects std::abs/std::sqrt in constant expressions, while
//     GCC accepts them as builtins. That divergence kept CI (MSVC) red
//     while local MinGW builds passed. Pure-arithmetic constexpr builds
//     identically under both toolchains.
//
// References (truth tier: analytic_classical):
//   - Misner, Thorne, Wheeler — Gravitation (W.H. Freeman, 1973).
//   - Chandrasekhar — The Mathematical Theory of Black Holes (Oxford, 1983).
//   - Bardeen, Press, Teukolsky (1972), Astrophys. J. 178, 347.

#pragma once

#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>

namespace blackhole_ds::units {

// ============================================================================
// Core Physical Dimension Tags (compile-time only)
// ============================================================================

struct LengthTag {};
struct TimeTag {};
struct MassTag {};
struct EnergyTag {};
struct AngularMomentumTag {};
struct VelocityTag {};
struct DimensionlessTag {}; // redshift factor g, spin a/M, etc.

// ============================================================================
// Strong Quantity Template
// ============================================================================

template <typename Tag, typename Rep = double> class Quantity {
public:
    using tag_type = Tag;
    using rep_type = Rep;

    constexpr Quantity() noexcept : value_(Rep{0}) {}
    constexpr explicit Quantity(Rep v) noexcept : value_(v) {}

    [[nodiscard]] constexpr Rep value() const noexcept {
        return value_;
    }

    constexpr Quantity& operator+=(const Quantity& rhs) noexcept {
        value_ += rhs.value_;
        return *this;
    }
    constexpr Quantity& operator-=(const Quantity& rhs) noexcept {
        value_ -= rhs.value_;
        return *this;
    }
    constexpr Quantity& operator*=(Rep scalar) noexcept {
        value_ *= scalar;
        return *this;
    }
    constexpr Quantity& operator/=(Rep scalar) noexcept {
        value_ /= scalar;
        return *this;
    }

    [[nodiscard]] constexpr Quantity operator-() const noexcept {
        return Quantity{-value_};
    }

    [[nodiscard]] constexpr bool
    operator==(const Quantity& rhs) const noexcept {
        return value_ == rhs.value_;
    }
    [[nodiscard]] constexpr bool
    operator!=(const Quantity& rhs) const noexcept {
        return !(*this == rhs);
    }
    [[nodiscard]] constexpr bool operator<(const Quantity& rhs) const noexcept {
        return value_ < rhs.value_;
    }
    [[nodiscard]] constexpr bool
    operator<=(const Quantity& rhs) const noexcept {
        return value_ <= rhs.value_;
    }
    [[nodiscard]] constexpr bool operator>(const Quantity& rhs) const noexcept {
        return value_ > rhs.value_;
    }
    [[nodiscard]] constexpr bool
    operator>=(const Quantity& rhs) const noexcept {
        return value_ >= rhs.value_;
    }

private:
    Rep value_;
};

// ============================================================================
// Same-dimension binary arithmetic
//
// Defined only for identical tags, so Length + Time fails to find an
// overload and does not compile. This is the mechanism behind the
// "adding meters to seconds is a compile error" guarantee.
// ============================================================================

template <typename Tag, typename Rep>
[[nodiscard]] constexpr Quantity<Tag, Rep>
operator+(Quantity<Tag, Rep> lhs, const Quantity<Tag, Rep>& rhs) noexcept {
    lhs += rhs;
    return lhs;
}

template <typename Tag, typename Rep>
[[nodiscard]] constexpr Quantity<Tag, Rep>
operator-(Quantity<Tag, Rep> lhs, const Quantity<Tag, Rep>& rhs) noexcept {
    lhs -= rhs;
    return lhs;
}

template <typename Tag, typename Rep>
[[nodiscard]] constexpr Quantity<Tag, Rep> operator*(Quantity<Tag, Rep> q,
                                                     Rep scalar) noexcept {
    q *= scalar;
    return q;
}

template <typename Tag, typename Rep>
[[nodiscard]] constexpr Quantity<Tag, Rep>
operator*(Rep scalar, Quantity<Tag, Rep> q) noexcept {
    q *= scalar;
    return q;
}

template <typename Tag, typename Rep>
[[nodiscard]] constexpr Quantity<Tag, Rep> operator/(Quantity<Tag, Rep> q,
                                                     Rep scalar) noexcept {
    q /= scalar;
    return q;
}

// ============================================================================
// Dimension-Specific Aliases
// ============================================================================

using Length = Quantity<LengthTag>;
using Time = Quantity<TimeTag>;
using Mass = Quantity<MassTag>;
using Energy = Quantity<EnergyTag>;
using AngularMomentum = Quantity<AngularMomentumTag>;
using Velocity = Quantity<VelocityTag>;
using Dimensionless = Quantity<DimensionlessTag>;

// ============================================================================
// User-Defined Literals
// ============================================================================

constexpr Length operator""_m(long double v) noexcept {
    return Length{static_cast<double>(v)};
}
constexpr Length operator""_m(unsigned long long v) noexcept {
    return Length{static_cast<double>(v)};
}

constexpr Time operator""_s(long double v) noexcept {
    return Time{static_cast<double>(v)};
}
constexpr Time operator""_s(unsigned long long v) noexcept {
    return Time{static_cast<double>(v)};
}

constexpr Mass operator""_kg(long double v) noexcept {
    return Mass{static_cast<double>(v)};
}
constexpr Mass operator""_kg(unsigned long long v) noexcept {
    return Mass{static_cast<double>(v)};
}

// Solar mass (common in GR astrophysics)
constexpr Mass operator""_Msun(long double v) noexcept {
    return Mass{static_cast<double>(v) * 1.98847e30};
}
constexpr Mass operator""_Msun(unsigned long long v) noexcept {
    return Mass{static_cast<double>(v) * 1.98847e30};
}

// Geometric units (G = c = 1)
constexpr Mass operator""_M(long double v) noexcept {
    return Mass{static_cast<double>(v)};
}
constexpr Length operator""_rg(long double v) noexcept {
    return Length{static_cast<double>(v)};
} // r_g = GM/c^2

// ============================================================================
// Explicit cross-dimension operations (each one physically meaningful)
// ============================================================================

// Velocity = Length / Time
[[nodiscard]] constexpr Velocity operator/(Length l, Time t) noexcept {
    return Velocity{l.value() / t.value()};
}

// Rest energy in geometric units (G = c = 1): E = m.
[[nodiscard]] constexpr Energy mass_to_energy(Mass m) noexcept {
    return Energy{m.value()};
}

// ============================================================================
// Kahan Summation Accumulator
// For conserved quantities (E, Lz, Carter Q) over long integrations near
// horizons, where naive accumulation loses precision.
// ============================================================================

template <typename Q> class KahanAccumulator {
public:
    constexpr void add(Q term) noexcept {
        auto y = term.value() - correction_;
        auto t = sum_ + y;
        correction_ = (t - sum_) - y;
        sum_ = t;
    }

    [[nodiscard]] constexpr Q total() const noexcept {
        return Q{sum_};
    }
    constexpr void reset() noexcept {
        sum_ = 0.0;
        correction_ = 0.0;
    }

private:
    double sum_{0.0};
    double correction_{0.0};
};

// ============================================================================
// Analytic Validators (truth tier: analytic_classical)
//
// Cross-checks of computed observables against exact closed-form values in
// geometric units (r_g = GM/c^2, so M enters as a length scale).
//
// Constexpr is kept here ONLY because no std-math is used: the absolute
// value is hand-rolled and sqrt(27) is a precomputed literal. MSVC rejects
// std::abs/std::sqrt in constant expressions under C++20 (GCC accepts them
// as builtins) — that divergence is exactly what kept CI red while local
// builds passed. If a future validator needs real std-math, drop constexpr.
// ============================================================================

namespace validators {

namespace detail {
[[nodiscard]] constexpr double abs_val(double x) noexcept {
    return x < 0.0 ? -x : x;
}
} // namespace detail

// Schwarzschild photon sphere radius: r_ph = 3 GM/c^2 = 1.5 r_s (exact).
[[nodiscard]] constexpr bool photon_sphere_radius_valid(Length r_photon,
                                                        Mass M) noexcept {
    const double expected = 3.0 * M.value();
    const double rel_err =
        detail::abs_val(r_photon.value() - expected) / expected;
    return rel_err < 1e-12;
}

// Schwarzschild ISCO: r_isco = 6 GM/c^2 = 3 r_s (exact).
[[nodiscard]] constexpr bool isco_radius_valid(Length r_isco, Mass M) noexcept {
    const double expected = 6.0 * M.value();
    const double rel_err =
        detail::abs_val(r_isco.value() - expected) / expected;
    return rel_err < 1e-12;
}

// Schwarzschild shadow DIAMETER (exact): d = 2 * b_crit = 2 * sqrt(27) M
// = 6 * sqrt(3) M ~= 10.392304845413264 GM/c^2.
//
// Terminology guard (the source of a historical factor-of-2 bug in this
// project): the critical photon impact parameter b_crit = sqrt(27) M
// ~= 5.196 M is the shadow RADIUS. The EHT literature's "~5.2" figure is
// the diameter measured in SCHWARZSCHILD RADII (10.39 M / r_s where
// r_s = 2M). Never mix the two unit systems.
//
// Spin dependence is intentionally not modeled here: the Kerr shadow's
// mean diameter shrinks by only a few percent toward extremal spin and
// depends on observer inclination. A Kerr shadow validator belongs with a
// real ray tracer, not a polynomial guess.
[[nodiscard]] constexpr bool
schwarzschild_shadow_diameter_valid(Length d_shadow, Mass M) noexcept {
    constexpr double two_sqrt27 = 10.392304845413264; // 2*sqrt(27) = 6*sqrt(3)
    const double expected = two_sqrt27 * M.value();
    const double rel_err =
        detail::abs_val(d_shadow.value() - expected) / expected;
    return rel_err < 1e-12;
}

} // namespace validators

} // namespace blackhole_ds::units

// ============================================================================
// Usage Contract (enforced by tests/smoke_tests.cpp)
// ============================================================================
//
// Geodesic integrators, Lyapunov estimators, and exporters use the strong
// types above for dimensioned quantities. The guarantees, all of which have
// compile-time or runtime tests:
//
//   Length r1{10.0}, r2{2.0};
//   Time   t{5.0};
//   auto sum  = r1 + r2;     // OK: same dimension
//   auto sc   = r1 * 2.0;    // OK: scalar scaling
//   auto v    = r1 / t;      // OK: explicit Length/Time -> Velocity
//   auto bad  = r1 + t;      // COMPILE ERROR: no operator for mixed tags
//
// Conserved quantities accumulate through KahanAccumulator<Energy> etc. and
// are cross-checked against the analytic validators at integration
// milestones.
//
// End of units.hpp
