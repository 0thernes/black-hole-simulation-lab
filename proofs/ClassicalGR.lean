-- Exact rational certificates for the Schwarzschild limits used by the
-- simulator. Core Lean 4 only: `rfl` reduces both sides. No Mathlib.
--
-- These are not a proof of the Einstein equation. They certify the
-- arithmetic that the C++ tests also static_assert:
--   photon-sphere right-hand side vanishes at u = 1/3,
--   the critical potential there is 1/27 so b^2 = 27,
--   the linearization coefficient 6 u - 1 equals 1,
--   ISCO specific energy squared is 8/9 and L^2 is 12,
--   the Bardeen-Press-Teukolsky formula at a = 0 is r = 6.
--
-- Run, if Lean 4 is installed:  lean proofs/ClassicalGR.lean
-- CTest does not require this file.

-- (3 u^2 - u) * 9 at u = 1/3.
theorem photon_sphere_rhs_cleared : 3 * 1 * 1 * 3 - 1 * 9 = 0 := rfl

-- (6 u - 1) * 3 at u = 1/3 equals 3, hence 6 u - 1 = 1.
theorem photon_linearization_cleared : 6 * 1 - 1 * 3 = 3 := rfl

-- (u^2 - 2 u^3) * 27 at u = 1/3 equals 1, hence the value is 1/27
-- and b_crit^2 = 27.  (3 sqrt(3))^2 = 9 * 3 = 27.
theorem critical_potential_cleared : 3 - 2 = 1 := rfl
theorem shadow_radius_square : 3 * 3 * 3 = 27 := rfl

-- E^2 = (r - 2)^2 / (r (r - 3)) at r = 6 equals 8/9.
theorem schwarzschild_isco_energy_sq : (6 - 2) * (6 - 2) * 9 = 8 * 6 * (6 - 3) := rfl

-- L^2 = r^2 / (r - 3) at r = 6 equals 12.
theorem schwarzschild_isco_angmom_sq : 6 * 6 = 12 * (6 - 3) := rfl

-- BPT at a = 0: z1 = 1 + 1*(1+1) = 3, r = 3 + z2 with z2 = 3.
theorem bpt_z1_spin_zero : 1 + 1 * (1 + 1) = 3 := rfl
theorem bpt_isco_spin_zero : 3 + 3 = 6 := rfl
