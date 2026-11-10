# What to build from this archive

Ranked by what would change the simulator people actually run. Counts are how many of the 328 digest entries mention the topic in the title or the digest's own summary. A mention is not an instruction. Quantum-gravity entries are listed so they stay out of the classical kernel.

1. **Keep the vacuum shadow and the lensed disk as the reference, and treat everything else as a labeled comparison.** 59 entries mention a shadow, a photon ring, or a photon sphere, and 66 mention EHT, M87*, or Sgr A*. The code already has the Schwarzschild circle of radius sqrt(27) M and the Bardeen Kerr boundary. The useful next check is a table of this renderer's shadow size and asymmetry against the numbers those papers quote for Kerr, not a new metric inside the kernel.

2. **The star field behind the disk.** The README had named this as the open visual gap. It is the background the shadow papers assume when they talk about what an image shows around the hole. This change set adds it for the Schwarzschild disk: escaped rays sample a synthetic celestial sphere at the geodesic's escape azimuth. The catalog is a picture. The deflection is the orbit integral. The Kerr disk still has a dark floor, because that tracer does not yet return an escape direction.

3. **CUDA, one thread per pixel, checked against the CPU image.** Only 24 entries mention ray tracing, a GPU, or CUDA, and 4 mention wave optics. The archive does not contain a GPU recipe. The port is still the project's own headline, and the wave-optics papers (for example the 2026-10-24 digest's full-wave lensing note) are a reminder that the geometric rays are an approximation to be tested, not a license to skip the CPU oracle. The staged plan is ADR-0013. The build stays on the CPU when there is no GPU.

4. **Do not put plasma, hair, or a dark-matter halo into the default ray.** 13 entries mention plasma. Many more replace Kerr with a charged, hairy, or halo-dressed metric and then compare the shadow to EHT. Those are the papers that can fool a picture into looking "more realistic" while leaving the tested Kerr geodesic. If one of them is ever drawn, it needs its own truth label and a switch that defaults off.

5. **Thin-disk emissivity is still a picture on screen.** 28 entries mention accretion. The renderer still multiplies a radial ramp by g^4. That g is the GR redshift. The ramp is not a radiative-transfer model. The Page–Thorne flux itself is now a tested function (`include/blackhole_ds/disk/novikov_thorne.hpp`); it is not yet what the pixels use. Swapping the ramp for that flux is a later readout, not a new picture.

6. **Ringdown stays in the QNM module.** 94 entries mention ringdown or quasinormal modes. The Leaver solver, the WKB check, and the Berti fit are the place those papers can be compared. They do not change a pixel.

7. **Leave the information-paradox papers out of the integrator.** 47 entries mention a Page curve, an entanglement island, holography, or the information paradox. The feed's own header says this section must not touch the classical kernel. That rule still holds.

Not ranked, because the archive does not contain it: a 2025 daily feed. There is no such drop. See the audit.
