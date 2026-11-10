// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/disk/novikov_thorne.hpp
// Equatorial circular geodesics and the Page-Thorne radiative flux of a
// Novikov-Thorne thin disk. Geometric units, M = 1.
//
// Established GR (analytic_classical), Bardeen, Press & Teukolsky 1972,
// ApJ 178, 347, doi:10.1086/151796. Prograde circular orbits:
//
//   Omega = 1 / (r^{3/2} + a)
//   E     = (r^{3/2} - 2 r^{1/2} + a)
//           / (r^{3/4} sqrt(r^{3/2} - 3 r^{1/2} + 2 a))
//   L     = (r^2 - 2 a r^{1/2} + a^2)
//           / (r^{3/4} sqrt(r^{3/2} - 3 r^{1/2} + 2 a))
//
// Model assumption (not the vacuum field equations): the disk is thin,
// its midplane is the equator, and the gas sits on these circular
// geodesics. Novikov & Thorne 1973; the time-averaged flux through one
// face is Page & Thorne 1974, ApJ 191, 499, doi:10.1086/152990:
//
//   F(r) = (Mdot / 4 pi) * (-dOmega/dr) / (E - Omega L)^2
//          * integral_{r_isco}^{r} (E - Omega L) (dL/dr) dr
//
// F(r_isco) = 0. For a = 0 the integral is elementary; that closed form
// is the reference. For a != 0 the integral is computed numerically
// (numerical_approximation of the same model). Mdot is the rest-mass
// accretion rate. This is not a covariant radiative-transfer solution
// and not a magnetized disk.
//
// Bolometric intensity. In vacuum, I_nu / nu^3 is invariant along a ray
// (Lindquist 1966; Misner, Thorne & Wheeler 1973). For an isotropic
// emitter in the orbiting frame the bolometric integral of that law is
// I_obs = g^4 I_em. Taking I_em = F / pi is Lambert's law, a model
// assumption on top of the invariant. The redshift factor g itself is
// computed by the caller (disk_image.hpp / kerr_disk_image.hpp).

#pragma once

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>

#include "blackhole_ds/metrics/kerr.hpp"

namespace blackhole_ds::disk {

inline constexpr double nt_pi = 3.14159265358979323846;

template <typename T> [[nodiscard]] T circular_omega(T r, double a) {
    return T(1) / (std::pow(r, T(1.5)) + T(a));
}

template <typename T> [[nodiscard]] T circular_energy(T r, double a) {
    const T s = std::sqrt(r);
    const T rad = r * s - T(3) * s + T(2) * a;
    const T den = std::pow(r, T(0.75)) * std::sqrt(rad);
    return (r * s - T(2) * s + T(a)) / den;
}

template <typename T> [[nodiscard]] T circular_angmom(T r, double a) {
    const T s = std::sqrt(r);
    const T rad = r * s - T(3) * s + T(2) * a;
    const T den = std::pow(r, T(0.75)) * std::sqrt(rad);
    return (r * r - T(2) * a * s + T(a * a)) / den;
}

// Binding efficiency eta = 1 - E(r_isco). Schwarzschild: 1 - sqrt(8/9).
[[nodiscard]] inline double radiative_efficiency(double a) {
    const double risco = metrics::kerr::isco_dimensionless(a);
    // The circular-orbit formula is 0/0 exactly at the ISCO. Step just
    // outside, where the expressions are finite, except for the known
    // Schwarzschild value which is exact.
    if (std::abs(a) < 1e-14) {
        return 1.0 - std::sqrt(8.0 / 9.0);
    }
    const double r = std::max(risco * (1.0 + 1e-10), risco + 1e-8);
    return 1.0 - circular_energy(r, a);
}

// Antiderivative of (E - Omega L) dL/dr for a = 0, M = 1.
// I(u) with u = sqrt(r), measured from the ISCO u = sqrt(6).
[[nodiscard]] inline double schwarzschild_flux_integral(double r) {
    if (r <= 6.0) {
        return 0.0;
    }
    const double s3 = std::sqrt(3.0);
    const auto at = [s3](double u) {
        return u - (s3 / 2.0) * std::log((u - s3) / (u + s3));
    };
    return at(std::sqrt(r)) - at(std::sqrt(6.0));
}

// Page-Thorne flux for Schwarzschild, one face, M = 1.
// Truth tier: analytic_classical inside the thin-disk model.
[[nodiscard]] inline double schwarzschild_page_thorne_flux(double r,
                                                           double mdot = 1.0) {
    if (!(r > 6.0) || !(mdot >= 0.0)) {
        return 0.0;
    }
    const double I = schwarzschild_flux_integral(r);
    const double pref =
        (mdot / (4.0 * nt_pi)) * 1.5 * std::pow(r, -1.5) / (r - 3.0);
    return pref * I;
}

namespace detail {

template <typename F> [[nodiscard]] double complex_step(F f, double x) {
    const double h = 1e-8;
    const std::complex<double> z(x, h);
    return std::imag(f(z)) / h;
}

template <typename F>
[[nodiscard]] double simpson(F f, double a, double b, int n) {
    if (n < 2) {
        n = 2;
    }
    if (n % 2 != 0) {
        ++n;
    }
    const double h = (b - a) / static_cast<double>(n);
    double s = f(a) + f(b);
    for (int i = 1; i < n; ++i) {
        const double w = (i % 2 == 0) ? 2.0 : 4.0;
        s += w * f(a + h * static_cast<double>(i));
    }
    return s * h / 3.0;
}

} // namespace detail

// Page-Thorne flux for Kerr (prograde, |a| < 1). a = 0 uses the closed form.
// Truth tier: numerical_approximation of the thin-disk model for a != 0.
[[nodiscard]] inline double
page_thorne_flux(double r, double a, double mdot = 1.0, int panels = 4000) {
    if (!(mdot >= 0.0) || !(r > 0.0)) {
        return 0.0;
    }
    if (std::abs(a) < 1e-14) {
        return schwarzschild_page_thorne_flux(r, mdot);
    }
    a = std::clamp(a, -0.999999, 0.999999);
    const double risco = metrics::kerr::isco_dimensionless(a);
    if (!(r > risco)) {
        return 0.0;
    }
    // The circular-orbit formulae are singular at r_isco (the radicand
    // vanishes). The integrand is finite; start a hair outside.
    const double r0 = risco + 1e-6 * std::max(1.0, risco);
    if (!(r > r0)) {
        return 0.0;
    }
    const auto integrand = [a](double rr) {
        const double el = circular_energy(rr, a) -
                          circular_omega(rr, a) * circular_angmom(rr, a);
        const double lp = detail::complex_step(
            [a](std::complex<double> z) { return circular_angmom(z, a); }, rr);
        return el * lp;
    };
    const double integral = detail::simpson(integrand, r0, r, panels);
    const double el =
        circular_energy(r, a) - circular_omega(r, a) * circular_angmom(r, a);
    const double om_p = detail::complex_step(
        [a](std::complex<double> z) { return circular_omega(z, a); }, r);
    if (!(el > 0.0)) {
        return 0.0;
    }
    return (mdot / (4.0 * nt_pi)) * (-om_p) / (el * el) * integral;
}

// I_obs = g^4 * (F / pi). g is the redshift factor nu_obs/nu_em.
// F/pi is Lambert's law (model). g^4 is the vacuum invariant (GR).
[[nodiscard]] inline double observed_bolometric_intensity(double flux,
                                                          double g) {
    if (!(flux >= 0.0) || !(g >= 0.0)) {
        return 0.0;
    }
    return (flux / nt_pi) * std::pow(g, 4.0);
}

} // namespace blackhole_ds::disk
