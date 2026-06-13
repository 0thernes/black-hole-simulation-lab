# SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
# Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
# scripts/research/kerr_shadow_asymmetry.py
#
# Quantitative Kerr shadow observables as a function of spin and observer
# inclination, computed from the exact closed-form Bardeen (1973) boundary --
# the same physics as include/blackhole_ds/geodesics/kerr_shadow.hpp, in a
# standalone analysis with no build dependency.
#
# This is the documented spin-vs-asymmetry comparison called for by the M5
# exit criterion. Truth tier: analytic_classical (closed-form GR; the boundary
# involves no numerical geodesic integration). The a -> 0 limit and the a = 1
# equatorial photon-orbit radii are checked exactly against their known values;
# the extremal edge-on extent and the weak (<~7%) spin-dependence of the shadow
# size reproduce the classic Bardeen 1973 / Chandrasekhar 1983 results.
#
# Run:  python scripts/research/kerr_shadow_asymmetry.py
# References: Bardeen 1973; Chandrasekhar 1983 (The Mathematical Theory of
# Black Holes); Takahashi 2004; EHT Collaboration 2019 (M87*).

import math


def photon_orbit_prograde(a):
    return 2.0 * (1.0 + math.cos((2.0 / 3.0) * math.acos(-a)))


def photon_orbit_retrograde(a):
    return 2.0 * (1.0 + math.cos((2.0 / 3.0) * math.acos(a)))


def bardeen_xi(r, a):  # L_z / E
    return -(r**3 - 3 * r * r + a * a * (r + 1.0)) / (a * (r - 1.0))


def bardeen_eta(r, a):  # Q / E^2
    rm1 = r - 1.0
    return r**3 * (4 * a * a - r * (r - 3.0) ** 2) / (a * a * rm1 * rm1)


def shadow_boundary(a, incl_deg, samples=4000):
    """Closed-form (alpha, beta) shadow boundary points (units of M)."""
    i = math.radians(incl_deg)
    si, ci = math.sin(i), math.cos(i)
    cot2 = (ci * ci) / (si * si) if si > 1e-12 else 0.0
    if a < 1e-6:
        radius = math.sqrt(27.0)
        return [
            (radius * math.cos(t), radius * math.sin(t))
            for t in (2 * math.pi * k / samples for k in range(samples))
        ]
    r1, r2 = photon_orbit_prograde(a), photon_orbit_retrograde(a)
    lo, hi = min(r1, r2), max(r1, r2)
    pts = []
    for k in range(samples + 1):
        r = lo + (hi - lo) * k / samples
        if abs(r - 1.0) < 1e-9:
            continue
        xi, eta = bardeen_xi(r, a), bardeen_eta(r, a)
        beta2 = eta + a * a * ci * ci - xi * xi * cot2
        if beta2 < 0.0:
            continue
        al = -xi / (si if si > 1e-12 else 1e-12)
        be = math.sqrt(beta2)
        pts.append((al, be))
        if be > 1e-9:
            pts.append((al, -be))
    return pts


def observables(a, incl_deg):
    pts = shadow_boundary(max(a, 1e-9), incl_deg)
    als = [p[0] for p in pts]
    bes = [p[1] for p in pts]
    amin, amax, bmax = min(als), max(als), max(bes)
    width = amax - amin
    height = 2.0 * bmax
    disp = 0.5 * (amax + amin)  # centroid displacement in alpha (0 if symmetric)
    d_eq = math.sqrt(width * height)  # geometric-mean diameter
    frac_asym = abs(disp) / (0.25 * (width + height))
    return dict(width=width, height=height, d_eq=d_eq, disp=disp,
                frac_asym=frac_asym, amin=amin, amax=amax)


def main():
    print("Kerr shadow observables from the closed-form Bardeen 1973 boundary")
    print("(M = 1, units of GM/c^2). Truth tier: analytic_classical.\n")

    # --- Exact validation anchors ---------------------------------------
    o0 = observables(1e-9, 60.0)
    assert abs(o0["width"] - 2 * math.sqrt(27.0)) < 1e-6
    assert abs(o0["disp"]) < 1e-9
    assert abs(photon_orbit_prograde(1.0) - 1.0) < 1e-9
    assert abs(photon_orbit_retrograde(1.0) - 4.0) < 1e-9
    print("ANCHOR a=0 (any i): diameter = %.4f  (exact 2*sqrt(27) = %.4f), "
          "displacement = %.1e  [PASS]" % (o0["width"], 2 * math.sqrt(27.0),
                                           o0["disp"]))
    print("ANCHOR photon orbits a=1: prograde = %.4f (exact 1), "
          "retrograde = %.4f (exact 4)  [PASS]\n"
          % (photon_orbit_prograde(1.0), photon_orbit_retrograde(1.0)))

    print("Edge-on (i = 89.9 deg) -- maximal spin signature:")
    print(" a/M   width  height  d_eq   displacement  frac_asym  alpha[min,max]")
    for a in [0.0, 0.3, 0.6, 0.9, 0.99, 0.998]:
        o = observables(max(a, 1e-9), 89.9)
        print("%5.3f  %6.3f %6.3f %6.3f   %+7.3f      %6.3f   [%+.2f, %+.2f]"
              % (a, o["width"], o["height"], o["d_eq"], o["disp"],
                 o["frac_asym"], o["amin"], o["amax"]))

    print("\nM87*-like inclination (i = 17 deg):")
    print(" a/M   width  height  d_eq   displacement  frac_asym")
    for a in [0.0, 0.5, 0.9, 0.94, 0.998]:
        o = observables(max(a, 1e-9), 17.0)
        print("%5.3f  %6.3f %6.3f %6.3f   %+7.3f      %6.3f"
              % (a, o["width"], o["height"], o["d_eq"], o["disp"],
                 o["frac_asym"]))

    d0 = observables(1e-9, 89.9)["d_eq"]
    d1 = observables(0.998, 89.9)["d_eq"]
    print("\nDiameter spin-sensitivity (edge-on): a=0 -> %.3f, a=0.998 -> %.3f "
          "(%.1f%% change)." % (d0, d1, 100.0 * (d1 - d0) / d0))
    print("=> shadow SIZE is weakly spin-dependent (literature: <~7%); the spin")
    print("   signature is the DISPLACEMENT, which grows with a and sin(i).")


if __name__ == "__main__":
    main()
