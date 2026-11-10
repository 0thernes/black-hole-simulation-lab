// SPDX-License-Identifier: LicenseRef-Proprietary-AllRightsReserved
// Copyright (c) 2026 0thernes <0_0@0thernes.art>. All Rights Reserved.
// blackhole_ds/viz/renderer.hpp
// Seam between the CLI and the ray tracer. Today every pixel is the CPU
// tracer in disk_image.hpp. ADR-0013 stages the CUDA port behind this
// function so a later device kernel can replace the body without a second
// call site. Until that kernel exists, render_lensed_disk is the CPU
// path even when the CUDA probe translation unit is linked.
//
// Truth tier: infrastructure. The pixels keep the tiers of disk_image.hpp
// and starfield.hpp.

#pragma once

#include "blackhole_ds/viz/disk_image.hpp"

namespace blackhole_ds::viz {

// "cpu" until a device kernel actually shades pixels. A linked CUDA probe
// does not change this string: the probe only asks how many devices exist.
[[nodiscard]] inline const char* render_backend_name() noexcept {
    return "cpu";
}

[[nodiscard]] inline Image render_lensed_disk(const DiskView& view) {
    return render_disk_image(view);
}

} // namespace blackhole_ds::viz
